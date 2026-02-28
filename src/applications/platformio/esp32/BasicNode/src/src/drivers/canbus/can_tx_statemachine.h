/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file can_tx_statemachine.h
 * @brief Orchestrates CAN frame transmission for all OpenLCB message types.
 *
 * @details Checks hardware buffer availability, selects the correct message-type
 * handler, and manages multi-frame sequencing until the full payload is sent.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_TX_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_TX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Dependency-injection interface for the CAN transmit state machine.
     *
     * @details All 6 pointers are REQUIRED (must not be NULL).
     *
     * @see CanTxStatemachine_initialize
     */
    typedef struct {

        /** @brief REQUIRED. Query hardware TX buffer availability. Typical: application CAN driver function. */
        bool (*is_tx_buffer_empty)(void);

        /** @brief REQUIRED. Transmit one frame of an addressed OpenLCB message. Typical: CanTxMessageHandler_addressed_msg_frame. */
        bool (*handle_addressed_msg_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /** @brief REQUIRED. Transmit one frame of an unaddressed OpenLCB message. Typical: CanTxMessageHandler_unaddressed_msg_frame. */
        bool (*handle_unaddressed_msg_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /** @brief REQUIRED. Transmit one frame of a datagram message. Typical: CanTxMessageHandler_datagram_frame. */
        bool (*handle_datagram_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /** @brief REQUIRED. Transmit one frame of a stream message (placeholder). Typical: CanTxMessageHandler_stream_frame. */
        bool (*handle_stream_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /** @brief REQUIRED. Transmit a pre-built raw CAN frame. Typical: CanTxMessageHandler_can_frame. */
        bool (*handle_can_frame)(can_msg_t *can_msg);

    } interface_can_tx_statemachine_t;


    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface_can_tx_statemachine Pointer to a populated
     *        @ref interface_can_tx_statemachine_t. Must remain valid for the
     *        lifetime of the application. All 6 pointers must be non-NULL.
     *
     * @warning NOT thread-safe - call during single-threaded initialization only.
     *
     * @see CanTxMessageHandler_initialize - initialize first
     */
    extern void CanTxStatemachine_initialize(const interface_can_tx_statemachine_t *interface_can_tx_statemachine);

    /**
     * @brief Converts and transmits an @ref openlcb_msg_t as one or more CAN frames.
     *
     * @details Returns false immediately if the hardware TX buffer is not empty.
     * Determines message type (addressed / unaddressed / datagram / stream), then
     * loops until the entire payload is transmitted as an atomic multi-frame sequence.
     *
     * @param openlcb_msg  OpenLCB message to transmit. Must not be NULL.
     *
     * @return true when the full message is transmitted, false if the TX buffer was busy
     *         or a hardware error occurred.
     *
     * @warning May block briefly while transmitting multi-frame messages.
     * @warning NOT thread-safe - serialize with other callers.
     *
     * @see CanTxStatemachine_send_can_message - for raw CAN frames
     */
    extern bool CanTxStatemachine_send_openlcb_message(openlcb_msg_t *openlcb_msg);

    /**
     * @brief Transmits a pre-built raw @ref can_msg_t directly to the hardware.
     *
     * @details No OpenLCB processing and no buffer-availability check — caller is
     * responsible for ensuring the hardware is ready.  Used primarily for CAN control
     * frames (CID, RID, AMD) during alias allocation.
     *
     * @param can_msg  Fully-constructed CAN frame. Must not be NULL.
     *
     * @return true on success, false on hardware failure.
     *
     * @see CanTxStatemachine_send_openlcb_message - for OpenLCB messages
     */
    extern bool CanTxStatemachine_send_can_message(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TX_STATEMACHINE__ */
