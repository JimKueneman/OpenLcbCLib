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
 * @file bootloader.h
 *
 * Public API for the standalone bootloader.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_OPENLCB_BOOTLOADER__
#define __BOOTLOADER_LIB_OPENLCB_BOOTLOADER__

#include <stdbool.h>

#include "bootloader_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         *     Main entry point for the bootloader. Never returns.
         *
         *     Initializes hardware, checks whether to enter bootloader or jump
         *     to the application, then runs the bootloader main loop.
         *
         *     @param can_driver     CAN physical layer driver interface
         *     @param openlcb_driver hardware driver interface
         */
    extern void Bootloader_entry(const bootloader_can_driver_t *can_driver, const bootloader_openlcb_driver_t *openlcb_driver);

        /**
         *     Initializes the bootloader state and transport.
         *
         *     @param can_driver     CAN physical layer driver interface
         *     @param openlcb_driver hardware driver interface
         *     @param request        why the bootloader was entered (from
         *                           is_bootloader_requested(), called once by
         *                           Bootloader_entry before initialize_hardware)
         *     @return true if the application should be started (valid checksum,
         *             bootloader not requested), false if the bootloader should run
         */
    extern bool Bootloader_init(const bootloader_can_driver_t *can_driver, const bootloader_openlcb_driver_t *openlcb_driver, bootloader_request_t request);

        /**
         *     Runs one iteration of the bootloader main loop.
         *
         *     @return true if the board should reboot, false to keep running
         */
    extern bool Bootloader_loop(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_OPENLCB_BOOTLOADER__ */
