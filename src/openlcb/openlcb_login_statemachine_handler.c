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
 * \file openlcb_login_message_handler.c
 *
 * When a node is logging into the network on a CAN bus it must follow a specific
 * flow to allocate a unique alias ID and broadcast its events.  This is the handler 
 * that is called from the CAN main statemachine to accomplish that when a new node
 * is created.
 *
 * @author Jim Kueneman
 * @date 11 Oct 2025
 */

#include "openlcb_login_statemachine_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_defines.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"


static interface_openlcb_login_message_handler_t* _interface;

void OpenLcbLoginMessageHandler_initialize(const interface_openlcb_login_message_handler_t *interface) {

    _interface = (interface_openlcb_login_message_handler_t*) interface;

}

void OpenLcbLoginMessageHandler_load_initialization_complete(openlcb_login_statemachine_info_t *statemachine_info) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr, 
            statemachine_info->openlcb_node->alias, 
            statemachine_info->openlcb_node->id, 
            0, 
            0, 
            MTI_INITIALIZATION_COMPLETE, 
            6);

    if (statemachine_info->openlcb_node->parameters->protocol_support & PSI_SIMPLE) {

        statemachine_info->outgoing_msg_info.msg_ptr->mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    }

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, statemachine_info->openlcb_node->id, 0);

    statemachine_info->openlcb_node->state.initalized = true;
    statemachine_info->openlcb_node->producers.enumerator.running = true;
    statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
    statemachine_info->outgoing_msg_info.valid = true;

    statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

}

void OpenLcbLoginMessageHandler_load_producer_event(openlcb_login_statemachine_info_t *statemachine_info) {

    if (statemachine_info->openlcb_node->producers.count == 0) {

        statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

        return;

    }

    uint16_t event_mti = _interface->extract_producer_event_state_mti(statemachine_info->openlcb_node, statemachine_info->openlcb_node->producers.enumerator.enum_index);

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr, 
            statemachine_info->openlcb_node->alias, 
            statemachine_info->openlcb_node->id, 
            0, 
            0, 
            event_mti, 
            8);
    event_id_t event_id = statemachine_info->openlcb_node->producers.list[statemachine_info->openlcb_node->producers.enumerator.enum_index].event;
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, event_id);
  
    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;

    statemachine_info->openlcb_node->producers.enumerator.enum_index++;
    statemachine_info->outgoing_msg_info.enumerate = true;
    statemachine_info->outgoing_msg_info.valid = true;

    if (statemachine_info->openlcb_node->producers.enumerator.enum_index >= statemachine_info->openlcb_node->producers.count) {

        statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
        statemachine_info->openlcb_node->producers.enumerator.running = false;
        statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;
        statemachine_info->openlcb_node->consumers.enumerator.running = true;
        statemachine_info->outgoing_msg_info.enumerate = false;

        statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

        return;

    }

}

void OpenLcbLoginMessageHandler_load_consumer_event(openlcb_login_statemachine_info_t *statemachine_info) {

    if (statemachine_info->openlcb_node->consumers.count == 0) {

        statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;

        return;

    }

    uint16_t event_mti = _interface->extract_consumer_event_state_mti(statemachine_info->openlcb_node, statemachine_info->openlcb_node->consumers.enumerator.enum_index);

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr, 
            statemachine_info->openlcb_node->alias, 
            statemachine_info->openlcb_node->id, 
            0, 
            0, 
            event_mti, 
            8);
    event_id_t event_id = statemachine_info->openlcb_node->consumers.list[statemachine_info->openlcb_node->consumers.enumerator.enum_index].event;
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, event_id);
    
    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;

    statemachine_info->openlcb_node->consumers.enumerator.enum_index++;
    statemachine_info->outgoing_msg_info.enumerate = true;
    statemachine_info->outgoing_msg_info.valid = true;

    if (statemachine_info->openlcb_node->consumers.enumerator.enum_index >= statemachine_info->openlcb_node->consumers.count) {

        statemachine_info->openlcb_node->consumers.enumerator.running = false;
        statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;

        statemachine_info->outgoing_msg_info.enumerate = false;

        statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;

        return;

    }

}


