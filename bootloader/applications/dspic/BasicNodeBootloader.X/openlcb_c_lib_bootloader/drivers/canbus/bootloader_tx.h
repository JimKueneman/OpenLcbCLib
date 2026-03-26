/** @copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * @file bootloader_tx.h
 *
 * CAN transmit engine for the standalone bootloader. Provides non-blocking
 * single frame send and blocking multi-frame addressed message send with
 * RX draining between frames.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_TX__
#define __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_TX__

#include <stdint.h>
#include <stdbool.h>

#include "../../openlcb/bootloader_types.h"
#include "bootloader_can.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     *     Initializes the TX engine.
     *
     *     @param can_driver CAN hardware driver interface
     */
    void BootloaderTx_init(const bootloader_can_driver_t *can_driver);

    /**
     *     Non-blocking single frame send. Attempts to send immediately.
     *
     *     @param frame the CAN frame to send
     *     @return true if sent, false if TX buffer busy (retry later)
     */
    bool BootloaderTx_send_frame(const bootloader_can_frame_t *frame);

    /**
     *     Blocking multi-frame send for addressed messages (SNIP, PIP, OIR).
     *     Constructs proper MULTIFRAME_FIRST/MIDDLE/FINAL framing bits in
     *     data byte 0. Calls BootloaderRx_poll() while spinning to keep the
     *     RX buffer drained during the send.
     *
     *     For messages that fit in a single frame (len <= 6), this sets
     *     MULTIFRAME_ONLY and sends via a single blocking call.
     *
     *     @param mti the message type indicator (12-bit CAN MTI)
     *     @param src_alias our CAN alias (12-bit)
     *     @param dest_alias destination CAN alias (12-bit)
     *     @param our_alias alias to pass to BootloaderRx_poll for filtering
     *     @param current_tick monotonic 100ms tick for BootloaderRx_poll
     *     @param data payload bytes (after the 2-byte address header)
     *     @param len number of payload bytes
     *     @return true if all frames sent
     */
    bool BootloaderTx_send_multiframe(
            uint16_t mti,
            uint16_t src_alias,
            uint16_t dest_alias,
            uint16_t our_alias,
            uint8_t current_tick,
            const uint8_t *data,
            uint8_t len);

    /**
     *     Blocking send for a global (unaddressed) OpenLCB message.
     *     Constructs the CAN ID from the MTI and source alias, copies
     *     payload data directly into the frame.
     *
     *     @param mti the message type indicator (12-bit CAN MTI)
     *     @param src_alias our CAN alias (12-bit)
     *     @param our_alias alias to pass to BootloaderRx_poll for filtering
     *     @param current_tick monotonic 100ms tick for BootloaderRx_poll
     *     @param data payload bytes (up to 8)
     *     @param len number of payload bytes
     *     @return true when sent
     */
    extern bool BootloaderTx_send_global(uint16_t mti, uint16_t src_alias, uint16_t our_alias, uint8_t current_tick, const uint8_t *data, uint8_t len);

    /**
     *     Blocking send for a datagram (single or multi-frame).
     *     Builds DATAGRAM_ONLY for payloads <= 8 bytes, or
     *     DATAGRAM_FIRST / DATAGRAM_MIDDLE / DATAGRAM_FINAL sequence
     *     for larger payloads. Calls BootloaderRx_poll() while spinning.
     *
     *     @param src_alias our CAN alias (12-bit)
     *     @param dest_alias destination CAN alias (12-bit)
     *     @param our_alias alias to pass to BootloaderRx_poll for filtering
     *     @param current_tick monotonic 100ms tick for BootloaderRx_poll
     *     @param data datagram payload bytes
     *     @param len number of payload bytes (max 72)
     *     @return true when sent
     */
    extern bool BootloaderTx_send_datagram(
            uint16_t src_alias,
            uint16_t dest_alias,
            uint16_t our_alias,
            uint8_t current_tick,
            const uint8_t *data,
            uint8_t len);

    /**
     *     Blocking send for Datagram Received OK.
     *     Constructs a single addressed CAN frame with the datagram OK MTI,
     *     destination alias in data[0:1], and flags byte in data[2].
     *
     *     @param src_alias our CAN alias (12-bit)
     *     @param dest_alias destination CAN alias (12-bit)
     *     @param our_alias alias to pass to BootloaderRx_poll for filtering
     *     @param current_tick monotonic 100ms tick for BootloaderRx_poll
     *     @param flags datagram OK flags (e.g. DATAGRAM_OK_REPLY_PENDING)
     *     @return true when sent
     */
    extern bool BootloaderTx_send_datagram_ok(uint16_t src_alias, uint16_t dest_alias, uint16_t our_alias, uint8_t current_tick, uint8_t flags);

    /**
     *     Blocking send for Datagram Rejected.
     *     Constructs a single addressed CAN frame with the datagram rejected
     *     MTI, destination alias in data[0:1], and error code in data[2:3].
     *
     *     @param src_alias our CAN alias (12-bit)
     *     @param dest_alias destination CAN alias (12-bit)
     *     @param our_alias alias to pass to BootloaderRx_poll for filtering
     *     @param current_tick monotonic 100ms tick for BootloaderRx_poll
     *     @param error_code OpenLCB error code (16-bit, big-endian in frame)
     *     @return true when sent
     */
    extern bool BootloaderTx_send_datagram_rejected(uint16_t src_alias, uint16_t dest_alias, uint16_t our_alias, uint8_t current_tick, uint16_t error_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_TX__ */
