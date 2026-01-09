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
 * \file protocol_config_mem_write_handler.c
 *
 *
 * @author Jim Kueneman
 * @date 23 Oct 2025
 */

#include "protocol_config_mem_write_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf
#include <string.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"


static interface_protocol_config_mem_write_handler_t* _interface;

void ProtocolConfigMemWriteHandler_initialize(const interface_protocol_config_mem_write_handler_t *interface_protocol_config_mem_write_handler) {

    _interface = (interface_protocol_config_mem_write_handler_t*) interface_protocol_config_mem_write_handler;

}

static void _extract_write_command_parameters(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[1] == CONFIG_MEM_WRITE_SPACE_IN_BYTE_6) {

        config_mem_write_request_info->encoding = ADDRESS_SPACE_IN_BYTE_6;
        config_mem_write_request_info->bytes = statemachine_info->incoming_msg_info.msg_ptr->payload_count - 7;
        config_mem_write_request_info->data_start = 7;

    } else {

        config_mem_write_request_info->encoding = ADDRESS_SPACE_IN_BYTE_1;
        config_mem_write_request_info->bytes = statemachine_info->incoming_msg_info.msg_ptr->payload_count - 6;
        config_mem_write_request_info->data_start = 6;
    }

    config_mem_write_request_info->address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);
    config_mem_write_request_info->write_buffer = (configuration_memory_buffer_t*) & statemachine_info->incoming_msg_info.msg_ptr->payload[config_mem_write_request_info->data_start];

}

static uint16_t _is_valid_write_parameters(config_mem_write_request_info_t *config_mem_write_request_info) {

    if (!config_mem_write_request_info->space_info->present) {

        return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN;
    }

    if (config_mem_write_request_info->space_info->read_only) {

        return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_WRITE_TO_READ_ONLY;
    }

    if (config_mem_write_request_info->address > config_mem_write_request_info->space_info->highest_address) {

        return ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS;
    }

    if (config_mem_write_request_info->bytes > 64) {

        return ERROR_PERMANENT_INVALID_ARGUMENTS;
    }

    if (config_mem_write_request_info->bytes == 0) {

        return ERROR_PERMANENT_INVALID_ARGUMENTS;
    }

    return S_OK;
}

static void _check_for_write_overrun(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    // Don't read past the end of the space

    if ((config_mem_write_request_info->address + config_mem_write_request_info->bytes) >= config_mem_write_request_info->space_info->highest_address) {

        config_mem_write_request_info->bytes = (uint8_t) (config_mem_write_request_info->space_info->highest_address - config_mem_write_request_info->address) + 1; // length +1 due to 0...end

    }
}

static void _dispatch_write_request(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    uint16_t error_code = S_OK;

    _extract_write_command_parameters(statemachine_info, config_mem_write_request_info);

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

        error_code = _is_valid_write_parameters(config_mem_write_request_info);

        if (error_code) {

            _interface->load_datagram_received_rejected_message(statemachine_info, error_code);

        } else {

            if (_interface->delayed_reply_time) {

                _interface->load_datagram_received_ok_message(statemachine_info, _interface->delayed_reply_time(statemachine_info, config_mem_write_request_info));

            } else {

                _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

            }

            statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
            statemachine_info->incoming_msg_info.enumerate = true; // call this again for the data
        }

        return;
    }

    // Try to Complete Command Request, we know that config_mem_write_request_info->write_space_func is valid if we get here

    _check_for_write_overrun(statemachine_info, config_mem_write_request_info);
    config_mem_write_request_info->write_space_func(statemachine_info, config_mem_write_request_info);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // Done
    statemachine_info->incoming_msg_info.enumerate = false; // done

}

void ProtocolConfigMemWriteHandler_write_space_config_description_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_config_definition_info;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_configuration_definition;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

void ProtocolConfigMemWriteHandler_write_space_all(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_all;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_all;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

void ProtocolConfigMemWriteHandler_write_space_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_config_mem;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_config_memory;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

void ProtocolConfigMemWriteHandler_write_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_acdi_manufacturer;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

void ProtocolConfigMemWriteHandler_write_space_acdi_user(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_acdi_user;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_user;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

void ProtocolConfigMemWriteHandler_write_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_traction_function_config_definition_info;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_traction_function_definition_info;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

void ProtocolConfigMemWriteHandler_write_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_traction_function_config_memory;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_traction_function_config_memory;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

void ProtocolConfigMemWriteHandler_write_space_firmware(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_firmware;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_firmware;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

void ProtocolConfigMemWriteHandler_write_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {
}

void ProtocolConfigMemWriteHandler_write_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {
}

void ProtocolConfigMemWriteHandler_write_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {
}

// ----------------------------------------------------------------------------
// Implemented Write Requests
// ----------------------------------------------------------------------------


static uint16_t _write_data(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    uint16_t write_count = 0;

    if (_interface->config_memory_write) {

        write_count = _interface->config_memory_write(
                statemachine_info->openlcb_node,
                config_mem_write_request_info->address,
                config_mem_write_request_info->bytes,
                config_mem_write_request_info->write_buffer
                );

        statemachine_info->outgoing_msg_info.msg_ptr->payload_count += write_count;

        if (write_count < config_mem_write_request_info->bytes) {
            
            OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(statemachine_info, config_mem_write_request_info, ERROR_TEMPORARY_TRANSFER_ERROR);

        }

    } else {
        
        
        OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(statemachine_info, config_mem_write_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

    }

    statemachine_info->outgoing_msg_info.valid = true;

    return write_count;

}

void ProtocolConfigMemWriteHandler_write_request_config_mem(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

     OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(statemachine_info, config_mem_write_request_info);

    _write_data(statemachine_info, config_mem_write_request_info);

}

void ProtocolConfigMemWriteHandler_write_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(statemachine_info, config_mem_write_request_info);

    switch (config_mem_write_request_info->address) {

        case CONFIG_MEM_ACDI_USER_NAME_ADDRESS:
            
            // TODO MAPPING TO ACDI ADDRESSES....

            _write_data(statemachine_info, config_mem_write_request_info);

            break;

        case CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS:

            // TODO MAPPING TO ACDI ADDRESSES....
            
            _write_data(statemachine_info, config_mem_write_request_info);

            break;

        default:

            OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(statemachine_info, config_mem_write_request_info, ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);
  
            break;
    }

    statemachine_info->outgoing_msg_info.valid = true;
}
