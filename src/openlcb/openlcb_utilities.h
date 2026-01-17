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
 * @brief Common utility functions for working with buffers and node structures
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
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
 * @brief Loads an OpenLcb message structure with source, destination, and MTI information
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param source_alias CAN alias of the source node
 * @param source_id 64-bit node ID of the source node
 * @param dest_alias CAN alias of the destination node
 * @param dest_id 64-bit node ID of the destination node
 * @param mti Message Type Indicator
 * @return None
 */
extern void OpenLcbUtilities_load_openlcb_message(openlcb_msg_t *openlcb_msg, uint16_t source_alias, node_id_t source_id, uint16_t dest_alias, node_id_t dest_id, uint16_t mti);

/**
 * @brief Copies an event ID into the OpenLcb message payload
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param event_id Event ID to copy or check
 * @return None
 */
extern void OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg_t *openlcb_msg, event_id_t event_id);

/**
 * @brief Copies a node ID into the OpenLcb message payload at the specified index
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param node_id Node ID to copy or extract
 * @param index Index position in the payload or buffer
 * @return None
 */
extern void OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg_t *openlcb_msg, node_id_t node_id, uint16_t index);

/**
 * @brief Copies a single byte into the OpenLcb message payload at the specified offset
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param byte Byte value to copy
 * @param offset Offset position in the payload
 * @return None
 */
extern void OpenLcbUtilities_copy_byte_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint8_t byte, uint16_t offset);

/**
 * @brief Copies a 16-bit word into the OpenLcb message payload at the specified index
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param word 16-bit word value to copy
 * @param index Index position in the payload or buffer
 * @return None
 */
extern void OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t word, uint16_t index);

/**
 * @brief Copies a 32-bit double-word into the OpenLcb message payload at the specified index
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param doubleword 32-bit double-word value to copy
 * @param index Index position in the payload or buffer
 * @return None
 */
extern void OpenLcbUtilities_copy_dword_to_openlcb_payload(openlcb_msg_t *openlcb_msg, uint32_t doubleword, uint16_t index);

/**
 * @brief Copies a null-terminated string into the OpenLcb message payload
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param string Null-terminated string to copy
 * @param index Index position in the payload or buffer
 * @return Number of bytes copied
 */
extern uint16_t OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg_t *openlcb_msg, const char string[], uint16_t index);

/**
 * @brief Copies a byte array into the OpenLcb message payload
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param byte_array Array of bytes to copy
 * @param payload_index Index position in the payload
 * @param requested_bytes Number of bytes requested
 * @return Number of bytes copied
 */
extern uint16_t OpenLcbUtilities_copy_byte_array_to_openlcb_payload(openlcb_msg_t *openlcb_msg, const uint8_t byte_array[], uint16_t payload_index, uint16_t requested_bytes);

/**
 * @brief Clears the payload section of an OpenLcb message
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @return None
 */
extern void OpenLcbUtilities_clear_openlcb_message_payload(openlcb_msg_t *openlcb_msg);

/**
 * @brief Clears all fields of an OpenLcb message structure
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @return None
 */
extern void OpenLcbUtilities_clear_openlcb_message(openlcb_msg_t *openlcb_msg);

/**
 * @brief Extracts a node ID from the OpenLcb message payload at the specified index
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param index Index position in the payload or buffer
 * @return Extracted node ID value
 */
extern node_id_t OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t index);

/**
 * @brief Extracts an event ID from the OpenLcb message payload
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @return Extracted event ID value
 */
extern event_id_t OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg_t *openlcb_msg);

/**
 * @brief Extracts a single byte from the OpenLcb message payload at the specified index
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param index Index position in the payload or buffer
 * @return Extracted byte value or count
 */
extern uint8_t OpenLcbUtilities_extract_byte_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t index);

/**
 * @brief Extracts a 16-bit word from the OpenLcb message payload at the specified index
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param index Index position in the payload or buffer
 * @return Extracted word value
 */
extern uint16_t OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t index);

/**
 * @brief Extracts a 32-bit double-word from the OpenLcb message payload at the specified index
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @param index Index position in the payload or buffer
 * @return Extracted double-word value or calculated offset
 */
extern uint32_t OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg_t *openlcb_msg, uint16_t index);

/**
 * @brief Counts the number of null bytes in the OpenLcb message payload
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @return Extracted byte value or count
 */
extern uint8_t OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg_t *openlcb_msg);

/**
 * @brief Checks if the OpenLcb message is an addressed message
 * @param openlcb_msg Pointer to the OpenLcb message structure
 * @return True if condition is met, false otherwise
 */
extern bool OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg_t *openlcb_msg);

/**
 * @brief Sets the multi-frame flag in the target byte
 * @param target Pointer to target byte to set flag
 * @param flag Multi-frame flag value to set
 * @return None
 */
extern void OpenLcbUtilities_set_multi_frame_flag(uint8_t *target, uint8_t flag); // MULTIFRAME_ONLY, MULTIFRAME_FIRST, MULTIFRAME_MIDDLE, MULTIFRAME_FINAL

/**
 * @brief Checks if a producer event ID is assigned to the specified node
 * @param openlcb_node Pointer to the OpenLcb node structure
 * @param event_id Event ID to copy or check
 * @param event_index Pointer to store the event index if found
 * @return True if condition is met, false otherwise
 */
extern bool OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t *event_index);

/**
 * @brief Checks if a consumer event ID is assigned to the specified node
 * @param openlcb_node Pointer to the OpenLcb node structure
 * @param event_id Event ID to copy or check
 * @param event_index Pointer to store the event index if found
 * @return True if condition is met, false otherwise
 */
extern bool OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t *event_index);

/**
 * @brief Calculates the memory offset into the node address space
 * @param openlcb_node Pointer to the OpenLcb node structure
 * @return Extracted double-word value or calculated offset
 */
extern uint32_t OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node_t *openlcb_node);

/**
 * @brief Converts payload type enumeration to payload length in bytes
 * @param payload_type Payload type enumeration
 * @return Payload length in bytes
 */
extern uint16_t OpenLcbUtilities_payload_type_to_len(payload_type_enum payload_type);

/**
 * @brief Extracts a node ID from the configuration memory buffer at the specified index
 * @param buffer Pointer to configuration memory buffer
 * @param index Index position in the payload or buffer
 * @return Extracted node ID value
 */
extern node_id_t OpenLcbUtilities_extract_node_id_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

/**
 * @brief Extracts a 16-bit word from the configuration memory buffer at the specified index
 * @param buffer Pointer to configuration memory buffer
 * @param index Index position in the payload or buffer
 * @return Extracted word value
 */
extern uint16_t OpenLcbUtilities_extract_word_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

/**
 * @brief Copies a node ID to the configuration memory buffer at the specified index
 * @param buffer Pointer to configuration memory buffer
 * @param node_id Node ID to copy or extract
 * @param index Index position in the payload or buffer
 * @return None
 */
extern void OpenLcbUtilities_copy_node_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, node_id_t node_id, uint8_t index);

/**
 * @brief Copies an event ID to the configuration memory buffer at the specified index
 * @param buffer Pointer to configuration memory buffer
 * @param event_id Event ID to copy or check
 * @param index Index position in the payload or buffer
 * @return None
 */
extern void OpenLcbUtilities_copy_event_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, event_id_t event_id, uint8_t index);

/**
 * @brief Copies data from the configuration memory buffer to an event ID
 * @param buffer Pointer to configuration memory buffer
 * @param index Index position in the payload or buffer
 * @return Extracted event ID value
 */
extern event_id_t OpenLcbUtilities_copy_config_mem_buffer_to_event_id(configuration_memory_buffer_t *buffer, uint8_t index);

/**
 * @brief Loads the configuration memory write failure reply message header
 * @param statemachine_info Pointer to state machine information structure
 * @param config_mem_write_request_info Pointer to write request information
 * @param error_code Error code to include in the message
 * @return None
 */
extern void OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, uint16_t error_code);

/**
 * @brief Loads the configuration memory write success reply message header
 * @param statemachine_info Pointer to state machine information structure
 * @param config_mem_write_request_info Pointer to write request information
 * @return None
 */
extern void OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

/**
 * @brief Loads the configuration memory read failure reply message header
 * @param statemachine_info Pointer to state machine information structure
 * @param config_mem_read_request_info Pointer to read request information
 * @param error_code Error code to include in the message
 * @return None
 */
extern void OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info, uint16_t error_code);

/**
 * @brief Loads the configuration memory read success reply message header
 * @param statemachine_info Pointer to state machine information structure
 * @param config_mem_read_request_info Pointer to read request information
 * @return None
 */
extern void OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_UTILITIES__ */
