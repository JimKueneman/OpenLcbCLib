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
* @file openlcb_utilities.h
* @brief Common utility functions for OpenLCB message and buffer manipulation
*
* @details This module provides utility functions for working with OpenLCB messages,
* payloads, configuration memory buffers, and node structures. These are helper
* functions used throughout the OpenLCB library for data conversion, message
* construction, and buffer operations.
*
* The utilities are organized into several categories:
* - Message structure initialization and clearing
* - Payload data insertion (events, node IDs, integers, strings, byte arrays)
* - Payload data extraction (events, node IDs, integers)
* - Configuration memory buffer operations
* - Message classification and validation
* - Configuration memory reply message construction
*
* @author Jim Kueneman
* @date 17 Jan 2026
*/

#ifndef __OPENLCB_OPENLCB_UTILITIES__
#define __OPENLCB_OPENLCB_UTILITIES__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
        * @brief Initializes an OpenLCB message structure with source, destination, and MTI
        *
        * @details Loads the message header fields and clears the payload to zeros.
        * This is the primary function for preparing a new message for transmission.
        *
        * Use cases:
        * - Preparing outgoing messages in protocol handlers
        * - Initializing reply messages in response to received messages
        * - Setting up event producer/consumer messages
        *
        * @param openlcb_msg Pointer to the message structure to initialize
        * @param source_alias 12-bit CAN alias of the source node
        * @param source_id 48-bit unique Node ID of the source node
        * @param dest_alias 12-bit CAN alias of the destination node (0 for global messages)
        * @param dest_id 48-bit unique Node ID of the destination node (0 for global messages)
        * @param mti Message Type Indicator defining the message type
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning The payload buffer size is determined by openlcb_msg->payload_type.
        *
        * @attention Always call this before manually setting payload bytes to ensure
        *            the payload starts in a known cleared state.
        *
        * @see OpenLcbUtilities_clear_openlcb_message - Complete message reset
        * @see OpenLcbUtilities_clear_openlcb_message_payload - Payload-only reset
        */
    extern void OpenLcbUtilities_load_openlcb_message(openlcb_msg_t *openlcb_msg, uint16_t source_alias, uint64_t source_id, uint16_t dest_alias, uint64_t dest_id, uint16_t mti);

        /**
        * @brief Copies an 8-byte event ID into the message payload
        *
        * @details Writes the event ID in big-endian format to the payload starting at offset 0.
        * Increments payload_count by 8. Event IDs are 64-bit values that identify
        * producer and consumer events per the Event Transport protocol.
        *
        * Use cases:
        * - Building Producer Identified messages
        * - Building Consumer Identified messages
        * - Building Producer/Consumer Event Report messages
        *
        * @param openlcb_msg Pointer to the message structure
        * @param event_id 64-bit event identifier
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Assumes payload has at least 8 bytes available.
        *
        * @attention This function always writes to offset 0. For events at other
        *            positions, manually copy bytes or use a different function.
        *
        * @note Updates payload_count to reflect the added bytes.
        *
        * @see OpenLcbUtilities_extract_event_id_from_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_copy_event_id_to_config_mem_buffer - Config memory version
        */
    extern void OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg_t *openlcb_msg, event_id_t event_id);

        /**
        * @brief Copies a 6-byte node ID into the message payload at a specified offset
        *
        * @details Writes the node ID in big-endian format to the payload.
        * Increments payload_count by 6. Node IDs are 48-bit unique identifiers
        * per the OpenLCB specification.
        *
        * Use cases:
        * - Building Verified Node ID messages
        * - Building Simple Node Ident Info replies
        * - Building protocol support inquiry responses
        *
        * @param openlcb_msg Pointer to the message structure
        * @param node_id 48-bit unique node identifier
        * @param offset Byte offset in the payload where node ID should be written
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Assumes payload has at least (offset + 6) bytes available.
        * @warning Does not validate offset bounds.
        *
        * @note Updates payload_count to reflect the added bytes.
        *
        * @see OpenLcbUtilities_extract_node_id_from_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_copy_node_id_to_config_mem_buffer - Config memory version
        */
    extern void OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg_t *openlcb_msg, node_id_t node_id, uint16_t offset);

        /**
        * @brief Copies a single byte into the message payload at a specified offset
        *
        * @details Writes one byte to the payload and increments payload_count by 1.
        *
        * Use cases:
        * - Writing command codes in datagrams
        * - Writing status flags in protocol messages
        * - Writing individual configuration bytes
        *
        * @param openlcb_msg Pointer to the message structure
        * @param byte The byte value to write
        * @param offset Byte offset in the payload where the byte should be written
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Does not validate offset bounds against payload size.
        *
        * @note Updates payload_count to reflect the added byte.
        *
        * @see OpenLcbUtilities_extract_byte_from_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_copy_word_to_openlcb_payload - For 16-bit values
        */
    extern void OpenLcbUtilities_copy_byte_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint8_t byte, uint16_t offset);
    
        /**
        * @brief Copies a 16-bit word into the message payload at a specified offset
        *
        * @details Writes the word in big-endian format (MSB first) and increments
        * payload_count by 2.
        *
        * Use cases:
        * - Writing memory addresses in configuration memory protocol
        * - Writing error codes in datagram replies
        * - Writing 16-bit configuration values
        *
        * @param openlcb_msg Pointer to the message structure
        * @param word The 16-bit value to write
        * @param offset Byte offset in the payload where the word should be written
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Assumes payload has at least (offset + 2) bytes available.
        * @warning Does not validate offset bounds.
        *
        * @note Updates payload_count to reflect the added bytes.
        * @note Byte order is big-endian per OpenLCB specification.
        *
        * @see OpenLcbUtilities_extract_word_from_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_copy_dword_to_openlcb_payload - For 32-bit values
        */
    extern void OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t word, uint16_t offset);

        /**
        * @brief Copies a 32-bit doubleword into the message payload at a specified offset
        *
        * @details Writes the doubleword in big-endian format (MSB first) and increments
        * payload_count by 4.
        *
        * Use cases:
        * - Writing 32-bit memory addresses in configuration memory protocol
        * - Writing large numeric values in protocol messages
        * - Writing timestamps or counters
        *
        * @param openlcb_msg Pointer to the message structure
        * @param doubleword The 32-bit value to write
        * @param offset Byte offset in the payload where the doubleword should be written
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Assumes payload has at least (offset + 4) bytes available.
        * @warning Does not validate offset bounds.
        *
        * @note Updates payload_count to reflect the added bytes.
        * @note Byte order is big-endian per OpenLCB specification.
        *
        * @see OpenLcbUtilities_extract_dword_from_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_copy_word_to_openlcb_payload - For 16-bit values
        */
    extern void OpenLcbUtilities_copy_dword_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint32_t doubleword, uint16_t offset);

        /**
        * @brief Copies a null-terminated string into the message payload
        *
        * @details Copies characters from the string until null terminator is found or
        * payload space is exhausted. Always appends a null terminator. Returns the
        * total number of bytes written including the null terminator.
        *
        * Use cases:
        * - Building Simple Node Ident Info (SNIP) replies with manufacturer/model strings
        * - Writing user-defined strings in configuration data
        * - Building text-based protocol messages
        *
        * @param openlcb_msg Pointer to the message structure
        * @param string Null-terminated C string to copy
        * @param offset Byte offset in the payload where string should start
        * @return Number of bytes written including the null terminator
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
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
        * @see OpenLcbUtilities_copy_byte_array_to_openlcb_payload - For binary data
        */
    extern uint16_t OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg_t *openlcb_msg, const char string[], uint16_t offset);

        /**
        * @brief Copies a byte array into the message payload
        *
        * @details Copies the requested number of bytes from the array, or until
        * payload space is exhausted. Returns the actual number of bytes copied.
        *
        * Use cases:
        * - Copying configuration memory data into read reply datagrams
        * - Building multi-byte protocol responses
        * - Transferring binary data blocks
        *
        * @param openlcb_msg Pointer to the message structure
        * @param byte_array Array of bytes to copy
        * @param offset Byte offset in the payload where array should start
        * @param requested_bytes Number of bytes to attempt to copy
        * @return Actual number of bytes copied (may be less if payload full)
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning byte_array must have at least requested_bytes available.
        * @warning May copy fewer bytes than requested if payload space exhausted.
        *
        * @attention Check return value to determine if all requested bytes were copied.
        * @attention Does not add null terminator (unlike copy_string).
        *
        * @note Updates payload_count to reflect bytes actually copied.
        * @note Return value indicates actual bytes written, useful for multi-frame messages.
        *
        * @see OpenLcbUtilities_copy_string_to_openlcb_payload - For null-terminated strings
        */
    extern uint16_t OpenLcbUtilities_copy_byte_array_to_openlcb_payload(openlcb_msg_t *openlcb_msg, const uint8_t byte_array[], uint16_t offset, uint16_t requested_bytes);

        /**
        * @brief Clears only the payload portion of a message structure
        *
        * @details Sets all payload bytes to zero and resets payload_count to 0.
        * Does not modify message header fields (source, dest, MTI).
        *
        * Use cases:
        * - Reusing a message structure for a new reply
        * - Clearing payload before building multi-part response
        * - Resetting payload between retry attempts
        *
        * @param openlcb_msg Pointer to the message structure
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        *
        * @attention Preserves message header (aliases, IDs, MTI). Use
        *            OpenLcbUtilities_clear_openlcb_message to clear everything.
        *
        * @note Payload size is determined by openlcb_msg->payload_type.
        *
        * @see OpenLcbUtilities_clear_openlcb_message - Clears entire message
        * @see OpenLcbUtilities_load_openlcb_message - Initializes message with values
        */
    extern void OpenLcbUtilities_clear_openlcb_message_payload(openlcb_msg_t *openlcb_msg);

        /**
        * @brief Completely clears and resets a message structure
        *
        * @details Zeros all header fields, clears payload, and resets all state flags.
        * Sets allocated and inprocess flags to false, resets reference count and timerticks.
        *
        * Use cases:
        * - Returning a message buffer to the pool
        * - Preparing a buffer for initial allocation
        * - Recovering from error conditions
        *
        * @param openlcb_msg Pointer to the message structure
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        *
        * @attention This clears ALL fields including state flags and reference count.
        *            Only use when completely discarding message context.
        *
        * @attention After calling this, the message should be considered unallocated
        *            and should not be used until properly initialized again.
        *
        * @see OpenLcbUtilities_clear_openlcb_message_payload - Clears only payload
        * @see OpenLcbUtilities_load_openlcb_message - Initializes message with values
        */
    extern void OpenLcbUtilities_clear_openlcb_message(openlcb_msg_t *openlcb_msg);

        /**
        * @brief Extracts a 6-byte node ID from the message payload
        *
        * @details Reads 6 bytes in big-endian format starting at the specified offset
        * and assembles them into a 48-bit node_id_t value.
        *
        * Use cases:
        * - Parsing Verified Node ID messages
        * - Extracting node IDs from protocol inquiry responses
        * - Reading node IDs from configuration memory
        *
        * @param openlcb_msg Pointer to the message structure
        * @param offset Byte offset in the payload where node ID starts
        * @return The assembled 48-bit node ID
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Assumes payload has at least (offset + 6) bytes available.
        * @warning Does not validate offset bounds.
        * @warning Reading beyond payload bounds returns undefined values.
        *
        * @note Does not modify payload_count.
        *
        * @see OpenLcbUtilities_copy_node_id_to_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_extract_node_id_from_config_mem_buffer - Config memory version
        */
    extern node_id_t OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /**
        * @brief Extracts an 8-byte event ID from the message payload
        *
        * @details Reads 8 bytes in big-endian format starting at offset 0 and
        * assembles them into a 64-bit event_id_t value.
        *
        * Use cases:
        * - Parsing Producer/Consumer Event Report messages
        * - Extracting event IDs from Identify messages
        * - Reading event IDs from protocol queries
        *
        * @param openlcb_msg Pointer to the message structure
        * @return The assembled 64-bit event ID
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Assumes payload has at least 8 bytes available.
        * @warning Reading from empty payload returns undefined values.
        *
        * @attention Always reads from offset 0. For events at other positions,
        *            manually extract bytes or modify this function.
        *
        * @note Does not modify payload_count.
        *
        * @see OpenLcbUtilities_copy_event_id_to_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_copy_config_mem_buffer_to_event_id - Config memory version
        */
    extern event_id_t OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg_t *openlcb_msg);

        /**
        * @brief Extracts a single byte from the message payload
        *
        * @details Reads one byte from the specified offset in the payload.
        *
        * Use cases:
        * - Reading command codes from datagrams
        * - Extracting status flags from protocol messages
        * - Reading individual configuration bytes
        *
        * @param openlcb_msg Pointer to the message structure
        * @param offset Byte offset in the payload to read from
        * @return The byte value at the specified offset
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Does not validate offset bounds against payload size.
        * @warning Reading beyond payload bounds returns undefined values.
        *
        * @note Does not modify payload_count.
        *
        * @see OpenLcbUtilities_copy_byte_to_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_extract_word_from_openlcb_payload - For 16-bit values
        */
    extern uint8_t OpenLcbUtilities_extract_byte_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /**
        * @brief Extracts a 16-bit word from the message payload
        *
        * @details Reads 2 bytes in big-endian format (MSB first) starting at the
        * specified offset and assembles them into a 16-bit value.
        *
        * Use cases:
        * - Reading memory addresses from configuration memory protocol
        * - Extracting error codes from datagram replies
        * - Reading 16-bit configuration values
        *
        * @param openlcb_msg Pointer to the message structure
        * @param offset Byte offset in the payload where word starts
        * @return The assembled 16-bit value
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Assumes payload has at least (offset + 2) bytes available.
        * @warning Does not validate offset bounds.
        * @warning Reading beyond payload bounds returns undefined values.
        *
        * @note Does not modify payload_count.
        * @note Byte order is big-endian per OpenLCB specification.
        *
        * @see OpenLcbUtilities_copy_word_to_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_extract_dword_from_openlcb_payload - For 32-bit values
        */
    extern uint16_t OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /**
        * @brief Extracts a 32-bit doubleword from the message payload
        *
        * @details Reads 4 bytes in big-endian format (MSB first) starting at the
        * specified offset and assembles them into a 32-bit value.
        *
        * Use cases:
        * - Reading 32-bit memory addresses from configuration memory protocol
        * - Extracting large numeric values from protocol messages
        * - Reading timestamps or counters
        *
        * @param openlcb_msg Pointer to the message structure
        * @param offset Byte offset in the payload where doubleword starts
        * @return The assembled 32-bit value
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        * @warning Assumes payload has at least (offset + 4) bytes available.
        * @warning Does not validate offset bounds.
        * @warning Reading beyond payload bounds returns undefined values.
        *
        * @note Does not modify payload_count.
        * @note Byte order is big-endian per OpenLCB specification.
        *
        * @see OpenLcbUtilities_copy_dword_to_openlcb_payload - Reverse operation
        * @see OpenLcbUtilities_extract_word_from_openlcb_payload - For 16-bit values
        */
    extern uint32_t OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);
    
        /**
        * @brief Counts the number of null bytes (0x00) in the message payload
        *
        * @details Scans through payload_count bytes and counts occurrences of 0x00.
        * Used primarily for validating SNIP (Simple Node Ident Info) message format,
        * which requires exactly 4 null terminators for the 4 string fields.
        *
        * Use cases:
        * - Validating received SNIP messages have correct format
        * - Verifying string-based protocol messages
        * - Detecting malformed messages
        *
        * @param openlcb_msg Pointer to the message structure
        * @return Count of null bytes found in the payload
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        *
        * @attention Only scans up to payload_count bytes, not the entire buffer.
        *
        * @note For SNIP messages, exactly 4 nulls are expected (manufacturer,
        *       model, hardware version, software version).
        *
        * @see protocol_snip.c - Uses this for SNIP validation
        */
    extern uint8_t OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg_t *openlcb_msg);

        /**
        * @brief Determines if a message is addressed (not global)
        *
        * @details Checks the MTI (Message Type Indicator) for the destination
        * address present bit. Addressed messages have a specific destination node,
        * while global messages are broadcast to all nodes.
        *
        * Use cases:
        * - Routing messages in the main state machine
        * - Determining if a message requires alias resolution
        * - Filtering messages for specific node handling
        *
        * @param openlcb_msg Pointer to the message structure
        * @return true if message is addressed to a specific node, false if global
        *
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        *
        * @note Global messages have dest_alias = 0 and MTI without address bit set.
        * @note Addressed messages require valid dest_alias or dest_id.
        *
        * @see OpenLcbUtilities_is_addressed_message_for_node - Checks specific node match
        * @see MASK_DEST_ADDRESS_PRESENT - MTI bit mask used for check
        */
    extern bool OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg_t *openlcb_msg);

        /**
        * @brief Sets the multi-frame flag in a target byte
        *
        * @details Modifies the upper nibble of the target byte to set the multi-frame
        * control flag while preserving the lower nibble. Used in datagram and stream
        * protocols for frame sequencing.
        *
        * Use cases:
        * - Marking first frame in multi-frame datagram
        * - Marking middle frames in sequence
        * - Marking final frame in sequence
        * - Marking single-frame (only) messages
        *
        * @param target Pointer to the byte to modify
        * @param flag Multi-frame flag value (MULTIFRAME_ONLY, MULTIFRAME_FIRST,
        *             MULTIFRAME_MIDDLE, or MULTIFRAME_FINAL)
        *
        * @warning target must not be NULL. No NULL check performed.
        *
        * @attention Upper nibble is completely replaced. Lower nibble is preserved.
        * @attention Valid flag values are defined in openlcb_defines.h.
        *
        * @note This is typically used on the first byte of configuration memory
        *       or datagram protocol command bytes.
        *
        * @see protocol_config_mem_*.c - Uses for multi-frame config operations
        */
    extern void OpenLcbUtilities_set_multi_frame_flag(uint8_t *target, uint8_t flag);

        /**
        * @brief Checks if an addressed message is for a specific node
        *
        * @details Compares the message destination (alias or ID) against the node's
        * alias and ID. Returns true if either matches, meaning this node is the
        * intended recipient.
        *
        * Use cases:
        * - Filtering incoming addressed messages in state machine
        * - Determining if node should process a protocol message
        * - Validating message routing in multi-node systems
        *
        * @param openlcb_node Pointer to the node structure to check
        * @param openlcb_msg Pointer to the message structure
        * @return true if message is addressed to this node, false otherwise
        *
        * @warning openlcb_node must not be NULL. No NULL check performed.
        * @warning openlcb_msg must not be NULL. No NULL check performed.
        *
        * @attention Matches on EITHER alias OR node ID. A match on either field
        *            is sufficient to return true.
        *
        * @note Global messages (dest_alias = 0, dest_id = 0) will return false
        *       unless the node somehow has alias or ID = 0 (invalid state).
        *
        * @see OpenLcbUtilities_is_addressed_openlcb_message - Checks if message is addressed at all
        */
    extern bool OpenLcbUtilities_is_addressed_message_for_node(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg);

        /**
        * @brief Checks if a producer event is assigned to a node
        *
        * @details Searches the node's producer event list for a matching event ID.
        * If found, returns true and stores the list index in event_index.
        *
        * Use cases:
        * - Processing Producer Identify requests
        * - Validating event ownership before sending reports
        * - Looking up producer state information
        *
        * @param openlcb_node Pointer to the node structure to search
        * @param event_id The 64-bit event ID to search for
        * @param event_index Pointer to store the list index if found
        * @return true if event found in producer list, false otherwise
        *
        * @warning openlcb_node must not be NULL. No NULL check performed.
        * @warning event_index must not be NULL. No NULL check performed.
        * @warning event_index is only valid when function returns true.
        *
        * @attention event_index is NOT modified if event is not found.
        *
        * @note Linear search with O(n) complexity where n = producer count.
        *
        * @see OpenLcbUtilities_is_consumer_event_assigned_to_node - Consumer version
        * @see protocol_event_transport.c - Uses this for event identification
        */
    extern bool OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t *event_index);

        /**
        * @brief Checks if a consumer event is assigned to a node
        *
        * @details Searches the node's consumer event list for a matching event ID.
        * If found, returns true and stores the list index in event_index.
        *
        * Use cases:
        * - Processing Consumer Identify requests
        * - Validating event subscriptions
        * - Looking up consumer state information
        *
        * @param openlcb_node Pointer to the node structure to search
        * @param event_id The 64-bit event ID to search for
        * @param event_index Pointer to store the list index if found
        * @return true if event found in consumer list, false otherwise
        *
        * @warning openlcb_node must not be NULL. No NULL check performed.
        * @warning event_index must not be NULL. No NULL check performed.
        * @warning event_index is only valid when function returns true.
        *
        * @attention event_index is NOT modified if event is not found.
        *
        * @note Linear search with O(n) complexity where n = consumer count.
        *
        * @see OpenLcbUtilities_is_producer_event_assigned_to_node - Producer version
        * @see protocol_event_transport.c - Uses this for event identification
        */
    extern bool OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t *event_index);

        /**
        * @brief Calculates the memory offset for a node's configuration space
        *
        * @details Computes the byte offset into the global configuration memory
        * array where this node's configuration space begins. Handles both absolute
        * addressing (when low_address_valid is false) and relative addressing
        * (when low_address_valid is true).
        *
        * Use cases:
        * - Multi-node configuration memory implementations
        * - Mapping node index to memory address
        * - Validating configuration memory access requests
        *
        * @param openlcb_node Pointer to the node structure
        * @return Byte offset into global configuration memory for this node
        *
        * @warning openlcb_node must not be NULL. No NULL check performed.
        * @warning openlcb_node->parameters must not be NULL. No NULL check performed.
        *
        * @attention If low_address_valid is true, uses (highest - lowest) as size.
        * @attention If low_address_valid is false, uses highest_address as size.
        *
        * @note Result is node_size * node_index, allowing sequential allocation.
        *
        * @see protocol_config_mem_*.c - Uses this for address space calculations
        */
    extern uint32_t OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node_t *openlcb_node);

        /**
        * @brief Converts payload type enum to byte length
        *
        * @details Returns the maximum payload size in bytes for a given payload type.
        * Used throughout the library to determine buffer sizes and validate operations.
        *
        * Use cases:
        * - Allocating message buffers
        * - Validating payload operations
        * - Determining maximum data capacity
        *
        * @param payload_type The payload type enum value
        * @return Maximum payload size in bytes, or 0 for unknown types
        *
        * @note Return values per OpenLCB specification:
        *       - BASIC: LEN_MESSAGE_BYTES_BASIC
        *       - DATAGRAM: LEN_MESSAGE_BYTES_DATAGRAM  
        *       - SNIP: LEN_MESSAGE_BYTES_SNIP
        *       - STREAM: LEN_MESSAGE_BYTES_STREAM
        *       - default: 0
        *
        * @see openlcb_defines.h - Defines the LEN_MESSAGE_BYTES_* constants
        * @see payload_type_enum - Enum definition in openlcb_types.h
        */
    extern uint16_t OpenLcbUtilities_payload_type_to_len(payload_type_enum payload_type);

        /**
        * @brief Extracts a 6-byte node ID from a configuration memory buffer
        *
        * @details Reads 6 bytes in big-endian format from the buffer starting at
        * the specified index and assembles them into a 48-bit node_id_t value.
        *
        * Use cases:
        * - Reading node IDs from configuration memory
        * - Parsing configuration data structures
        * - Extracting stored node identifiers
        *
        * @param buffer Pointer to the configuration memory buffer
        * @param index Byte offset in the buffer where node ID starts
        * @return The assembled 48-bit node ID
        *
        * @warning buffer must not be NULL. No NULL check performed.
        * @warning Assumes buffer has at least (index + 6) bytes available.
        * @warning Does not validate index bounds.
        * @warning Reading beyond buffer bounds returns undefined values.
        *
        * @see OpenLcbUtilities_copy_node_id_to_config_mem_buffer - Reverse operation
        * @see OpenLcbUtilities_extract_node_id_from_openlcb_payload - Payload version
        */
    extern node_id_t OpenLcbUtilities_extract_node_id_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

        /**
        * @brief Extracts a 16-bit word from a configuration memory buffer
        *
        * @details Reads 2 bytes in big-endian format from the buffer starting at
        * the specified index and assembles them into a 16-bit value.
        *
        * Use cases:
        * - Reading configuration parameters
        * - Extracting stored addresses or offsets
        * - Parsing configuration data structures
        *
        * @param buffer Pointer to the configuration memory buffer
        * @param index Byte offset in the buffer where word starts
        * @return The assembled 16-bit value
        *
        * @warning buffer must not be NULL. No NULL check performed.
        * @warning Assumes buffer has at least (index + 2) bytes available.
        * @warning Does not validate index bounds.
        * @warning Reading beyond buffer bounds returns undefined values.
        *
        * @note Byte order is big-endian per OpenLCB specification.
        *
        * @see OpenLcbUtilities_extract_word_from_openlcb_payload - Payload version
        */
    extern uint16_t OpenLcbUtilities_extract_word_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

        /**
        * @brief Copies a 6-byte node ID into a configuration memory buffer
        *
        * @details Writes the node ID in big-endian format to the buffer starting
        * at the specified index.
        *
        * Use cases:
        * - Storing node IDs in configuration memory
        * - Writing configuration data structures
        * - Persisting node identifiers
        *
        * @param buffer Pointer to the configuration memory buffer
        * @param node_id The 48-bit node ID to write
        * @param index Byte offset in the buffer where node ID should be written
        *
        * @warning buffer must not be NULL. No NULL check performed.
        * @warning Assumes buffer has at least (index + 6) bytes available.
        * @warning Does not validate index bounds.
        *
        * @note Byte order is big-endian per OpenLCB specification.
        *
        * @see OpenLcbUtilities_extract_node_id_from_config_mem_buffer - Reverse operation
        * @see OpenLcbUtilities_copy_node_id_to_openlcb_payload - Payload version
        */
    extern void OpenLcbUtilities_copy_node_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, node_id_t node_id, uint8_t index);

        /**
        * @brief Copies an 8-byte event ID into a configuration memory buffer
        *
        * @details Writes the event ID in big-endian format to the buffer starting
        * at the specified index.
        *
        * Use cases:
        * - Storing event IDs in configuration memory
        * - Writing producer/consumer event configurations
        * - Persisting event identifiers
        *
        * @param buffer Pointer to the configuration memory buffer
        * @param event_id The 64-bit event ID to write
        * @param index Byte offset in the buffer where event ID should be written
        *
        * @warning buffer must not be NULL. No NULL check performed.
        * @warning Assumes buffer has at least (index + 8) bytes available.
        * @warning Does not validate index bounds.
        *
        * @note Byte order is big-endian per OpenLCB specification.
        *
        * @see OpenLcbUtilities_copy_config_mem_buffer_to_event_id - Reverse operation
        * @see OpenLcbUtilities_copy_event_id_to_openlcb_payload - Payload version
        */
    extern void OpenLcbUtilities_copy_event_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, event_id_t event_id, uint8_t index);

        /**
        * @brief Extracts an 8-byte event ID from a configuration memory buffer
        *
        * @details Reads 8 bytes in big-endian format from the buffer starting at
        * the specified index and assembles them into a 64-bit event_id_t value.
        *
        * Use cases:
        * - Reading event IDs from configuration memory
        * - Loading producer/consumer event configurations
        * - Retrieving stored event identifiers
        *
        * @param buffer Pointer to the configuration memory buffer
        * @param index Byte offset in the buffer where event ID starts
        * @return The assembled 64-bit event ID
        *
        * @warning buffer must not be NULL. No NULL check performed.
        * @warning Assumes buffer has at least (index + 8) bytes available.
        * @warning Does not validate index bounds.
        * @warning Reading beyond buffer bounds returns undefined values.
        *
        * @note Byte order is big-endian per OpenLCB specification.
        *
        * @see OpenLcbUtilities_copy_event_id_to_config_mem_buffer - Reverse operation
        * @see OpenLcbUtilities_extract_event_id_from_openlcb_payload - Payload version
        */
    extern event_id_t OpenLcbUtilities_copy_config_mem_buffer_to_event_id(configuration_memory_buffer_t *buffer, uint8_t index);
    
        /**
        * @brief Loads a configuration memory write failure reply message header
        *
        * @details Constructs a datagram reply message indicating a configuration
        * memory write operation failed. Includes the error code and original
        * request address. Handles both 4-byte and 6-byte address encoding formats.
        *
        * Use cases:
        * - Responding to write requests for read-only memory spaces
        * - Indicating invalid address ranges
        * - Reporting configuration memory errors
        *
        * @param statemachine_info Pointer to state machine context with message buffers
        * @param config_mem_write_request_info Pointer to original write request details
        * @param error_code 16-bit error code indicating failure reason
        *
        * @warning statemachine_info must not be NULL. No NULL check performed.
        * @warning config_mem_write_request_info must not be NULL. No NULL check performed.
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
        * @see OpenLcbUtilities_load_config_mem_reply_write_ok_message_header - Success version
        * @see protocol_config_mem_*.c - Uses this for error responses
        */
    extern void OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, uint16_t error_code);

        /**
        * @brief Loads a configuration memory write success reply message header
        *
        * @details Constructs a datagram reply message indicating a configuration
        * memory write operation succeeded. Includes the original request address.
        * Handles both 4-byte and 6-byte address encoding formats.
        *
        * Use cases:
        * - Acknowledging successful configuration memory writes
        * - Confirming data was stored correctly
        * - Completing write request transactions
        *
        * @param statemachine_info Pointer to state machine context with message buffers
        * @param config_mem_write_request_info Pointer to original write request details
        *
        * @warning statemachine_info must not be NULL. No NULL check performed.
        * @warning config_mem_write_request_info must not be NULL. No NULL check performed.
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
        * @see OpenLcbUtilities_load_config_mem_reply_write_fail_message_header - Failure version
        * @see protocol_config_mem_*.c - Uses this for success responses
        */
    extern void OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);
    
        /**
        * @brief Loads a configuration memory read failure reply message header
        *
        * @details Constructs a datagram reply message indicating a configuration
        * memory read operation failed. Includes the error code at the data start
        * position where data would have been placed. Handles both 4-byte and 6-byte
        * address encoding formats.
        *
        * Use cases:
        * - Responding to read requests for invalid addresses
        * - Indicating unavailable memory spaces
        * - Reporting configuration memory errors
        *
        * @param statemachine_info Pointer to state machine context with message buffers
        * @param config_mem_read_request_info Pointer to original read request details
        * @param error_code 16-bit error code indicating failure reason
        *
        * @warning statemachine_info must not be NULL. No NULL check performed.
        * @warning config_mem_read_request_info must not be NULL. No NULL check performed.
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
        * @see OpenLcbUtilities_load_config_mem_reply_read_ok_message_header - Success version
        * @see protocol_config_mem_*.c - Uses this for error responses
        */
    extern void OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info, uint16_t error_code);

        /**
        * @brief Loads a configuration memory read success reply message header
        *
        * @details Constructs a datagram reply message header indicating a configuration
        * memory read operation succeeded. Sets up the header fields; actual data bytes
        * must be added separately by the caller. Handles both 4-byte and 6-byte
        * address encoding formats.
        *
        * Use cases:
        * - Preparing successful configuration memory read replies
        * - Setting up header before copying data bytes
        * - Acknowledging valid read requests
        *
        * @param statemachine_info Pointer to state machine context with message buffers
        * @param config_mem_read_request_info Pointer to original read request details
        *
        * @warning statemachine_info must not be NULL. No NULL check performed.
        * @warning config_mem_read_request_info must not be NULL. No NULL check performed.
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
        * @see OpenLcbUtilities_load_config_mem_reply_read_fail_message_header - Failure version
        * @see OpenLcbUtilities_copy_byte_array_to_openlcb_payload - For adding data
        * @see protocol_config_mem_*.c - Uses this for success responses
        */
    extern void OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

    extern event_id_t OpenLcbUtilities_generate_event_range_id(event_id_t base_event_id, event_range_count_enum count);

    extern bool OpenLcbUtilities_is_event_id_in_consumer_ranges(openlcb_node_t *openlcb_node, event_id_t event_id);

    extern bool OpenLcbUtilities_is_event_id_in_producer_ranges(openlcb_node_t *openlcb_node, event_id_t event_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_UTILITIES__ */
