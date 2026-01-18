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
 * @brief Implementation of the CAN transmit state machine
 *
 * @details Orchestrates transmission of OpenLCB messages by checking hardware buffer
 * availability and dispatching to appropriate message type handlers. Manages multi-frame
 * message sequencing to ensure atomic transmission of message sequences.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
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

    /**
     * @brief Initializes the CAN transmit state machine
     *
     * @details Algorithm:
     * -# Cast away const qualifier from interface pointer
     * -# Store interface pointer in static module variable
     *
     * Use cases:
     * - Called once during application initialization
     * - Required before any OpenLCB message or CAN frame transmission
     *
     * @verbatim
     * @param interface_can_tx_statemachine Pointer to interface structure containing required function pointers
     * @endverbatim
     *
     * @warning MUST be called during application initialization before transmission begins
     * @warning NOT thread-safe - call only from main initialization context
     *
     * @attention Call after handler modules initialized but before network traffic starts
     *
     * @see CanTxMessageHandler_initialize - Initialize message handlers first
     */
void CanTxStatemachine_initialize(const interface_can_tx_statemachine_t *interface_can_tx_statemachine) {

    _interface = (interface_can_tx_statemachine_t*) interface_can_tx_statemachine;

}

    /**
     * @brief Routes OpenLCB message to appropriate handler based on type
     *
     * @details Algorithm:
     * -# Check if message is addressed:
     *    - If addressed:
     *      - Check MTI for specific types:
     *        - If MTI_DATAGRAM: route to datagram handler
     *        - If MTI_STREAM_*: route to stream handler
     *        - Otherwise: route to addressed message handler
     *    - If unaddressed:
     *      - Route to unaddressed message handler
     * -# Return handler result
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message to transmit
     * @param worker_can_msg Pointer to working CAN frame buffer
     * @param payload_index Pointer to current position in payload
     * @endverbatim
     *
     * @return True if frame transmitted successfully, false otherwise
     *
     * @attention Message type determined by addressing and MTI
     * @attention Datagram and stream messages use special frame formats
     *
     * @see CanTxMessageHandler_addressed_msg_frame - Addressed message handler
     * @see CanTxMessageHandler_datagram_frame - Datagram handler
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
     * @brief Transmits an OpenLCB message on the CAN physical layer
     *
     * @details Algorithm:
     * -# Allocate working CAN message buffer on stack
     * -# Initialize payload index to 0
     * -# Check if transmit buffer is empty:
     *    - If not empty: return false immediately
     * -# Check if message has payload:
     *    - If no payload (count == 0):
     *      - Transmit single frame
     *      - Return result
     * -# If has payload:
     *    - Transmit first frame
     *    - If transmission fails: return false
     *    - While payload index < total payload count:
     *      - Transmit next frame (will be middle or last)
     *      - Continue until all payload transmitted
     *    - Return true
     * -# Return false if initial transmission failed
     *
     * Use cases:
     * - Sending any OpenLCB protocol message
     * - Transmitting events, datagrams, configuration data
     * - Broadcasting node status information
     *
     * @verbatim
     * @param openlcb_msg Pointer to OpenLCB message to transmit
     * @endverbatim
     *
     * @return True if message fully transmitted, false if hardware buffer full or transmission failed
     *
     * @warning Returns false immediately if transmit buffer not empty - caller must retry
     * @warning Blocks until entire multi-frame message transmitted or failure occurs
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention Multi-frame messages sent as atomic sequence - no interruption by same/lower priority
     * @attention Message must have valid MTI and addressing information
     *
     * @see CanTxStatemachine_send_can_message - For raw CAN frames
     * @see CanTxMessageHandler_addressed_msg_frame - Addressed message handler
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

    /**
     * @brief Transmits a raw CAN frame on the physical layer
     *
     * @details Algorithm:
     * -# Call interface handle_can_frame function
     * -# Return transmission result
     *
     * Use cases:
     * - Transmitting alias allocation frames during node login
     * - Sending CAN control messages
     * - Direct hardware-level CAN operations
     *
     * @verbatim
     * @param can_msg Pointer to CAN message buffer containing frame to transmit
     * @endverbatim
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Frame must be fully constructed before calling
     * @warning No buffer availability check performed - caller responsible
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention No OpenLCB processing - raw CAN transmission only
     *
     * @see CanTxStatemachine_send_openlcb_message - For OpenLCB messages
     */
bool CanTxStatemachine_send_can_message(can_msg_t* can_msg) {

    return _interface->handle_can_frame(can_msg);

}
