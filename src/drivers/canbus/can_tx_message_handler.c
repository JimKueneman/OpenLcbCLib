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
 * @brief Implementation of message handlers for CAN transmit operations
 *
 * @details Implements conversion of OpenLCB messages to CAN frame format with proper
 * fragmentation for multi-frame messages. Handles addressed, unaddressed, datagram,
 * and stream message types according to OpenLCB CAN Frame Transfer specification.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
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

// CAN frame type constants for OpenLCB messages
const uint32_t OPENLCB_MESSAGE_DATAGRAM_ONLY = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_ONLY;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FIRST;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_MIDDLE;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FINAL;
const uint32_t OPENLCB_MESSAGE_STANDARD_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | OPENLCB_MESSAGE_STANDARD_FRAME_TYPE;

static interface_can_tx_message_handler_t *_interface;

    /**
     * @brief Initializes the CAN transmit message handler module
     *
     * @details Algorithm:
     * -# Cast away const qualifier from interface pointer
     * -# Store interface pointer in static module variable
     *
     * Use cases:
     * - Called once during application initialization
     * - Required before any CAN message transmission
     *
     * @verbatim
     * @param interface_can_tx_message_handler Pointer to interface structure containing required function pointers
     * @endverbatim
     *
     * @warning MUST be called during application initialization before any transmit operations
     * @warning NOT thread-safe - call only from main initialization context
     *
     * @attention Call after CAN hardware initialization but before CAN traffic begins
     *
     * @see CanTxStatemachine_initialize - Initialize transmit state machine
     */
void CanTxMessageHandler_initialize(const interface_can_tx_message_handler_t *interface_can_tx_message_handler) {

    _interface = (interface_can_tx_message_handler_t*) interface_can_tx_message_handler;

}

    /**
     * @brief Constructs CAN identifier for datagram only frame
     *
     * @details Algorithm:
     * -# Start with OPENLCB_MESSAGE_DATAGRAM_ONLY base value
     * -# Shift destination alias left 12 bits to position in CAN header
     * -# OR in source alias to lower 12 bits
     * -# Return complete 29-bit CAN identifier
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message containing alias information
     * @endverbatim
     *
     * @return 29-bit CAN identifier for datagram only frame
     *
     * @attention CAN identifier format: [reserved][frame_type][dest_alias][source_alias]
     *
     * @see _construct_identifier_datagram_first_frame - For first frame of multi-frame datagram
     */
static uint32_t _construct_identifier_datagram_only_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_ONLY | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

    /**
     * @brief Constructs CAN identifier for datagram first frame
     *
     * @details Algorithm:
     * -# Start with OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME base value
     * -# Shift destination alias left 12 bits to position in CAN header
     * -# OR in source alias to lower 12 bits
     * -# Return complete 29-bit CAN identifier
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message containing alias information
     * @endverbatim
     *
     * @return 29-bit CAN identifier for datagram first frame
     *
     * @attention First frame indicator allows receiver to prepare for multi-frame sequence
     *
     * @see _construct_identifier_datagram_middle_frame - For subsequent frames
     */
static uint32_t _construct_identifier_datagram_first_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

    /**
     * @brief Constructs CAN identifier for datagram middle frame
     *
     * @details Algorithm:
     * -# Start with OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME base value
     * -# Shift destination alias left 12 bits to position in CAN header
     * -# OR in source alias to lower 12 bits
     * -# Return complete 29-bit CAN identifier
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message containing alias information
     * @endverbatim
     *
     * @return 29-bit CAN identifier for datagram middle frame
     *
     * @attention Middle frames carry exactly 8 bytes of payload
     *
     * @see _construct_identifier_datagram_last_frame - For final frame
     */
static uint32_t _construct_identifier_datagram_middle_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

    /**
     * @brief Constructs CAN identifier for datagram last frame
     *
     * @details Algorithm:
     * -# Start with OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME base value
     * -# Shift destination alias left 12 bits to position in CAN header
     * -# OR in source alias to lower 12 bits
     * -# Return complete 29-bit CAN identifier
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message containing alias information
     * @endverbatim
     *
     * @return 29-bit CAN identifier for datagram last frame
     *
     * @attention Last frame may carry 0-8 bytes of payload
     * @attention Last frame signals end of datagram to receiver
     *
     * @see _construct_identifier_datagram_only_frame - For single-frame datagrams
     */
static uint32_t _construct_identifier_datagram_last_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

    /**
     * @brief Constructs CAN identifier for unaddressed message
     *
     * @details Algorithm:
     * -# Start with OPENLCB_MESSAGE_STANDARD_FRAME base value
     * -# Mask MTI to lowest 12 bits
     * -# Shift masked MTI left 12 bits to position in CAN header
     * -# OR in source alias to lower 12 bits
     * -# Return complete 29-bit CAN identifier
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message containing MTI and alias
     * @endverbatim
     *
     * @return 29-bit CAN identifier for unaddressed message frame
     *
     * @attention MTI embedded in CAN header content field
     * @attention No destination alias in header - broadcast message
     *
     * @see _construct_addressed_message_identifier - For addressed messages
     */
static uint32_t _construct_unaddressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_STANDARD_FRAME | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

    /**
     * @brief Constructs CAN identifier for addressed message
     *
     * @details Algorithm:
     * -# Start with OPENLCB_MESSAGE_STANDARD_FRAME base value
     * -# Mask MTI to lowest 12 bits
     * -# Shift masked MTI left 12 bits to position in CAN header
     * -# OR in source alias to lower 12 bits
     * -# Return complete 29-bit CAN identifier
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message containing MTI and alias
     * @endverbatim
     *
     * @return 29-bit CAN identifier for addressed message frame
     *
     * @attention Destination alias carried in first 2 payload bytes, not header
     * @attention MTI bit 3 indicates addressed message
     *
     * @see _construct_unaddressed_message_identifier - For broadcast messages
     */
static uint32_t _construct_addressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_STANDARD_FRAME | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

    /**
     * @brief Transmits a CAN frame and invokes optional callback
     *
     * @details Algorithm:
     * -# Call interface transmit_can_frame function
     * -# Store result
     * -# If transmission successful AND callback registered:
     *    - Invoke on_transmit callback
     * -# Return transmission result
     *
     * @verbatim
     * @param can_msg Pointer to CAN message to transmit
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention Callback only invoked on successful transmission
     *
     * @see interface_can_tx_message_handler_t - Interface definition
     */
static bool _transmit_can_frame(can_msg_t* can_msg) {

    bool result = _interface->transmit_can_frame(can_msg);

    if (_interface->on_transmit && result) {

        _interface->on_transmit(can_msg);

    }

    return result;

}

    /**
     * @brief Transmits a single-frame datagram
     *
     * @details Algorithm:
     * -# Set CAN identifier to datagram only frame type
     * -# Call transmit_can_frame to send
     * -# Return transmission result
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB datagram message
     * @param can_msg_worker Pointer to working CAN frame buffer
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention Payload must already be loaded into can_msg_worker
     *
     * @see _datagram_first_frame - For multi-frame datagrams
     */
static bool _datagram_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identifier_datagram_only_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

    /**
     * @brief Transmits first frame of multi-frame datagram
     *
     * @details Algorithm:
     * -# Set CAN identifier to datagram first frame type
     * -# Call transmit_can_frame to send
     * -# Return transmission result
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB datagram message
     * @param can_msg_worker Pointer to working CAN frame buffer
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention First frame must carry exactly 8 bytes of payload
     * @attention Signals receiver to expect more frames
     *
     * @see _datagram_middle_frame - For subsequent frames
     */
static bool _datagram_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identifier_datagram_first_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

    /**
     * @brief Transmits middle frame of multi-frame datagram
     *
     * @details Algorithm:
     * -# Set CAN identifier to datagram middle frame type
     * -# Call transmit_can_frame to send
     * -# Return transmission result
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB datagram message
     * @param can_msg_worker Pointer to working CAN frame buffer
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention Middle frames must carry exactly 8 bytes of payload
     * @attention Multiple middle frames allowed in sequence
     *
     * @see _datagram_last_frame - For final frame
     */
static bool _datagram_middle_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identifier_datagram_middle_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

    /**
     * @brief Transmits last frame of multi-frame datagram
     *
     * @details Algorithm:
     * -# Set CAN identifier to datagram last frame type
     * -# Call transmit_can_frame to send
     * -# Return transmission result
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB datagram message
     * @param can_msg_worker Pointer to working CAN frame buffer
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention Last frame may carry 0-8 bytes of payload
     * @attention Signals completion of datagram to receiver
     *
     * @see _datagram_only_frame - For single-frame datagrams
     */
static bool _datagram_last_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identifier_datagram_last_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

    /**
     * @brief Transmits single-frame addressed message
     *
     * @details Algorithm:
     * -# Set multi-frame flag in payload to MULTIFRAME_ONLY
     * -# Call transmit_can_frame to send
     * -# Return transmission result
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message
     * @param can_msg Pointer to CAN frame with identifier and destination already set
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention Destination alias already in first 2 payload bytes
     * @attention Payload limited to 6 bytes (8 - 2 for destination)
     *
     * @see _addressed_message_first_frame - For multi-frame messages
     */
static bool _addressed_message_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_ONLY);
    return _transmit_can_frame(can_msg);

}

    /**
     * @brief Transmits first frame of multi-frame addressed message
     *
     * @details Algorithm:
     * -# Set multi-frame flag in payload to MULTIFRAME_FIRST
     * -# Call transmit_can_frame to send
     * -# Return transmission result
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message
     * @param can_msg Pointer to CAN frame with identifier and destination already set
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention First frame must carry exactly 6 bytes (8 - 2 for destination)
     * @attention Multi-frame flag in high nibble of first payload byte
     *
     * @see _addressed_message_middle_frame - For continuation frames
     */
static bool _addressed_message_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FIRST);
    return _transmit_can_frame(can_msg);

}

    /**
     * @brief Transmits middle frame of multi-frame addressed message
     *
     * @details Algorithm:
     * -# Set multi-frame flag in payload to MULTIFRAME_MIDDLE
     * -# Call transmit_can_frame to send
     * -# Return transmission result
     *
     * @verbatim
     * @param can_msg Pointer to CAN frame with identifier and destination already set
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention Middle frames carry destination in first 2 bytes plus 6 data bytes
     * @attention Multiple middle frames allowed in sequence
     *
     * @see _addressed_message_last_frame - For final frame
     */
static bool _addressed_message_middle_frame(can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_MIDDLE);
    return _transmit_can_frame(can_msg);

}

    /**
     * @brief Transmits last frame of multi-frame addressed message
     *
     * @details Algorithm:
     * -# Set multi-frame flag in payload to MULTIFRAME_FINAL
     * -# Call transmit_can_frame to send
     * -# Return transmission result
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message
     * @param can_msg Pointer to CAN frame with identifier and destination already set
     * @endverbatim
     *
     * @return True if transmission successful, false otherwise
     *
     * @attention Last frame carries 2-8 total bytes (including destination)
     * @attention Signals completion of message to receiver
     *
     * @see _addressed_message_only_frame - For single-frame messages
     */
static bool _addressed_message_last_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FINAL);
    return _transmit_can_frame(can_msg);

}

    /**
     * @brief Loads destination alias into first two payload bytes
     *
     * @details Algorithm:
     * -# Extract high byte of destination alias (bits 8-11)
     * -# Store in payload byte 0
     * -# Extract low byte of destination alias (bits 0-7)
     * -# Store in payload byte 1
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message containing destination alias
     * @param can_msg Pointer to CAN frame to receive destination in payload
     * @endverbatim
     *
     * @attention Destination alias always occupies first 2 payload bytes in addressed messages
     * @attention Multi-frame flag will be OR'd into high nibble of byte 0 later
     *
     * @see _addressed_message_only_frame - Uses loaded destination
     */
static void _load_destination_address_in_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    can_msg->payload[0] = (openlcb_msg->dest_alias >> 8) & 0xFF; // Setup the first two CAN data bytes with the destination address
    can_msg->payload[1] = openlcb_msg->dest_alias & 0xFF;

}

    /**
     * @brief Converts and transmits datagram message as CAN frame(s)
     *
     * @details Algorithm:
     * -# Copy OpenLCB payload to CAN payload starting at current index
     * -# Determine frame type based on payload size and current position:
     *    - If total payload ≤ 8 bytes: send as only frame
     *    - Else if at start (index < 8): send as first frame
     *    - Else if more data remains: send as middle frame
     *    - Else: send as last frame
     * -# If transmission successful:
     *    - Increment payload index by bytes copied
     * -# Return transmission result
     *
     * Use cases:
     * - Sending Memory Configuration Protocol requests
     * - Sending Remote Button Protocol commands
     * - Transmitting any datagram-based protocol data
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB datagram message to transmit
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in datagram payload (updated after transmission)
     * @endverbatim
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Maximum datagram size is 72 bytes on CAN transport
     * @warning Transmission failure leaves payload index unchanged
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention Frame sequence: only OR first→middle(s)→last
     * @attention All frames carry maximum 8 bytes except possibly last frame
     *
     * @see CanTxMessageHandler_stream_frame - For streaming data
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
     * @brief Converts and transmits unaddressed message as CAN frame(s)
     *
     * @details Algorithm:
     * -# Check if message fits in single frame (≤ 8 bytes):
     *    - Copy OpenLCB payload to CAN payload
     *    - Construct unaddressed message identifier
     *    - Transmit frame
     *    - If successful: update payload index
     * -# If multi-frame required:
     *    - Currently not implemented (see TODO)
     * -# Return transmission result
     *
     * Use cases:
     * - Broadcasting Initialization Complete
     * - Broadcasting Producer/Consumer Event Reports
     * - Broadcasting Verified Node ID
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message to transmit (no dest_alias required)
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in OpenLCB payload (updated after transmission)
     * @endverbatim
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Multi-frame unaddressed messages not currently implemented
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention All 8 payload bytes available (no destination alias overhead)
     *
     * @see CanTxMessageHandler_addressed_msg_frame - For targeted messages
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
     * @brief Converts and transmits addressed message as CAN frame(s)
     *
     * @details Algorithm:
     * -# Load destination alias into first 2 payload bytes
     * -# Construct addressed message identifier with MTI
     * -# Copy OpenLCB payload to CAN payload starting at byte 2
     * -# Determine frame type based on total payload size:
     *    - If ≤ 6 bytes: send as only frame
     *    - Else if at start (index < 6): send as first frame
     *    - Else if more data remains: send as middle frame
     *    - Else: send as last frame
     * -# If transmission successful:
     *    - Increment payload index by bytes copied
     * -# Return transmission result
     *
     * Use cases:
     * - Sending Protocol Support Inquiry to specific node
     * - Sending Verify Node ID to specific node
     * - Sending any message requiring destination address
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message to transmit (must have dest_alias set)
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in OpenLCB payload (updated after transmission)
     * @endverbatim
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Transmission failure leaves payload index unchanged - caller must retry
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention First two payload bytes reserved for destination alias in all frames
     * @attention Multi-frame messages use framing flags: only/first/middle/last
     *
     * @see CanTxMessageHandler_unaddressed_msg_frame - For broadcast messages
     * @see CanUtilities_copy_openlcb_payload_to_can_payload - Payload copying helper
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

    /**
     * @brief Converts and transmits stream message as CAN frame(s)
     *
     * @details Algorithm:
     * -# Return true immediately (placeholder implementation)
     *
     * Use cases:
     * - Firmware upgrade data transfer (when implemented)
     * - Large file transfers (when implemented)
     * - Continuous data streaming (when implemented)
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB stream message to transmit
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in stream payload (updated after transmission)
     * @endverbatim
     *
     * @return Currently always returns true (placeholder implementation)
     *
     * @warning Stream protocol NOT fully implemented - placeholder only
     * @warning Do not rely on this function for production stream transfers
     *
     * @attention Function requires full implementation before use
     *
     * @see CanTxMessageHandler_datagram_frame - For datagram transfers
     */
bool CanTxMessageHandler_stream_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    // ToDo: implement streams
    return true;
}

    /**
     * @brief Transmits a pre-built CAN frame to the physical bus
     *
     * @details Algorithm:
     * -# Call internal transmit function which:
     *    - Calls hardware transmit function
     *    - Invokes callback if transmission successful
     * -# Return transmission result
     *
     * Use cases:
     * - Transmitting alias allocation frames during node login
     * - Sending CAN control messages
     * - Direct CAN bus operations
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer containing frame to transmit
     * @endverbatim
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Frame must be fully constructed before calling
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention No OpenLCB processing performed - raw CAN transmission
     *
     * @see CanTxMessageHandler_addressed_msg_frame - For OpenLCB messages
     */
bool CanTxMessageHandler_can_frame(can_msg_t* can_msg) {

    return _transmit_can_frame(can_msg);

}
