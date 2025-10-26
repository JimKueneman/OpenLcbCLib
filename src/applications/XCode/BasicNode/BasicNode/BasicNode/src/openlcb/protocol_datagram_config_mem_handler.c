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

#include "protocol_datagram_config_mem_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"

//#define CONFIG_REPLY_OK_OFFSET 0x10
//#define CONFIG_REPLY_FAIL_OFFSET 0x18
//
//static interface_openlcb_protocol_datagram_config_mem_handler_t *_interface;
//
//typedef struct {
//    space_encoding_enum address_space_encoding;
//    uint8_t data_start_offset;
//    uint32_t address_requested;
//    uint16_t bytes_requested;
//    user_address_space_info_t *space_info;
//    uint16_t pending_timeout; // (0s - 32768s in 2^N steps)
//
//} config_mem_message_data_t;
//
//typedef enum {
//    ACK_OK_REPLY,
//    ACK_FAIL_REPLY,
//    ACK_COMPLETED
//
//} config_mem_datagram_reply_result_enum;
//
//typedef enum {
//    CONFIG_MEM_HANDLER_PROCESSING_ACK_OK,
//    CONFIG_MEM_HANDLER_READY,
//    CONFIG_MEM_HANDLER_ABORTED
//
//} config_mem_handler_result_enum;
//
//void ProtocolDatagramConfigMemHandler_initialize(const interface_openlcb_protocol_datagram_config_mem_handler_t *interface_openlcb_protocol_datagram_config_mem_handler) {
//
//    _interface = (interface_openlcb_protocol_datagram_config_mem_handler_t*) interface_openlcb_protocol_datagram_config_mem_handler;
//
//}
//
//static uint16_t _validate_memory_space_read_arguments(config_mem_message_data_t *config_mem_message_data) {
//
//    if (config_mem_message_data->address_requested > config_mem_message_data->space_info->highest_address) {
//
//        return ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS;
//
//    }
//
//    if (config_mem_message_data->bytes_requested > 64) {
//
//        return ERROR_PERMANENT_INVALID_ARGUMENTS;
//
//    }
//
//    if (config_mem_message_data->bytes_requested == 0) {
//
//        return ERROR_PERMANENT_INVALID_ARGUMENTS;
//
//    }
//
//    if ((config_mem_message_data->address_requested + config_mem_message_data->bytes_requested) > config_mem_message_data->space_info->highest_address) {
//
//        config_mem_message_data->bytes_requested = (uint8_t) (config_mem_message_data->space_info->highest_address - (config_mem_message_data->address_requested + 1)); // length +1 due to 0...end
//
//    }
//
//    return 0;
//}
//
//static uint16_t _validate_memory_space_write_arguments(config_mem_message_data_t *config_mem_message_data) {
//
//    if (config_mem_message_data->space_info->read_only) {
//
//        return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_WRITE_TO_READ_ONLY;
//
//    }
//
//    return _validate_memory_space_read_arguments(config_mem_message_data);
//
//}
//
//static config_mem_datagram_reply_result_enum _process_datagram_acknowledge(openlcb_statemachine_info_t *statemachine_info, config_mem_message_data_t *config_mem_message_data) {
//
//    if (statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//
//        return ACK_COMPLETED;
//
//    }
//
//    if (!config_mem_message_data->space_info->present) {
//
//        OpenLcbUtilities_load_openlcb_message(
//                statemachine_info->outgoing_msg_info.msg_ptr,
//                statemachine_info->openlcb_node->alias,
//                statemachine_info->openlcb_node->id,
//                statemachine_info->incoming_msg_info.msg_ptr->dest_alias,
//                statemachine_info->incoming_msg_info.msg_ptr->dest_id,
//                MTI_DATAGRAM_REJECTED_REPLY,
//                1);
//
//        OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN, 0);
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 2;
//
//        statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
//        statemachine_info->outgoing_msg_info.valid = true;
//
//        return ACK_FAIL_REPLY;
//
//    }
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->dest_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->dest_id,
//            MTI_DATAGRAM_OK_REPLY,
//            1);
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = 0x00;
//    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 1;
//
//    if (config_mem_message_data->pending_timeout > 1) {
//
//        uint16_t calculated_time = 2;
//        uint8_t N = 1;
//
//        while ((calculated_time < config_mem_message_data->pending_timeout) && (N < (15 - 1))) {
//
//            calculated_time = calculated_time << 2; // double each time is equivalent to the 2^N factor in the spec
//            N++;
//
//        }
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_OK_REPLY_PENDING | N;
//
//    }
//
//    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
//    statemachine_info->outgoing_msg_info.valid = true;
//
//    return ACK_OK_REPLY;
//}
//
//space_encoding_enum _decode_address_space_position(openlcb_msg_t* openlcb_msg) {
//
//    // In Read/Write/Stream the bottom nibble is zero (not true for Write under Mask though)
//
//    if ((*openlcb_msg->payload[1] & 0x0F) == 0x00) {
//
//        return ADDRESS_SPACE_IN_BYTE_6;
//
//    }
//
//    return ADDRESS_SPACE_IN_BYTE_1;
//
//}
//
//static void _load_config_mem_reply_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_message_data_t *config_mem_message_data) {
//
//    config_mem_message_data->data_start_offset = 6 + config_mem_message_data->address_space_encoding;
//    config_mem_message_data->address_requested = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);
//    config_mem_message_data->bytes_requested = OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, config_mem_message_data->data_start_offset);
//
//    // Load the common reply fields that all messages will have
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->dest_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->dest_id,
//            MTI_DATAGRAM,
//            0);
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = *statemachine_info->incoming_msg_info.msg_ptr->payload[1] + CONFIG_REPLY_OK_OFFSET; // generate an OK reply by default for Read/Write/Stream
//    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, config_mem_message_data->address_requested, 2);
//
//}
//
//static void _load_config_memory_fail_return_msg(openlcb_statemachine_info_t *statemachine_info, config_mem_message_data_t *config_mem_message_data, uint16_t error_code) {
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = *statemachine_info->incoming_msg_info.msg_ptr->payload[1] + CONFIG_REPLY_FAIL_OFFSET; // generate a reply failure for Read/Write/Stream
//    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;
//
//    if (config_mem_message_data->address_space_encoding == ADDRESS_SPACE_IN_BYTE_6) {
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[6] = *statemachine_info->incoming_msg_info.msg_ptr->payload[6];
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count++;
//
//    }
//
//    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, error_code, config_mem_message_data->data_start_offset);
//
//}
//
//static config_mem_handler_result_enum _process_read_request(openlcb_statemachine_info_t *statemachine_info, config_mem_message_data_t *config_mem_message_data) {
//
//    uint16_t error_code = 0;
//
//    switch (_process_datagram_acknowledge(statemachine_info, config_mem_message_data)) {
//
//        case ACK_OK_REPLY:
//
//            return CONFIG_MEM_HANDLER_PROCESSING_ACK_OK;
//
//        case ACK_FAIL_REPLY:
//
//            return CONFIG_MEM_HANDLER_ABORTED;
//
//        case ACK_COMPLETED:
//
//            error_code = _validate_memory_space_read_arguments(config_mem_message_data);
//
//            if (error_code > 0) {
//
//                _load_config_memory_fail_return_msg(statemachine_info, config_mem_message_data, error_code);
//
//                return CONFIG_MEM_HANDLER_ABORTED;
//
//            }
//
//            return CONFIG_MEM_HANDLER_READY;
//
//    }
//
//    return true;
//
//}
//
//static config_mem_handler_result_enum _process_write_request(openlcb_statemachine_info_t *statemachine_info, config_mem_message_data_t *config_mem_message_data) {
//
//    uint16_t error_code = 0;
//
//    switch (_process_datagram_acknowledge(statemachine_info, config_mem_message_data)) {
//
//        case ACK_OK_REPLY:
//
//            return CONFIG_MEM_HANDLER_PROCESSING_ACK_OK;
//
//        case ACK_FAIL_REPLY:
//
//            return CONFIG_MEM_HANDLER_ABORTED;
//
//        case ACK_COMPLETED:
//
//            error_code = _validate_memory_space_write_arguments(config_mem_message_data);
//
//            if (error_code > 0) {
//
//                _load_config_memory_fail_return_msg(statemachine_info, config_mem_message_data, error_code);
//
//                return CONFIG_MEM_HANDLER_ABORTED;
//
//            }
//
//            return CONFIG_MEM_HANDLER_READY;
//
//    }
//
//    return true;
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_read_space_config_description_info_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    config_mem_message_data_t config_mem_message_data = {.address_requested = 0, .bytes_requested = 0, .data_start_offset = 0};
//    config_mem_message_data.address_space_encoding = _decode_address_space_position(statemachine_info->incoming_msg_info.msg_ptr);
//    config_mem_message_data.space_info = (user_address_space_info_t*) & statemachine_info->openlcb_node->parameters->address_space_configuration_definition; // a const so need to un-const to stop compiler from complaining
//
//    switch (_process_read_request(statemachine_info, &config_mem_message_data)) {
//
//        case CONFIG_MEM_HANDLER_PROCESSING_ACK_OK: // not done yet with a message to send in worker_msg but need to call again to do the reply
//
//            statemachine_info->outgoing_msg_info.valid = false;
//
//            return;
//
//        case CONFIG_MEM_HANDLER_ABORTED: // done with last message we need to send in worker_msg, either a Datagram Rejected or Command Fail Config Mem Message
//
//            statemachine_info->outgoing_msg_info.valid = false;
//
//            return;
//
//        case CONFIG_MEM_HANDLER_READY: // done with the actual reply for the read
//
//            _load_config_mem_reply_message_header(statemachine_info, &config_mem_message_data);
//            OpenLcbUtilities_copy_byte_array_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr,
//                    &statemachine_info->openlcb_node->parameters->cdi[config_mem_message_data.address_requested],
//                    config_mem_message_data.data_start_offset, 
//                    config_mem_message_data.bytes_requested);
//
//            statemachine_info->outgoing_msg_info.msg_ptr->payload_count = config_mem_message_data.data_start_offset + config_mem_message_data.bytes_requested;
//
//            statemachine_info->outgoing_msg_info.valid = true;
//
//            return;
//    }
//
//    statemachine_info->outgoing_msg_info.valid = false; // unexpected error, make rouge message is not sent and flag done
//}
//
//void ProtocolDatagramConfigMemHandler_memory_read_space_all_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->parameters->address_space_all.present) {
//
//
//    }
//
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_read_space_configuration_memory_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->parameters->address_space_config_memory.present) {
//
//
//    }
//
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_read_space_acdi_manufacturer_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer.present) {
//
//    }
//
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_read_space_acdi_user_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->parameters->address_space_acdi_user.present) {
//
//    }
//
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_read_space_traction_function_definition_info_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->parameters->address_space_traction_function_definition_info.present) {
//
//        return;
//    }
//
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_read_space_traction_function_config_memory_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->parameters->address_space_traction_function_config_memory.present) {
//
//        return;
//    }
//
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_write_space_config_description_info_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    statemachine_info->outgoing_msg_info.valid = false; // Not a writable space
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_write_space_all_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    statemachine_info->outgoing_msg_info.valid = false; // Not a writable space
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_write_space_configuration_memory_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    config_mem_message_data_t config_mem_message_data = {.address_requested = 0, .bytes_requested = 0, .data_start_offset = 0};
//    config_mem_message_data.address_space_encoding = _decode_address_space_position(statemachine_info->incoming_msg_info.msg_ptr);
//    config_mem_message_data.space_info = (user_address_space_info_t*) & statemachine_info->openlcb_node->parameters->address_space_config_memory; // a const so need to un-const to stop compiler from complaining
//
//    switch (_process_write_request(statemachine_info, &config_mem_message_data)) {
//
//        case CONFIG_MEM_HANDLER_PROCESSING_ACK_OK: // not done yet with a message to send in worker_msg but need to call again to do the reply
//
//            statemachine_info->outgoing_msg_info.valid = false;
//
//        case CONFIG_MEM_HANDLER_ABORTED: // done with last message we need to send in worker_msg, either a Datagram Rejected or Command Fail Config Mem Message
//
//            statemachine_info->outgoing_msg_info.valid = false;
//
//        case CONFIG_MEM_HANDLER_READY: // done with the actual reply for the read
//
//            _load_config_mem_reply_message_header(statemachine_info, &config_mem_message_data);
//            //       OpenLcbUtilities_copy_byte_array_to_openlcb_payload(statemachine_info->outgoing_msg, &statemachine_info->openlcb_node->parameters->cdi[config_mem_message_data.address_requested], config_mem_message_data.data_start_offset, config_mem_message_data.bytes_requested);
//            //      statemachine_info->outgoing_msg->payload_count = config_mem_message_data.data_start_offset + config_mem_message_data.bytes_requested;
//
//            statemachine_info->outgoing_msg_info.valid = true;
//
//    }
//
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_write_space_acdi_manufacturer_message(openlcb_statemachine_info_t *statemachine_info) {
//
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_write_space_acdi_user_message(openlcb_statemachine_info_t *statemachine_info) {
//
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_write_space_traction_function_definition_info_message(openlcb_statemachine_info_t *statemachine_info) {
//
//
//}
//
//void ProtocolDatagramConfigMemHandler_memory_write_space_traction_function_config_memory_message(openlcb_statemachine_info_t *statemachine_info) {
//
//
//}
//
//
//
//
//
///// OLD stuff.......................
//
//static const user_address_space_info_t* _decode_to_space_definition(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {
//
//    switch (*openlcb_msg->payload[2]) {
//
//        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:
//
//            return (&openlcb_node->parameters->address_space_configuration_definition);
//
//        case ADDRESS_SPACE_ALL:
//
//            return &openlcb_node->parameters->address_space_all;
//
//        case ADDRESS_SPACE_CONFIGURATION_MEMORY:
//
//            return&openlcb_node->parameters->address_space_config_memory;
//
//        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:
//
//            return&openlcb_node->parameters->address_space_acdi_manufacturer;
//
//        case ADDRESS_SPACE_ACDI_USER_ACCESS:
//
//            return&openlcb_node->parameters->address_space_acdi_user;
//
//        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:
//
//            return &openlcb_node->parameters->address_space_traction_function_definition_info;
//
//        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:
//
//            return &openlcb_node->parameters->address_space_traction_function_config_memory;
//
//        case ADDRESS_SPACE_FIRMWARE:
//
//            return &openlcb_node->parameters->address_space_firmware;
//
//        default:
//
//            return NULL;
//
//    }
//
//}
//
//
//static void _buffer_datagram_message_for_temporary_ack_reject_resend(openlcb_statemachine_info_t *statemachine_info) {
//
//    _interface->clear_resend_datagram_message(statemachine_info->openlcb_node);
//
//    // Take a reference and store the sent message in case we have to resend it
//    OpenLcbBufferStore_inc_reference_count(statemachine_info->incoming_msg_info.msg_ptr);
//
//    statemachine_info->openlcb_node->last_received_datagram = statemachine_info->incoming_msg_info.msg_ptr;
//
//}
//
//void ProtocolDatagramConfigMemHandler_try_transmit(openlcb_statemachine_info_t *statemachine_info) {
//
//    //    if (_interface->transmit_openlcb_message(worker_msg)) {
//    //
//    //        if (!openlcb_node->state.resend_datagram) // if we are currently process a resend don't reload it
//    //
//    _buffer_datagram_message_for_temporary_ack_reject_resend(statemachine_info);
//    //
//    //    }
//
//}
//
//void ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code) {
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->source_id,
//            MTI_DATAGRAM_REJECTED_REPLY,
//            2);
//
//    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, error_code, 0);
//
//    statemachine_info->incoming_msg_info.enumerate = false; // d not call the message again to handle it after the NACK
//    statemachine_info->outgoing_msg_info.valid = true;
//
//}
//
//static void _send_datagram_ack_reply(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_code) {
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->source_id,
//            MTI_DATAGRAM_OK_REPLY,
//            2);
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = (uint8_t) reply_pending_code;
//    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 1;
//
//    statemachine_info->incoming_msg_info.enumerate = true; // call the message again to handle it after the ACK
//    statemachine_info->outgoing_msg_info.valid = true;
//
//}
//
//static uint16_t _read_memory_space_cdi(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_read_arguments(&openlcb_node->parameters->address_space_configuration_definition, &config_mem_message_data);
//    //
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    return reply_payload_index + OpenLcbUtilities_copy_byte_array_to_openlcb_payload(worker_msg, &openlcb_node->parameters->cdi[data_address], reply_payload_index, requested_byte_count);
//
//    return 0;
//}
//
//static uint16_t _read_memory_space_all(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_read_arguments(&openlcb_node->parameters->address_space_all, &config_mem_message_data);
//    //
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    return ERROR_PERMANENT_NOT_IMPLEMENTED;
//
//    return 0;
//
//}
//
//static uint16_t _read_memory_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_read_arguments(&openlcb_node->parameters->address_space_config_memory, &config_mem_message_data);
//    //
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node);
//    //
//    //    return reply_payload_index + _interface->configuration_memory_read(data_address, requested_byte_count, (configuration_memory_buffer_t*) (&worker_msg->payload[reply_payload_index]));
//
//    return 0;
//
//}
//
//static uint16_t _read_memory_space_acdi_manufacurer(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_read_arguments(&openlcb_node->parameters->address_space_acdi_manufacturer, &config_mem_message_data);
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    switch (data_address) {
//    //
//    //        case ACDI_ADDRESS_SPACE_FB_VERSION_ADDRESS:
//    //
//    //            return _interface->snip_load_manufacturer_version_id(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);
//    //
//    //        case ACDI_ADDRESS_SPACE_FB_MANUFACTURER_ADDRESS:
//    //
//    //            return _interface->snip_load_name(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);
//    //
//    //        case ACDI_ADDRESS_SPACE_FB_MODEL_ADDRESS:
//    //
//    //            return _interface->snip_load_model(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);
//    //
//    //        case ACDI_ADDRESS_SPACE_FB_HARDWARE_VERSION_ADDRESS:
//    //
//    //            return _interface->snip_load_hardware_version(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);
//    //
//    //        case ACDI_ADDRESS_SPACE_FB_SOFTWARE_VERSION_ADDRESS:
//    //
//    //            return _interface->snip_load_software_version(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);
//    //
//    //        default:
//    //
//    //            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL;
//    //
//    //    }
//
//    return 0;
//}
//
//static uint16_t _read_memory_space_acdi_user(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_read_arguments(&openlcb_node->parameters->address_space_acdi_user, &config_mem_message_data);
//    //
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    switch (data_address) {
//    //
//    //        case ACDI_ADDRESS_SPACE_FC_VERSION_ADDRESS:
//    //
//    //            return _interface->snip_load_user_version_id(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);
//    //
//    //        case ACDI_ADDRESS_SPACE_FC_NAME_ADDRESS:
//    //
//    //            return _interface->snip_load_user_name(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);
//    //
//    //        case ACDI_ADDRESS_SPACE_FC_DESCRIPTION_ADDRESS:
//    //
//    //            return _interface->snip_load_user_description(openlcb_node, worker_msg, reply_payload_index, requested_byte_count);
//    //
//    //        default:
//    //
//    //            return ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL;
//    //
//    //    }
//
//    return 0;
//
//}
//
//static uint16_t _read_memory_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_read_arguments(&openlcb_node->parameters->address_space_traction_function_definition_info, &config_mem_message_data);
//    //
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    return reply_payload_index + OpenLcbUtilities_copy_byte_array_to_openlcb_payload(worker_msg, &openlcb_node->parameters->fdi[data_address], reply_payload_index, requested_byte_count);
//
//    return 0;
//}
//
//static uint16_t _read_memory_space_train_function_configuration_memory(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_read_arguments(&openlcb_node->parameters->address_space_traction_function_config_memory, &config_mem_message_data);
//    //
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node);
//    //
//    //    return 0;
//    //
//    //    //  return reply_payload_index + DriverConfigurationMemory_read(data_address, requested_byte_count, (configuration_memory_buffer_t*) (&worker_msg->payload[reply_payload_index]));
//
//    return 0;
//
//}
//
//static uint16_t _read_memory_space(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count, uint8_t space) {
//
//    switch (space) {
//
//        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:
//
//            return _read_memory_space_cdi(statemachine_info, data_address, reply_payload_index, requested_byte_count);
//
//        case ADDRESS_SPACE_ALL:
//
//            return _read_memory_space_all(statemachine_info, data_address, reply_payload_index, requested_byte_count);
//
//        case ADDRESS_SPACE_CONFIGURATION_MEMORY:
//
//            return _read_memory_space_configuration_memory(statemachine_info, data_address, reply_payload_index, requested_byte_count);
//
//        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:
//
//            return _read_memory_space_acdi_manufacurer(statemachine_info, data_address, reply_payload_index, requested_byte_count);
//
//        case ADDRESS_SPACE_ACDI_USER_ACCESS:
//
//            return _read_memory_space_acdi_user(statemachine_info, data_address, reply_payload_index, requested_byte_count);
//
//        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:
//
//            return _read_memory_space_train_function_definition_info(statemachine_info, data_address, reply_payload_index, requested_byte_count);
//
//        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:
//
//            return _read_memory_space_train_function_configuration_memory(statemachine_info, data_address, reply_payload_index, requested_byte_count);
//
//        default:
//
//            return ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN;
//    }
//
//}
//
//static uint16_t _write_memory_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t requested_byte_count) {
//    //
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_write_arguments(&openlcb_node->parameters->address_space_config_memory, &config_mem_message_data);
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node);
//    //
//    //    uint16_t write_count = _interface->configuration_memory_write(data_address, requested_byte_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));
//    //
//    //    if (_interface->on_config_mem_write) {
//    //
//    //        _interface->on_config_mem_write(data_address, requested_byte_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));
//    //
//    //    }
//    //
//    //    return write_count;
//
//    return 0;
//}
//
//static uint16_t _write_memory_space_acdi_user(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t data_count) {
//    //
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_write_arguments(&openlcb_node->parameters->address_space_acdi_user, &config_mem_message_data);
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    data_address = data_address - 1; // ACDI addresses are shifted to the right one for the Version byte
//    //
//    //    // ADCI spaces are always mapped referenced to zero so offset by where the config memory starts 
//    //    if (openlcb_node->parameters->address_space_config_memory.low_address_valid)
//    //        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;
//    //
//    //    // TODO: Should I check for and insert a terminating NULL if it is missing... 
//    //
//    //    return _write_memory_space_configuration_memory(openlcb_node, openlcb_msg, data_address, reply_payload_index, data_count);
//
//    return 0;
//
//}
//
//static uint16_t _write_memory_space_train_function_configuration_memory(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t data_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_write_arguments(&openlcb_node->parameters->address_space_traction_function_config_memory, &config_mem_message_data);
//    //
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node);
//    //
//    //    return 0;
//    //    //TODO:
//    //    //  THIS IS WRONG.... NOT CONFIGURATION MEMORY WRITE REALLY
//    //    //  return DriverConfigurationMemory_write(data_address, data_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));
//
//    return 0;
//
//}
//
//static uint16_t _write_memory_space_firmware(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t data_count) {
//
//    //    config_mem_message_data_t config_mem_message_data;
//    //
//    //    uint16_t invalid = _validate_memory_space_write_arguments(&openlcb_node->parameters->address_space_firmware, &config_mem_message_data);
//    //
//    //    if (invalid) {
//    //
//    //        return invalid;
//    //
//    //    }
//    //
//    //    //   data_address = data_address + openlcb_node->parameters->firmware_image_offset;
//    //
//    //    //    TODO: This should not be the Configuration Memory 
//    //
//    //    return 0;
//    //    //   return DriverConfigurationMemory_get_write_callback()(data_address, data_count, (configuration_memory_buffer_t*) (&openlcb_msg->payload[reply_payload_index]));
//
//    return 0;
//
//}
//
//static uint16_t _write_memory_space(openlcb_statemachine_info_t *statemachine_info, uint32_t data_address, uint16_t reply_payload_index, uint16_t data_count, uint8_t space) {
//
//    switch (space) {
//
//        case ADDRESS_SPACE_CONFIGURATION_MEMORY:
//
//            return _write_memory_space_configuration_memory(statemachine_info, data_address, reply_payload_index, data_count);
//
//        case ADDRESS_SPACE_ACDI_USER_ACCESS:
//
//            return _write_memory_space_acdi_user(statemachine_info, data_address, reply_payload_index, data_count);
//
//        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:
//
//            return _write_memory_space_train_function_configuration_memory(statemachine_info, data_address, reply_payload_index, data_count);
//
//        case ADDRESS_SPACE_FIRMWARE:
//
//            return _write_memory_space_firmware(statemachine_info, data_address, reply_payload_index, data_count);
//
//        default:
//
//            return ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN;
//    }
//
//}
//
//
//// Memory read handlers
//
//void ProtocolDatagramConfigMemHandler_handle_memory_read_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, DATAGRAM_OK_REPLY_PENDING);
//
//        return;
//
//    }
//
//    uint16_t data_count = (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) & 0x7F; // top bit reserved
//    uint16_t reply_payload_index = 6;
//    uint32_t data_address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->source_id,
//            MTI_DATAGRAM,
//            0);
//
//    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[1] == DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6) {
//
//        reply_payload_index = 7;
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[6] = space;
//        data_count = (*statemachine_info->incoming_msg_info.msg_ptr->payload[7]) & 0x7F; // top bit reserved
//
//    }
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
//    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, data_address, 2);
//
//    uint16_t read_result_or_error_code = _read_memory_space(statemachine_info, data_address, reply_payload_index, data_count, space);
//
//    if (read_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = return_msg_ok; // read_result is the current payload index in this case
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = read_result_or_error_code;
//
//    } else {
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = return_msg_fail;
//        OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, read_result_or_error_code, reply_payload_index); // read_result is the error code in this case
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = reply_payload_index + 2;
//
//    }
//
//    ProtocolDatagramConfigMemHandler_try_transmit(statemachine_info);
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_read_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, DATAGRAM_OK_REPLY_PENDING);
//
//        return;
//
//    }
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_read_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, DATAGRAM_OK_REPLY_PENDING);
//
//        return;
//
//    }
//
//}
//
//
//// Memory write handlers
//
//void ProtocolDatagramConfigMemHandler_handle_memory_write_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, DATAGRAM_OK_REPLY_PENDING);
//
//        return;
//
//    }
//
//    uint16_t data_count = statemachine_info->incoming_msg_info.msg_ptr->payload_count - 6;
//    uint16_t payload_index = 6;
//    uint32_t data_address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->source_id,
//            MTI_DATAGRAM,
//            0);
//
//    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[1] == DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6) {
//
//        payload_index = 7;
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[6] = space;
//        data_count = statemachine_info->incoming_msg_info.msg_ptr->payload_count - 7;
//
//    }
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
//    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, data_address, 2);
//
//    uint16_t write_result_or_error_code = _write_memory_space(statemachine_info, data_address, payload_index, data_count, space);
//
//    if (write_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = return_msg_ok; // write_result is the current payload index in this case
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = payload_index;
//
//    } else {
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = return_msg_fail;
//        OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, write_result_or_error_code, payload_index); // write_result is the error code in this case
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = payload_index + 2;
//
//    }
//
//    ProtocolDatagramConfigMemHandler_try_transmit(statemachine_info);
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_write_under_mask_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {
//
//    uint8_t reply_payload_index = 6;
//    uint32_t data_address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->source_id,
//            MTI_DATAGRAM, 0);
//
//    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[1] == DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6) {
//
//        reply_payload_index = 7;
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[6] = space;
//
//    }
//
//    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, data_address, 2);
//
//    uint16_t data_count = (statemachine_info->incoming_msg_info.msg_ptr->payload_count - reply_payload_index);
//
//    if ((data_count == 0) || (statemachine_info->incoming_msg_info.msg_ptr->payload_count % 2 != 0)) {
//
//        ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(statemachine_info, ERROR_PERMANENT_INVALID_ARGUMENTS);
//
//        //     return;
//
//    }
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, DATAGRAM_OK_REPLY_PENDING);
//
//        //     return;
//
//    }
//
//    // Use the worker messages payload as a buffer to read/write in the existing values
//    uint16_t _result_or_error_code = _read_memory_space(statemachine_info, data_address, reply_payload_index, data_count, space);
//
//    if (_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {
//
//        uint16_t data_pair_index = reply_payload_index;
//        uint16_t data_index = reply_payload_index;
//
//        while (data_pair_index < (reply_payload_index + data_count)) {
//
//            // (data_byte & data_mask) | (existing_byte & ~data_mask)
//            *statemachine_info->outgoing_msg_info.msg_ptr->payload[data_index] =
//                    (*statemachine_info->incoming_msg_info.msg_ptr->payload[data_pair_index + 1] & *statemachine_info->incoming_msg_info.msg_ptr->payload[data_pair_index]) |
//                    (*statemachine_info->outgoing_msg_info.msg_ptr->payload[data_index] & ~(*statemachine_info->incoming_msg_info.msg_ptr->payload[data_pair_index])
//                    );
//
//            data_pair_index = data_pair_index + 2;
//            data_index = data_index + 1;
//
//        }
//
//        _result_or_error_code = _write_memory_space(statemachine_info, data_address, reply_payload_index, _result_or_error_code, space);
//
//        if (_result_or_error_code < LEN_MESSAGE_BYTES_DATAGRAM) {
//
//            *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = return_msg_ok; // read_result is the current payload index in this case
//            statemachine_info->outgoing_msg_info.msg_ptr->payload_count = reply_payload_index;
//
//
//        } else {
//
//            *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = return_msg_fail;
//            OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, _result_or_error_code, reply_payload_index); // read_result is the error code in this case
//            statemachine_info->outgoing_msg_info.msg_ptr->payload_count = reply_payload_index + 2;
//
//        }
//
//    } else {
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = return_msg_fail;
//        OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, _result_or_error_code, reply_payload_index); // read_result is the error code in this case
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = reply_payload_index + 2;
//
//    }
//
//    ProtocolDatagramConfigMemHandler_try_transmit(statemachine_info);
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_write_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, 0);
//
//        return;
//
//    }
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_write_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, 0);
//
//        return;
//
//    }
//
//}
//
//// Other memory access handlers
//
//void ProtocolDatagramConfigMemHandler_handle_memory_options_cmd_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, DATAGRAM_OK_REPLY_PENDING);
//
//        return;
//
//    }
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->source_id,
//            MTI_DATAGRAM, 0);
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_OPTIONS_REPLY;
//
//    uint16_t available_commands = 0x00;
//
//    if (statemachine_info->openlcb_node->parameters->configuration_options.write_under_mask_supported) {
//
//        available_commands = available_commands | 0x8000;
//
//    }
//
//    if (statemachine_info->openlcb_node->parameters->configuration_options.unaligned_reads_supported) {
//
//        available_commands = available_commands | 0x4000;
//
//    }
//    if (statemachine_info->openlcb_node->parameters->configuration_options.unaligned_writes_supported) {
//
//        available_commands = available_commands | 0x2000;
//
//    }
//
//    if (statemachine_info->openlcb_node->parameters->configuration_options.read_from_manufacturer_space_0xfc_supported) {
//
//        available_commands = available_commands | 0x0800;
//
//    }
//
//    if (statemachine_info->openlcb_node->parameters->configuration_options.read_from_user_space_0xfb_supported) {
//
//        available_commands = available_commands | 0x0400;
//
//    }
//
//    if (statemachine_info->openlcb_node->parameters->configuration_options.write_to_user_space_0xfb_supported) {
//
//        available_commands = available_commands | 0x0200;
//
//    }
//
//    if (statemachine_info->openlcb_node->parameters->configuration_options.stream_read_write_supported) {
//
//        available_commands = available_commands | 0x0001;
//
//    }
//
//    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, available_commands, 2);
//
//    uint8_t write_lengths = 0x80 | 0x40 | 0x020 | 0x02;
//
//    if (statemachine_info->openlcb_node->parameters->configuration_options.stream_read_write_supported) {
//
//        write_lengths = write_lengths | 0x01;
//
//    }
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[4] = write_lengths;
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[5] = statemachine_info->openlcb_node->parameters->configuration_options.high_address_space;
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[6] = statemachine_info->openlcb_node->parameters->configuration_options.low_address_space;
//
//    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 7;
//
//    if (statemachine_info->openlcb_node->parameters->configuration_options.description[0] != 0x00)
//
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = statemachine_info->outgoing_msg_info.msg_ptr->payload_count + OpenLcbUtilities_copy_string_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, statemachine_info->openlcb_node->parameters->configuration_options.description, statemachine_info->outgoing_msg_info.msg_ptr->payload_count);
//
//
//    ProtocolDatagramConfigMemHandler_try_transmit(statemachine_info);
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_options_reply_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, 0);
//
//        //     return;
//
//    }
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_get_address_space_info_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, DATAGRAM_OK_REPLY_PENDING);
//
//        //      return;
//
//    }
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->source_id,
//            MTI_DATAGRAM,
//            0);
//
//    const user_address_space_info_t* target_space = _decode_to_space_definition(statemachine_info->openlcb_node, statemachine_info->incoming_msg_info.msg_ptr);
//    uint8_t invalid_space;
//
//    if (target_space) {
//
//        invalid_space = (!target_space->present);
//
//    } else {
//
//        invalid_space = true;
//
//    }
//
//
//    if (invalid_space) {
//
//        OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_NOT_PRESENT;
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[2] = *statemachine_info->incoming_msg_info.msg_ptr->payload[2];
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[7] = 0x01;
//
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;
//
//        //       return;
//
//    }
//
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_PRESENT;
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[2] = target_space->address_space;
//    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, target_space->highest_address, 3);
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[7] = 0x00;
//
//    if (target_space->read_only) {
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[7] = *statemachine_info->outgoing_msg_info.msg_ptr->payload[7] | 0x01;
//
//    }
//
//    if (target_space->low_address_valid) {
//
//        *statemachine_info->outgoing_msg_info.msg_ptr->payload[7] = *statemachine_info->outgoing_msg_info.msg_ptr->payload[7] | 0x02;
//        OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, target_space->low_address, 8);
//
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 12;
//
//    } else {
//
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;
//
//    }
//
//    if (target_space->description[0] != 0x00) {
//
//        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = statemachine_info->outgoing_msg_info.msg_ptr->payload_count + OpenLcbUtilities_copy_string_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, target_space->description, statemachine_info->outgoing_msg_info.msg_ptr->payload_count);
//    }
//
//    ProtocolDatagramConfigMemHandler_try_transmit(statemachine_info);
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_get_address_space_info_reply_not_present_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, 0);
//
//        //      return;
//
//    }
//
//}
//
//void ProtocolDatagramHandlers_handle_memory_get_address_space_info_reply_present_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, 0);
//
//        //    return;
//
//    }
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_reserve_lock_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, DATAGRAM_OK_REPLY_PENDING);
//
//        //      return;
//
//    }
//
//    node_id_t new_node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 2);
//
//    if (statemachine_info->openlcb_node->lock_node == 0) {
//
//        statemachine_info->openlcb_node->lock_node = new_node_id;
//
//    } else {
//
//        if (new_node_id == 0)
//
//            statemachine_info->openlcb_node->lock_node = 0;
//
//    }
//
//    OpenLcbUtilities_load_openlcb_message(
//            statemachine_info->outgoing_msg_info.msg_ptr,
//            statemachine_info->openlcb_node->alias,
//            statemachine_info->openlcb_node->id,
//            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
//            statemachine_info->incoming_msg_info.msg_ptr->source_id,
//            MTI_DATAGRAM,
//            8);
//
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
//    *statemachine_info->outgoing_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_RESERVE_LOCK_REPLY;
//    OpenLcbUtilities_copy_node_id_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, statemachine_info->openlcb_node->lock_node, 2);
//
//    ProtocolDatagramConfigMemHandler_try_transmit(statemachine_info);
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_get_unique_id_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED);
//
//        return;
//
//    }
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_unfreeze_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[2] == ADDRESS_SPACE_FIRMWARE) {
//
//        if (statemachine_info->openlcb_node->parameters->address_space_firmware.present) {
//
//            if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//                _send_datagram_ack_reply(statemachine_info, 0);
//                
//                statemachine_info->incoming_msg_info.enumerate = true; // recall after ACK 
//                statemachine_info->outgoing_msg_info.valid = true;
//
//                return;
//
//            } else {
//
//
//
//                if (_interface->on_config_mem_unfreeze_firmware_update) {
//
//                    _interface->on_config_mem_unfreeze_firmware_update(statemachine_info);
//
//                }
//
//                statemachine_info->incoming_msg_info.enumerate = false; // reset after ACK and any reply
//                statemachine_info->outgoing_msg_info.valid = false;
//
//                return;
//
//            }
//
//        }
//
//    }
//
//    ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);
//
//    statemachine_info->incoming_msg_info.enumerate = false; // reset after ACK and any reply
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_freeze_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[2] == ADDRESS_SPACE_FIRMWARE) {
//
//        if (statemachine_info->openlcb_node->parameters->address_space_firmware.present) {
//
//            if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//                _send_datagram_ack_reply(statemachine_info, 0);
//
//                return;
//
//            } else {
//
//
//                if (_interface->on_config_mem_freeze_firmware_update) {
//
//                    _interface->on_config_mem_freeze_firmware_update(statemachine_info);
//
//                }
//
//                statemachine_info->incoming_msg_info.enumerate = false; // reset after ACK and any reply
//                statemachine_info->outgoing_msg_info.valid = false;
//
//                return;
//
//            }
//
//        }
//
//    }
//
//
//    ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);
//
//    statemachine_info->incoming_msg_info.enumerate = false; // reset after ACK and any reply
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_update_complete_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, 0);
//
//        return;
//
//    }
//
//    statemachine_info->incoming_msg_info.enumerate = false; // reset after ACK and any reply
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_reset_reboot_message(openlcb_statemachine_info_t *statemachine_info) {
//
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, 0);
//
//        return;
//
//    }
//
//    if (_interface->reboot) {
//
//        _interface->reboot();
//
//    }
//
//    statemachine_info->incoming_msg_info.enumerate = false; // reset after ACK and any reply
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
//
//void ProtocolDatagramConfigMemHandler_handle_memory_factory_reset_message(openlcb_statemachine_info_t *statemachine_info) {
//
//    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {
//
//        _send_datagram_ack_reply(statemachine_info, 0);
//
//        return;
//
//    }
//
//    if (_interface->on_configuration_memory_factory_reset) {
//
//        _interface->on_configuration_memory_factory_reset();
//
//    }
//
//    statemachine_info->incoming_msg_info.enumerate = false; // reset after ACK and any reply
//    statemachine_info->outgoing_msg_info.valid = false;
//
//}
