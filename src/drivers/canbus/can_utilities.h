/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file can_utilities.h
 * @brief Utility functions for manipulating CAN frame buffers
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_UTILITIES__
#define __DRIVERS_CANBUS_CAN_UTILITIES__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Clears all fields of a CAN message structure to zero
     *
     * @details Sets the identifier, payload count, and all payload bytes to zero,
     * effectively resetting the CAN message structure to a clean state.
     *
     * Use cases:
     * - Initializing a new CAN message structure
     * - Resetting a CAN message before reuse
     * - Clearing message data before filling with new content
     *
     * @param can_msg Pointer to CAN message buffer to clear
     *
     * @warning NULL pointer will cause undefined behavior
     * @warning Does not check if buffer is currently in use
     *
     * @attention Ensure pointer is valid before calling
     *
     * @see CanUtilities_load_can_message - Loads message with data
     */
    extern void CanUtilities_clear_can_message(can_msg_t *can_msg);


    /**
     * @brief Loads a CAN message with identifier, payload size, and data bytes
     *
     * @details Initializes all fields of a CAN message structure with the provided
     * values. Sets the 29-bit CAN identifier, payload byte count, and up to 8 data bytes.
     *
     * Use cases:
     * - Creating CAN control frames (CID, RID, AMD)
     * - Building CAN message frames from OpenLCB messages
     * - Preparing CAN frames for transmission
     *
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
     *
     * @warning NULL pointer will cause undefined behavior
     * @warning Payload size values greater than 8 may cause issues
     *
     * @attention All 8 byte parameters must be provided even if payload_size is less than 8
     *
     * @see CanUtilities_clear_can_message - Clears message structure
     * @see CanUtilities_copy_64_bit_to_can_message - Alternative for 8-byte payloads
     */
    extern void CanUtilities_load_can_message(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8);


    /**
     * @brief Copies a 48-bit Node ID into CAN message payload
     *
     * @details Converts a 48-bit OpenLCB Node ID into 6 bytes and stores them in the
     * CAN message payload starting at the specified offset. Updates the payload count
     * to reflect the total number of bytes used.
     *
     * Use cases:
     * - Building AMD (Alias Map Definition) frames
     * - Building Verified Node ID messages
     * - Sending Node ID in CAN control frames
     *
     * @param can_msg Pointer to CAN message buffer to receive Node ID
     * @param node_id 48-bit OpenLCB Node ID to copy
     * @param start_offset Starting position in payload array (0-2)
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
    extern uint8_t CanUtilities_copy_node_id_to_payload(can_msg_t *can_msg, uint64_t node_id, uint8_t start_offset);


    /**
     * @brief Copies payload data from OpenLCB message to CAN message
     *
     * @details Transfers bytes from an OpenLCB message payload to a CAN message payload,
     * starting at specified offsets in each buffer. Copies up to the remaining space in
     * the 8-byte CAN payload.
     *
     * Use cases:
     * - Fragmenting large OpenLCB messages into CAN frames
     * - Creating multi-frame CAN messages
     * - Building first, middle, or last CAN frames from OpenLCB datagram
     *
     * @param openlcb_msg Pointer to OpenLCB message buffer (source)
     * @param can_msg Pointer to CAN message buffer (destination)
     * @param openlcb_start_index Starting index in OpenLCB payload
     * @param can_start_index Starting index in CAN payload (typically 0 or 2)
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
    extern uint8_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint16_t openlcb_start_index, uint8_t can_start_index);


    /**
     * @brief Appends CAN payload data to OpenLCB message payload
     *
     * @details Transfers bytes from a CAN message payload to an OpenLCB message payload,
     * starting at a specified offset in the CAN payload and appending to the end of the
     * OpenLCB payload. Used for reassembling multi-frame CAN messages.
     *
     * Use cases:
     * - Assembling multi-frame CAN messages into OpenLCB message
     * - Processing datagram frames (first, middle, last)
     * - Reconstructing fragmented OpenLCB messages
     *
     * @param openlcb_msg Pointer to OpenLCB message buffer (destination)
     * @param can_msg Pointer to CAN message buffer (source)
     * @param can_start_index Starting index in CAN payload to copy from
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
    extern uint8_t CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint8_t can_start_index);


    /**
     * @brief Copies a 64-bit value into CAN message payload
     *
     * @details Converts a 64-bit value into 8 bytes (big-endian) and stores them in the
     * CAN message payload. Useful for loading Event IDs or other 64-bit values.
     *
     * Use cases:
     * - Loading Event IDs into CAN message payloads
     * - Sending 64-bit timestamps or identifiers
     * - Building messages with 8-byte data fields
     *
     * @param can_msg Pointer to CAN message buffer to receive data
     * @param data 64-bit value to copy into payload
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
    extern uint8_t CanUtilities_copy_64_bit_to_can_message(can_msg_t *can_msg, uint64_t data);


    /**
     * @brief Copies identifier and payload from one CAN message to another
     *
     * @details Performs a complete copy of the CAN identifier and all valid payload bytes
     * from the source message to the target message. Does not copy state flags.
     *
     * Use cases:
     * - Duplicating CAN messages for logging
     * - Creating retry copies of messages
     * - Forwarding messages through gateways
     *
     * @param can_msg_source Pointer to source CAN message buffer
     * @param can_msg_target Pointer to destination CAN message buffer
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
    extern uint8_t CanUtilities_copy_can_message(can_msg_t *can_msg_source, can_msg_t *can_msg_target);


    /**
     * @brief Extracts a 48-bit Node ID from CAN message payload
     *
     * @details Reads the first 6 bytes from the CAN message payload and converts them
     * into a 48-bit Node ID value. Assumes big-endian byte order (MSB first).
     *
     * Use cases:
     * - Processing AMD (Alias Map Definition) frames
     * - Extracting Node ID from Verified Node ID messages
     * - Reading Node ID from CAN control frames
     *
     * @param can_msg Pointer to CAN message buffer containing Node ID
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
    extern node_id_t CanUtilities_extract_can_payload_as_node_id(can_msg_t *can_msg);


    /**
     * @brief Extracts the 12-bit source alias from CAN identifier
     *
     * @details Masks and extracts the lowest 12 bits (bits 0-11) from the CAN
     * identifier field, which contains the source node's alias.
     *
     * Use cases:
     * - Identifying the source node of received CAN frames
     * - Routing messages to correct node handlers
     * - Alias conflict detection during login
     *
     * @param can_msg Pointer to CAN message buffer containing identifier
     *
     * @return 12-bit source alias (0x000-0xFFF)
     *
     * @warning NULL pointer will cause undefined behavior
     *
     * @attention Valid alias range is 0x001-0xFFF (0x000 is invalid)
     *
     * @see CanUtilities_extract_dest_alias_from_can_message - Gets destination alias
     */
    extern uint16_t CanUtilities_extract_source_alias_from_can_identifier(can_msg_t *can_msg);


    /**
     * @brief Extracts the destination alias from CAN message
     *
     * @details Examines the CAN frame type and extracts the destination alias from the
     * appropriate location. For addressed messages, reads from payload bytes 0-1. For
     * datagrams, reads from identifier bits. Returns 0 if message has no destination.
     *
     * Use cases:
     * - Determining if message is addressed to a specific node
     * - Routing addressed messages to correct node
     * - Processing datagram frames
     *
     * @param can_msg Pointer to CAN message buffer to examine
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
    extern uint16_t CanUtilities_extract_dest_alias_from_can_message(can_msg_t *can_msg);


    /**
     * @brief Converts CAN MTI to standard OpenLCB MTI
     *
     * @details Extracts and converts the 12-bit CAN MTI from the identifier field into
     * the standard 16-bit OpenLCB MTI format. Handles special cases like multi-frame
     * PCER (Producer/Consumer Event Report) messages.
     *
     * Use cases:
     * - Processing received CAN frames
     * - Dispatching messages to protocol handlers
     * - Converting between CAN and OpenLCB message formats
     *
     * @param can_msg Pointer to CAN message buffer containing identifier
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
    extern uint16_t CanUtilities_convert_can_mti_to_openlcb_mti(can_msg_t *can_msg);


    /**
     * @brief Counts NULL bytes in both CAN and OpenLCB message payloads
     *
     * @details Scans the valid payload bytes in both the CAN message and OpenLCB message,
     * counting the total number of NULL (0x00) bytes found. Used for SNIP validation.
     *
     * Use cases:
     * - Validating SNIP (Simple Node Information Protocol) messages
     * - Checking for proper null-terminated strings
     * - Verifying message format compliance
     *
     * @param openlcb_msg Pointer to OpenLCB message buffer with payload
     * @param can_msg Pointer to CAN message buffer with payload
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
    extern uint8_t CanUtilities_count_nulls_in_payloads(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg);


    /**
     * @brief Tests if CAN frame contains an OpenLCB message
     *
     * @details Checks the CAN identifier to determine if the frame type bit indicates
     * this is an OpenLCB message frame (as opposed to a CAN-only control frame).
     *
     * Use cases:
     * - Filtering received CAN frames
     * - Separating OpenLCB messages from CAN control frames
     * - Routing frames to appropriate handlers
     *
     * @param can_msg Pointer to CAN message buffer to test
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
    extern bool CanUtilities_is_openlcb_message(can_msg_t *can_msg);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_UTILITIES__ */
