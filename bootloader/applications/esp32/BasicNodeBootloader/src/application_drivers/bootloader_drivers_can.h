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
 * @file bootloader_drivers_can.h
 *
 * ESP32 CAN physical-layer driver for the standalone bootloader.
 * Provides the platform-specific TWAI read/write functions that satisfy
 * the bootloader_can_driver_t DI interface.
 *
 * The bootloader runs as a single-threaded polling loop -- no FreeRTOS
 * receive task is created.  twai_receive() and twai_transmit() are called
 * with a zero timeout so they return immediately if no frame is available.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#ifndef __ESP32_BOOTLOADER_DRIVERS_CAN__
#define __ESP32_BOOTLOADER_DRIVERS_CAN__

#include "../openlcb_c_lib_bootloader/openlcb/bootloader_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         *     Polls the TWAI RX queue for a received CAN frame.  If a frame is
         *     available it is copied into the caller's struct.  Only extended
         *     (29-bit) frames are accepted; standard frames are discarded.
         *
         *     Called with a zero timeout (non-blocking).
         *
         *     @param frame  output struct populated on success
         *     @return true if a frame was read, false if the queue was empty
         */
    extern bool BootloaderDriversCan_read_received_frame(bootloader_can_frame_t *frame);

        /**
         *     Attempts to queue a CAN frame for transmission through the TWAI
         *     peripheral.  Returns false if the TX queue is full.
         *
         *     Called with a zero timeout (non-blocking).
         *
         *     @param frame  frame to transmit
         *     @return true if the frame was queued, false if the TX queue was full
         */
    extern bool BootloaderDriversCan_try_send_frame(const bootloader_can_frame_t *frame);

        /**
         *     Returns the CAN alias the application was using before it
         *     dropped back into bootloader mode (read from RTC NOINIT RAM).
         *     Returns 0 if no cached alias is available (cold boot or button
         *     entry), which causes the CAN transport layer to negotiate a new
         *     alias from scratch.
         *
         *     @return 12-bit alias, or 0 for auto-generation
         */
    extern uint16_t BootloaderDriversCan_get_cached_alias_passed_from_application(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESP32_BOOTLOADER_DRIVERS_CAN__ */
