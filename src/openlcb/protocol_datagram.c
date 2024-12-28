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

#include "protocol_datagram.h"

#include "stdio.h" // printf
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_buffer_store.h"
#include "openlcb_tx_driver.h"
#include "protocol_snip.h"
#include "../drivers/driver_mcu.h"
#include "../drivers/driver_configuration_memory.h"

void ProtocolDatagram_initialize(void) {



}

void ProtocolDatagram_clear_resend_datagram_message(openlcb_node_t* openlcb_node) {

    if (openlcb_node->last_received_datagram) {

        BufferStore_freeBuffer(openlcb_node->last_received_datagram);

        openlcb_node->last_received_datagram = (void*) 0;

    }

    openlcb_node->state.resend_datagram = FALSE;

}

void _buffer_datagram_message_for_temporary_ack_reject_resend(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    ProtocolDatagram_clear_resend_datagram_message(openlcb_node);

    // Take a reference and store the sent message in case we have to resend it
    BufferStore_inc_reference_count(openlcb_msg);

    openlcb_node->last_received_datagram = openlcb_msg;

    openlcb_node->state.openlcb_msg_handled = TRUE;


}

void _try_transmist(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

        if (!openlcb_node->state.resend_datagram) // if we are currently process a resend don't reload it

            _buffer_datagram_message_for_temporary_ack_reject_resend(openlcb_node, openlcb_msg);

    }

}

const user_address_space_info_t* _decode_to_space_definition(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

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

        case ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:
            return &openlcb_node->parameters->address_space_train_function_definition;

        case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:
            return &openlcb_node->parameters->address_space_train_function_config_memory;

        default:

            return (void*) 0;

    }

}

void _send_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint16_olcb_t error_code) {

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM_REJECTED_REPLY, 2);
    Utilities_copy_word_to_openlcb_payload(worker_msg, error_code, 0);

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg))

        openlcb_node->state.openlcb_msg_handled = TRUE;


}

void _send_datagram_ack_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint16_olcb_t reply_pending_code) {

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM_OK_REPLY, 2);

    *worker_msg->payload[0] = reply_pending_code;
    worker_msg->payload_count = 1;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg))

        openlcb_node->state.openlcb_datagram_ack_sent = TRUE;

}

uint16_olcb_t _validate_memory_read_space_parameters(const user_address_space_info_t* space_info, uint32_olcb_t data_address, uint8_olcb_t* data_count) {

    if (!space_info->present)
        return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND;

    if (data_address > space_info->highest_address)
        return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_COMMAND;

    if (*data_count > 64)
        return ERROR_CODE_PERMANENT_COUNT_OUT_OF_RANGE;

    if (*data_count == 0)
        return ERROR_CODE_PERMANENT_COUNT_OUT_OF_RANGE;

    if ((data_address + *data_count) > space_info->highest_address)
        *data_count = space_info->highest_address - data_address;

    return 0;
}

uint16_olcb_t _validate_memory_write_space_parameters(const user_address_space_info_t* space_info, uint32_olcb_t data_address, uint8_olcb_t* data_count) {

    if (space_info->read_only)
        return ERROR_PERMANENT_NOT_IMPLEMENTED;

    return _validate_memory_read_space_parameters(space_info, data_address, data_count);

}

uint16_olcb_t read_memory_space_cdi(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_configuration_definition, data_address, &data_count);
    if (invalid)
        return invalid;

    return reply_payload_index + Utilities_copy_byte_array_to_openlcb_payload(worker_msg, &openlcb_node->parameters->cdi[data_address], reply_payload_index, data_count);

}

uint16_olcb_t _read_memory_space_all(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_all, data_address, &data_count);
    if (invalid)
        return invalid;

    return ERROR_PERMANENT_NOT_IMPLEMENTED;

}

uint16_olcb_t _read_memory_space_configuration_memory(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_config_memory, data_address, &data_count);
    if (invalid)
        return invalid;

    data_address = data_address + Utilities_calculate_memory_offset_into_node_space(openlcb_node);

    return reply_payload_index + DriverConfigurationMemory_read(data_address, data_count, (configuration_memory_buffer_t*) (&worker_msg->payload[reply_payload_index]));

}

uint16_olcb_t _read_memory_space_acdi_manufacurer(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_acdi_manufacturer, data_address, &data_count);
    if (invalid)
        return invalid;

    switch (data_address) {

        case ACDI_ADDRESS_SPACE_FB_VERSION_ADDRESS:
            return ProtocolSnip_load_manufacturer_version_id(openlcb_node, worker_msg, reply_payload_index, data_count);

        case ACDI_ADDRESS_SPACE_FB_MANUFACTURER_ADDRESS:
            return ProtocolSnip_load_name(openlcb_node, worker_msg, reply_payload_index, data_count);

        case ACDI_ADDRESS_SPACE_FB_MODEL_ADDRESS:
            return ProtocolSnip_load_model(openlcb_node, worker_msg, reply_payload_index, data_count);

        case ACDI_ADDRESS_SPACE_FB_HARDWARE_VERSION_ADDRESS:
            return ProtocolSnip_load_hardware_version(openlcb_node, worker_msg, reply_payload_index, data_count);

        case ACDI_ADDRESS_SPACE_FB_SOFTWARE_VERSION_ADDRESS:
            return ProtocolSnip_load_software_version(openlcb_node, worker_msg, reply_payload_index, data_count);

        default:
            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL;

    }

}

uint16_olcb_t _read_memory_space_acdi_user(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_acdi_user, data_address, &data_count);
    if (invalid)
        return invalid;

    switch (data_address) {

        case ACDI_ADDRESS_SPACE_FC_VERSION_ADDRESS:
            return ProtocolSnip_load_user_version_id(openlcb_node, worker_msg, reply_payload_index, data_count);

        case ACDI_ADDRESS_SPACE_FC_NAME_ADDRESS:
            return ProtocolSnip_load_user_name(openlcb_node, worker_msg, reply_payload_index, data_count);

        case ACDI_ADDRESS_SPACE_FC_DESCRIPTION_ADDRESS:
            return ProtocolSnip_load_user_description(openlcb_node, worker_msg, reply_payload_index, data_count);

        default:

            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL;

    }

}

uint16_olcb_t _read_memory_space_train_function_definition_info(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_train_function_definition, data_address, &data_count);
    if (invalid)
        return invalid;

    return reply_payload_index + Utilities_copy_byte_array_to_openlcb_payload(worker_msg, &openlcb_node->parameters->fdi[data_address], reply_payload_index, data_count);
}

uint16_olcb_t _read_memory_space_train_function_configuration_memory(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_read_space_parameters(&openlcb_node->parameters->address_space_train_function_config_memory, data_address, &data_count);
    if (invalid)
        return invalid;

    data_address = data_address + Utilities_calculate_memory_offset_into_node_space(openlcb_node);

    return reply_payload_index + DriverConfigurationMemory_read(data_address, data_count, (configuration_memory_buffer_t*) (&worker_msg->payload[reply_payload_index]));

}

uint16_olcb_t _read_memory_space(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count, uint8_olcb_t space) {

    switch (space) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:
            return read_memory_space_cdi(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_ALL:
            return _read_memory_space_all(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:
            return _read_memory_space_configuration_memory(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:
            return _read_memory_space_acdi_manufacurer(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_ACDI_USER_ACCESS:
            return _read_memory_space_acdi_user(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:
            return _read_memory_space_train_function_definition_info(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:
            return _read_memory_space_train_function_configuration_memory(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        default:
            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND;
    }

}

uint16_olcb_t _write_memory_space_configuration_memory(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_write_space_parameters(&openlcb_node->parameters->address_space_config_memory, data_address, &data_count);
    if (invalid)
        return invalid;

    data_address = data_address + Utilities_calculate_memory_offset_into_node_space(openlcb_node);

    return DriverConfigurationMemory_write(data_address, data_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));

}

uint16_olcb_t _write_memory_space_acdi_user(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_write_space_parameters(&openlcb_node->parameters->address_space_acdi_user, data_address, &data_count);
    if (invalid)
        return invalid;

    data_address = data_address - 1; // ACDI addresses are shifted to the right one for the Version byte

    // ADCI spaces are always mapped referenced to zero so offset by where the config memory starts 
    if (openlcb_node->parameters->address_space_config_memory.low_address_valid)
        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    // TODO: Should I check for and insert a terminating NULL if it is missing... 

    return _write_memory_space_configuration_memory(openlcb_node, openlcb_msg, data_address, reply_payload_index, data_count);

}

uint16_olcb_t _write_memory_space_train_function_configuration_memory(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count) {

    uint16_olcb_t invalid = _validate_memory_write_space_parameters(&openlcb_node->parameters->address_space_train_function_config_memory, data_address, &data_count);
    if (invalid)
        return invalid;

    data_address = data_address + Utilities_calculate_memory_offset_into_node_space(openlcb_node);

    return DriverConfigurationMemory_write(data_address, data_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));

}

uint16_olcb_t _write_memory_space(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint32_olcb_t data_address, uint16_olcb_t reply_payload_index, uint8_olcb_t data_count, uint8_olcb_t space) {

    switch (space) {

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:
            return _write_memory_space_configuration_memory(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_ACDI_USER_ACCESS:
            return _write_memory_space_acdi_user(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:
            return _write_memory_space_train_function_configuration_memory(openlcb_node, worker_msg, data_address, reply_payload_index, data_count);

        default:
            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND;
    }

}


// Memory read handlers

void _handle_memory_read_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space, uint8_olcb_t return_msg_ok, uint8_olcb_t return_msg_fail) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    uint16_olcb_t data_count = *openlcb_msg->payload[6];
    uint16_olcb_t reply_payload_index = 6;
    uint32_olcb_t data_address = Utilities_extract_dword_from_openlcb_payload(openlcb_msg, 2);

    if (*openlcb_msg->payload[1] == DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6) {

        reply_payload_index = 7;
        *worker_msg->payload[6] = space;
        data_count = *openlcb_msg->payload[7];

    }

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    Utilities_copy_dword_to_openlcb_payload(worker_msg, data_address, 2);


    uint16_olcb_t read_result_or_error_code = _read_memory_space(openlcb_node, worker_msg, data_address, reply_payload_index, data_count, space);

    if (read_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {

        *worker_msg->payload[1] = return_msg_ok; // read_result is the current payload index in this case
        worker_msg->payload_count = read_result_or_error_code;

    } else {

        *worker_msg->payload[1] = return_msg_fail;
        Utilities_copy_word_to_openlcb_payload(worker_msg, read_result_or_error_code, reply_payload_index); // read_result is the error code in this case
        worker_msg->payload_count = reply_payload_index + 2;

    }

    _try_transmist(openlcb_node, openlcb_msg, worker_msg);

}

void _handle_memory_read_reply_ok_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    // Reply if we sent a Memory Read Message

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_read_reply_fail_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    // Reply if we sent a Memory Read Message

    openlcb_node->state.openlcb_msg_handled = TRUE;

}


// Memory write handlers

void _handle_memory_write_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space, uint8_olcb_t return_msg_ok, uint8_olcb_t return_msg_fail) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    uint16_olcb_t data_count = openlcb_msg->payload_count - 6;
    uint16_olcb_t payload_index = 6;
    uint32_olcb_t data_address = Utilities_extract_dword_from_openlcb_payload(openlcb_msg, 2);

    if (*openlcb_msg->payload[1] == DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6) {

        payload_index = 7;
        *worker_msg->payload[6] = space;
        data_count = openlcb_msg->payload_count - 7;

    }

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    Utilities_copy_dword_to_openlcb_payload(worker_msg, data_address, 2);

    uint16_olcb_t write_result_or_error_code = _write_memory_space(openlcb_node, openlcb_msg, data_address, payload_index, data_count, space);

    if (write_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {

        *worker_msg->payload[1] = return_msg_ok; // write_result is the current payload index in this case
        worker_msg->payload_count = payload_index;

    } else {

        *worker_msg->payload[1] = return_msg_fail;
        Utilities_copy_word_to_openlcb_payload(worker_msg, write_result_or_error_code, payload_index); // write_result is the error code in this case
        worker_msg->payload_count = payload_index + 2;

    }

    _try_transmist(openlcb_node, openlcb_msg, worker_msg);

}

void _handle_memory_write_under_mask_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space, uint8_olcb_t return_msg_ok, uint8_olcb_t return_msg_fail) {


    uint16_olcb_t reply_payload_index = 6;
    uint32_olcb_t data_address = Utilities_extract_dword_from_openlcb_payload(openlcb_msg, 2);

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;

    if (*openlcb_msg->payload[1] == DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6) {

        reply_payload_index = 7;
        *worker_msg->payload[6] = space;

    }

    Utilities_copy_dword_to_openlcb_payload(worker_msg, data_address, 2);
    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    uint16_olcb_t data_count = (openlcb_msg->payload_count - reply_payload_index);

    if ((data_count == 0) || (openlcb_msg->payload_count % 2 != 0)) {

        _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_INVALID_ARGUMENTS);

        return;

    }

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    // Use the worker messages payload as a buffer to read/write in the existing values
    uint16_olcb_t _result_or_error_code = _read_memory_space(openlcb_node, worker_msg, data_address, reply_payload_index, data_count, space);

    if (_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {

        uint8_olcb_t data_pair_index = reply_payload_index;
        uint8_olcb_t data_index = reply_payload_index;

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
            Utilities_copy_word_to_openlcb_payload(worker_msg, _result_or_error_code, reply_payload_index); // read_result is the error code in this case
            worker_msg->payload_count = reply_payload_index + 2;

        }

    } else {

        *worker_msg->payload[1] = return_msg_fail;
        Utilities_copy_word_to_openlcb_payload(worker_msg, _result_or_error_code, reply_payload_index); // read_result is the error code in this case
        worker_msg->payload_count = reply_payload_index + 2;

    }

    _try_transmist(openlcb_node, openlcb_msg, worker_msg);

}

void _handle_memory_write_reply_ok_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    // Reply if we sent a Memory Write Message

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_write_reply_fail_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    // Reply if we sent a Memory Write Message

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

// Other memory access handlers

void _handle_memory_options_cmd_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;

    *worker_msg->payload[1] = DATAGRAM_MEMORY_OPTIONS_REPLY;

    uint16_olcb_t available_commands = 0x00;

    if (openlcb_node->parameters->configuration_options.write_under_mask_supported)
        available_commands = available_commands | 0x8000;
    if (openlcb_node->parameters->configuration_options.unaligned_reads_supported)
        available_commands = available_commands | 0x4000;
    if (openlcb_node->parameters->configuration_options.unaligned_writes_supported)
        available_commands = available_commands | 0x2000;
    if (openlcb_node->parameters->configuration_options.read_from_manufacturer_space_0xfc_supported)
        available_commands = available_commands | 0x0800;
    if (openlcb_node->parameters->configuration_options.read_from_user_space_0xfb_supported)
        available_commands = available_commands | 0x0400;
    if (openlcb_node->parameters->configuration_options.write_to_user_space_0xfb_supported)
        available_commands = available_commands | 0x0200;
    if (openlcb_node->parameters->configuration_options.stream_read_write_supported)
        available_commands = available_commands | 0x0001;
    Utilities_copy_word_to_openlcb_payload(worker_msg, available_commands, 2);

    uint8_olcb_t write_lengths = 0x80 | 0x40 | 0x020 | 0x02;

    if (openlcb_node->parameters->configuration_options.stream_read_write_supported)
        write_lengths = write_lengths | 0x01;

    *worker_msg->payload[4] = write_lengths;
    *worker_msg->payload[5] = openlcb_node->parameters->high_address_space;
    *worker_msg->payload[6] = openlcb_node->parameters->low_address_space;

    worker_msg->payload_count = 7;

    if (openlcb_node->parameters->configuration_options.description[0] != 0x00)

        worker_msg->payload_count = worker_msg->payload_count + Utilities_copy_string_to_openlcb_payload(worker_msg, openlcb_node->parameters->configuration_options.description, worker_msg->payload_count);


    _try_transmist(openlcb_node, openlcb_msg, worker_msg);

}

void _handle_memory_options_reply_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    // Reply if we sent a Memory Get Options Message

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_get_address_space_info_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    const user_address_space_info_t* target_space = _decode_to_space_definition(openlcb_node, openlcb_msg);
    uint8_olcb_t invalid_space;

    if (target_space)
        invalid_space = (!target_space->present);
    else
        invalid_space = TRUE;


    if (invalid_space) {

        Utilities_clear_openlcb_message_payload(worker_msg);

        *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
        *worker_msg->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_NOT_PRESENT;
        *worker_msg->payload[2] = *openlcb_msg->payload[2];
        *worker_msg->payload[7] = 0x01;

        worker_msg->payload_count = 8;

        if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg))
            openlcb_node->state.openlcb_msg_handled = TRUE;

        return;

    }


    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *worker_msg->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_PRESENT;
    *worker_msg->payload[2] = target_space->address_space;
    Utilities_copy_dword_to_openlcb_payload(worker_msg, target_space->highest_address, 3);
    *worker_msg->payload[7] = 0x00;

    if (target_space->read_only)
        *worker_msg->payload[7] = *worker_msg->payload[7] | 0x01;

    if (target_space->low_address_valid) {

        *worker_msg->payload[7] = *worker_msg->payload[7] | 0x02;
        Utilities_copy_dword_to_openlcb_payload(worker_msg, target_space->low_address, 8);

        worker_msg->payload_count = 12;

    } else

        worker_msg->payload_count = 8;

    if (target_space->description[0] != 0x00)

        worker_msg->payload_count = worker_msg->payload_count + Utilities_copy_string_to_openlcb_payload(worker_msg, target_space->description, worker_msg->payload_count);


    _try_transmist(openlcb_node, openlcb_msg, worker_msg);

}

void _handle_memory_get_address_space_info_reply_not_present_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    // Reply if we sent a Memory Address Space Info message and the space was not present

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_get_address_space_info_reply_present_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    // Reply if we sent a Memory Address Space Info message and the space was present

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_reserve_lock_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, MTI_DATAGRAM_OK_REPLY_PENDING);

        return;

    }

    node_id_t new_node_id = Utilities_extract_node_id_from_openlcb_payload(openlcb_msg, 2);

    if (openlcb_node->lock_node == 0)

        openlcb_node->lock_node = new_node_id;

    else {

        if (new_node_id == 0)

            openlcb_node->lock_node = 0;

    }

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_DATAGRAM, 0);

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *worker_msg->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_RESERVE_LOCK_REPLY;
    Utilities_copy_node_id_to_openlcb_payload(worker_msg, openlcb_node->lock_node, 2);

    _try_transmist(openlcb_node, openlcb_msg, worker_msg);

}

void _handle_memory_get_unique_id_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED);

        return;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_unfreeze_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED);

        return;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_freeze_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED);

        return;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_update_complete_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_reset_reboot_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    McuDriver_reboot();

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_factory_reset_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolDatagram_handle_datagram(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!Utilities_addressed_message_needs_processing(openlcb_node, openlcb_msg))
        return;

    switch (*openlcb_msg->payload[0]) {

        case DATAGRAM_MEMORY_CONFIGURATION: // are we 0x20?

            switch (*openlcb_msg->payload[1]) {

                case DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6:
                    _handle_memory_read_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6], DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6, DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6);
                    return;
                case DATAGRAM_MEMORY_READ_SPACE_FD:
                    _handle_memory_read_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY, DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FD, DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FD);
                    return;
                case DATAGRAM_MEMORY_READ_SPACE_FE:
                    _handle_memory_read_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL, DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FE, DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FE);
                    return;
                case DATAGRAM_MEMORY_READ_SPACE_FF:
                    _handle_memory_read_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO, DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FF, DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FF);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6:
                    _handle_memory_read_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FD:
                    _handle_memory_read_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FE:
                    _handle_memory_read_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FF:
                    _handle_memory_read_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6:
                    _handle_memory_read_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FD:
                    _handle_memory_read_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FE:
                    _handle_memory_read_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FF:
                    _handle_memory_read_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                    return;
                case DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6:
                    _handle_memory_write_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6], DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
                    return;
                case DATAGRAM_MEMORY_WRITE_SPACE_FD:
                    _handle_memory_write_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD);
                    return;
                case DATAGRAM_MEMORY_WRITE_SPACE_FE:
                    _handle_memory_write_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE);
                    return;
                case DATAGRAM_MEMORY_WRITE_SPACE_FF:
                    _handle_memory_write_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6:
                    _handle_memory_write_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD:
                    _handle_memory_write_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE:
                    _handle_memory_write_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF:
                    _handle_memory_write_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6:
                    _handle_memory_write_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD:
                    _handle_memory_write_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE:
                    _handle_memory_write_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF:
                    _handle_memory_write_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                    return;
                case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6:
                    _handle_memory_write_under_mask_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6], DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
                    return;
                case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FD:
                    _handle_memory_write_under_mask_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD);
                    return;
                case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FE:
                    _handle_memory_write_under_mask_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE);
                    return;
                case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FF:
                    _handle_memory_write_under_mask_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF);
                    return;
                case DATAGRAM_MEMORY_OPTIONS_CMD:
                    _handle_memory_options_cmd_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_OPTIONS_REPLY:
                    _handle_memory_options_reply_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_INFO_CMD:
                    _handle_memory_get_address_space_info_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_NOT_PRESENT:
                    _handle_memory_get_address_space_info_reply_not_present_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_PRESENT:
                    _handle_memory_get_address_space_info_reply_present_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_RESERVE_LOCK:
                    _handle_memory_reserve_lock_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_GET_UNIQUE_ID:
                    _handle_memory_get_unique_id_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_UNFREEZE:
                    _handle_memory_unfreeze_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_FREEZE:
                    _handle_memory_freeze_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_UPDATE_COMPLETE:
                    _handle_memory_update_complete_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_RESET_REBOOT:
                    _handle_memory_reset_reboot_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_FACTORY_RESET:
                    _handle_memory_factory_reset_message(openlcb_node, openlcb_msg, worker_msg);
                    return;

                default:
                    _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND);

            } // switch sub-command

        default:

            _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_COMMAND);

            return;

    } // switch command


    _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

}

void Protocol_Datagram_handle_datagram_ok_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    if (openlcb_node->last_received_datagram) {

        BufferStore_freeBuffer(openlcb_node->last_received_datagram);
        openlcb_node->last_received_datagram = (void*) 0;

    }

    openlcb_node->state.resend_datagram = FALSE;
    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolDatagram_handle_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;


    if (Utilities_extract_word_from_openlcb_payload(openlcb_msg, 0) && ERROR_TEMPORARY == ERROR_TEMPORARY) {

        if (openlcb_node->last_received_datagram) {

            openlcb_node->state.resend_datagram = TRUE;

        }

    } else {

        BufferStore_freeBuffer(openlcb_node->last_received_datagram);
        openlcb_node->last_received_datagram = (void*) 0;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void DatagramProtocol_100ms_time_tick(void) {


}




