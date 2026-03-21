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
* @date 18 Mar 2026
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

    /** @brief Second context for sibling dispatch of outgoing messages.
     *  Uses its own outgoing slot so sibling responses don't corrupt the
     *  main dispatch. Shares the same protocol handler functions. */
static openlcb_statemachine_info_t _sibling_statemachine_info;

    /** @brief Tracks whether any train node matched during the current enumeration. */
static bool _train_search_match_found;

    /** @brief TRUE while we are iterating siblings for an outgoing message. */
static bool _sibling_dispatch_active;

// ---- Sibling response queue (depth > 1 chains) ----

#define SIBLING_RESPONSE_QUEUE_DEPTH 5

    /** @brief Circular queue of sibling responses awaiting dispatch. */
static openlcb_worker_message_t _sibling_response_queue[SIBLING_RESPONSE_QUEUE_DEPTH];

static uint8_t _sibling_response_queue_head;
static uint8_t _sibling_response_queue_tail;

    /** @brief High-water mark for runtime monitoring of chain depth. */
static uint8_t _sibling_response_queue_high_water;

// ---- Path B pending slot ----

    /** @brief Single-slot pending message for Path B sibling dispatch.
     *  Holds a copy of the last application-layer send so the run loop
     *  can dispatch it to siblings. */
static openlcb_worker_message_t _path_b_pending_msg;
static openlcb_msg_t *_path_b_pending_ptr;
static bool _path_b_pending;

    /**
    * @brief Stores the callback interface and wires up the outgoing message buffer.
    *
    * @details Algorithm:
    * -# Store interface pointer
    * -# Link outgoing message buffer pointers, set payload type to STREAM
    * -# Clear message and payload, mark buffer as allocated
    * -# Set incoming message to NULL, clear enumerate flag, set node to NULL
    *
    * @verbatim
    * @param interface_openlcb_main_statemachine Pointer to populated interface structure
    * @endverbatim
    */
void OpenLcbMainStatemachine_initialize(
            const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine) {

    _interface = interface_openlcb_main_statemachine;

    // Main context — unchanged
    _statemachine_info.outgoing_msg_info.msg_ptr = &_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload =
            (openlcb_payload_t *) _statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload_type = WORKER;
    OpenLcbUtilities_clear_openlcb_message(_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_statemachine_info.outgoing_msg_info.msg_ptr);
    _statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _statemachine_info.incoming_msg_info.msg_ptr = NULL;
    _statemachine_info.incoming_msg_info.enumerate = false;
    _statemachine_info.openlcb_node = NULL;

    // Sibling context — same pattern, its own outgoing slot
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr = &_sibling_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->payload =
            (openlcb_payload_t *) _sibling_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->payload_type = WORKER;
    OpenLcbUtilities_clear_openlcb_message(_sibling_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_sibling_statemachine_info.outgoing_msg_info.msg_ptr);
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _sibling_statemachine_info.incoming_msg_info.msg_ptr = NULL;
    _sibling_statemachine_info.incoming_msg_info.enumerate = false;
    _sibling_statemachine_info.openlcb_node = NULL;

    _sibling_dispatch_active = false;

    // Path B pending slot
    _path_b_pending_ptr = &_path_b_pending_msg.openlcb_msg;
    _path_b_pending_ptr->payload = (openlcb_payload_t *) _path_b_pending_msg.openlcb_payload;
    _path_b_pending_ptr->payload_type = WORKER;
    _path_b_pending = false;

    // Sibling response queue
    for (int i = 0; i < SIBLING_RESPONSE_QUEUE_DEPTH; i++) {

        _sibling_response_queue[i].openlcb_msg.payload =
                (openlcb_payload_t *) _sibling_response_queue[i].openlcb_payload;
        _sibling_response_queue[i].openlcb_msg.payload_type = WORKER;

    }

    _sibling_response_queue_head = 0;
    _sibling_response_queue_tail = 0;
    _sibling_response_queue_high_water = 0;

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
// Sibling Response Queue Helpers
// ============================================================================

    /** @brief Copies a message into the sibling response queue for later dispatch. */
static void _sibling_response_queue_push(openlcb_msg_t *msg) {

    uint8_t next_tail = (_sibling_response_queue_tail + 1) % SIBLING_RESPONSE_QUEUE_DEPTH;

    if (next_tail == _sibling_response_queue_head) {

        return; // queue full — chain depth exceeds design limit

    }

    openlcb_msg_t *slot = &_sibling_response_queue[_sibling_response_queue_tail].openlcb_msg;

    slot->mti           = msg->mti;
    slot->source_alias  = msg->source_alias;
    slot->source_id     = msg->source_id;
    slot->dest_alias    = msg->dest_alias;
    slot->dest_id       = msg->dest_id;
    slot->payload_count = msg->payload_count;
    slot->state.loopback = true;

    for (uint16_t i = 0; i < msg->payload_count; i++) {

        *slot->payload[i] = *msg->payload[i];

    }

    _sibling_response_queue_tail = next_tail;

    uint8_t depth = (uint8_t) ((_sibling_response_queue_tail - _sibling_response_queue_head
            + SIBLING_RESPONSE_QUEUE_DEPTH) % SIBLING_RESPONSE_QUEUE_DEPTH);

    if (depth > _sibling_response_queue_high_water) {

        _sibling_response_queue_high_water = depth;

    }

}

    /** @brief Pops the next message from the sibling response queue. NULL if empty. */
static openlcb_msg_t *_sibling_response_queue_pop(void) {

    if (_sibling_response_queue_head == _sibling_response_queue_tail) {

        return NULL;

    }

    openlcb_msg_t *slot = &_sibling_response_queue[_sibling_response_queue_head].openlcb_msg;
    _sibling_response_queue_head = (_sibling_response_queue_head + 1) % SIBLING_RESPONSE_QUEUE_DEPTH;

    return slot;

}

// ============================================================================
// Sibling Dispatch Functions
// ============================================================================

    /**
     * @brief Begins sibling dispatch of the outgoing message.
     *
     * @details Called after handle_outgoing sends the message to the wire.
     * Points the sibling context's incoming_msg_info at the main outgoing
     * message, then fetches the first node for sibling iteration.
     *
     * @return true if sibling dispatch started, false if only 1 node (no siblings)
     */
static bool _sibling_dispatch_begin(void) {

    if (_interface->openlcb_node_get_count() <= 1) {

        return false;

    }

    // Point sibling's incoming at the outgoing message we just sent
    _sibling_statemachine_info.incoming_msg_info.msg_ptr =
            _statemachine_info.outgoing_msg_info.msg_ptr;
    _sibling_statemachine_info.incoming_msg_info.enumerate = false;

    // Mark the outgoing as loopback so self-skip works in does_node_process_msg
    _sibling_statemachine_info.incoming_msg_info.msg_ptr->state.loopback = true;

    // Get first sibling node
    _sibling_statemachine_info.openlcb_node =
            _interface->openlcb_node_get_first(OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX);

    _sibling_dispatch_active = true;

    return true;

}

    /**
     * @brief Sends the sibling's pending outgoing response to the wire.
     *
     * @details Also pushes the response to the sibling response queue so
     * other siblings can see it (depth > 1 chains).
     *
     * @return true if a message was pending (caller should retry), false if idle
     */
static bool _sibling_handle_outgoing(void) {

    if (_sibling_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_sibling_statemachine_info.outgoing_msg_info.msg_ptr)) {

            // Queue the response for sibling dispatch (depth > 1 chains)
            _sibling_response_queue_push(_sibling_statemachine_info.outgoing_msg_info.msg_ptr);

            _sibling_statemachine_info.outgoing_msg_info.valid = false;

        }

        return true;

    }

    return false;

}

    /**
     * @brief Re-enters the sibling handler for multi-message enumerate responses.
     *
     * @return true if re-enumeration active, false if complete
     */
static bool _sibling_handle_reenumerate(void) {

    if (_sibling_statemachine_info.incoming_msg_info.enumerate) {

        _interface->process_main_statemachine(&_sibling_statemachine_info);

        return true;

    }

    return false;

}

    /**
     * @brief Dispatches the outgoing message to the current sibling node.
     *
     * @details Skips the originating node (self-skip handled by
     * does_node_process_msg via source_id comparison). Dispatches to nodes
     * in RUNSTATE_RUN only.
     *
     * @return true if dispatch occurred or node skipped, false if no node
     */
static bool _sibling_dispatch_current(void) {

    if (!_sibling_statemachine_info.openlcb_node) {

        // Deferred cleanup path: last sibling's response was drained by 2a.
        // Clear the main outgoing slot now that dispatch is truly complete.
        _sibling_dispatch_active = false;
        _sibling_statemachine_info.incoming_msg_info.msg_ptr = NULL;
        _statemachine_info.outgoing_msg_info.msg_ptr->state.loopback = false;
        _statemachine_info.outgoing_msg_info.valid = false;

        return false;

    }

    if (_sibling_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

        _interface->process_main_statemachine(&_sibling_statemachine_info);

    }

    return true;

}

    /**
     * @brief Advances to the next sibling node.
     *
     * @return true if advanced (more siblings or reached end), false if not active
     */
static bool _sibling_dispatch_advance(void) {

    if (!_sibling_statemachine_info.openlcb_node) {

        return false;

    }

    _sibling_statemachine_info.openlcb_node =
            _interface->openlcb_node_get_next(OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX);

    if (!_sibling_statemachine_info.openlcb_node) {

        if (!_sibling_statemachine_info.outgoing_msg_info.valid) {

            // Last sibling has no pending response — deactivate immediately.
            _sibling_dispatch_active = false;
            _sibling_statemachine_info.incoming_msg_info.msg_ptr = NULL;

        }

        // If last sibling HAS a pending response, keep _sibling_dispatch_active = true
        // so Priority 2a fires next cycle to send it.  _sibling_dispatch_current()
        // will deactivate and clear the main slot once 2a has drained the response.

        return true;

    }

    return true;

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

    OpenLcbUtilities_copy_word_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL,
            0);

    OpenLcbUtilities_copy_word_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->incoming_msg_info.msg_ptr->mti,
            2);

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

            if (_interface->broadcast_time_event_handler && _interface->is_broadcast_time_event && statemachine_info->openlcb_node->index == 0) {

                event_id_t event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);
                if (_interface->is_broadcast_time_event(event_id)) {

                    _interface->broadcast_time_event_handler(statemachine_info, event_id);
                    break;

                }

            }

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

            event_id_t event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

            if (_interface->broadcast_time_event_handler && _interface->is_broadcast_time_event && statemachine_info->openlcb_node->index == 0) {

                if (_interface->is_broadcast_time_event(event_id)) {

                    _interface->broadcast_time_event_handler(statemachine_info, event_id);

                    break;

                }

            }

            // Global Emergency event intercept -- check ALL train nodes
            if (_interface->train_emergency_event_handler && _interface->is_emergency_event && statemachine_info->openlcb_node->train_state) {

                if (_interface->is_emergency_event(event_id)) {

                    _interface->train_emergency_event_handler(statemachine_info, event_id);

                    break;

                }

            }

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

            if (OpenLcbUtilities_is_addressed_message_for_node(
                        statemachine_info->openlcb_node, 
                        statemachine_info->incoming_msg_info.msg_ptr)) {

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

            // Start sibling dispatch if multiple nodes exist.
            // The outgoing slot stays valid until sibling dispatch completes.
            if (!_sibling_dispatch_begin()) {

                // Single node — no siblings, clear immediately
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

        _train_search_match_found = false;

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
    * @brief Runs one iteration of the main state machine dispatch loop.
    *
    * @details Priority order:
    * -# Send pending main outgoing (skip if held for sibling dispatch)
    * -# Sibling dispatch: send sibling response, reenumerate, dispatch current, advance
    * -# Check sibling response queue or Path B pending for next dispatch cycle
    * -# Re-enumerate main handler for multi-message responses
    * -# Pop next incoming message from FIFO
    * -# Enumerate first node for the message
    * -# Enumerate next node
    */
void OpenLcbMainStatemachine_run(void) {

    // ── Priority 1: Send pending main outgoing message ──────────────
    // If valid and NOT in sibling dispatch, try to send to wire.
    // If valid and IN sibling dispatch, skip (held for siblings to read).
    if (!_sibling_dispatch_active) {

        if (_interface->handle_outgoing_openlcb_message()) {

            return;

        }

    }

    // ── Priority 2: Sibling dispatch of the outgoing message ────────
    // After sending to wire, show the outgoing msg to each sibling.
    // One step per _run() call — same cadence as node enumeration.
    if (_sibling_dispatch_active) {

        // 2a: Send any pending sibling response to wire first
        if (_sibling_handle_outgoing()) {

            return;

        }

        // 2b: If sibling handler is mid-enumerate, continue it
        if (_sibling_handle_reenumerate()) {

            return;

        }

        // 2c: Dispatch to current sibling node
        if (_sibling_dispatch_current()) {

            // After dispatch, advance to next sibling for next _run()
            _sibling_dispatch_advance();

            // If dispatch just completed (no more siblings), clear main slot
            if (!_sibling_dispatch_active) {

                _statemachine_info.outgoing_msg_info.msg_ptr->state.loopback = false;
                _statemachine_info.outgoing_msg_info.valid = false;

            }

            return;

        }

    }

    // ── Priority 2.5: Sibling response queue or Path B pending ──────
    if (!_sibling_dispatch_active) {

        openlcb_msg_t *queued = _sibling_response_queue_pop();

        if (!queued && _path_b_pending) {

            queued = _path_b_pending_ptr;
            _path_b_pending = false;

        }

        if (queued) {

            _sibling_statemachine_info.incoming_msg_info.msg_ptr = queued;
            _sibling_statemachine_info.incoming_msg_info.enumerate = false;

            _sibling_statemachine_info.openlcb_node =
                    _interface->openlcb_node_get_first(OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX);

            _sibling_dispatch_active = true;

            return;

        }

    }

    // ── Priority 3: Re-enumerate main handler for multi-message ─────
    if (_interface->handle_try_reenumerate()) {

        return;

    }

    // ── Priority 4: Pop next incoming from wire FIFO ────────────────
    if (_interface->handle_try_pop_next_incoming_openlcb_message()) {

        return;

    }

    // ── Priority 5: Enumerate first node for incoming message ───────
    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    // ── Priority 6: Enumerate next node ─────────────────────────────
    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}

    /** @brief Returns pointer to internal state.  For unit testing only. */
openlcb_statemachine_info_t *OpenLcbMainStatemachine_get_statemachine_info(void) {

    return &_statemachine_info;

}

    /** @brief Returns pointer to sibling dispatch state.  For unit testing only. */
openlcb_statemachine_info_t *OpenLcbMainStatemachine_get_sibling_statemachine_info(void) {

    return &_sibling_statemachine_info;

}

    /** @brief Returns the high-water mark of the sibling response queue. */
uint8_t OpenLcbMainStatemachine_get_sibling_response_queue_high_water(void) {

    return _sibling_response_queue_high_water;

}

    /**
     * @brief Wrapper around the transport send that adds sibling dispatch.
     *
     * @details Algorithm:
     * -# Send the message to the wire via the real transport callback
     * -# If only one node, return immediately (no siblings)
     * -# Copy message header and payload into the Path B pending slot
     * -# Set loopback flag so self-skip works during sibling dispatch
     * -# The run loop will dispatch it to siblings on subsequent _run() calls
     *
     * @verbatim
     * @param msg  Pointer to the outgoing openlcb_msg_t (often stack-allocated)
     * @endverbatim
     *
     * @return true if wire send succeeded, false if transport busy
     */
bool OpenLcbMainStatemachine_send_with_sibling_dispatch(openlcb_msg_t *msg) {

    // Send to wire via the real transport function
    if (!_interface->send_openlcb_msg(msg)) {

        return false;

    }

    // If only one node, no siblings to notify
    if (_interface->openlcb_node_get_count() <= 1) {

        return true;

    }

    // Copy into pending slot for sibling dispatch by run loop
    openlcb_msg_t *pending = _path_b_pending_ptr;

    pending->mti           = msg->mti;
    pending->source_alias  = msg->source_alias;
    pending->source_id     = msg->source_id;
    pending->dest_alias    = msg->dest_alias;
    pending->dest_id       = msg->dest_id;
    pending->payload_count = msg->payload_count;
    pending->state.loopback = true;

    for (uint16_t i = 0; i < msg->payload_count; i++) {

        *pending->payload[i] = *msg->payload[i];

    }

    _path_b_pending = true;

    return true;

}
