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
 * @file bootloader_rx.h
 *
 * CAN receive engine for the standalone bootloader. Polls hardware, filters
 * frames, buffers non-datagram frames addressed to us, assembles multi-frame
 * datagrams into a complete buffer, flags assembly errors, and tracks alias
 * collision flags.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_RX__
#define __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_RX__

#include <stdint.h>
#include <stdbool.h>

#include "../../openlcb/bootloader_types.h"
#include "bootloader_can.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    #define BOOTLOADER_RX_BUFFER_SIZE 64

    /* ================================================================== */
    /* Datagram assembly error type                                        */
    /* ================================================================== */

    /** Error information from datagram assembly. */
    typedef struct {

        unsigned has_error : 1;
        uint16_t error_code;
        uint16_t error_src_alias;

    } bootloader_rx_error_t;

    /* ================================================================== */
    /* Initialization                                                      */
    /* ================================================================== */

        /**
         *     Initializes the RX engine.
         *
         *     @param can_driver CAN hardware driver interface
         */
    void BootloaderRx_init(const bootloader_can_driver_t *can_driver);

    /* ================================================================== */
    /* Polling                                                             */
    /* ================================================================== */

        /**
         *     Polls hardware for new CAN frames, filters, and buffers them.
         *     Datagram frames are assembled directly into the datagram buffer
         *     and do not enter the frame FIFO.  Non-datagram frames (control,
         *     addressed, global) are buffered in the frame FIFO as before.
         *     Sets collision flags if alias conflicts are detected.
         *     Call this from the main loop and from TX spin loops.
         *
         *     @param our_alias    the current alias to filter against
         *     @param current_tick monotonic 100ms tick for assembly timeout
         */
    void BootloaderRx_poll(uint16_t our_alias, uint8_t current_tick);

    /* ================================================================== */
    /* Non-datagram frame FIFO                                             */
    /* ================================================================== */

        /**
         *     @return true if at least one filtered frame is available
         */
    bool BootloaderRx_has_frame(void);

        /**
         *     Dequeues the oldest filtered frame.
         *
         *     @param frame output frame
         *     @return true if a frame was dequeued, false if buffer empty
         */
    bool BootloaderRx_get_frame(bootloader_can_frame_t *frame);

    /* ================================================================== */
    /* Datagram assembly                                                   */
    /* ================================================================== */

        /**
         *     @return true if a complete datagram has been assembled
         */
    bool BootloaderRx_has_datagram(void);

        /**
         *     Copies the assembled datagram to the caller and clears the flag.
         *
         *     @param src_alias        output: source alias of the datagram sender
         *     @param buffer           output: datagram payload (caller provides 72 bytes)
         *     @param datagram_length  output: payload length
         *     @return true if a datagram was available, false if none pending
         */
    bool BootloaderRx_get_datagram(uint16_t *src_alias, uint8_t *buffer, uint8_t *datagram_length);

    /* ================================================================== */
    /* Assembly error reporting                                            */
    /* ================================================================== */

        /**
         *     @return true if a datagram assembly error occurred
         */
    bool BootloaderRx_has_error(void);

        /**
         *     Returns the pending error and clears the flag.
         *
         *     @return error information (error_code and source alias)
         */
    bootloader_rx_error_t BootloaderRx_get_error(void);

    /* ================================================================== */
    /* Collision flags                                                      */
    /* ================================================================== */

        /**
         *     @return true if a CID collision was detected since last clear
         */
    bool BootloaderRx_collision_cid(void);

        /**
         *     @return true if a non-CID collision was detected since last clear
         */
    bool BootloaderRx_collision_non_cid(void);

        /**
         *     Clears both collision flags.
         */
    void BootloaderRx_clear_collision_flags(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_RX__ */
