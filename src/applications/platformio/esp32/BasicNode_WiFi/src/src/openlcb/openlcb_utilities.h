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
 * @brief Common utility functions for OpenLCB message and buffer manipulation.
 *
 * @details Provides helpers for message construction, payload insert/extract,
 * configuration memory buffer operations, event range calculations, broadcast
 * time event encoding/decoding, and train search event encoding/decoding.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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

    // =========================================================================
    // Message Structure Operations
    // =========================================================================

        /**
         * @brief Loads message header fields and clears the payload to zeros.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to initialize
         * @param source_alias 12-bit CAN alias of the source node
         * @param source_id 48-bit @ref node_id_t of the source node
         * @param dest_alias 12-bit CAN alias of the destination (0 for global)
         * @param dest_id 48-bit @ref node_id_t of the destination (0 for global)
         * @param mti Message Type Indicator
         *
         * @warning NULL pointer causes undefined behavior.
         */
    extern void OpenLcbUtilities_load_openlcb_message(openlcb_msg_t *openlcb_msg, uint16_t source_alias, uint64_t source_id, uint16_t dest_alias, uint64_t dest_id, uint16_t mti);

        /** @brief Zeros all payload bytes and resets payload_count. Header preserved. */
    extern void OpenLcbUtilities_clear_openlcb_message_payload(openlcb_msg_t *openlcb_msg);

        /** @brief Zeros entire message including header, state flags, and reference count. */
    extern void OpenLcbUtilities_clear_openlcb_message(openlcb_msg_t *openlcb_msg);

    // =========================================================================
    // Payload Insert Functions (all big-endian, all increment payload_count)
    // =========================================================================

        /** @brief Copies an 8-byte event ID to payload at offset 0. */
    extern void OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg_t *openlcb_msg, event_id_t event_id);

        /** @brief Copies a 6-byte node ID to payload at the given offset. */
    extern void OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg_t *openlcb_msg, node_id_t node_id, uint16_t offset);

        /** @brief Copies one byte to payload at the given offset. */
    extern void OpenLcbUtilities_copy_byte_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint8_t byte, uint16_t offset);

        /** @brief Copies a 16-bit word (big-endian) to payload at the given offset. */
    extern void OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t word, uint16_t offset);

        /** @brief Copies a 32-bit doubleword (big-endian) to payload at the given offset. */
    extern void OpenLcbUtilities_copy_dword_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint32_t doubleword, uint16_t offset);

        /**
         * @brief Copies a null-terminated string into the payload.
         *
         * @details Truncates if payload space is insufficient but always adds a
         * null terminator.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t
         * @param string Null-terminated source string
         * @param offset Starting byte offset in the payload
         *
         * @return Number of bytes written including the null terminator.
         */
    extern uint16_t OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg_t *openlcb_msg, const char string[], uint16_t offset);

        /**
         * @brief Copies a byte array into the payload.
         *
         * @details May copy fewer bytes than requested if payload space is exhausted.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t
         * @param byte_array Source data
         * @param offset Starting byte offset in the payload
         * @param requested_bytes Number of bytes to attempt to copy
         *
         * @return Actual number of bytes copied.
         */
    extern uint16_t OpenLcbUtilities_copy_byte_array_to_openlcb_payload(openlcb_msg_t *openlcb_msg, const uint8_t byte_array[], uint16_t offset, uint16_t requested_bytes);

    // =========================================================================
    // Payload Extract Functions (all big-endian, none modify payload_count)
    // =========================================================================

        /** @brief Extracts a 6-byte node ID from payload at the given offset. */
    extern node_id_t OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /** @brief Extracts an 8-byte event ID from payload at offset 0. */
    extern event_id_t OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg_t *openlcb_msg);

        /** @brief Extracts one byte from payload at the given offset. */
    extern uint8_t OpenLcbUtilities_extract_byte_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /** @brief Extracts a 16-bit word (big-endian) from payload at the given offset. */
    extern uint16_t OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /** @brief Extracts a 32-bit doubleword (big-endian) from payload at the given offset. */
    extern uint32_t OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

    // =========================================================================
    // Message Classification
    // =========================================================================

        /** @brief Returns the count of null bytes (0x00) in the payload. Used for SNIP validation. */
    extern uint8_t OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg_t *openlcb_msg);

        /** @brief Returns true if the MTI has the destination-address-present bit set. */
    extern bool OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg_t *openlcb_msg);

        /** @brief Returns true if the message destination matches this node's alias or ID. */
    extern bool OpenLcbUtilities_is_addressed_message_for_node(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg);

        /** @brief Sets the multi-frame control flag in the upper nibble of target, preserving the lower nibble. */
    extern void OpenLcbUtilities_set_multi_frame_flag(uint8_t *target, uint8_t flag);

    // =========================================================================
    // Event Assignment Lookups
    // =========================================================================

        /**
         * @brief Searches the node's producer list for a matching event ID.
         *
         * @param openlcb_node Node to search
         * @param event_id Event ID to find
         * @param event_index Receives the list index if found (undefined on false return)
         *
         * @return true if event found in producer list.
         */
    extern bool OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t *event_index);

        /**
         * @brief Searches the node's consumer list for a matching event ID.
         *
         * @param openlcb_node Node to search
         * @param event_id Event ID to find
         * @param event_index Receives the list index if found (undefined on false return)
         *
         * @return true if event found in consumer list.
         */
    extern bool OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t *event_index);

    // =========================================================================
    // Node / Buffer Helpers
    // =========================================================================

        /** @brief Returns the byte offset into global config memory where this node's space begins. */
    extern uint32_t OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node_t *openlcb_node);

        /** @brief Converts a @ref payload_type_enum to its maximum byte length. Returns 0 for unknown types. */
    extern uint16_t OpenLcbUtilities_payload_type_to_len(payload_type_enum payload_type);

    // =========================================================================
    // Configuration Memory Buffer Operations (all big-endian)
    // =========================================================================

        /** @brief Extracts a 6-byte node ID from a config memory buffer at the given index. */
    extern node_id_t OpenLcbUtilities_extract_node_id_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

        /** @brief Extracts a 16-bit word from a config memory buffer at the given index. */
    extern uint16_t OpenLcbUtilities_extract_word_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

        /** @brief Copies a 6-byte node ID into a config memory buffer at the given index. */
    extern void OpenLcbUtilities_copy_node_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, node_id_t node_id, uint8_t index);

        /** @brief Copies an 8-byte event ID into a config memory buffer at the given index. */
    extern void OpenLcbUtilities_copy_event_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, event_id_t event_id, uint8_t index);

        /** @brief Extracts an 8-byte event ID from a config memory buffer at the given index. */
    extern event_id_t OpenLcbUtilities_copy_config_mem_buffer_to_event_id(configuration_memory_buffer_t *buffer, uint8_t index);

    // =========================================================================
    // Configuration Memory Reply Builders
    // =========================================================================

        /**
         * @brief Builds a config memory write-failure reply datagram header.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_write_request_info @ref config_mem_write_request_info_t from original request
         * @param error_code 16-bit error code
         */
    extern void OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, uint16_t error_code);

        /** @brief Builds a config memory write-success reply datagram header. */
    extern void OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

        /**
         * @brief Builds a config memory read-failure reply datagram header.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_read_request_info @ref config_mem_read_request_info_t from original request
         * @param error_code 16-bit error code
         */
    extern void OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info, uint16_t error_code);

        /**
         * @brief Builds a config memory read-success reply datagram header only.
         *
         * @details Caller must append actual data bytes separately after this call.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_read_request_info @ref config_mem_read_request_info_t from original request
         */
    extern void OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

    // =========================================================================
    // Event Range Utilities
    // =========================================================================

        /**
         * @brief Generates a masked Event ID covering a range of consecutive events.
         *
         * @param base_event_id Starting @ref event_id_t of the range
         * @param count Number of events in the range (@ref event_range_count_enum)
         *
         * @return Masked @ref event_id_t for a Range Identified message.
         */
    extern event_id_t OpenLcbUtilities_generate_event_range_id(event_id_t base_event_id, event_range_count_enum count);

        /** @brief Returns true if the event ID falls within any of the node's consumer ranges. */
    extern bool OpenLcbUtilities_is_event_id_in_consumer_ranges(openlcb_node_t *openlcb_node, event_id_t event_id);

        /** @brief Returns true if the event ID falls within any of the node's producer ranges. */
    extern bool OpenLcbUtilities_is_event_id_in_producer_ranges(openlcb_node_t *openlcb_node, event_id_t event_id);

    // =========================================================================
    // Broadcast Time Event Utilities
    // =========================================================================

        /** @brief Returns true if the event ID belongs to the broadcast time event space. */
    extern bool OpenLcbUtilities_is_broadcast_time_event(event_id_t event_id);

        /** @brief Extracts the 48-bit clock ID (upper 6 bytes) from a broadcast time event ID. */
    extern uint64_t OpenLcbUtilities_extract_clock_id_from_time_event(event_id_t event_id);

        /** @brief Returns the @ref broadcast_time_event_type_enum for a broadcast time event ID. */
    extern broadcast_time_event_type_enum OpenLcbUtilities_get_broadcast_time_event_type(event_id_t event_id);

        /**
         * @brief Extracts hour and minute from a broadcast time event ID.
         *
         * @param event_id Broadcast time event ID
         * @param hour Receives hour (0-23)
         * @param minute Receives minute (0-59)
         *
         * @return true if values are valid, false if out of range.
         */
    extern bool OpenLcbUtilities_extract_time_from_event_id(event_id_t event_id, uint8_t *hour, uint8_t *minute);

        /**
         * @brief Extracts month and day from a broadcast time event ID.
         *
         * @param event_id Broadcast time event ID
         * @param month Receives month (1-12)
         * @param day Receives day (1-31)
         *
         * @return true if values are valid, false if out of range.
         */
    extern bool OpenLcbUtilities_extract_date_from_event_id(event_id_t event_id, uint8_t *month, uint8_t *day);

        /**
         * @brief Extracts year from a broadcast time event ID.
         *
         * @param event_id Broadcast time event ID
         * @param year Receives year (0-4095)
         *
         * @return true if extraction successful.
         */
    extern bool OpenLcbUtilities_extract_year_from_event_id(event_id_t event_id, uint16_t *year);

        /**
         * @brief Extracts the 12-bit signed fixed-point rate from a broadcast time event ID.
         *
         * @param event_id Broadcast time event ID
         * @param rate Receives rate (10.2 fixed point, e.g. 0x0004 = 1.00)
         *
         * @return true if extraction successful.
         */
    extern bool OpenLcbUtilities_extract_rate_from_event_id(event_id_t event_id, int16_t *rate);

        /** @brief Creates a Report/Set Time event ID from clock_id, hour, minute. */
    extern event_id_t OpenLcbUtilities_create_time_event_id(uint64_t clock_id, uint8_t hour, uint8_t minute, bool is_set);

        /** @brief Creates a Report/Set Date event ID from clock_id, month, day. */
    extern event_id_t OpenLcbUtilities_create_date_event_id(uint64_t clock_id, uint8_t month, uint8_t day, bool is_set);

        /** @brief Creates a Report/Set Year event ID from clock_id, year. */
    extern event_id_t OpenLcbUtilities_create_year_event_id(uint64_t clock_id, uint16_t year, bool is_set);

        /** @brief Creates a Report/Set Rate event ID from clock_id, rate. */
    extern event_id_t OpenLcbUtilities_create_rate_event_id(uint64_t clock_id, int16_t rate, bool is_set);

        /** @brief Creates a command event ID (Query, Start, Stop, Date Rollover) for the given clock. */
    extern event_id_t OpenLcbUtilities_create_command_event_id(uint64_t clock_id, broadcast_time_event_type_enum command);

    // =========================================================================
    // Train Search Event Utilities
    // =========================================================================

        /** @brief Returns true if the event ID belongs to the train search space (upper 4 bytes = 0x090099FF). */
    extern bool OpenLcbUtilities_is_train_search_event(event_id_t event_id);

        /** @brief Extracts 6 search-query nibbles from a train search event ID into digits[]. */
    extern void OpenLcbUtilities_extract_train_search_digits(event_id_t event_id, uint8_t *digits);

        /** @brief Extracts the flags byte (byte 7) from a train search event ID. */
    extern uint8_t OpenLcbUtilities_extract_train_search_flags(event_id_t event_id);

        /** @brief Converts a 6-nibble digit array to a numeric DCC address, skipping leading 0xF nibbles. */
    extern uint16_t OpenLcbUtilities_train_search_digits_to_address(const uint8_t *digits);

        /** @brief Creates a train search event ID from a DCC address and flags byte. */
    extern event_id_t OpenLcbUtilities_create_train_search_event_id(uint16_t address, uint8_t flags);

        /** @brief Returns true if the event ID is one of the 4 well-known emergency events. */
    extern bool OpenLcbUtilities_is_emergency_event(event_id_t event_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_UTILITIES__ */
