/** \copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * @file openlcb_application_dcc_detector.c
 * @brief Encode/decode helpers for DCC Detection Protocol Event IDs.
 *
 * @details Implements the packing and unpacking of detector Event IDs as
 * specified in the OpenLCB DCC Detection Protocol Working Note.  Each Event ID
 * is an 8-byte value: 6 bytes of detector unique ID, 2 direction/occupancy
 * bits, and 14 bits of DCC address.
 *
 * @author Jim Kueneman
 * @date 04 Apr 2026
 */

#include "openlcb_application_dcc_detector.h"

#ifdef OPENLCB_COMPILE_DCC_DETECTOR


    /**
     * @brief Builds a DCC detector Event ID from its components.
     *
     * @details Algorithm:
     * -# Shift the 48-bit detector node ID into the upper 6 bytes of a 64-bit value
     * -# Shift the 2-bit direction field into bits [15:14]
     * -# Mask the 14-bit raw address into bits [13:0]
     * -# OR all three fields together
     *
     * @verbatim
     * @param detector_node_id 48-bit node ID of the detector
     * @param direction dcc_detector_direction_enum occupancy status
     * @param raw_address_14 14-bit DCC address field (already includes type prefix)
     * @endverbatim
     *
     * @return Assembled event_id_t
     */
event_id_t OpenLcbApplicationDccDetector_encode_event_id(node_id_t detector_node_id, dcc_detector_direction_enum direction, uint16_t raw_address_14) {

    uint64_t result = (detector_node_id & 0xFFFFFFFFFFFFULL) << 16;

    result |= (uint64_t) ((uint16_t) direction << DCC_DETECTOR_DIRECTION_SHIFT);
    result |= (uint64_t) (raw_address_14 & DCC_DETECTOR_ADDRESS_MASK);

    return (event_id_t) result;

}


    /**
     * @brief Builds a 14-bit raw address field for a DCC short address.
     *
     * @details Algorithm:
     * -# Shift the short-address prefix (0x38) into the upper 6 bits
     * -# OR with the 8-bit address in the lower 8 bits
     *
     * @verbatim
     * @param short_address DCC short address (0-127)
     * @endverbatim
     *
     * @return 14-bit raw address field with short-address prefix
     */
uint16_t OpenLcbApplicationDccDetector_make_short_address(uint8_t short_address) {

    return (uint16_t) ((DCC_DETECTOR_SHORT_ADDRESS_PREFIX << 8) | short_address);

}


    /**
     * @brief Builds a 14-bit raw address field for a DCC consist address.
     *
     * @details Algorithm:
     * -# Shift the consist-address prefix (0x39) into the upper 6 bits
     * -# OR with the 8-bit address in the lower 8 bits
     *
     * @verbatim
     * @param consist_address DCC consist address (0-127)
     * @endverbatim
     *
     * @return 14-bit raw address field with consist-address prefix
     */
uint16_t OpenLcbApplicationDccDetector_make_consist_address(uint8_t consist_address) {

    return (uint16_t) ((DCC_DETECTOR_CONSIST_ADDRESS_PREFIX << 8) | consist_address);

}


    /**
     * @brief Extracts the direction/occupancy status from a detector Event ID.
     *
     * @details Algorithm:
     * -# Mask the lower 16 bits of the Event ID
     * -# Shift right by 14 to isolate the 2-bit direction field
     *
     * @verbatim
     * @param event_id event_id_t to decode
     * @endverbatim
     *
     * @return dcc_detector_direction_enum value
     */
dcc_detector_direction_enum OpenLcbApplicationDccDetector_extract_direction(event_id_t event_id) {

    uint16_t tail = (uint16_t) (event_id & 0xFFFF);

    return (dcc_detector_direction_enum) ((tail & DCC_DETECTOR_DIRECTION_MASK) >> DCC_DETECTOR_DIRECTION_SHIFT);

}


    /**
     * @brief Determines the DCC address category from a detector Event ID.
     *
     * @details Algorithm:
     * -# Extract the 14-bit raw address from the lower bits
     * -# Check for the track-empty sentinel (0x3800)
     * -# Check the upper 6 bits for short-address prefix (0x38)
     * -# Check the upper 6 bits for consist-address prefix (0x39)
     * -# Otherwise classify as long address
     *
     * @verbatim
     * @param event_id event_id_t to decode
     * @endverbatim
     *
     * @return dcc_detector_address_type_enum category
     */
dcc_detector_address_type_enum OpenLcbApplicationDccDetector_extract_address_type(event_id_t event_id) {

    uint16_t raw = (uint16_t) (event_id & DCC_DETECTOR_ADDRESS_MASK);

    if (raw == DCC_DETECTOR_TRACK_EMPTY) {

        return dcc_detector_address_track_empty;

    }

    uint8_t high_byte = (uint8_t) (raw >> 8);

    if (high_byte == DCC_DETECTOR_SHORT_ADDRESS_PREFIX) {

        return dcc_detector_address_short;

    }

    if (high_byte == DCC_DETECTOR_CONSIST_ADDRESS_PREFIX) {

        return dcc_detector_address_consist;

    }

    return dcc_detector_address_long;

}


    /**
     * @brief Extracts the raw 14-bit DCC address field from a detector Event ID.
     *
     * @details Algorithm:
     * -# Mask the lower 14 bits of the Event ID
     *
     * @verbatim
     * @param event_id event_id_t to decode
     * @endverbatim
     *
     * @return 14-bit raw DCC address field
     */
uint16_t OpenLcbApplicationDccDetector_extract_raw_address(event_id_t event_id) {

    return (uint16_t) (event_id & DCC_DETECTOR_ADDRESS_MASK);

}


    /**
     * @brief Extracts the usable DCC address from a detector Event ID.
     *
     * @details Algorithm:
     * -# Extract address type to determine decoding
     * -# For short and consist addresses, mask off the prefix to get the 8-bit address
     * -# For track-empty, return 0
     * -# For long addresses, return the full 14-bit value
     *
     * @verbatim
     * @param event_id event_id_t to decode
     * @endverbatim
     *
     * @return DCC address value (meaning depends on address type)
     */
uint16_t OpenLcbApplicationDccDetector_extract_dcc_address(event_id_t event_id) {

    uint16_t raw = (uint16_t) (event_id & DCC_DETECTOR_ADDRESS_MASK);
    dcc_detector_address_type_enum addr_type = OpenLcbApplicationDccDetector_extract_address_type(event_id);

    switch (addr_type) {

        case dcc_detector_address_track_empty:

            return 0;

        case dcc_detector_address_short:
        case dcc_detector_address_consist:

            return raw & 0x00FF;

        case dcc_detector_address_long:

            return raw;

    }

    return raw;

}


    /**
     * @brief Extracts the 48-bit detector node ID from a detector Event ID.
     *
     * @details Algorithm:
     * -# Shift the Event ID right by 16 to isolate the upper 6 bytes
     * -# Mask to 48 bits
     *
     * @verbatim
     * @param event_id event_id_t to decode
     * @endverbatim
     *
     * @return 48-bit node_id_t of the detector
     */
node_id_t OpenLcbApplicationDccDetector_extract_detector_id(event_id_t event_id) {

    return (node_id_t) ((event_id >> 16) & 0xFFFFFFFFFFFFULL);

}


    /**
     * @brief Tests whether a detector Event ID represents "track is empty".
     *
     * @details Algorithm:
     * -# Mask the lower 14 bits of the Event ID
     * -# Compare against the track-empty sentinel (0x3800)
     *
     * @verbatim
     * @param event_id event_id_t to test
     * @endverbatim
     *
     * @return true if the Event ID encodes the track-empty sentinel
     */
bool OpenLcbApplicationDccDetector_is_track_empty(event_id_t event_id) {

    return (event_id & DCC_DETECTOR_ADDRESS_MASK) == DCC_DETECTOR_TRACK_EMPTY;

}


#endif /* OPENLCB_COMPILE_DCC_DETECTOR */
