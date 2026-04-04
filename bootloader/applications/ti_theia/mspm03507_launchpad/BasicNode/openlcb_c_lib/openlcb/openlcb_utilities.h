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
 * @date 4 Mar 2026
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

        /**
         * @brief Zeros all payload bytes and resets payload_count. Header preserved.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to clear payload from.
         */
    extern void OpenLcbUtilities_clear_openlcb_message_payload(openlcb_msg_t *openlcb_msg);

        /**
         * @brief Zeros entire message including header, state flags, and reference count.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to fully clear.
         */
    extern void OpenLcbUtilities_clear_openlcb_message(openlcb_msg_t *openlcb_msg);

    // =========================================================================
    // Payload Insert Functions (all big-endian, all increment payload_count)
    // =========================================================================

        /**
         * @brief Copies an 8-byte event ID to payload at offset 0.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to write into.
         * @param event_id    64-bit @ref event_id_t to store in the payload.
         */
    extern void OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg_t *openlcb_msg, event_id_t event_id);

        /**
         * @brief Copies a 6-byte node ID to payload at the given offset.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to write into.
         * @param node_id     48-bit @ref node_id_t to store in the payload.
         * @param offset      Starting byte offset in the payload.
         */
    extern void OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg_t *openlcb_msg, node_id_t node_id, uint16_t offset);

        /**
         * @brief Copies one byte to payload at the given offset.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to write into.
         * @param byte        Byte value to store.
         * @param offset      Byte offset in the payload.
         */
    extern void OpenLcbUtilities_copy_byte_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint8_t byte, uint16_t offset);

        /**
         * @brief Copies a 16-bit word (big-endian) to payload at the given offset.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to write into.
         * @param word        16-bit value to store in big-endian order.
         * @param offset      Starting byte offset in the payload.
         */
    extern void OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t word, uint16_t offset);

        /**
         * @brief Copies a 32-bit doubleword (big-endian) to payload at the given offset.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to write into.
         * @param doubleword  32-bit value to store in big-endian order.
         * @param offset      Starting byte offset in the payload.
         */
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

        /**
         * @brief Extracts a 6-byte node ID from payload at the given offset.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to read from.
         * @param offset      Starting byte offset in the payload.
         *
         * @return 48-bit @ref node_id_t assembled from the payload bytes.
         */
    extern node_id_t OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /**
         * @brief Extracts an 8-byte event ID from payload at offset 0.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to read from.
         *
         * @return 64-bit @ref event_id_t assembled from the payload bytes.
         */
    extern event_id_t OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg_t *openlcb_msg);

        /**
         * @brief Extracts one byte from payload at the given offset.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to read from.
         * @param offset      Byte offset in the payload.
         *
         * @return Byte value at the specified offset.
         */
    extern uint8_t OpenLcbUtilities_extract_byte_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /**
         * @brief Extracts a 16-bit word (big-endian) from payload at the given offset.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to read from.
         * @param offset      Starting byte offset in the payload.
         *
         * @return 16-bit value assembled from two big-endian payload bytes.
         */
    extern uint16_t OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

        /**
         * @brief Extracts a 32-bit doubleword (big-endian) from payload at the given offset.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to read from.
         * @param offset      Starting byte offset in the payload.
         *
         * @return 32-bit value assembled from four big-endian payload bytes.
         */
    extern uint32_t OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t offset);

    // =========================================================================
    // Message Classification
    // =========================================================================

        /**
         * @brief Returns the count of null bytes (0x00) in the payload. Used for SNIP validation.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to scan.
         *
         * @return Number of null bytes found in the payload.
         */
    extern uint8_t OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg_t *openlcb_msg);

        /**
         * @brief Returns true if the MTI has the destination-address-present bit set.
         *
         * @param openlcb_msg Pointer to the @ref openlcb_msg_t to check.
         *
         * @return true if the message is addressed, false if global.
         */
    extern bool OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg_t *openlcb_msg);

        /**
         * @brief Returns true if the message destination matches this node's alias or ID.
         *
         * @param openlcb_node Pointer to the @ref openlcb_node_t to match against.
         * @param openlcb_msg  Pointer to the @ref openlcb_msg_t to check.
         *
         * @return true if the message is addressed to this node.
         */
    extern bool OpenLcbUtilities_is_addressed_message_for_node(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg);

        /**
         * @brief Sets the multi-frame control flag in the upper nibble of target, preserving the lower nibble.
         *
         * @param target Pointer to the byte whose upper nibble will be replaced.
         * @param flag   Flag value to write into the upper nibble.
         */
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

        /**
         * @brief Returns the byte offset into global config memory where this node's space begins.
         *
         * @param openlcb_node Pointer to the @ref openlcb_node_t to calculate the offset for.
         *
         * @return Byte offset into the global configuration memory array.
         */
    extern uint32_t OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node_t *openlcb_node);

        /**
         * @brief Converts a @ref payload_type_enum to its maximum byte length. Returns 0 for unknown types.
         *
         * @param payload_type The @ref payload_type_enum value to convert.
         *
         * @return Maximum payload byte length for the given type, or 0 if unknown.
         */
    extern uint16_t OpenLcbUtilities_payload_type_to_len(payload_type_enum payload_type);

    // =========================================================================
    // Configuration Memory Buffer Operations (all big-endian)
    // =========================================================================

        /**
         * @brief Extracts a 6-byte node ID from a config memory buffer at the given index.
         *
         * @param buffer Pointer to the @ref configuration_memory_buffer_t to read from.
         * @param index  Starting byte index in the buffer.
         *
         * @return 48-bit @ref node_id_t assembled from the buffer bytes.
         */
    extern node_id_t OpenLcbUtilities_extract_node_id_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

        /**
         * @brief Extracts a 16-bit word from a config memory buffer at the given index.
         *
         * @param buffer Pointer to the @ref configuration_memory_buffer_t to read from.
         * @param index  Starting byte index in the buffer.
         *
         * @return 16-bit value assembled from two big-endian buffer bytes.
         */
    extern uint16_t OpenLcbUtilities_extract_word_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

        /**
         * @brief Copies a 6-byte node ID into a config memory buffer at the given index.
         *
         * @param buffer  Pointer to the @ref configuration_memory_buffer_t to write into.
         * @param node_id 48-bit @ref node_id_t to store.
         * @param index   Starting byte index in the buffer.
         */
    extern void OpenLcbUtilities_copy_node_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, node_id_t node_id, uint8_t index);

        /**
         * @brief Copies an 8-byte event ID into a config memory buffer at the given index.
         *
         * @param buffer   Pointer to the @ref configuration_memory_buffer_t to write into.
         * @param event_id 64-bit @ref event_id_t to store.
         * @param index    Starting byte index in the buffer.
         */
    extern void OpenLcbUtilities_copy_event_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, event_id_t event_id, uint8_t index);

        /**
         * @brief Extracts an 8-byte event ID from a config memory buffer at the given index.
         *
         * @param buffer Pointer to the @ref configuration_memory_buffer_t to read from.
         * @param index  Starting byte index in the buffer.
         *
         * @return 64-bit @ref event_id_t assembled from the buffer bytes.
         */
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

        /**
         * @brief Builds a config memory write-success reply datagram header.
         *
         * @param statemachine_info           Pointer to the @ref openlcb_statemachine_info_t context.
         * @param config_mem_write_request_info Pointer to the @ref config_mem_write_request_info_t from the original request.
         */
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

        /**
         * @brief Returns true if the event ID falls within any of the node's consumer ranges.
         *
         * @param openlcb_node Pointer to the @ref openlcb_node_t whose consumer ranges are checked.
         * @param event_id     64-bit @ref event_id_t to test.
         *
         * @return true if the event ID falls within a consumer range.
         */
    extern bool OpenLcbUtilities_is_event_id_in_consumer_ranges(openlcb_node_t *openlcb_node, event_id_t event_id);

        /**
         * @brief Returns true if the event ID falls within any of the node's producer ranges.
         *
         * @param openlcb_node Pointer to the @ref openlcb_node_t whose producer ranges are checked.
         * @param event_id     64-bit @ref event_id_t to test.
         *
         * @return true if the event ID falls within a producer range.
         */
    extern bool OpenLcbUtilities_is_event_id_in_producer_ranges(openlcb_node_t *openlcb_node, event_id_t event_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_UTILITIES__ */
