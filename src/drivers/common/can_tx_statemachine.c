
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
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_utilities.h"


interface_can_tx_statemachine_t *_interface_can_tx_statemachine;

void CanTxStatemachine_initialize(const interface_can_tx_statemachine_t *interface_can_tx_statemachine) {

    _interface_can_tx_statemachine = (interface_can_tx_statemachine_t*) interface_can_tx_statemachine;

}

bool _transmit_openlcb_message(openlcb_msg_t* openlcb_msg, can_msg_t *worker_can_msg, uint16_t *payload_index) {


    if (OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg)) {

        switch (openlcb_msg->mti) {

            case MTI_DATAGRAM:

                return _interface_can_tx_statemachine->handle_datagram_frame(openlcb_msg, worker_can_msg, payload_index);

                break;

            case MTI_STREAM_COMPLETE:
            case MTI_STREAM_INIT_REPLY:
            case MTI_STREAM_INIT_REQUEST:
            case MTI_STREAM_PROCEED:

                return _interface_can_tx_statemachine->handle_stream_frame(openlcb_msg, worker_can_msg, payload_index);

                break;

            default:
                
                return _interface_can_tx_statemachine->handle_addressed_msg_frame(openlcb_msg, worker_can_msg, payload_index);

                break;

        }

    } else {

        return _interface_can_tx_statemachine->handle_unaddressed_msg_frame(openlcb_msg, worker_can_msg, payload_index);

    }

}

bool CanTxStatemachine_transmit_openlcb_message(openlcb_msg_t* openlcb_msg) {

    if (!_interface_can_tx_statemachine->is_tx_buffer_empty()) {

        return false;

    }

    can_msg_t worker_can_msg;
    uint16_t payload_index = 0;
    
    if (openlcb_msg->payload_count == 0) {
        
        return _transmit_openlcb_message(openlcb_msg, &worker_can_msg, &payload_index);
        
    }

    while (payload_index < openlcb_msg->payload_count) {
        
        if (!_transmit_openlcb_message(openlcb_msg, &worker_can_msg, &payload_index)) {
            
            return false;
            
        };

    }

    return true;

}

bool CanTxStatemachine_transmit_can_message(can_msg_t* can_msg) {

    return _interface_can_tx_statemachine->handle_can_frame(can_msg);

}
