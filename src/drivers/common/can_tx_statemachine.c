
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
 * necessary, else it packs up the CAN frame from the message struture and send it
 * to the CAN Driver to transmit on the physical layer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_tx_statemachine.h"

#include "stdio.h" // printf
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_utilities.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "can_utilities.h"
#include "can_types.h"
#include "../driver_can.h"

#include "../../applications/test/can_tx_statemachine/can_tx_statemachine.X/debug.h"


// #define CAN_TX_TEST

void CanTxStatemachine_initialize() {


}

uint32_t _construct_identfier_datagram_only_frame(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_ONLY | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t _construct_identfier_datagram_first_frame(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FIRST | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t _construct_identfier_datagram_middle_frame(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_MIDDLE | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t construct_identfier_datagram_last_frame(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FINAL | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t Construct_Identfier_for_unaddress_msg(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

uint32_t _construct_addressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

void _transmit_can_frame(can_msg_t* can_msg) {

#ifdef CAN_TX_TEST 
    
    printf("\n");
    PrintCanMsg(can_msg);
    printf("\n\n");
    
#else
    
    DriverCan_transmit_raw_can_frame(TX_CHANNEL_OPENLCB_MSG, can_msg);
    
#endif

}

void _datagram_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_only_frame(openlcb_msg);
    _transmit_can_frame(can_msg_worker);

}

void _datagram_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_first_frame(openlcb_msg);
    _transmit_can_frame(can_msg_worker);
    
}

void _datagram_middle_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_middle_frame(openlcb_msg);
    _transmit_can_frame(can_msg_worker);
    
}

void _datagram_last_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = construct_identfier_datagram_last_frame(openlcb_msg);
    _transmit_can_frame(can_msg_worker);

}

void _addressed_message_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    Utilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_ONLY);
    _transmit_can_frame(can_msg);

}

void _addressed_message_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    Utilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FIRST);
    _transmit_can_frame(can_msg);

}

void _addressed_message_middle(can_msg_t* can_msg) {

    Utilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_MIDDLE);
    _transmit_can_frame(can_msg);

}

void _addressed_message_last(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    Utilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FINAL);
    _transmit_can_frame(can_msg);

}

void _load_destination_address_in_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    can_msg->payload[0] = (openlcb_msg->dest_alias >> 8) & 0xFF; // Setup the first two CAN data bytes with the destination address
    can_msg->payload[1] = openlcb_msg->dest_alias & 0xFF;

}

uint16_t _handle_datagram(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t openlcb_start_index) {

    uint16_t result = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, openlcb_start_index, OFFSET_CAN_WITHOUT_DEST_ADDRESS);

    if (openlcb_msg->payload_count <= LEN_CAN_BYTE_ARRAY)

        _datagram_only_frame(openlcb_msg, can_msg_worker);

    else if (openlcb_start_index < LEN_CAN_BYTE_ARRAY)

        _datagram_first_frame(openlcb_msg, can_msg_worker);

    else if (openlcb_start_index + result < openlcb_msg->payload_count)

        _datagram_middle_frame(openlcb_msg, can_msg_worker);

    else

        _datagram_last_frame(openlcb_msg, can_msg_worker);

    return result;

}

uint16_t _handle_unaddressed_message(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t openlcb_start_index) {

    uint16_t result = 0;

    if (openlcb_msg->payload_count <= LEN_CAN_BYTE_ARRAY) { // single frame

        can_msg_worker->identifier = Construct_Identfier_for_unaddress_msg(openlcb_msg);
        result = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, openlcb_start_index, OFFSET_CAN_WITHOUT_DEST_ADDRESS);
        _transmit_can_frame(can_msg_worker);

    } else { // multi frame

        // TODO: Is there such a thing as a unaddressed multi frame?

    }
    
    return result;

}

uint16_t _handle_addressed_message(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t openlcb_start_index) {

    _load_destination_address_in_payload(openlcb_msg, can_msg_worker);


    can_msg_worker->identifier = _construct_addressed_message_identifier(openlcb_msg);

    uint16_t result = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, openlcb_start_index, OFFSET_CAN_WITH_DEST_ADDRESS);


    if (openlcb_msg->payload_count <= 6) // Account for 2 bytes used for dest alias

        _addressed_message_only_frame(openlcb_msg, can_msg_worker);

    else if (openlcb_start_index < 6) // Account for 2 bytes used for dest alias

        _addressed_message_first_frame(openlcb_msg, can_msg_worker);

    else if ((openlcb_start_index + result) < openlcb_msg->payload_count)

        _addressed_message_middle(can_msg_worker);

    else

        _addressed_message_last(openlcb_msg, can_msg_worker);

    return result;
}

uint16_t CanTxStatemachine_try_transmit_openlcb_message(can_msg_t* can_msg_worker, openlcb_msg_t* openlcb_msg, uint16_t openlcb_start_index) {

    uint16_t result = 0;
    
    if (!DriverCan_is_can_tx_buffer_clear(TX_CHANNEL_CAN_CONTROL))

        return result;


    if (Utilities_is_addressed_openlcb_message(openlcb_msg)) {

        switch (openlcb_msg->mti) {

            case MTI_DATAGRAM:
            {
                result = _handle_datagram(openlcb_msg, can_msg_worker, openlcb_start_index);
                break;

            }

            case MTI_STREAM_COMPLETE:
            case MTI_STREAM_INIT_REPLY:
            case MTI_STREAM_INIT_REQUEST:
            case MTI_STREAM_PROCEED:
            {
                
                break;

            }

            default:

                result = _handle_addressed_message(openlcb_msg, can_msg_worker, openlcb_start_index);
                
                break;

        }

    } else {

        result = _handle_unaddressed_message(openlcb_msg, can_msg_worker, openlcb_start_index);

    }

    return result;

}

uint8_t CanTxStatemachine_try_transmit_can_message(can_msg_t* can_msg) {

    if (DriverCan_is_can_tx_buffer_clear(TX_CHANNEL_CAN_CONTROL)) {

        DriverCan_transmit_raw_can_frame(TX_CHANNEL_CAN_CONTROL, can_msg);

        return TRUE;

    }

    return FALSE;

}
