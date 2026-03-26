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
 * @date 18 Mar 2026
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

        case WORKER:

            return LEN_MESSAGE_BYTES_WORKER;

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
    openlcb_msg->timer.assembly_ticks = 0;

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
    openlcb_msg->timer.assembly_ticks = 0;
    openlcb_msg->reference_count = 0;
    openlcb_msg->state.allocated = false;
    openlcb_msg->state.inprocess = false;
    openlcb_msg->state.invalid = false;

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

     for (int i = 0; i < openlcb_node->consumers.range_count; i++) {

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

     for (int i = 0; i < openlcb_node->producers.range_count; i++) {

         range = &openlcb_node->producers.range_list[i];
         event_id_t start_event = range->start_base;
         event_id_t end_event = range->start_base + range->event_count;

         if ((event_id >= start_event) && (event_id <= end_event)) {

             return true;

         }

     }

     return false;
 }

    /** @brief Generates a masked Event ID covering a range of consecutive events. */
 event_id_t OpenLcbUtilities_generate_event_range_id(event_id_t base_event_id, event_range_count_enum count) {

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

