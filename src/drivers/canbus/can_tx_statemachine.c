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
 * @details Routes outgoing OpenLCB messages to the correct frame handler
 * (addressed, unaddressed, datagram, or stream) and loops until the full
 * payload is transmitted as an atomic multi-frame sequence.  Also provides
 * a pass-through path for pre-built raw CAN control frames.
 *
 * @author Jim Kueneman
 * @date 4 Mar 2026
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

#ifdef OPENLCB_COMPILE_TRAIN
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#endif


/** @brief Saved pointer to the dependency-injected transmit interface. */
static interface_can_tx_statemachine_t *_interface;

    /** @brief Stores the dependency-injection interface pointer. */
void CanTxStatemachine_initialize(const interface_can_tx_statemachine_t *interface_can_tx_statemachine) {

    _interface = (interface_can_tx_statemachine_t*) interface_can_tx_statemachine;

}

#ifdef OPENLCB_COMPILE_TRAIN

    /**
     * @brief Sniffs outgoing Listener Config Reply messages and synchronizes
     *        the listener alias table.
     *
     * @details The CAN layer watches for outgoing Train Control Reply messages
     * that carry Listener Config Attach/Detach results. On a successful attach,
     * it registers the listener's node_id in the alias table and immediately
     * sends a targeted AME to resolve the alias. On a successful detach, it
     * unregisters the listener.
     *
     * This keeps alias table management entirely in the CAN layer where it
     * belongs — the protocol layer does not need to know about CAN aliases.
     *
     * Reply payload layout:
     *   Byte 0: 0x30 (TRAIN_LISTENER_CONFIG)
     *   Byte 1: 0x01 (ATTACH) or 0x02 (DETACH)
     *   Bytes 2-7: Listener Node ID (6 bytes, big-endian)
     *   Byte 8: Result code (0x00 = success, non-zero = fail)
     */
static void _sniff_listener_config_reply(openlcb_msg_t *msg) {

    if (!_interface->listener_register) { return; }

    if (msg->mti != MTI_TRAIN_REPLY) { return; }

    if (msg->payload_count < 9) { return; }

    uint8_t cmd = *msg->payload[0];

    if (cmd != TRAIN_LISTENER_CONFIG) { return; }

    uint8_t sub_cmd = *msg->payload[1];
    uint8_t result = *msg->payload[8];

    if (result != 0) { return; }

    // Extract listener node_id from bytes 2-7 (big-endian)
    node_id_t listener_id =
            ((uint64_t) *msg->payload[2] << 40) |
            ((uint64_t) *msg->payload[3] << 32) |
            ((uint64_t) *msg->payload[4] << 24) |
            ((uint64_t) *msg->payload[5] << 16) |
            ((uint64_t) *msg->payload[6] << 8) |
            ((uint64_t) *msg->payload[7]);

    if (sub_cmd == TRAIN_LISTENER_ATTACH) {

        _interface->listener_register(listener_id);

        // Immediately send a targeted AME to resolve the new listener's alias
        if (_interface->lock_shared_resources && _interface->unlock_shared_resources) {

            _interface->lock_shared_resources();
            can_msg_t *ame = CanBufferStore_allocate_buffer();
            _interface->unlock_shared_resources();

            if (ame) {

                ame->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AME | msg->source_alias;
                ame->payload_count = 6;
                CanUtilities_copy_node_id_to_payload(ame, listener_id, 0);

                _interface->lock_shared_resources();
                CanBufferFifo_push(ame);
                _interface->unlock_shared_resources();

            }

        }

    } else if (sub_cmd == TRAIN_LISTENER_DETACH && _interface->listener_unregister) {

        _interface->listener_unregister(listener_id);

    }

}

#endif

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
     * -# Discard immediately if state.invalid is set (AMR scrub marked it).
     * -# If dest_alias == 0 and dest_id != 0, resolve the alias via
     *    listener_find_by_node_id (DI, nullable). Drop the message if
     *    unresolvable (return true so the caller clears the outgoing slot).
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

    if (openlcb_msg->state.invalid) {

        return true;

    }

    // Resolve listener alias via DI if needed (forwarded consist commands)
    if (openlcb_msg->dest_alias == 0 && openlcb_msg->dest_id != 0
            && _interface->listener_find_by_node_id) {

        listener_alias_entry_t *entry =
                _interface->listener_find_by_node_id(openlcb_msg->dest_id);

        if (entry && entry->alias != 0) {

            openlcb_msg->dest_alias = entry->alias;

        } else {

            return true;  // alias unresolvable — drop message, don't retry

        }

    }

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

#ifdef OPENLCB_COMPILE_TRAIN
        _sniff_listener_config_reply(openlcb_msg);
#endif

        return true;

    }

    return false;

}

    /** @brief Transmits a pre-built raw @ref can_msg_t via the hardware handler. */
bool CanTxStatemachine_send_can_message(can_msg_t* can_msg) {

    return _interface->handle_can_frame(can_msg);

}
