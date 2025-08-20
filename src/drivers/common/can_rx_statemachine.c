
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

static uint16_t _extract_can_mti_from_can_identifier(can_msg_t *can_msg) {

    return (can_msg->identifier >> 12) & 0x0FFF;
}

static void _state_machine_incoming_can_driver_callback(uint8_t channel, can_msg_t* can_msg) {

    if (CanUtilities_is_openlcb_message(can_msg)) {

        //  Handle pure OpenLCB Messages

        switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

            case CAN_FRAME_TYPE_GLOBAL_ADDRESSED:

                if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT) {

                    uint16_t can_mti = _extract_can_mti_from_can_identifier(can_msg);

                    switch (can_msg->payload[0] & 0xF0) { // Extract Framing Bits

                        case MULTIFRAME_ONLY:

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
                } else { // No Destination Address

                    if (_interface->handle_single_frame) {

                        _interface->handle_single_frame(can_msg, OFFSET_NO_DEST_ID, BASIC);

                    }

                }

                break;

            case CAN_FRAME_TYPE_DATAGRAM_ONLY:

                if (_interface->handle_single_frame) {

                    _interface->handle_single_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, BASIC);

                }

                break;

            case CAN_FRAME_TYPE_DATAGRAM_FIRST:

                if (_interface->handle_first_frame) {

                    _interface->handle_first_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, DATAGRAM);

                }

                break;

            case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:

                if (_interface->handle_middle_frame) {

                    _interface->handle_middle_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER);

                }

                break;

            case CAN_FRAME_TYPE_DATAGRAM_FINAL:

                if (_interface->handle_last_frame) {

                    _interface->handle_last_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER);

                }

                break;

            case CAN_FRAME_TYPE_RESERVED:

                break;

            case CAN_FRAME_TYPE_STREAM:
                
                if (_interface->handle_stream) {

                    _interface->handle_stream(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, STREAM);

                }

                break;
        }

    } else {

        // CAN Control Messages

        switch (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

            case 0:
                
                switch (can_msg->identifier & MASK_CAN_VARIABLE_FIELD) {
                    

                    case CAN_CONTROL_FRAME_RID: // Reserve ID

                        if (_interface->handle_rid) {

                            _interface->handle_rid(can_msg);

                        }

                        break;

                    case CAN_CONTROL_FRAME_AMD: // Alias Map Definition

                        if (_interface->handle_amd) {

                            _interface->handle_amd(can_msg);

                        }

                        break;

                    case CAN_CONTROL_FRAME_AME:  
                        
                        if (_interface->handle_ame) {
  
                            _interface->handle_ame(can_msg);

                        }

                        break;

                    case CAN_CONTROL_FRAME_AMR:
 
                        if (_interface->handle_amr) {

                            _interface->handle_amr(can_msg);

                        }

                        break;

                    case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0: // Advanced feature for gateways/routers/etc.
                    case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1:
                    case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2:
                    case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3:

                        if (_interface->handle_error_information_report) {
                       
                            _interface->handle_error_information_report(can_msg);
                        
                        }
                        
                        break;

                    default:

                        // Do nothing
                        break; // default

                }

                break;

            default:

                switch (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

                    case CAN_CONTROL_FRAME_CID7:
                    case CAN_CONTROL_FRAME_CID6:
                    case CAN_CONTROL_FRAME_CID5:
                    case CAN_CONTROL_FRAME_CID4:


                        if (_interface->handle_cid) {
                       
                            _interface->handle_cid(can_msg);
                        
                        }

                        break;

                    case CAN_CONTROL_FRAME_CID3:
                    case CAN_CONTROL_FRAME_CID2:
                    case CAN_CONTROL_FRAME_CID1:

                        break;
                }

                break; // default

        } // CAN control messages

    }

}

// Call on startup to initialize variables and callbacks

void CanRxStatemachine_initialize(const interface_can_rx_statemachine_t *interface_can_rx_statemachine) {

    _interface = (interface_can_rx_statemachine_t*) interface_can_rx_statemachine;
    
    if (interface_can_rx_statemachine->can_rx_register_target_callback) {

      interface_can_rx_statemachine->can_rx_register_target_callback(&_state_machine_incoming_can_driver_callback);
      
    }

}




