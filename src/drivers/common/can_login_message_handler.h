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
 * \file can_login_message_handler.h
 *
 * When a node is logging into the network on a CAN bus it must follow a specific
 * flow to allocate a unique alias ID and broadcast its events.  This is the handler 
 * that is called from the CAN main statemachine to accomplish that when a new node
 * is created.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_LOGIN_MESSAGE_HANDLER__
#define	__CAN_LOGIN_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    typedef struct {
        bool (*try_transmit_openlcb_message)(openlcb_msg_t* openlcb_msg);
        bool (*try_transmit_can_message)(can_msg_t* can_msg);
        uint16_t (*extract_producer_event_state_mti)(openlcb_node_t* openlcb_node, uint16_t event_index);
        uint16_t (*extract_consumer_event_state_mti)(openlcb_node_t* openlcb_node, uint16_t event_index);     
        callback_alias_change_t (*get_alias_change)(void);

    } interface_can_login_message_handler_t;
    
    
    extern void CanLoginMessageHandler_initialize(const interface_can_login_message_handler_t *interface);

    extern void CanLoginMessageHandler_init(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_generate_seed(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_generate_alias(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_cid07(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_cid06(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_cid05(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_cid04(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_wait_200ms(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_rid(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_amd(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_initialization_complete(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_producer_events(openlcb_node_t* next_node);

    extern void CanLoginMessageHandler_transmit_consumer_events(openlcb_node_t* next_node);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_LOGIN_MESSAGE_HANDLER__ */

