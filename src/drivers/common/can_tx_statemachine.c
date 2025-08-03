
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
 * \file can_tx_statemachine.c
 *
 * Takes an OpenLcb message structure and splits it into CAN frames to transmit if 
 * necessary, else it packs up the CAN frame from the message structure and send it
 * to the CAN Driver to transmit on the physical layer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_tx_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "../driver_can.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_application_callbacks.h"


// #define CAN_TX_TEST

const uint32_t OPENLCB_MESSAGE_DATAGRAM_ONLY = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_ONLY;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FIRST;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_MIDDLE;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FINAL;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_UNADDRESSED = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_ADDRESSED = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED;

void CanTxStatemachine_initialize(void) {


}

uint32_t _construct_identfier_datagram_only_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_ONLY | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t _construct_identfier_datagram_first_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t _construct_identfier_datagram_middle_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t _construct_identfier_datagram_last_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t _construct_unaddressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_UNADDRESSED | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

uint32_t _construct_addressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_ADDRESSED | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

bool _transmit_can_frame(can_msg_t* can_msg) {

#ifdef CAN_TX_TEST 

    printf("\n");
    PrintCanMsg(can_msg);
    printf("\n\n");

    return true;

#else

    parameterless_callback_t tx_callback = OpenLcbApplicationCallbacks_get_can_tx();

    uint8_t result = DriverCan_transmit_raw_can_frame(TX_CHANNEL_OPENLCB_MSG, can_msg);

    if (tx_callback && result) {

        tx_callback();

    }

    return result;

#endif

}

bool _datagram_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_only_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

bool _datagram_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_first_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

bool _datagram_middle_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_middle_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

bool _datagram_last_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_last_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

bool _addressed_message_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_ONLY);
    return _transmit_can_frame(can_msg);

}

bool _addressed_message_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FIRST);
    return _transmit_can_frame(can_msg);

}

bool _addressed_message_middle(can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_MIDDLE);
    return _transmit_can_frame(can_msg);

}

bool _addressed_message_last(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FINAL);
    return _transmit_can_frame(can_msg);

}

void _load_destination_address_in_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    can_msg->payload[0] = (openlcb_msg->dest_alias >> 8) & 0xFF; // Setup the first two CAN data bytes with the destination address
    can_msg->payload[1] = openlcb_msg->dest_alias & 0xFF;

}

bool _handle_datagram_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

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

bool _handle_unaddressed_msg_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

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

bool _handle_addressed_msg_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    _load_destination_address_in_payload(openlcb_msg, can_msg_worker);


    can_msg_worker->identifier = _construct_addressed_message_identifier(openlcb_msg);
    uint8_t len_msg_frame = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, *openlcb_start_index, OFFSET_CAN_WITH_DEST_ADDRESS);
    bool result = false;

    if (openlcb_msg->payload_count <= 6) {// Account for 2 bytes used for dest alias

        result = _addressed_message_only_frame(openlcb_msg, can_msg_worker);

    } else if (*openlcb_start_index < 6) { // Account for 2 bytes used for dest alias

        result = _addressed_message_first_frame(openlcb_msg, can_msg_worker);

    } else if ((*openlcb_start_index + len_msg_frame) < openlcb_msg->payload_count) {

        result = _addressed_message_middle(can_msg_worker);

    } else {

        result = _addressed_message_last(openlcb_msg, can_msg_worker);

    }
    
    if (result) {
        
        *openlcb_start_index = *openlcb_start_index + len_msg_frame;
        
    }
    
    return result;

}

bool CanTxStatemachine_try_transmit_openlcb_message(openlcb_msg_t* openlcb_msg) {

    if (!DriverCan_is_can_tx_buffer_clear(TX_CHANNEL_CAN_CONTROL)) {

        return false;

    }

    can_msg_t worker_can_msg;
    uint16_t payload_index = 0;
    
    // TODO: break the message into can frame and queue them in the can frame buffer for sending to not block the main loop here.

    while (payload_index < openlcb_msg->payload_count) {

        if (OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg)) {

            switch (openlcb_msg->mti) {

                case MTI_DATAGRAM:

                     _handle_datagram_frame(openlcb_msg, &worker_can_msg, &payload_index);

                    break;

                case MTI_STREAM_COMPLETE:
                case MTI_STREAM_INIT_REPLY:
                case MTI_STREAM_INIT_REQUEST:
                case MTI_STREAM_PROCEED:

                    return true; // TODO:  Implement Streams

                    break;

                default:

                    _handle_addressed_msg_frame(openlcb_msg, &worker_can_msg, &payload_index);

                    break;

            }

        } else {

             _handle_unaddressed_msg_frame(openlcb_msg, &worker_can_msg, &payload_index);

        }

    }
    
    return true;

}

bool CanTxStatemachine_try_transmit_can_message(can_msg_t* can_msg) {

    return _transmit_can_frame(can_msg);

}
