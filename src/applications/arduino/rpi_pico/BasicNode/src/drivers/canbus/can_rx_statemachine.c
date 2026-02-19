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
 * @file can_rx_statemachine.c
 * @brief Implementation of the CAN receive state machine
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "can_rx_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"

#define OFFSET_DEST_ID_IN_PAYLOAD     2
#define OFFSET_DEST_ID_IN_IDENTIFIER  0
#define OFFSET_NO_DEST_ID             0

static interface_can_rx_statemachine_t *_interface;

    /**
     * @brief Initializes the CAN Receive State Machine module
     *
     * @details Algorithm:
     * -# Store pointer to dependency injection interface
     * -# Interface remains valid for application lifetime
     *
     * Use cases:
     * - Application startup
     * - System initialization
     *
     * @verbatim
     * @param interface_can_rx_statemachine Pointer to interface structure
     * @endverbatim
     *
     * @warning Must be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call after CanRxMessageHandler_initialize
     *
     * @see interface_can_rx_statemachine_t - Interface definition
     */
void CanRxStatemachine_initialize(const interface_can_rx_statemachine_t *interface_can_rx_statemachine) {

    _interface = (interface_can_rx_statemachine_t*) interface_can_rx_statemachine;

}

static uint16_t _extract_can_mti_from_can_identifier(can_msg_t *can_msg) {

    return (can_msg->identifier >> 12) & 0x0FFF;
}

    /**
     * @brief Processes addressed OpenLCB message frames
     *
     * @details Algorithm:
     * -# Extract destination alias from payload
     * -# Check if destination is our node
     * -# If not: return (not for us)
     * -# Extract framing bits
     * -# Determine payload type from MTI
     * -# Handle legacy SNIP special case
     * -# Dispatch based on framing bits
     *
     * Use cases:
     * - Processing datagrams
     * - Processing addressed messages
     * - Multi-frame assembly
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame
     * @param can_mti 12-bit CAN MTI
     * @endverbatim
     *
     * @warning Returns silently if not for our node
     * @warning NOT thread-safe
     *
     * @attention First 2 payload bytes contain destination alias
     *
     * @see _handle_openlcb_msg_can_frame_unaddressed - Handles global messages
     */
static void _handle_openlcb_msg_can_frame_addressed(can_msg_t* can_msg, uint16_t can_mti) {

    // Handle addressed message, note this assumes the message has already been tested be for one of our nodes

    switch (can_msg->payload[0] & MASK_MULTIFRAME_BITS) { // Extract Framing Bits

        case MULTIFRAME_ONLY:

            // Special case when SNIPs were defined before the framing bits where added to the protocol

            if (can_mti == MTI_SIMPLE_NODE_INFO_REPLY) {

                if (_interface->handle_can_legacy_snip) {

                    _interface->handle_can_legacy_snip(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, SNIP);

                }

            } else {

                if (_interface->handle_single_frame) {

                    _interface->handle_single_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, BASIC);

                }

            }

            break;

        case MULTIFRAME_FIRST:

            // Special case when SNIPs were defined before the framing bits where added to the protocol

            if (can_mti == MTI_SIMPLE_NODE_INFO_REPLY) {

                if (_interface->handle_first_frame) {

                    _interface->handle_first_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, SNIP);

                }

            } else {

                if (_interface->handle_first_frame) {

                    // TODO: This could be dangerous if a future message used more than 2 frames.... (larger than LEN_MESSAGE_BYTES_BASIC)

                    _interface->handle_first_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, BASIC);

                }

            }

            break;

        case MULTIFRAME_MIDDLE:

            if (_interface->handle_middle_frame) {

                _interface->handle_middle_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD);

            }

            break;

        case MULTIFRAME_FINAL:

            if (_interface->handle_last_frame) {

                _interface->handle_last_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD);

            }

            break;

        default:

            break;

    }

}

    /**
     * @brief Processes global (unaddressed) OpenLCB messages
     *
     * @details Algorithm:
     * -# Extract framing bits if present
     * -# Determine payload type
     * -# Dispatch based on framing bits
     *
     * Use cases:
     * - Event reports
     * - Verify Node ID global
     * - Initialization Complete
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame
     * @param can_mti 12-bit CAN MTI
     * @endverbatim
     *
     * @warning NOT thread-safe
     *
     * @attention No destination alias in payload
     *
     * @see _handle_openlcb_msg_can_frame_addressed - Handles addressed
     */
static void _handle_openlcb_msg_can_frame_unaddressed(can_msg_t* can_msg, uint16_t can_mti) {

    switch (can_mti) {

        // PC Event Report with payload is a unicorn global message and needs special attention

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD_FIRST:

        {

            if (_interface->handle_first_frame) {

                _interface->handle_first_frame(can_msg, OFFSET_NO_DEST_ID, SNIP);

            }

            break;
        }

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD_MIDDLE:
        {

            if (_interface->handle_middle_frame) {

                _interface->handle_middle_frame(can_msg, OFFSET_NO_DEST_ID);

            }

            break;
        }

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD_LAST:
        {

            if (_interface->handle_last_frame) {

                _interface->handle_last_frame(can_msg, OFFSET_NO_DEST_ID);

            }

            break;
        }

        default:

            if (_interface->handle_single_frame) {

                _interface->handle_single_frame(can_msg, OFFSET_NO_DEST_ID, BASIC);

            }

            break;

    }

}

static void _handle_can_type_frame(can_msg_t* can_msg) {

    // Raw CAN messages coming in from the wire that are CAN interpretations of OpenLcb defined messages

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

    case OPENLCB_MESSAGE_STANDARD_FRAME_TYPE: // It is a global or addressed message type

        if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT)
        {

            // If it is a message targeting a destination node make sure it is for one of our nodes

            if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg)))
            {

                break;
            }

            // Addressed message for one of our nodes

            _handle_openlcb_msg_can_frame_addressed(can_msg, _extract_can_mti_from_can_identifier(can_msg));
        }
        else
        {

            // Global message just handle it

            /**
             * @brief Extracts 12-bit CAN MTI from 29-bit identifier
             *
             * @details Algorithm:
             * -# Mask identifier with MASK_CAN_FRAME_TYPE_MTI
             * -# Right shift by 12 bits
             * -# Return 12-bit CAN MTI
             *
             * Use cases:
             * - Decoding message type
             * - Message routing
             *
             * @verbatim
             * @param can_msg Pointer to CAN message
             * @endverbatim
             *
             * @return 12-bit CAN MTI from identifier bits [23:12]
             *
             * @warning Only valid for OpenLCB message frames (OPENLCB_MESSAGE_STANDARD_FRAME_TYPE)
             *
             * @see CanUtilities_convert_can_mti_to_openlcb_mti - Converts to full MTI
             */
            _handle_openlcb_msg_can_frame_unaddressed(can_msg, _extract_can_mti_from_can_identifier(can_msg));
        }

        break;

    case CAN_FRAME_TYPE_DATAGRAM_ONLY:

        // If it is a datagram make sure it is for one of our nodes

        if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg)))
        {

            break;
        }

        // Datagram message for one of our nodes

        if (_interface->handle_single_frame)
        {

            _interface->handle_single_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, BASIC);
        }

        break;

    case CAN_FRAME_TYPE_DATAGRAM_FIRST:

        // If it is a datagram make sure it is for one of our nodes

        if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg)))
        {

            break;
        }

        // Datagram message for one of our nodes

        if (_interface->handle_first_frame)
        {

            _interface->handle_first_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, DATAGRAM);
        }

        break;

    case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:

        // If it is a datagram make sure it is for one of our nodes

        if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg)))
        {

            break;
        }

        // Datagram message for one of our nodes

        if (_interface->handle_middle_frame)
        {

            _interface->handle_middle_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER);
        }

        break;

    case CAN_FRAME_TYPE_DATAGRAM_FINAL:

        // If it is a datagram make sure it is for one of our nodes

        if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg)))
        {

            break;
        }

        // Datagram message for one of our nodes

        if (_interface->handle_last_frame)
        {

            _interface->handle_last_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER);
        }

        break;

    case CAN_FRAME_TYPE_RESERVED:

        break;

    case CAN_FRAME_TYPE_STREAM:

        // If it is a stream message make sure it is for one of our nodes

        if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg)))
        {

            break;
        }

        // Stream message for one of our nodes

        if (_interface->handle_stream_frame)
        {

            _interface->handle_stream_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, STREAM);
        }

        break;

    default:

        break;

    }

}

static void _handle_can_control_frame_variable_field(can_msg_t* can_msg) {

    switch (can_msg->identifier & MASK_CAN_VARIABLE_FIELD) {

        case CAN_CONTROL_FRAME_RID: // Reserve ID

            if (_interface->handle_rid_frame) {

                _interface->handle_rid_frame(can_msg);

            }

            break;

        case CAN_CONTROL_FRAME_AMD: // Alias Map Definition

            if (_interface->handle_amd_frame) {

                _interface->handle_amd_frame(can_msg);

            }

            break;

        case CAN_CONTROL_FRAME_AME:

            if (_interface->handle_ame_frame) {

                _interface->handle_ame_frame(can_msg);

            }

            break;

        case CAN_CONTROL_FRAME_AMR:

            if (_interface->handle_amr_frame) {

                _interface->handle_amr_frame(can_msg);

            }

            break;

        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0: // Advanced feature for gateways/routers/etc.
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1:
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2:
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3:

            if (_interface->handle_error_info_report_frame) {

                _interface->handle_error_info_report_frame(can_msg);

            }

            break;

        default:

            // Do nothing
            break; // default

    }

}

static void _handle_can_control_frame_sequence_number(can_msg_t* can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

        case CAN_CONTROL_FRAME_CID7:
        case CAN_CONTROL_FRAME_CID6:
        case CAN_CONTROL_FRAME_CID5:
        case CAN_CONTROL_FRAME_CID4:
        case CAN_CONTROL_FRAME_CID3:
        case CAN_CONTROL_FRAME_CID2:
        case CAN_CONTROL_FRAME_CID1:

            if (_interface->handle_cid_frame) {

                _interface->handle_cid_frame(can_msg);

            }

            break;

        default:

            break;

    }

}

static void _handle_can_control_frame(can_msg_t* can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

        case 0:

    /**
     * @brief Handles CAN control frames (0x0700-0x071F)
     *
     * @details Algorithm:
     * -# Extract variable field from bits [26:12]
     * -# Decode frame type (RID/AMD/AME/AMR/Error)
     * -# Dispatch to handler
     *
     * Use cases:
     * - Alias management
     * - Network discovery
     * - Error reporting
     *
     * @verbatim
     * @param can_msg Pointer to received control frame
     * @endverbatim
     *
     * @warning NOT thread-safe
     *
     * @attention Variable field in bits [26:12]
     *
     * @see CanRxMessageHandler_rid_frame - RID handler
     */
            _handle_can_control_frame_variable_field(can_msg);

            break;

        default:

    /**
     * @brief Handles CID frames (sequence number encoding)
     *
     * @details Algorithm:
     * -# Extract sequence field
     * -# Check if matches CID pattern
     * -# Dispatch to CID handler
     *
     * Use cases:
     * - Processing Check ID frames
     * - Alias conflict detection
     *
     * @verbatim
     * @param can_msg Pointer to received CID frame
     * @endverbatim
     *
     * @warning NOT thread-safe
     *
     * @attention CID sequence: 7, 6, 5, 4
     *
     * @see CanRxMessageHandler_cid_frame - CID handler
     */
            _handle_can_control_frame_sequence_number(can_msg);

            break; // default

    }

}

    /**
     * @brief Main entry point for incoming CAN frames from hardware driver
     *
     * @details Algorithm:
     * -# Call optional on_receive callback if registered
     * -# Use CanUtilities_is_openlcb_message to determine frame type
     * -# Route based on result:
     *    - true: OpenLCB message â†' Call _handle_can_type_frame
     *    - false: CAN control frame â†' Call _handle_can_control_frame
     *
     * Use cases:
     * - CAN receive interrupt handler
     * - CAN receive thread/task
     * - Polled CAN frame reception
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame from hardware driver
     * @endverbatim
     *
     * @warning Must not be called when shared resources are locked
     * @warning NOT thread-safe with main state machine
     * @warning Frame buffer must remain valid during processing
     *
     * @attention Application must coordinate with lock_shared_resources
     * @attention Common implementation: disable CAN RX interrupt during lock
     * @attention Alternative: queue frames when locked, process after unlock
     *
     * @note CanUtilities_is_openlcb_message checks identifier to distinguish types
     * @note OpenLCB messages use OPENLCB_MESSAGE_STANDARD_FRAME_TYPE or datagram/stream types
     * @note CAN control frames use different identifier encoding
     *
     * @see CanMainStatemachine_run - Main state machine that may lock resources
     * @see CanUtilities_is_openlcb_message - Frame type check utility
     * @see _handle_can_type_frame - Routes OpenLCB messages
     * @see _handle_can_control_frame - Routes CAN control frames
     */
void CanRxStatemachine_incoming_can_driver_callback(can_msg_t* can_msg) {

    // This is called directly from the incoming CAN receiver as raw Openlcb CAN messages

    // First see if the application has defined a callback
    if (_interface->on_receive) {

        _interface->on_receive(can_msg);

    }


    // Second split the message up between is it a CAN control message (AMR, AME, AMD, RID, CID, etc.)
    if (CanUtilities_is_openlcb_message(can_msg)) {

    /**
     * @brief Routes CAN frame based on frame type field
     *
     * @details Algorithm:
     * -# Mask CAN identifier with MASK_CAN_FRAME_TYPE
     * -# Switch on frame type:
     *    - OPENLCB_MESSAGE_STANDARD_FRAME_TYPE: Standard global/addressed message
     *    - CAN_FRAME_TYPE_DATAGRAM_ONLY: Single-frame datagram
     *    - CAN_FRAME_TYPE_DATAGRAM_FIRST: First frame of multi-frame datagram
     *    - CAN_FRAME_TYPE_DATAGRAM_MIDDLE: Middle frame of datagram
     *    - CAN_FRAME_TYPE_DATAGRAM_FINAL: Last frame of datagram
     *    - CAN_FRAME_TYPE_STREAM: Stream data frame
     * -# Dispatch to appropriate handler based on type
     *
     * Use cases:
     * - Routing all OpenLCB messages from CAN bus
     * - Frame type-specific handling
     * - Multi-frame message coordination
     *
     * @verbatim
     * @param can_msg Pointer to received OpenLCB message frame
     * @endverbatim
     *
     * @warning NOT thread-safe
     *
     * @attention Frame type field in identifier determines message structure
     * @attention OPENLCB_MESSAGE_STANDARD_FRAME_TYPE indicates standard format message
     *
     * @note Frame types defined by CAN Frame Transfer Standard
     *
     * @see OPENLCB_MESSAGE_STANDARD_FRAME_TYPE - Standard message frame type constant
     * @see CAN_FRAME_TYPE_DATAGRAM_* - Datagram frame type constants
     * @see CAN_FRAME_TYPE_STREAM - Stream frame type constant
     */
        _handle_can_type_frame(can_msg); //  Handle pure OpenLCB CAN Messages


    } else {

    /**
     * @brief Routes CAN control frames to handlers
     *
     * @details Algorithm:
     * -# Extract control frame type from bits [26:16]
     * -# Check if variable field (0x0700-0x071F)
     * -# Else check if CID pattern
     * -# Dispatch accordingly
     *
     * Use cases:
     * - Routing control frames
     * - Filtering invalid frames
     *
     * @verbatim
     * @param can_msg Pointer to received control frame
     * @endverbatim
     *
     * @warning Silently ignores unrecognized types
     * @warning NOT thread-safe
     *
     * @attention Control frames have frame type bit = 0
     *
     * @see _handle_can_control_frame_variable_field - RID/AMD/AME/AMR/Error
     */
        _handle_can_control_frame(can_msg); // CAN Control Messages

    }

}


