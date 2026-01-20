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
 * @brief Implementation of Simple Node Information Protocol (SNIP)
 * @author Jim Kueneman
 * @date 17 Jan 2026
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


static interface_openlcb_protocol_snip_t *_interface;

    /**
    * @brief Initializes the SNIP protocol handler
    *
    * @details Algorithm:
    * -# Store interface pointer in static variable
    * -# Make callback functions available to SNIP loader functions
    *
    * Use cases:
    * - Called once during application initialization
    * - Must be called before processing any SNIP requests
    *
    * @verbatim
    * @param interface_openlcb_protocol_snip Pointer to interface structure with callbacks
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Interface must remain valid for lifetime of application
    *
    * @see interface_openlcb_protocol_snip_t
    */
void ProtocolSnip_initialize(const interface_openlcb_protocol_snip_t *interface_openlcb_protocol_snip) {

    _interface = (interface_openlcb_protocol_snip_t*) interface_openlcb_protocol_snip;

}

    /**
    * @brief Internal helper to process SNIP string fields
    *
    * @details Algorithm:
    * -# Calculate string length using strlen
    * -# Check if string exceeds max buffer length
    * -# Determine if result should be null-terminated
    * -# If null-terminated:
    *    - Copy string to payload at offset
    *    - Add null terminator
    *    - Update offset and payload count
    * -# If not null-terminated:
    *    - Copy requested bytes without null terminator
    *    - Update offset and payload count
    *
    * @verbatim
    * @param outgoing_msg Message to load string into
    * @endverbatim
    * @verbatim
    * @param payload_offset Pointer to current payload offset (updated by function)
    * @endverbatim
    * @verbatim
    * @param str Source string to copy
    * @endverbatim
    * @verbatim
    * @param max_str_len Maximum string buffer length
    * @endverbatim
    * @verbatim
    * @param byte_count Number of bytes requested
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
    * @brief Internal helper to process SNIP version ID bytes
    *
    * @details Algorithm:
    * -# Write version byte to payload at current offset
    * -# Increment payload count
    * -# Increment offset pointer
    * -# Return updated offset
    *
    * @verbatim
    * @param outgoing_msg Message to load version into
    * @endverbatim
    * @verbatim
    * @param payload_data_offset Pointer to current payload offset (updated by function)
    * @endverbatim
    * @verbatim
    * @param version Version ID byte to write
    * @endverbatim
    *
    * @return Updated payload offset value
    */
static uint16_t _process_snip_version(openlcb_msg_t* outgoing_msg, uint16_t *payload_data_offset, const uint8_t version) {

    *outgoing_msg->payload[*payload_data_offset] = version;
    outgoing_msg->payload_count++;
    (*payload_data_offset)++;

    return *payload_data_offset;

}

    /**
    * @brief Loads Manufacturer Version ID into message payload
    *
    * @details Algorithm:
    * -# Check if requested_bytes > 0
    * -# If yes: Call _process_snip_version with manufacturer version byte
    * -# If no: Return 0
    *
    * Use cases:
    * - Building SNIP reply messages
    * - Responding to Simple Node Info Request
    *
    * @verbatim
    * @param openlcb_node Node being requested for the information
    * @endverbatim
    * @verbatim
    * @param outgoing_msg Message to load the information into its payload
    * @endverbatim
    * @verbatim
    * @param offset The payload index to copy the information to
    * @endverbatim
    * @verbatim
    * @param requested_bytes The max number of bytes to copy
    * @endverbatim
    *
    * @return Updated offset value, or 0 if no bytes requested
    *
    * @note The OpenLCB message's Payload Count is auto-updated
    *
    * @see _process_snip_version
    */
uint16_t ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    if (requested_bytes > 0) {

        return _process_snip_version(outgoing_msg, &offset, openlcb_node->parameters->snip.mfg_version);

    }

    return 0;

}

    /**
    * @brief Loads Manufacturer Name string into message payload
    *
    * @details Algorithm:
    * -# Call _process_snip_string with manufacturer name from node parameters
    * -# String is limited to LEN_SNIP_NAME_BUFFER size
    * -# Return updated offset value
    *
    * Use cases:
    * - Building SNIP reply messages
    * - Responding to Simple Node Info Request
    *
    * @verbatim
    * @param openlcb_node Node being requested for the information
    * @endverbatim
    * @verbatim
    * @param outgoing_msg Message to load the information into its payload
    * @endverbatim
    * @verbatim
    * @param offset The payload index to copy the information to
    * @endverbatim
    * @verbatim
    * @param requested_bytes The max number of bytes to copy
    * @endverbatim
    *
    * @return Updated offset value
    *
    * @note The OpenLCB message's Payload Count is auto-updated
    *
    * @see _process_snip_string
    */
uint16_t ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    _process_snip_string(outgoing_msg, &offset, openlcb_node->parameters->snip.name, LEN_SNIP_NAME_BUFFER, requested_bytes);

    return offset;

}

    /**
    * @brief Loads Manufacturer Model string into message payload
    *
    * @details Algorithm:
    * -# Call _process_snip_string with model string from node parameters
    * -# String is limited to LEN_SNIP_MODEL_BUFFER size
    * -# Return updated offset value
    *
    * Use cases:
    * - Building SNIP reply messages
    * - Responding to Simple Node Info Request
    *
    * @verbatim
    * @param openlcb_node Node being requested for the information
    * @endverbatim
    * @verbatim
    * @param outgoing_msg Message to load the information into its payload
    * @endverbatim
    * @verbatim
    * @param offset The payload index to copy the information to
    * @endverbatim
    * @verbatim
    * @param requested_bytes The max number of bytes to copy
    * @endverbatim
    *
    * @return Updated offset value
    *
    * @note The OpenLCB message's Payload Count is auto-updated
    *
    * @see _process_snip_string
    */
uint16_t ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    _process_snip_string(outgoing_msg, &offset, openlcb_node->parameters->snip.model, LEN_SNIP_MODEL_BUFFER, requested_bytes);

    return offset;

}

    /**
    * @brief Loads Manufacturer Hardware Version string into message payload
    *
    * @details Algorithm:
    * -# Call _process_snip_string with hardware version from node parameters
    * -# String is limited to LEN_SNIP_HARDWARE_VERSION_BUFFER size
    * -# Return updated offset value
    *
    * Use cases:
    * - Building SNIP reply messages
    * - Responding to Simple Node Info Request
    *
    * @verbatim
    * @param openlcb_node Node being requested for the information
    * @endverbatim
    * @verbatim
    * @param outgoing_msg Message to load the information into its payload
    * @endverbatim
    * @verbatim
    * @param offset The payload index to copy the information to
    * @endverbatim
    * @verbatim
    * @param requested_bytes The max number of bytes to copy
    * @endverbatim
    *
    * @return Updated offset value
    *
    * @note The OpenLCB message's Payload Count is auto-updated
    *
    * @see _process_snip_string
    */
uint16_t ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    _process_snip_string(outgoing_msg, &offset, openlcb_node->parameters->snip.hardware_version, LEN_SNIP_HARDWARE_VERSION_BUFFER, requested_bytes);

    return offset;

}

    /**
    * @brief Loads Manufacturer Software Version string into message payload
    *
    * @details Algorithm:
    * -# Call _process_snip_string with software version from node parameters
    * -# String is limited to LEN_SNIP_SOFTWARE_VERSION_BUFFER size
    * -# Return updated offset value
    *
    * Use cases:
    * - Building SNIP reply messages
    * - Responding to Simple Node Info Request
    *
    * @verbatim
    * @param openlcb_node Node being requested for the information
    * @endverbatim
    * @verbatim
    * @param outgoing_msg Message to load the information into its payload
    * @endverbatim
    * @verbatim
    * @param offset The payload index to copy the information to
    * @endverbatim
    * @verbatim
    * @param requested_bytes The max number of bytes to copy
    * @endverbatim
    *
    * @return Updated offset value
    *
    * @note The OpenLCB message's Payload Count is auto-updated
    *
    * @see _process_snip_string
    */
uint16_t ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    _process_snip_string(outgoing_msg, &offset, openlcb_node->parameters->snip.software_version, LEN_SNIP_SOFTWARE_VERSION_BUFFER, requested_bytes);

    return offset;

}

    /**
    * @brief Loads User Version ID byte into message payload
    *
    * @details Algorithm:
    * -# Check if requested_bytes > 0
    * -# If yes: Call _process_snip_version with user version byte
    * -# If no: Return 0
    *
    * Use cases:
    * - Building SNIP reply messages
    * - Responding to Simple Node Info Request
    *
    * @verbatim
    * @param openlcb_node Node being requested for the information
    * @endverbatim
    * @verbatim
    * @param outgoing_msg Message to load the information into its payload
    * @endverbatim
    * @verbatim
    * @param offset The payload index to copy the information to
    * @endverbatim
    * @verbatim
    * @param requested_bytes The max number of bytes to copy
    * @endverbatim
    *
    * @return Updated offset value, or 0 if no bytes requested
    *
    * @note The OpenLCB message's Payload Count is auto-updated
    *
    * @see _process_snip_version
    */
uint16_t ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes) {

    if (requested_bytes > 0) {

        return _process_snip_version(outgoing_msg, &offset, openlcb_node->parameters->snip.user_version);

    }

    return 0;

}

    /**
    * @brief Loads User Name string into message payload from configuration memory
    *
    * @details Algorithm:
    * -# Calculate data address (USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS)
    * -# Add low_address offset if valid
    * -# Read string from configuration memory using callback
    * -# Call _process_snip_string with read data
    * -# String is limited to LEN_SNIP_USER_NAME_BUFFER size
    * -# Return updated offset value
    *
    * Use cases:
    * - Building SNIP reply messages
    * - Responding to Simple Node Info Request
    *
    * @verbatim
    * @param openlcb_node Node being requested for the information
    * @endverbatim
    * @verbatim
    * @param outgoing_msg Message to load the information into its payload
    * @endverbatim
    * @verbatim
    * @param offset The payload index to copy the information to
    * @endverbatim
    * @verbatim
    * @param requested_bytes The max number of bytes to copy
    * @endverbatim
    *
    * @return Updated offset value
    *
    * @note The OpenLCB message's Payload Count is auto-updated
    * @note User name is read from configuration memory, not node parameters
    *
    * @see _process_snip_string
    * @see USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS
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
    * @brief Loads User Description string into message payload from configuration memory
    *
    * @details Algorithm:
    * -# Calculate data address (USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS)
    * -# Add low_address offset if valid
    * -# Read string from configuration memory using callback
    * -# Call _process_snip_string with read data
    * -# String is limited to LEN_SNIP_USER_DESCRIPTION_BUFFER size
    * -# Return updated offset value
    *
    * Use cases:
    * - Building SNIP reply messages
    * - Responding to Simple Node Info Request
    *
    * @verbatim
    * @param openlcb_node Node being requested for the information
    * @endverbatim
    * @verbatim
    * @param outgoing_msg Message to load the information into its payload
    * @endverbatim
    * @verbatim
    * @param offset The payload index to copy the information to
    * @endverbatim
    * @verbatim
    * @param requested_bytes The max number of bytes to copy
    * @endverbatim
    *
    * @return Updated offset value
    *
    * @note The OpenLCB message's Payload Count is auto-updated
    * @note User description is read from configuration memory, not node parameters
    *
    * @see _process_snip_string
    * @see USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS
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
    * @brief Handles incoming Simple Node Info Request message
    *
    * @details Algorithm:
    * -# Initialize payload offset to 0
    * -# Load OpenLCB message header with SNIP reply MTI
    * -# Call loader functions in sequence:
    *    - Manufacturer Version ID (1 byte)
    *    - Manufacturer Name (max LEN_SNIP_NAME_BUFFER-1)
    *    - Model (max LEN_SNIP_MODEL_BUFFER-1)
    *    - Hardware Version (max LEN_SNIP_HARDWARE_VERSION_BUFFER-1)
    *    - Software Version (max LEN_SNIP_SOFTWARE_VERSION_BUFFER-1)
    *    - User Version ID (1 byte)
    *    - User Name (max LEN_SNIP_USER_NAME_BUFFER-1)
    *    - User Description (max LEN_SNIP_USER_DESCRIPTION_BUFFER-1)
    * -# Set outgoing message valid flag
    *
    * Use cases:
    * - Processing MTI_SIMPLE_NODE_INFO_REQUEST
    * - Responding to SNIP queries from other nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Node must be initialized
    *
    * @note Each loader function updates the offset
    * @note Reply is sent to the requesting node
    *
    * @see MTI_SIMPLE_NODE_INFO_REPLY
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

    /**
    * @brief Handles incoming Simple Node Info Reply message
    *
    * @details Algorithm:
    * -# Set outgoing message valid flag to false (no reply needed)
    *
    * Use cases:
    * - Processing MTI_SIMPLE_NODE_INFO_REPLY from other nodes
    * - Receiving SNIP information from queried nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @note This is a stub - application can override to process replies
    * @note No outgoing message generated
    *
    * @see MTI_SIMPLE_NODE_INFO_REPLY
    */
void ProtocolSnip_handle_simple_node_info_reply(openlcb_statemachine_info_t *statemachine_info) {

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Validates a SNIP reply message structure
    *
    * @details Algorithm:
    * -# Check payload size doesn't exceed LEN_MESSAGE_BYTES_SNIP
    * -# Verify MTI is MTI_SIMPLE_NODE_INFO_REPLY
    * -# Count null terminators in payload
    * -# Verify exactly 6 null terminators present (one per field after version bytes)
    *
    * Use cases:
    * - Validating received SNIP replies
    * - Detecting malformed SNIP messages
    *
    * @verbatim
    * @param snip_reply_msg Pointer to message to validate
    * @endverbatim
    *
    * @return true if message is valid SNIP reply, false otherwise
    *
    * @warning Returns false for oversized payloads
    * @warning Returns false for incorrect MTI
    * @warning Returns false if null terminator count is not 6
    *
    * @note SNIP format requires exactly 6 null-terminated strings
    *
    * @see MTI_SIMPLE_NODE_INFO_REPLY
    * @see LEN_MESSAGE_BYTES_SNIP
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
