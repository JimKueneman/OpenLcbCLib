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
 * dsPIC33EP512MC506 hardware driver for the standalone bootloader OpenLCB layer.
 * Provides flash operations, lifecycle control, LED, timer, identity, and
 * checksum functions that satisfy the bootloader_openlcb_driver_t DI interface.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#ifndef __DSPIC33EP512MC506_BOOTLOADER_DRIVERS_OPENLCB__
#define __DSPIC33EP512MC506_BOOTLOADER_DRIVERS_OPENLCB__

#include "../openlcb_bootloader_src/openlcb/bootloader_types.h"

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
     *     (-D NO_CHECKSUM) -- but note that defining it only here affects
     *     only this driver file; bootloader_types.h is the recommended
     *     location so that bootloader.c also sees the flag.
     */
    /* #define NO_CHECKSUM */

    /* ================================================================== */
    /* Flash memory layout -- dsPIC33EP512MC506                            */
    /* ================================================================== */

    /*
     * dsPIC33EP512MC506 flash: ~536 KB, PC addresses 0x000000–0x042FFE.
     * Each instruction word is 24 bits, accessed at 2 consecutive PC
     * addresses (every even PC address holds one instruction).
     *
     * Flash erase/write unit (from MCC flash.h):
     *   FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS = 1024 instructions/page
     *   FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS      = 2048 PC units/page (0x800)
     *
     * Bootloader occupies the first 5 pages (0x000000–0x0027FE, inclusive).
     * Adjust APP_FLASH_START if the bootloader grows beyond 5 pages.
     */
    #define BOOTLOADER_FLASH_START     0x000000UL
    #define BOOTLOADER_FLASH_PAGES     5U                 /* pages reserved for bootloader */

    /*
     * Application region starts at page 5 (PC address 0x2800).
     *
     * *** 16-bit uintptr_t limitation (XC16) ***
     * XC16 defines uintptr_t as 16-bit (sizeof(void*) == 2 on dsPIC33).
     * The bootloader library casts flash addresses through uintptr_t, so
     * addresses above 0xFFFF are silently truncated.  APP_FLASH_END is
     * therefore capped at 0xFFFE -- the highest page-aligned PC address
     * that fits without truncation.  This provides approximately 54 KB
     * of firmware image space (31 pages × 4096 image-bytes/page).
     * To support larger applications, update the library to use uint32_t
     * instead of uintptr_t when passing flash addresses.
     */
    #define APP_FLASH_START            0x002800UL   /* PC addr: first app page           */
    #define APP_FLASH_END              0x00FFFEUL   /* PC addr: 16-bit uintptr_t ceiling */

    /*
     * dsPIC33 instruction: 24 bits of data + 8 phantom bits (always 0).
     * The firmware image binary encodes each instruction as 4 bytes:
     *   byte[0] = instruction[7:0]
     *   byte[1] = instruction[15:8]
     *   byte[2] = instruction[23:16]
     *   byte[3] = 0x00  (phantom -- ignored on read, zero on write)
     *
     * Smallest write unit: 2 instructions = 8 image bytes = 4 PC units
     * (FLASH_WriteDoubleWord24 writes exactly 2 instructions per call).
     */
    #define FLASH_WRITE_ALIGN          8U    /* image bytes per FLASH_WriteDoubleWord24 call */

    /* ================================================================== */
    /* Application header location                                         */
    /* ================================================================== */

    /*
     * dsPIC33 reset vector occupies the first 2 instructions at
     * APP_FLASH_START (a goto to __reset).  Place the app_header
     * immediately after, at PC address APP_FLASH_START + 0x04
     * (2nd instruction slot = 4 PC units in).
     */
    #define APP_HEADER_OFFSET          0x00000004UL
    #define APP_HEADER_ADDRESS         (APP_FLASH_START + APP_HEADER_OFFSET)

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
     *     @return BOOTLOADER_NOT_REQUESTED      -- normal boot
     *             BOOTLOADER_REQUESTED_BY_APP    -- app set magic flag
     *             BOOTLOADER_REQUESTED_BY_BUTTON -- hardware button held
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
    extern void BootloaderDriversOpenlcb_get_flash_boundaries(const void **flash_min, const void **flash_max, const struct bootloader_app_header **app_header);

    /**
     *     Returns the start address and size of the flash page that
     *     contains the given PC address.  dsPIC33EP pages are uniform
     *     (1024 instructions = 4096 image bytes).
     *
     *     @param address            PC address within the page
     *     @param page_start         output: page-aligned PC address
     *     @param page_length_bytes  output: page size in image bytes (4096)
     */
    extern void BootloaderDriversOpenlcb_get_flash_page_info(const void *address, const void **page_start, uint32_t *page_length_bytes);

    /**
     *     Erases the flash page that contains the given PC address.
     *     Uses FLASH_Unlock / FLASH_ErasePage / FLASH_Lock from MCC flash.h.
     *
     *     @param address  PC address within the page to erase
     *     @return 0 on success, ERROR_PERMANENT on failure
     */
    extern uint16_t BootloaderDriversOpenlcb_erase_flash_page(const void *address);

    /**
     *     Programs flash using FLASH_WriteDoubleWord24.  Writes 2
     *     instructions (8 image bytes) per call; any trailing sub-8-byte
     *     remainder is padded with 0xFF (erased flash value).
     *
     *     @param address     destination PC address (must be 4-PC-unit aligned)
     *     @param data        source firmware image data
     *     @param size_bytes  number of image bytes to write
     *     @return 0 on success, ERROR_PERMANENT on failure
     */
    extern uint16_t BootloaderDriversOpenlcb_write_flash_bytes(const void *address, const void *data, uint32_t size_bytes);

    /**
     *     Called after a write sequence completes.  Returns 0 immediately
     *     (dsPIC33 flash writes are synchronous via MCC flash driver).
     *
     *     @return 0 on success
     */
    extern uint16_t BootloaderDriversOpenlcb_finalize_flash(compute_checksum_func_t compute_checksum_helper);

    /* ================================================================== */
    /* Flash read                                                          */
    /* ================================================================== */

    /**
     *     Reads bytes from program flash into a RAM buffer using dsPIC33
     *     table-read instructions (FLASH_ReadWord24 / tblrdl+tblrdh).
     *
     *     Each FLASH_ReadWord24 call provides 3 data bytes (bits[23:0]) from
     *     one 24-bit instruction word; the address advances by 2 PC units per
     *     instruction.  Bits[31:24] of the hardware return value are undefined
     *     and are not used.
     *
     *     @param flash_addr  PC address of the first instruction to read
     *     @param dest_ram    destination RAM buffer
     *     @param size_bytes  number of data bytes to fill in dest_ram
     */
    extern void BootloaderDriversOpenlcb_read_flash_bytes(uint32_t flash_addr, void *dest_ram, uint32_t size_bytes);

    /* ================================================================== */
    /* Checksum                                                            */
    /* ================================================================== */

    /**
     *     Computes a triple-CRC16-IBM checksum over a flash region using
     *     FLASH_ReadWord24 table reads.
     *
     *     On dsPIC33 the data pointer passed from the library is actually
     *     a PC address stored as a const void*.  size_bytes is the PC-unit
     *     span of the region (from pointer arithmetic in the library);
     *     num_instructions = size_bytes / 2.
     *
     *     @param data      PC address of the flash region (as const void*)
     *     @param size      PC-unit span of the region
     *     @param checksum  output: four uint32_t values (three CRCs + zero pad)
     */
    extern void BootloaderDriversOpenlcb_compute_checksum(const void *data, uint32_t size, uint32_t *checksum);

    /* ================================================================== */
    /* Timer                                                               */
    /* ================================================================== */

    /**
     *     Returns a value that increments every 100ms.  Reads the MCC
     *     TMR2 software counter (TMR2_SoftwareCounterGet()) and casts to
     *     uint8_t.  TMR2 must be configured with a 100 ms period.
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

#endif /* __DSPIC33EP512MC506_BOOTLOADER_DRIVERS_OPENLCB__ */
