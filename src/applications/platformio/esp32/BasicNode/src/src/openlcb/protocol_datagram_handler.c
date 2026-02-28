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
 * @file protocol_datagram_handler.c
 * @brief Datagram protocol handler — reliable 0–72 byte addressed transfers.
 *
 * @details Dispatches incoming datagrams to per-address-space callbacks for
 * read, write, write-under-mask, and stream variants (both datagram- and
 * stream-transport).  Callback-based: the application populates an
 * @ref interface_protocol_datagram_handler_t with handler pointers; NULL
 * pointers cause automatic rejection with SUBCOMMAND_UNKNOWN.
 *
 * Also handles Datagram Received OK / Rejected replies including retry
 * logic for temporary errors.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 *
 * @see protocol_datagram_handler.h
 * @see DatagramTransportS.pdf
 * @see MemoryConfigurationS.pdf
 */

#include "protocol_datagram_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"


    /** @brief Stored callback interface pointer; set by _initialize(). */
static interface_protocol_datagram_handler_t *_interface;

    /**
     * @brief Stores the callback interface.  Call once at startup.
     *
     * @details Algorithm:
     * -# Cast away const and store the pointer in module-level static
     *
     * @verbatim
     * @param interface_protocol_datagram_handler  Populated callback table.
     * @endverbatim
     *
     * @warning Structure must remain valid for application lifetime.
     */
void ProtocolDatagramHandler_initialize(const interface_protocol_datagram_handler_t *interface_protocol_datagram_handler) {

    _interface = (interface_protocol_datagram_handler_t *) interface_protocol_datagram_handler;

}

    /**
     * @brief Invoke handler_ptr, or auto-reject if NULL.
     *
     * @details Algorithm:
     * -# If handler_ptr is NULL, send SUBCOMMAND_UNKNOWN rejection
     * -# Otherwise call handler_ptr(statemachine_info)
     *
     * @verbatim
     * @param statemachine_info  Current state-machine context.
     * @param handler_ptr        Callback, or NULL for auto-reject.
     * @endverbatim
     */
static void _handle_subcommand(openlcb_statemachine_info_t *statemachine_info, memory_handler_t handler_ptr) {

    if (!handler_ptr) {

        ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

        return;

    }

    handler_ptr(statemachine_info);

}

    /** @brief Dispatch datagram read request by address space (payload[6]). */
static void _handle_read_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_train_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_train_function_config_memory);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch datagram read-reply-OK by address space (payload[6]). */
static void _handle_read_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_ok);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_train_function_definition_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_train_function_config_memory_reply_ok);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch datagram read-reply-FAIL by address space (payload[6]). */
static void _handle_read_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_fail);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_train_function_definition_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_train_function_config_memory_reply_fail);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch stream read request by address space (payload[6]). */
static void _handle_read_stream_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_train_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_train_function_config_memory);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch stream read-reply-OK by address space (payload[6]). */
static void _handle_read_stream_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all_reply_ok);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_manufacturer_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_user_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_train_function_definition_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_train_function_config_memory_reply_ok);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch stream read-reply-FAIL by address space (payload[6]). */
static void _handle_read_stream_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all_reply_fail);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_manufacturer_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_user_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_train_function_definition_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_train_function_config_memory_reply_fail);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch datagram write request by address space (payload[6]). */
static void _handle_write_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all);

            break;
        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_train_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_train_function_config_memory);

            break;

        case CONFIG_MEM_SPACE_FIRMWARE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_firmware_upgrade);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch datagram write-reply-OK by address space (payload[6]). */
static void _handle_write_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_ok);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_train_function_definition_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_train_function_config_memory_reply_ok);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch datagram write-reply-FAIL by address space (payload[6]). */
static void _handle_write_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_fail);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_train_function_definition_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_train_function_config_memory_reply_fail);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch stream write request by address space (payload[6]). */
static void _handle_write_stream_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all);

            break;
        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_train_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_train_function_config_memory);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch stream write-reply-OK by address space (payload[6]). */
static void _handle_write_stream_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all_reply_ok);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_manufacturer_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_user_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_train_function_definition_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_train_function_config_memory_reply_ok);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch stream write-reply-FAIL by address space (payload[6]). */
static void _handle_write_stream_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all_reply_fail);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_manufacturer_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_user_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_train_function_definition_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_train_function_config_memory_reply_fail);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /** @brief Dispatch write-under-mask request by address space (payload[6]). */
static void _handle_write_under_mask_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_all);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_train_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_train_function_config_memory);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
     * @brief Top-level dispatcher for all memory-configuration subcommands.
     *
     * @details Algorithm:
     * -# Switch on payload[1] (subcommand byte)
     * -# Shorthand codes (FD/FE/FF) dispatch directly to the handler
     * -# "space in byte 6" codes delegate to the _handle_*_at_offset_6 helpers
     * -# Config commands (options, lock, freeze, reset, etc.) dispatch directly
     * -# Unknown subcommands are rejected
     *
     * @verbatim
     * @param statemachine_info  Context with incoming 0x20 datagram.
     * @endverbatim
     */
static void _handle_datagram_memory_configuration_command(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[1]) { // which space?

        case CONFIG_MEM_READ_SPACE_IN_BYTE_6:

            _handle_read_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory);

            break;

        case CONFIG_MEM_READ_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all);

            break;

        case CONFIG_MEM_READ_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info);

            break;

        case CONFIG_MEM_READ_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_read_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_READ_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_ok);

            break;

        case CONFIG_MEM_READ_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_READ_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_read_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_READ_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_fail);

            break;

        case CONFIG_MEM_READ_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6:

            _handle_read_stream_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_STREAM_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory);

            break;

        case CONFIG_MEM_READ_STREAM_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all);

            break;

        case CONFIG_MEM_READ_STREAM_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_read_stream_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all_reply_ok);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_read_stream_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all_reply_fail);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_WRITE_SPACE_IN_BYTE_6:

            _handle_write_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory);

            break;

        case CONFIG_MEM_WRITE_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all);

            break;

        case CONFIG_MEM_WRITE_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info);

            break;

        case CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_write_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_WRITE_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_ok);

            break;

        case CONFIG_MEM_WRITE_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_write_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_fail);

            break;

        case CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_WRITE_UNDER_MASK_SPACE_IN_BYTE_6:

            _handle_write_under_mask_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_configuration_memory);

            break;

        case CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_all);

            break;

        case CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_config_description_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_SPACE_IN_BYTE_6:

            _handle_write_stream_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory);

            break;

        case CONFIG_MEM_WRITE_STREAM_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all);

            break;

        case CONFIG_MEM_WRITE_STREAM_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_write_stream_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all_reply_ok);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_write_stream_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all_reply_fail);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_OPTIONS_CMD:

            _handle_subcommand(statemachine_info, _interface->memory_options_cmd);

            break;

        case CONFIG_MEM_OPTIONS_REPLY:

            _handle_subcommand(statemachine_info, _interface->memory_options_reply);

            break;

        case CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info);

            break;

        case CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info_reply_not_present);

            break;

        case CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info_reply_present);

            break;

        case CONFIG_MEM_RESERVE_LOCK:

            _handle_subcommand(statemachine_info, _interface->memory_reserve_lock);

            break;

        case CONFIG_MEM_RESERVE_LOCK_REPLY:

            _handle_subcommand(statemachine_info, _interface->memory_reserve_lock_reply);

            break;

        case CONFIG_MEM_GET_UNIQUE_ID:

            _handle_subcommand(statemachine_info, _interface->memory_get_unique_id);

            break;


        case CONFIG_MEM_GET_UNIQUE_ID_REPLY:

            _handle_subcommand(statemachine_info, _interface->memory_get_unique_id_reply);

            break;


        case CONFIG_MEM_UNFREEZE:

            _handle_subcommand(statemachine_info, _interface->memory_unfreeze);

            break;

        case CONFIG_MEM_FREEZE:

            _handle_subcommand(statemachine_info, _interface->memory_freeze);

            break;

        case CONFIG_MEM_UPDATE_COMPLETE:

            _handle_subcommand(statemachine_info, _interface->memory_update_complete);

            break;

        case CONFIG_MEM_RESET_REBOOT:

            _handle_subcommand(statemachine_info, _interface->memory_reset_reboot);

            break;

        case CONFIG_MEM_FACTORY_RESET:

            _handle_subcommand(statemachine_info, _interface->memory_factory_reset);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    } // switch sub-command

}

    /**
     * @brief Main entry point — switches on payload[0] command byte.
     *
     * @details Algorithm:
     * -# If payload[0] == CONFIG_MEM_CONFIGURATION (0x20), delegate to
     *    _handle_datagram_memory_configuration_command()
     * -# Otherwise reject with COMMAND_UNKNOWN
     *
     * @verbatim
     * @param statemachine_info  Context with the received datagram.
     * @endverbatim
     */
void ProtocolDatagramHandler_datagram(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[0]) { // commands

        case CONFIG_MEM_CONFIGURATION: // are we 0x20?

            _handle_datagram_memory_configuration_command(statemachine_info);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_COMMAND_UNKNOWN);

            break;

    } // switch command


}

    /**
     * @brief Build a Datagram Received OK message (MTI 0x0A28).
     *
     * @details Algorithm:
     * -# Convert reply_pending_time_in_seconds to a 4-bit power-of-2 exponent
     *    (0 = no reply pending, 1 = 2 s, 2 = 4 s, … 15 = 32768 s)
     * -# Build MTI_DATAGRAM_OK_REPLY addressed back to the sender
     * -# Store DATAGRAM_OK_REPLY_PENDING | exponent in payload[0]
     * -# Mark outgoing message valid
     *
     * @verbatim
     * @param statemachine_info               Current context.
     * @param reply_pending_time_in_seconds   0 for simple OK, else seconds until
     *                                        reply (rounded up to 2^N).
     * @endverbatim
     */
void ProtocolDatagramHandler_load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds) {

    uint8_t exponent = 0;

    if (reply_pending_time_in_seconds > 0) {

            if (reply_pending_time_in_seconds <= 2) {

                exponent = 1;

            } else if (reply_pending_time_in_seconds <= 4) {

                exponent = 2;

            } else if (reply_pending_time_in_seconds <= 8) {

                exponent = 3;

            } else if (reply_pending_time_in_seconds <= 16) {

                exponent = 4;

            } else if (reply_pending_time_in_seconds <= 32) {

                exponent = 5;

            } else if (reply_pending_time_in_seconds <= 64) {

                exponent = 6;

            } else if (reply_pending_time_in_seconds <= 128) {

                exponent = 7;

            } else if (reply_pending_time_in_seconds <= 256) {

                exponent = 8;

            } else if (reply_pending_time_in_seconds <= 512) {

                exponent = 9;

            } else if (reply_pending_time_in_seconds <= 1024) {

                exponent = 0x0A;

            } else if (reply_pending_time_in_seconds <= 2048) {

                exponent = 0x0B;

            } else if (reply_pending_time_in_seconds <= 4096) {

                exponent = 0x0C;

            } else if (reply_pending_time_in_seconds <= 8192) {

                exponent = 0x0D;

            } else if (reply_pending_time_in_seconds <= 16384) {

                exponent = 0x0E;

            } else {

                exponent = 0x0F;

            }

        }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_DATAGRAM_OK_REPLY);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            DATAGRAM_OK_REPLY_PENDING | exponent,
            0);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Build a Datagram Rejected message (MTI 0x0A48).
     *
     * @details Algorithm:
     * -# Build MTI_DATAGRAM_REJECTED_REPLY addressed back to the sender
     * -# Copy 16-bit return_code into payload[0..1]
     * -# Mark outgoing message valid
     *
     * @verbatim
     * @param statemachine_info  Current context.
     * @param return_code        OpenLCB error code (0x1xxx permanent, 0x2xxx temporary).
     * @endverbatim
     */
void ProtocolDatagramHandler_load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_DATAGRAM_REJECTED_REPLY);

    OpenLcbUtilities_copy_word_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            return_code,
            0);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Handle incoming Datagram Received OK (MTI 0x0A28).
     *
     * @details Algorithm:
     * -# Clear resend flag and free the stored datagram buffer
     * -# Set outgoing_msg_info.valid = false (nothing to send)
     *
     * @verbatim
     * @param statemachine_info  Context with the received OK reply.
     * @endverbatim
     */
void ProtocolDatagramHandler_datagram_received_ok(openlcb_statemachine_info_t *statemachine_info) {

    ProtocolDatagramHandler_clear_resend_datagram_message(statemachine_info->openlcb_node);

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
     * @brief Handle incoming Datagram Rejected (MTI 0x0A48).
     *
     * @details Algorithm:
     * -# Extract error code from payload word 0
     * -# If ERROR_TEMPORARY bit set and a stored datagram exists, set
     *    resend_datagram flag for retry
     * -# If permanent error, clear resend flag and free stored buffer
     * -# Set outgoing_msg_info.valid = false
     *
     * @verbatim
     * @param statemachine_info  Context with the received rejection.
     * @endverbatim
     */
void ProtocolDatagramHandler_datagram_rejected(openlcb_statemachine_info_t *statemachine_info) {

    if ((OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0) & ERROR_TEMPORARY) == ERROR_TEMPORARY) {

        if (statemachine_info->openlcb_node->last_received_datagram) {

            statemachine_info->openlcb_node->state.resend_datagram = true;

        }

    } else {

        ProtocolDatagramHandler_clear_resend_datagram_message(statemachine_info->openlcb_node);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
     * @brief Free stored datagram and clear resend flag for a node.
     *
     * @details Algorithm:
     * -# If last_received_datagram exists, lock/free/unlock it and NULL the pointer
     * -# Clear resend_datagram flag
     *
     * @verbatim
     * @param openlcb_node  Target node.
     * @endverbatim
     *
     * @note Safe to call when no datagram is stored.
     */
void ProtocolDatagramHandler_clear_resend_datagram_message(openlcb_node_t *openlcb_node) {

    if (openlcb_node->last_received_datagram) {

        _interface->lock_shared_resources();
        OpenLcbBufferStore_free_buffer(openlcb_node->last_received_datagram);
        _interface->unlock_shared_resources();

        openlcb_node->last_received_datagram = NULL;

    }

    openlcb_node->state.resend_datagram = false;

}

    /** @brief 100 ms timer tick — placeholder for datagram timeout management. */
void ProtocolDatagramHandler_100ms_timer_tick(void) {


}

