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
 * @file can_tx_message_handler.h
 * @brief Message handlers that convert OpenLCB messages to outgoing CAN frames.
 *
 * @details Handles fragmentation and framing-bit encoding for addressed, unaddressed,
 * datagram, stream, and raw CAN frame types per the OpenLCB CAN Frame Transfer Standard.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_TX_MESSAGE_HANDLER__
#define __DRIVERS_CANBUS_CAN_TX_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

    /**
     * @brief Dependency-injection interface for the CAN transmit message handlers.
     *
     * @details Provides the hardware transmit callback (REQUIRED) and an optional
     * post-transmit notification callback.
     *
     * @see CanTxMessageHandler_initialize
     */
typedef struct {

        /**
         * @brief REQUIRED.  Write a fully-built @ref can_msg_t to the CAN controller.
         *
         * @details Called after the frame is constructed.  The Tx state machine
         * pre-checks buffer availability via is_tx_buffer_empty, so this should
         * normally succeed.
         *
         * @return true on success, false on hardware error.
         *
         * @note Must not be NULL.
         */
        bool (*transmit_can_frame)(can_msg_t *can_msg);

        /**
         * @brief OPTIONAL.  Called immediately after a successful transmission.
         *
         * @details Useful for counters, LEDs, or protocol analysers.
         * Must execute quickly (microseconds).  May be NULL.
         */
        void (*on_transmit)(can_msg_t *can_msg);

} interface_can_tx_message_handler_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface_can_tx_message_handler Pointer to a populated
     *        @ref interface_can_tx_message_handler_t. Must remain valid for the
     *        lifetime of the application.
     *
     * @warning NOT thread-safe - call during single-threaded initialization only.
     * @warning transmit_can_frame must not be NULL.
     *
     * @see CanTxStatemachine_initialize
     */
    extern void CanTxMessageHandler_initialize(const interface_can_tx_message_handler_t *interface_can_tx_message_handler);

    /**
     * @brief Transmits one CAN frame for an addressed @ref openlcb_msg_t.
     *
     * @details Encodes the destination alias into payload bytes 0-1 with framing flags
     * (ONLY / FIRST / MIDDLE / LAST), copies up to 6 payload bytes, and transmits.
     * On success, advances *openlcb_start_index by the number of bytes sent.
     *
     * @param openlcb_msg          Source message (dest_alias must be valid).
     * @param can_msg_worker       Scratch CAN frame buffer used to build the frame.
     * @param openlcb_start_index  Current position in the OpenLCB payload; updated on success.
     *
     * @return true if the frame was transmitted, false on hardware failure.
     *
     * @see CanTxMessageHandler_unaddressed_msg_frame
     */
    extern bool CanTxMessageHandler_addressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

    /**
     * @brief Transmits one CAN frame for an unaddressed (broadcast) @ref openlcb_msg_t.
     *
     * @details All 8 payload bytes are available (no destination alias overhead).
     * Multi-frame unaddressed messages are not currently implemented.
     * On success, advances *openlcb_start_index by the number of bytes sent.
     *
     * @param openlcb_msg          Source message.
     * @param can_msg_worker       Scratch CAN frame buffer.
     * @param openlcb_start_index  Current payload position; updated on success.
     *
     * @return true if the frame was transmitted, false on hardware failure.
     *
     * @see CanTxMessageHandler_addressed_msg_frame
     */
    extern bool CanTxMessageHandler_unaddressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

    /**
     * @brief Transmits one CAN frame for a datagram @ref openlcb_msg_t (up to 72 bytes total).
     *
     * @details Selects the correct datagram frame type (ONLY / FIRST / MIDDLE / LAST)
     * based on the current payload index and total size.  On success, advances
     * *openlcb_start_index by the number of bytes sent.
     *
     * @param openlcb_msg          Source datagram message.
     * @param can_msg_worker       Scratch CAN frame buffer.
     * @param openlcb_start_index  Current payload position; updated on success.
     *
     * @return true if the frame was transmitted, false on hardware failure.
     *
     * @see CanTxMessageHandler_addressed_msg_frame
     */
    extern bool CanTxMessageHandler_datagram_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

    /**
     * @brief Placeholder for stream protocol transmit (not yet implemented).
     *
     * @param openlcb_msg          Source stream message.
     * @param can_msg_worker       Scratch CAN frame buffer.
     * @param openlcb_start_index  Current payload position.
     *
     * @return Always true (no-op placeholder).
     *
     * @warning Do not use in production until implemented.
     */
    extern bool CanTxMessageHandler_stream_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

    /**
     * @brief Transmits a pre-built raw @ref can_msg_t directly to the hardware.
     *
     * @details No OpenLCB processing — frame must be fully constructed before calling.
     * Used for CAN control frames (CID, RID, AMD) during alias allocation.
     *
     * @param can_msg  Fully-constructed CAN frame to transmit.
     *
     * @return true if the frame was transmitted, false on hardware failure.
     *
     * @see CanLoginMessageHandler_state_load_cid07
     * @see CanLoginMessageHandler_state_load_rid
     * @see CanLoginMessageHandler_state_load_amd
     */
    extern bool CanTxMessageHandler_can_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TX_MESSAGE_HANDLER__ */
