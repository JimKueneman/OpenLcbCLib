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
 * @file bootloader_can_driver.h
 *
 * CAN-specific hardware driver interface. These extern functions are
 * implemented in application_drivers/ alongside the BootloaderDriver_*
 * functions. Only used by the CAN transport layer (bootloader_can.c).
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN_DRIVER__
#define __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN_DRIVER__

#include <stdint.h>
#include <stdbool.h>

#include "bootloader_can.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     *     Polls for an incoming CAN frame.
     *
     *     @param frame will be loaded with the incoming frame
     *     @return true if a frame was received, false if no frame available
     */
    extern bool BootloaderCanDriver_read_can_frame(bootloader_can_frame_t *frame);

    /**
     *     Attempts to send a CAN frame.
     *
     *     @param frame the frame to send
     *     @return true if sent, false if the TX buffer is busy (retry later)
     */
    extern bool BootloaderCanDriver_try_send_can_frame(const bootloader_can_frame_t *frame);

    /**
     *     Returns a suggested CAN alias for this node.
     *
     *     If the application saved its alias before rebooting into the
     *     bootloader, return that alias here for seamless handoff.
     *
     *     @return suggested 12-bit alias, or 0 to auto-generate from node ID
     */
    extern uint16_t BootloaderCanDriver_node_alias(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN_DRIVER__ */
