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
 * \file protocol_snip.c
 *
 * Implementation of the Openlcb Simple Node Identification Protocol (SNIP).
 * Handlers are call from the openlcb_main_statemachine.c processing when a 
 * datagram message is being processed from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "protocol_snip.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf
#include <string.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_list.h"


static interface_openlcb_protocol_snip_t *_interface;

void ProtocolSnip_initialize(const interface_openlcb_protocol_snip_t *interface_openlcb_protocol_snip) {

    _interface = (interface_openlcb_protocol_snip_t*) interface_openlcb_protocol_snip;

}

static void _process_snip_string(openlcb_msg_t* worker_msg, uint16_t *payload_offset, const char *str, uint16_t max_str_len, uint16_t bytes_to_copy) {

    bool result_is_null_terminated = false;
    uint16_t string_length = strlen(str);

    if (string_length > max_str_len - 1) {

        string_length = max_str_len - 1;
        result_is_null_terminated = true;

    } else {

        result_is_null_terminated = string_length <= bytes_to_copy;

    }

    if (result_is_null_terminated) {

        memcpy(&worker_msg->payload[*payload_offset], str, string_length);  
        *payload_offset = *payload_offset + string_length;
        *worker_msg->payload[*payload_offset] = 0x00;
        (*payload_offset)++;
        worker_msg->payload_count = worker_msg->payload_count + string_length + 1;

    } else {

        memcpy(&worker_msg->payload[*payload_offset], str, bytes_to_copy);
        *payload_offset = *payload_offset + bytes_to_copy;
        worker_msg->payload_count = worker_msg->payload_count + bytes_to_copy;

    }

}

static uint8_t _process_snip_version(openlcb_msg_t* worker_msg, uint16_t *payload_offset, const uint8_t version) {


    *worker_msg->payload[*payload_offset] = version;
    worker_msg->payload_count++;
    (*payload_offset)++;

    return *payload_offset;


}

uint16_t ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes) {

    if (requested_bytes > 0) {

        return _process_snip_version(worker_msg, &payload_offset, openlcb_node->parameters->snip.mfg_version);

    }

    return 0;

}

uint16_t ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes) {

    _process_snip_string(worker_msg, &payload_offset, openlcb_node->parameters->snip.name, LEN_SNIP_NAME_BUFFER, requested_bytes);

    return payload_offset;

}

uint16_t ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes) {

    _process_snip_string(worker_msg, &payload_offset, openlcb_node->parameters->snip.model, LEN_SNIP_MODEL_BUFFER, requested_bytes);

    return payload_offset;

}

uint16_t ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes) {

    _process_snip_string(worker_msg, &payload_offset, openlcb_node->parameters->snip.hardware_version, LEN_SNIP_HARDWARE_VERSION_BUFFER, requested_bytes);

    return payload_offset;

}

uint16_t ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes) {

    _process_snip_string(worker_msg, &payload_offset, openlcb_node->parameters->snip.software_version, LEN_SNIP_SOFTWARE_VERSION_BUFFER, requested_bytes);

    return payload_offset;

}

uint16_t ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes) {

    if (requested_bytes > 0) {

        return _process_snip_version(worker_msg, &payload_offset, openlcb_node->parameters->snip.user_version);

    }

    return 0;

}

uint16_t ProtocolSnip_load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes) {


    configuration_memory_buffer_t configuration_memory_buffer;
    uint32_t data_address = 0; // User Name is always the first 63 Bytes in the Configuration Space

    if (openlcb_node->parameters->address_space_config_memory.low_address_valid) {

        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    }

    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node); // offset for multiple nodes

    _interface->configuration_memory_read(data_address, requested_bytes, &configuration_memory_buffer);

    _process_snip_string(worker_msg, &payload_offset, (char*)(&configuration_memory_buffer[0]), LEN_SNIP_USER_NAME_BUFFER, requested_bytes);

    return payload_offset;

}

uint16_t ProtocolSnip_load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes) {

    configuration_memory_buffer_t configuration_memory_buffer;
    uint32_t data_address = LEN_SNIP_USER_NAME_BUFFER; // User Name is always the first 63 Bytes in the Configuration Space and Description next 64 bytes

    if (openlcb_node->parameters->address_space_config_memory.low_address_valid) {

        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    }

    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node); // offset for multiple nodes

    _interface->configuration_memory_read(data_address, requested_bytes, &configuration_memory_buffer); // grab string from config memory
    
    _process_snip_string(worker_msg, &payload_offset, (char*)(&configuration_memory_buffer[0]), LEN_SNIP_USER_DESCRIPTION_BUFFER, requested_bytes);
    
    return payload_offset;

}

void ProtocolSnip_handle_simple_node_info_request(openlcb_statemachine_info_t *statemachine_info) {

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg,
                                          statemachine_info->openlcb_node->alias,
                                          statemachine_info->openlcb_node->id,
                                          statemachine_info->incoming_msg->source_alias,
                                          statemachine_info->incoming_msg->source_id,
                                          MTI_SIMPLE_NODE_INFO_REPLY,
                                          0);

    uint16_t payload_offset = 0;
    payload_offset = ProtocolSnip_load_manufacturer_version_id(statemachine_info->openlcb_node, statemachine_info->outgoing_msg, payload_offset, 1);
    payload_offset = ProtocolSnip_load_name(statemachine_info->openlcb_node, statemachine_info->outgoing_msg, payload_offset, LEN_SNIP_NAME_BUFFER - 1);
    payload_offset = ProtocolSnip_load_model(statemachine_info->openlcb_node, statemachine_info->outgoing_msg, payload_offset, LEN_SNIP_MODEL_BUFFER - 1);
    payload_offset = ProtocolSnip_load_hardware_version(statemachine_info->openlcb_node, statemachine_info->outgoing_msg, payload_offset, LEN_SNIP_HARDWARE_VERSION_BUFFER - 1);
    payload_offset = ProtocolSnip_load_software_version(statemachine_info->openlcb_node, statemachine_info->outgoing_msg, payload_offset, LEN_SNIP_SOFTWARE_VERSION_BUFFER - 1);
    payload_offset = ProtocolSnip_load_user_version_id(statemachine_info->openlcb_node, statemachine_info->outgoing_msg, payload_offset, 1);
    payload_offset = ProtocolSnip_load_user_name(statemachine_info->openlcb_node, statemachine_info->outgoing_msg, payload_offset, LEN_SNIP_USER_NAME_BUFFER - 1);
    payload_offset = ProtocolSnip_load_user_description(statemachine_info->openlcb_node, statemachine_info->outgoing_msg, payload_offset, LEN_SNIP_USER_DESCRIPTION_BUFFER - 1);
    statemachine_info->outgoing_msg->payload_count = payload_offset;
 
    statemachine_info->outgoing_msg_valid = true;

}

void ProtocolSnip_handle_simple_node_info_reply(openlcb_statemachine_info_t *statemachine_info) {
    
    statemachine_info->outgoing_msg_valid = false;
    
}


bool ProtocolSnip_validate_snip_reply(openlcb_msg_t* snip_reply_msg) {

    if (snip_reply_msg->payload_count > LEN_MESSAGE_BYTES_SNIP) { // serious issue if this occurs

        return false;

    }

    if (snip_reply_msg->mti != MTI_SIMPLE_NODE_INFO_REPLY) {

        return false;
        
    }

    uint16_t null_count = OpenLcbUtilities_count_nulls_in_openlcb_payload(snip_reply_msg);

    if (null_count != 6) {

        return false;
        
    }

    return true;
}

