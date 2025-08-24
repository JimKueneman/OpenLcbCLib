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
 * \file protocol_datagram.c
 *
 * Implementation of the Openlcb Datagram Protocol.  Handlers are call from the 
 * openlcb_main_statemachine.c processing when a datagram message is being processed
 * from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "protocol_datagram_handlers.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"

static interface_openlcb_protocol_datagram_config_mem_handler_t *_interface;

void ProtocolDatagramConfigMemHandler_initialize(const interface_openlcb_protocol_datagram_config_mem_handler_t *interface_openlcb_protocol_datagram_config_mem_handler) {

    _interface = (interface_openlcb_protocol_datagram_config_mem_handler_t*) interface_openlcb_protocol_datagram_config_mem_handler;

}

static const user_address_space_info_t* _decode_to_space_definition(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    switch (*openlcb_msg->payload[2]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            return (&openlcb_node->parameters->address_space_configuration_definition);

        case ADDRESS_SPACE_ALL:

            return &openlcb_node->parameters->address_space_all;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            return&openlcb_node->parameters->address_space_config_memory;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            return&openlcb_node->parameters->address_space_acdi_manufacturer;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            return&openlcb_node->parameters->address_space_acdi_user;

#ifdef SUPPORT_TRACTION
#ifndef SUPPORT_FIRMWARE_BOOTLOADER
        case ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            return &openlcb_node->parameters->address_space_train_function_definition;

        case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            return &openlcb_node->parameters->address_space_train_function_config_memory;
#endif
#endif

#ifdef SUPPORT_FIRMWARE_BOOTLOADER
        case ADDRESS_SPACE_FIRMWARE:
            return &openlcb_node->parameters->address_space_firmware;
#endif
        default:

            return NULL;

    }

}

void ProtocolDatagramConfigMemHandler_clear_resend_datagram_message(openlcb_node_t* openlcb_node) {

    if (openlcb_node->last_received_datagram) {

        OpenLcbBufferStore_free_buffer(openlcb_node->last_received_datagram);

        openlcb_node->last_received_datagram = NULL;

    }

    openlcb_node->state.resend_datagram = false;

}

static void _buffer_datagram_message_for_temporary_ack_reject_resend(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    ProtocolDatagramConfigMemHandler_clear_resend_datagram_message(openlcb_node);

    // Take a reference and store the sent message in case we have to resend it
    OpenLcbBufferStore_inc_reference_count(openlcb_msg);

    openlcb_node->last_received_datagram = openlcb_msg;

}

void ProtocolDatagramConfigMemHandler_try_transmit(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

//    if (_interface->transmit_openlcb_message(worker_msg)) {
//
//        if (!openlcb_node->state.resend_datagram) // if we are currently process a resend don't reload it
//
          _buffer_datagram_message_for_temporary_ack_reject_resend(openlcb_node, openlcb_msg);
//
//    }

}

void ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint16_t error_code) {

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM_REJECTED_REPLY, 2);
    OpenLcbUtilities_copy_word_to_openlcb_payload(worker_msg, error_code, 0);

}

static void _send_datagram_ack_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint16_t reply_pending_code) {

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM_OK_REPLY, 2);

    *worker_msg->payload[0] = (uint8_t) reply_pending_code;
    worker_msg->payload_count = 1;
}

static uint16_t _validate_memory_read_space_parameters(const user_address_space_info_t* space_info, uint32_t data_address, uint16_t* requested_byte_count) {

    if (!space_info->present) {

        return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND;

    }

    if (data_address > space_info->highest_address) {

        return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_COMMAND;

    }

    if (*requested_byte_count > 64) {

        return ERROR_CODE_PERMANENT_COUNT_OUT_OF_RANGE;

    }

    if (*requested_byte_count == 0) {

        return ERROR_CODE_PERMANENT_COUNT_OUT_OF_RANGE;

    }

    if ((data_address + *requested_byte_count) > space_info->highest_address) {

        *requested_byte_count = (uint8_t) (space_info->highest_address - data_address + 1); // length +1 due to 0...end

    }

    return 0;
}

static uint16_t _validate_memory_write_space_parameters(const user_address_space_info_t* space_info, uint32_t data_address, uint16_t* requested_byte_count) {

    if (space_info->read_only) {

        return ERROR_PERMANENT_NOT_IMPLEMENTED;

    }

    return _validate_memory_read_space_parameters(space_info, data_address, requested_byte_count);

}

#ifndef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _read_memory_space_cdi(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {

    uint16_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_configuration_definition, data_address, &requested_byte_count);
    if (invalid) {

        return invalid;

    }

    return reply_payload_index + OpenLcbUtilities_copy_byte_array_to_openlcb_payload(worker_msg, &openlcb_node->parameters->cdi[data_address], reply_payload_index, requested_byte_count);

}
#endif

#ifndef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _read_memory_space_all(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {

    uint16_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_all, data_address, &requested_byte_count);
    if (invalid) {

        return invalid;

    }

    return ERROR_PERMANENT_NOT_IMPLEMENTED;

}
#endif

#ifndef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _read_memory_space_configuration_memory(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {

    uint16_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_config_memory, data_address, &requested_byte_count);
    if (invalid) {

        return invalid;

    }

    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node);

    return reply_payload_index + _interface->configuration_memory_read(data_address, requested_byte_count, (configuration_memory_buffer_t*) (&worker_msg->payload[reply_payload_index]));

}
#endif

#ifndef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _read_memory_space_acdi_manufacurer(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {

    uint16_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_acdi_manufacturer, data_address, &requested_byte_count);
    if (invalid) {

        return invalid;

    }

    switch (data_address) {

        case ACDI_ADDRESS_SPACE_FB_VERSION_ADDRESS:

            return _interface->snip_load_manufacturer_version_id(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);

        case ACDI_ADDRESS_SPACE_FB_MANUFACTURER_ADDRESS:

            return _interface->snip_load_name(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);

        case ACDI_ADDRESS_SPACE_FB_MODEL_ADDRESS:

            return _interface->snip_load_model(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);

        case ACDI_ADDRESS_SPACE_FB_HARDWARE_VERSION_ADDRESS:

            return _interface->snip_load_hardware_version(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);

        case ACDI_ADDRESS_SPACE_FB_SOFTWARE_VERSION_ADDRESS:

            return _interface->snip_load_software_version(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);

        default:

            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL;

    }

}
#endif

#ifndef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _read_memory_space_acdi_user(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {

    uint16_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_acdi_user, data_address, &requested_byte_count);
    if (invalid) {

        return invalid;

    }

    switch (data_address) {

        case ACDI_ADDRESS_SPACE_FC_VERSION_ADDRESS:

            return _interface->snip_load_user_version_id(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);

        case ACDI_ADDRESS_SPACE_FC_NAME_ADDRESS:

            return _interface->snip_load_user_name(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);

        case ACDI_ADDRESS_SPACE_FC_DESCRIPTION_ADDRESS:

            return _interface->snip_load_user_description(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);

        default:

            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL;

    }

}
#endif

#ifdef SUPPORT_TRACTION
#ifndef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _read_memory_space_train_function_definition_info(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {

    uint16_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_train_function_definition, data_address, &requested_byte_count);
    if (invalid) {

        return invalid;

    }

    return reply_payload_index + OpenLcbUtilities_copy_byte_array_to_openlcb_payload(worker_msg, &openlcb_node->parameters->fdi[data_address], reply_payload_index, requested_byte_count);
}

static uint16_t _read_memory_space_train_function_configuration_memory(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {

    uint16_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_train_function_config_memory, data_address, &requested_byte_count);
    if (invalid) {

        return invalid;

    }

    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node);

    return reply_payload_index + DriverConfigurationMemory_read(data_address, requested_byte_count, (configuration_memory_buffer_t*) (&worker_msg->payload[reply_payload_index]));

}
#endif
#endif

static uint16_t _read_memory_space(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count, uint8_t space) {

    switch (space) {

#ifndef SUPPORT_FIRMWARE_BOOTLOADER
        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            return _read_memory_space_cdi(openlcb_node, worker_msg, data_address, reply_payload_index, requested_byte_count);

        case ADDRESS_SPACE_ALL:

            return _read_memory_space_all(openlcb_node, worker_msg, data_address, reply_payload_index, requested_byte_count);

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            return _read_memory_space_configuration_memory(openlcb_node, worker_msg, data_address, reply_payload_index, requested_byte_count);

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            return _read_memory_space_acdi_manufacurer(openlcb_node, worker_msg, data_address, reply_payload_index, requested_byte_count);

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            return _read_memory_space_acdi_user(openlcb_node, worker_msg, data_address, reply_payload_index, requested_byte_count);
#ifdef SUPPORT_TRACTION
        case ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            return _read_memory_space_train_function_definition_info(openlcb_node, worker_msg, data_address, reply_payload_index, requested_byte_count);

        case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            return _read_memory_space_train_function_configuration_memory(openlcb_node, worker_msg, data_address, reply_payload_index, requested_byte_count);
#endif
#endif
        default:

            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND;
    }

}

#ifndef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _write_memory_space_configuration_memory(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {


    uint16_t invalid = _validate_memory_write_space_parameters(&openlcb_node->parameters->address_space_config_memory, data_address, &requested_byte_count);
    if (invalid) {

        return invalid;

    }

    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node);

    uint16_t write_count = _interface->configuration_memory_write(data_address, requested_byte_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));

    if (_interface->on_config_mem_write) {

        _interface->on_config_mem_write(data_address, requested_byte_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));

    }

    return write_count;
}
#endif

#ifndef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _write_memory_space_acdi_user(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t data_count) {

    uint16_t invalid = _validate_memory_write_space_parameters(&openlcb_node->parameters->address_space_acdi_user, data_address, &data_count);
    if (invalid) {

        return invalid;

    }

    data_address = data_address - 1; // ACDI addresses are shifted to the right one for the Version byte

    // ADCI spaces are always mapped referenced to zero so offset by where the config memory starts 
    if (openlcb_node->parameters->address_space_config_memory.low_address_valid)
        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    // TODO: Should I check for and insert a terminating NULL if it is missing... 

    return _write_memory_space_configuration_memory(openlcb_node, openlcb_msg, data_address, reply_payload_index, data_count);

}
#endif

#ifdef SUPPORT_TRACTION

static uint16_t _write_memory_space_train_function_configuration_memory(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t data_count) {

    uint16_t invalid = _validate_memory_write_space_parameters(&openlcb_node->parameters->address_space_train_function_config_memory, data_address, &data_count);
    if (invalid) {

        return invalid;

    }

    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node);

TODO:
    THIS IS WRONG.... NOT CONFIGURATION MEMORY WRITE REALLY
    return DriverConfigurationMemory_write(data_address, data_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));

}
#endif

#ifdef SUPPORT_FIRMWARE_BOOTLOADER

static uint16_t _write_memory_space_firmware(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t data_count) {

    uint16_t invalid = _validate_memory_write_space_parameters(&openlcb_node->parameters->address_space_firmware, data_address, &data_count);
    if (invalid) {

        return invalid;

    }

    //   data_address = data_address + openlcb_node->parameters->firmware_image_offset;

    //    TODO: This should not be the Configuration Memory 
    return DriverConfigurationMemory_get_write_callback()(data_address, data_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));

}
#endif

static uint16_t _write_memory_space(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_t data_address, uint16_t reply_payload_index, uint16_t data_count, uint8_t space) {

    switch (space) {
#ifndef SUPPORT_FIRMWARE_BOOTLOADER

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            return _write_memory_space_configuration_memory(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            return _write_memory_space_acdi_user(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);
#endif
#ifdef SUPPORT_TRACTION
#ifndef SUPPORT_FIRMWARE_BOOTLOADER
        case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            return _write_memory_space_train_function_configuration_memory(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);
#endif
#endif
#ifdef SUPPORT_FIRMWARE_BOOTLOADER
        case ADDRESS_SPACE_FIRMWARE:

            return _write_memory_space_firmware(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);
#endif

        default:

            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND;
    }

}


// Memory read handlers

void ProtocolDatagramConfigMemHandler_handle_memory_read_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    uint16_t data_count = (*openlcb_msg->payload[6]) & 0x7F; // top bit reserved
    uint16_t reply_payload_index = 6;
    uint32_t data_address = OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg, 2);

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    if (*openlcb_msg->payload[1] == DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6) {

        reply_payload_index = 7;
        *worker_msg->payload[6] = space;
        data_count = (*openlcb_msg->payload[7]) & 0x7F; // top bit reserved

    }

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(worker_msg, data_address, 2);

    uint16_t read_result_or_error_code = _read_memory_space(openlcb_node, worker_msg, data_address, reply_payload_index, data_count, space);

    if (read_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {

        *worker_msg->payload[1] = return_msg_ok; // read_result is the current payload index in this case
        worker_msg->payload_count = read_result_or_error_code;

    } else {
        *worker_msg->payload[1] = return_msg_fail;
        OpenLcbUtilities_copy_word_to_openlcb_payload(worker_msg, read_result_or_error_code, reply_payload_index); // read_result is the error code in this case
        worker_msg->payload_count = reply_payload_index + 2;

    }

    ProtocolDatagramConfigMemHandler_try_transmit(openlcb_node, openlcb_msg, worker_msg);

}

void ProtocolDatagramConfigMemHandler_handle_memory_read_reply_ok_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

}

void ProtocolDatagramConfigMemHandler_handle_memory_read_reply_fail_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }
    
}


// Memory write handlers

void ProtocolDatagramConfigMemHandler_handle_memory_write_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    uint16_t data_count = openlcb_msg->payload_count - 6;
    uint16_t payload_index = 6;
    uint32_t data_address = OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg, 2);

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    if (*openlcb_msg->payload[1] == DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6) {

        payload_index = 7;
        *worker_msg->payload[6] = space;
        data_count = openlcb_msg->payload_count - 7;

    }

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(worker_msg, data_address, 2);

    uint16_t write_result_or_error_code = _write_memory_space(openlcb_node, openlcb_msg, data_address, payload_index, data_count, space);

    if (write_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {

        *worker_msg->payload[1] = return_msg_ok; // write_result is the current payload index in this case
        worker_msg->payload_count = payload_index;

    } else {

        *worker_msg->payload[1] = return_msg_fail;
        OpenLcbUtilities_copy_word_to_openlcb_payload(worker_msg, write_result_or_error_code, payload_index); // write_result is the error code in this case
        worker_msg->payload_count = payload_index + 2;

    }

    ProtocolDatagramConfigMemHandler_try_transmit(openlcb_node, openlcb_msg, worker_msg);

}

void ProtocolDatagramConfigMemHandler_handle_memory_write_under_mask_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {


    uint8_t reply_payload_index = 6;
    uint32_t data_address = OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg, 2);

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    if (*openlcb_msg->payload[1] == DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6) {

        reply_payload_index = 7;
        *worker_msg->payload[6] = space;

    }

    OpenLcbUtilities_copy_dword_to_openlcb_payload(worker_msg, data_address, 2);

    uint16_t data_count = (openlcb_msg->payload_count - reply_payload_index);

    if ((data_count == 0) || (openlcb_msg->payload_count % 2 != 0)) {

        ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_INVALID_ARGUMENTS);

        return;

    }

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    // Use the worker messages payload as a buffer to read/write in the existing values
    uint16_t _result_or_error_code = _read_memory_space(openlcb_node, worker_msg, data_address, reply_payload_index, data_count, space);

    if (_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {

        uint16_t data_pair_index = reply_payload_index;
        uint16_t data_index = reply_payload_index;

        while (data_pair_index < (reply_payload_index + data_count)) {

            // (data_byte & data_mask) | (existing_byte & ~data_mask)
            *worker_msg->payload[data_index] =
                    (*openlcb_msg->payload[data_pair_index + 1] & *openlcb_msg->payload[data_pair_index]) |
                    (*worker_msg->payload[data_index] & ~(*openlcb_msg->payload[data_pair_index])
                    );

            data_pair_index = data_pair_index + 2;
            data_index = data_index + 1;

        }

        _result_or_error_code = _write_memory_space(openlcb_node, worker_msg, data_address, reply_payload_index, _result_or_error_code, space);

        if (_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {

            *worker_msg->payload[1] = return_msg_ok; // read_result is the current payload index in this case
            worker_msg->payload_count = reply_payload_index;


        } else {

            *worker_msg->payload[1] = return_msg_fail;
            OpenLcbUtilities_copy_word_to_openlcb_payload(worker_msg, _result_or_error_code, reply_payload_index); // read_result is the error code in this case
            worker_msg->payload_count = reply_payload_index + 2;

        }

    } else {

        *worker_msg->payload[1] = return_msg_fail;
        OpenLcbUtilities_copy_word_to_openlcb_payload(worker_msg, _result_or_error_code, reply_payload_index); // read_result is the error code in this case
        worker_msg->payload_count = reply_payload_index + 2;

    }

    ProtocolDatagramConfigMemHandler_try_transmit(openlcb_node, openlcb_msg, worker_msg);

}

void ProtocolDatagramConfigMemHandler_handle_memory_write_reply_ok_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

}

void ProtocolDatagramConfigMemHandler_handle_memory_write_reply_fail_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }


}

// Other memory access handlers

void ProtocolDatagramConfigMemHandler_handle_memory_options_cmd_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;

    *worker_msg->payload[1] = DATAGRAM_MEMORY_OPTIONS_REPLY;

    uint16_t available_commands = 0x00;

    if (openlcb_node->parameters->configuration_options.write_under_mask_supported) {

        available_commands = available_commands | 0x8000;

    }

    if (openlcb_node->parameters->configuration_options.unaligned_reads_supported) {

        available_commands = available_commands | 0x4000;

    }
    if (openlcb_node->parameters->configuration_options.unaligned_writes_supported) {

        available_commands = available_commands | 0x2000;

    }

    if (openlcb_node->parameters->configuration_options.read_from_manufacturer_space_0xfc_supported) {

        available_commands = available_commands | 0x0800;

    }

    if (openlcb_node->parameters->configuration_options.read_from_user_space_0xfb_supported) {

        available_commands = available_commands | 0x0400;

    }

    if (openlcb_node->parameters->configuration_options.write_to_user_space_0xfb_supported) {

        available_commands = available_commands | 0x0200;

    }

    if (openlcb_node->parameters->configuration_options.stream_read_write_supported) {

        available_commands = available_commands | 0x0001;

    }

    OpenLcbUtilities_copy_word_to_openlcb_payload(worker_msg, available_commands, 2);

    uint8_t write_lengths = 0x80 | 0x40 | 0x020 | 0x02;

    if (openlcb_node->parameters->configuration_options.stream_read_write_supported) {

        write_lengths = write_lengths | 0x01;

    }

    *worker_msg->payload[4] = write_lengths;
    *worker_msg->payload[5] = openlcb_node->parameters->configuration_options.high_address_space;
    *worker_msg->payload[6] = openlcb_node->parameters->configuration_options.low_address_space;

    worker_msg->payload_count = 7;

    if (openlcb_node->parameters->configuration_options.description[0] != 0x00)

        worker_msg->payload_count = worker_msg->payload_count + OpenLcbUtilities_copy_string_to_openlcb_payload(worker_msg, openlcb_node->parameters->configuration_options.description, worker_msg->payload_count);


    ProtocolDatagramConfigMemHandler_try_transmit(openlcb_node, openlcb_msg, worker_msg);

}

void ProtocolDatagramConfigMemHandler_handle_memory_options_reply_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

}

void ProtocolDatagramConfigMemHandler_handle_memory_get_address_space_info_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    const user_address_space_info_t* target_space = _decode_to_space_definition(openlcb_node, openlcb_msg);
    uint8_t invalid_space;

    if (target_space) {

        invalid_space = (!target_space->present);

    } else {

        invalid_space = true;

    }


    if (invalid_space) {

        OpenLcbUtilities_clear_openlcb_message_payload(worker_msg);

        *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
        *worker_msg->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_NOT_PRESENT;
        *worker_msg->payload[2] = *openlcb_msg->payload[2];
        *worker_msg->payload[7] = 0x01;

        worker_msg->payload_count = 8;

        return;

    }


    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *worker_msg->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_PRESENT;
    *worker_msg->payload[2] = target_space->address_space;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(worker_msg, target_space->highest_address, 3);
    *worker_msg->payload[7] = 0x00;

    if (target_space->read_only) {

        *worker_msg->payload[7] = *worker_msg->payload[7] | 0x01;

    }

    if (target_space->low_address_valid) {

        *worker_msg->payload[7] = *worker_msg->payload[7] | 0x02;
        OpenLcbUtilities_copy_dword_to_openlcb_payload(worker_msg, target_space->low_address, 8);

        worker_msg->payload_count = 12;

    } else {

        worker_msg->payload_count = 8;

    }

    if (target_space->description[0] != 0x00) {

        worker_msg->payload_count = worker_msg->payload_count + OpenLcbUtilities_copy_string_to_openlcb_payload(worker_msg, target_space->description, worker_msg->payload_count);
    }

    ProtocolDatagramConfigMemHandler_try_transmit(openlcb_node, openlcb_msg, worker_msg);

}

void ProtocolDatagramConfigMemHandler_handle_memory_get_address_space_info_reply_not_present_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

}



void ProtocolDatagramHandlers_handle_memory_get_address_space_info_reply_present_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

}

void ProtocolDatagramConfigMemHandler_handle_memory_reserve_lock_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    node_id_t new_node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg, 2);

    if (openlcb_node->lock_node == 0) {

        openlcb_node->lock_node = new_node_id;

    } else {

        if (new_node_id == 0)

            openlcb_node->lock_node = 0;

    }

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 8);

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *worker_msg->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_RESERVE_LOCK_REPLY;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(worker_msg, openlcb_node->lock_node, 2);

    ProtocolDatagramConfigMemHandler_try_transmit(openlcb_node, openlcb_msg, worker_msg);

}


void ProtocolDatagramConfigMemHandler_handle_memory_get_unique_id_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED);

        return;

    }
}

void ProtocolDatagramConfigMemHandler_handle_memory_unfreeze_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (*openlcb_msg->payload[2] == ADDRESS_SPACE_FIRMWARE) {

        if (openlcb_node->parameters->address_space_firmware.present) {

            if (!openlcb_node->state.openlcb_datagram_ack_sent) {

                _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

                return;

            } else {

              

                if (_interface->onconfig_mem_unfreeze_firmware_update) {

                    _interface->onconfig_mem_unfreeze_firmware_update(openlcb_node, openlcb_msg, worker_msg);

                }

                return;

            }

        }

    }

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);

        return;

    } else {

    }

}

void ProtocolDatagramConfigMemHandler_handle_memory_freeze_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (*openlcb_msg->payload[2] == ADDRESS_SPACE_FIRMWARE) {

        if (openlcb_node->parameters->address_space_firmware.present) {

            if (!openlcb_node->state.openlcb_datagram_ack_sent) {

                _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

                return;

            } else {
      

                if (_interface->onconfig_mem_freeze_firmware_update) {

                    _interface->onconfig_mem_freeze_firmware_update(openlcb_node, openlcb_msg, worker_msg);

                }

                return;

            }

        }

    }

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);

        return;

    } else {

        

    }

}

void ProtocolDatagramConfigMemHandler_handle_memory_update_complete_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

   
}

void ProtocolDatagramConfigMemHandler_handle_memory_reset_reboot_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    if (_interface->reboot) {

        _interface->reboot();

    }


}

void ProtocolDatagramConfigMemHandler_handle_memory_factory_reset_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    if (_interface->on_configuration_memory_factory_reset) {

        _interface->on_configuration_memory_factory_reset();

    }

   

}
