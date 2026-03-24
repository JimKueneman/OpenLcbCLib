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
 * @file mspm03507_bootloader_drivers.h
 *
 * Platform-specific defines for the MSPM0G3507 LaunchPad bootloader.
 * Implements the extern functions declared in bootloader_driver.h and
 * bootloader_can_driver.h.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __MSPM03507_BOOTLOADER_DRIVERS__
#define __MSPM03507_BOOTLOADER_DRIVERS__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* ================================================================== */
    /* Flash memory layout                                                 */
    /* ================================================================== */

    /** Bootloader occupies the first 16KB of flash. */
    #define BOOTLOADER_FLASH_START     0x00000000U
    #define BOOTLOADER_FLASH_SIZE      0x00004000U

    /** Application region: 16KB to 127KB. */
    #define APP_FLASH_START            0x00004000U
    #define APP_FLASH_END              0x0001FBFFU

    /** Node ID stored in the last 1KB sector of main flash (production-programmed). */
    #define NODEID_FLASH_ADDRESS       0x0001FC00U

    /** MSPM0G3507 flash sector size: 1KB uniform. */
    #define FLASH_SECTOR_SIZE          1024U

    /** MSPM0G3507 flash write granularity: 8 bytes (64-bit word). */
    #define FLASH_WRITE_ALIGN          8U

    /* ================================================================== */
    /* Application header location                                         */
    /* ================================================================== */

    /**
     *     The app_header sits at a fixed offset within the application image.
     *     On Cortex-M0+, the vector table is at least 48 entries x 4 bytes = 192
     *     bytes. Place the app_header right after the vector table at offset 0xC0
     *     (192 bytes) from the app start.
     */
    #define APP_HEADER_OFFSET          0x000000C0U
    #define APP_HEADER_ADDRESS         (APP_FLASH_START + APP_HEADER_OFFSET)

    /* ================================================================== */
    /* LED pin (PA0, negative logic)                                       */
    /* ================================================================== */

    #define BOOTLOADER_LED_PORT        GPIOA
    #define BOOTLOADER_LED_PIN         DL_GPIO_PIN_0

    /* ================================================================== */
    /* Button pin (PB21, negative logic, needs pull-up)                    */
    /* ================================================================== */

    #define BOOTLOADER_BUTTON_PORT     GPIOB
    #define BOOTLOADER_BUTTON_PIN      DL_GPIO_PIN_21

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MSPM03507_BOOTLOADER_DRIVERS__ */
