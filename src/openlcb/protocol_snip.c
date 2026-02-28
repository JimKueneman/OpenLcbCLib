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
 * @file protocol_snip.c
 * @brief Simple Node Information Protocol (SNIP) — builds and validates
 *        SNIP reply messages from node parameters and config memory.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 *
 * @see protocol_snip.h
 * @see SimpleNodeInformationS.pdf
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


    /** @brief Stored callback interface pointer; set by _initialize(). */
static interface_openlcb_protocol_snip_t *_interface;

    /**
     * @brief Stores the callback interface.  Call once at startup.
     *
     * @details Algorithm:
     * -# Cast away const and store pointer in module-level static
     *
     * @verbatim
     * @param interface_openlcb_protocol_snip  Populated callback table.
     * @endverbatim
     *
     * @warning Structure must remain valid for application lifetime.
     */
void ProtocolSnip_initialize(const interface_openlcb_protocol_snip_t *interface_openlcb_protocol_snip) {

    _interface = (interface_openlcb_protocol_snip_t*) interface_openlcb_protocol_snip;

}

    /**
     * @brief Copy a null-terminated string field into the SNIP payload.
     *
     * @details Algorithm:
     * -# Clamp string length to max_str_len − 1
     * -# If full string fits, copy + append null terminator
     * -# Otherwise copy only byte_count bytes (no terminator)
     * -# Update *payload_offset and outgoing_msg->payload_count
     *
     * @verbatim
     * @param outgoing_msg     Destination message.
     * @param payload_offset   Pointer to current offset (updated in place).
     * @param str              Source string.
     * @param max_str_len      Buffer size limit (includes null).
     * @param byte_count       Max bytes the caller wants written.
     * @endverbatim
     */
static void _process_snip_string(openlcb_msg_t* outgoing_msg, uint16_t *payload_offset, const char *str, uint16_t max_str_len, uint16_t byte_count) {

    bool result_is_null_terminated = false;
    uint16_t string_length = strlen(str);

    if (string_length > max_str_len - 1) {

        string_length = max_str_len - 1;
        result_is_null_terminated = true;

    } else {

        result_is_null_terminated = string_length <= byte_count;

    }

    if (result_is_null_terminated) {

        memcpy(&outgoing_msg->payload[*payload_offset], str, string_length);
        *payload_offset = *payload_offset + string_length;
        *outgoing_msg->payload[*payload_offset] = 0x00;
        (*payload_offset)++;
        outgoing_msg->payload_count += (string_length + 1);

    } else {

        memcpy(&outgoing_msg->payload[*payload_offset], str, byte_count);
        *payload_offset = *payload_offset + byte_count;
        outgoing_msg->payload_count += byte_count;

    }

}

    /**
     * @brief Write a single version-ID byte into the SNIP payload.
     *
     * @details Algorithm:
     * -# Store version byte at *payload_data_offset, bump offset and count
     *
     * @verbatim
     * @param outgoing_msg         Destination message.
     * @param payload_data_offset  Pointer to current offset (updated).
     * @param version              Version byte to write.
     * @endverbatim
     *
     * @return Updated offset.
     */
static uint16_t _process_snip_version(openlcb_msg_t* outgoing_msg, uint16_t *payload_data_offset, const uint8_t version) {

    *outgoing_msg->payload[*payload_data_offset] = version;
    outgoing_msg->payload_count++;
    (*payload_data_offset)++;

    return *payload_data_offset;

}

    /** @brief Copy mfg version byte into payload.  Returns updated offset. */
uint16_t ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    if (requested_bytes > 0) {

        return _process_snip_version(outgoing_msg, &offset, openlcb_node->parameters->snip.mfg_version);

    }

    return 0;

}

    /** @brief Copy manufacturer name string into payload.  Returns updated offset. */
uint16_t ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    _process_snip_string(outgoing_msg, &offset, openlcb_node->parameters->snip.name, LEN_SNIP_NAME_BUFFER, requested_bytes);

    return offset;

}

    /** @brief Copy model string into payload.  Returns updated offset. */
uint16_t ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    _process_snip_string(outgoing_msg, &offset, openlcb_node->parameters->snip.model, LEN_SNIP_MODEL_BUFFER, requested_bytes);

    return offset;

}

    /** @brief Copy hardware version string into payload.  Returns updated offset. */
uint16_t ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    _process_snip_string(outgoing_msg, &offset, openlcb_node->parameters->snip.hardware_version, LEN_SNIP_HARDWARE_VERSION_BUFFER, requested_bytes);

    return offset;

}

    /** @brief Copy software version string into payload.  Returns updated offset. */
uint16_t ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    _process_snip_string(outgoing_msg, &offset, openlcb_node->parameters->snip.software_version, LEN_SNIP_SOFTWARE_VERSION_BUFFER, requested_bytes);

    return offset;

}

    /** @brief Copy user version byte into payload.  Returns updated offset. */
uint16_t ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    if (requested_bytes > 0) {

        return _process_snip_version(outgoing_msg, &offset, openlcb_node->parameters->snip.user_version);

    }

    return 0;

}

    /**
     * @brief Read user name from config memory and copy into payload.
     *
     * @details Algorithm:
     * -# Compute config-memory address (base + low_address offset if valid)
     * -# Read via config_memory_read callback
     * -# Copy into payload via _process_snip_string
     *
     * @verbatim
     * @param openlcb_node      Source node.
     * @param outgoing_msg      Destination message.
     * @param offset            Current payload offset.
     * @param requested_bytes   Max bytes to copy.
     * @endverbatim
     *
     * @return Updated offset.
     */
uint16_t ProtocolSnip_load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {


    configuration_memory_buffer_t configuration_memory_buffer;
    uint32_t data_address = USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS; // User Name is by default the first 63 Bytes in the Configuration Space

    if (openlcb_node->parameters->address_space_config_memory.low_address_valid) {

        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    }

    _interface->config_memory_read(openlcb_node, data_address, requested_bytes, &configuration_memory_buffer);

    _process_snip_string(outgoing_msg, &offset, (char*) (&configuration_memory_buffer[0]), LEN_SNIP_USER_NAME_BUFFER, requested_bytes);

    return offset;

}

    /**
     * @brief Read user description from config memory and copy into payload.
     *
     * @details Algorithm:
     * -# Compute config-memory address (base + low_address offset if valid)
     * -# Read via config_memory_read callback
     * -# Copy into payload via _process_snip_string
     *
     * @verbatim
     * @param openlcb_node      Source node.
     * @param outgoing_msg      Destination message.
     * @param offset            Current payload offset.
     * @param requested_bytes   Max bytes to copy.
     * @endverbatim
     *
     * @return Updated offset.
     */
uint16_t ProtocolSnip_load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    configuration_memory_buffer_t configuration_memory_buffer;
    uint32_t data_address = USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS; // User Name is by default the first 63 Bytes in the Configuration Space and Description next 64 bytes

    if (openlcb_node->parameters->address_space_config_memory.low_address_valid) {

        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    }

    _interface->config_memory_read(openlcb_node, data_address, requested_bytes, &configuration_memory_buffer); // grab string from config memory

    _process_snip_string(outgoing_msg, &offset, (char*) (&configuration_memory_buffer[0]), LEN_SNIP_USER_DESCRIPTION_BUFFER, requested_bytes);

    return offset;

}

    /**
     * @brief Build and return a SNIP reply (MTI 0x0A08).
     *
     * @details Algorithm:
     * -# Prepare outgoing message header addressed to the requester
     * -# Append 8 fields sequentially: mfg version, name, model, HW ver,
     *    SW ver, user version, user name, user description
     * -# Mark outgoing message valid
     *
     * @verbatim
     * @param statemachine_info  Context with the incoming SNIP request.
     * @endverbatim
     */
void ProtocolSnip_handle_simple_node_info_request(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t payload_offset = 0;

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_SIMPLE_NODE_INFO_REPLY);

    payload_offset = ProtocolSnip_load_manufacturer_version_id(
            statemachine_info->openlcb_node,
            statemachine_info->outgoing_msg_info.msg_ptr,
            payload_offset,
            1);

    payload_offset = ProtocolSnip_load_name(
            statemachine_info->openlcb_node,
            statemachine_info->outgoing_msg_info.msg_ptr,
            payload_offset,
            LEN_SNIP_NAME_BUFFER - 1);

    payload_offset = ProtocolSnip_load_model(
            statemachine_info->openlcb_node,
            statemachine_info->outgoing_msg_info.msg_ptr,
            payload_offset,
            LEN_SNIP_MODEL_BUFFER - 1);

    payload_offset = ProtocolSnip_load_hardware_version(
            statemachine_info->openlcb_node,
            statemachine_info->outgoing_msg_info.msg_ptr,
            payload_offset,
            LEN_SNIP_HARDWARE_VERSION_BUFFER - 1);

    payload_offset = ProtocolSnip_load_software_version(
            statemachine_info->openlcb_node,
            statemachine_info->outgoing_msg_info.msg_ptr,
            payload_offset,
            LEN_SNIP_SOFTWARE_VERSION_BUFFER - 1);

    payload_offset = ProtocolSnip_load_user_version_id(
            statemachine_info->openlcb_node,
            statemachine_info->outgoing_msg_info.msg_ptr,
            payload_offset,
            1);

    payload_offset = ProtocolSnip_load_user_name(
            statemachine_info->openlcb_node,
            statemachine_info->outgoing_msg_info.msg_ptr,
            payload_offset,
            LEN_SNIP_USER_NAME_BUFFER - 1);

    payload_offset = ProtocolSnip_load_user_description(
            statemachine_info->openlcb_node,
            statemachine_info->outgoing_msg_info.msg_ptr,
            payload_offset,
            LEN_SNIP_USER_DESCRIPTION_BUFFER - 1);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /** @brief Handle incoming SNIP reply — no automatic response generated. */
void ProtocolSnip_handle_simple_node_info_reply(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
     * @brief Validate a SNIP reply: correct MTI, valid length, exactly 6 nulls.
     *
     * @details Algorithm:
     * -# Reject if payload_count > LEN_MESSAGE_BYTES_SNIP
     * -# Reject if MTI != MTI_SIMPLE_NODE_INFO_REPLY
     * -# Count null bytes in payload; must equal 6
     *
     * @verbatim
     * @param snip_reply_msg  Message to validate.
     * @endverbatim
     *
     * @return true if the message is a well-formed SNIP reply.
     */
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
