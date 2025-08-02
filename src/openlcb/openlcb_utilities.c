
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
 * \file openlcb_utilities.c
 *
 * Common utility functions that are useful to work on buffers and node structures
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "openlcb_utilities.h"

#include "stdio.h" // printf
#include "openlcb_defines.h"
#include "openlcb_types.h"

uint16_olcb_t Utilities_payload_type_to_len(payload_type_enum_t payload_type) {

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

uint32_olcb_t Utilities_calculate_memory_offset_into_node_space(openlcb_node_t* openlcb_node) {

    uint32_olcb_t offset_per_node = openlcb_node->parameters->address_space_config_memory.highest_address;

    if (openlcb_node->parameters->address_space_config_memory.low_address_valid)
        offset_per_node = openlcb_node->parameters->address_space_config_memory.highest_address - openlcb_node->parameters->address_space_config_memory.low_address;

    return (offset_per_node * openlcb_node->index);

}

void Utilities_load_openlcb_message(openlcb_msg_t* openlcb_msg, uint16_olcb_t source_alias, uint64_olcb_t source_id, uint16_olcb_t dest_alias, uint64_olcb_t dest_id, uint16_olcb_t mti, uint16_olcb_t payload_count) {

    if (openlcb_msg) {

        openlcb_msg->dest_alias = dest_alias;
        openlcb_msg->dest_id = dest_id;
        openlcb_msg->source_alias = source_alias;
        openlcb_msg->source_id = source_id;
        openlcb_msg->mti = mti;
        openlcb_msg->payload_count = payload_count;
        openlcb_msg->timerticks = 0;

        uint16_olcb_t data_count = Utilities_payload_type_to_len(openlcb_msg->payload_type);

        for (int_olcb_t i = 0; i < data_count; i++) {

            *openlcb_msg->payload[i] = 0x00;
        }

    }

}

void Utilities_clear_openlcb_message_payload(openlcb_msg_t* openlcb_msg) {

    uint16_olcb_t data_len = Utilities_payload_type_to_len(openlcb_msg->payload_type);

    for (int_olcb_t i = 0; i < data_len; i++) {

        *openlcb_msg->payload[i] = 0;

    }

    openlcb_msg->payload_count = 0;

}

void Utilities_copy_event_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, event_id_t event_id) {

    for (int_olcb_t i = 7; i >= 0; i--) {

        *openlcb_msg->payload[i] = event_id & 0xFF;
        event_id = event_id >> 8;

    }

    openlcb_msg->payload_count = 8;

}

void Utilities_copy_word_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_olcb_t word, uint16_olcb_t offset) {

    *openlcb_msg->payload[0 + offset] = (uint8_olcb_t) ((word >> 8) & 0xFF);
    *openlcb_msg->payload[1 + offset] = (uint8_olcb_t) (word & 0xFF);

}

void Utilities_copy_dword_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint32_olcb_t doubleword, uint16_olcb_t offset) {

    *openlcb_msg->payload[0 + offset] = (uint8_olcb_t) ((doubleword >> 24) & 0xFF);
    *openlcb_msg->payload[1 + offset] = (uint8_olcb_t) ((doubleword >> 16) & 0xFF);
    *openlcb_msg->payload[2 + offset] = (uint8_olcb_t) ((doubleword >> 8) & 0xFF);
    *openlcb_msg->payload[3 + offset] = (uint8_olcb_t) (doubleword & 0xFF);

}

uint16_olcb_t Utilities_copy_string_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const char string[], uint16_olcb_t payload_index) {

    uint16_olcb_t counter = 0;
    uint16_olcb_t payload_len = 0;

    while (string[counter] != 0x00) {

        payload_len = Utilities_payload_type_to_len(openlcb_msg->payload_type);

        if ((counter + payload_index) < payload_len - 1) { // leave room for a null

            *openlcb_msg->payload[counter + payload_index] = (uint8_olcb_t) string[counter];
            counter++;

        } else
            break;

    }

    *openlcb_msg->payload[counter + payload_index] = 0x00;
    counter++;

    return counter;

}

uint16_olcb_t Utilities_copy_byte_array_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const uint8_olcb_t byte_array[], uint16_olcb_t payload_index, uint16_olcb_t requested_bytes) {

    uint16_olcb_t counter = 0;
    uint16_olcb_t payload_len = 0;

    payload_len = Utilities_payload_type_to_len(openlcb_msg->payload_type);

    for (uint16_olcb_t i = 0; i < requested_bytes; i++) {

        if ((i + payload_index) < payload_len) {

            *openlcb_msg->payload[i + payload_index] = byte_array[i];
            counter++;

        } else {

            break;

        }

    }

    return counter;

}

void Utilities_copy_node_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, node_id_t node_id, uint16_olcb_t index) {

    openlcb_msg->payload_count = 6 + index;

    for (int_olcb_t i = 5; i >= 0; i--) {

        *openlcb_msg->payload[i + index] = node_id & 0xFF;
        node_id = node_id >> 8;

    }

}

node_id_t Utilities_extract_node_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_olcb_t index) {

    return (
            ((uint64_olcb_t) * openlcb_msg->payload[0 + index] << 40) |
            ((uint64_olcb_t) * openlcb_msg->payload[1 + index] << 32) |
            ((uint64_olcb_t) * openlcb_msg->payload[2 + index] << 24) |
            ((uint64_olcb_t) * openlcb_msg->payload[3 + index] << 16) |
            ((uint64_olcb_t) * openlcb_msg->payload[4 + index] << 8) |
            ((uint64_olcb_t) * openlcb_msg->payload[5 + index])
            );

}

event_id_t Utilities_extract_event_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    return (
            ((uint64_olcb_t) * openlcb_msg->payload[0] << 56) |
            ((uint64_olcb_t) * openlcb_msg->payload[1] << 48) |
            ((uint64_olcb_t) * openlcb_msg->payload[2] << 40) |
            ((uint64_olcb_t) * openlcb_msg->payload[3] << 32) |
            ((uint64_olcb_t) * openlcb_msg->payload[4] << 24) |
            ((uint64_olcb_t) * openlcb_msg->payload[5] << 16) |
            ((uint64_olcb_t) * openlcb_msg->payload[6] << 8) |
            ((uint64_olcb_t) * openlcb_msg->payload[7])
            );
}

uint16_olcb_t Utilities_extract_word_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_olcb_t offset) {

    return (
            ((uint16_olcb_t) * openlcb_msg->payload[0 + offset] << 8) |
            ((uint16_olcb_t) * openlcb_msg->payload[1 + offset])
            );

}

uint32_olcb_t Utilities_extract_dword_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_olcb_t offset) {

    return (
            ((uint32_olcb_t) * openlcb_msg->payload[0 + offset] << 24) |
            ((uint32_olcb_t) * openlcb_msg->payload[1 + offset] << 16) |
            ((uint32_olcb_t) * openlcb_msg->payload[2 + offset] << 8) |
            ((uint32_olcb_t) * openlcb_msg->payload[3 + offset])
            );

}

void Utilities_set_multi_frame_flag(uint8_olcb_t* target, uint8_olcb_t flag) {

    *target = *target | flag;

}

uint8_olcb_t Utilities_is_addressed_openlcb_message(openlcb_msg_t* openlcb_msg) {

    return ((openlcb_msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT);

}

uint8_olcb_t Utilities_count_nulls_in_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    uint8_olcb_t count = 0;

    for (int_olcb_t i = 0; i < openlcb_msg->payload_count; i++) {

        if (*openlcb_msg->payload[i] == 0x00)

            count = count + 1;

    }

    return count;

}

uint8_olcb_t Utilities_is_message_for_node(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    if ((openlcb_node->alias == openlcb_msg->dest_alias) | (openlcb_node->id == openlcb_msg->dest_id))

        return TRUE;

    else

        return FALSE;

}

uint8_olcb_t Utilities_is_producer_event_assigned_to_node(openlcb_node_t* openlcb_node, uint64_olcb_t event_id, uint16_olcb_t *event_index) {

    uint16_olcb_t indexer = 0;

    for (int_olcb_t i = 0; i < openlcb_node->producers.count; i++) {

        if (i < USER_DEFINED_PRODUCER_COUNT)

            if (openlcb_node->producers.list[i] == event_id) {

                (*event_index) = indexer;
                indexer++;

                return TRUE;

            }


    }

    return FALSE;

}

uint8_olcb_t Utilities_is_consumer_event_assigned_to_node(openlcb_node_t* openlcb_node, uint64_olcb_t event_id, uint16_olcb_t* event_index) {

    uint16_olcb_t indexer = 0;

    for (int_olcb_t i = 0; i < openlcb_node->consumers.count; i++) {

        if (i < USER_DEFINED_CONSUMER_COUNT)

            if (openlcb_node->consumers.list[i] == event_id) {

                (*event_index) = indexer;
                indexer++;

                return TRUE;

            }

    }

    return FALSE;


}

uint8_olcb_t Utilities_addressed_message_needs_processing(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {


    if (Utilities_is_message_for_node(openlcb_node, openlcb_msg)) {

        if (openlcb_node->state.openlcb_msg_handled)

            return FALSE;

        else

            return TRUE;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

    return FALSE;

}
