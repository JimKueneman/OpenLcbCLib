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
 * @brief Common utility functions for OpenLCB message and buffer manipulation.
 *
 * @details All multi-byte values follow OpenLCB big-endian (network byte order)
 * convention. Payload insert functions increment payload_count; extract functions
 * do not modify it.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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
#include "openlcb_application_broadcast_time.h"

// =============================================================================
// Message Structure Operations
// =============================================================================

    /** @brief Converts a @ref payload_type_enum to its maximum byte length. */
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

    /** @brief Returns the byte offset into global config memory where this node's space begins. */
uint32_t OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node_t* openlcb_node) {

    uint32_t offset_per_node = openlcb_node->parameters->address_space_config_memory.highest_address;

    if (openlcb_node->parameters->address_space_config_memory.low_address_valid) {

        offset_per_node = openlcb_node->parameters->address_space_config_memory.highest_address - openlcb_node->parameters->address_space_config_memory.low_address;

    }

    return (offset_per_node * openlcb_node->index);

}

    /** @brief Loads message header fields and clears the payload to zeros. */
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

    /** @brief Zeros all payload bytes and resets payload_count. Header preserved. */
void OpenLcbUtilities_clear_openlcb_message_payload(openlcb_msg_t* openlcb_msg) {

    uint16_t data_len = OpenLcbUtilities_payload_type_to_len(openlcb_msg->payload_type);

    for (int i = 0; i < data_len; i++) {

    *openlcb_msg->payload[i] = 0;

    }

    openlcb_msg->payload_count = 0;

}

    /** @brief Zeros entire message including header, state flags, and reference count. */
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

// =============================================================================
// Payload Insert Functions (all big-endian, all increment payload_count)
// =============================================================================

    /** @brief Copies an 8-byte event ID to payload at offset 0. */
void OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, event_id_t event_id) {

    for (int i = 7; i >= 0; i--) {

    *openlcb_msg->payload[i] = event_id & 0xFF;
        openlcb_msg->payload_count++;
        event_id = event_id >> 8;

    }

    openlcb_msg->payload_count = 8;

}

    /** @brief Copies one byte to payload at the given offset. */
void OpenLcbUtilities_copy_byte_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint8_t byte, uint16_t offset) {

    *openlcb_msg->payload[offset] = byte;

    openlcb_msg->payload_count++;

}

    /** @brief Copies a 16-bit word (big-endian) to payload at the given offset. */
void OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t word, uint16_t offset) {

    *openlcb_msg->payload[0 + offset] = (uint8_t) ((word >> 8) & 0xFF);
    *openlcb_msg->payload[1 + offset] = (uint8_t) (word & 0xFF);

    openlcb_msg->payload_count += 2;

}

    /** @brief Copies a 32-bit doubleword (big-endian) to payload at the given offset. */
void OpenLcbUtilities_copy_dword_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint32_t doubleword, uint16_t offset) {

    *openlcb_msg->payload[0 + offset] = (uint8_t) ((doubleword >> 24) & 0xFF);
    *openlcb_msg->payload[1 + offset] = (uint8_t) ((doubleword >> 16) & 0xFF);
    *openlcb_msg->payload[2 + offset] = (uint8_t) ((doubleword >> 8) & 0xFF);
    *openlcb_msg->payload[3 + offset] = (uint8_t) (doubleword & 0xFF);

    openlcb_msg->payload_count += 4;

}

    /**
     * @brief Copies a null-terminated string into the payload.
     *
     * @details Truncates if payload space is insufficient but always adds a
     * null terminator.
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

    /** @brief Copies a byte array into the payload. May copy fewer bytes if payload space is exhausted. */
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

    /** @brief Copies a 6-byte node ID (big-endian) to payload at the given offset. */
void OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, node_id_t node_id, uint16_t offset) {

    for (int i = 5; i >= 0; i--) {

    *openlcb_msg->payload[(uint16_t) i + offset] = node_id & 0xFF;
        openlcb_msg->payload_count++;
        node_id = node_id >> 8;

    }

}

// =============================================================================
// Payload Extract Functions (all big-endian, none modify payload_count)
// =============================================================================

    /** @brief Extracts a 6-byte node ID from payload at the given offset. */
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

    /** @brief Extracts an 8-byte event ID from payload at offset 0. */
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

    /** @brief Extracts one byte from payload at the given offset. */
uint8_t OpenLcbUtilities_extract_byte_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (*openlcb_msg->payload[offset]);

}

    /** @brief Extracts a 16-bit word (big-endian) from payload at the given offset. */
uint16_t OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (
            ((uint16_t) * openlcb_msg->payload[0 + offset] << 8) |
            ((uint16_t) * openlcb_msg->payload[1 + offset])
            );

}

    /** @brief Extracts a 32-bit doubleword (big-endian) from payload at the given offset. */
uint32_t OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (
            ((uint32_t) * openlcb_msg->payload[0 + offset] << 24) |
            ((uint32_t) * openlcb_msg->payload[1 + offset] << 16) |
            ((uint32_t) * openlcb_msg->payload[2 + offset] << 8) |
            ((uint32_t) * openlcb_msg->payload[3 + offset])
            );

}

// =============================================================================
// Message Classification
// =============================================================================

    /** @brief Sets the multi-frame control flag in the upper nibble of target, preserving the lower nibble. */
void OpenLcbUtilities_set_multi_frame_flag(uint8_t* target, uint8_t flag) {

    *target = *target & 0x0F;

    *target = *target | flag;

}

    /** @brief Returns true if the MTI has the destination-address-present bit set. */
bool OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg_t* openlcb_msg) {

    return ((openlcb_msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT);

}

    /** @brief Returns the count of null bytes (0x00) in the payload. Used for SNIP validation. */
uint8_t OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    uint8_t count = 0;

    for (int i = 0; i < openlcb_msg->payload_count; i++) {

        if (*openlcb_msg->payload[i] == 0x00) {

            count = count + 1;

        }

    }

    return count;

}

    /** @brief Returns true if the message destination matches this node's alias or ID. */
bool OpenLcbUtilities_is_addressed_message_for_node(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    if ((openlcb_node->alias == openlcb_msg->dest_alias) || (openlcb_node->id == openlcb_msg->dest_id)) {

        return true;

    } else {

        return false;

    }

}

// =============================================================================
// Event Assignment Lookups
// =============================================================================

    /** @brief Searches the node's producer list for a matching event ID. */
bool OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node_t* openlcb_node, event_id_t event_id, uint16_t *event_index) {

    for (int i = 0; i < openlcb_node->producers.count; i++) {

        if (openlcb_node->producers.list[i].event == event_id) {

            (*event_index) = (uint16_t) i;

            return true;

        }

    }

    return false;

}

    /** @brief Searches the node's consumer list for a matching event ID. */
bool OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node_t* openlcb_node, event_id_t event_id, uint16_t* event_index) {

    for (int i = 0; i < openlcb_node->consumers.count; i++) {

        if (openlcb_node->consumers.list[i].event == event_id) {

            (*event_index) = (uint16_t) i;

            return true;

        }

    }

    return false;

}

// =============================================================================
// Configuration Memory Buffer Operations (all big-endian)
// =============================================================================

    /** @brief Extracts a 6-byte node ID from a config memory buffer at the given index. */
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

    /** @brief Extracts a 16-bit word from a config memory buffer at the given index. */
uint16_t OpenLcbUtilities_extract_word_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index) {

    return (
            ((uint16_t) (*buffer)[0 + index] << 8) |
            ((uint16_t) (*buffer)[1 + index])
            );

}

    /** @brief Copies a 6-byte node ID into a config memory buffer at the given index. */
void OpenLcbUtilities_copy_node_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, node_id_t node_id, uint8_t index) {

    for (int i = 5; i >= 0; i--) {

        (*buffer)[i + index] = node_id & 0xFF;
        node_id = node_id >> 8;

    }

}

    /** @brief Copies an 8-byte event ID into a config memory buffer at the given index. */
void OpenLcbUtilities_copy_event_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, event_id_t event_id, uint8_t index) {


    for (int i = 7; i >= 0; i--) {

        (*buffer)[i + index] = event_id & 0xFF;
        event_id = event_id >> 8;

    }

}

    /** @brief Extracts an 8-byte event ID from a config memory buffer at the given index. */
event_id_t OpenLcbUtilities_copy_config_mem_buffer_to_event_id(configuration_memory_buffer_t *buffer, uint8_t index) {


    event_id_t retval = 0L;

    for (int i = 0; i <= 7; i++) {

        retval = retval << 8;
        retval |= (*buffer)[i + index] & 0xFF;

    }

    return retval;

}

// =============================================================================
// Configuration Memory Reply Builders
// =============================================================================

    /** @brief Builds a config memory write-success reply datagram header. */
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
     * @brief Builds a config memory write-failure reply datagram header.
     *
     * @details Error code placement depends on address encoding:
     * ADDRESS_SPACE_IN_BYTE_6 places it at offset 7, otherwise offset 6.
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
     * @brief Builds a config memory read-success reply datagram header only.
     *
     * @details Caller must append actual data bytes separately after this call.
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
     * @brief Builds a config memory read-failure reply datagram header.
     *
     * @details Error code is placed at the data_start offset where actual
     * data would have been.
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

// =============================================================================
// Event Range Utilities
// =============================================================================

    /** @brief Returns true if the event ID falls within any of the node's consumer ranges. */
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

    /** @brief Returns true if the event ID falls within any of the node's producer ranges. */
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

    /** @brief Generates a masked Event ID covering a range of consecutive events. */
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

// =============================================================================
// Broadcast Time Event Utilities
// =============================================================================

    /** @brief Returns true if the event ID belongs to the broadcast time event space. */
 bool OpenLcbUtilities_is_broadcast_time_event(event_id_t event_id)
 {

     uint64_t clock_id;

     clock_id = event_id & BROADCAST_TIME_MASK_CLOCK_ID;

     if (clock_id == BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK)
     {

         return true;
     }

     if (clock_id == BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK)
     {

         return true;
     }

     if (clock_id == BROADCAST_TIME_ID_ALTERNATE_CLOCK_1)
     {

         return true;
     }

     if (clock_id == BROADCAST_TIME_ID_ALTERNATE_CLOCK_2)
     {

         return true;
     }

     // Check registered custom clocks
     return OpenLcbApplicationBroadcastTime_get_clock(clock_id) != NULL;
 }

    /** @brief Extracts the 48-bit clock ID (upper 6 bytes) from a broadcast time event ID. */
 uint64_t OpenLcbUtilities_extract_clock_id_from_time_event(event_id_t event_id)
 {

     return event_id & BROADCAST_TIME_MASK_CLOCK_ID;
 }

    /** @brief Returns the @ref broadcast_time_event_type_enum for a broadcast time event ID. */
 broadcast_time_event_type_enum OpenLcbUtilities_get_broadcast_time_event_type(event_id_t event_id)
 {

     uint16_t command_data;

     command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

     if (command_data == BROADCAST_TIME_QUERY)
     {

         return BROADCAST_TIME_EVENT_QUERY;
     }

     if (command_data == BROADCAST_TIME_STOP)
     {

         return BROADCAST_TIME_EVENT_STOP;
     }

     if (command_data == BROADCAST_TIME_START)
     {

         return BROADCAST_TIME_EVENT_START;
     }

     if (command_data == BROADCAST_TIME_DATE_ROLLOVER)
     {

         return BROADCAST_TIME_EVENT_DATE_ROLLOVER;
     }

     // Set Rate: 0xC000-0xCFFF
     if (command_data >= BROADCAST_TIME_SET_RATE_BASE && command_data <= 0xCFFF)
     {

         return BROADCAST_TIME_EVENT_SET_RATE;
     }

     // Set Year: 0xB000-0xBFFF
     if (command_data >= BROADCAST_TIME_SET_YEAR_BASE && command_data <= 0xBFFF)
     {

         return BROADCAST_TIME_EVENT_SET_YEAR;
     }

     // Set Date: 0xA100-0xACFF
     if (command_data >= BROADCAST_TIME_SET_DATE_BASE && command_data <= 0xACFF)
     {

         return BROADCAST_TIME_EVENT_SET_DATE;
     }

     // Set Time: 0x8000-0x97FF
     if (command_data >= BROADCAST_TIME_SET_TIME_BASE && command_data <= 0x97FF)
     {

         return BROADCAST_TIME_EVENT_SET_TIME;
     }

     // Report Rate: 0x4000-0x4FFF
     if (command_data >= BROADCAST_TIME_REPORT_RATE_BASE && command_data <= 0x4FFF)
     {

         return BROADCAST_TIME_EVENT_REPORT_RATE;
     }

     // Report Year: 0x3000-0x3FFF
     if (command_data >= BROADCAST_TIME_REPORT_YEAR_BASE && command_data <= 0x3FFF)
     {

         return BROADCAST_TIME_EVENT_REPORT_YEAR;
     }

     // Report Date: 0x2100-0x2CFF
     if (command_data >= BROADCAST_TIME_REPORT_DATE_BASE && command_data <= 0x2CFF)
     {

         return BROADCAST_TIME_EVENT_REPORT_DATE;
     }

     // Report Time: 0x0000-0x17FF
     if (command_data <= 0x17FF)
     {

         return BROADCAST_TIME_EVENT_REPORT_TIME;
     }

     return BROADCAST_TIME_EVENT_UNKNOWN;
 }

    /** @brief Extracts hour and minute from a broadcast time event ID. Returns false if out of range. */
 bool OpenLcbUtilities_extract_time_from_event_id(event_id_t event_id, uint8_t *hour, uint8_t *minute)
 {

     uint16_t command_data;
     uint8_t h;
     uint8_t m;

     if (!hour || !minute)
     {

         return false;
     }

     command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

     // Strip the set command offset if present
     if (command_data >= BROADCAST_TIME_SET_COMMAND_OFFSET)
     {

         command_data = command_data - BROADCAST_TIME_SET_COMMAND_OFFSET;
     }

     h = (uint8_t)(command_data >> 8);
     m = (uint8_t)(command_data & 0xFF);

     if (h >= 24 || m >= 60)
     {

         return false;
     }

     *hour = h;
     *minute = m;

     return true;
 }

    /** @brief Extracts month and day from a broadcast time event ID. Returns false if out of range. */
 bool OpenLcbUtilities_extract_date_from_event_id(event_id_t event_id, uint8_t *month, uint8_t *day)
 {

     uint16_t command_data;
     uint8_t mon;
     uint8_t d;

     if (!month || !day)
     {

         return false;
     }

     command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

     // Strip the set command offset if present
     if (command_data >= BROADCAST_TIME_SET_COMMAND_OFFSET)
     {

         command_data = command_data - BROADCAST_TIME_SET_COMMAND_OFFSET;
     }

     // Date format: byte 6 = 0x20 + month, byte 7 = day
     // So command_data upper byte = 0x20 + month
     mon = (uint8_t)((command_data >> 8) - 0x20);
     d = (uint8_t)(command_data & 0xFF);

     if (mon < 1 || mon > 12 || d < 1 || d > 31)
     {

         return false;
     }

     *month = mon;
     *day = d;

     return true;
 }

    /** @brief Extracts year from a broadcast time event ID. Returns false if out of range. */
 bool OpenLcbUtilities_extract_year_from_event_id(event_id_t event_id, uint16_t *year)
 {

     uint16_t command_data;
     uint16_t y;

     if (!year)
     {

         return false;
     }

     command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

     // Strip the set command offset if present
     if (command_data >= BROADCAST_TIME_SET_COMMAND_OFFSET)
     {

         command_data = command_data - BROADCAST_TIME_SET_COMMAND_OFFSET;
     }

     // Year format: 0x3000 + year (0-4095)
     y = command_data - BROADCAST_TIME_REPORT_YEAR_BASE;

     if (y > 4095)
     {

         return false;
     }

     *year = y;

     return true;
 }

    /**
     * @brief Extracts the 12-bit signed fixed-point rate from a broadcast time event ID.
     *
     * @details Rate format is 10.2 fixed point. Sign-extends bit 11 for negative rates.
     */
 bool OpenLcbUtilities_extract_rate_from_event_id(event_id_t event_id, int16_t *rate)
 {

     uint16_t command_data;
     uint16_t raw_rate;

     if (!rate)
     {

         return false;
     }

     command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

     // Strip the set command offset if present
     if (command_data >= BROADCAST_TIME_SET_COMMAND_OFFSET)
     {

         command_data = command_data - BROADCAST_TIME_SET_COMMAND_OFFSET;
     }

     // Rate format: 0x4000 + 12-bit signed fixed point
     raw_rate = command_data - BROADCAST_TIME_REPORT_RATE_BASE;

     // 12-bit signed: sign extend if bit 11 is set
     if (raw_rate & 0x0800)
     {

         *rate = (int16_t)(raw_rate | 0xF000);
     }
     else
     {

         *rate = (int16_t)raw_rate;
     }

     return true;
 }

    /** @brief Creates a Report/Set Time event ID from clock_id, hour, minute. */
 event_id_t OpenLcbUtilities_create_time_event_id(uint64_t clock_id, uint8_t hour, uint8_t minute, bool is_set)
 {

     uint16_t command_data;

     command_data = ((uint16_t)hour << 8) | (uint16_t)minute;

     if (is_set)
     {

         command_data = command_data + BROADCAST_TIME_SET_COMMAND_OFFSET;
     }

     return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;
 }

    /** @brief Creates a Report/Set Date event ID from clock_id, month, day. */
 event_id_t OpenLcbUtilities_create_date_event_id(uint64_t clock_id, uint8_t month, uint8_t day, bool is_set)
 {

     uint16_t command_data;

     command_data = ((uint16_t)(0x20 + month) << 8) | (uint16_t)day;

     if (is_set)
     {

         command_data = command_data + BROADCAST_TIME_SET_COMMAND_OFFSET;
     }

     return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;
 }

    /** @brief Creates a Report/Set Year event ID from clock_id, year. */
 event_id_t OpenLcbUtilities_create_year_event_id(uint64_t clock_id, uint16_t year, bool is_set)
 {

     uint16_t command_data;

     command_data = BROADCAST_TIME_REPORT_YEAR_BASE + year;

     if (is_set)
     {

         command_data = command_data + BROADCAST_TIME_SET_COMMAND_OFFSET;
     }

     return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;
 }

    /** @brief Creates a Report/Set Rate event ID from clock_id, rate. */
 event_id_t OpenLcbUtilities_create_rate_event_id(uint64_t clock_id, int16_t rate, bool is_set)
 {

     uint16_t command_data;

     command_data = BROADCAST_TIME_REPORT_RATE_BASE + ((uint16_t)rate & 0x0FFF);

     if (is_set)
     {

         command_data = command_data + BROADCAST_TIME_SET_COMMAND_OFFSET;
     }

     return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;
 }

    /** @brief Creates a command event ID (Query, Start, Stop, Date Rollover) for the given clock. */
 event_id_t OpenLcbUtilities_create_command_event_id(uint64_t clock_id, broadcast_time_event_type_enum command)
 {

     uint16_t command_data;

     switch (command)
     {

     case BROADCAST_TIME_EVENT_QUERY:

         command_data = BROADCAST_TIME_QUERY;
         break;

     case BROADCAST_TIME_EVENT_STOP:

         command_data = BROADCAST_TIME_STOP;
         break;

     case BROADCAST_TIME_EVENT_START:

         command_data = BROADCAST_TIME_START;
         break;

     case BROADCAST_TIME_EVENT_DATE_ROLLOVER:

         command_data = BROADCAST_TIME_DATE_ROLLOVER;
         break;

     default:

         command_data = 0;
         break;
     }

     return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;
 }

// =============================================================================
// Train Search Event Utilities
// =============================================================================

    /** @brief Returns true if the event ID belongs to the train search space. */
bool OpenLcbUtilities_is_train_search_event(event_id_t event_id) {

    return (event_id & TRAIN_SEARCH_MASK) == EVENT_TRAIN_SEARCH_SPACE;

}

    /** @brief Extracts 6 search-query nibbles from a train search event ID into digits[]. */
void OpenLcbUtilities_extract_train_search_digits(event_id_t event_id, uint8_t *digits) {

    if (!digits) { return; }

    // Bytes 4-6 contain 6 nibbles (bits 31-8)
    // Byte 4 = bits 31-24: nibbles 0 and 1
    // Byte 5 = bits 23-16: nibbles 2 and 3
    // Byte 6 = bits 15-8:  nibbles 4 and 5

    uint32_t lower = (uint32_t)(event_id & 0xFFFFFFFF);

    digits[0] = (uint8_t)((lower >> 28) & 0x0F);
    digits[1] = (uint8_t)((lower >> 24) & 0x0F);
    digits[2] = (uint8_t)((lower >> 20) & 0x0F);
    digits[3] = (uint8_t)((lower >> 16) & 0x0F);
    digits[4] = (uint8_t)((lower >> 12) & 0x0F);
    digits[5] = (uint8_t)((lower >> 8) & 0x0F);

}

    /** @brief Extracts the flags byte (byte 7) from a train search event ID. */
uint8_t OpenLcbUtilities_extract_train_search_flags(event_id_t event_id) {

    return (uint8_t)(event_id & 0xFF);

}

    /** @brief Converts a 6-nibble digit array to a numeric DCC address, skipping leading 0xF nibbles. */
uint16_t OpenLcbUtilities_train_search_digits_to_address(const uint8_t *digits) {

    if (!digits) { return 0; }

    uint16_t address = 0;

    for (int i = 0; i < 6; i++) {

        if (digits[i] <= 9) {

            address = address * 10 + digits[i];

        }

    }

    return address;

}

    /** @brief Creates a train search event ID from a DCC address and flags byte. */
event_id_t OpenLcbUtilities_create_train_search_event_id(uint16_t address, uint8_t flags) {

    // Encode address as decimal digits into 6 nibbles, right-justified, padded with 0xF
    uint8_t digits[6];
    int i;

    for (i = 0; i < 6; i++) {

        digits[i] = 0x0F;

    }

    // Fill from right to left with decimal digits
    i = 5;
    if (address == 0) {

        digits[i] = 0;

    } else {

        while (address > 0 && i >= 0) {

            digits[i] = (uint8_t)(address % 10);
            address /= 10;
            i--;

        }

    }

    // Build the lower 4 bytes: 6 nibbles + flags byte
    uint32_t lower = 0;
    lower |= ((uint32_t)digits[0] << 28);
    lower |= ((uint32_t)digits[1] << 24);
    lower |= ((uint32_t)digits[2] << 20);
    lower |= ((uint32_t)digits[3] << 16);
    lower |= ((uint32_t)digits[4] << 12);
    lower |= ((uint32_t)digits[5] << 8);
    lower |= (uint32_t)flags;

    return EVENT_TRAIN_SEARCH_SPACE | (event_id_t)lower;

}

    /** @brief Returns true if the event ID is one of the 4 well-known emergency events. */
bool OpenLcbUtilities_is_emergency_event(event_id_t event_id) {

    return (event_id == EVENT_ID_EMERGENCY_OFF) ||
           (event_id == EVENT_ID_CLEAR_EMERGENCY_OFF) ||
           (event_id == EVENT_ID_EMERGENCY_STOP) ||
           (event_id == EVENT_ID_CLEAR_EMERGENCY_STOP);

}
