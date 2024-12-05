
/*
 * File:   openlcb_utilities.c
 * Author: jimkueneman
 *
 * Created on November 6, 2024, 5:44 AM
 */


#include "xc.h"
#include "stdio.h" // printf
#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"

void Utilities_clone_openlcb_message(openlcb_msg_t* source, openlcb_msg_t* target) {

    if (source && target) {

        if (source->payload_size == target->payload_size) {

            target->dest_alias = source->dest_alias;
            target->dest_id = source->dest_id;
            target->mti = source->mti;
            target->payload = source->payload;
            target->payload_count = source->payload_count;
            target->payload_size = source->payload_size;
            target->retry_count = source->retry_count;
            target->source_alias = source->source_alias;
            target->source_id = source->source_id;
            target->state = source->state;
            target->timerticks = source->timerticks;

        }

    }

}

void Utilities_load_openlcb_message(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t source_id, uint16_t dest_alias, uint64_t dest_id, uint16_t mti, uint16_t payload_count) {

    if (openlcb_msg) {

        openlcb_msg->dest_alias = dest_alias;
        openlcb_msg->dest_id = dest_id;
        openlcb_msg->source_alias = source_alias;
        openlcb_msg->source_id = source_id;
        openlcb_msg->mti = mti;
        openlcb_msg->payload_count = payload_count;
        openlcb_msg->retry_count = 0;
        openlcb_msg->timerticks = 0;

    }

}

void Utilities_clear_openlcb_message_payload(openlcb_msg_t* openlcb_msg) {

    for (int i = 0; i < openlcb_msg->payload_size; i++)
        *openlcb_msg->payload[i] = 0;

    openlcb_msg->payload_count = 0;

}

void Utilities_copy_event_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, event_id_t event_id) {

    for (int i = 7; i >= 0; i--) {

        *openlcb_msg->payload[i] = event_id & 0xFF;
        event_id = event_id >> 8;

    }

    openlcb_msg->payload_count = 8;

}

void Utilities_copy_openlcb_message(openlcb_msg_t* source, openlcb_msg_t* target) {

    if (source->payload_size == target->payload_size) {

        target->dest_alias = source->dest_alias;
        target->dest_id = source->dest_id;
        target->mti = source->mti;
        target->payload_count = source->payload_count;
        target->source_alias = source->source_alias;
        target->source_id = source->source_id;

        for (int i = 0; i < source->payload_count; i++)
            *target->payload[i] = *source->payload[i];

    }

}

void Utilities_copy_word_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t word, uint8_t offset) {

    *openlcb_msg->payload[0 + offset] = (uint8_t) ((word >> 8) & 0xFF);
    *openlcb_msg->payload[1 + offset] = (uint8_t) (word & 0xFF);

}

void Utilities_copy_dword_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint32_t doubleword, uint8_t offset) {

    *openlcb_msg->payload[0 + offset] = (uint8_t) ((doubleword >> 24) & 0xFF);
    *openlcb_msg->payload[1 + offset] = (uint8_t) ((doubleword >> 16) & 0xFF);
    *openlcb_msg->payload[2 + offset] = (uint8_t) ((doubleword >> 8) & 0xFF);
    *openlcb_msg->payload[3 + offset] = (uint8_t) (doubleword & 0xFF);

}

uint16_t Utilities_copy_string_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const char string[], uint8_t payload_index) {

    uint16_t counter = 0;

    while (string[counter] != 0x00) {

        if ((counter + payload_index) < openlcb_msg->payload_size - 1) { // leave room for a null

            *openlcb_msg->payload[counter + payload_index] = (uint8_t) string[counter];
            counter++;

        } else
            break;

    }

    *openlcb_msg->payload[counter + payload_index] = 0x00;
    counter++;

    return counter;

}

uint16_t Utilities_copy_byte_array_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const uint8_t byte_array[], uint8_t payload_index, uint16_t data_count) {

    uint16_t counter = 0;

    for (int i = 0; i < data_count; i++) {

        if ((i + payload_index) < openlcb_msg->payload_size) {

            *openlcb_msg->payload[i + payload_index] = byte_array[i];
            counter++;

        } else
            break;

    }

    return counter;

}

void Utilities_copy_node_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, node_id_t node_id, uint8_t index) {

    openlcb_msg->payload_count = 6 + index;

    for (int iIndex = 5; iIndex >= 0; iIndex--) {
        *openlcb_msg->payload[iIndex + index] = node_id & 0xFF;
        node_id = node_id >> 8;

    }

}

void Utilities_copy_64_bit_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint64_t data) {

    for (int i = 7; i >= 0; i--) {

        *openlcb_msg->payload[i] = data & 0xFF;
        data = data >> 8;

    }

    openlcb_msg->payload_count = 8;

}

node_id_t Utilities_extract_node_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint8_t index) {

    return (
            ((uint64_t) * openlcb_msg->payload[0 + index] << 40) |
            ((uint64_t) * openlcb_msg->payload[1 + index] << 32) |
            ((uint64_t) * openlcb_msg->payload[2 + index] << 24) |
            ((uint64_t) * openlcb_msg->payload[3 + index] << 16) |
            ((uint64_t) * openlcb_msg->payload[4 + index] << 8) |
            ((uint64_t) * openlcb_msg->payload[5 + index])
            );

}

event_id_t Utilities_extract_event_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg) {

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

uint16_t Utilities_extract_word_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (
            ((uint16_t) * openlcb_msg->payload[0 + offset] << 8) |
            ((uint16_t) * openlcb_msg->payload[1 + offset])
            );

}

uint32_t Utilities_extract_dword_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {

    return (
            ((uint32_t) * openlcb_msg->payload[0 + offset] << 24) |
            ((uint32_t) * openlcb_msg->payload[1 + offset] << 16) |
            ((uint32_t) * openlcb_msg->payload[2 + offset] << 8) |
            ((uint32_t) * openlcb_msg->payload[3 + offset])
            );

}

void Utilities_set_multi_frame_flag(uint8_t* target, uint8_t flag) {

    *target = *target | flag;

}

uint8_t Utilities_is_addressed_openlcb_message(openlcb_msg_t* openlcb_msg) {

    return ((openlcb_msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT);

}

uint8_t Utilities_count_nulls_in_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    uint8_t count = 0;

    for (int i = 0; i < openlcb_msg->payload_count; i++) {

        if (*openlcb_msg->payload[i] == 0x00)

            count = count + 1;

    }

    return count;

}

uint8_t Utilities_is_message_for_node(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    if ((openlcb_node->alias == openlcb_msg->dest_alias) | (openlcb_node->id == openlcb_msg->dest_id))

        return TRUE

    else

        return FALSE

    }

int Utilities_is_producer_event_assigned_to_node(openlcb_node_t* openlcb_node, uint64_t event_id) {

    for (int i = 0; i < openlcb_node->parameters->producer_count; i++) {

        if (i < LEN_CONSUMER_MAX_COUNT)

            if (openlcb_node->producers.list[i] == event_id)

                return i;


    }

    return -1;

}

int Utilities_is_consumer_event_assigned_to_node(openlcb_node_t* openlcb_node, uint64_t event_id) {

    for (int i = 0; i < openlcb_node->parameters->consumer_count; i++) {

        if (i < LEN_CONSUMER_MAX_COUNT)

            if (openlcb_node->consumers.list[i] == event_id)

                return i;

    }

    return -1;


}

uint8_t Utilities_addressed_message_needs_processing(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {


    if (Utilities_is_message_for_node(openlcb_node, openlcb_msg)) {

        if (openlcb_node->state.openlcb_msg_handled)

            return FALSE

        else

            return TRUE;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

    return FALSE;

}