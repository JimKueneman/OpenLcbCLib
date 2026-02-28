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
* @brief Core message network protocol — Verify Node ID, Protocol Support
*        Inquiry/Reply, Initialization Complete, Optional Interaction Rejected,
*        Terminate Due To Error.
*
* @author Jim Kueneman
* @date 28 Feb 2026
*
* @see MessageNetworkS.pdf
*/

#ifndef __OPENLCB_PROTOCOL_MESSAGE_NETWORK__
#define    __OPENLCB_PROTOCOL_MESSAGE_NETWORK__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /** @brief Callback interface (reserved — currently empty, kept for API symmetry). */
typedef struct {


} interface_openlcb_protocol_message_network_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup.
         *
         * @param interface_openlcb_protocol_message_network  Pointer to @ref interface_openlcb_protocol_message_network_t (must remain valid for application lifetime).
         */
    extern void ProtocolMessageNetwork_initialize(const interface_openlcb_protocol_message_network_t *interface_openlcb_protocol_message_network);

        /**
         * @brief Handle Initialization Complete (full node).  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_initialization_complete(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle Initialization Complete Simple.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_initialization_complete_simple(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle Protocol Support Inquiry — replies with this node’s PSI flags.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle Protocol Support Reply.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_protocol_support_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle global Verify Node ID — replies if payload matches or is empty.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle addressed Verify Node ID — always replies.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle Verified Node ID — checks for duplicate Node ID.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_verified_node_id(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle Optional Interaction Rejected.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle Terminate Due To Error.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolMessageNetwork_handle_terminate_due_to_error(openlcb_statemachine_info_t *statemachine_info);

#ifdef    __cplusplus
}
#endif /* __cplusplus */


#endif    /* __OPENLCB_PROTOCOL_MESSAGE_NETWORK__ */
