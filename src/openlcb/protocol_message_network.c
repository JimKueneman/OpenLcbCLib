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
* @file protocol_message_network.c
* @brief Implementation of core message network protocol
* @author Jim Kueneman
* @date 17 Jan 2026
*/

#include "protocol_message_network.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"

// Interface reserved for future callbacks and configuration
// Currently empty but initialized for consistency with other protocol modules
static interface_openlcb_protocol_message_network_t *_interface;

    /**
    * @brief Initializes the Message Network protocol layer
    *
    * @details Algorithm:
    * -# Store pointer to callback interface in static variable
    * -# Interface remains valid for application lifetime
    *
    * Use cases:
    * - Called during application startup
    * - Required before processing any OpenLCB messages
    *
    * @verbatim
    * @param interface_openlcb_protocol_message_network Pointer to callback interface structure
    * @endverbatim
    *
    * @warning Pointer must remain valid for lifetime of application
    * @warning NOT thread-safe - call during single-threaded initialization only
    *
    * @attention Call before enabling CAN message reception
    * @attention Currently no callbacks are registered, but interface is maintained for consistency
    *
    * @see interface_openlcb_protocol_message_network_t - Callback interface structure
    */
void ProtocolMessageNetwork_initialize(const interface_openlcb_protocol_message_network_t *interface_openlcb_protocol_message_network) {

    _interface = (interface_openlcb_protocol_message_network_t *) interface_openlcb_protocol_message_network;

}

    /**
    * @brief Loads a Duplicate Node ID Detected event message
    *
    * @details Algorithm:
    * -# Check if duplicate ID already detected (prevents multiple reports)
    * -# If already detected, return early
    * -# Construct PC Event Report message
    * -# Set source to this node's alias and ID
    * -# Set destination to the duplicate node's alias and ID
    * -# Copy DUPLICATE_NODE_DETECTED event ID to payload
    * -# Set duplicate_id_detected flag to prevent future reports
    * -# Mark outgoing message as valid
    *
    * Use cases:
    * - Reporting duplicate Node ID detection to network
    * - Alerting network of configuration problem
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Only reports duplicate once per boot cycle
    * @note Uses EVENT_ID_DUPLICATE_NODE_DETECTED well-known event
    * @note Payload count is set automatically by OpenLcbUtilities_copy_event_id_to_openlcb_payload
    */
static void _load_duplicate_node_id(openlcb_statemachine_info_t *statemachine_info) {

    if (statemachine_info->openlcb_node->state.duplicate_id_detected) { // Already handled this once

        return;

    }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_PC_EVENT_REPORT);

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            EVENT_ID_DUPLICATE_NODE_DETECTED);

    statemachine_info->openlcb_node->state.duplicate_id_detected = true;
    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Loads a Verified Node ID response message
    *
    * @details Algorithm:
    * -# Determine appropriate MTI based on node type:
    *    - If node is SIMPLE: use MTI_VERIFIED_NODE_ID_SIMPLE
    *    - Otherwise: use MTI_VERIFIED_NODE_ID
    * -# Construct Verified Node ID message
    * -# Set source to this node's alias and ID
    * -# Set destination to requesting node's alias and ID
    * -# Copy this node's 48-bit Node ID to payload
    * -# Mark outgoing message as valid
    *
    * Use cases:
    * - Responding to Verify Node ID requests
    * - Announcing node presence on network
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note MTI varies based on whether node implements Simple or Full protocol
    * @note Payload count is set automatically by OpenLcbUtilities_copy_node_id_to_openlcb_payload
    */
static void _load_verified_node_id(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t mti = MTI_VERIFIED_NODE_ID;

    if (statemachine_info->openlcb_node->parameters->protocol_support & PSI_SIMPLE) {

        mti = MTI_VERIFIED_NODE_ID_SIMPLE;

    }

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            mti);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->id,
            0);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Handles Initialization Complete message
    *
    * @details Algorithm:
    * -# Mark outgoing message as invalid (no automatic response required)
    *
    * Use cases:
    * - Detecting new nodes joining the network
    * - Updating node discovery tables
    * - Triggering configuration queries to new nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Full initialization complete includes complete node capabilities
    * @note Application can monitor for this message to detect new nodes
    *
    * @see ProtocolMessageNetwork_handle_initialization_complete_simple - Simple node version
    */
void ProtocolMessageNetwork_handle_initialization_complete(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Initialization Complete Simple message
    *
    * @details Algorithm:
    * -# Mark outgoing message as invalid (no automatic response required)
    *
    * Use cases:
    * - Detecting simple nodes joining the network
    * - Distinguishing simple from full nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Simple nodes implement a subset of the full protocol
    * @note Application can monitor for this message to detect simple nodes
    *
    * @see ProtocolMessageNetwork_handle_initialization_complete - Full node version
    */
void ProtocolMessageNetwork_handle_initialization_complete_simple(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Protocol Support Inquiry message
    *
    * @details Algorithm:
    * -# Read protocol support flags from node parameters
    * -# If firmware upgrade is active:
    *    - Clear FIRMWARE_UPGRADE bit
    *    - Set FIRMWARE_UPGRADE_ACTIVE bit
    * -# Construct Protocol Support Reply message
    * -# Set source to this node's alias and ID
    * -# Set destination to requesting node's alias and ID
    * -# Copy 6 bytes of protocol support flags to payload:
    *    - Byte 0: Upper 8 bits of support flags
    *    - Byte 1: Middle 8 bits of support flags
    *    - Byte 2: Lower 8 bits of support flags
    *    - Bytes 3-5: Reserved (0x00)
    * -# Mark outgoing message as valid
    *
    * Use cases:
    * - Configuration tools discovering node capabilities
    * - Protocol negotiation between nodes
    * - Feature detection
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Constructs and sends Protocol Support Reply message
    * @note Support flags are read from node's parameters structure
    * @note Handles firmware upgrade state specially
    * @note Payload count is set automatically by OpenLcbUtilities_copy_byte_to_openlcb_payload
    *
    * @see ProtocolMessageNetwork_handle_protocol_support_reply - Handles responses
    */
void ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_statemachine_info_t *statemachine_info) {

    uint64_t support_flags = statemachine_info->openlcb_node->parameters->protocol_support;

    if (statemachine_info->openlcb_node->state.firmware_upgrade_active) {

        support_flags = (support_flags & ~((uint64_t) PSI_FIRMWARE_UPGRADE)) | (uint64_t) PSI_FIRMWARE_UPGRADE_ACTIVE;

    }

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_PROTOCOL_SUPPORT_REPLY);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) (support_flags >> 16) & 0xFF, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) (support_flags >> 8) & 0xFF, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) (support_flags >> 0) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x00, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x00, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x00, 5);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Handles Protocol Support Reply message
    *
    * @details Algorithm:
    * -# Mark outgoing message as invalid (no automatic response required)
    *
    * Use cases:
    * - Receiving protocol capabilities from remote nodes
    * - Building node capability tables
    * - Adapting communication based on remote capabilities
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Application can extract capability flags from incoming message payload
    *
    * @see ProtocolMessageNetwork_handle_protocol_support_inquiry - Triggers this response
    */
void ProtocolMessageNetwork_handle_protocol_support_reply(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles global Verify Node ID message
    *
    * @details Algorithm:
    * -# Check if message payload contains a Node ID (payload_count > 0)
    * -# If payload contains Node ID:
    *    - Extract the Node ID from payload
    *    - Compare with this node's ID
    *    - If match: Call _load_verified_node_id() to prepare response
    *    - If no match: Mark outgoing message as invalid (no response)
    *    - Return to caller
    * -# If payload is empty (global request):
    *    - Call _load_verified_node_id() to prepare response
    *
    * Use cases:
    * - Network-wide node discovery
    * - Verifying presence of specific node
    * - Detecting duplicate Node IDs
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note If payload contains Node ID, only responds if it matches this node
    * @note If payload is empty, responds unconditionally (global request)
    * @note Response is either Verified Node ID or Verified Node ID Simple
    *
    * @see ProtocolMessageNetwork_handle_verify_node_id_addressed - Addressed version
    * @see ProtocolMessageNetwork_handle_verified_node_id - Response message handler
    */
void ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_statemachine_info_t *statemachine_info) {

    if (statemachine_info->incoming_msg_info.msg_ptr->payload_count > 0) {

        if (OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0) == statemachine_info->openlcb_node->id) {

            _load_verified_node_id(statemachine_info);

            return;

        }

        statemachine_info->outgoing_msg_info.valid = false; // nothing to do

        return;

    }

    _load_verified_node_id(statemachine_info);

}

    /**
    * @brief Handles addressed Verify Node ID message
    *
    * @details Algorithm:
    * -# Call _load_verified_node_id() to prepare Verified Node ID response
    *
    * Use cases:
    * - Targeted node verification
    * - Confirming node is still online
    * - Directed discovery
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always responds (message is addressed to this node)
    * @note Response is either Verified Node ID or Verified Node ID Simple
    *
    * @see ProtocolMessageNetwork_handle_verify_node_id_global - Broadcast version
    * @see ProtocolMessageNetwork_handle_verified_node_id - Response message handler
    */
void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_statemachine_info_t *statemachine_info) {

    _load_verified_node_id(statemachine_info);

}

    /**
    * @brief Handles Verified Node ID message
    *
    * @details Algorithm:
    * -# Extract Node ID from incoming message payload
    * -# Compare extracted ID with this node's ID
    * -# If IDs match (duplicate detected):
    *    - Call _load_duplicate_node_id() to prepare error event
    *    - Return to caller
    * -# If IDs don't match:
    *    - Mark outgoing message as invalid (no response needed)
    *
    * Use cases:
    * - Detecting duplicate Node IDs on the network
    * - Learning about other nodes on the network
    * - Node discovery and tracking
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note If Node ID matches this node, sends Duplicate Node Detected event
    * @note Duplicate detection only triggers once per boot
    * @note If Node IDs don't match, no response is generated
    *
    * @see ProtocolMessageNetwork_handle_verify_node_id_global - Triggers this message
    * @see ProtocolMessageNetwork_handle_verify_node_id_addressed - Triggers this message
    */
void ProtocolMessageNetwork_handle_verified_node_id(openlcb_statemachine_info_t *statemachine_info) {

    if (OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0) == statemachine_info->openlcb_node->id) {

        _load_duplicate_node_id(statemachine_info);

        return;

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Optional Interaction Rejected message
    *
    * @details Algorithm:
    * -# Mark outgoing message as invalid (no automatic response required)
    *
    * Use cases:
    * - Handling feature negotiation failures
    * - Detecting unsupported protocols on remote nodes
    * - Graceful degradation when features unavailable
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Application should check for this after sending optional protocol requests
    *
    * @see ProtocolMessageNetwork_handle_protocol_support_inquiry - Proactive capability checking
    */
void ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Terminate Due To Error message
    *
    * @details Algorithm:
    * -# Mark outgoing message as invalid (no automatic response required)
    *
    * Use cases:
    * - Detecting serious errors in remote nodes
    * - Cleaning up resources associated with failed node
    * - Error logging and diagnostics
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Error details may be in message payload
    * @note This indicates a serious problem in the remote node
    */
void ProtocolMessageNetwork_handle_terminate_due_to_error(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}
