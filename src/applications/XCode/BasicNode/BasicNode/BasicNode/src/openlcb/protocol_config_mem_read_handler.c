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
 * \file protocol_config_mem_read_handler.c
 *
 *
 * @author Jim Kueneman
 * @date 23 Oct 2025
 */

#include "protocol_config_mem_read_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"

static interface_protocol_config_mem_read_handler_t *_interface;

void ProtocolConfigMemReadHandler_initialize(const interface_protocol_config_mem_read_handler_t *interface_openlcb_protocol_config_mem_read_handler) {

    _interface = (interface_protocol_config_mem_read_handler_t *) interface_openlcb_protocol_config_mem_read_handler;
}

static void _extract_read_command_parameters(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    config_mem_read_request_info->address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);

    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[1] == DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6) {

        config_mem_read_request_info->encoding = ADDRESS_SPACE_IN_BYTE_6;
        config_mem_read_request_info->bytes = *statemachine_info->incoming_msg_info.msg_ptr->payload[7];
        config_mem_read_request_info->data_start = 7;

    } else {

        config_mem_read_request_info->encoding = ADDRESS_SPACE_IN_BYTE_1;
        config_mem_read_request_info->bytes = *statemachine_info->incoming_msg_info.msg_ptr->payload[6];
        config_mem_read_request_info->data_start = 6;
    }
    
}

uint16_t _is_valid_read_parameters(config_mem_read_request_info_t *config_mem_read_request_info) {

    if (!config_mem_read_request_info->space_info->present) {

        return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN;
    }

    if (config_mem_read_request_info->address > config_mem_read_request_info->space_info->highest_address) {

        return ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS;
    }

    if (config_mem_read_request_info->bytes > 64) {

        return ERROR_PERMANENT_INVALID_ARGUMENTS;
    }

    if (config_mem_read_request_info->bytes == 0) {

        return ERROR_PERMANENT_INVALID_ARGUMENTS;
    }

    return S_OK;
}

static void _check_for_read_overrun(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    // Don't read past the end of the space

    if ((config_mem_read_request_info->address + config_mem_read_request_info->bytes) >= config_mem_read_request_info->space_info->highest_address) {

        config_mem_read_request_info->bytes = (uint8_t) (config_mem_read_request_info->space_info->highest_address - config_mem_read_request_info->address) + 1; // length +1 due to 0...end

    }
}

static void _load_config_mem_reply_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

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

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            *statemachine_info->incoming_msg_info.msg_ptr->payload[1] + CONFIG_REPLY_OK_OFFSET, // generate an OK reply by default for Read/Write/Stream
            1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            config_mem_read_request_info->address,
            2);

    if (config_mem_read_request_info->encoding == ADDRESS_SPACE_IN_BYTE_6) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
                *statemachine_info->incoming_msg_info.msg_ptr->payload[6], // generate an OK reply by default for Read/Write/Stream
                6);

    }

    //   statemachine_info->incoming_msg_info.msg_ptr->payload_count = 6;

}

static void _handle_read_request(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    uint16_t error_code = S_OK;

    _extract_read_command_parameters(statemachine_info, config_mem_read_request_info);

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

        error_code = _is_valid_read_parameters(config_mem_read_request_info);

        if (error_code) {

            _interface->load_datagram_received_rejected_message(statemachine_info, error_code);

        } else {
            _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

            statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
            statemachine_info->incoming_msg_info.enumerate = true; // call this again for the data
        }

        return;
    }

    // Complete Command Request
    if (config_mem_read_request_info->read_space_func) {

        _check_for_read_overrun(statemachine_info, config_mem_read_request_info);
        config_mem_read_request_info->read_space_func(statemachine_info, config_mem_read_request_info);

    }

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // reset
    statemachine_info->incoming_msg_info.enumerate = false; // done
}

static void _read_request_configuration_definition_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_read_request_info);

    if (_interface->on_read_space_config_decscription_info) {

        _interface->on_read_space_config_decscription_info(statemachine_info, config_mem_read_request_info);

        return;
    }

    OpenLcbUtilities_copy_byte_array_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            &statemachine_info->openlcb_node->parameters->cdi[config_mem_read_request_info->address],
            config_mem_read_request_info->data_start,
            config_mem_read_request_info->bytes);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = (config_mem_read_request_info->data_start) + config_mem_read_request_info->bytes;

    statemachine_info->outgoing_msg_info.valid = true;
}

static void _read_request_all(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_read_request_info);

    if (_interface->on_read_space_all) {

        _interface->on_read_space_all(statemachine_info, config_mem_read_request_info);

        return;
    }

    statemachine_info->outgoing_msg_info.valid = false;
}

static void _read_request_config_mem(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_read_request_info);

    if (_interface->on_read_space_configuration_memory) {
       
        _interface->on_read_space_configuration_memory(statemachine_info, config_mem_read_request_info);

        return;
    }
    
    if (_interface->configuration_memory_read) {
        
        uint16_t read_count = _interface->configuration_memory_read(
                config_mem_read_request_info->address, 
                config_mem_read_request_info->bytes, 
                (configuration_memory_buffer_t*) &statemachine_info->outgoing_msg_info.msg_ptr->payload[config_mem_read_request_info->data_start]
                );
       
        statemachine_info->outgoing_msg_info.msg_ptr->payload_count += read_count;
    }

    statemachine_info->outgoing_msg_info.valid = true;
}

static void _read_request_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_read_request_info);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = config_mem_read_request_info->data_start;

    if (_interface->on_read_space_acdi_manufacturer) {

        _interface->on_read_space_acdi_manufacturer(statemachine_info, config_mem_read_request_info);

        return;
    }

    switch (config_mem_read_request_info->address) {

        case ACDI_ADDRESS_SPACE_FB_VERSION_ADDRESS:

            _interface->snip_load_manufacturer_version_id(
                    statemachine_info->openlcb_node,
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_read_request_info->data_start,
                    config_mem_read_request_info->bytes
                    );

            break;

        case ACDI_ADDRESS_SPACE_FB_MANUFACTURER_ADDRESS:

            _interface->snip_load_name(
                    statemachine_info->openlcb_node,
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_read_request_info->data_start,
                    config_mem_read_request_info->bytes
                    );

            break;

        case ACDI_ADDRESS_SPACE_FB_MODEL_ADDRESS:

            _interface->snip_load_model(
                    statemachine_info->openlcb_node,
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_read_request_info->data_start,
                    config_mem_read_request_info->bytes
                    );

            break;

        case ACDI_ADDRESS_SPACE_FB_HARDWARE_VERSION_ADDRESS:

            _interface->snip_load_hardware_version(
                    statemachine_info->openlcb_node,
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_read_request_info->data_start,
                    config_mem_read_request_info->bytes
                    );

            break;

        case ACDI_ADDRESS_SPACE_FB_SOFTWARE_VERSION_ADDRESS:

            _interface->snip_load_software_version(
                    statemachine_info->openlcb_node,
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_read_request_info->data_start,
                    config_mem_read_request_info->bytes
                    );

            break;

        default:

            _interface->load_datagram_received_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);

            break;
    }

    statemachine_info->outgoing_msg_info.valid = true;
}

static void _read_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_read_request_info);

    if (_interface->on_read_space_acdi_user) {

        _interface->on_read_space_acdi_user(statemachine_info, config_mem_read_request_info);

        return;
    }

    switch (config_mem_read_request_info->address) {

        case ACDI_ADDRESS_SPACE_FC_VERSION_ADDRESS:

            _interface->snip_load_user_version_id(
                    statemachine_info->openlcb_node,
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_read_request_info->data_start,
                    config_mem_read_request_info->bytes
                    );

            break;

        case ACDI_ADDRESS_SPACE_FC_NAME_ADDRESS:

            _interface->snip_load_user_name(
                    statemachine_info->openlcb_node,
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_read_request_info->data_start,
                    config_mem_read_request_info->bytes
                    );

            break;

        case ACDI_ADDRESS_SPACE_FC_DESCRIPTION_ADDRESS:

            _interface->snip_load_user_description(
                    statemachine_info->openlcb_node,
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_read_request_info->data_start,
                    config_mem_read_request_info->bytes
                    );

            break;

        default:

            _interface->load_datagram_received_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);

            break;
    }

    statemachine_info->outgoing_msg_info.valid = true;
}

static void _read_request_traction_function_configuration_definition_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_read_request_info);

    if (_interface->on_read_space_traction_config_decscription_info) {

        _interface->on_read_space_traction_config_decscription_info(statemachine_info, config_mem_read_request_info);

        return;
    }

    statemachine_info->outgoing_msg_info.valid = false;
}

static void _read_request_traction_function_configuration_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_read_request_info);

    if (_interface->on_read_space_traction_config_memory) {

        _interface->on_read_space_traction_config_memory(statemachine_info, config_mem_read_request_info);

        return;
    }

    statemachine_info->outgoing_msg_info.valid = false;
}

void ProtocolConfigMemReadHandler_memory_read_space_config_description_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = &_read_request_configuration_definition_info;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_configuration_definition;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

void ProtocolConfigMemReadHandler_memory_read_space_all(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = &_read_request_all;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_all;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

void ProtocolConfigMemReadHandler_memory_read_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = &_read_request_config_mem;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_config_memory;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

void ProtocolConfigMemReadHandler_memory_read_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = &_read_request_acdi_manufacturer;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

void ProtocolConfigMemReadHandler_memory_read_space_acdi_user(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = &_read_request_acdi_user;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_user;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

void ProtocolConfigMemReadHandler_memory_read_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = &_read_request_traction_function_configuration_definition_info;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_traction_function_definition_info;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

void ProtocolConfigMemReadHandler_memory_read_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = &_read_request_traction_function_configuration_memory;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_traction_function_config_memory;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

void ProtocolConfigMemReadHandler_memory_read_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {
}

void ProtocolConfigMemReadHandlerr_memory_read_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {
}

void ProtocolConfigMemReadHandler_memory_read_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {
}
