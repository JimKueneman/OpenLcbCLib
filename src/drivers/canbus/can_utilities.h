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
 * @file can_utilities.h
 * @brief Utility functions for manipulating CAN frame buffers
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef ___DRIVERS_CANBUS_CAN_UTILITIES__
#define ___DRIVERS_CANBUS_CAN_UTILITIES__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Sets the fields of the pointer to a can_msg_t structure to zero/NULL
     * @param can_msg Pointer to a CAN frame buffer to clear
     * @return None
     */
    extern void CanUtilities_clear_can_message(can_msg_t *can_msg);


    /**
     * @brief Initalizes the fields of the pointer to the passed parameters
     * @param can_msg Pointer to a CAN frame buffer to fill in
     * @param identifier CAN MTI to load
     * @param payload_size How many bytes are valid in the payload
     * @param byte Payload Byte 1
     * @param byte Payload Byte 2
     * @param byte Payload Byte 3
     * @param byte Payload Byte 4
     * @param byte Payload Byte 5
     * @param byte Payload Byte 6
     * @param byte Payload Byte 7
     * @param byte Payload Byte 8
     * @return None
     */
    extern void CanUtilities_load_can_message(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8);


    /**
     * @brief Loads the payload and payload size fields of the passed CAN frame buffer
     * with the passed Node ID starting at the passed offset in the array.
     * @param can_msg Pointer to a CAN frame buffer to load the payload
     * @param node_id Node ID to load into the CAN buffers payload array
     * @param start_offset Where the Node ID starts in the 8 byte array
     * @return Number of bytes copied
     */
    extern uint8_t CanUtilities_copy_node_id_to_payload(can_msg_t *can_msg, uint64_t node_id, uint8_t start_offset);


    /**
     * @brief Loads the payload and payload size fields of the passed OpenLcb/LCC message buffer to the payload fields in
     * the passed CAN frame buffer at the offsets.
     * @param openlcb_msg Pointer to an OpenLcb/LCC buffer as the source payload
     * @param can_msg Pointer to a CAN frame buffer with the target payload
     * @param openlcb_start_index Where in the OpenLcb/LCC buffer payload to start copying from
     * @param can_start_index Where the CAN frame buffer to start copying to
     * @return Number of bytes copied
     */
    extern uint8_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint16_t openlcb_start_index, uint8_t can_start_index);


    /**
     * @brief Loads the payload and payload size fields of the passed CAN frame buffer with the payload fields in
     * the passed OpenLcb/LCC message buffer at the offsets.
     * @param openlcb_msg Pointer to an OpenLcb/LCC buffer as the target payload
     * @param can_msg Pointer to a CAN frame buffer with the source payload
     * @param openlcb_start_index Where in the OpenLcb/LCC buffer payload to start copying to
     * @param can_start_index Where the CAN frame buffer to start copying from
     * @return Number of bytes copied
     */
    extern uint8_t CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg, uint8_t can_start_index);


    /**
     * @brief Loads the payload and updates the payload size field of the passed CAN frame buffer
     * with the passed 64 bit value.
     * @param can_msg Pointer to a CAN frame buffer to load the payload
     * @param node_id 64 bit value to load into the CAN buffers payload array
     * @return Number of bytes copied
     */
    extern uint8_t CanUtilities_copy_64_bit_to_can_message(can_msg_t *can_msg, uint64_t data);


    /**
     * @brief Copies the payload and the payload size fields from the source passed CAN frame buffer
     * to the target payload and the payload size fields.
     * @param can_msg_source Pointer to a CAN frame buffer with the source payload fields
     * @param can_msg_target Pointer to a CAN frame buffer with the target payload fields
     * @return Number of bytes copied
     */
    extern uint8_t CanUtilities_copy_can_message(can_msg_t *can_msg_source, can_msg_t *can_msg_target);


    /**
     * @brief Extracts the first 6 bytes in the passed CAN frame buffer's payload as a Node ID
     * @param can_msg_source Pointer to a CAN frame buffer with the source payload fields
     * @return The Node ID extracted
     */
    extern node_id_t CanUtilities_extract_can_payload_as_node_id(can_msg_t *can_msg);


    /**
     * @brief Extracts the last 3 nibbles from the identifier field in the passed CAN frame buffer
     * @param can_msg_source Pointer to a CAN frame buffer with the identifier
     * @return The source Alias extracted from the CAN identifier
     */
    extern uint16_t CanUtilities_extract_source_alias_from_can_identifier(can_msg_t *can_msg);


    /**
     * @brief Extracts any destination Alias ID from the CAN frame buffer
     * @note This function examines the frame and pulls out the destination Alias based on what type of message it is.  If the message does not carry a destination Alias it return 0
     * @param can_msg_source Pointer to a CAN frame buffer with the identifier
     * @return The destination Alias extracted from the CAN identifier or 0 if there is no destination carried in this frame
     */
    extern uint16_t CanUtilities_extract_dest_alias_from_can_message(can_msg_t *can_msg);


     /**
     * @brief Converts a CAN MTI to an OpenLcb/LCC MTI from the passed CAN frame identifier
     * @param can_msg_source Pointer to a CAN frame buffer with the identifier
     * @return OpenLcb MTI that was mapped to by the CAN MTI in the identifier
     */
    extern uint16_t CanUtilities_convert_can_mti_to_openlcb_mti(can_msg_t *can_msg);


    /**
     * @brief Runs through the payloads of the passed buffers to count how many NULLs are in both combined
     * @param openlcb_msg Pointer to a OpenLcb/LCC buffer with a payload
     * @param can_msg_source Pointer to a CAN frame buffer with a payload
     * @return Number of NULL bytes in both buffer payloads within the valid space defined by the payload sizes
     */
    extern uint8_t CanUtilities_count_nulls_in_payloads(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg);


    /**
     * @brief Tests if the passed CAN frame buffer is either a complete or partial
     * OpenLcb/LCC message (as opposed to a CAN only message frame).
     * @param can_msg_source Pointer to a CAN frame buffer to test
     * @return True if the message is either a complete or partial OpenLcb/LCC message, else False
     */
    extern bool CanUtilities_is_openlcb_message(can_msg_t *can_msg);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ___DRIVERS_COMMON_CAN_UTILITIES__ */
