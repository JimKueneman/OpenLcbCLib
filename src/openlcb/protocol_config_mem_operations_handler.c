/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file protocol_config_mem_operations_handler.c
 *
 *
 * @author Jim Kueneman
 * @date 23 Oct 2025
 */

#include "protocol_config_mem_operations_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf
#include <string.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"


static interface_protocol_config_mem_operations_handler_t* _interface;

void ProtocolConfigMemOperationsHandler_initialize(const interface_protocol_config_mem_operations_handler_t *interface_protocol_config_mem_operations_handler) {

    _interface = (interface_protocol_config_mem_operations_handler_t*) interface_protocol_config_mem_operations_handler;

}

static const user_address_space_info_t* _decode_to_space_definition(openlcb_statemachine_info_t *statemachine_info, uint8_t space_offset) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[space_offset]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            return (&statemachine_info->openlcb_node->parameters->address_space_configuration_definition);

        case ADDRESS_SPACE_ALL:

            return &statemachine_info->openlcb_node->parameters->address_space_all;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            return&statemachine_info->openlcb_node->parameters->address_space_config_memory;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            return&statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            return&statemachine_info->openlcb_node->parameters->address_space_acdi_user;

        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            return &statemachine_info->openlcb_node->parameters->address_space_traction_function_definition_info;

        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            return &statemachine_info->openlcb_node->parameters->address_space_traction_function_config_memory;

        case ADDRESS_SPACE_FIRMWARE:

            return &statemachine_info->openlcb_node->parameters->address_space_firmware;

        default:

            return NULL;

    }

}

static void _load_config_mem_reply_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_read_request_info) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_DATAGRAM);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            DATAGRAM_MEMORY_CONFIGURATION,
            0);

    statemachine_info->outgoing_msg_info.valid = false; // Assume there is not a message to send by default

}

static uint8_t _available_write_flags(openlcb_statemachine_info_t *statemachine_info) {

    uint8_t write_lengths = 0x80 | 0x40 | 0x020 | 0x02;

    if (statemachine_info->openlcb_node->parameters->configuration_options.stream_read_write_supported) {

        write_lengths = write_lengths | 0x01;

    }

    return write_lengths;

}

static uint16_t _available_commands_flags(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t result = 0x0000;

    if (statemachine_info->openlcb_node->parameters->configuration_options.write_under_mask_supported) {

        result = result | 0x8000;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.unaligned_reads_supported) {

        result = result | 0x4000;

    }
    if (statemachine_info->openlcb_node->parameters->configuration_options.unaligned_writes_supported) {

        result = result | 0x2000;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.read_from_manufacturer_space_0xfc_supported) {

        result = result | 0x0800;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.read_from_user_space_0xfb_supported) {

        result = result | 0x0400;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.write_to_user_space_0xfb_supported) {

        result = result | 0x0200;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.stream_read_write_supported) {

        result = result | 0x0001;

    }

    return result;

}

static uint8_t _available_address_space_info_flags(config_mem_operations_request_info_t *config_mem_operations_request_info) {

    uint8_t flags = 0x0;

    if (config_mem_operations_request_info->space_info->read_only) {

        flags = flags | 0x01;

    }

    if (config_mem_operations_request_info->space_info->low_address_valid) {

        flags = flags | 0x02;

    }

    return flags;
}

static void _load_datagram_ok_message(openlcb_statemachine_info_t *statemachine_info) {

    _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
    statemachine_info->incoming_msg_info.enumerate = true; // call this again for the data

}

static void _load_datagram_fail_message(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code) {

    _interface->load_datagram_received_rejected_message(statemachine_info, error_code);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // done
    statemachine_info->incoming_msg_info.enumerate = false; // done

}

static void _handle_operations_request(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
        
        if (config_mem_operations_request_info->operations_func) {

            _load_datagram_ok_message(statemachine_info);
            
        } else {
            
            _load_datagram_fail_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);
        }

        return;
    }

    // Complete Command Request
    if (config_mem_operations_request_info->operations_func) {

        config_mem_operations_request_info->operations_func(statemachine_info, config_mem_operations_request_info);

    }
    
    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // reset
    statemachine_info->incoming_msg_info.enumerate = false; // done
}

void ProtocolConfigMemOperationsHandler_request_options_cmd(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            DATAGRAM_MEMORY_OPTIONS_REPLY,
            1);

    OpenLcbUtilities_copy_word_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            _available_commands_flags(statemachine_info),
            2);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            _available_write_flags(statemachine_info),
            4);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->parameters->configuration_options.high_address_space,
            5);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->parameters->configuration_options.low_address_space,
            6);

    if (strlen(statemachine_info->openlcb_node->parameters->configuration_options.description) > 0x00) {

        OpenLcbUtilities_copy_string_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
                statemachine_info->openlcb_node->parameters->configuration_options.description,
                statemachine_info->outgoing_msg_info.msg_ptr->payload_count);

    }

    statemachine_info->outgoing_msg_info.valid = true;

}

void ProtocolConfigMemOperationsHandler_request_get_address_space_info(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    uint8_t description_offset = 8;

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    if (config_mem_operations_request_info->space_info) {

        if (config_mem_operations_request_info->space_info->present) {

            OpenLcbUtilities_copy_byte_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    DATAGRAM_MEMORY_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT,
                    1);

            OpenLcbUtilities_copy_byte_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    *statemachine_info->incoming_msg_info.msg_ptr->payload[2],
                    2);

            OpenLcbUtilities_copy_dword_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_operations_request_info->space_info->highest_address,
                    3);

            OpenLcbUtilities_copy_byte_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    _available_address_space_info_flags(config_mem_operations_request_info),
                    7);

            if (config_mem_operations_request_info->space_info->low_address_valid) {

                OpenLcbUtilities_copy_dword_to_openlcb_payload(
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_operations_request_info->space_info->low_address,
                        8);

                description_offset = 12;

            }

            if (strlen(config_mem_operations_request_info->space_info->description) > 0) {

                OpenLcbUtilities_copy_string_to_openlcb_payload(
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_operations_request_info->space_info->description,
                        description_offset);

            }

            statemachine_info->outgoing_msg_info.valid = true;

            return;

        }

    }

    // default reply

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            DATAGRAM_MEMORY_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT,
            1);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            *statemachine_info->incoming_msg_info.msg_ptr->payload[1],
            2);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8; // OpenLcbChecker needs 8 

    statemachine_info->outgoing_msg_info.valid = true;

}

void ProtocolConfigMemOperationsHandler_request_reserve_lock(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    node_id_t new_node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(
            statemachine_info->incoming_msg_info.msg_ptr,
            2);

    if (statemachine_info->openlcb_node->owner_node == 0) {

        statemachine_info->openlcb_node->owner_node = new_node_id;

    } else {

        if (new_node_id == 0)

            statemachine_info->openlcb_node->owner_node = 0;

    }

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            DATAGRAM_MEMORY_RESERVE_LOCK_REPLY,
            1);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->owner_node,
            2);

    statemachine_info->outgoing_msg_info.valid = true;

}

void ProtocolConfigMemOperationsHandler_options_cmd(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_options_cmd;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);
}

void ProtocolConfigMemOperationsHandler_options_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_options_cmd_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_get_address_space_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info_reply_not_present;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info_reply_present;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_reserve_lock(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reserve_lock;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_reserve_lock_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reserve_lock_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);


}

void ProtocolConfigMemOperationsHandler_get_unique_id(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_unique_id;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_get_unique_id_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_unique_id_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_unfreeze(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_unfreeze;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_freeze(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_freeze;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_update_complete(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_update_complete;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_reset_reboot(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reset_reboot;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

void ProtocolConfigMemOperationsHandler_factory_reset(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_factory_reset;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}



