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
 * @file can_utilities.c
 * @brief Implementation of utility functions for CAN frame buffers.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "can_utilities.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"

#include "../../openlcb/openlcb_utilities.h"

    /** @brief Clears identifier, payload_count, and all payload bytes in a @ref can_msg_t. */
void CanUtilities_clear_can_message(can_msg_t *can_msg) {

    can_msg->identifier = 0;
    can_msg->payload_count = 0;

    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++) {

        can_msg->payload[i] = 0x00;

    }

}

    /**
     * @brief Loads identifier, payload size, and all 8 data bytes into a @ref can_msg_t.
     *
     * @verbatim
     * @param can_msg      Destination buffer.
     * @param identifier   29-bit CAN extended identifier.
     * @param payload_size Number of valid payload bytes (0-8).
     * @param byte1..byte8 Payload bytes in order.
     * @endverbatim
     */
void CanUtilities_load_can_message(
            can_msg_t *can_msg,
            uint32_t identifier,
            uint8_t payload_size,
            uint8_t byte1,
            uint8_t byte2,
            uint8_t byte3,
            uint8_t byte4,
            uint8_t byte5,
            uint8_t byte6,
            uint8_t byte7,
            uint8_t byte8) {

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

    /**
     * @brief Copies a 48-bit Node ID into CAN message payload starting at start_offset.
     *
     * @details Algorithm:
     * -# Validate start_offset <= 2; return 0 if invalid.
     * -# Set payload_count to 6 + start_offset.
     * -# Write 6 bytes MSB-first from node_id into payload[start_offset..start_offset+5].
     * -# Return payload_count.
     *
     * @verbatim
     * @param can_msg      Destination buffer.
     * @param node_id      48-bit @ref node_id_t to copy.
     * @param start_offset Byte position in payload to begin writing (0-2).
     * @endverbatim
     *
     * @return Bytes written (start_offset + 6), or 0 if start_offset is out of range.
     *
     * @see CanUtilities_extract_can_payload_as_node_id
     */
uint8_t CanUtilities_copy_node_id_to_payload(can_msg_t *can_msg, uint64_t node_id, uint8_t start_offset) {

    if (start_offset > 2) {

        return 0;

    }
    can_msg->payload_count = 6 + start_offset;

    for (int i = (start_offset + 5); i >= (0 + start_offset); i--) { // This is a count down...

        can_msg->payload[i] = node_id & 0xFF;
        node_id = node_id >> 8;

    }

    return can_msg->payload_count;

}

    /**
     * @brief Copies payload bytes from an @ref openlcb_msg_t into a @ref can_msg_t.
     *
     * @details Algorithm:
     * -# Return 0 if openlcb_msg payload_count is 0.
     * -# Copy from openlcb_msg->payload[openlcb_start_index] into can_msg->payload[can_start_index..7].
     * -# Stop when either buffer is exhausted.
     * -# Set can_msg->payload_count to can_start_index + bytes copied.
     * -# Return bytes copied.
     *
     * @verbatim
     * @param openlcb_msg         Source OpenLCB message.
     * @param can_msg             Destination CAN frame buffer.
     * @param openlcb_start_index Starting index in the OpenLCB payload.
     * @param can_start_index     Starting index in the CAN payload (0 or 2).
     * @endverbatim
     *
     * @return Number of bytes copied.
     *
     * @see CanUtilities_append_can_payload_to_openlcb_payload
     */
uint8_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint16_t openlcb_start_index, uint8_t can_start_index) {

    can_msg->payload_count = 0;
    uint8_t count = 0;

    if (openlcb_msg->payload_count == 0) {

        return 0;
    }

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

    /**
     * @brief Appends CAN payload bytes to the end of an @ref openlcb_msg_t payload.
     *
     * @details Algorithm:
     * -# Get OpenLCB buffer capacity from payload_type.
     * -# Copy can_msg->payload[can_start_index..payload_count-1] into openlcb_msg payload.
     * -# Stop when openlcb_msg is full.
     * -# Return number of bytes copied.
     *
     * @verbatim
     * @param openlcb_msg     Destination OpenLCB message.
     * @param can_msg         Source CAN frame.
     * @param can_start_index Starting index in the CAN payload.
     * @endverbatim
     *
     * @return Number of bytes copied.
     *
     * @warning Overflow is silently truncated when the OpenLCB buffer is full.
     *
     * @see CanUtilities_copy_openlcb_payload_to_can_payload
     */
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

    /**
     * @brief Copies a 64-bit value MSB-first into all 8 payload bytes and sets payload_count to 8.
     *
     * @verbatim
     * @param can_msg Destination buffer.
     * @param data    64-bit value (e.g. Event ID).
     * @endverbatim
     *
     * @return Always 8.
     */
uint8_t CanUtilities_copy_64_bit_to_can_message(can_msg_t *can_msg, uint64_t data) {

    for (int i = 7; i >= 0; i--) {

        can_msg->payload[i] = data & 0xFF;
        data = data >> 8;

    }

    can_msg->payload_count = 8;

    return can_msg->payload_count;

}

    /**
     * @brief Copies identifier and valid payload bytes from source to target @ref can_msg_t.
     *
     * @details Does not copy state flags. Target payload_count is set to match source.
     *
     * @verbatim
     * @param can_msg_source Source buffer.
     * @param can_msg_target Destination buffer.
     * @endverbatim
     *
     * @return Number of payload bytes copied.
     */
uint8_t CanUtilities_copy_can_message(can_msg_t *can_msg_source, can_msg_t *can_msg_target) {

    can_msg_target->identifier = can_msg_source->identifier;

    for (int i = 0; i < can_msg_source->payload_count; i++) {

        can_msg_target->payload[i] = can_msg_source->payload[i];

    }

    can_msg_target->payload_count = can_msg_source->payload_count;

    return can_msg_target->payload_count;

}

    /**
     * @brief Reads payload bytes 0-5 and returns them as a 48-bit @ref node_id_t (big-endian).
     *
     * @verbatim
     * @param can_msg Source buffer. Must have at least 6 valid payload bytes.
     * @endverbatim
     *
     * @return 48-bit @ref node_id_t.
     *
     * @see CanUtilities_copy_node_id_to_payload
     */
node_id_t CanUtilities_extract_can_payload_as_node_id(can_msg_t *can_msg) {

    return (
            ((node_id_t) can_msg->payload[0] << 40) |
            ((node_id_t) can_msg->payload[1] << 32) |
            ((node_id_t) can_msg->payload[2] << 24) |
            ((node_id_t) can_msg->payload[3] << 16) |
            ((node_id_t) can_msg->payload[4] << 8) |
            ((node_id_t) can_msg->payload[5]));

}

    /**
     * @brief Returns the 12-bit source alias from bits 0-11 of the CAN identifier.
     *
     * @verbatim
     * @param can_msg Source buffer.
     * @endverbatim
     *
     * @return Source alias (0x000-0xFFF).
     *
     * @see CanUtilities_extract_dest_alias_from_can_message
     */
uint16_t CanUtilities_extract_source_alias_from_can_identifier(can_msg_t *can_msg) {

    return (can_msg->identifier & 0x000000FFF);

}

    /**
     * @brief Returns the 12-bit destination alias from the appropriate location in a @ref can_msg_t.
     *
     * @details Algorithm:
     * -# For standard/stream frames: if MASK_CAN_DEST_ADDRESS_PRESENT is set, read alias from payload[0-1].
     * -# For datagram frames: extract alias from identifier bits 12-23.
     * -# Otherwise return 0 (global/broadcast frame).
     *
     * @verbatim
     * @param can_msg Source buffer.
     * @endverbatim
     *
     * @return Destination alias (0x001-0xFFF), or 0 if the frame is global.
     *
     * @see CanUtilities_extract_source_alias_from_can_identifier
     */
uint16_t CanUtilities_extract_dest_alias_from_can_message(can_msg_t *can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

    case OPENLCB_MESSAGE_STANDARD_FRAME_TYPE:
    case CAN_FRAME_TYPE_STREAM:

        if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT) {

            return ((uint16_t)((can_msg->payload[0] & 0x0F) << 8) | (uint16_t)(can_msg->payload[1]));
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

    /**
     * @brief Converts the CAN frame MTI bits to the corresponding 16-bit OpenLCB MTI.
     *
     * @details Algorithm:
     * -# For standard/stream frames: extract bits 12-23 as MTI; map PCER first/middle/last to MTI_PC_EVENT_REPORT_WITH_PAYLOAD.
     * -# For any datagram frame type: return MTI_DATAGRAM.
     * -# Otherwise return 0 (CAN control frames have no OpenLCB MTI).
     *
     * @verbatim
     * @param can_msg Source buffer.
     * @endverbatim
     *
     * @return 16-bit OpenLCB MTI, or 0 for control frames.
     *
     * @see CanUtilities_is_openlcb_message
     */
uint16_t CanUtilities_convert_can_mti_to_openlcb_mti(can_msg_t *can_msg) {

    uint16_t mti = 0;

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

    case OPENLCB_MESSAGE_STANDARD_FRAME_TYPE:
    case CAN_FRAME_TYPE_STREAM:

        mti = (can_msg->identifier >> 12) & 0x0FFF;

        switch (mti) {

        case CAN_MTI_PCER_WITH_PAYLOAD_FIRST:
        case CAN_MTI_PCER_WITH_PAYLOAD_MIDDLE:
        case CAN_MTI_PCER_WITH_PAYLOAD_LAST:

            mti = MTI_PC_EVENT_REPORT_WITH_PAYLOAD;
            break;

        }

        return mti;

    case CAN_FRAME_TYPE_DATAGRAM_ONLY:
    case CAN_FRAME_TYPE_DATAGRAM_FIRST:
    case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:
    case CAN_FRAME_TYPE_DATAGRAM_FINAL:

        return MTI_DATAGRAM;

    default:

        break;

    }

    return 0;
}

    /** @brief Counts NULL (0x00) bytes in a CAN payload up to payload_count. */
static uint8_t _count_nulls_in_can_payload(can_msg_t *can_msg) {

    uint8_t count = 0;

    for (int i = 0; i < can_msg->payload_count; i++) {

        if (can_msg->payload[i] == 0x00) {

            count++;

        }

    }

    return count;
}

    /**
     * @brief Counts NULL bytes in both an @ref openlcb_msg_t and a @ref can_msg_t payload combined.
     *
     * @details Used to detect SNIP message completion (exactly 6 NULLs required).
     *
     * @verbatim
     * @param openlcb_msg OpenLCB message with accumulated SNIP data.
     * @param can_msg     Current CAN frame being appended.
     * @endverbatim
     *
     * @return Total NULL byte count across both payloads.
     */
uint8_t CanUtilities_count_nulls_in_payloads(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg) {

    return _count_nulls_in_can_payload(can_msg) + OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg);

}

    /** @brief Returns true if the CAN frame carries an OpenLCB message (CAN_OPENLCB_MSG bit set). */
bool CanUtilities_is_openlcb_message(can_msg_t *can_msg) {

    return (can_msg->identifier & CAN_OPENLCB_MSG) == CAN_OPENLCB_MSG;

}
