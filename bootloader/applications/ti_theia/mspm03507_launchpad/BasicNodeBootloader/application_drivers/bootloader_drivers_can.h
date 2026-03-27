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
 * MSPM0G3507 CAN physical-layer driver for the standalone bootloader.
 * Provides the platform-specific MCAN read/write functions that satisfy
 * the bootloader_can_driver_t DI interface.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __MSPM03507_BOOTLOADER_DRIVERS_CAN__
#define __MSPM03507_BOOTLOADER_DRIVERS_CAN__

#include "../openlcb_c_lib_bootloader/openlcb/bootloader_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         *     Polls the MCAN RX FIFO 1 for a received CAN frame.  If a frame is
         *     available it is copied into the caller's struct and the FIFO read
         *     index is acknowledged.
         *
         *     @param frame  output struct populated on success
         *     @return true if a frame was read, false if the FIFO was empty
         */
    extern bool BootloaderDriversCan_read_received_frame(bootloader_can_frame_t *frame);

        /**
         *     Attempts to transmit a CAN frame through the MCAN TX buffer.
         *     Uses buffer 0 if free, falls back to buffer 1, or returns false
         *     when both buffers are pending.
         *
         *     @param frame  frame to transmit
         *     @return true if the frame was queued, false if no TX buffer was free
         */
    extern bool BootloaderDriversCan_try_send_frame(const bootloader_can_frame_t *frame);

        /**
         *     Returns the pre-assigned CAN alias for this node.  Returns 0
         *     to let the CAN transport layer generate its own alias during
         *     initialization.
         *
         *     @return 12-bit alias, or 0 for auto-generation
         */
    extern uint16_t BootloaderDriversCan_get_cached_alias_passed_from_application(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MSPM03507_BOOTLOADER_DRIVERS_CAN__ */
