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
 * \file protocol_message_network.h
 *
 * Implementation of the Openlcb core Messages that must be handled by all nodes.
 * Handlers are call from the openlcb_main_statemachine.c processing when a 
 * datagram message is being processed from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __PROTOCOL_MESSAGE_NETWORK__
#define	__PROTOCOL_MESSAGE_NETWORK__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {
    bool (*transmit_openlcb_message)(openlcb_msg_t* openlcb_msg);

} interface_openlcb_protocol_message_network_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void ProtocolMessageNetwork_initialize(const interface_openlcb_protocol_message_network_t *interface_openlcb_protocol_message_network);

    extern void ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolMessageNetwork_handle_verified_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolMessageNetwork_send_interaction_rejected(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolMessageNetwork_buffer_optional_interaction_message_for_resend(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

    extern void ProtocolMessageNetwork_clear_resend_optional_message(openlcb_node_t* openlcb_node);


#ifdef	__cplusplus
}
#endif /* __cplusplus */


#endif	/* __PROTOCOL_MESSAGE_NETWORK__ */

