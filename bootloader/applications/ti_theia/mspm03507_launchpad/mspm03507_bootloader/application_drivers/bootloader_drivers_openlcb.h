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
 * @file bootloader_drivers_openlcb.h
 *
 * MSPM0G3507 hardware driver for the standalone bootloader OpenLCB layer.
 * Provides flash operations, lifecycle control, LED, timer, identity, and
 * checksum functions that satisfy the bootloader_openlcb_driver_t DI interface.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __MSPM03507_BOOTLOADER_DRIVERS_OPENLCB__
#define __MSPM03507_BOOTLOADER_DRIVERS_OPENLCB__

#include "../src/openlcb/bootloader_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* ================================================================== */
    /* User configuration                                                  */
    /* ================================================================== */

    /**
     *     To bypass checksum validation, uncomment NO_CHECKSUM in
     *     bootloader_types.h.  That header is included by all library
     *     files, so a single edit enables the bypass everywhere.
     *
     *     Alternatively, define it here or as a compiler flag
     *     (-D NO_CHECKSUM) — but note that defining it only here affects
     *     only this driver file; bootloader_types.h is the recommended
     *     location so that bootloader.c also sees the flag.
     */
    /* #define NO_CHECKSUM */

    /* ================================================================== */
    /* Flash memory layout                                                 */
    /* ================================================================== */

    /** Bootloader occupies the first 15KB of flash. */
    #define BOOTLOADER_FLASH_START     0x00000000U
    #define BOOTLOADER_FLASH_SIZE      0x00003C00U

    /** Application region: 15KB to 127KB. */
    #define APP_FLASH_START            0x00003C00U
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

    /* ================================================================== */
    /* Lifecycle                                                           */
    /* ================================================================== */

    /**
     *     Resets hardware to a known state and initializes all peripherals
     *     needed by the bootloader (clocks, CAN, GPIOs, SysTick).
     */
    extern void BootloaderDriversOpenlcb_initialize_hardware(void);

    /**
     *     Checks whether the bootloader should stay in bootloader mode.
     *
     *     @return BOOTLOADER_NOT_REQUESTED      — normal boot
     *             BOOTLOADER_REQUESTED_BY_APP    — app set magic flag
     *             BOOTLOADER_REQUESTED_BY_BUTTON — hardware button held
     */
    extern bootloader_request_t BootloaderDriversOpenlcb_is_bootloader_requested(void);

    /**
     *     Disables interrupts, relocates the vector table to the application
     *     region, sets the stack pointer, and jumps to the application reset
     *     handler.  Does not return.
     */
    extern void BootloaderDriversOpenlcb_jump_to_application(void);

    /**
     *     Triggers an NVIC system reset.  Does not return.
     */
    extern void BootloaderDriversOpenlcb_reboot(void);

    /**
     *     @brief Tears down all peripherals and ARM core state before handing off to the other binary.
     *
     *     @details On MSPM0 this stops SysTick, disables and clears all NVIC interrupts, and resets the MCAN peripheral.
     */
    extern void BootloaderDriversOpenlcb_cleanup_before_handoff(void);

    /* ================================================================== */
    /* Flash operations                                                    */
    /* ================================================================== */

    /**
     *     Returns the application flash boundaries and the location of
     *     the embedded app_header struct.
     *
     *     @param flash_min   output: start of writable application flash
     *     @param flash_max   output: last byte of writable application flash
     *     @param app_header  output: pointer to the app_header in flash
     */
    extern void BootloaderDriversOpenlcb_get_flash_boundaries(const void **flash_min, const void **flash_max, const struct bootloader_app_header **app_header);

    /**
     *     Returns the start address and size of the 1KB flash sector that
     *     contains the given address.
     *
     *     @param address            address within the sector
     *     @param page_start         output: sector-aligned start address
     *     @param page_length_bytes  output: sector size in bytes (1024)
     */
    extern void BootloaderDriversOpenlcb_get_flash_page_info(const void *address, const void **page_start, uint32_t *page_length_bytes);

    /**
     *     Erases the 1KB flash sector that contains the given address.
     *
     *     @param address  any address within the sector to erase
     */
    extern uint16_t BootloaderDriversOpenlcb_erase_flash_page(const void *address);

    /**
     *     Programs flash in 8-byte aligned writes.  Data shorter than the
     *     alignment granularity is padded with 0xFF.
     *
     *     @param address     destination address in flash (must be 8-byte aligned)
     *     @param data        source data to write
     *     @param size_bytes  number of bytes to write
     */
    extern uint16_t BootloaderDriversOpenlcb_write_flash(const void *address, const void *data, uint32_t size_bytes);

    /**
     *     Called after a write sequence completes.  Returns 0 on the
     *     MSPM0G3507 (flash writes are synchronous).
     *
     *     @return 0 on success
     */
    extern uint16_t BootloaderDriversOpenlcb_finalize_flash(compute_checksum_func_t compute_checksum_helper);

    /* ================================================================== */
    /* Checksum                                                            */
    /* ================================================================== */

    /**
     *     Computes a triple-CRC16-IBM checksum over a block of data.
     *
     *     @param data      pointer to the data to checksum
     *     @param size      number of bytes
     *     @param checksum  output: four uint32_t values (three CRCs + zero pad)
     */
    extern void BootloaderDriversOpenlcb_compute_checksum(const void *data, uint32_t size, uint32_t *checksum);

    /* ================================================================== */
    /* Timer                                                               */
    /* ================================================================== */

    /**
     *     Returns the current SysTick counter value.  Incremented by the
     *     SysTick_Handler ISR.
     *
     *     @return monotonic tick count (wraps at 255)
     */
    extern uint8_t BootloaderDriversOpenlcb_get_100ms_timer_tick(void);

    /* ================================================================== */
    /* Identity                                                            */
    /* ================================================================== */

    /**
     *     Returns the 48-bit OpenLCB node ID for this board.
     *
     *     @return node ID (upper 16 bits zero)
     */
    extern uint64_t BootloaderDriversOpenlcb_get_persistent_node_id(void);

    /* ================================================================== */
    /* LED                                                                 */
    /* ================================================================== */

    /**
     *     Drives the bootloader status LED (PA0, negative logic).
     *
     *     @param led    which logical LED function is being set
     *     @param value  true = LED on, false = LED off
     */
    extern void BootloaderDriversOpenlcb_set_status_led(bootloader_led_enum led, bool value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MSPM03507_BOOTLOADER_DRIVERS_OPENLCB__ */
