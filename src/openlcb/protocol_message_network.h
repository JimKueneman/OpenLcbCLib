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
* @file protocol_message_network.h
* @brief Core message network protocol implementation required by all nodes
* @author Jim Kueneman
* @date 17 Jan 2026
*/

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_MESSAGE_NETWORK__
#define    __OPENLCB_PROTOCOL_MESSAGE_NETWORK__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
    * @brief Interface structure for Message Network protocol callbacks
    *
    * @details This structure is reserved for future callback functions related to
    * core message network operations. Currently empty but maintained for API
    * consistency with other protocol modules.
    *
    * @note Currently no callbacks are defined for the Message Network protocol
    * @note Structure maintained for future expansion and API consistency
    *
    * @see ProtocolMessageNetwork_initialize - Registers this interface
    */
typedef struct {


} interface_openlcb_protocol_message_network_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
        * @brief Initializes the Message Network protocol layer
        *
        * @details Registers the application's callback interface with the Message Network
        * protocol handler. Must be called once during system initialization before any
        * message network operations.
        *
        * Use cases:
        * - Called during application startup
        * - Required before processing any OpenLCB messages
        *
        * @param interface_openlcb_protocol_message_network Pointer to callback interface structure
        *
        * @warning interface_openlcb_protocol_message_network must remain valid for lifetime of application
        * @warning NOT thread-safe - call during single-threaded initialization only
        *
        * @attention Call before enabling CAN message reception
        * @attention Currently no callbacks are registered, but interface is maintained for consistency
        *
        * @see interface_openlcb_protocol_message_network_t - Callback interface structure
        */
    extern void ProtocolMessageNetwork_initialize(const interface_openlcb_protocol_message_network_t *interface_openlcb_protocol_message_network);

        /**
        * @brief Handles Initialization Complete message
        *
        * @details Processes notification that a remote node has completed its initialization
        * sequence and is now fully operational on the network. This is the full version sent
        * by standard nodes.
        *
        * Use cases:
        * - Detecting new nodes joining the network
        * - Updating node discovery tables
        * - Triggering configuration queries to new nodes
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Full initialization complete includes complete node capabilities
        *
        * @see ProtocolMessageNetwork_handle_initialization_complete_simple - Simple node version
        */
    extern void ProtocolMessageNetwork_handle_initialization_complete(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Initialization Complete Simple message
        *
        * @details Processes notification that a simple node has completed its initialization
        * sequence. Simple nodes have reduced capabilities compared to full nodes.
        *
        * Use cases:
        * - Detecting simple nodes joining the network
        * - Distinguishing simple from full nodes
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Simple nodes implement a subset of the full protocol
        *
        * @see ProtocolMessageNetwork_handle_initialization_complete - Full node version
        */
    extern void ProtocolMessageNetwork_handle_initialization_complete_simple(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Protocol Support Inquiry message
        *
        * @details Processes a request from a remote node asking which protocols this node
        * supports. Responds with a Protocol Support Reply containing the node's capability
        * flags.
        *
        * Use cases:
        * - Configuration tools discovering node capabilities
        * - Protocol negotiation between nodes
        * - Feature detection
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Constructs and sends Protocol Support Reply message
        * @note Support flags are read from node's parameters structure
        * @note Handles firmware upgrade state specially
        *
        * @see ProtocolMessageNetwork_handle_protocol_support_reply - Handles responses
        */
    extern void ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Protocol Support Reply message
        *
        * @details Processes a response from a remote node indicating which protocols it
        * supports. The reply contains capability flags that describe the node's features.
        *
        * Use cases:
        * - Receiving protocol capabilities from remote nodes
        * - Building node capability tables
        * - Adapting communication based on remote capabilities
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Application can extract capability flags from incoming message payload
        *
        * @see ProtocolMessageNetwork_handle_protocol_support_inquiry - Triggers this response
        */
    extern void ProtocolMessageNetwork_handle_protocol_support_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles global Verify Node ID message
        *
        * @details Processes a broadcast request for all nodes (or a specific node if payload
        * contains a Node ID) to respond with their Node ID. Responds with Verified Node ID
        * if the request matches this node or is a global request.
        *
        * Use cases:
        * - Network-wide node discovery
        * - Verifying presence of specific node
        * - Detecting duplicate Node IDs
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note If payload contains Node ID, only responds if it matches this node
        * @note If payload is empty, responds unconditionally (global request)
        * @note Response is either Verified Node ID or Verified Node ID Simple
        *
        * @see ProtocolMessageNetwork_handle_verify_node_id_addressed - Addressed version
        * @see ProtocolMessageNetwork_handle_verified_node_id - Response message handler
        */
    extern void ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles addressed Verify Node ID message
        *
        * @details Processes a request directed specifically to this node to verify its
        * Node ID. Always responds with Verified Node ID message.
        *
        * Use cases:
        * - Targeted node verification
        * - Confirming node is still online
        * - Directed discovery
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always responds (message is addressed to this node)
        * @note Response is either Verified Node ID or Verified Node ID Simple
        *
        * @see ProtocolMessageNetwork_handle_verify_node_id_global - Broadcast version
        * @see ProtocolMessageNetwork_handle_verified_node_id - Response message handler
        */
    extern void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Verified Node ID message
        *
        * @details Processes a Verified Node ID message from a remote node. Checks if the
        * reported Node ID matches this node's ID, which would indicate a duplicate Node ID
        * condition on the network.
        *
        * Use cases:
        * - Detecting duplicate Node IDs on the network
        * - Learning about other nodes on the network
        * - Node discovery and tracking
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note If Node ID matches this node, sends Duplicate Node Detected event
        * @note Duplicate detection only triggers once per boot
        * @note If Node IDs don't match, no response is generated
        *
        * @see ProtocolMessageNetwork_handle_verify_node_id_global - Triggers this message
        * @see ProtocolMessageNetwork_handle_verify_node_id_addressed - Triggers this message
        */
    extern void ProtocolMessageNetwork_handle_verified_node_id(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Optional Interaction Rejected message
        *
        * @details Processes notification that a remote node has rejected an optional
        * protocol interaction that this node attempted. Indicates the remote node does
        * not support the requested feature.
        *
        * Use cases:
        * - Handling feature negotiation failures
        * - Detecting unsupported protocols on remote nodes
        * - Graceful degradation when features unavailable
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Application should check for this after sending optional protocol requests
        *
        * @see ProtocolMessageNetwork_handle_protocol_support_inquiry - Proactive capability checking
        */
    extern void ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Terminate Due To Error message
        *
        * @details Processes notification that a remote node is terminating communication
        * due to an error condition. This is a fatal error indication from the remote node.
        *
        * Use cases:
        * - Detecting serious errors in remote nodes
        * - Cleaning up resources associated with failed node
        * - Error logging and diagnostics
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Error details may be in message payload
        * @note This indicates a serious problem in the remote node
        */
    extern void ProtocolMessageNetwork_handle_terminate_due_to_error(openlcb_statemachine_info_t *statemachine_info);

#ifdef    __cplusplus
}
#endif /* __cplusplus */


#endif    /* __OPENLCB_PROTOCOL_MESSAGE_NETWORK__ */
