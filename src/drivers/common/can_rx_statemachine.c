
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
 * \file can_rx_statemachine.c
 *
 * Handles the raw CAN frame structures and Pushes them onto the FIFO stack to be
 * processed in the main loop.  If the message is across multiple CAN frames it uses 
 * the Buffer List to hold the OpenLcb message as it builds it through gathering the
 * various CAN frames that make it up.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
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

void CanRxStatemachine_initialize(const interface_can_rx_statemachine_t *interface_can_rx_statemachine) {

    _interface = (interface_can_rx_statemachine_t*) interface_can_rx_statemachine;

}

static uint16_t _extract_can_mti_from_can_identifier(can_msg_t *can_msg) {

    return (can_msg->identifier >> 12) & 0x0FFF;
}

static void _handle_openlcb_msg_can_frame_addressed(can_msg_t* can_msg, uint16_t can_mti) {
    
    // Handle addressed message, note this assumes the message has already been tested be for one of our nodes

    switch (can_msg->payload[0] & 0xF0) { // Extract Framing Bits

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
    }

}

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

        case CAN_FRAME_TYPE_GLOBAL_ADDRESSED:
            
            // First see if it is normal Global or Addressed Openlcb Message

            if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT) {
                
                // If it is a message targeting a destination node make sure it is for one of our nodes

                if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg))) {

                    break;

                }

                // Addressed message for one of our nodes
                
                _handle_openlcb_msg_can_frame_addressed(can_msg, _extract_can_mti_from_can_identifier(can_msg));

            } else { 
                
                // Global message just handle it

                _handle_openlcb_msg_can_frame_unaddressed(can_msg, _extract_can_mti_from_can_identifier(can_msg));

            }

            break;

        case CAN_FRAME_TYPE_DATAGRAM_ONLY:
            
            // If it is a datagram make sure it is for one of our nodes

            if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg))) {

                break;

            }

            // Datagram message for one of our nodes
            
            if (_interface->handle_single_frame) {

                _interface->handle_single_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, BASIC);

            }

            break;

        case CAN_FRAME_TYPE_DATAGRAM_FIRST:
            
            // If it is a datagram make sure it is for one of our nodes

            if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg))) {

                break;

            }
            
            // Datagram message for one of our nodes

            if (_interface->handle_first_frame) {

                _interface->handle_first_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, DATAGRAM);

            }

            break;

        case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:
            
            // If it is a datagram make sure it is for one of our nodes

            if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg))) {

                break;

            }
            
            // Datagram message for one of our nodes

            if (_interface->handle_middle_frame) {

                _interface->handle_middle_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER);

            }

            break;

        case CAN_FRAME_TYPE_DATAGRAM_FINAL:
            
            // If it is a datagram make sure it is for one of our nodes

            if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg))) {

                break;

            }
            
            // Datagram message for one of our nodes

            if (_interface->handle_last_frame) {

                _interface->handle_last_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER);

            }

            break;

        case CAN_FRAME_TYPE_RESERVED:

            break;

        case CAN_FRAME_TYPE_STREAM:
            
            // If it is a stream message make sure it is for one of our nodes

            if (!_interface->alias_mapping_find_mapping_by_alias(CanUtilities_extract_dest_alias_from_can_message(can_msg))) {

                break;

            }
            
            // Stream message for one of our nodes

            if (_interface->handle_stream_frame) {

                _interface->handle_stream_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, STREAM);

            }

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
    }

}

static void _handle_can_control_frame(can_msg_t* can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

        case 0:

            _handle_can_control_frame_variable_field(can_msg);

            break;

        default:

            _handle_can_control_frame_sequence_number(can_msg);

            break; // default

    }

}

void CanRxStatemachine_incoming_can_driver_callback(can_msg_t* can_msg) {
    
    // This is called directly from the incoming CAN receiver as raw Openlcb CAN messages

    // First see if the application has defined a callback
    if (_interface->on_receive) {

        _interface->on_receive(can_msg);

    }


    // Second split the message up between is it a CAN control message (AMR, AME, AMD, RID, CID, etc.)
    if (CanUtilities_is_openlcb_message(can_msg)) {

        _handle_can_type_frame(can_msg); //  Handle pure OpenLCB CAN Messages


    } else {

        _handle_can_control_frame(can_msg); // CAN Control Messages

    }

}


