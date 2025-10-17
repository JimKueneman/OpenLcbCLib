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

#include "protocol_datagram_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"


typedef void(*memory_handler_t)(openlcb_statemachine_info_t *statemachine_info);

static interface_protocol_datagram_handler_t *_interface;

void ProtocolDatagramHandler_initialize(const interface_protocol_datagram_handler_t *interface_protocol_datagram_handler) {

    _interface = (interface_protocol_datagram_handler_t*) interface_protocol_datagram_handler;

}

static void _load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code) {

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.openlcb_msg, statemachine_info->openlcb_node->alias, statemachine_info->openlcb_node->id, statemachine_info->incoming_msg_info.openlcb_msg->source_alias, statemachine_info->incoming_msg_info.openlcb_msg->source_id, MTI_DATAGRAM_OK_REPLY, 2);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.openlcb_msg, return_code, 0);

}

static void _load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code) {

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.openlcb_msg, statemachine_info->openlcb_node->alias, statemachine_info->openlcb_node->id, statemachine_info->incoming_msg_info.openlcb_msg->source_alias, statemachine_info->incoming_msg_info.openlcb_msg->source_id, MTI_DATAGRAM_REJECTED_REPLY, 2);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.openlcb_msg, return_code, 0);

}

static void _handle_subcommand(openlcb_statemachine_info_t *statemachine_info, memory_handler_t handler_ptr) {

    // TODO: NEED TO THINK ABOUT WITH MULTIPLE NODES THAT CARRY DIFFERNT NODE PARAMETER STRUCTURES THIS WON'T WORK CORRECT.... NEED TO FIGURE THIS OUT... THE 
    // HANDLER NEEDS TO LOOK AT THE NODE PARAMETERS AND DO THE RIGHT THING I BELIEVE... THIS MODULE CAN'T MAKE THOSE DECISIONS ABOUT ACK/NACK REPLYIES TO THE DATAGRAM MESSAGES

    if (handler_ptr) {

        if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

            // TODO: HOW TO HANDLE RETURNING DELAYED REPLY AND EXPECTED TIME TILL REPLY COMES

            _load_datagram_received_ok_message(statemachine_info, 0x00);

            return; // keep pumping this message

        }
        
        handler_ptr(statemachine_info);

        return;

    } else {

        _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

        return;

    }

}

static void _handle_read_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[6]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_message);

            break;

        case ADDRESS_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_message);

            break;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_message);

            break;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer_message);

            break;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_definition_info_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_config_memory_message);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

static void _handle_read_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[6]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_ok_message);

            break;

        case ADDRESS_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_ok_message);

            break;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_ok_message);

            break;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer_reply_ok_message);

            break;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user_reply_ok_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_definition_info_reply_ok_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_config_memory_reply_ok_message);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

static void _handle_read_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[6]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_fail_message);

            break;

        case ADDRESS_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_fail_message);

            break;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_fail_message);

            break;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer_reply_fail_message);

            break;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user_reply_fail_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_definition_info_reply_fail_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_config_memory_reply_fail_message);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

static void _handle_write_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[6]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_message);

            break;

        case ADDRESS_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_message);

            break;
        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_message);

            break;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer_message);

            break;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_definition_info_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_config_memory_message);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

static void _handle_write_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[6]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_ok_message);

            break;

        case ADDRESS_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_ok_message);

            break;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_ok_message);

            break;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer_reply_ok_message);

            break;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user_reply_ok_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_definition_info_reply_ok_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_config_memory_reply_ok_message);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

static void _handle_write_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[6]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_fail_message);

            break;

        case ADDRESS_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_fail_message);

            break;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_fail_message);

            break;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer_reply_fail_message);

            break;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user_reply_fail_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_definition_info_reply_fail_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_config_memory_reply_fail_message);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

static void _handle_write_under_mask_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[6]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_config_description_info_message);

            break;

        case ADDRESS_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_all_message);

            break;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_configuration_memory_message);

            break;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_acdi_manufacturer_message);

            break;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_acdi_user_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_traction_function_definition_info_message);

            break;

        case ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_traction_function_config_memory_message);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

static void _handle_datagram_memory_configuration(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[1]) { // which space?

        case DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6:

            _handle_read_address_space_at_offset_6(statemachine_info);

            break;

        case DATAGRAM_MEMORY_READ_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_message);

            break;

        case DATAGRAM_MEMORY_READ_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_message);

            break;

        case DATAGRAM_MEMORY_READ_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_message);

            break;

        case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_read_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_ok_message);

            break;

        case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_ok_message);

            break;

        case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_ok_message);

            break;

        case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_read_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_fail_message);

            break;

        case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_fail_message);

            break;

        case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_fail_message);

            break;

        case DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6:

            _handle_write_address_space_at_offset_6(statemachine_info);

            break;

        case DATAGRAM_MEMORY_WRITE_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_message);

            break;

        case DATAGRAM_MEMORY_WRITE_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_message);

            break;

        case DATAGRAM_MEMORY_WRITE_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_message);

            break;

        case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_write_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_ok_message);

            break;

        case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_ok_message);

            break;

        case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_ok_message);

            break;

        case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_write_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_fail_message);

            break;

        case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_fail_message);

            break;

        case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_fail_message);

            break;

        case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6:

            _handle_write_under_mask_address_space_at_offset_6(statemachine_info);

            break;

        case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_configuration_memory_message);

            break;

        case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_all_message);

            break;

        case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_config_description_info_message);

            break;

        case DATAGRAM_MEMORY_OPTIONS_CMD:

            _handle_subcommand(statemachine_info, _interface->memory_options_cmd_message);

            break;

        case DATAGRAM_MEMORY_OPTIONS_REPLY:

            _handle_subcommand(statemachine_info, _interface->memory_options_reply_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_INFO_CMD:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_NOT_PRESENT:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info_reply_not_present_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_PRESENT:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info_reply_not_present_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_RESERVE_LOCK:

            _handle_subcommand(statemachine_info, _interface->memory_reserve_lock_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_GET_UNIQUE_ID:

            _handle_subcommand(statemachine_info, _interface->memory_get_unique_id_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_UNFREEZE:

            _handle_subcommand(statemachine_info, _interface->memory_unfreeze_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_FREEZE:

            _handle_subcommand(statemachine_info, _interface->memory_freeze_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_UPDATE_COMPLETE:

            _handle_subcommand(statemachine_info, _interface->memory_update_complete_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_RESET_REBOOT:

            _handle_subcommand(statemachine_info, _interface->memory_reset_reboot_message);

            break;

        case DATAGRAM_MEMORY_CONFIGURATION_FACTORY_RESET:

            _handle_subcommand(statemachine_info, _interface->memory_factory_reset_message);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    } // switch sub-command

}

void ProtocolDatagramHandler_handle_datagram(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.openlcb_msg->payload[0]) { // commands

        case DATAGRAM_MEMORY_CONFIGURATION: // are we 0x20?

            _handle_datagram_memory_configuration(statemachine_info);

            break;

        default:

            _load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_COMMAND_UNKNOWN);

            break;

    } // switch command


}

void Protocol_DatagramHandler_handle_datagram_received_ok(openlcb_statemachine_info_t *statemachine_info) {

    if (statemachine_info->openlcb_node->last_received_datagram) {

        OpenLcbBufferStore_free_buffer(statemachine_info->openlcb_node->last_received_datagram);
        statemachine_info->openlcb_node->last_received_datagram = NULL;

    }

    statemachine_info-> openlcb_node->state.resend_datagram = false;

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolDatagramHandler_handle_datagram_rejected(openlcb_statemachine_info_t *statemachine_info) {

    if (OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->incoming_msg_info.openlcb_msg, 0) && ERROR_TEMPORARY == ERROR_TEMPORARY) {

        if (statemachine_info->openlcb_node->last_received_datagram) {

            statemachine_info->openlcb_node->state.resend_datagram = true;

        }

    } else {

        OpenLcbBufferStore_free_buffer(statemachine_info->openlcb_node->last_received_datagram);
        statemachine_info->openlcb_node->last_received_datagram = NULL;

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolDatagramHandler_100ms_timer_tick(void) {


}






