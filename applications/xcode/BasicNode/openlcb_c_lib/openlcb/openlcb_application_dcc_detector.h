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
 * @file openlcb_application_dcc_detector.h
 * @brief Encode/decode helpers for DCC Detection Protocol Event IDs.
 *
 * @details Provides constants and utility functions to build and parse the
 * 8-byte Event IDs defined by the OpenLCB DCC Detection Protocol Working Note.
 * Each Event ID packs a 6-byte detector unique ID, 2 direction/occupancy bits,
 * and a 14-bit DCC address into a standard @ref event_id_t.
 *
 * @author Jim Kueneman
 * @date 04 Apr 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_APPLICATION_DCC_DETECTOR__
#define __OPENLCB_OPENLCB_APPLICATION_DCC_DETECTOR__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef OPENLCB_COMPILE_DCC_DETECTOR

// =============================================================================
// Direction / occupancy status (upper 2 bits of the 16-bit tail)
// =============================================================================

/** @brief Unoccupied (exit) -- decoder has left the monitored section. */
#define DCC_DETECTOR_UNOCCUPIED          0x00

/** @brief Occupied, forward direction (entry). */
#define DCC_DETECTOR_OCCUPIED_FORWARD    0x01

/** @brief Occupied, reverse direction (entry). */
#define DCC_DETECTOR_OCCUPIED_REVERSE    0x02

/** @brief Occupied, direction unknown (entry). */
#define DCC_DETECTOR_OCCUPIED_UNKNOWN    0x03

// =============================================================================
// DCC address type prefixes (upper 6 bits of the 14-bit address field)
// =============================================================================

/** @brief High-byte prefix indicating a DCC short address (bits [13:8] = 0x38). */
#define DCC_DETECTOR_SHORT_ADDRESS_PREFIX    0x38

/** @brief High-byte prefix indicating a DCC consist address (bits [13:8] = 0x39). */
#define DCC_DETECTOR_CONSIST_ADDRESS_PREFIX  0x39

// =============================================================================
// Special sentinel values
// =============================================================================

/** @brief 14-bit value representing "track is empty" (short address 0). */
#define DCC_DETECTOR_TRACK_EMPTY    0x3800

// =============================================================================
// Bit-field geometry
// =============================================================================

/** @brief Number of bits in the DCC address field. */
#define DCC_DETECTOR_ADDRESS_BITS   14

/** @brief Mask for the 14-bit DCC address within the 16-bit tail. */
#define DCC_DETECTOR_ADDRESS_MASK   0x3FFF

/** @brief Mask for the 2 direction/occupancy bits within the 16-bit tail. */
#define DCC_DETECTOR_DIRECTION_MASK 0xC000

/** @brief Bit position of the direction/occupancy field in the 16-bit tail. */
#define DCC_DETECTOR_DIRECTION_SHIFT 14

// =============================================================================
// Types
// =============================================================================

    /** @brief Direction and occupancy status reported by a DCC detector. */
typedef enum {

    dcc_detector_unoccupied = DCC_DETECTOR_UNOCCUPIED,
    dcc_detector_occupied_forward = DCC_DETECTOR_OCCUPIED_FORWARD,
    dcc_detector_occupied_reverse = DCC_DETECTOR_OCCUPIED_REVERSE,
    dcc_detector_occupied_unknown = DCC_DETECTOR_OCCUPIED_UNKNOWN

} dcc_detector_direction_enum;

    /** @brief Category of the DCC address encoded in a detector Event ID. */
typedef enum {

    dcc_detector_address_long,
    dcc_detector_address_short,
    dcc_detector_address_consist,
    dcc_detector_address_track_empty

} dcc_detector_address_type_enum;

// =============================================================================
// Public functions
// =============================================================================

        /**
         * @brief Builds a DCC detector Event ID from its components.
         *
         * @details Packs a 6-byte detector node ID, a 2-bit direction/occupancy
         * status, and a 14-bit raw DCC address into a single @ref event_id_t.
         * The caller is responsible for encoding the DCC address with the correct
         * prefix (0x38xx for short, 0x39xx for consist, or raw long address).
         *
         * @param detector_node_id  48-bit @ref node_id_t of the detector.
         * @param direction         @ref dcc_detector_direction_enum occupancy status.
         * @param raw_address_14    14-bit DCC address field (already includes type prefix).
         *
         * @return Assembled @ref event_id_t.
         */
    extern event_id_t OpenLcbApplicationDccDetector_encode_event_id(node_id_t detector_node_id, dcc_detector_direction_enum direction, uint16_t raw_address_14);

        /**
         * @brief Builds a 14-bit raw address field for a DCC short address.
         *
         * @details Combines the short-address prefix (0x38) with the 8-bit
         * address to produce the 14-bit value expected by
         * OpenLcbApplicationDccDetector_encode_event_id().
         *
         * @param short_address  DCC short address (0-127).
         *
         * @return 14-bit raw address field with short-address prefix.
         */
    extern uint16_t OpenLcbApplicationDccDetector_make_short_address(uint8_t short_address);

        /**
         * @brief Builds a 14-bit raw address field for a DCC consist address.
         *
         * @details Combines the consist-address prefix (0x39) with the 8-bit
         * address to produce the 14-bit value expected by
         * OpenLcbApplicationDccDetector_encode_event_id().
         *
         * @param consist_address  DCC consist address (0-127).
         *
         * @return 14-bit raw address field with consist-address prefix.
         */
    extern uint16_t OpenLcbApplicationDccDetector_make_consist_address(uint8_t consist_address);

        /**
         * @brief Extracts the direction/occupancy status from a detector Event ID.
         *
         * @details Returns the 2-bit direction/occupancy field from the upper
         * bits of the 16-bit tail of the Event ID.
         *
         * @param event_id  @ref event_id_t to decode.
         *
         * @return @ref dcc_detector_direction_enum value.
         */
    extern dcc_detector_direction_enum OpenLcbApplicationDccDetector_extract_direction(event_id_t event_id);

        /**
         * @brief Determines the DCC address category from a detector Event ID.
         *
         * @details Inspects the upper 6 bits of the 14-bit address field to
         * classify the address as short, consist, track-empty, or long.
         *
         * @param event_id  @ref event_id_t to decode.
         *
         * @return @ref dcc_detector_address_type_enum category.
         */
    extern dcc_detector_address_type_enum OpenLcbApplicationDccDetector_extract_address_type(event_id_t event_id);

        /**
         * @brief Extracts the raw 14-bit DCC address field from a detector Event ID.
         *
         * @details Returns the full 14-bit address including any type prefix.
         * Use OpenLcbApplicationDccDetector_extract_address_type() to determine how to
         * interpret the value.
         *
         * @param event_id  @ref event_id_t to decode.
         *
         * @return 14-bit raw DCC address field.
         */
    extern uint16_t OpenLcbApplicationDccDetector_extract_raw_address(event_id_t event_id);

        /**
         * @brief Extracts the usable DCC address from a detector Event ID.
         *
         * @details For short and consist addresses, strips the type prefix and
         * returns the 8-bit address.  For long addresses, returns the full
         * 14-bit value.  For track-empty, returns 0.
         *
         * @param event_id  @ref event_id_t to decode.
         *
         * @return DCC address value (meaning depends on address type).
         */
    extern uint16_t OpenLcbApplicationDccDetector_extract_dcc_address(event_id_t event_id);

        /**
         * @brief Extracts the 48-bit detector node ID from a detector Event ID.
         *
         * @details Returns the upper 6 bytes of the Event ID as a @ref node_id_t.
         *
         * @param event_id  @ref event_id_t to decode.
         *
         * @return 48-bit @ref node_id_t of the detector.
         */
    extern node_id_t OpenLcbApplicationDccDetector_extract_detector_id(event_id_t event_id);

        /**
         * @brief Tests whether a detector Event ID represents "track is empty".
         *
         * @details Returns true when the 14-bit address field equals
         * DCC_DETECTOR_TRACK_EMPTY (0x3800), regardless of the direction bits.
         *
         * @param event_id  @ref event_id_t to test.
         *
         * @return true if the Event ID encodes the track-empty sentinel.
         */
    extern bool OpenLcbApplicationDccDetector_is_track_empty(event_id_t event_id);

#endif /* OPENLCB_COMPILE_DCC_DETECTOR */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_APPLICATION_DCC_DETECTOR__ */
