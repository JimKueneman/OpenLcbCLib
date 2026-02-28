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
* @date 28 Feb 2026
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
#include <stdio.h> // printf

#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_list.h"
#include "openlcb_defines.h"
#include "openlcb_buffer_fifo.h"



    /** @brief Stored callback interface pointer for protocol handler dispatch. */
static const interface_openlcb_main_statemachine_t *_interface;

    /** @brief Static state machine context for message routing and node enumeration. */
static openlcb_statemachine_info_t _statemachine_info;

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

    _statemachine_info.outgoing_msg_info.msg_ptr = &_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload = 
            (openlcb_payload_t *) _statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload_type = STREAM;
    OpenLcbUtilities_clear_openlcb_message(_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_statemachine_info.outgoing_msg_info.msg_ptr);
    _statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _statemachine_info.incoming_msg_info.msg_ptr = NULL;
    _statemachine_info.incoming_msg_info.enumerate = false;
    _statemachine_info.openlcb_node = NULL;

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

            // Train Search intercept -- check ALL train nodes
            if (_interface->train_search_event_handler && statemachine_info->openlcb_node->train_state) {

                if (OpenLcbUtilities_is_train_search_event(producer_event_id)) {

                    _interface->train_search_event_handler(statemachine_info, producer_event_id);

                    break;

                }

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

            if (_interface->broadcast_time_event_handler && statemachine_info->openlcb_node->index == 0) {

                event_id_t event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);
                if (OpenLcbUtilities_is_broadcast_time_event(event_id)) {

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

            if (_interface->broadcast_time_event_handler && statemachine_info->openlcb_node->index == 0) {

                if (OpenLcbUtilities_is_broadcast_time_event(event_id))
                {

                    _interface->broadcast_time_event_handler(statemachine_info, event_id);

                    break;

                }
            }

            // Global Emergency event intercept -- check ALL train nodes
            if (_interface->train_emergency_event_handler && statemachine_info->openlcb_node->train_state) {

                if (OpenLcbUtilities_is_emergency_event(event_id)) {

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

            _statemachine_info.outgoing_msg_info.valid = false; // done

        }

        return true; // keep trying till it can get sent

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

        return (!_statemachine_info.incoming_msg_info.msg_ptr);

    }

    return false;

}

    /**
    * @brief Begins node enumeration by fetching the first node and dispatching the message.
    *
    * @details Algorithm:
    * -# If node pointer already set, return false (already enumerating)
    * -# Get first node; if NULL free the message and return true
    * -# If node is in RUNSTATE_RUN, dispatch message via process_main_statemachine
    * -# Return true
    *
    * @return true if enumeration step taken, false if no action needed
    */
bool OpenLcbMainStatemachine_handle_try_enumerate_first_node(void) {

    if (!_statemachine_info.openlcb_node) {

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
    * @details Algorithm:
    * -# Send pending outgoing message (highest priority), return if busy
    * -# Handle multi-message re-enumeration, return if active
    * -# Pop next incoming message from FIFO, return if attempted
    * -# Enumerate first node for the message, return if acted
    * -# Enumerate next node, return if acted
    */
void OpenLcbMainStatemachine_run(void) {

    // Get any pending message out first
    if (_interface->handle_outgoing_openlcb_message()) {

        return;

    }

    // If the message handler needs to send multiple messages then 
    // enumerate the same incoming/outgoing message again
    if (_interface->handle_try_reenumerate()) {

        return;

    }

    // Pop the next incoming message and dispatch it to the active node
    if (_interface->handle_try_pop_next_incoming_openlcb_message()) {

        return;

    }

    // Grab the first OpenLcb Node
    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    // Enumerate all the OpenLcb Nodes
    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}

    /** @brief Returns pointer to internal state.  For unit testing only. */
openlcb_statemachine_info_t *OpenLcbMainStatemachine_get_statemachine_info(void) {

    return &_statemachine_info;

}
