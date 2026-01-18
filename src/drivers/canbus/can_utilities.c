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
 * @file can_utilities.c
 * @brief Implementation of utility functions for CAN frame buffers
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "can_utilities.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"

#include "../../openlcb/openlcb_utilities.h"

    /**
     * @brief Clears all fields of a CAN message structure to zero
     *
     * @details Algorithm:
     * -# Set identifier field to 0
     * -# Set payload_count field to 0
     * -# Iterate through all 8 payload bytes
     * -# Set each payload byte to 0x00
     *
     * Use cases:
     * - Initializing a new CAN message structure
     * - Resetting a CAN message before reuse
     * - Clearing message data before filling with new content
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer to clear
     * @endverbatim
     *
     * @warning NULL pointer will cause undefined behavior
     * @warning Does not check if buffer is currently in use
     *
     * @attention Ensure pointer is valid before calling
     *
     * @see CanUtilities_load_can_message - Loads message with data
     */
void CanUtilities_clear_can_message(can_msg_t *can_msg) {

    can_msg->identifier = 0;
    can_msg->payload_count = 0;

    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++) {

        can_msg->payload[i] = 0x00;

    }

}

    /**
     * @brief Loads a CAN message with identifier, payload size, and data bytes
     *
     * @details Algorithm:
     * -# Store identifier value in identifier field
     * -# Store payload size in payload_count field
     * -# Store byte1 through byte8 in payload array indices 0-7
     *
     * Use cases:
     * - Creating CAN control frames (CID, RID, AMD)
     * - Building CAN message frames from OpenLCB messages
     * - Preparing CAN frames for transmission
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer to initialize
     * @param identifier 29-bit CAN extended identifier value
     * @param payload_size Number of valid payload bytes (0-8)
     * @param byte1 First payload byte
     * @param byte2 Second payload byte
     * @param byte3 Third payload byte
     * @param byte4 Fourth payload byte
     * @param byte5 Fifth payload byte
     * @param byte6 Sixth payload byte
     * @param byte7 Seventh payload byte
     * @param byte8 Eighth payload byte
     * @endverbatim
     *
     * @warning NULL pointer will cause undefined behavior
     * @warning Payload size values greater than 8 may cause issues
     *
     * @attention All 8 byte parameters must be provided even if payload_size is less than 8
     *
     * @see CanUtilities_clear_can_message - Clears message structure
     * @see CanUtilities_copy_64_bit_to_can_message - Alternative for 8-byte payloads
     */
void CanUtilities_load_can_message(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8) {

    can_msg->identifier = identifier;
    can_msg->payload_count = payload_size;
    can_msg->payload[0] = byte1;
    can_msg->payload[1] = byte2;
    can_msg->payload[2] = byte3;
    can_msg->payload[3] = byte4;
    can_msg->payload[4] = byte5;
    can_msg->payload[5] = byte6;
    can_msg->payload[6] = byte7;
    can_msg->payload[7] = byte8;

}

    /**
     * @brief Copies a 48-bit Node ID into CAN message payload
     *
     * @details Algorithm:
     * -# Validate start_offset is not greater than 2
     * -# If invalid offset, return 0
     * -# Calculate payload_count as 6 + start_offset
     * -# Iterate backward through 6 byte positions (from start_offset+5 down to start_offset)
     * -# For each position:
     *    - Extract lowest byte from node_id using mask 0xFF
     *    - Store byte in current payload position
     *    - Right-shift node_id by 8 bits for next byte
     * -# Return payload_count
     *
     * Use cases:
     * - Building AMD (Alias Map Definition) frames
     * - Building Verified Node ID messages
     * - Sending Node ID in CAN control frames
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer to receive Node ID
     * @param node_id 48-bit OpenLCB Node ID to copy
     * @param start_offset Starting position in payload array (0-2)
     * @endverbatim
     *
     * @return Number of bytes copied (always 6 + start_offset), or 0 if offset invalid
     *
     * @warning Returns 0 if start_offset is greater than 2
     * @warning NULL pointer will cause undefined behavior
     *
     * @attention Valid start_offset range is 0-2 to ensure 6-byte Node ID fits in 8-byte payload
     * @attention Updates payload_count to start_offset + 6
     *
     * @see CanUtilities_extract_can_payload_as_node_id - Extracts Node ID from payload
     */
uint8_t CanUtilities_copy_node_id_to_payload(can_msg_t *can_msg, uint64_t node_id, uint8_t start_offset) {

    if (start_offset > 2) {

        return 0;

    }
    can_msg->payload_count = 6 + start_offset;

    for (int i = (start_offset + 5); i >= (0 + start_offset); i--) { // This is a count down...

        can_msg->payload[i] = node_id & 0xFF;
        node_id = node_id >> 8;

    }

    return can_msg->payload_count;

}

    /**
     * @brief Copies payload data from OpenLCB message to CAN message
     *
     * @details Algorithm:
     * -# Set CAN payload_count to 0
     * -# Initialize copy counter to 0
     * -# If OpenLCB payload_count is 0, return 0
     * -# Iterate from can_start_index to end of CAN payload (8 bytes)
     * -# For each CAN payload position:
     *    - Copy byte from OpenLCB payload at openlcb_start_index
     *    - Increment openlcb_start_index
     *    - Increment copy counter
     *    - If reached end of OpenLCB payload, break
     * -# Set CAN payload_count to can_start_index + copy counter
     * -# Return copy counter
     *
     * Use cases:
     * - Fragmenting large OpenLCB messages into CAN frames
     * - Creating multi-frame CAN messages
     * - Building first, middle, or last CAN frames from OpenLCB datagram
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message buffer (source)
     * @param can_msg Pointer to CAN message buffer (destination)
     * @param openlcb_start_index Starting index in OpenLCB payload
     * @param can_start_index Starting index in CAN payload (typically 0 or 2)
     * @endverbatim
     *
     * @return Number of bytes copied from OpenLCB to CAN payload
     *
     * @warning NULL pointers will cause undefined behavior
     * @warning Does not validate index values are within bounds
     *
     * @attention Caller must ensure indices are valid
     * @attention Updates CAN payload_count to reflect total bytes
     *
     * @see CanUtilities_append_can_payload_to_openlcb_payload - Reverse operation
     */
uint8_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint16_t openlcb_start_index, uint8_t can_start_index) {

    can_msg->payload_count = 0;
    uint8_t count = 0;

    if (openlcb_msg->payload_count == 0) {

        return 0;
    }

    for (int i = can_start_index; i < LEN_CAN_BYTE_ARRAY; i++) {

        can_msg->payload[i] = *openlcb_msg->payload[openlcb_start_index];

        openlcb_start_index++;

        count++;

        // have we hit the end of the OpenLcb payload?
        if (openlcb_start_index >= openlcb_msg->payload_count) {

            break;

        }

    }

    can_msg->payload_count = can_start_index + count;

    return count;
}

    /**
     * @brief Appends CAN payload data to OpenLCB message payload
     *
     * @details Algorithm:
     * -# Initialize result counter to 0
     * -# Get OpenLCB buffer length based on payload type
     * -# Iterate from can_start_index to end of CAN payload_count
     * -# For each CAN payload byte:
     *    - Check if OpenLCB payload_count is less than buffer capacity
     *    - If space available:
     *      - Copy byte from CAN payload to OpenLCB payload at current count
     *      - Increment OpenLCB payload_count
     *      - Increment result counter
     *    - If no space, break loop
     * -# Return result counter
     *
     * Use cases:
     * - Assembling multi-frame CAN messages into OpenLCB message
     * - Processing datagram frames (first, middle, last)
     * - Reconstructing fragmented OpenLCB messages
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message buffer (destination)
     * @param can_msg Pointer to CAN message buffer (source)
     * @param can_start_index Starting index in CAN payload to copy from
     * @endverbatim
     *
     * @return Number of bytes copied from CAN to OpenLCB payload
     *
     * @warning NULL pointers will cause undefined behavior
     * @warning Buffer overflow possible if OpenLCB payload exceeds capacity
     *
     * @attention Stops copying when OpenLCB buffer capacity reached
     * @attention Updates OpenLCB payload_count as bytes are added
     *
     * @see CanUtilities_copy_openlcb_payload_to_can_payload - Reverse operation
     */
uint8_t CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint8_t can_start_index) {

    uint8_t result = 0;
    uint16_t buffer_len = OpenLcbUtilities_payload_type_to_len(openlcb_msg->payload_type);

    for (int i = can_start_index; i < can_msg->payload_count; i++) {

        if (openlcb_msg->payload_count < buffer_len) {

            *openlcb_msg->payload[openlcb_msg->payload_count] = can_msg->payload[i];

            openlcb_msg->payload_count++;

            result++;

        } else {

            break;

        }
    }

    return result;
}

    /**
     * @brief Copies a 64-bit value into CAN message payload
     *
     * @details Algorithm:
     * -# Iterate backward through 8 byte positions (index 7 down to 0)
     * -# For each position:
     *    - Extract lowest byte from data using mask 0xFF
     *    - Store byte in current payload position
     *    - Right-shift data by 8 bits for next byte
     * -# Set payload_count to 8
     * -# Return payload_count (8)
     *
     * Use cases:
     * - Loading Event IDs into CAN message payloads
     * - Sending 64-bit timestamps or identifiers
     * - Building messages with 8-byte data fields
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer to receive data
     * @param data 64-bit value to copy into payload
     * @endverbatim
     *
     * @return Number of bytes copied (always 8)
     *
     * @warning NULL pointer will cause undefined behavior
     *
     * @attention Always sets payload_count to 8
     * @attention Data is stored in big-endian format (MSB first)
     *
     * @see CanUtilities_copy_node_id_to_payload - Similar for Node IDs (6 bytes)
     */
uint8_t CanUtilities_copy_64_bit_to_can_message(can_msg_t *can_msg, uint64_t data) {

    for (int i = 7; i >= 0; i--) {

        can_msg->payload[i] = data & 0xFF;
        data = data >> 8;

    }

    can_msg->payload_count = 8;

    return can_msg->payload_count;

}

    /**
     * @brief Copies identifier and payload from one CAN message to another
     *
     * @details Algorithm:
     * -# Copy source identifier to target identifier
     * -# Iterate through payload bytes from 0 to source payload_count
     * -# For each byte, copy from source payload to target payload
     * -# Copy source payload_count to target payload_count
     * -# Return target payload_count
     *
     * Use cases:
     * - Duplicating CAN messages for logging
     * - Creating retry copies of messages
     * - Forwarding messages through gateways
     *
     * @verbatim
     * @param can_msg_source Pointer to source CAN message buffer
     * @param can_msg_target Pointer to destination CAN message buffer
     * @endverbatim
     *
     * @return Number of payload bytes copied
     *
     * @warning NULL pointers will cause undefined behavior
     * @warning Does not copy allocation state or other metadata
     *
     * @attention Only copies identifier and payload fields
     * @attention Target payload_count is set to match source
     *
     * @see CanUtilities_clear_can_message - For initializing target first
     */
uint8_t CanUtilities_copy_can_message(can_msg_t *can_msg_source, can_msg_t *can_msg_target) {

    can_msg_target->identifier = can_msg_source->identifier;

    for (int i = 0; i < can_msg_source->payload_count; i++) {

        can_msg_target->payload[i] = can_msg_source->payload[i];

    }

    can_msg_target->payload_count = can_msg_source->payload_count;

    return can_msg_target->payload_count;

}

    /**
     * @brief Extracts a 48-bit Node ID from CAN message payload
     *
     * @details Algorithm:
     * -# Extract byte at payload[0], shift left 40 bits
     * -# Extract byte at payload[1], shift left 32 bits, OR with result
     * -# Extract byte at payload[2], shift left 24 bits, OR with result
     * -# Extract byte at payload[3], shift left 16 bits, OR with result
     * -# Extract byte at payload[4], shift left 8 bits, OR with result
     * -# Extract byte at payload[5], OR with result
     * -# Return combined 48-bit Node ID value
     *
     * Use cases:
     * - Processing AMD (Alias Map Definition) frames
     * - Extracting Node ID from Verified Node ID messages
     * - Reading Node ID from CAN control frames
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer containing Node ID
     * @endverbatim
     *
     * @return 48-bit Node ID extracted from payload bytes 0-5
     *
     * @warning NULL pointer will cause undefined behavior
     * @warning Does not validate payload contains at least 6 bytes
     *
     * @attention Assumes payload bytes 0-5 contain valid Node ID
     * @attention Returns Node ID in big-endian format
     *
     * @see CanUtilities_copy_node_id_to_payload - Reverse operation
     */
node_id_t CanUtilities_extract_can_payload_as_node_id(can_msg_t *can_msg) {

    return (
            ((node_id_t) can_msg->payload[0] << 40) |
            ((node_id_t) can_msg->payload[1] << 32) |
            ((node_id_t) can_msg->payload[2] << 24) |
            ((node_id_t) can_msg->payload[3] << 16) |
            ((node_id_t) can_msg->payload[4] << 8) |
            ((node_id_t) can_msg->payload[5]));

}

    /**
     * @brief Extracts the 12-bit source alias from CAN identifier
     *
     * @details Algorithm:
     * -# Apply mask 0x00000FFF to identifier field
     * -# Return masked result containing bits 0-11 (source alias)
     *
     * Use cases:
     * - Identifying the source node of received CAN frames
     * - Routing messages to correct node handlers
     * - Alias conflict detection during login
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer containing identifier
     * @endverbatim
     *
     * @return 12-bit source alias (0x000-0xFFF)
     *
     * @warning NULL pointer will cause undefined behavior
     *
     * @attention Valid alias range is 0x001-0xFFF (0x000 is invalid)
     *
     * @see CanUtilities_extract_dest_alias_from_can_message - Gets destination alias
     */
uint16_t CanUtilities_extract_source_alias_from_can_identifier(can_msg_t *can_msg) {

    return (can_msg->identifier & 0x000000FFF);

}

    /**
     * @brief Extracts the destination alias from CAN message
     *
     * @details Algorithm:
     * -# Check identifier against MASK_CAN_FRAME_TYPE to determine frame type
     * -# If OPENLCB_MESSAGE_STANDARD_FRAME_TYPE or CAN_FRAME_TYPE_STREAM:
     *    - Check if MASK_CAN_DEST_ADDRESS_PRESENT bit is set
     *    - If set:
     *      - Extract low nibble of payload[0], shift left 8 bits
     *      - OR with payload[1]
     *      - Return 12-bit destination alias
     *    - If not set, break to return 0
     * -# If CAN_FRAME_TYPE_DATAGRAM_* (any datagram type):
     *    - Extract bits 12-23 from identifier
     *    - Apply mask 0x00000FFF
     *    - Return 12-bit destination alias
     * -# Default (no destination present):
     *    - Return 0
     *
     * Use cases:
     * - Determining if message is addressed to a specific node
     * - Routing addressed messages to correct node
     * - Processing datagram frames
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer to examine
     * @endverbatim
     *
     * @return 12-bit destination alias (0x000-0xFFF), or 0 if no destination present
     *
     * @warning NULL pointer will cause undefined behavior
     * @warning Returns 0 for both "no destination" and "destination = 0"
     *
     * @note Broadcast messages return 0 (no specific destination)
     * @note Different frame types store destination in different locations
     *
     * @attention Handles multiple CAN frame formats (standard, datagram, stream)
     *
     * @see CanUtilities_extract_source_alias_from_can_identifier - Gets source alias
     */
uint16_t CanUtilities_extract_dest_alias_from_can_message(can_msg_t *can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

    case OPENLCB_MESSAGE_STANDARD_FRAME_TYPE:
    case CAN_FRAME_TYPE_STREAM:

        if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT) {

            return ((uint16_t)((can_msg->payload[0] & 0x0F) << 8) | (uint16_t)(can_msg->payload[1]));
        }

        break;

    case CAN_FRAME_TYPE_DATAGRAM_ONLY:
    case CAN_FRAME_TYPE_DATAGRAM_FIRST:
    case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:
    case CAN_FRAME_TYPE_DATAGRAM_FINAL:

        return (can_msg->identifier >> 12) & 0x000000FFF;

    default:

        break;

    }

    return 0;
}

    /**
     * @brief Converts CAN MTI to standard OpenLCB MTI
     *
     * @details Algorithm:
     * -# Initialize mti variable to 0
     * -# Check identifier against MASK_CAN_FRAME_TYPE to determine frame type
     * -# If OPENLCB_MESSAGE_STANDARD_FRAME_TYPE or CAN_FRAME_TYPE_STREAM:
     *    - Extract bits 12-23 from identifier (shift right 12, mask 0x0FFF)
     *    - Check if mti is PCER first/middle/last frame
     *    - If so, convert to common MTI_PC_EVENT_REPORT_WITH_PAYLOAD
     *    - Return converted mti
     * -# If CAN_FRAME_TYPE_DATAGRAM_* (any datagram type):
     *    - Return MTI_DATAGRAM
     * -# Default (unrecognized frame type):
     *    - Return 0
     *
     * Use cases:
     * - Processing received CAN frames
     * - Dispatching messages to protocol handlers
     * - Converting between CAN and OpenLCB message formats
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer containing identifier
     * @endverbatim
     *
     * @return 16-bit OpenLCB MTI value, or 0 if frame type is unknown
     *
     * @warning NULL pointer will cause undefined behavior
     * @warning Returns 0 for unrecognized frame types
     *
     * @note PCER first/middle/last frames are converted to common PCER MTI
     * @note Datagram frames (all types) return MTI_DATAGRAM
     *
     * @attention CAN control frames (CID, RID, AMD) return 0 as they have no OpenLCB MTI
     *
     * @see CanUtilities_is_openlcb_message - Checks if frame contains OpenLCB message
     */
uint16_t CanUtilities_convert_can_mti_to_openlcb_mti(can_msg_t *can_msg) {

    uint16_t mti = 0;

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

    case OPENLCB_MESSAGE_STANDARD_FRAME_TYPE:
    case CAN_FRAME_TYPE_STREAM:

        mti = (can_msg->identifier >> 12) & 0x0FFF;

        switch (mti) {

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD_FIRST:
        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD_MIDDLE:
        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD_LAST:

            mti = MTI_PC_EVENT_REPORT_WITH_PAYLOAD;
            break;

        }

        return mti;

    case CAN_FRAME_TYPE_DATAGRAM_ONLY:
    case CAN_FRAME_TYPE_DATAGRAM_FIRST:
    case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:
    case CAN_FRAME_TYPE_DATAGRAM_FINAL:

        return MTI_DATAGRAM;

    default:

        break;

    }

    return 0;
}

    /**
     * @brief Counts NULL bytes in CAN message payload (static helper)
     *
     * @details Algorithm:
     * -# Initialize count to 0
     * -# Iterate through payload from 0 to payload_count
     * -# For each byte:
     *    - If byte equals 0x00, increment count
     * -# Return count
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer with payload
     * @endverbatim
     *
     * @return Count of NULL bytes in CAN payload
     *
     * @note This is an internal helper function
     * @note Only counts bytes within valid payload_count range
     */
static uint8_t _count_nulls_in_can_payload(can_msg_t *can_msg) {

    uint8_t count = 0;

    for (int i = 0; i < can_msg->payload_count; i++) {

        if (can_msg->payload[i] == 0x00) {

            count++;

        }

    }

    return count;
}

    /**
     * @brief Counts NULL bytes in both CAN and OpenLCB message payloads
     *
     * @details Algorithm:
     * -# Call _count_nulls_in_can_payload() to count CAN payload NULLs
     * -# Call OpenLcbUtilities_count_nulls_in_openlcb_payload() for OpenLCB payload NULLs
     * -# Return sum of both counts
     *
     * Use cases:
     * - Validating SNIP (Simple Node Information Protocol) messages
     * - Checking for proper null-terminated strings
     * - Verifying message format compliance
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message buffer with payload
     * @param can_msg Pointer to CAN message buffer with payload
     * @endverbatim
     *
     * @return Total count of NULL bytes found in both payloads
     *
     * @warning NULL pointers will cause undefined behavior
     *
     * @note Only counts bytes within valid payload_count of each message
     * @note SNIP messages should have exactly 6 NULL terminators
     *
     * @attention Used primarily for SNIP message validation
     *
     * @see ProtocolSnip_validate_snip_reply - Uses this for validation
     */
uint8_t CanUtilities_count_nulls_in_payloads(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg) {

    return _count_nulls_in_can_payload(can_msg) + OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg);

}

    /**
     * @brief Tests if CAN frame contains an OpenLCB message
     *
     * @details Algorithm:
     * -# Apply CAN_OPENLCB_MSG mask to identifier
     * -# Compare masked result with CAN_OPENLCB_MSG value
     * -# Return true if equal (frame type bit indicates OpenLCB message)
     * -# Return false otherwise (CAN-only control frame)
     *
     * Use cases:
     * - Filtering received CAN frames
     * - Separating OpenLCB messages from CAN control frames
     * - Routing frames to appropriate handlers
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer to test
     * @endverbatim
     *
     * @return True if frame contains OpenLCB message, false if CAN-only frame
     *
     * @warning NULL pointer will cause undefined behavior
     *
     * @note CAN control frames (CID, RID, AMD, AME, AMR) return false
     * @note All OpenLCB message types (events, datagrams, streams) return true
     *
     * @attention Does not validate message contents, only checks frame type bit
     *
     * @see CanUtilities_convert_can_mti_to_openlcb_mti - Converts MTI for OpenLCB messages
     */
bool CanUtilities_is_openlcb_message(can_msg_t *can_msg) {

    return (can_msg->identifier & CAN_OPENLCB_MSG) == CAN_OPENLCB_MSG;

}
