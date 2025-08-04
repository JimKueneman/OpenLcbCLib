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
 * \file can_utilities.c
 *
 * Utilities to help deal with the CAN buffers
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_utilities.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"

#include "../../openlcb/openlcb_utilities.h"

void CanUtilities_clear_can_message(can_msg_t *can_msg) {

    can_msg->identifier = 0;
    can_msg->payload_count = 0;

    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++) {

        can_msg->payload[i] = 0x00;

    }

}

uint16_t CanUtilities_extract_source_alias_from_can_identifier(can_msg_t *can_msg) {

    return (can_msg->identifier & 0x000000FFF);

}

uint16_t CanUtilties_extract_dest_alias_from_can_message(can_msg_t *can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

        case CAN_FRAME_TYPE_GLOBAL_ADDRESSED:
        case CAN_FRAME_TYPE_STREAM:

            if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT) {

                return ((uint16_t) ((can_msg->payload[0] & 0x0F) << 8) | (uint16_t) (can_msg->payload[1]));

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

uint16_t CanUtilties_convert_can_mti_to_openlcb_mti(can_msg_t *can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

        case CAN_FRAME_TYPE_GLOBAL_ADDRESSED:
        case CAN_FRAME_TYPE_STREAM:

            return (can_msg->identifier >> 12) & 0x0FFF;

        case CAN_FRAME_TYPE_DATAGRAM_ONLY:
        case CAN_FRAME_TYPE_DATAGRAM_FIRST:
        case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:
        case CAN_FRAME_TYPE_DATAGRAM_FINAL:

            return MTI_DATAGRAM;

        default:

            return 0;

    }
}

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

uint8_t CanUtilities_copy_can_payload_to_openlcb_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint8_t can_start_index) {

    openlcb_msg->payload_count = 0; // replacing the contents so reset to index 0

    return CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg, can_msg, can_start_index);

}

static uint8_t _count_nulls_in_can_payload(can_msg_t *can_msg) {

    uint8_t count = 0;

    for (int i = 0; i < can_msg->payload_count; i++) {

        if (can_msg->payload[i] == 0x00) {

            count++;

        }

    };

    return count;
}

uint8_t CanUtilities_count_nulls_in_payloads(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg) {

    return _count_nulls_in_can_payload(can_msg) + OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg);

}

bool CanUtilities_is_openlcb_message(can_msg_t *can_msg) {

    return (can_msg->identifier & CAN_OPENLCB_MSG) == CAN_OPENLCB_MSG;

}

uint64_t CanUtilities_extract_can_payload_as_node_id(can_msg_t *can_msg) {


    return (
            ((uint64_t) can_msg->payload[0] << 40) |
            ((uint64_t) can_msg->payload[1] << 32) |
            ((uint64_t) can_msg->payload[2] << 24) |
            ((uint64_t) can_msg->payload[3] << 16) |
            ((uint64_t) can_msg->payload[4] << 8) |
            ((uint64_t) can_msg->payload[5]));

}

uint8_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint16_t openlcb_start_index, uint8_t can_start_index) {

    can_msg->payload_count = 0;
    uint8_t count = 0;

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

void CanUtilties_load_can_message(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8) {

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

uint8_t CanUtilities_copy_node_id_to_payload(can_msg_t *can_msg, uint64_t node_id, uint8_t start_offset) {

    can_msg->payload_count = 6 + start_offset;

    for (int i = (start_offset + 5); i >= (0 + start_offset); i--) { // This is a count down...

        can_msg->payload[i] = node_id & 0xFF;
        node_id = node_id >> 8;

    }

    return can_msg->payload_count;

}

bool CanUtilities_copy_64_bit_to_can_message(can_msg_t *can_msg, uint64_t data) {

    for (int i = 7; i >= 0; i--) {

        can_msg->payload[i] = data & 0xFF;
        data = data >> 8;

    }

    can_msg->payload_count = 8;

    return true;
}

bool CanUtilities_copy_can_message(can_msg_t *can_msg_source, can_msg_t *can_msg_target) {

    can_msg_target->identifier = can_msg_source->identifier;

    for (int i = 0; i < can_msg_source->payload_count; i++) {

        can_msg_target->payload[i] = can_msg_source->payload[i];

    }

    can_msg_target->payload_count = can_msg_source->payload_count;

    return true;
}
