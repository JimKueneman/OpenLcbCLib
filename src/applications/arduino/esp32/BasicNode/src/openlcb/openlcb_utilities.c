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
* @file openlcb_utilities.c
* @brief Implementation of common utility functions for OpenLCB message and buffer manipulation
*
* @details This module implements utility functions for working with OpenLCB messages,
* payloads, configuration memory buffers, and node structures. These are helper
* functions used throughout the OpenLCB library for data conversion, message
* construction, and buffer operations.
*
* The implementation provides:
* - Big-endian byte order handling per OpenLCB specification
* - Safe message construction with automatic payload counting
* - Configuration memory buffer manipulation
* - Message classification and routing helpers
* - Configuration memory protocol reply construction
*
* All byte manipulation follows OpenLCB's big-endian (network byte order) convention.
* Multi-byte values are stored with the most significant byte first.
*
* @author Jim Kueneman
* @date 17 Jan 2026
*/

#include "openlcb_utilities.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_buffer_store.h"

    /**
    * @brief Converts payload type enum to byte length
    *
    * @details Algorithm:
    * -# Switch on payload_type enum value
    * -# Return corresponding LEN_MESSAGE_BYTES_* constant
    * -# Return 0 for unknown types
    *
    * Use cases:
    * - Allocating message buffers
    * - Validating payload operations
    * - Determining maximum data capacity
    *
    * @verbatim
    * @param payload_type The payload type enum value
    * @endverbatim
    * @return Maximum payload size in bytes, or 0 for unknown types
    *
    * @note Return values per OpenLCB specification:
    *       - BASIC: LEN_MESSAGE_BYTES_BASIC
    *       - DATAGRAM: LEN_MESSAGE_BYTES_DATAGRAM  
    *       - SNIP: LEN_MESSAGE_BYTES_SNIP
    *       - STREAM: LEN_MESSAGE_BYTES_STREAM
    *       - default: 0
    *
    * @remark Constant time operation.
    *
    * @see openlcb_defines.h - Defines the LEN_MESSAGE_BYTES_* constants
    * @see payload_type_enum - Enum definition in openlcb_types.h
    */
uint16_t OpenLcbUtilities_payload_type_to_len(payload_type_enum payload_type) {

    switch (payload_type) {

        case BASIC:

            return LEN_MESSAGE_BYTES_BASIC;

        case DATAGRAM:

            return LEN_MESSAGE_BYTES_DATAGRAM;

        case SNIP:

            return LEN_MESSAGE_BYTES_SNIP;

        case STREAM:

            return LEN_MESSAGE_BYTES_STREAM;

        default:

            return 0;
    }

}

    /**
    * @brief Calculates the memory offset for a node's configuration space
    *
    * @details Algorithm:
    * -# Read highest_address from node parameters
    * -# If low_address_valid is true, compute size as (highest - lowest)
    * -# If low_address_valid is false, use highest_address as size
    * -# Multiply size by node index to get offset
    * -# Return the calculated offset
    *
    * This allows multiple nodes to share a contiguous configuration memory space,
    * with each node occupying a sequential block of memory determined by its index.
    *
    * Use cases:
    * - Multi-node configuration memory implementations
    * - Mapping node index to memory address
    * - Validating configuration memory access requests
    *
    * @verbatim
    * @param openlcb_node Pointer to the node structure
    * @endverbatim
    * @return Byte offset into global configuration memory for this node
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning openlcb_node->parameters must not be NULL. No NULL check performed.
    *
    * @attention If low_address_valid is true, uses (highest - lowest) as size.
    * @attention If low_address_valid is false, uses highest_address as size.
    *
    * @note Result is node_size * node_index, allowing sequential allocation.
    *
    * @remark Constant time operation.
    *
    * @see protocol_config_mem_*.c - Uses this for address space calculations
    */
uint32_t OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node_t* openlcb_node) {

    uint32_t offset_per_node = openlcb_node->parameters->address_space_config_memory.highest_address;

    if (openlcb_node->parameters->address_space_config_memory.low_address_valid) {

        offset_per_node = openlcb_node->parameters->address_space_config_memory.highest_address - openlcb_node->parameters->address_space_config_memory.low_address;

    }

    return (offset_per_node * openlcb_node->index);

}

    /**
    * @brief Initializes an OpenLCB message structure with source, destination, and MTI
    *
    * @details Algorithm:
    * -# Store dest_alias and dest_id
    * -# Store source_alias and source_id
    * -# Store mti (Message Type Indicator)
    * -# Set payload_count to 0
    * -# Set timerticks to 0
    * -# Get payload length from payload_type
    * -# Zero all payload bytes
    *
    * This is the primary function for preparing a new message for transmission.
    *
    * Use cases:
    * - Preparing outgoing messages in protocol handlers
    * - Initializing reply messages in response to received messages
    * - Setting up event producer/consumer messages
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure to initialize
    * @endverbatim
    * @verbatim
    * @param source_alias 12-bit CAN alias of the source node
    * @endverbatim
    * @verbatim
    * @param source_id 48-bit unique Node ID of the source node
    * @endverbatim
    * @verbatim
    * @param dest_alias 12-bit CAN alias of the destination node (0 for global messages)
    * @endverbatim
    * @verbatim
    * @param dest_id 48-bit unique Node ID of the destination node (0 for global messages)
    * @endverbatim
    * @verbatim
    * @param mti Message Type Indicator defining the message type
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning The buffer size is determined by openlcb_msg->payload_type.
    *
    * @attention Always call this before manually setting payload bytes to ensure
    *            the payload starts in a known cleared state.
    *
    * @remark Linear time operation.
    *
    * @see OpenLcbUtilities_clear_openlcb_message - Complete message reset
    * @see OpenLcbUtilities_clear_openlcb_message_payload - Payload-only reset
    */
void OpenLcbUtilities_load_openlcb_message(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t source_id, uint16_t dest_alias, uint64_t dest_id, uint16_t mti) {

    openlcb_msg->dest_alias = dest_alias;
    openlcb_msg->dest_id = dest_id;
    openlcb_msg->source_alias = source_alias;
    openlcb_msg->source_id = source_id;
    openlcb_msg->mti = mti;
    openlcb_msg->payload_count = 0;
    openlcb_msg->timerticks = 0;

    uint16_t data_count = OpenLcbUtilities_payload_type_to_len(openlcb_msg->payload_type);

    for (int i = 0; i < data_count; i++) {

    *openlcb_msg->payload[i] = 0x00;

    }

}

    /**
    * @brief Clears only the payload portion of a message structure
    *
    * @details Algorithm:
    * -# Get payload length from payload_type
    * -# Loop through all payload bytes
    * -# Set each byte to 0
    * -# Set payload_count to 0
    *
    * Does not modify message header fields (source, dest, MTI).
    *
    * Use cases:
    * - Reusing a message structure for a new reply
    * - Clearing payload before building multi-part response
    * - Resetting payload between retry attempts
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    *
    * @attention Preserves message header (aliases, IDs, MTI). Use
    *            OpenLcbUtilities_clear_openlcb_message to clear everything.
    *
    * @note Payload size is determined by openlcb_msg->payload_type.
    *
    * @remark Linear time operation.
    *
    * @see OpenLcbUtilities_clear_openlcb_message - Clears entire message
    * @see OpenLcbUtilities_load_openlcb_message - Initializes message with values
    */
void OpenLcbUtilities_clear_openlcb_message_payload(openlcb_msg_t* openlcb_msg) {

    uint16_t data_len = OpenLcbUtilities_payload_type_to_len(openlcb_msg->payload_type);

    for (int i = 0; i < data_len; i++) {

    *openlcb_msg->payload[i] = 0;

    }

    openlcb_msg->payload_count = 0;

}

    /**
    * @brief Completely clears and resets a message structure
    *
    * @details Algorithm:
    * -# Set dest_alias to 0
    * -# Set dest_id to 0
    * -# Set source_alias to 0
    * -# Set source_id to 0
    * -# Set mti to 0
    * -# Set payload_count to 0
    * -# Set timerticks to 0
    * -# Set reference_count to 0
    * -# Set allocated flag to false
    * -# Set inprocess flag to false
    *
    * Zeros all header fields, clears payload, and resets all state flags.
    *
    * Use cases:
    * - Returning a message buffer to the pool
    * - Preparing a buffer for initial allocation
    * - Recovering from error conditions
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    *
    * @attention This clears ALL fields including state flags and reference count.
    *            Only use when completely discarding message context.
    *
    * @attention After calling this, the message should be considered unallocated
    *            and should not be used until properly initialized again.
    *
    * @note Does NOT zero the payload bytes themselves, only the header and state.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_clear_openlcb_message_payload - Clears only payload
    * @see OpenLcbUtilities_load_openlcb_message - Initializes message with values
    */
void OpenLcbUtilities_clear_openlcb_message(openlcb_msg_t *openlcb_msg) {

    openlcb_msg->dest_alias = 0;
    openlcb_msg->dest_id = 0;
    openlcb_msg->source_alias = 0;
    openlcb_msg->source_id = 0;
    openlcb_msg->mti = 0;
    openlcb_msg->payload_count = 0;
    openlcb_msg->timerticks = 0;
    openlcb_msg->reference_count = 0;
    openlcb_msg->state.allocated = false;
    openlcb_msg->state.inprocess = false;

}

    /**
    * @brief Copies an 8-byte event ID into the message payload
    *
    * @details Algorithm:
    * -# Loop from byte index 7 down to 0 (big-endian order)
    * -# Extract least significant byte of event_id
    * -# Write byte to payload[i]
    * -# Increment payload_count
    * -# Right-shift event_id by 8 bits
    * -# Repeat for all 8 bytes
    *
    * Writes the event ID in big-endian format to the payload starting at offset 0.
    * Event IDs are 64-bit values that identify producer and consumer events per
    * the Event Transport protocol.
    *
    * Use cases:
    * - Building Producer Identified messages
    * - Building Consumer Identified messages
    * - Building Producer/Consumer Event Report messages
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param event_id 64-bit event identifier
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes payload has at least 8 bytes available.
    *
    * @attention This function always writes to offset 0. For events at other
    *            positions, manually copy bytes or use a different function.
    *
    * @note Updates payload_count to reflect the added bytes (increments by 8).
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_extract_event_id_from_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_copy_event_id_to_config_mem_buffer - Config memory version
    */
void OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, event_id_t event_id) {

    for (int i = 7; i >= 0; i--) {

    *openlcb_msg->payload[i] = event_id & 0xFF;
        openlcb_msg->payload_count++;
        event_id = event_id >> 8;

    }

    openlcb_msg->payload_count = 8;

}

    /**
    * @brief Copies a single byte into the message payload at a specified offset
    *
    * @details Algorithm:
    * -# Write byte to payload[offset]
    * -# Increment payload_count by 1
    *
    * Use cases:
    * - Writing command codes in datagrams
    * - Writing status flags in protocol messages
    * - Writing individual configuration bytes
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param byte The byte value to write
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where the byte should be written
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Does not validate offset bounds against payload size.
    *
    * @note Updates payload_count to reflect the added byte.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_extract_byte_from_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_copy_word_to_openlcb_payload - For 16-bit values
    */
void OpenLcbUtilities_copy_byte_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint8_t byte, uint16_t offset) {

    *openlcb_msg->payload[offset] = byte;

    openlcb_msg->payload_count++;

}

    /**
    * @brief Copies a 16-bit word into the message payload at a specified offset
    *
    * @details Algorithm:
    * -# Extract high byte (bits 15-8) by right-shifting 8 bits
    * -# Write high byte to payload[offset]
    * -# Extract low byte (bits 7-0) by masking with 0xFF
    * -# Write low byte to payload[offset + 1]
    * -# Increment payload_count by 2
    *
    * Writes the word in big-endian format (MSB first).
    *
    * Use cases:
    * - Writing memory addresses in configuration memory protocol
    * - Writing error codes in datagram replies
    * - Writing 16-bit configuration values
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param word The 16-bit value to write
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where the word should be written
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes payload has at least (offset + 2) bytes available.
    * @warning Does not validate offset bounds.
    *
    * @note Updates payload_count to reflect the added bytes.
    * @note Byte order is big-endian per OpenLCB specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_extract_word_from_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_copy_dword_to_openlcb_payload - For 32-bit values
    */
void OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t word, uint16_t offset) {

    *openlcb_msg->payload[0 + offset] = (uint8_t) ((word >> 8) & 0xFF);
    *openlcb_msg->payload[1 + offset] = (uint8_t) (word & 0xFF);

    openlcb_msg->payload_count += 2;

}

    /**
    * @brief Copies a 32-bit doubleword into the message payload at a specified offset
    *
    * @details Algorithm:
    * -# Extract byte 3 (bits 31-24) by right-shifting 24 bits
    * -# Write byte 3 to payload[offset]
    * -# Extract byte 2 (bits 23-16) by right-shifting 16 bits
    * -# Write byte 2 to payload[offset + 1]
    * -# Extract byte 1 (bits 15-8) by right-shifting 8 bits
    * -# Write byte 1 to payload[offset + 2]
    * -# Extract byte 0 (bits 7-0) by masking with 0xFF
    * -# Write byte 0 to payload[offset + 3]
    * -# Increment payload_count by 4
    *
    * Writes the doubleword in big-endian format (MSB first).
    *
    * Use cases:
    * - Writing 32-bit memory addresses in configuration memory protocol
    * - Writing large numeric values in protocol messages
    * - Writing timestamps or counters
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param doubleword The 32-bit value to write
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where the doubleword should be written
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes payload has at least (offset + 4) bytes available.
    * @warning Does not validate offset bounds.
    *
    * @note Updates payload_count to reflect the added bytes.
    * @note Byte order is big-endian per OpenLCB specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_extract_dword_from_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_copy_word_to_openlcb_payload - For 16-bit values
    */
void OpenLcbUtilities_copy_dword_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint32_t doubleword, uint16_t offset) {

    *openlcb_msg->payload[0 + offset] = (uint8_t) ((doubleword >> 24) & 0xFF);
    *openlcb_msg->payload[1 + offset] = (uint8_t) ((doubleword >> 16) & 0xFF);
    *openlcb_msg->payload[2 + offset] = (uint8_t) ((doubleword >> 8) & 0xFF);
    *openlcb_msg->payload[3 + offset] = (uint8_t) (doubleword & 0xFF);

    openlcb_msg->payload_count += 4;

}

    /**
    * @brief Copies a null-terminated string into the message payload
    *
    * @details Algorithm:
    * -# Initialize counter to 0
    * -# Get payload length from payload_type
    * -# While source string[counter] is not null:
    *    -# If (counter + offset) < (payload_len - 1):
    *       -# Copy string[counter] to payload[counter + offset]
    *       -# Increment payload_count
    *       -# Increment counter
    *    -# Else break (no more space)
    * -# Write null terminator to payload[counter + offset]
    * -# Increment payload_count and counter for null
    * -# Return total bytes written (counter)
    *
    * Copies characters from the string until null terminator is found or
    * payload space is exhausted. Always appends a null terminator.
    *
    * Use cases:
    * - Building Simple Node Ident Info (SNIP) replies with manufacturer/model strings
    * - Writing user-defined strings in configuration data
    * - Building text-based protocol messages
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param string Null-terminated C string to copy
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where string should start
    * @endverbatim
    * @return Number of bytes written including the null terminator
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning string must be null-terminated or buffer overflow may occur.
    * @warning Truncates string if payload space is insufficient, but always
    *          adds null terminator (may overwrite last character).
    *
    * @attention Always reserves one byte for null terminator. Maximum string
    *            length is (payload_size - offset - 1).
    *
    * @note Updates payload_count to include all bytes written plus null terminator.
    * @note Return value can be used to calculate offset for next data in payload.
    *
    * @remark Linear time operation.
    *
    * @see OpenLcbUtilities_copy_byte_array_to_openlcb_payload - For binary data
    */
uint16_t OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const char string[], uint16_t offset) {

    uint16_t counter = 0;
    uint16_t payload_len = 0;

    payload_len = OpenLcbUtilities_payload_type_to_len(openlcb_msg->payload_type);

    while (string[counter] != 0x00) {

        if ((counter + offset) < payload_len - 1) {

    *openlcb_msg->payload[counter + offset] = (uint8_t) string[counter];
            openlcb_msg->payload_count++;
            counter++;

        } else {

            break;

        }

    }

    *openlcb_msg->payload[counter + offset] = 0x00;
    openlcb_msg->payload_count++;
    counter++;

    return counter;

}

    /**
    * @brief Copies a byte array into the message payload
    *
    * @details Algorithm:
    * -# Initialize counter to 0
    * -# Get payload length from payload_type
    * -# For i = 0 to requested_bytes - 1:
    *    -# If (i + offset) < payload_len:
    *       -# Copy byte_array[i] to payload[i + offset]
    *       -# Increment payload_count
    *       -# Increment counter
    *    -# Else break (no more space)
    * -# Return counter (actual bytes copied)
    *
    * Copies the requested number of bytes from the array, or until
    * payload space is exhausted. Returns the actual number of bytes copied.
    *
    * Use cases:
    * - Copying configuration memory data into read reply datagrams
    * - Building multi-byte protocol responses
    * - Transferring binary data blocks
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param byte_array Array of bytes to copy
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where array should start
    * @endverbatim
    * @verbatim
    * @param requested_bytes Number of bytes to attempt to copy
    * @endverbatim
    * @return Actual number of bytes copied (may be less if payload full)
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning byte_array must have at least requested_bytes available.
    * @warning May copy fewer bytes than requested if payload space exhausted.
    *
    * @attention Check return value to determine if all requested bytes were copied.
    * @attention Does not add null terminator (unlike copy_string).
    *
    * @note Updates payload_count to reflect bytes actually copied.
    * @note Return value indicates actual bytes written, useful for multi-frame messages.
    *
    * @remark Linear time operation.
    *
    * @see OpenLcbUtilities_copy_string_to_openlcb_payload - For null-terminated strings
    */
uint16_t OpenLcbUtilities_copy_byte_array_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const uint8_t byte_array[], uint16_t offset, uint16_t requested_bytes) {

    uint16_t counter = 0;
    uint16_t payload_len = 0;

    payload_len = OpenLcbUtilities_payload_type_to_len(openlcb_msg->payload_type);

    for (uint16_t i = 0; i < requested_bytes; i++) {

        if ((i + offset) < payload_len) {

    *openlcb_msg->payload[i + offset] = byte_array[i];
            openlcb_msg->payload_count++;
            counter++;

        } else {

            break;

        }

    }

    return counter;

}

    /**
    * @brief Copies a 6-byte node ID into the message payload at a specified offset
    *
    * @details Algorithm:
    * -# Loop from byte index 5 down to 0 (big-endian order)
    * -# Extract least significant byte of node_id
    * -# Write byte to payload[i + offset]
    * -# Increment payload_count
    * -# Right-shift node_id by 8 bits
    * -# Repeat for all 6 bytes
    *
    * Writes the node ID in big-endian format to the payload.
    * Node IDs are 48-bit unique identifiers per the OpenLCB specification.
    *
    * Use cases:
    * - Building Verified Node ID messages
    * - Building Simple Node Ident Info replies
    * - Building protocol support inquiry responses
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param node_id 48-bit unique node identifier
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where node ID should be written
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes payload has at least (offset + 6) bytes available.
    * @warning Does not validate offset bounds.
    *
    * @note Updates payload_count to reflect the added bytes (increments by 6).
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_extract_node_id_from_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_copy_node_id_to_config_mem_buffer - Config memory version
    */
void OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, node_id_t node_id, uint16_t offset) {

    for (int i = 5; i >= 0; i--) {

    *openlcb_msg->payload[(uint16_t) i + offset] = node_id & 0xFF;
        openlcb_msg->payload_count++;
        node_id = node_id >> 8;

    }

}

    /**
    * @brief Extracts a 6-byte node ID from the message payload
    *
    * @details Algorithm:
    * -# Initialize result to 0
    * -# Read payload[0 + offset], shift left 40 bits, OR into result
    * -# Read payload[1 + offset], shift left 32 bits, OR into result
    * -# Read payload[2 + offset], shift left 24 bits, OR into result
    * -# Read payload[3 + offset], shift left 16 bits, OR into result
    * -# Read payload[4 + offset], shift left 8 bits, OR into result
    * -# Read payload[5 + offset], OR into result
    * -# Return assembled 48-bit node ID
    *
    * Reads 6 bytes in big-endian format starting at the specified offset
    * and assembles them into a 48-bit node_id_t value.
    *
    * Use cases:
    * - Parsing Verified Node ID messages
    * - Extracting node IDs from protocol inquiry responses
    * - Reading node IDs from configuration memory
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where node ID starts
    * @endverbatim
    * @return The assembled 48-bit node ID
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes payload has at least (offset + 6) bytes available.
    * @warning Does not validate offset bounds.
    * @warning Reading beyond payload bounds returns undefined values.
    *
    * @note Does not modify payload_count.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_copy_node_id_to_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_extract_node_id_from_config_mem_buffer - Config memory version
    */
node_id_t OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (
            ((uint64_t) * openlcb_msg->payload[0 + offset] << 40) |
            ((uint64_t) * openlcb_msg->payload[1 + offset] << 32) |
            ((uint64_t) * openlcb_msg->payload[2 + offset] << 24) |
            ((uint64_t) * openlcb_msg->payload[3 + offset] << 16) |
            ((uint64_t) * openlcb_msg->payload[4 + offset] << 8) |
            ((uint64_t) * openlcb_msg->payload[5 + offset])
            );

}

    /**
    * @brief Extracts an 8-byte event ID from the message payload
    *
    * @details Algorithm:
    * -# Initialize result to 0
    * -# Read payload[0], shift left 56 bits, OR into result
    * -# Read payload[1], shift left 48 bits, OR into result
    * -# Read payload[2], shift left 40 bits, OR into result
    * -# Read payload[3], shift left 32 bits, OR into result
    * -# Read payload[4], shift left 24 bits, OR into result
    * -# Read payload[5], shift left 16 bits, OR into result
    * -# Read payload[6], shift left 8 bits, OR into result
    * -# Read payload[7], OR into result
    * -# Return assembled 64-bit event ID
    *
    * Reads 8 bytes in big-endian format starting at offset 0 and
    * assembles them into a 64-bit event_id_t value.
    *
    * Use cases:
    * - Parsing Producer/Consumer Event Report messages
    * - Extracting event IDs from Identify messages
    * - Reading event IDs from protocol queries
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @return The assembled 64-bit event ID
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes payload has at least 8 bytes available.
    * @warning Reading from empty payload returns undefined values.
    *
    * @attention Always reads from offset 0. For events at other positions,
    *            manually extract bytes or modify this function.
    *
    * @note Does not modify payload_count.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_copy_event_id_to_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_copy_config_mem_buffer_to_event_id - Config memory version
    */
event_id_t OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    return (
            ((uint64_t) * openlcb_msg->payload[0] << 56) |
            ((uint64_t) * openlcb_msg->payload[1] << 48) |
            ((uint64_t) * openlcb_msg->payload[2] << 40) |
            ((uint64_t) * openlcb_msg->payload[3] << 32) |
            ((uint64_t) * openlcb_msg->payload[4] << 24) |
            ((uint64_t) * openlcb_msg->payload[5] << 16) |
            ((uint64_t) * openlcb_msg->payload[6] << 8) |
            ((uint64_t) * openlcb_msg->payload[7])
            );

}

    /**
    * @brief Extracts a single byte from the message payload
    *
    * @details Algorithm:
    * -# Read and return payload[offset]
    *
    * Use cases:
    * - Reading command codes from datagrams
    * - Extracting status flags from protocol messages
    * - Reading individual configuration bytes
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload to read from
    * @endverbatim
    * @return The byte value at the specified offset
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Does not validate offset bounds against payload size.
    * @warning Reading beyond payload bounds returns undefined values.
    *
    * @note Does not modify payload_count.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_copy_byte_to_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_extract_word_from_openlcb_payload - For 16-bit values
    */
uint8_t OpenLcbUtilities_extract_byte_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (*openlcb_msg->payload[offset]);

}

    /**
    * @brief Extracts a 16-bit word from the message payload
    *
    * @details Algorithm:
    * -# Read payload[0 + offset], shift left 8 bits
    * -# Read payload[1 + offset]
    * -# OR the two bytes together
    * -# Return assembled 16-bit value
    *
    * Reads 2 bytes in big-endian format (MSB first) starting at the
    * specified offset and assembles them into a 16-bit value.
    *
    * Use cases:
    * - Reading memory addresses from configuration memory protocol
    * - Extracting error codes from datagram replies
    * - Reading 16-bit configuration values
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where word starts
    * @endverbatim
    * @return The assembled 16-bit value
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes payload has at least (offset + 2) bytes available.
    * @warning Does not validate offset bounds.
    * @warning Reading beyond payload bounds returns undefined values.
    *
    * @note Does not modify payload_count.
    * @note Byte order is big-endian per OpenLCB specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_copy_word_to_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_extract_dword_from_openlcb_payload - For 32-bit values
    */
uint16_t OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (
            ((uint16_t) * openlcb_msg->payload[0 + offset] << 8) |
            ((uint16_t) * openlcb_msg->payload[1 + offset])
            );

}

    /**
    * @brief Extracts a 32-bit doubleword from the message payload
    *
    * @details Algorithm:
    * -# Read payload[0 + offset], shift left 24 bits
    * -# Read payload[1 + offset], shift left 16 bits
    * -# Read payload[2 + offset], shift left 8 bits
    * -# Read payload[3 + offset]
    * -# OR all four bytes together
    * -# Return assembled 32-bit value
    *
    * Reads 4 bytes in big-endian format (MSB first) starting at the
    * specified offset and assembles them into a 32-bit value.
    *
    * Use cases:
    * - Reading 32-bit memory addresses from configuration memory protocol
    * - Extracting large numeric values from protocol messages
    * - Reading timestamps or counters
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @verbatim
    * @param offset Byte offset in the payload where doubleword starts
    * @endverbatim
    * @return The assembled 32-bit value
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes payload has at least (offset + 4) bytes available.
    * @warning Does not validate offset bounds.
    * @warning Reading beyond payload bounds returns undefined values.
    *
    * @note Does not modify payload_count.
    * @note Byte order is big-endian per OpenLCB specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_copy_dword_to_openlcb_payload - Reverse operation
    * @see OpenLcbUtilities_extract_word_from_openlcb_payload - For 16-bit values
    */
uint32_t OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (
            ((uint32_t) * openlcb_msg->payload[0 + offset] << 24) |
            ((uint32_t) * openlcb_msg->payload[1 + offset] << 16) |
            ((uint32_t) * openlcb_msg->payload[2 + offset] << 8) |
            ((uint32_t) * openlcb_msg->payload[3 + offset])
            );

}

    /**
    * @brief Sets the multi-frame flag in a target byte
    *
    * @details Algorithm:
    * -# Clear upper nibble by ANDing with 0x0F (preserves lower nibble)
    * -# Set flag in upper nibble by ORing with flag value
    *
    * Modifies the upper nibble of the target byte to set the multi-frame
    * control flag while preserving the lower nibble. Used in datagram and stream
    * protocols for frame sequencing.
    *
    * Use cases:
    * - Marking first frame in multi-frame datagram
    * - Marking middle frames in sequence
    * - Marking final frame in sequence
    * - Marking single-frame (only) messages
    *
    * @verbatim
    * @param target Pointer to the byte to modify
    * @endverbatim
    * @verbatim
    * @param flag Multi-frame flag value (MULTIFRAME_ONLY, MULTIFRAME_FIRST,
    * @endverbatim
    *             MULTIFRAME_MIDDLE, or MULTIFRAME_FINAL)
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    *
    * @attention Upper nibble is completely replaced. Lower nibble is preserved.
    * @attention Valid flag values are defined in openlcb_defines.h.
    *
    * @note This is typically used on the first byte of configuration memory
    *       or datagram protocol command bytes.
    *
    * @remark Constant time operation.
    *
    * @see protocol_config_mem_*.c - Uses for multi-frame config operations
    */
void OpenLcbUtilities_set_multi_frame_flag(uint8_t* target, uint8_t flag) {

    *target = *target & 0x0F;

    *target = *target | flag;

}

    /**
    * @brief Determines if a message is addressed (not global)
    *
    * @details Algorithm:
    * -# Read mti field from message
    * -# AND with MASK_DEST_ADDRESS_PRESENT
    * -# Return true if result equals MASK_DEST_ADDRESS_PRESENT
    * -# Return false otherwise
    *
    * Checks the MTI (Message Type Indicator) for the destination
    * address present bit. Addressed messages have a specific destination node,
    * while global messages are broadcast to all nodes.
    *
    * Use cases:
    * - Routing messages in the main state machine
    * - Determining if a message requires alias resolution
    * - Filtering messages for specific node handling
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @return true if message is addressed to a specific node, false if global
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    *
    * @note Global messages have dest_alias = 0 and MTI without address bit set.
    * @note Addressed messages require valid dest_alias or dest_ID.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_is_addressed_message_for_node - Checks specific node match
    * @see MASK_DEST_ADDRESS_PRESENT - MTI bit mask used for check
    */
bool OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg_t* openlcb_msg) {

    return ((openlcb_msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT);

}

    /**
    * @brief Counts the number of null bytes (0x00) in the message payload
    *
    * @details Algorithm:
    * -# Initialize count to 0
    * -# For i = 0 to payload_count - 1:
    *    -# If payload[i] equals 0x00:
    *       -# Increment count
    * -# Return count
    *
    * Scans through payload_count bytes and counts occurrences of 0x00.
    * Used primarily for validating SNIP (Simple Node Ident Info) message format,
    * which requires exactly 4 null terminators for the 4 string fields.
    *
    * Use cases:
    * - Validating received SNIP messages have correct format
    * - Verifying string-based protocol messages
    * - Detecting malformed messages
    *
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @return Count of null bytes found in the payload
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    *
    * @attention Only scans up to payload_count bytes, not the entire buffer.
    *
    * @note For SNIP messages, exactly 4 nulls are expected (manufacturer,
    *       model, hardware version, software version).
    *
    * @remark Linear time operation.
    *
    * @see protocol_snip.c - Uses this for SNIP validation
    */
uint8_t OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    uint8_t count = 0;

    for (int i = 0; i < openlcb_msg->payload_count; i++) {

        if (*openlcb_msg->payload[i] == 0x00) {

            count = count + 1;

        }

    }

    return count;

}

    /**
    * @brief Checks if an addressed message is for a specific node
    *
    * @details Algorithm:
    * -# Compare message dest_alias with node alias
    * -# If match, return true
    * -# Compare message dest_id with node ID
    * -# If match, return true
    * -# Return false if neither matches
    *
    * Compares the message destination (alias or ID) against the node's
    * alias and ID. Returns true if either matches, meaning this node is the
    * intended recipient.
    *
    * Use cases:
    * - Filtering incoming addressed messages in state machine
    * - Determining if node should process a protocol message
    * - Validating message routing in multi-node systems
    *
    * @verbatim
    * @param openlcb_node Pointer to the node structure to check
    * @endverbatim
    * @verbatim
    * @param openlcb_msg Pointer to the message structure
    * @endverbatim
    * @return true if message is addressed to this node, false otherwise
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Pointer must not be NULL. No NULL check performed.
    *
    * @attention Matches on EITHER alias OR node ID. A match on either field
    *            is sufficient to return true.
    *
    * @note Global messages (dest_alias = 0, dest_id = 0) will return false
    *       unless the node somehow has alias or ID = 0 (invalid state).
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_is_addressed_openlcb_message - Checks if message is addressed at all
    */
bool OpenLcbUtilities_is_addressed_message_for_node(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    if ((openlcb_node->alias == openlcb_msg->dest_alias) || (openlcb_node->id == openlcb_msg->dest_id)) {

        return true;

    } else {

        return false;

    }

}

    /**
    * @brief Checks if a producer event is assigned to a node
    *
    * @details Algorithm:
    * -# Loop through node's producer list (i = 0 to producers.count - 1)
    * -# Compare producers.list[i].event with event_id
    * -# If match found:
    *    -# Store i in *event_index
    *    -# Return true
    * -# If loop completes without match, return false
    *
    * Searches the node's producer event list for a matching event ID.
    * If found, returns true and stores the list index in event_index.
    *
    * Use cases:
    * - Processing Producer Identify requests
    * - Validating event ownership before sending reports
    * - Looking up producer state information
    *
    * @verbatim
    * @param openlcb_node Pointer to the node structure to search
    * @endverbatim
    * @verbatim
    * @param event_id The 64-bit event ID to search for
    * @endverbatim
    * @verbatim
    * @param event_index Pointer to store the list index if found
    * @endverbatim
    * @return true if event found in producer list, false otherwise
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning event_index is only valid when function returns true.
    *
    * @attention event_index is NOT modified if event is not found.
    *
    * @note Linear search with O(n) complexity where n = producer count.
    *
    * @remark O(n) worst case, where n = producer count.
    *
    * @see OpenLcbUtilities_is_consumer_event_assigned_to_node - Consumer version
    * @see protocol_event_transport.c - Uses this for event identification
    */
bool OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node_t* openlcb_node, event_id_t event_id, uint16_t *event_index) {

    for (int i = 0; i < openlcb_node->producers.count; i++) {

        if (openlcb_node->producers.list[i].event == event_id) {

            (*event_index) = (uint16_t) i;

            return true;

        }

    }

    return false;

}

    /**
    * @brief Checks if a consumer event is assigned to a node
    *
    * @details Algorithm:
    * -# Loop through node's consumer list (i = 0 to consumers.count - 1)
    * -# Compare consumers.list[i].event with event_id
    * -# If match found:
    *    -# Store i in *event_index
    *    -# Return true
    * -# If loop completes without match, return false
    *
    * Searches the node's consumer event list for a matching event ID.
    * If found, returns true and stores the list index in event_index.
    *
    * Use cases:
    * - Processing Consumer Identify requests
    * - Validating event subscriptions
    * - Looking up consumer state information
    *
    * @verbatim
    * @param openlcb_node Pointer to the node structure to search
    * @endverbatim
    * @verbatim
    * @param event_id The 64-bit event ID to search for
    * @endverbatim
    * @verbatim
    * @param event_index Pointer to store the list index if found
    * @endverbatim
    * @return true if event found in consumer list, false otherwise
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning event_index is only valid when function returns true.
    *
    * @attention event_index is NOT modified if event is not found.
    *
    * @note Linear search with O(n) complexity where n = consumer count.
    *
    * @remark O(n) worst case, where n = consumer count.
    *
    * @see OpenLcbUtilities_is_producer_event_assigned_to_node - Producer version
    * @see protocol_event_transport.c - Uses this for event identification
    */
bool OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node_t* openlcb_node, event_id_t event_id, uint16_t* event_index) {

    for (int i = 0; i < openlcb_node->consumers.count; i++) {

        if (openlcb_node->consumers.list[i].event == event_id) {

            (*event_index) = (uint16_t) i;

            return true;

        }

    }

    return false;

}

    /**
    * @brief Extracts a 6-byte node ID from a configuration memory buffer
    *
    * @details Algorithm:
    * -# Read buffer[0 + index], shift left 40 bits
    * -# Read buffer[1 + index], shift left 32 bits
    * -# Read buffer[2 + index], shift left 24 bits
    * -# Read buffer[3 + index], shift left 16 bits
    * -# Read buffer[4 + index], shift left 8 bits
    * -# Read buffer[5 + index]
    * -# OR all six bytes together
    * -# Return assembled 48-bit node ID
    *
    * Reads 6 bytes in big-endian format from the buffer starting at
    * the specified index and assembles them into a 48-bit node_id_t value.
    *
    * Use cases:
    * - Reading node IDs from configuration memory
    * - Parsing configuration data structures
    * - Extracting stored node identifiers
    *
    * @verbatim
    * @param buffer Pointer to the configuration memory buffer
    * @endverbatim
    * @verbatim
    * @param index Byte offset in the buffer where node ID starts
    * @endverbatim
    * @return The assembled 48-bit node ID
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes buffer has at least (index + 6) bytes available.
    * @warning Does not validate index bounds.
    * @warning Reading beyond buffer bounds returns undefined values.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_copy_node_id_to_config_mem_buffer - Reverse operation
    * @see OpenLcbUtilities_extract_node_id_from_openlcb_payload - Payload version
    */
node_id_t OpenLcbUtilities_extract_node_id_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index) {

    return (
            ((uint64_t) (*buffer)[0 + index] << 40) |
            ((uint64_t) (*buffer)[1 + index] << 32) |
            ((uint64_t) (*buffer)[2 + index] << 24) |
            ((uint64_t) (*buffer)[3 + index] << 16) |
            ((uint64_t) (*buffer)[4 + index] << 8) |
            ((uint64_t) (*buffer)[5 + index])
            );

}

    /**
    * @brief Extracts a 16-bit word from a configuration memory buffer
    *
    * @details Algorithm:
    * -# Read buffer[0 + index], shift left 8 bits
    * -# Read buffer[1 + index]
    * -# OR the two bytes together
    * -# Return assembled 16-bit value
    *
    * Reads 2 bytes in big-endian format from the buffer starting at
    * the specified index and assembles them into a 16-bit value.
    *
    * Use cases:
    * - Reading configuration parameters
    * - Extracting stored addresses or offsets
    * - Parsing configuration data structures
    *
    * @verbatim
    * @param buffer Pointer to the configuration memory buffer
    * @endverbatim
    * @verbatim
    * @param index Byte offset in the buffer where word starts
    * @endverbatim
    * @return The assembled 16-bit value
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes buffer has at least (index + 2) bytes available.
    * @warning Does not validate index bounds.
    * @warning Reading beyond buffer bounds returns undefined values.
    *
    * @note Byte order is big-endian per OpenLCB specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_extract_word_from_openlcb_payload - Payload version
    */
uint16_t OpenLcbUtilities_extract_word_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index) {

    return (
            ((uint16_t) (*buffer)[0 + index] << 8) |
            ((uint16_t) (*buffer)[1 + index])
            );

}

    /**
    * @brief Copies a 6-byte node ID into a configuration memory buffer
    *
    * @details Algorithm:
    * -# Loop from byte index 5 down to 0 (big-endian order)
    * -# Extract least significant byte of node_id
    * -# Write byte to buffer[i + index]
    * -# Right-shift node_id by 8 bits
    * -# Repeat for all 6 bytes
    *
    * Writes the node ID in big-endian format to the buffer starting
    * at the specified index.
    *
    * Use cases:
    * - Storing node IDs in configuration memory
    * - Writing configuration data structures
    * - Persisting node identifiers
    *
    * @verbatim
    * @param buffer Pointer to the configuration memory buffer
    * @endverbatim
    * @verbatim
    * @param node_id The 48-bit node ID to write
    * @endverbatim
    * @verbatim
    * @param index Byte offset in the buffer where node ID should be written
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes buffer has at least (index + 6) bytes available.
    * @warning Does not validate index bounds.
    *
    * @note Byte order is big-endian per OpenLCB specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_extract_node_id_from_config_mem_buffer - Reverse operation
    * @see OpenLcbUtilities_copy_node_id_to_openlcb_payload - Payload version
    */
void OpenLcbUtilities_copy_node_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, node_id_t node_id, uint8_t index) {

    for (int i = 5; i >= 0; i--) {

        (*buffer)[i + index] = node_id & 0xFF;
        node_id = node_id >> 8;

    }

}

    /**
    * @brief Copies an 8-byte event ID into a configuration memory buffer
    *
    * @details Algorithm:
    * -# Loop from byte index 7 down to 0 (big-endian order)
    * -# Extract least significant byte of event_id
    * -# Write byte to buffer[i + index]
    * -# Right-shift event_id by 8 bits
    * -# Repeat for all 8 bytes
    *
    * Writes the event ID in big-endian format to the buffer starting
    * at the specified index.
    *
    * Use cases:
    * - Storing event IDs in configuration memory
    * - Writing producer/consumer event configurations
    * - Persisting event identifiers
    *
    * @verbatim
    * @param buffer Pointer to the configuration memory buffer
    * @endverbatim
    * @verbatim
    * @param event_id The 64-bit event ID to write
    * @endverbatim
    * @verbatim
    * @param index Byte offset in the buffer where event ID should be written
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes buffer has at least (index + 8) bytes available.
    * @warning Does not validate index bounds.
    *
    * @note Byte order is big-endian per OpenLCB specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_copy_config_mem_buffer_to_event_id - Reverse operation
    * @see OpenLcbUtilities_copy_event_id_to_openlcb_payload - Payload version
    */
void OpenLcbUtilities_copy_event_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, event_id_t event_id, uint8_t index) {


    for (int i = 7; i >= 0; i--) {

        (*buffer)[i + index] = event_id & 0xFF;
        event_id = event_id >> 8;

    }

}

    /**
    * @brief Extracts an 8-byte event ID from a configuration memory buffer
    *
    * @details Algorithm:
    * -# Initialize retval to 0
    * -# For i = 0 to 7:
    *    -# Shift retval left by 8 bits
    *    -# OR with buffer[i + index] masked with 0xFF
    * -# Return assembled event ID
    *
    * Reads 8 bytes in big-endian format from the buffer starting at
    * the specified index and assembles them into a 64-bit event_id_t value.
    *
    * Use cases:
    * - Reading event IDs from configuration memory
    * - Loading producer/consumer event configurations
    * - Retrieving stored event identifiers
    *
    * @verbatim
    * @param buffer Pointer to the configuration memory buffer
    * @endverbatim
    * @verbatim
    * @param index Byte offset in the buffer where event ID starts
    * @endverbatim
    * @return The assembled 64-bit event ID
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Assumes buffer has at least (index + 8) bytes available.
    * @warning Does not validate index bounds.
    * @warning Reading beyond buffer bounds returns undefined values.
    *
    * @note Byte order is big-endian per OpenLCB specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_copy_event_id_to_config_mem_buffer - Reverse operation
    * @see OpenLcbUtilities_extract_event_id_from_openlcb_payload - Payload version
    */
event_id_t OpenLcbUtilities_copy_config_mem_buffer_to_event_id(configuration_memory_buffer_t *buffer, uint8_t index) {


    event_id_t retval = 0L;

    for (int i = 0; i <= 7; i++) {

        retval = retval << 8;
        retval |= (*buffer)[i + index] & 0xFF;

    }

    return retval;

}

    /**
    * @brief Loads a configuration memory write success reply message header
    *
    * @details Algorithm:
    * -# Clear outgoing message payload_count to 0
    * -# Load message header with source/dest from state machine info
    * -# Set MTI to MTI_DATAGRAM
    * -# Copy CONFIG_MEM_CONFIGURATION to payload offset 0
    * -# Copy original command byte + OK_OFFSET to payload offset 1
    * -# Copy write request address (4 bytes) to payload offset 2
    * -# If encoding is ADDRESS_SPACE_IN_BYTE_6:
    *    -# Copy address space byte from incoming message to payload offset 6
    * -# Set outgoing_msg_info.valid to false (caller must enable)
    *
    * Constructs a datagram reply message indicating a configuration
    * memory write operation succeeded. Includes the original request address.
    * Handles both 4-byte and 6-byte address encoding formats.
    *
    * Use cases:
    * - Acknowledging successful configuration memory writes
    * - Confirming data was stored correctly
    * - Completing write request transactions
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context with message buffers
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to original write request details
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning statemachine_info->outgoing_msg_info.msg_ptr must be valid.
    *
    * @attention Sets outgoing_msg_info.valid to false by default. Application
    *            must set to true to actually send the reply.
    *
    * @attention Address space byte (byte 6) only included if encoding is
    *            ADDRESS_SPACE_IN_BYTE_6.
    *
    * @note Reply format follows OpenLCB Configuration Memory protocol specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_load_config_mem_reply_write_fail_message_header - Failure version
    * @see protocol_config_mem_*.c - Uses this for success responses
    */
void OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

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

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] + CONFIG_MEM_REPLY_OK_OFFSET,
            1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            config_mem_write_request_info->address,
            2);

    if (config_mem_write_request_info->encoding == ADDRESS_SPACE_IN_BYTE_6) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6],
                6);

    }


    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Loads a configuration memory write failure reply message header
    *
    * @details Algorithm:
    * -# Clear outgoing message payload_count to 0
    * -# Load message header with source/dest from state machine info
    * -# Set MTI to MTI_DATAGRAM
    * -# Copy CONFIG_MEM_CONFIGURATION to payload offset 0
    * -# Copy original command byte + FAIL_OFFSET to payload offset 1
    * -# Copy write request address (4 bytes) to payload offset 2
    * -# If encoding is ADDRESS_SPACE_IN_BYTE_6:
    *    -# Copy address space byte from incoming message to payload offset 6
    *    -# Copy error_code (2 bytes) to payload offset 7
    * -# Else:
    *    -# Copy error_code (2 bytes) to payload offset 6
    * -# Set outgoing_msg_info.valid to false (caller must enable)
    *
    * Constructs a datagram reply message indicating a configuration
    * memory write operation failed. Includes the error code and original
    * request address. Handles both 4-byte and 6-byte address encoding formats.
    *
    * Use cases:
    * - Responding to write requests for read-only memory spaces
    * - Indicating invalid address ranges
    * - Reporting configuration memory errors
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context with message buffers
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to original write request details
    * @endverbatim
    * @verbatim
    * @param error_code 16-bit error code indicating failure reason
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning statemachine_info->outgoing_msg_info.msg_ptr must be valid.
    *
    * @attention Sets outgoing_msg_info.valid to false by default. Application
    *            must set to true to actually send the reply.
    *
    * @attention Error code placement depends on address encoding:
    *            - ADDRESS_SPACE_IN_BYTE_6: error at offset 7
    *            - Other encoding: error at offset 6
    *
    * @note Reply format follows OpenLCB Configuration Memory protocol specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_load_config_mem_reply_write_ok_message_header - Success version
    * @see protocol_config_mem_*.c - Uses this for error responses
    */
void OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, uint16_t error_code) {

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

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] + CONFIG_MEM_REPLY_FAIL_OFFSET,
            1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            config_mem_write_request_info->address,
            2);

    if (config_mem_write_request_info->encoding == ADDRESS_SPACE_IN_BYTE_6) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6],
                6);

        OpenLcbUtilities_copy_word_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
                error_code,
                7);

    } else {

        OpenLcbUtilities_copy_word_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
                error_code,
                6);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Loads a configuration memory read success reply message header
    *
    * @details Algorithm:
    * -# Clear outgoing message payload_count to 0
    * -# Load message header with source/dest from state machine info
    * -# Set MTI to MTI_DATAGRAM
    * -# Copy CONFIG_MEM_CONFIGURATION to payload offset 0
    * -# Copy original command byte + OK_OFFSET to payload offset 1
    * -# Copy read request address (4 bytes) to payload offset 2
    * -# If encoding is ADDRESS_SPACE_IN_BYTE_6:
    *    -# Copy address space byte from incoming message to payload offset 6
    * -# Set outgoing_msg_info.valid to false (caller must enable)
    *
    * Constructs a datagram reply message header indicating a configuration
    * memory read operation succeeded. Sets up the header fields; actual data bytes
    * must be added separately by the caller. Handles both 4-byte and 6-byte
    * address encoding formats.
    *
    * Use cases:
    * - Preparing successful configuration memory read replies
    * - Setting up header before copying data bytes
    * - Acknowledging valid read requests
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context with message buffers
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to original read request details
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning statemachine_info->outgoing_msg_info.msg_ptr must be valid.
    *
    * @attention This function only loads the HEADER. The caller must append
    *            the actual data bytes using copy_byte_array or similar functions.
    *
    * @attention Sets outgoing_msg_info.valid to false by default. Application
    *            must set to true after adding data to actually send the reply.
    *
    * @attention Address space byte (byte 6) only included if encoding is
    *            ADDRESS_SPACE_IN_BYTE_6.
    *
    * @note Reply format follows OpenLCB Configuration Memory protocol specification.
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_load_config_mem_reply_read_fail_message_header - Failure version
    * @see OpenLcbUtilities_copy_byte_array_to_openlcb_payload - For adding data
    * @see protocol_config_mem_*.c - Uses this for success responses
    */
void OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

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

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] + CONFIG_MEM_REPLY_OK_OFFSET,
            1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            config_mem_read_request_info->address,
            2);

    if (config_mem_read_request_info->encoding == ADDRESS_SPACE_IN_BYTE_6) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6], 
                6);

    }


    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Loads a configuration memory read failure reply message header
    *
    * @details Algorithm:
    * -# Clear outgoing message payload_count to 0
    * -# Load message header with source/dest from state machine info
    * -# Set MTI to MTI_DATAGRAM
    * -# Copy CONFIG_MEM_CONFIGURATION to payload offset 0
    * -# Copy original command byte + FAIL_OFFSET to payload offset 1
    * -# Copy read request address (4 bytes) to payload offset 2
    * -# If encoding is ADDRESS_SPACE_IN_BYTE_6:
    *    -# Copy address space byte from incoming message to payload offset 6
    * -# Copy error_code (2 bytes) to payload at config_mem_read_request_info->data_start
    *
    * Constructs a datagram reply message indicating a configuration
    * memory read operation failed. Includes the error code at the data start
    * position where data would have been placed. Handles both 4-byte and 6-byte
    * address encoding formats.
    *
    * Use cases:
    * - Responding to read requests for invalid addresses
    * - Indicating unavailable memory spaces
    * - Reporting configuration memory errors
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context with message buffers
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to original read request details
    * @endverbatim
    * @verbatim
    * @param error_code 16-bit error code indicating failure reason
    * @endverbatim
    *
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning Pointer must not be NULL. No NULL check performed.
    * @warning statemachine_info->outgoing_msg_info.msg_ptr must be valid.
    *
    * @attention Error code is placed at config_mem_read_request_info->data_start
    *            offset, where the actual data would have been.
    *
    * @attention Address space byte (byte 6) only included if encoding is
    *            ADDRESS_SPACE_IN_BYTE_6.
    *
    * @note Reply format follows OpenLCB Configuration Memory protocol specification.
    * @note Does NOT set outgoing_msg_info.valid (differs from write functions).
    *
    * @remark Constant time operation.
    *
    * @see OpenLcbUtilities_load_config_mem_reply_read_ok_message_header - Success version
    * @see protocol_config_mem_*.c - Uses this for error responses
    */
void OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info, uint16_t error_code) {

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

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] + CONFIG_MEM_REPLY_FAIL_OFFSET,
            1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            config_mem_read_request_info->address,
            2);

    if (config_mem_read_request_info->encoding == ADDRESS_SPACE_IN_BYTE_6) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6], 
                6);

    }

    OpenLcbUtilities_copy_word_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            error_code,
            config_mem_read_request_info->data_start);

}

 bool OpenLcbUtilities_is_event_id_in_consumer_ranges(openlcb_node_t *openlcb_node, event_id_t event_id) {

     event_id_range_t *range;

     for (int i = 0; i < openlcb_node->consumers.range_count; i++)
     {

         range = &openlcb_node->consumers.range_list[i];
         event_id_t start_event = range->start_base;
         event_id_t end_event = range->start_base + range->event_count;

         if ((event_id >= start_event) && (event_id <= end_event)) {

             return true;

         }
     }

     return false;
 }

 bool OpenLcbUtilities_is_event_id_in_producer_ranges(openlcb_node_t *openlcb_node, event_id_t event_id) {

     event_id_range_t *range;

     for (int i = 0; i < openlcb_node->producers.range_count; i++)
     {

         range = &openlcb_node->producers.range_list[i];
         event_id_t start_event = range->start_base;
         event_id_t end_event = range->start_base + range->event_count;

         if ((event_id >= start_event) && (event_id <= end_event))
         {

             return true;
         }
     }

     return false;
 }

 event_id_t OpenLcbUtilities_generate_event_range_id(event_id_t base_event_id, event_range_count_enum count)
 {

     uint32_t bitsNeeded = 0;
     uint32_t temp = count - 1;
     while (temp > 0) {

         bitsNeeded++;
         temp >>= 1;

     }

     event_id_t mask = (1ULL << bitsNeeded) - 1;
     event_id_t rangeEventID = (base_event_id & ~mask) | mask;

     return rangeEventID;

 }
