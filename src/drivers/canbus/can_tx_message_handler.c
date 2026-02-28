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
 * @file can_tx_message_handler.c
 * @brief Implementation of message handlers for CAN transmit operations.
 *
 * @details Converts OpenLCB messages to CAN frames with proper fragmentation and
 * framing-bit encoding per the OpenLCB CAN Frame Transfer Standard.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "can_tx_message_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_utilities.h"

/** @brief Pre-built upper bits for a datagram-only (single-frame) CAN identifier. */
static const uint32_t _OPENLCB_MESSAGE_DATAGRAM_ONLY = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_ONLY;

/** @brief Pre-built upper bits for the first frame of a multi-frame datagram. */
static const uint32_t _OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FIRST;

/** @brief Pre-built upper bits for a middle frame of a multi-frame datagram. */
static const uint32_t _OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_MIDDLE;

/** @brief Pre-built upper bits for the last frame of a multi-frame datagram. */
static const uint32_t _OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FINAL;

/** @brief Pre-built upper bits for a standard OpenLCB message CAN identifier. */
static const uint32_t _OPENLCB_MESSAGE_STANDARD_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | OPENLCB_MESSAGE_STANDARD_FRAME_TYPE;

static interface_can_tx_message_handler_t *_interface;

    /** @brief Stores the dependency-injection interface pointer. */
void CanTxMessageHandler_initialize(const interface_can_tx_message_handler_t *interface_can_tx_message_handler) {

    _interface = (interface_can_tx_message_handler_t*) interface_can_tx_message_handler;

}

// ---- CAN identifier builders ----

    /** @brief Builds the 29-bit identifier for a datagram-only (single-frame) CAN frame. */
static uint32_t _construct_identifier_datagram_only_frame(openlcb_msg_t* openlcb_msg) {

    return (_OPENLCB_MESSAGE_DATAGRAM_ONLY | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

    /** @brief Builds the 29-bit identifier for the first frame of a multi-frame datagram. */
static uint32_t _construct_identifier_datagram_first_frame(openlcb_msg_t* openlcb_msg) {

    return (_OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

    /** @brief Builds the 29-bit identifier for a middle frame of a multi-frame datagram. */
static uint32_t _construct_identifier_datagram_middle_frame(openlcb_msg_t* openlcb_msg) {

    return (_OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

    /** @brief Builds the 29-bit identifier for the last frame of a multi-frame datagram. */
static uint32_t _construct_identifier_datagram_last_frame(openlcb_msg_t* openlcb_msg) {

    return (_OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

    /** @brief Builds the 29-bit identifier for a global (unaddressed) OpenLCB standard frame. */
static uint32_t _construct_unaddressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (_OPENLCB_MESSAGE_STANDARD_FRAME | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

    /** @brief Builds the 29-bit identifier for an addressed OpenLCB standard frame (dest alias goes in payload). */
static uint32_t _construct_addressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (_OPENLCB_MESSAGE_STANDARD_FRAME | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

// ---- Low-level transmit helper ----

    /**
     * @brief Calls the hardware transmit function and invokes the optional on_transmit callback.
     *
     * @verbatim
     * @param can_msg Fully-constructed CAN frame to send.
     * @endverbatim
     *
     * @return true on success, false on hardware error.
     */
static bool _transmit_can_frame(can_msg_t* can_msg) {

    bool result = _interface->transmit_can_frame(can_msg);

    if (_interface->on_transmit && result) {

        _interface->on_transmit(can_msg);

    }

    return result;

}

// ---- Datagram frame type helpers ----

    /** @brief Sets datagram-only identifier and transmits. */
static bool _datagram_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identifier_datagram_only_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

    /** @brief Sets datagram-first identifier and transmits. */
static bool _datagram_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identifier_datagram_first_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

    /** @brief Sets datagram-middle identifier and transmits. */
static bool _datagram_middle_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identifier_datagram_middle_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

    /** @brief Sets datagram-last identifier and transmits. */
static bool _datagram_last_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identifier_datagram_last_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

// ---- Addressed frame framing-bit helpers ----

    /** @brief Sets MULTIFRAME_ONLY framing bits and transmits. */
static bool _addressed_message_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_ONLY);
    return _transmit_can_frame(can_msg);

}

    /** @brief Sets MULTIFRAME_FIRST framing bits and transmits. */
static bool _addressed_message_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FIRST);
    return _transmit_can_frame(can_msg);

}

    /** @brief Sets MULTIFRAME_MIDDLE framing bits and transmits. */
static bool _addressed_message_middle_frame(can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_MIDDLE);
    return _transmit_can_frame(can_msg);

}

    /** @brief Sets MULTIFRAME_FINAL framing bits and transmits. */
static bool _addressed_message_last_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FINAL);
    return _transmit_can_frame(can_msg);

}

    /** @brief Writes the 12-bit destination alias into payload bytes 0 (high nibble) and 1 (low byte). */
static void _load_destination_address_in_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    can_msg->payload[0] = (openlcb_msg->dest_alias >> 8) & 0xFF; // Setup the first two CAN data bytes with the destination address
    can_msg->payload[1] = openlcb_msg->dest_alias & 0xFF;

}

// ---- Public API ----

    /**
     * @brief Transmits one datagram CAN frame, selecting ONLY/FIRST/MIDDLE/LAST automatically.
     *
     * @details Algorithm:
     * -# Copy up to 8 payload bytes from openlcb_msg into can_msg_worker starting at *openlcb_start_index.
     * -# Choose frame type: ONLY (total <= 8), FIRST (index < 8), MIDDLE (more data remains), LAST.
     * -# Transmit. On success, advance *openlcb_start_index by bytes copied.
     * -# Return transmission result.
     *
     * @verbatim
     * @param openlcb_msg          Source datagram message (max 72 bytes).
     * @param can_msg_worker       Scratch CAN frame buffer.
     * @param openlcb_start_index  Current payload position; updated on success.
     * @endverbatim
     *
     * @return true if transmitted, false on hardware failure.
     *
     * @warning Index is unchanged on failure — caller must retry.
     */
bool CanTxMessageHandler_datagram_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    bool result = false;
    uint8_t len_msg_frame = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, *openlcb_start_index, OFFSET_CAN_WITHOUT_DEST_ADDRESS);

    if (openlcb_msg->payload_count <= LEN_CAN_BYTE_ARRAY) {

        result = _datagram_only_frame(openlcb_msg, can_msg_worker);

    } else if (*openlcb_start_index < LEN_CAN_BYTE_ARRAY) {

        result = _datagram_first_frame(openlcb_msg, can_msg_worker);

    } else if (*openlcb_start_index + len_msg_frame < openlcb_msg->payload_count) {

        result = _datagram_middle_frame(openlcb_msg, can_msg_worker);

    } else {

        result = _datagram_last_frame(openlcb_msg, can_msg_worker);

    }

    if (result) {

        *openlcb_start_index = *openlcb_start_index + len_msg_frame;

    }

    return result;

}

    /**
     * @brief Transmits one unaddressed (broadcast) OpenLCB CAN frame.
     *
     * @details Algorithm:
     * -# If payload fits in one frame (<= 8 bytes): copy payload, build identifier, transmit.
     * -# On success, advance *openlcb_start_index. Multi-frame unaddressed not yet implemented.
     * -# Return transmission result.
     *
     * @verbatim
     * @param openlcb_msg          Source message (no dest_alias needed).
     * @param can_msg_worker       Scratch CAN frame buffer.
     * @param openlcb_start_index  Current payload position; updated on success.
     * @endverbatim
     *
     * @return true if transmitted, false on hardware failure.
     *
     * @warning Multi-frame unaddressed messages are not yet implemented.
     */
bool CanTxMessageHandler_unaddressed_msg_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    bool result = false;

    if (openlcb_msg->payload_count <= LEN_CAN_BYTE_ARRAY) { // single frame

        uint8_t len_msg_frame = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, *openlcb_start_index, OFFSET_CAN_WITHOUT_DEST_ADDRESS);
        can_msg_worker->identifier = _construct_unaddressed_message_identifier(openlcb_msg);

        result = _transmit_can_frame(can_msg_worker);

        if (result) {

            *openlcb_start_index = *openlcb_start_index + len_msg_frame;

        }

    } else { // multi frame

        // TODO: Is there such a thing as a unaddressed multi frame?

    }

    return result;

}

    /**
     * @brief Transmits one addressed OpenLCB CAN frame, selecting ONLY/FIRST/MIDDLE/LAST automatically.
     *
     * @details Algorithm:
     * -# Write destination alias into payload bytes 0-1.
     * -# Build the standard-frame identifier (MTI + source alias).
     * -# Copy up to 6 payload bytes starting at *openlcb_start_index into bytes 2-7.
     * -# Choose framing bits: ONLY (<= 6 bytes total), FIRST (index < 6), MIDDLE, or LAST.
     * -# Transmit. On success, advance *openlcb_start_index by bytes copied.
     * -# Return transmission result.
     *
     * @verbatim
     * @param openlcb_msg          Source message (dest_alias must be valid).
     * @param can_msg_worker       Scratch CAN frame buffer.
     * @param openlcb_start_index  Current payload position; updated on success.
     * @endverbatim
     *
     * @return true if transmitted, false on hardware failure.
     *
     * @warning Index is unchanged on failure — caller must retry.
     */
bool CanTxMessageHandler_addressed_msg_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    _load_destination_address_in_payload(openlcb_msg, can_msg_worker);


    can_msg_worker->identifier = _construct_addressed_message_identifier(openlcb_msg);
    uint8_t len_msg_frame = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, *openlcb_start_index, OFFSET_CAN_WITH_DEST_ADDRESS);
    bool result = false;

    if (openlcb_msg->payload_count <= 6) {// Account for 2 bytes used for dest alias

        result = _addressed_message_only_frame(openlcb_msg, can_msg_worker);

    } else if (*openlcb_start_index < 6) { // Account for 2 bytes used for dest alias

        result = _addressed_message_first_frame(openlcb_msg, can_msg_worker);

    } else if ((*openlcb_start_index + len_msg_frame) < openlcb_msg->payload_count) {

        result = _addressed_message_middle_frame(can_msg_worker);

    } else {

        result = _addressed_message_last_frame(openlcb_msg, can_msg_worker);

    }

    if (result) {

        *openlcb_start_index = *openlcb_start_index + len_msg_frame;

    }

    return result;

}

    /** @brief Stream transmit handler — not yet implemented, returns true as placeholder. */
bool CanTxMessageHandler_stream_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    // ToDo: implement streams
    return true;
}

    /** @brief Transmits a pre-built raw @ref can_msg_t directly to the hardware. */
bool CanTxMessageHandler_can_frame(can_msg_t* can_msg) {

    return _transmit_can_frame(can_msg);

}
