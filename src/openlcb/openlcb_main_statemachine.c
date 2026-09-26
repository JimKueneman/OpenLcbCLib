/** \copyright
* Copyright (c) 2024, Jim Kueneman
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @file openlcb_main_statemachine.c
* @brief Implementation of the main OpenLCB protocol state machine dispatcher
*
* @details This file implements the central message routing and processing
* engine for OpenLCB protocol handling. The state machine provides a unified
* dispatch mechanism that routes incoming messages to appropriate protocol
* handlers based on Message Type Indicator (MTI) values.
*
* Architecture:
* The implementation uses a single static state machine context that maintains:
* - Current incoming message being processed
* - Outgoing message buffer for responses
* - Current node being enumerated
* - Interface callbacks for all protocol handlers
*
* Processing model:
* Messages are processed through node enumeration, where each incoming message
* is evaluated against every active node in the system. Nodes can filter
* messages based on addressing (global vs addressed) and node state.
*
* The main processing loop (run function) operates in priority order:
* 1. Transmit pending outgoing messages (highest priority)
* 2. Handle multi-message responses via re-enumeration
* 3. Pop new incoming message from queue
* 4. Enumerate nodes and dispatch to handlers
*
* Protocol support:
* - Required: Message Network Protocol, Protocol Support (PIP)
* - Optional: SNIP, Events, Train, Datagrams, Streams
* Optional protocols with NULL handlers automatically generate Interaction
* Rejected responses for compliance with OpenLCB specifications.
*
* Thread safety:
* Resource locking callbacks protect access to shared buffer pools and FIFOs.
*
* @author Jim Kueneman
* @date 23 Apr 2026
*
* @see openlcb_main_statemachine.h - Public interface
* @see openlcb_types.h - Core data structures
* @see OpenLCB Standard S-9.7.3 - Message Network Protocol
*/

#include "openlcb_main_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_list.h"
#include "openlcb_defines.h"
#include "openlcb_buffer_fifo.h"



    /** @brief Stored callback interface pointer for protocol handler dispatch. */
static const interface_openlcb_main_statemachine_t *_interface;

    /** @brief Static state machine context for message routing and node enumeration. */
static openlcb_statemachine_info_t _statemachine_info;

    // USER_DEFINED_SIBLING_DISPATCH_STACK_DEPTH defaults in openlcb_defines.h (it also sizes the node enumerator keys)

    /** @brief Default depth of the application send queue. */
#ifndef USER_DEFINED_APPLICATION_SEND_QUEUE_DEPTH
#define USER_DEFINED_APPLICATION_SEND_QUEUE_DEPTH 4
#endif

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
    /** @brief Tracks whether any train node matched during the current enumeration. */
static bool _train_search_match_found;
#endif /* OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH */

// ---- Sibling dispatch stack ----
//
// A message sent by a local node is sent to the wire and then shown to every
// other local node before the loop moves on.  When one of those nodes answers,
// its answer is finished the same way (wire, then every other local node)
// before the original message is shown to the next node: one level deeper on
// this stack.  Each level holds the message being shown, the node it is up to,
// and its own outgoing slot for the answer of that node.  Depth grows with how
// far answers chain, not with how many nodes there are.

    /** @brief Who owns the message a level is showing, released when the level finishes. */
typedef enum {

    SIBLING_MSG_OWNER_MAIN,
    SIBLING_MSG_OWNER_LEVEL,
    SIBLING_MSG_OWNER_APPLICATION_QUEUE

} sibling_msg_owner_enum;

    /** @brief One level of the sibling dispatch stack. */
typedef struct {

    openlcb_statemachine_info_t info;
    sibling_msg_owner_enum owner;

} sibling_dispatch_level_t;

    /** @brief The sibling dispatch stack. */
static sibling_dispatch_level_t _sibling_levels[USER_DEFINED_SIBLING_DISPATCH_STACK_DEPTH];

    /** @brief Number of active levels (0 = nothing being shown to siblings). */
static uint8_t _sibling_depth;

    /** @brief Deepest the stack has been, for runtime monitoring. */
static uint8_t _sibling_depth_high_water;

    /** @brief Messages that could not be shown to siblings because the stack was full. */
static uint16_t _sibling_depth_overflow_count;

// ---- Application send queue ----
//
// Application sends (including sends from callbacks) on a device with more
// than one node are queued, and each is sent and shown to the local nodes
// when the stack is empty and no incoming message is being processed.

    /** @brief Circular queue of application sends. */
static openlcb_worker_message_t _application_send_queue[USER_DEFINED_APPLICATION_SEND_QUEUE_DEPTH];

static uint8_t _application_send_queue_head;
static uint8_t _application_send_queue_count;

    /** @brief Application sends refused because the queue was full. */
static uint16_t _application_send_queue_overflow_count;

    /** @brief Wires a context's outgoing message buffer to its own storage. */
static void _initialize_context(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.msg_ptr = &statemachine_info->outgoing_msg_info.openlcb_msg.openlcb_msg;
    statemachine_info->outgoing_msg_info.msg_ptr->payload =
            (openlcb_payload_t *) statemachine_info->outgoing_msg_info.openlcb_msg.openlcb_payload;
    statemachine_info->outgoing_msg_info.msg_ptr->payload_type = WORKER;
    OpenLcbUtilities_clear_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);
    statemachine_info->outgoing_msg_info.msg_ptr->state.allocated = true;
    statemachine_info->outgoing_msg_info.valid = false;
    statemachine_info->outgoing_msg_info.enumerate = false;

    statemachine_info->incoming_msg_info.msg_ptr = NULL;
    statemachine_info->incoming_msg_info.enumerate = false;
    statemachine_info->openlcb_node = NULL;

}

    /**
    * @brief Stores the callback interface and wires up the message buffers.
    *
    * @details Algorithm:
    * -# Store interface pointer
    * -# Wire the main context and every sibling stack level to its own outgoing buffer
    * -# Wire the application send queue buffers and empty the queue
    *
    * @verbatim
    * @param interface_openlcb_main_statemachine Pointer to populated interface structure
    * @endverbatim
    */
void OpenLcbMainStatemachine_initialize(const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine) {

    _interface = interface_openlcb_main_statemachine;

    _initialize_context(&_statemachine_info);

    for (int i = 0; i < USER_DEFINED_SIBLING_DISPATCH_STACK_DEPTH; i++) {

        _initialize_context(&_sibling_levels[i].info);
        _sibling_levels[i].owner = SIBLING_MSG_OWNER_MAIN;

    }

    _sibling_depth = 0;
    _sibling_depth_high_water = 0;
    _sibling_depth_overflow_count = 0;

    for (int i = 0; i < USER_DEFINED_APPLICATION_SEND_QUEUE_DEPTH; i++) {

        _application_send_queue[i].openlcb_msg.payload =
                (openlcb_payload_t *) _application_send_queue[i].openlcb_payload;
        _application_send_queue[i].openlcb_msg.payload_type = WORKER;

    }

    _application_send_queue_head = 0;
    _application_send_queue_count = 0;
    _application_send_queue_overflow_count = 0;

}

    /** @brief Frees the current incoming message buffer (thread-safe, NULL-safe). */
static void _free_incoming_message(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info->incoming_msg_info.msg_ptr) {

        return;

    }

    _interface->lock_shared_resources();
    OpenLcbBufferStore_free_buffer(statemachine_info->incoming_msg_info.msg_ptr);
    _interface->unlock_shared_resources();
    statemachine_info->incoming_msg_info.msg_ptr = NULL;

}

// ============================================================================
// Sibling Dispatch Stack
// ============================================================================

    /**
     * @brief Starts showing a message just sent to the wire to the other local nodes.
     *
     * @details Pushes a level whose incoming message is msg.  The level's owner
     * keeps msg unchanged until the level finishes.  Nothing is pushed when
     * there is only one node, or when the stack is full (counted).
     *
     * @param msg    The message to show (already sent to the wire).
     * @param owner  Who owns msg, released when the level finishes.
     *
     * @return true if a level was pushed, false if msg needs no local delivery.
     */
static bool _sibling_push(openlcb_msg_t *msg, sibling_msg_owner_enum owner) {

    if (_interface->openlcb_node_get_count() <= 1) {

        return false;

    }

    if (_sibling_depth >= USER_DEFINED_SIBLING_DISPATCH_STACK_DEPTH) {

        _sibling_depth_overflow_count++;

        return false;

    }

    sibling_dispatch_level_t *level = &_sibling_levels[_sibling_depth];

    msg->state.loopback = true;   // the sender skips its own copy

    level->owner = owner;
    level->info.incoming_msg_info.msg_ptr = msg;
    level->info.incoming_msg_info.enumerate = false;
    level->info.outgoing_msg_info.valid = false;
    level->info.current_tick = _interface->get_current_tick();
    level->info.openlcb_node =
            _interface->openlcb_node_get_first(OPENLCB_SIBLING_STACK_NODE_ENUMERATOR_BASE + _sibling_depth);

    _sibling_depth++;

    if (_sibling_depth > _sibling_depth_high_water) {

        _sibling_depth_high_water = _sibling_depth;

    }

    return true;

}

    /** @brief Removes the oldest application send from the queue. */
static void _application_send_queue_pop_head(void) {

    if (_application_send_queue_count == 0) {

        return;

    }

    _application_send_queue_head = (_application_send_queue_head + 1) % USER_DEFINED_APPLICATION_SEND_QUEUE_DEPTH;
    _application_send_queue_count--;

}

    /** @brief Finishes the top level: releases the message it showed back to its owner. */
static void _sibling_pop(void) {

    sibling_dispatch_level_t *level = &_sibling_levels[_sibling_depth - 1];

    level->info.incoming_msg_info.msg_ptr->state.loopback = false;
    level->info.incoming_msg_info.msg_ptr = NULL;
    level->info.openlcb_node = NULL;

    _sibling_depth--;

    switch (level->owner) {

        case SIBLING_MSG_OWNER_MAIN:

            _statemachine_info.outgoing_msg_info.valid = false;

            break;

        case SIBLING_MSG_OWNER_LEVEL:

            _sibling_levels[_sibling_depth - 1].info.outgoing_msg_info.valid = false;

            break;

        case SIBLING_MSG_OWNER_APPLICATION_QUEUE:

            _application_send_queue_pop_head();

            break;

    }

}

    /** @brief Moves the top level on to the next node, unless its current node is still answering. */
static void _sibling_advance(sibling_dispatch_level_t *level) {

    if (level->info.incoming_msg_info.enumerate) {

        return;

    }

    level->info.openlcb_node =
            _interface->openlcb_node_get_next(OPENLCB_SIBLING_STACK_NODE_ENUMERATOR_BASE + (_sibling_depth - 1));

}

    /**
     * @brief One step of sibling delivery on the top level of the stack.
     *
     * @details Priority:
     * -# An answer made on this level goes to the wire, then one level deeper
     * -# A node still answering (enumerate) is called again for its next message
     * -# When every node has been shown the message, the level is finished
     * -# Otherwise the current node is shown the message
     */
static void _sibling_run_top(void) {

    sibling_dispatch_level_t *level = &_sibling_levels[_sibling_depth - 1];

    if (level->info.outgoing_msg_info.valid) {

        if (!_interface->send_openlcb_msg(level->info.outgoing_msg_info.msg_ptr)) {

            return;   // wire busy: stay here until it goes

        }

        if (!_sibling_push(level->info.outgoing_msg_info.msg_ptr, SIBLING_MSG_OWNER_LEVEL)) {

            level->info.outgoing_msg_info.valid = false;

        }

        return;

    }

    if (level->info.incoming_msg_info.enumerate) {

        _interface->process_main_statemachine(&level->info);
        _sibling_advance(level);

        return;

    }

    if (!level->info.openlcb_node) {

        _sibling_pop();

        return;

    }

    if (level->info.openlcb_node->state.run_state == RUNSTATE_RUN) {

        _interface->process_main_statemachine(&level->info);

    }

    _sibling_advance(level);

}

    /**
    * @brief Returns true if the node should process this message.
    *
    * @details Algorithm:
    * -# Return false if node is NULL or not initialized
    * -# Accept global (unaddressed) messages
    * -# Accept addressed messages whose dest alias/ID matches this node
    * -# Special case: always accept MTI_VERIFY_NODE_ID_GLOBAL
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @return true if node should process message, false otherwise
    */
bool OpenLcbMainStatemachine_does_node_process_msg(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info->openlcb_node) {

        return false;

    }

    if (!statemachine_info->incoming_msg_info.msg_ptr) {

        return false;

    }

    // Self-skip: the originating node must not process its own loopback copy
    if (statemachine_info->incoming_msg_info.msg_ptr->state.loopback &&
            statemachine_info->openlcb_node->id ==
            statemachine_info->incoming_msg_info.msg_ptr->source_id) {

        return false;

    }

    return ( (statemachine_info->openlcb_node->state.initialized) &&
            (
            ((statemachine_info->incoming_msg_info.msg_ptr->mti & MASK_DEST_ADDRESS_PRESENT) != 
                        MASK_DEST_ADDRESS_PRESENT) || // if not addressed process it
            (((statemachine_info->openlcb_node->alias == 
                        statemachine_info->incoming_msg_info.msg_ptr->dest_alias) || 
                        (statemachine_info->openlcb_node->id == 
                        statemachine_info->incoming_msg_info.msg_ptr->dest_id)) && 
                        ((statemachine_info->incoming_msg_info.msg_ptr->mti & MASK_DEST_ADDRESS_PRESENT) == 
                        MASK_DEST_ADDRESS_PRESENT)) ||
            (statemachine_info->incoming_msg_info.msg_ptr->mti == 
                        MTI_VERIFY_NODE_ID_GLOBAL) // special case
            )
            );

}

    /**
    * @brief Builds an Optional Interaction Rejected response for the current message.
    *
    * @details Algorithm:
    * -# Validate all required pointers (return early if NULL)
    * -# Load OIR message with error code and triggering MTI in payload
    * -# Set valid flag for transmission
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    */
void OpenLcbMainStatemachine_load_interaction_rejected(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info) {

        return;

    }

    if (!statemachine_info->openlcb_node) {

        return;

    }

    if (!statemachine_info->outgoing_msg_info.msg_ptr) {

        return;

    }

    if (!statemachine_info->incoming_msg_info.msg_ptr) {

        return;

    }

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_OPTIONAL_INTERACTION_REJECTED);

    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL, 0);

    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, statemachine_info->incoming_msg_info.msg_ptr->mti, 2);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Routes incoming message to the correct protocol handler based on MTI.
    *
    * @details Algorithm:
    * -# Return early if NULL or does_node_process_msg() is false
    * -# Switch on MTI (40 message types: SNIP, Message Network, PIP,
    *    Event Transport, Train, Datagram, Stream)
    * -# For optional handlers that are NULL: send Interaction Rejected
    *    on request MTIs, silently ignore reply/indication MTIs
    * -# Default: reject unknown addressed MTIs, ignore unknown global MTIs
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context with message and node information
    * @endverbatim
    */
void OpenLcbMainStatemachine_process_main_statemachine(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info) {

        return;

    }

    if (!_interface->does_node_process_msg(statemachine_info)) {

        return;

    }

    switch (statemachine_info->incoming_msg_info.msg_ptr->mti) {

        case MTI_SIMPLE_NODE_INFO_REQUEST:

            if (_interface->snip_simple_node_info_request) {

                _interface->snip_simple_node_info_request(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_SIMPLE_NODE_INFO_REPLY:

            if (_interface->snip_simple_node_info_reply) {

                _interface->snip_simple_node_info_reply(statemachine_info);

            }

            break;

        case MTI_INITIALIZATION_COMPLETE:

            if (_interface->message_network_initialization_complete) {

                _interface->message_network_initialization_complete(statemachine_info);

            }

            break;

        case MTI_INITIALIZATION_COMPLETE_SIMPLE:

            if (_interface->message_network_initialization_complete_simple) {

                _interface->message_network_initialization_complete_simple(statemachine_info);

            }

            break;

        case MTI_PROTOCOL_SUPPORT_INQUIRY:

            if (_interface->message_network_protocol_support_inquiry) {

                _interface->message_network_protocol_support_inquiry(statemachine_info);

            }

            break;

        case MTI_PROTOCOL_SUPPORT_REPLY:

            if (_interface->message_network_protocol_support_reply) {

                _interface->message_network_protocol_support_reply(statemachine_info);

            }

            break;

        case MTI_VERIFY_NODE_ID_ADDRESSED:

            if (_interface->message_network_verify_node_id_addressed) {

                _interface->message_network_verify_node_id_addressed(statemachine_info);

            }

            break;

        case MTI_VERIFY_NODE_ID_GLOBAL:

            if (_interface->message_network_verify_node_id_global) {

                _interface->message_network_verify_node_id_global(statemachine_info);

            }

            break;

        case MTI_VERIFIED_NODE_ID:
        case MTI_VERIFIED_NODE_ID_SIMPLE:

            if (_interface->message_network_verified_node_id) {

                _interface->message_network_verified_node_id(statemachine_info);

            }

            break;

        case MTI_OPTIONAL_INTERACTION_REJECTED:

            if (_interface->message_network_optional_interaction_rejected) {

                _interface->message_network_optional_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_TERMINATE_DUE_TO_ERROR:

            if (_interface->message_network_terminate_due_to_error) {

                _interface->message_network_terminate_due_to_error(statemachine_info);

            }

#ifdef OPENLCB_COMPILE_STREAM

            if (_interface->stream_terminate_due_to_error) {

                _interface->stream_terminate_due_to_error(statemachine_info);

            }

#endif /* OPENLCB_COMPILE_STREAM */

            break;

        case MTI_CONSUMER_IDENTIFY:

            if (_interface->event_transport_consumer_identify) {

                _interface->event_transport_consumer_identify(statemachine_info);

            }

            break;

        case MTI_CONSUMER_RANGE_IDENTIFIED:

            if (_interface->event_transport_consumer_range_identified) {

                _interface->event_transport_consumer_range_identified(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFIED_UNKNOWN:

            if (_interface->event_transport_consumer_identified_unknown) {

                _interface->event_transport_consumer_identified_unknown(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFIED_SET:

            if (_interface->event_transport_consumer_identified_set) {

                _interface->event_transport_consumer_identified_set(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFIED_CLEAR:

            if (_interface->event_transport_consumer_identified_clear) {

                _interface->event_transport_consumer_identified_clear(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFIED_RESERVED:

            if (_interface->event_transport_consumer_identified_reserved) {

                _interface->event_transport_consumer_identified_reserved(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFY: {

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)

            event_id_t producer_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

            bool is_train_search = _interface->train_search_event_handler &&
                                   _interface->is_train_search_event &&
                                   _interface->is_train_search_event(producer_event_id);

            if (is_train_search) {

                // Dispatch to train search handler for train nodes only
                if (statemachine_info->openlcb_node->train_state) {

                    _interface->train_search_event_handler(statemachine_info, producer_event_id);

                    if (statemachine_info->outgoing_msg_info.valid) {

                        _train_search_match_found = true;

                    }

                }

                // On last node with no match, invoke no-match handler
                if (_interface->openlcb_node_is_last(OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX) &&
                    !_train_search_match_found &&
                    _interface->train_search_no_match_handler) {

                    _interface->train_search_no_match_handler(statemachine_info, producer_event_id);

                }

                break;

            }

#endif /* OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH */

            if (_interface->event_transport_producer_identify) {

                _interface->event_transport_producer_identify(statemachine_info);

            }

            break;

        }

        case MTI_PRODUCER_RANGE_IDENTIFIED:

            if (_interface->event_transport_producer_range_identified) {

                _interface->event_transport_producer_range_identified(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFIED_UNKNOWN:

            if (_interface->event_transport_producer_identified_unknown) {

                _interface->event_transport_producer_identified_unknown(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFIED_SET:

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)

            if (_interface->train_search_reply_handler && _interface->is_train_search_event && statemachine_info->openlcb_node->index == 0) {

                event_id_t event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);
                if (_interface->is_train_search_event(event_id)) {

                    _interface->train_search_reply_handler(statemachine_info, event_id);
                    break;

                }

            }

#endif /* OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH */

#ifdef OPENLCB_COMPILE_BROADCAST_TIME

            if (_interface->broadcast_time_event_handler && _interface->is_broadcast_time_event && statemachine_info->openlcb_node->index == 0) {

                event_id_t event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);
                if (_interface->is_broadcast_time_event(event_id)) {

                    _interface->broadcast_time_event_handler(statemachine_info, event_id);
                    break;

                }

            }

#endif /* OPENLCB_COMPILE_BROADCAST_TIME */

            if (_interface->event_transport_producer_identified_set) {

                _interface->event_transport_producer_identified_set(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFIED_CLEAR:

            if (_interface->event_transport_producer_identified_clear) {

                _interface->event_transport_producer_identified_clear(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFIED_RESERVED:

            if (_interface->event_transport_producer_identified_reserved) {

                _interface->event_transport_producer_identified_reserved(statemachine_info);

            }

            break;

        case MTI_EVENTS_IDENTIFY_DEST:

            if (_interface->event_transport_identify_dest) {

                _interface->event_transport_identify_dest(statemachine_info);

            }

            break;

        case MTI_EVENTS_IDENTIFY:

            if (_interface->event_transport_identify) {

                _interface->event_transport_identify(statemachine_info);

            }

            break;

        case MTI_EVENT_LEARN:

            if (_interface->event_transport_learn) {

                _interface->event_transport_learn(statemachine_info);

            }

            break;

        case MTI_PC_EVENT_REPORT: {

#if defined(OPENLCB_COMPILE_BROADCAST_TIME) || defined(OPENLCB_COMPILE_TRAIN)

            event_id_t event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

#endif /* OPENLCB_COMPILE_BROADCAST_TIME || OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_BROADCAST_TIME

            if (_interface->broadcast_time_event_handler && _interface->is_broadcast_time_event && statemachine_info->openlcb_node->index == 0) {

                if (_interface->is_broadcast_time_event(event_id)) {

                    _interface->broadcast_time_event_handler(statemachine_info, event_id);

                    break;

                }

            }

#endif /* OPENLCB_COMPILE_BROADCAST_TIME */

#ifdef OPENLCB_COMPILE_TRAIN

            // Global Emergency event intercept -- check ALL train nodes
            if (_interface->train_emergency_event_handler && _interface->is_emergency_event && statemachine_info->openlcb_node->train_state) {

                if (_interface->is_emergency_event(event_id)) {

                    _interface->train_emergency_event_handler(statemachine_info, event_id);

                    break;

                }

            }

#endif /* OPENLCB_COMPILE_TRAIN */

            if (_interface->event_transport_pc_report) {

                _interface->event_transport_pc_report(statemachine_info);

            }

            break;

        }

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD:

            if (_interface->event_transport_pc_report_with_payload) {

                _interface->event_transport_pc_report_with_payload(statemachine_info);

            }

            break;

#ifdef OPENLCB_COMPILE_TRAIN

        case MTI_TRAIN_PROTOCOL:

            if (_interface->train_control_command) {

                _interface->train_control_command(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_TRAIN_REPLY:

            if (_interface->train_control_reply) {

                _interface->train_control_reply(statemachine_info);

            }

            break;

        case MTI_SIMPLE_TRAIN_INFO_REQUEST:

            if (_interface->simple_train_node_ident_info_request) {

                _interface->simple_train_node_ident_info_request(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_SIMPLE_TRAIN_INFO_REPLY:

            if (_interface->simple_train_node_ident_info_reply) {

                _interface->simple_train_node_ident_info_reply(statemachine_info);

            }

            break;

#endif /* OPENLCB_COMPILE_TRAIN */

        case MTI_DATAGRAM:

            if (_interface->datagram) {

                _interface->datagram(statemachine_info);

            } else {

                if (_interface->load_datagram_rejected) {

                    _interface->load_datagram_rejected(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED);

                }

            }

            break;

        case MTI_DATAGRAM_OK_REPLY:

            if (_interface->datagram_ok_reply) {

                _interface->datagram_ok_reply(statemachine_info);

            }

            break;

        case MTI_DATAGRAM_REJECTED_REPLY:

            if (_interface->datagram_rejected_reply) {

                _interface->datagram_rejected_reply(statemachine_info);

            }

            break;

        case MTI_STREAM_INIT_REQUEST:

            if (_interface->stream_initiate_request) {

                _interface->stream_initiate_request(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_STREAM_INIT_REPLY:

            if (_interface->stream_initiate_reply) {

                _interface->stream_initiate_reply(statemachine_info);

            }

            break;

        case MTI_STREAM_SEND:

            if (_interface->stream_send_data) {

                _interface->stream_send_data(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_STREAM_PROCEED:

            if (_interface->stream_data_proceed) {

                _interface->stream_data_proceed(statemachine_info);

            }

            break;

        case MTI_STREAM_COMPLETE:

            if (_interface->stream_data_complete) {

                _interface->stream_data_complete(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        default:

            if (OpenLcbUtilities_is_addressed_message_for_node(statemachine_info->openlcb_node, statemachine_info->incoming_msg_info.msg_ptr)) {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

    }


}

    /**
    * @brief Sends the pending outgoing message if one is valid.
    *
    * @details Algorithm:
    * -# If outgoing valid flag is set, call send_openlcb_msg callback
    * -# On success clear the valid flag
    * -# Return true if a message was pending, false if idle
    *
    * @return true if message pending (caller should retry), false if nothing to send
    */
bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg_info.msg_ptr)) {

            // Show it to the other local nodes; the outgoing slot stays valid
            // until that is finished.
            if (!_sibling_push(_statemachine_info.outgoing_msg_info.msg_ptr, SIBLING_MSG_OWNER_MAIN)) {

                // Single node (or stack full) - done
                _statemachine_info.outgoing_msg_info.valid = false;

            }

        }

        return true;

    }

    return false;

}

    /**
    * @brief Re-dispatches the current message when a handler requests multi-message enumeration.
    *
    * @details Algorithm:
    * -# If enumerate flag is set, call process_main_statemachine again
    * -# Return true while flag remains set, false when enumeration is complete
    *
    * @return true if re-enumeration active, false if complete
    */
bool OpenLcbMainStatemachine_handle_try_reenumerate(void) {

    if (_statemachine_info.incoming_msg_info.enumerate) {

        // Continue the processing of the incoming message on the node
        _interface->process_main_statemachine(&_statemachine_info);

        return true; // keep going until target clears the enumerate flag

    }

    return false;

}

    /**
    * @brief Pops the next incoming message from the receive FIFO when idle.
    *
    * @details Algorithm:
    * -# If already holding a message, return false
    * -# Lock shared resources, pop from FIFO, unlock
    * -# Return true if pop attempted (even if queue was empty), false if busy
    *
    * @return true if pop attempted, false if still processing previous message
    */
bool OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message(void) {

    if (!_statemachine_info.incoming_msg_info.msg_ptr) {

        _interface->lock_shared_resources();
        _statemachine_info.incoming_msg_info.msg_ptr = OpenLcbBufferFifo_pop();
        _interface->unlock_shared_resources();

        if (_statemachine_info.incoming_msg_info.msg_ptr &&
                _statemachine_info.incoming_msg_info.msg_ptr->state.invalid) {

            _free_incoming_message(&_statemachine_info);

            return true;

        }

        _statemachine_info.current_tick = _interface->get_current_tick();

        return (!_statemachine_info.incoming_msg_info.msg_ptr);

    }

    return false;

}

    /**
    * @brief Begins node enumeration by fetching the first node and dispatching the message.
    *
    * @details Algorithm:
    * -# If node pointer already set, return false (already enumerating)
    * -# Reset train search match flag for new enumeration
    * -# Get first node; if NULL free the message and return true
    * -# If node is in RUNSTATE_RUN, dispatch message via process_main_statemachine
    * -# Return true
    *
    * @return true if enumeration step taken, false if no action needed
    */
bool OpenLcbMainStatemachine_handle_try_enumerate_first_node(void) {

    if (!_statemachine_info.openlcb_node) {

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
        _train_search_match_found = false;
#endif /* OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH */

        _statemachine_info.openlcb_node =
                    _interface->openlcb_node_get_first(OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) {

            // no nodes are allocated yet, free the message buffer
            _free_incoming_message(&_statemachine_info);

            return true; // done

        }

        if (_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

            // Do the processing of the incoming message on the node
            _interface->process_main_statemachine(&_statemachine_info);

        }

        return true; // done

    }

    return false;

}

    /**
    * @brief Advances to the next node and dispatches the current message.
    *
    * @details Algorithm:
    * -# If no current node, return false
    * -# Get next node; if NULL free the message and return true
    * -# If node is in RUNSTATE_RUN, dispatch message via process_main_statemachine
    * -# Return true
    *
    * @return true if enumeration active, false if no current node
    */
bool OpenLcbMainStatemachine_handle_try_enumerate_next_node(void) {

    if (_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = 
                    _interface->openlcb_node_get_next(OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) {

            // reached the end of the list, free the incoming message
            _free_incoming_message(&_statemachine_info);

            return true; // done

        }

        if (_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

            // Do the processing of the incoming message on the node
            _interface->process_main_statemachine(&_statemachine_info);

        }

        return true; // done

    }

    return false;

}

    /**
     * @brief Sends the oldest queued application message and shows it to the local nodes.
     *
     * @details Only called when the stack is empty and no incoming message is
     * being processed, so everything caused by earlier messages has gone out.
     *
     * @return true if a queued message was pending (caller should return), false if idle
     */
static bool _handle_application_send_queue(void) {

    if (_application_send_queue_count == 0) {

        return false;

    }

    openlcb_msg_t *msg = &_application_send_queue[_application_send_queue_head].openlcb_msg;

    if (!_interface->send_openlcb_msg(msg)) {

        return true;   // wire busy: try again next pass

    }

    if (!_sibling_push(msg, SIBLING_MSG_OWNER_APPLICATION_QUEUE)) {

        _application_send_queue_pop_head();

    }

    return true;

}

    /**
    * @brief Runs one iteration of the main state machine dispatch loop.
    *
    * @details Priority order:
    * -# While the sibling stack is active, only it runs (one step)
    * -# Send pending main outgoing, then show it to the local nodes
    * -# Re-enumerate main handler for multi-message responses
    * -# With no incoming message in progress, send the next queued application message
    * -# Pop next incoming message from FIFO
    * -# Enumerate first node for the message
    * -# Enumerate next node
    */
void OpenLcbMainStatemachine_run(void) {

    // A message is being shown to the local nodes: finish it first.
    if (_sibling_depth > 0) {

        _sibling_run_top();

        return;

    }

    if (_interface->handle_outgoing_openlcb_message()) {

        return;

    }

    if (_interface->handle_try_reenumerate()) {

        return;

    }

    if (!_statemachine_info.incoming_msg_info.msg_ptr) {

        if (_handle_application_send_queue()) {

            return;

        }

    }

    if (_interface->handle_try_pop_next_incoming_openlcb_message()) {

        return;

    }

    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}

    /** @brief Returns pointer to internal state.  For unit testing only. */
openlcb_statemachine_info_t *OpenLcbMainStatemachine_get_statemachine_info(void) {

    return &_statemachine_info;

}

    /** @brief Returns pointer to the first sibling dispatch level.  For unit testing only. */
openlcb_statemachine_info_t *OpenLcbMainStatemachine_get_sibling_statemachine_info(void) {

    return &_sibling_levels[0].info;

}

    /** @brief Returns the deepest the sibling dispatch stack has been. */
uint8_t OpenLcbMainStatemachine_get_sibling_response_queue_high_water(void) {

    return _sibling_depth_high_water;

}

    /**
     * @brief Sends an application message, showing it to the other local nodes.
     *
     * @details With one node the message goes straight to the transport.
     * With several nodes it is copied into the application send queue; the
     * run loop sends it and shows it to the other local nodes when the stack
     * is empty and no incoming message is being processed.
     *
     * @verbatim
     * @param msg  Pointer to the outgoing openlcb_msg_t (often stack-allocated)
     * @endverbatim
     *
     * @return true if sent (one node) or queued, false if the transport is
     *         busy (one node) or the queue is full (caller retries)
     */
bool OpenLcbMainStatemachine_send_with_sibling_dispatch(openlcb_msg_t *msg) {

    if (_interface->openlcb_node_get_count() <= 1) {

        return _interface->send_openlcb_msg(msg);

    }

    if ((_application_send_queue_count >= USER_DEFINED_APPLICATION_SEND_QUEUE_DEPTH) ||
            (msg->payload_count > LEN_MESSAGE_BYTES_WORKER)) {

        _application_send_queue_overflow_count++;

        return false;

    }

    uint8_t tail = (_application_send_queue_head + _application_send_queue_count) % USER_DEFINED_APPLICATION_SEND_QUEUE_DEPTH;
    openlcb_msg_t *slot = &_application_send_queue[tail].openlcb_msg;

    slot->mti           = msg->mti;
    slot->source_alias  = msg->source_alias;
    slot->source_id     = msg->source_id;
    slot->dest_alias    = msg->dest_alias;
    slot->dest_id       = msg->dest_id;
    slot->payload_count = msg->payload_count;
    slot->state.loopback = false;

    for (uint16_t i = 0; i < msg->payload_count; i++) {

        *slot->payload[i] = *msg->payload[i];

    }

    _application_send_queue_count++;

    return true;

}
