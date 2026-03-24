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
 * @file bootloader_driver.h
 *
 * Hardware driver interface for the standalone bootloader.
 * These extern functions must be implemented by each platform in
 * application_drivers/. They are shared across all transport layers.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_DRIVER__
#define __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_DRIVER__

#include <stdint.h>
#include <stdbool.h>

#include "../crc/bootloader_app_header.h"
#include "bootloader_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* ================================================================== */
    /* Lifecycle                                                           */
    /* ================================================================== */

    /**
     *     Initializes hardware to a safe state. Called before data/bss init.
     *     Must disable interrupts and set all outputs to safe values.
     */
    extern void BootloaderDriver_hw_set_to_safe(void);

    /**
     *     Initializes processor, clock, peripherals, CAN, LEDs.
     *     Called after data/bss segments are initialized.
     */
    extern void BootloaderDriver_hw_init(void);

    /**
     *     Checks whether the bootloader should be entered.
     *
     *     This function encapsulates all entry checks: GPIO button, magic word
     *     in RAM, app-initiated reboot flag, etc.
     *
     *     @return true if the bootloader should run, false to try the application
     */
    extern bool BootloaderDriver_request_bootloader(void);

    /**
     *     Enters the application. Never returns.
     *
     *     Typically jumps to the application reset vector.
     */
    extern void BootloaderDriver_application_entry(void);

    /**
     *     Resets the microcontroller. Never returns.
     */
    extern void BootloaderDriver_reboot(void);

    /* ================================================================== */
    /* Flash operations                                                    */
    /* ================================================================== */

    /**
     *     Returns the boundaries of the user-writable flash region.
     *
     *     @param flash_min set to first valid byte of writable flash
     *     @param flash_max set to last valid byte of writable flash
     *     @param app_header set to pointer to the app_header in flash
     */
    extern void BootloaderDriver_get_flash_boundaries(
            const void **flash_min,
            const void **flash_max,
            const bootloader_app_header_t **app_header);

    /**
     *     Returns the flash page containing a given address.
     *
     *     @param address the address to query
     *     @param page_start set to the first byte of the page
     *     @param page_length_bytes set to the page size in bytes
     */
    extern void BootloaderDriver_get_flash_page_info(
            const void *address,
            const void **page_start,
            uint32_t *page_length_bytes);

    /**
     *     Erases the flash page at a given address. Blocks until complete.
     *
     *     The implementation must handle reset vector protection if the page
     *     contains the reset vector.
     *
     *     @param address start of the page (from get_flash_page_info)
     */
    extern void BootloaderDriver_erase_flash_page(const void *address);

    /**
     *     Writes data to flash. Blocks until complete.
     *
     *     The implementation must handle reset vector protection if writing
     *     to the page containing the reset vector.
     *
     *     @param address destination address (aligned to platform requirements)
     *     @param data source buffer
     *     @param size_bytes number of bytes to write (multiple of platform word size)
     */
    extern void BootloaderDriver_write_flash(
            const void *address,
            const void *data,
            uint32_t size_bytes);

    /**
     *     Called when firmware upload is complete (UNFREEZE).
     *
     *     The platform can perform final validation here (e.g., verify CRC).
     *
     *     @return 0 on success, or an OpenLCB error code on failure
     */
    extern uint16_t BootloaderDriver_flash_complete(void);

    /* ================================================================== */
    /* Checksum                                                            */
    /* ================================================================== */

    /**
     *     Computes a 128-bit checksum over a block of data.
     *
     *     The default implementation calls BootloaderCrc_crc3_crc16_ibm().
     *     Platforms with hardware CRC engines can override this function.
     *
     *     @param data pointer to data to checksum
     *     @param size number of bytes
     *     @param checksum output array of BOOTLOADER_CHECKSUM_COUNT uint32_t values
     */
    extern void BootloaderDriver_checksum_data(
            const void *data,
            uint32_t size,
            uint32_t *checksum);

    /* ================================================================== */
    /* Identity                                                            */
    /* ================================================================== */

    /* ================================================================== */
    /* Timer                                                                */
    /* ================================================================== */

    /**
     *     Returns a monotonic tick counter (incremented by SysTick ISR).
     *     Each tick is 100 ms. Used for alias negotiation 200 ms wait.
     *
     *     @return current tick value (wraps at uint8_t max)
     */
    extern uint8_t BootloaderDriver_timer_tick(void);

    /* ================================================================== */
    /* Identity                                                            */
    /* ================================================================== */

    /**
     *     Returns the 48-bit OpenLCB node ID for this hardware.
     *
     *     @return node ID as a uint64_t (upper 16 bits unused)
     */
    extern uint64_t BootloaderDriver_node_id(void);

    /* ================================================================== */
    /* LED                                                                 */
    /* ================================================================== */

    /**
     *     Sets a status LED.
     *
     *     @param led the logical LED function
     *     @param value true to turn on, false to turn off
     */
    extern void BootloaderDriver_led(bootloader_led_enum led, bool value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_DRIVER__ */
