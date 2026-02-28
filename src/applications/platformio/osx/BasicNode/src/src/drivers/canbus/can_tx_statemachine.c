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
 * @file can_tx_statemachine.c
 * @brief Implementation of the CAN transmit state machine.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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


static interface_can_tx_statemachine_t *_interface;

    /** @brief Stores the dependency-injection interface pointer. */
void CanTxStatemachine_initialize(const interface_can_tx_statemachine_t *interface_can_tx_statemachine) {

    _interface = (interface_can_tx_statemachine_t*) interface_can_tx_statemachine;

}

    /**
     * @brief Routes an OpenLCB message to its appropriate CAN frame handler.
     *
     * @details Algorithm:
     * -# If the message is addressed: dispatch datagrams to datagram handler,
     *    stream MTIs to stream handler, all others to addressed_msg_frame.
     * -# If global: dispatch to unaddressed_msg_frame.
     * -# Return the handler's result.
     *
     * @verbatim
     * @param openlcb_msg   Message to transmit.
     * @param worker_can_msg Scratch CAN frame buffer.
     * @param payload_index  Current payload position; updated on success.
     * @endverbatim
     *
     * @return true if a frame was transmitted, false on hardware failure.
     */
static bool _transmit_openlcb_message(openlcb_msg_t* openlcb_msg, can_msg_t *worker_can_msg, uint16_t *payload_index) {


    if (OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg)) {

        switch (openlcb_msg->mti) {

            case MTI_DATAGRAM:

                return _interface->handle_datagram_frame(openlcb_msg, worker_can_msg, payload_index);

                break;

            case MTI_STREAM_COMPLETE:
            case MTI_STREAM_INIT_REPLY:
            case MTI_STREAM_INIT_REQUEST:
            case MTI_STREAM_PROCEED:

                return _interface->handle_stream_frame(openlcb_msg, worker_can_msg, payload_index);

                break;

            default:

                return _interface->handle_addressed_msg_frame(openlcb_msg, worker_can_msg, payload_index);

                break;

        }

    } else {

        return _interface->handle_unaddressed_msg_frame(openlcb_msg, worker_can_msg, payload_index);

    }

}

    /**
     * @brief Transmits a complete OpenLCB message, blocking until all frames are sent.
     *
     * @details Algorithm:
     * -# Return false immediately if the TX hardware buffer is busy.
     * -# If payload_count == 0: send a single zero-payload frame and return.
     * -# Otherwise, send the first frame; if it fails return false.
     * -# Loop calling _transmit_openlcb_message until payload_index == payload_count.
     * -# Return true when done.
     *
     * @verbatim
     * @param openlcb_msg Message to transmit.
     * @endverbatim
     *
     * @return true if the full message was sent, false if the hardware buffer was busy or failed.
     *
     * @warning Blocks until the entire multi-frame message is sent.
     */
bool CanTxStatemachine_send_openlcb_message(openlcb_msg_t* openlcb_msg) {

    can_msg_t worker_can_msg;
    uint16_t payload_index = 0;

    if (!_interface->is_tx_buffer_empty()) {

        return false;

    }

    if (openlcb_msg->payload_count == 0) {

        return _transmit_openlcb_message(openlcb_msg, &worker_can_msg, &payload_index);

    }

    if (_transmit_openlcb_message(openlcb_msg, &worker_can_msg, &payload_index)) {

        while (payload_index < openlcb_msg->payload_count) {                    // stall until everything is sent

            _transmit_openlcb_message(openlcb_msg, &worker_can_msg, &payload_index);

        }

        return true;

    }

    return false;

}

    /** @brief Transmits a pre-built raw @ref can_msg_t via the hardware handler. */
bool CanTxStatemachine_send_can_message(can_msg_t* can_msg) {

    return _interface->handle_can_frame(can_msg);

}
