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
     * @file protocol_config_mem_operations_handler.c
     * @brief Memory-config operations dispatcher — two-phase ACK-then-execute
     *        handling for options, address-space info, lock, freeze, reset, and
     *        factory-reset sub-commands.
     *
     * @author Jim Kueneman
     * @date 28 Feb 2026
     *
     * @see protocol_config_mem_operations_handler.h
     * @see MemoryConfigurationS.pdf
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


    /** @brief Stored callback interface pointer; set by _initialize(). */
static interface_protocol_config_mem_operations_handler_t* _interface;

    /**
     * @brief Stores the callback interface.  Call once at startup.
     *
     * @verbatim
     * @param interface_protocol_config_mem_operations_handler  Populated table.
     * @endverbatim
     *
     * @warning Structure must remain valid for application lifetime.
     */
void ProtocolConfigMemOperationsHandler_initialize(const interface_protocol_config_mem_operations_handler_t *interface_protocol_config_mem_operations_handler) {

    _interface = (interface_protocol_config_mem_operations_handler_t*) interface_protocol_config_mem_operations_handler;

}

    /**
     * @brief Map a space-ID byte to the node’s address-space definition struct.
     *
     * @details Algorithm:
     * -# Read space byte from payload[space_offset]
     * -# Switch on value (0xFF…0xEF) and return the matching struct pointer
     * -# Return NULL for unrecognised space IDs
     *
     * @verbatim
     * @param statemachine_info  Context with the incoming message.
     * @param space_offset       Payload index of the space-ID byte.
     * @endverbatim
     *
     * @return Pointer to user_address_space_info_t, or NULL.
     */
static const user_address_space_info_t* _decode_to_space_definition(openlcb_statemachine_info_t *statemachine_info, uint8_t space_offset) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[space_offset]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            return (&statemachine_info->openlcb_node->parameters->address_space_configuration_definition);

        case CONFIG_MEM_SPACE_ALL:

            return &statemachine_info->openlcb_node->parameters->address_space_all;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            return &statemachine_info->openlcb_node->parameters->address_space_config_memory;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            return &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            return &statemachine_info->openlcb_node->parameters->address_space_acdi_user;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            return &statemachine_info->openlcb_node->parameters->address_space_train_function_definition_info;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            return &statemachine_info->openlcb_node->parameters->address_space_train_function_config_memory;

        case CONFIG_MEM_SPACE_FIRMWARE:

            return &statemachine_info->openlcb_node->parameters->address_space_firmware;

        default:

            return NULL;

    }

}

    /**
     * @brief Prepare outgoing datagram header with CONFIG_MEM_CONFIGURATION byte.
     *
     * @details Algorithm:
     * -# Clear payload, load addressing, set MTI_DATAGRAM
     * -# Write CONFIG_MEM_CONFIGURATION at payload[0]
     * -# Set valid = false (caller completes payload and sets valid)
     *
     * @verbatim
     * @param statemachine_info             Context for message generation.
     * @param config_mem_read_request_info   Request info (unused but kept for API symmetry).
     * @endverbatim
     */
static void _load_config_mem_reply_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_read_request_info) {

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 0;

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_DATAGRAM);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            CONFIG_MEM_CONFIGURATION,
            0);

    statemachine_info->outgoing_msg_info.valid = false; // Assume there is not a message to send by default

}

    /** @brief Build the write-length flags byte for an options reply. */
static uint8_t _available_write_flags(openlcb_statemachine_info_t *statemachine_info) {

    uint8_t write_lengths = CONFIG_OPTIONS_WRITE_LENGTH_RESERVED;

    if (statemachine_info->openlcb_node->parameters->configuration_options.stream_read_write_supported) {

        write_lengths = write_lengths | CONFIG_OPTIONS_WRITE_LENGTH_STREAM_READ_WRITE;

    }

    return write_lengths;

}

    /** @brief Build the 16-bit available-commands flags for an options reply. */
static uint16_t _available_commands_flags(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t result = 0x0000;

    if (statemachine_info->openlcb_node->parameters->configuration_options.write_under_mask_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_WRITE_UNDER_MASK;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.unaligned_reads_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_UNALIGNED_READS;

    }
    if (statemachine_info->openlcb_node->parameters->configuration_options.unaligned_writes_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_UNALIGNED_WRITES;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.read_from_manufacturer_space_0xfc_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_ACDI_MANUFACTURER_READ;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.read_from_user_space_0xfb_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_ACDI_USER_READ;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.write_to_user_space_0xfb_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_ACDI_USER_WRITE;

    }

    return result;

}

    /** @brief Build the flags byte (read-only, low-addr-valid) for a space-info reply. */
static uint8_t _available_address_space_info_flags(config_mem_operations_request_info_t *config_mem_operations_request_info) {

    uint8_t flags = 0x0;

    if (config_mem_operations_request_info->space_info->read_only) {

        flags = flags | CONFIG_OPTIONS_SPACE_INFO_FLAG_READ_ONLY;

    }

    if (config_mem_operations_request_info->space_info->low_address_valid) {

        flags = flags | CONFIG_OPTIONS_SPACE_INFO_FLAG_USE_LOW_ADDRESS;

    }

    return flags;
}

    /** @brief Send Datagram Received OK; set flags for phase-2 re-invocation. */
static void _load_datagram_ok_message(openlcb_statemachine_info_t *statemachine_info) {

    _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
    statemachine_info->incoming_msg_info.enumerate = true; // call this again for the data

}

    /** @brief Send Datagram Received Rejected; clear flags to stop processing. */
static void _load_datagram_reject_message(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code) {

    _interface->load_datagram_received_rejected_message(statemachine_info, error_code);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // done
    statemachine_info->incoming_msg_info.enumerate = false; // done

}

    /**
     * @brief Two-phase dispatcher: phase 1 sends ACK, phase 2 calls the callback.
     *
     * @details Algorithm:
     * -# Phase 1 (ACK not yet sent): if callback registered → OK + re-invoke;
     *    otherwise → reject with NOT_IMPLEMENTED
     * -# Phase 2 (ACK sent): invoke callback, reset flags
     *
     * @verbatim
     * @param statemachine_info                   Context.
     * @param config_mem_operations_request_info   Carries the callback + space_info.
     * @endverbatim
     */
static void _handle_operations_request(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

        if (config_mem_operations_request_info->operations_func) {

            _load_datagram_ok_message(statemachine_info);

        } else {

            _load_datagram_reject_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);
        }

        return;
    }

    // Complete Command Request, if it was null the first pass with the datagram ACK check would have return NACK and with won't get called with a null
    config_mem_operations_request_info->operations_func(statemachine_info, config_mem_operations_request_info);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // reset
    statemachine_info->incoming_msg_info.enumerate = false; // done
}

    /**
     * @brief Build a Get Configuration Options reply datagram.
     *
     * @details Algorithm:
     * -# Load header, write OPTIONS_REPLY + command flags + write flags +
     *    high/low space bounds + optional description string
     * -# Mark outgoing message valid
     *
     * @verbatim
     * @param statemachine_info                   Context.
     * @param config_mem_operations_request_info   Request info.
     * @endverbatim
     */
void ProtocolConfigMemOperationsHandler_request_options_cmd(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            CONFIG_MEM_OPTIONS_REPLY,
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


    // elect to always send this optional byte
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

    /**
     * @brief Build a Get Address Space Info reply (present or not-present).
     *
     * @details Algorithm:
     * -# If space exists and is present: reply with highest address, flags,
     *    optional low address, optional description
     * -# Otherwise: reply NOT_PRESENT with padded 8-byte payload
     *
     * @verbatim
     * @param statemachine_info                   Context.
     * @param config_mem_operations_request_info   Carries space_info pointer.
     * @endverbatim
     */
void ProtocolConfigMemOperationsHandler_request_get_address_space_info(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    uint8_t description_offset = 8;

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    if (config_mem_operations_request_info->space_info) {

        if (config_mem_operations_request_info->space_info->present) {

            OpenLcbUtilities_copy_byte_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT,
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
            CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT,
            1);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[2],
            2);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8; // OpenLcbChecker needs 8

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Handle Lock/Reserve command: grant, release, or report current holder.
     *
     * @details Algorithm:
     * -# Extract requester Node ID from payload
     * -# If unlocked: grant to requester.  If locked and ID == 0: release.
     * -# Reply with LOCK_REPLY + current owner Node ID
     *
     * @verbatim
     * @param statemachine_info                   Context.
     * @param config_mem_operations_request_info   Request info.
     * @endverbatim
     */
void ProtocolConfigMemOperationsHandler_request_reserve_lock(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    node_id_t new_node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(
            statemachine_info->incoming_msg_info.msg_ptr,
            2);

    if (statemachine_info->openlcb_node->owner_node == 0) {

        statemachine_info->openlcb_node->owner_node = new_node_id;

    } else {

        if (new_node_id == 0) {

            statemachine_info->openlcb_node->owner_node = 0;

        }

    }

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            CONFIG_MEM_RESERVE_LOCK_REPLY,
            1);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->owner_node,
            2);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /** @brief Dispatch Get Configuration Options command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_options_cmd(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_options_cmd;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);
}

    /** @brief Dispatch Get Configuration Options reply to two-phase handler. */
void ProtocolConfigMemOperationsHandler_options_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_options_cmd_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Get Address Space Info command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_get_address_space_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Address Space Info Not Present reply to two-phase handler. */
void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info_reply_not_present;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Address Space Info Present reply to two-phase handler. */
void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info_reply_present;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Lock/Reserve command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_reserve_lock(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reserve_lock;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Lock/Reserve reply to two-phase handler. */
void ProtocolConfigMemOperationsHandler_reserve_lock_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reserve_lock_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);


}

    /** @brief Dispatch Get Unique ID command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_get_unique_id(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_unique_id;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Get Unique ID reply to two-phase handler. */
void ProtocolConfigMemOperationsHandler_get_unique_id_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_unique_id_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Unfreeze command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_unfreeze(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_unfreeze;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Freeze command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_freeze(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_freeze;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Update Complete command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_update_complete(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_update_complete;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Reset/Reboot command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_reset_reboot(openlcb_statemachine_info_t *statemachine_info) {

    node_id_t target_node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(
            statemachine_info->incoming_msg_info.msg_ptr, 2);

    if (target_node_id != statemachine_info->openlcb_node->id) {

        statemachine_info->outgoing_msg_info.valid = false;
        return;

    }

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reset_reboot;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /** @brief Dispatch Factory Reset command to two-phase handler. */
void ProtocolConfigMemOperationsHandler_factory_reset(openlcb_statemachine_info_t *statemachine_info) {

    node_id_t target_node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(
            statemachine_info->incoming_msg_info.msg_ptr, 2);

    if (target_node_id != statemachine_info->openlcb_node->id) {

        statemachine_info->outgoing_msg_info.valid = false;
        return;

    }

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_factory_reset;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}



