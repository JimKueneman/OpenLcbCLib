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
* - Optional: SNIP, Events, Traction, Datagrams, Streams
* Optional protocols with NULL handlers automatically generate Interaction
* Rejected responses for compliance with OpenLCB specifications.
*
* Thread safety:
* Resource locking callbacks protect access to shared buffer pools and FIFOs.
*
* @author Jim Kueneman
* @date 17 Jan 2026
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



static const interface_openlcb_main_statemachine_t *_interface;

static openlcb_statemachine_info_t _statemachine_info;

    /**
    * @brief Initializes the main OpenLCB state machine
    *
    * @details Algorithm:
    * -# Store interface pointer for later use
    * -# Initialize outgoing message buffer pointer structure
    * -# Configure outgoing payload pointer and type (STREAM)
    * -# Clear outgoing message and payload structures
    * -# Mark outgoing buffer as allocated
    * -# Initialize incoming message pointer to NULL
    * -# Clear enumerate flag
    * -# Initialize node pointer to NULL
    *
    * The outgoing message buffer is pre-allocated within the state machine
    * context to avoid dynamic allocation during message processing. This
    * buffer is reused for all outgoing responses.
    *
    * Use cases:
    * - Application startup initialization
    * - System configuration during boot
    * - Test harness setup
    *
    * @verbatim
    * @param interface_openlcb_main_statemachine Pointer to populated interface structure
    * @endverbatim
    *
    * @warning Must be called before the specified function
    * @warning The interface structure must remain valid for application lifetime
    * @warning All required function pointers must be non-NULL
    *
    * @attention Call only once during initialization
    * @attention Assumes interface parameter is non-NULL (caller responsibility)
    *
    * @see OpenLcbMainStatemachine_run - Main processing loop
    * @see interface_openlcb_main_statemachine_t - Interface structure definition
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

    /**
    * @brief Frees the current incoming message buffer
    *
    * @details Algorithm:
    * -# Check if incoming message pointer is NULL
    * -# If NULL, return early (nothing to free)
    * -# Acquire resource lock
    * -# Free buffer back to buffer store
    * -# Release resource lock
    * -# Set incoming message pointer to NULL
    *
    * This internal helper ensures thread-safe buffer release and prevents
    * double-free by NULL-checking and clearing the pointer after free.
    *
    * Use cases:
    * - Message processing complete
    * - Node enumeration finished
    * - No nodes available to process message
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @warning Assumes pointer is non-NULL
    *
    * @note Thread-safe via lock/unlock callbacks
    * @note Safe to call with NULL message pointer
    *
    * @see OpenLcbBufferStore_free_buffer - Buffer release function
    */
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
    * @brief Determines if current node should process the message
    *
    * @details Algorithm:
    * -# Check if node pointer is NULL (return false if so)
    * -# Evaluate node initialized state
    * -# Check if message is global (not addressed)
    * -# Check if message is addressed to this node (alias or ID match)
    * -# Special case: allow Verify Node ID Global through
    * -# Return true if any condition allows processing
    *
    * Message processing logic:
    * A node processes a message if ALL of these are true:
    * - Node is initialized
    * AND any of:
    *   - Message has no destination address (global message)
    *   - Message destination matches node alias or ID
    *   - Message is MTI_VERIFY_NODE_ID_GLOBAL (special case)
    *
    * The Verify Node ID Global is special because the handler determines
    * whether to respond based on optional node ID in the payload.
    *
    * Use cases:
    * - Message filtering in main state machine
    * - Unit testing address matching
    * - Custom routing logic
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @return true if node should process message, false otherwise
    *
    * @warning Returns false if statemachine_info or openlcb_node is NULL
    *
    * @note Returns false for uninitialized nodes
    *
    * @see OpenLcbMainStatemachine_process_main_statemachine - Uses this for filtering
    * @see MASK_DEST_ADDRESS_PRESENT - MTI addressing bit mask
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
    * @brief Loads an Optional Interaction Rejected response message
    *
    * @details Algorithm:
    * -# Validate statemachine_info is not NULL (return if NULL)
    * -# Validate openlcb_node is not NULL (return if NULL)
    * -# Validate outgoing message pointer is not NULL (return if NULL)
    * -# Validate incoming message pointer is not NULL (return if NULL)
    * -# Load base Optional Interaction Rejected message with source/dest info
    * -# Copy error code to payload at offset 0
    * -# Copy triggering MTI to payload at offset 2
    * -# Mark outgoing message as valid for transmission
    *
    * Error code used: ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL
    * This indicates the MTI is not implemented or the transport protocol is unknown.
    *
    * Payload structure:
    * - Bytes 0-1: Error code (16-bit)
    * - Bytes 2-3: MTI that triggered rejection (16-bit)
    *
    * Use cases:
    * - Default handler for unimplemented optional protocols
    * - Response to unknown addressed messages
    * - Protocol capability advertisement
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @warning Returns early if any required pointers are NULL
    * @warning Assumes pointer message buffer is available
    *
    * @attention Used automatically by state machine for unhandled MTIs
    *
    * @see OpenLcbMainStatemachine_process_main_statemachine - Calls this for unhandled MTIs
    * @see MTI_OPTIONAL_INTERACTION_REJECTED - Response MTI value
    * @see ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL - Error code
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
    * @brief Processes incoming message through protocol handlers
    *
    * @details Algorithm:
    * -# Validate statemachine_info parameter (return if NULL)
    * -# Check if node should process message via does_node_process_msg
    * -# If node should not process, return early
    * -# Switch on incoming message MTI value
    * -# For each MTI case:
    *    - Check if corresponding handler exists (non-NULL)
    *    - If handler exists, invoke it with statemachine_info
    *    - If handler is NULL (optional protocol):
    *      * For request MTIs: call load_interaction_rejected
    *      * For reply/indication MTIs: ignore (no action)
    * -# Default case: If message is addressed to node, send Interaction Rejected
    *
    * MTI Dispatch Table (40 message types supported):
    *
    * SNIP Protocol:
    * - MTI_SIMPLE_NODE_INFO_REQUEST → snip_simple_node_info_request (or reject)
    * - MTI_SIMPLE_NODE_INFO_REPLY → snip_simple_node_info_reply
    *
    * Message Network Protocol (required):
    * - MTI_INITIALIZATION_COMPLETE → message_network_initialization_complete
    * - MTI_INITIALIZATION_COMPLETE_SIMPLE → message_network_initialization_complete_simple
    * - MTI_VERIFY_NODE_ID_ADDRESSED → message_network_verify_node_id_addressed
    * - MTI_VERIFY_NODE_ID_GLOBAL → message_network_verify_node_id_global
    * - MTI_VERIFIED_NODE_ID → message_network_verified_node_id
    * - MTI_OPTIONAL_INTERACTION_REJECTED → message_network_optional_interaction_rejected
    * - MTI_TERMINATE_DO_TO_ERROR → message_network_terminate_due_to_error
    *
    * Protocol Support Protocol/PIP (required):
    * - MTI_PROTOCOL_SUPPORT_INQUIRY → message_network_protocol_support_inquiry
    * - MTI_PROTOCOL_SUPPORT_REPLY → message_network_protocol_support_reply
    *
    * Event Transport Protocol:
    * - MTI_CONSUMER_IDENTIFY → event_transport_consumer_identify
    * - MTI_CONSUMER_RANGE_IDENTIFIED → event_transport_consumer_range_identified
    * - MTI_CONSUMER_IDENTIFIED_UNKNOWN → event_transport_consumer_identified_unknown
    * - MTI_CONSUMER_IDENTIFIED_SET → event_transport_consumer_identified_set
    * - MTI_CONSUMER_IDENTIFIED_CLEAR → event_transport_consumer_identified_clear
    * - MTI_CONSUMER_IDENTIFIED_RESERVED → event_transport_consumer_identified_reserved
    * - MTI_PRODUCER_IDENTIFY → event_transport_producer_identify
    * - MTI_PRODUCER_RANGE_IDENTIFIED → event_transport_producer_range_identified
    * - MTI_PRODUCER_IDENTIFIED_UNKNOWN → event_transport_producer_identified_unknown
    * - MTI_PRODUCER_IDENTIFIED_SET → event_transport_producer_identified_set
    * - MTI_PRODUCER_IDENTIFIED_CLEAR → event_transport_producer_identified_clear
    * - MTI_PRODUCER_IDENTIFIED_RESERVED → event_transport_producer_identified_reserved
    * - MTI_EVENTS_IDENTIFY_DEST → event_transport_identify_dest
    * - MTI_EVENTS_IDENTIFY → event_transport_identify
    * - MTI_EVENT_LEARN → event_transport_learn
    * - MTI_PC_EVENT_REPORT → event_transport_pc_report
    * - MTI_PC_EVENT_REPORT_WITH_PAYLOAD → event_transport_pc_report_with_payload
    *
    * Traction Protocol:
    * - MTI_TRACTION_PROTOCOL → traction_control_command (or reject)
    * - MTI_TRACTION_REPLY → traction_control_reply
    * - MTI_SIMPLE_TRAIN_INFO_REQUEST → simple_train_node_ident_info_request (or reject)
    * - MTI_SIMPLE_TRAIN_INFO_REPLY → simple_train_node_ident_info_reply
    *
    * Datagram Protocol:
    * - MTI_DATAGRAM → datagram
    * - MTI_DATAGRAM_OK_REPLY → datagram_ok_reply
    * - MTI_DATAGRAM_REJECTED_REPLY → datagram_rejected_reply
    *
    * Stream Protocol:
    * - MTI_STREAM_INIT_REQUEST → stream_initiate_request
    * - MTI_STREAM_INIT_REPLY → stream_initiate_reply
    * - MTI_STREAM_SEND → stream_send_data
    * - MTI_STREAM_PROCEED → stream_data_proceed
    * - MTI_STREAM_COMPLETE → stream_data_complete
    *
    * Interaction Rejected behavior:
    * - Request messages (SNIP request, Traction command, Train info request):
    *   Generate Interaction Rejected if handler is NULL
    * - Reply/indication messages: Silently ignored if handler is NULL
    * - Unknown addressed MTIs: Generate Interaction Rejected
    * - Unknown global MTIs: Silently ignored
    *
    * Use cases:
    * - Internal message dispatching (called by run)
    * - Unit testing of message handling
    * - Custom message processing scenarios
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context with message and node information
    * @endverbatim
    *
    * @warning Dereferences statemachine_info - must not be NULL
    * @warning Calls interface function pointers - interface must be initialized
    *
    * @attention Typically called internally by run function
    *
    * @see OpenLcbMainStatemachine_run - Main caller of this function
    * @see OpenLcbMainStatemachine_does_node_process_msg - Message filtering
    * @see OpenLcbMainStatemachine_load_interaction_rejected - Rejection handler
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


            if (_interface->message_network_verified_node_id) {

                _interface->message_network_verified_node_id(statemachine_info);

            }

            break;

        case MTI_OPTIONAL_INTERACTION_REJECTED:

            if (_interface->message_network_optional_interaction_rejected) {

                _interface->message_network_optional_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_TERMINATE_DO_TO_ERROR:

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

        case MTI_PRODUCER_IDENTIFY:

            if (_interface->event_transport_producer_identify) {

                _interface->event_transport_producer_identify(statemachine_info);

            }

            break;

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

        case MTI_PC_EVENT_REPORT:

            if (_interface->broadcast_time_event_handler) {

                event_id_t event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

                if (OpenLcbUtilities_is_broadcast_time_event(event_id))
                {

                    _interface->broadcast_time_event_handler(statemachine_info, event_id);

                    break;
                    
                }
            }

            if (_interface->event_transport_pc_report) {

                _interface->event_transport_pc_report(statemachine_info);

            }

            break;

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD:

            if (_interface->event_transport_pc_report_with_payload) {

                _interface->event_transport_pc_report_with_payload(statemachine_info);

            }

            break;

        case MTI_TRACTION_PROTOCOL:

            if (_interface->traction_control_command) {

                _interface->traction_control_command(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_TRACTION_REPLY:

            if (_interface->traction_control_reply) {

                _interface->traction_control_reply(statemachine_info);

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
    * @brief Handles transmission of pending outgoing messages
    *
    * @details Algorithm:
    * -# Check if outgoing message is marked as valid
    * -# If valid, attempt to send via interface send_openlcb_msg callback
    * -# If send succeeds, clear the valid flag (message sent)
    * -# Return true if message was pending (caller should keep trying)
    * -# Return false if no message to send
    *
    * This function implements a simple retry mechanism. If the send function
    * returns false (unable to send), the valid flag remains set and the caller
    * will retry on the next iteration.
    *
    * Use cases:
    * - Outgoing message transmission in run loop
    * - Response message handling
    * - Unit testing of send logic
    *
    * @return true if message pending (keep calling), false if no message to send
    *
    * @note Keeps trying until message is successfully transmitted
    *
    * @attention Exposed for testing, normally called by run function
    *
    * @see OpenLcbMainStatemachine_run - Main caller
    * @see interface_openlcb_main_statemachine_t - send_openlcb_msg callback
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
    * @brief Handles re-enumeration for multi-message responses
    *
    * @details Algorithm:
    * -# Check if enumerate flag is set in incoming message info
    * -# If set, call process_main_statemachine with current context
    * -# Return true (continue enumeration until handler clears flag)
    * -# If not set, return false (enumeration complete)
    *
    * The enumerate flag is set by protocol handlers when they need to send
    * multiple response messages for a single incoming message. The handler
    * sets the flag, sends one message, and returns. On the next iteration,
    * this function calls the handler again with the same incoming message
    * until the handler clears the flag.
    *
    * Example: Event identification might need to send multiple Producer
    * Identified messages, one per event.
    *
    * Use cases:
    * - Multi-message protocol responses
    * - Event identification with multiple events
    * - Large data transfers requiring segmentation
    *
    * @return true if re-enumeration active (keep calling), false if complete
    *
    * @note Handler must clear enumerate flag when done
    *
    * @attention Exposed for testing, normally called by run function
    *
    * @see OpenLcbMainStatemachine_run - Main caller
    * @see OpenLcbMainStatemachine_process_main_statemachine - Message processor
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
    * @brief Pops next incoming message from receive queue
    *
    * @details Algorithm:
    * -# Check if incoming message pointer is already set
    * -# If already have message, return false (keep processing current)
    * -# Acquire resource lock
    * -# Pop message from buffer FIFO
    * -# Release resource lock
    * -# Return true if no message popped (continue to next step)
    * -# Return false if still processing message
    *
    * The function only pops a new message when the current message pointer
    * is NULL, indicating the previous message has been fully processed and
    * freed.
    *
    * Use cases:
    * - Message queue management in run loop
    * - Incoming message retrieval
    * - Unit testing message flow
    *
    * @return true if message popped or attempted (continue processing), false if already have message
    *
    * @note Uses lock/unlock callbacks for thread safety
    * @note Returns true when no message available to signal continuation
    *
    * @attention Exposed for testing, normally called by run function
    *
    * @see OpenLcbMainStatemachine_run - Main caller
    * @see OpenLcbBufferFifo_pop - FIFO access function
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
    * @brief Enumerates first node for message processing
    *
    * @details Algorithm:
    * -# Check if node pointer is already set
    * -# If already set, return false (continue to next node enum)
    * -# Get first node from node list using OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX
    * -# If no nodes exist (NULL returned):
    *    - Free the incoming message (no nodes to process it)
    *    - Return true (enumeration complete)
    * -# If node is in RUNSTATE_RUN:
    *    - Process message through state machine
    * -# Return true (first node enumeration complete)
    *
    * This begins the node enumeration process. Nodes not in RUN state are
    * skipped silently as they are not ready to process messages.
    *
    * Use cases:
    * - Node enumeration in run loop
    * - First node message processing
    * - Unit testing enumeration logic
    *
    * @return true if enumeration step complete (stop or continue to next), false if no action taken
    *
    * @note Skips nodes not in RUNSTATE_RUN
    * @note Frees message if no nodes allocated
    *
    * @attention Exposed for testing, normally called by run function
    *
    * @see OpenLcbMainStatemachine_run - Main caller
    * @see OpenLcbMainStatemachine_handle_try_enumerate_next_node - Continues enumeration
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
    * @brief Enumerates next node for message processing
    *
    * @details Algorithm:
    * -# Check if current node pointer is set
    * -# If not set, return false (no enumeration in progress)
    * -# Get next node from node list using OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX
    * -# If no more nodes (NULL returned):
    *    - Free the incoming message (all nodes processed)
    *    - Return true (enumeration complete)
    * -# If node is in RUNSTATE_RUN:
    *    - Process message through state machine
    * -# Return true (next node enumeration complete)
    *
    * This continues the node enumeration started by enumerate_first_node.
    * The function is called repeatedly until all nodes have been enumerated
    * and the incoming message has been offered to each active node.
    *
    * Use cases:
    * - Node enumeration continuation in run loop
    * - Multi-node message processing
    * - Unit testing enumeration logic
    *
    * @return true if enumeration active (keep calling), false if no current node
    *
    * @note Skips nodes not in RUNSTATE_RUN
    * @note Frees message when enumeration complete
    *
    * @attention Exposed for testing, normally called by run function
    *
    * @see OpenLcbMainStatemachine_run - Main caller
    * @see OpenLcbMainStatemachine_handle_try_enumerate_first_node - Starts enumeration
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
    * @brief Main state machine processing loop
    *
    * @details Algorithm:
    * -# Attempt to send any pending outgoing messages (highest priority)
    * -# If outgoing message pending, return (retry send next iteration)
    * -# Handle multi-message response re-enumeration
    * -# If re-enumeration active, return (continue same message)
    * -# Pop next incoming message from queue
    * -# If message pop attempted, return (process message next iteration)
    * -# Enumerate first node for message processing
    * -# If first node enumeration done, return
    * -# Enumerate next node for message processing
    * -# If next node enumeration done, return
    *
    * Processing priority (early return on work):
    * 1. Outgoing message transmission - Ensures responses go out promptly
    * 2. Re-enumeration - Complete multi-message sequences before new messages
    * 3. Incoming message retrieval - Get new work when idle
    * 4. First node enumeration - Start processing new message
    * 5. Next node enumeration - Continue processing across all nodes
    *
    * This design ensures fair processing where each step gets a chance to
    * execute before moving to the next. Early returns prevent any single
    * operation from blocking others.
    *
    * Use cases:
    * - Called repeatedly from main application loop
    * - Invoked from RTOS task
    * - Triggered by timer interrupt
    *
    * @warning Call as fast as possible for low latency
    * @warning Do not block in this function or callbacks
    *
    * @attention Requires OpenLcbMainStatemachine_initialize called first
    *
    * @see OpenLcbMainStatemachine_initialize - Must be called first
    * @see OpenLcbMainStatemachine_process_main_statemachine - Message dispatcher
    * @see OpenLcbMainStatemachine_handle_outgoing_openlcb_message - Step 1
    * @see OpenLcbMainStatemachine_handle_try_reenumerate - Step 2
    * @see OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message - Step 3
    * @see OpenLcbMainStatemachine_handle_try_enumerate_first_node - Step 4
    * @see OpenLcbMainStatemachine_handle_try_enumerate_next_node - Step 5
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

    /**
    * @brief Returns pointer to internal state machine information structure
    *
    * @details Algorithm:
    * -# Return address of static _statemachine_info structure
    *
    * This function provides controlled access to the internal state machine
    * context for testing and debugging purposes. The structure contains all
    * state information including current node, incoming/outgoing messages,
    * and enumeration flags.
    *
    * Typical testing workflow:
    * -# Initialize state machine normally
    * -# Get pointer to state structure
    * -# Manually set flags or messages for test scenario
    * -# Call function under test
    * -# Verify state changes and results
    * -# Reset state for next test
    *
    * State structure contents:
    * - openlcb_node: Currently processing node
    * - incoming_msg_info.msg_ptr: Incoming message being processed
    * - incoming_msg_info.enumerate: Multi-message response flag
    * - outgoing_msg_info.msg_ptr: Outgoing message buffer
    * - outgoing_msg_info.valid: Outgoing message pending flag
    *
    * Use cases:
    * - Unit testing handle_outgoing_openlcb_message()
    * - Unit testing handle_try_reenumerate()
    * - Unit testing handle_try_pop_next_incoming_openlcb_message()
    * - Unit testing handle_try_enumerate_first_node()
    * - Unit testing handle_try_enumerate_next_node()
    * - Integration testing of run() function
    * - Debugging state machine behavior
    *
    * @return Pointer to internal static _statemachine_info structure
    *
    * @warning For testing/debugging only - not for production use
    * @warning Direct state modification bypasses normal validation
    *
    * @attention Tests must restore state to valid condition after use
    * @attention Do not cache pointer across initialize() calls
    *
    * @note Similar to OpenLcbLoginStatemachine_get_statemachine_info()
    *
    * @see openlcb_statemachine_info_t - Structure definition
    * @see OpenLcbMainStatemachine_initialize - State initialization
    * @see OpenLcbMainStatemachine_run - Main state consumer
    */
openlcb_statemachine_info_t *OpenLcbMainStatemachine_get_statemachine_info(void) {

    return &_statemachine_info;

}
