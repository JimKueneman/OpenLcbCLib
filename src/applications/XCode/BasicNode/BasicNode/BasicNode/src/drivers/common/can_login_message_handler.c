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
 * \file can_login_message_handler.c
 *
 * When a node is logging into the network on a CAN bus it must follow a specific
 * flow to allocate a unique alias ID and broadcast its events.  This is the handler 
 * that is called from the CAN main statemachine to accomplish that when a new node
 * is created.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_login_message_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb//openlcb_buffer_store.h"


static interface_can_login_message_handler_t* _interface;

static uint64_t _generate_seed(uint64_t start_seed) {

    uint32_t lfsr1 = start_seed & 0xFFFFFF;
    uint32_t lfsr2 = (start_seed >> 24) & 0xFFFFFF;

    uint32_t temp1 = ((lfsr1 << 9) | ((lfsr2 >> 15) & 0x1FF)) & 0xFFFFFF;
    uint32_t temp2 = (lfsr2 << 9) & 0xFFFFFF;

    lfsr1 = lfsr1 + temp1 + 0x1B0CA3L;
    lfsr2 = lfsr2 + temp2 + 0x7A4BA9L;

    lfsr1 = (lfsr1 & 0xFFFFFF) + ((lfsr2 & 0xFF000000) >> 24);
    lfsr2 = lfsr2 & 0xFFFFFF;

    return ( (uint64_t) lfsr1 << 24) | lfsr2;

}

static uint16_t _generate_alias(uint64_t seed) {

    uint32_t lfsr2 = seed & 0xFFFFFF;
    uint32_t lfsr1 = (seed >> 24) & 0xFFFFFF;

    return ( lfsr1 ^ lfsr2 ^ (lfsr1 >> 12) ^ (lfsr2 >> 12)) & 0x0FFF;

}

void CanLoginMessageHandler_initialize(const interface_can_login_message_handler_t *interface) {

    _interface = (interface_can_login_message_handler_t*) interface;

}

void CanLoginMessageHandler_init(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->seed = can_statemachine_info->openlcb_node->id;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS; // Jump over Generate Seed that only is if we have an Alias conflict and have to jump back

}

void CanLoginMessageHandler_generate_seed(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->seed = _generate_seed(can_statemachine_info->openlcb_node->seed);
    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS;

}

void CanLoginMessageHandler_generate_alias(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->alias = _generate_alias(can_statemachine_info->openlcb_node->seed);

    if (_interface->on_alias_change) {

        _interface->on_alias_change(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    }

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;

}

void CanLoginMessageHandler_load_cid07(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->outgoing_can_msg->payload_count = 0;
    can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((can_statemachine_info->openlcb_node->id >> 24) & 0xFFF000) | can_statemachine_info->openlcb_node->alias); // AA0203040506
    can_statemachine_info->outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_06;

}

void CanLoginMessageHandler_load_cid06(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->outgoing_can_msg->payload_count = 0;
    can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((can_statemachine_info->openlcb_node->id >> 12) & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;

}

void CanLoginMessageHandler_load_cid05(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->outgoing_can_msg->payload_count = 0;
    can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((can_statemachine_info->openlcb_node->id & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_04;

}

void CanLoginMessageHandler_load_cid04(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->outgoing_can_msg->payload_count = 0;
    can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((can_statemachine_info->openlcb_node->id << 12) & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->openlcb_node->timerticks = 0;
    can_statemachine_info->outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;

}

void CanLoginMessageHandler_wait_200ms(can_statemachine_info_t *can_statemachine_info) {

    if (can_statemachine_info->openlcb_node->timerticks > 3) {

        can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_RESERVE_ID;

    }

}

void CanLoginMessageHandler_load_rid(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_statemachine_info->openlcb_node->alias;
    can_statemachine_info->outgoing_can_msg->payload_count = 0;
    can_statemachine_info->outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_ALIAS_MAP_DEFINITION;

}

void CanLoginMessageHandler_load_amd(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_statemachine_info->openlcb_node->alias;
    CanUtilities_copy_node_id_to_payload(can_statemachine_info->outgoing_can_msg, can_statemachine_info->openlcb_node->id, 0);
    can_statemachine_info->outgoing_can_msg_valid = true;
    can_statemachine_info->openlcb_node->state.permitted = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;
    
}

void CanLoginMessageHandler_load_initialization_complete(can_statemachine_info_t *can_statemachine_info) {

    OpenLcbUtilities_load_openlcb_message(can_statemachine_info->outgoing_openlcb_msg, can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id, 0, 0, MTI_INITIALIZATION_COMPLETE, 6);

    if (can_statemachine_info->openlcb_node->parameters->protocol_support & PSI_SIMPLE) {

        can_statemachine_info->outgoing_openlcb_msg->mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    }

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(can_statemachine_info->outgoing_openlcb_msg, can_statemachine_info->openlcb_node->id, 0);

    can_statemachine_info->openlcb_node->state.initalized = true;
    can_statemachine_info->openlcb_node->producers.enumerator.running = true;
    can_statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
    can_statemachine_info->outgoing_openlcb_msg_valid = true;
    
    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;


}

void CanLoginMessageHandler_load_producer_events(can_statemachine_info_t *can_statemachine_info) {

    if (can_statemachine_info->openlcb_node->producers.count == 0) {

        can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
        
        return;

    }

    uint16_t event_mti = _interface->extract_producer_event_state_mti(can_statemachine_info->openlcb_node, can_statemachine_info->openlcb_node->producers.enumerator.enum_index);

    OpenLcbUtilities_load_openlcb_message(can_statemachine_info->outgoing_openlcb_msg, can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id, 0, 0, event_mti, 8);
    event_id_t event_id = can_statemachine_info->openlcb_node->producers.list[can_statemachine_info->openlcb_node->producers.enumerator.enum_index].event;
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(can_statemachine_info->outgoing_openlcb_msg, event_id);

    can_statemachine_info->openlcb_node->producers.enumerator.enum_index++;
    can_statemachine_info->outgoing_openlcb_msg_valid = true;
    
    if (can_statemachine_info->openlcb_node->producers.enumerator.enum_index >= can_statemachine_info->openlcb_node->producers.count) {

        can_statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
        can_statemachine_info->openlcb_node->producers.enumerator.running = false;
        can_statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;
        can_statemachine_info->openlcb_node->consumers.enumerator.running = true;
        can_statemachine_info->enumerating = false;

        can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
        
        return;

    }
    
    can_statemachine_info->enumerating = true;

}

void CanLoginMessageHandler_load_consumer_events(can_statemachine_info_t *can_statemachine_info) {

    if (can_statemachine_info->openlcb_node->consumers.count == 0) {

        can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;
        
        return;

    }

    uint16_t event_mti = _interface->extract_consumer_event_state_mti(can_statemachine_info->openlcb_node, can_statemachine_info->openlcb_node->consumers.enumerator.enum_index);

    OpenLcbUtilities_load_openlcb_message(can_statemachine_info->outgoing_openlcb_msg, can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id, 0, 0, event_mti, 8);
    event_id_t event_id = can_statemachine_info->openlcb_node->consumers.list[can_statemachine_info->openlcb_node->consumers.enumerator.enum_index].event;
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(can_statemachine_info->outgoing_openlcb_msg, event_id);

    can_statemachine_info->openlcb_node->consumers.enumerator.enum_index++;
    can_statemachine_info->outgoing_openlcb_msg_valid = true;

    if (can_statemachine_info->openlcb_node->consumers.enumerator.enum_index >= can_statemachine_info->openlcb_node->consumers.count) {

        can_statemachine_info->openlcb_node->consumers.enumerator.running = false;
        can_statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;
        
        can_statemachine_info->enumerating = false;

        can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;
        
        return;

    }
    
    can_statemachine_info->enumerating = true;

}
