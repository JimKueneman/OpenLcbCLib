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

void OpenLcbLoginMessageHandler_load_initialization_complete(openlcb_statemachine_info_t *openlcb_statemachine_info) {

    OpenLcbUtilities_load_openlcb_message(openlcb_statemachine_info->outgoing_msg_info.openlcb_msg, openlcb_statemachine_info->openlcb_node->alias, openlcb_statemachine_info->openlcb_node->id, 0, 0, MTI_INITIALIZATION_COMPLETE, 6);

    if (openlcb_statemachine_info->openlcb_node->parameters->protocol_support & PSI_SIMPLE) {

        openlcb_statemachine_info->outgoing_msg_info.openlcb_msg->mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    }

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_statemachine_info->outgoing_msg_info.openlcb_msg, openlcb_statemachine_info->openlcb_node->id, 0);

    openlcb_statemachine_info->openlcb_node->state.initalized = true;
    openlcb_statemachine_info->openlcb_node->producers.enumerator.running = true;
    openlcb_statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
    openlcb_statemachine_info->outgoing_msg_info.valid = true;

    openlcb_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

}

void OpenLcbLoginMessageHandler_load_producer_event(openlcb_statemachine_info_t *openlcb_statemachine_info) {

    if (openlcb_statemachine_info->openlcb_node->producers.count == 0) {

        openlcb_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

        return;

    }

    uint16_t event_mti = _interface->extract_producer_event_state_mti(openlcb_statemachine_info->openlcb_node, openlcb_statemachine_info->openlcb_node->producers.enumerator.enum_index);

    OpenLcbUtilities_load_openlcb_message(openlcb_statemachine_info->outgoing_msg_info.openlcb_msg, openlcb_statemachine_info->openlcb_node->alias, openlcb_statemachine_info->openlcb_node->id, 0, 0, event_mti, 8);
    event_id_t event_id = openlcb_statemachine_info->openlcb_node->producers.list[openlcb_statemachine_info->openlcb_node->producers.enumerator.enum_index].event;
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_statemachine_info->outgoing_msg_info.openlcb_msg, event_id);

    openlcb_statemachine_info->openlcb_node->producers.enumerator.enum_index++;
    openlcb_statemachine_info->outgoing_msg_info.enumerating = true;

    if (openlcb_statemachine_info->openlcb_node->producers.enumerator.enum_index >= openlcb_statemachine_info->openlcb_node->producers.count) {

        openlcb_statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
        openlcb_statemachine_info->openlcb_node->producers.enumerator.running = false;
        openlcb_statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;
        openlcb_statemachine_info->openlcb_node->consumers.enumerator.running = true;
        openlcb_statemachine_info->outgoing_msg_info.enumerating = false;

        openlcb_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

        return;

    }

}

void OpenLcbLoginMessageHandler_load_consumer_event(openlcb_statemachine_info_t *openlcb_statemachine_info) {

    if (openlcb_statemachine_info->openlcb_node->consumers.count == 0) {

        openlcb_statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;

        return;

    }

    uint16_t event_mti = _interface->extract_consumer_event_state_mti(openlcb_statemachine_info->openlcb_node, openlcb_statemachine_info->openlcb_node->consumers.enumerator.enum_index);

    OpenLcbUtilities_load_openlcb_message(openlcb_statemachine_info->outgoing_msg_info.openlcb_msg, openlcb_statemachine_info->openlcb_node->alias, openlcb_statemachine_info->openlcb_node->id, 0, 0, event_mti, 8);
    event_id_t event_id = openlcb_statemachine_info->openlcb_node->consumers.list[openlcb_statemachine_info->openlcb_node->consumers.enumerator.enum_index].event;
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_statemachine_info->outgoing_msg_info.openlcb_msg, event_id);

    openlcb_statemachine_info->openlcb_node->consumers.enumerator.enum_index++;
    openlcb_statemachine_info->outgoing_msg_info.enumerating = true;

    if (openlcb_statemachine_info->openlcb_node->consumers.enumerator.enum_index >= openlcb_statemachine_info->openlcb_node->consumers.count) {

        openlcb_statemachine_info->openlcb_node->consumers.enumerator.running = false;
        openlcb_statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;

        openlcb_statemachine_info->outgoing_msg_info.enumerating = false;

        openlcb_statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;

        return;

    }

}


