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
 * \file protocol_message_network.c
 *
 * Implementation of the Openlcb core Messages that must be handled by all nodes.
 * Handlers are call from the openlcb_main_statemachine.c processing when a 
 * datagram message is being processed from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "protocol_message_network.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"

static interface_openlcb_protocol_message_network_t *_interface;

void ProtocolMessageNetwork_initialize(const interface_openlcb_protocol_message_network_t *interface_openlcb_protocol_message_network) {

    _interface = (interface_openlcb_protocol_message_network_t*) interface_openlcb_protocol_message_network;

}

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

 //   statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;
    
    statemachine_info->openlcb_node->state.duplicate_id_detected = true;
    statemachine_info->outgoing_msg_info.valid = true;

}

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

 //   statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 6;
    
    statemachine_info->outgoing_msg_info.valid = true;

}

void ProtocolMessageNetwork_handle_initialization_complete(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolMessageNetwork_handle_initialization_complete_simple(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

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
    
 //   statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 6;
    
    statemachine_info->outgoing_msg_info.valid = true;

}

void ProtocolMessageNetwork_handle_protocol_support_reply(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

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

void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_statemachine_info_t *statemachine_info) {

    _load_verified_node_id(statemachine_info);

}

void ProtocolMessageNetwork_handle_verified_node_id(openlcb_statemachine_info_t *statemachine_info) {

    if (OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0) == statemachine_info->openlcb_node->id) {

        _load_duplicate_node_id(statemachine_info);

        return;

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolMessageNetwork_handle_terminate_due_to_error(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}


