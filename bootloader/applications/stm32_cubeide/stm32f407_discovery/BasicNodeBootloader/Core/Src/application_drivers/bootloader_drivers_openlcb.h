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
 * STM32F407 hardware driver for the standalone bootloader OpenLCB layer.
 * Provides flash operations, lifecycle control, LED, timer, identity, and
 * checksum functions that satisfy the bootloader_openlcb_driver_t DI interface.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#ifndef __STM32F407_BOOTLOADER_DRIVERS_OPENLCB__
#define __STM32F407_BOOTLOADER_DRIVERS_OPENLCB__

#include "../openlcb_c_lib_bootloader/openlcb/bootloader_types.h"

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
    /* Flash memory layout — STM32F407VG (1MB flash)                      */
    /* ================================================================== */

    /*
     * Bootloader occupies sectors 0–1 (2 x 16KB = 32KB).
     * Internal flash base on STM32F407 is 0x08000000.
     */
    #define BOOTLOADER_FLASH_START     0x08000000U
    #define BOOTLOADER_FLASH_SIZE      0x00008000U   /* 32KB */

    /*
     * Application region: sectors 2–10 (0x08008000 – 0x080DFFFF).
     * Sector 11 (0x080E0000 – 0x080FFFFF, 128KB) is reserved for node ID
     * storage and must not be overwritten by firmware images.
     */
    #define APP_FLASH_START            0x08008000U
    #define APP_FLASH_END              0x080DFFFFU

    /*
     * Node ID stored in the last 128KB sector (sector 11).
     * TODO: move to a dedicated sector or EEPROM emulation region once
     * production programming is defined.
     */
    #define NODEID_FLASH_ADDRESS       0x080E0000U

    /*
     * STM32F407 has non-uniform sectors: 4x16KB, 1x64KB, 7x128KB.
     * FLASH_SECTOR_SIZE is NOT a constant — get_flash_page_info() must
     * compute the correct sector size dynamically from the address.
     * This placeholder is provided only so code that references it still
     * compiles; do not use it for erase/write calculations.
     */
    #define FLASH_SECTOR_SIZE          16384U   /* smallest sector = 16KB */

    /*
     * STM32F407 flash write granularity: 4 bytes (word-mode programming).
     * HAL_FLASH_Program is called with FLASH_TYPEPROGRAM_WORD.
     */
    #define FLASH_WRITE_ALIGN          4U

    /* ================================================================== */
    /* Application header location                                         */
    /* ================================================================== */

    /*
     * Cortex-M4 vector table: up to 98 entries x 4 bytes = 392 bytes.
     * Place the app_header right after the vector table at offset 0x200
     * (512 bytes) from the app start, giving plenty of headroom.
     */
    #define APP_HEADER_OFFSET          0x00000200U
    #define APP_HEADER_ADDRESS         (APP_FLASH_START + APP_HEADER_OFFSET)

    /* ================================================================== */
    /* Lifecycle                                                           */
    /* ================================================================== */

        /**
         *     Resets hardware to a known state and initializes all peripherals
         *     needed by the bootloader (clocks, CAN, GPIOs, SysTick).
         */
    extern void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request);

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
         *     @details De-initialises CAN1, stops SysTick, and clears all NVIC interrupt enables
         *     and pending flags so the target binary starts in a known clean state.
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
    extern void BootloaderDriversOpenlcb_get_flash_boundaries(uint32_t *flash_min, uint32_t *flash_max, uint32_t *app_header);

        /**
         *     Returns the start address and size of the flash sector that
         *     contains the given address.  STM32F407 has non-uniform sectors
         *     (16KB, 64KB, or 128KB) so the size varies by address.
         *
         *     @param address            address within the sector
         *     @param page_start         output: sector-aligned start address
         *     @param page_length_bytes  output: sector size in bytes
         */
    extern void BootloaderDriversOpenlcb_get_flash_page_info(uint32_t address, uint32_t *page_start, uint32_t *page_length_bytes);

        /**
         *     Erases the flash sector that contains the given address.
         *     Sector size is 16KB, 64KB, or 128KB depending on address.
         *
         *     @param address  any address within the sector to erase
         */
    extern uint16_t BootloaderDriversOpenlcb_erase_flash_page(uint32_t address);

        /**
         *     Programs flash in 4-byte (word) aligned writes using
         *     HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD).  Data shorter than
         *     the alignment granularity is padded with 0xFF.
         *
         *     @param address     destination address in flash (must be 4-byte aligned)
         *     @param data        source data to write
         *     @param size_bytes  number of bytes to write
         */
    extern uint16_t BootloaderDriversOpenlcb_write_flash_bytes(uint32_t address, const uint8_t *data, uint32_t size_bytes);

        /**
         *     Reads bytes from flash into a RAM buffer.
         *
         *     STM32F407 is a Von Neumann architecture: flash is memory-mapped
         *     at 0x08000000 and is readable via a normal data pointer.  This
         *     implementation is therefore a simple memcpy().
         *
         *     Contrast with the dsPIC33 implementation, which must use
         *     FLASH_ReadWord24() (TBLRDL/TBLRDH table-read instructions) because
         *     dsPIC33 is Harvard: program flash and SRAM are separate address
         *     spaces and a data pointer cannot legally address flash.
         *
         *     The bootloader library always calls this DI function instead of
         *     dereferencing flash pointers directly, so the same bootloader.c
         *     source compiles and runs correctly on both architectures.
         *
         *     @param flash_addr   source address in flash (physical address,
         *                         e.g. 0x08008000 for start of app region)
         *     @param dest_ram     destination buffer in SRAM
         *     @param size_bytes   number of bytes to copy
         */
    extern void BootloaderDriversOpenlcb_read_flash_bytes(uint32_t flash_addr, void *dest_ram, uint32_t size_bytes);

        /**
         *     Called after a write sequence completes.  STM32F407 flash writes
         *     are synchronous so this returns 0 immediately.
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
    extern void BootloaderDriversOpenlcb_compute_checksum(uint32_t flash_addr, uint32_t size, uint32_t *checksum);

    /* ================================================================== */
    /* Timer                                                               */
    /* ================================================================== */

        /**
         *     Returns a value that increments every 100ms.  Derived from
         *     HAL_GetTick() / 100 and cast to uint8_t (wraps at 256).
         *
         *     @return 100ms tick count (wraps at 255)
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
         *     Drives the bootloader status LEDs (positive logic).
         *     Pin assignments follow the CubeMX names in main.h.
         *
         *     @param led    which logical LED function is being set
         *     @param value  true = LED on, false = LED off
         */
    extern void BootloaderDriversOpenlcb_set_status_led(bootloader_led_enum led, bool value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32F407_BOOTLOADER_DRIVERS_OPENLCB__ */
