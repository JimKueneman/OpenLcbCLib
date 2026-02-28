/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file can_utilities.h
 * @brief Utility functions for manipulating @ref can_msg_t frame buffers.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_UTILITIES__
#define __DRIVERS_CANBUS_CAN_UTILITIES__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /** @brief Clears identifier, payload_count, and all payload bytes in a @ref can_msg_t. */
    extern void CanUtilities_clear_can_message(can_msg_t *can_msg);

        /**
         * @brief Loads identifier, payload size, and all 8 data bytes into a @ref can_msg_t.
         *
         * @details All 8 byte parameters must be provided even if payload_size is less than 8.
         *
         * @param can_msg         Destination buffer.
         * @param identifier      29-bit CAN extended identifier.
         * @param payload_size    Number of valid payload bytes (0-8).
         * @param byte1           Payload byte 1.
         * @param byte2           Payload byte 2.
         * @param byte3           Payload byte 3.
         * @param byte4           Payload byte 4.
         * @param byte5           Payload byte 5.
         * @param byte6           Payload byte 6.
         * @param byte7           Payload byte 7.
         * @param byte8           Payload byte 8.
         *
         * @warning NULL can_msg causes undefined behavior.
         */
    extern void CanUtilities_load_can_message(
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
            uint8_t byte8);

        /**
         * @brief Copies a 48-bit @ref node_id_t into 6 payload bytes starting at start_offset.
         *
         * @details Updates payload_count to start_offset + 6. Valid start_offset range: 0-2
         * (must fit 6 bytes into the 8-byte payload).
         *
         * @param can_msg       Destination buffer.
         * @param node_id       48-bit @ref node_id_t to copy.
         * @param start_offset  Byte position in payload to begin writing (0-2).
         *
         * @return Bytes written (start_offset + 6), or 0 if start_offset is out of range.
         *
         * @see CanUtilities_extract_can_payload_as_node_id
         */
    extern uint8_t CanUtilities_copy_node_id_to_payload(can_msg_t *can_msg, uint64_t node_id, uint8_t start_offset);

        /**
         * @brief Copies payload bytes from an @ref openlcb_msg_t into a @ref can_msg_t.
         *
         * @details Used to fragment a large OpenLCB payload across multiple CAN frames.
         * Updates can_msg payload_count to reflect total bytes written.
         *
         * @param openlcb_msg         Source OpenLCB message.
         * @param can_msg             Destination CAN frame buffer.
         * @param openlcb_start_index Starting index in the OpenLCB payload.
         * @param can_start_index     Starting index in the CAN payload (0 or 2).
         *
         * @return Number of bytes copied.
         *
         * @see CanUtilities_append_can_payload_to_openlcb_payload
         */
    extern uint8_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint16_t openlcb_start_index, uint8_t can_start_index);

        /**
         * @brief Appends CAN payload bytes to the end of an @ref openlcb_msg_t payload.
         *
         * @details Used to reassemble multi-frame CAN messages. Stops when the OpenLCB
         * buffer capacity is reached. Updates openlcb_msg payload_count.
         *
         * @param openlcb_msg     Destination OpenLCB message.
         * @param can_msg         Source CAN frame.
         * @param can_start_index Starting index in the CAN payload.
         *
         * @return Number of bytes copied.
         *
         * @warning Buffer overflow is silently truncated — ensure buffer type fits expected payload.
         *
         * @see CanUtilities_copy_openlcb_payload_to_can_payload
         */
    extern uint8_t CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint8_t can_start_index);

        /**
         * @brief Copies a 64-bit value into all 8 payload bytes of a @ref can_msg_t (big-endian).
         *
         * @details Always sets payload_count to 8.
         *
         * @param can_msg  Destination buffer.
         * @param data     64-bit value (e.g. Event ID).
         *
         * @return Always 8.
         */
    extern uint8_t CanUtilities_copy_64_bit_to_can_message(can_msg_t *can_msg, uint64_t data);

        /**
         * @brief Copies identifier and valid payload bytes from source to target @ref can_msg_t.
         *
         * @details Does not copy state flags (allocated). Target payload_count is set to
         * match source.
         *
         * @param can_msg_source  Source buffer.
         * @param can_msg_target  Destination buffer.
         *
         * @return Number of payload bytes copied.
         */
    extern uint8_t CanUtilities_copy_can_message(can_msg_t *can_msg_source, can_msg_t *can_msg_target);

        /**
         * @brief Reads payload bytes 0-5 and returns them as a 48-bit @ref node_id_t (big-endian).
         *
         * @param can_msg  Source buffer. Must have at least 6 valid payload bytes.
         *
         * @return 48-bit @ref node_id_t.
         *
         * @see CanUtilities_copy_node_id_to_payload
         */
    extern node_id_t CanUtilities_extract_can_payload_as_node_id(can_msg_t *can_msg);

        /**
         * @brief Returns the 12-bit source alias from bits 0-11 of the CAN identifier.
         *
         * @param can_msg  Source buffer.
         *
         * @return Source alias (0x000-0xFFF). Valid range is 0x001-0xFFF.
         *
         * @see CanUtilities_extract_dest_alias_from_can_message
         */
    extern uint16_t CanUtilities_extract_source_alias_from_can_identifier(can_msg_t *can_msg);

        /**
         * @brief Returns the 12-bit destination alias from the appropriate location in a @ref can_msg_t.
         *
         * @details Addressed messages carry the destination in payload bytes 0-1; datagrams
         * carry it in the identifier. Returns 0 for global (broadcast) frames.
         *
         * @param can_msg  Source buffer.
         *
         * @return Destination alias (0x001-0xFFF), or 0 if the frame is global.
         *
         * @see CanUtilities_extract_source_alias_from_can_identifier
         */
    extern uint16_t CanUtilities_extract_dest_alias_from_can_message(can_msg_t *can_msg);

        /**
         * @brief Converts the CAN frame MTI bits to the corresponding 16-bit OpenLCB MTI.
         *
         * @details Handles multi-frame PCER and datagram special cases. Returns 0 for
         * CAN control frames (CID, RID, AMD, etc.) which have no OpenLCB MTI.
         *
         * @param can_msg  Source buffer.
         *
         * @return 16-bit OpenLCB MTI, or 0 for unrecognised or control frames.
         *
         * @see CanUtilities_is_openlcb_message
         */
    extern uint16_t CanUtilities_convert_can_mti_to_openlcb_mti(can_msg_t *can_msg);

        /**
         * @brief Counts NULL (0x00) bytes across both an @ref openlcb_msg_t and a @ref can_msg_t payload.
         *
         * @details Used to detect SNIP message completion (exactly 6 NULLs required).
         *
         * @param openlcb_msg  OpenLCB message with accumulated SNIP data.
         * @param can_msg      Current CAN frame being appended.
         *
         * @return Total NULL byte count across both payloads.
         */
    extern uint8_t CanUtilities_count_nulls_in_payloads(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg);

        /**
         * @brief Returns true if the CAN frame carries an OpenLCB message (bit 27 set).
         *
         * @details CAN control frames (CID, RID, AMD, AME, AMR) return false.
         *
         * @param can_msg  Source buffer.
         *
         * @return true for OpenLCB message frames, false for CAN-only control frames.
         *
         * @see CanUtilities_convert_can_mti_to_openlcb_mti
         */
    extern bool CanUtilities_is_openlcb_message(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_UTILITIES__ */
