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
 * ESP32 hardware driver for the standalone bootloader OpenLCB layer.
 * Provides flash operations, lifecycle control, LED, timer, identity, and
 * checksum functions that satisfy the bootloader_openlcb_driver_t DI interface.
 *
 * Flash model:
 *   The inactive OTA partition (ota_0 or ota_1, whichever is not currently
 *   running) is used as the write target.  get_flash_boundaries() returns the
 *   memory-mapped DROM address range of that partition so the bootloader
 *   library sees a flat address space identical to every other port.
 *   erase_flash_page() and write_flash_bytes() translate the mapped address
 *   back to a partition-relative offset before calling the esp_partition API.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#ifndef __ESP32_BOOTLOADER_DRIVERS_OPENLCB__
#define __ESP32_BOOTLOADER_DRIVERS_OPENLCB__

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
         */
    /* #define NO_CHECKSUM */

    /* ================================================================== */
    /* Flash memory layout -- ESP32 OTA                                    */
    /* ================================================================== */

    /*
     * ESP32 SPI flash is memory-mapped (read-only) via the MMU into the
     * DROM region starting at 0x3F400000.  Physical SPI flash offset 0
     * maps to virtual address 0x3F400000.
     *
     * For a given OTA partition at physical offset P:
     *   mapped_start = ESP32_FLASH_MMAP_BASE + P
     *
     * Writes and erases must use the esp_partition API with a partition-
     * relative byte offset:
     *   part_offset = mapped_addr - ESP32_FLASH_MMAP_BASE - partition->address
     *
     * Erase granularity is one 4 KB sector.  Writes must be 4-byte aligned.
     *
     * APP_FLASH_START and APP_FLASH_END are computed dynamically in
     * get_flash_boundaries() from esp_ota_get_next_update_partition() --
     * they are NOT compile-time constants on ESP32.
     */

        /** Base of the DROM memory-mapped flash window on ESP32. */
    #define ESP32_FLASH_MMAP_BASE      0x3F400000UL

        /** Erase granularity: one SPI flash sector (4 KB). */
    #define ESP32_FLASH_SECTOR_SIZE    4096U

        /**
         *     Minimum write alignment required by esp_partition_write.
         *     Writes must be a multiple of 4 bytes.
         */
    #define FLASH_WRITE_ALIGN          4U

    /* ================================================================== */
    /* Application header location                                         */
    /* ================================================================== */

    /*
     * Byte offset within the firmware.bin where the 36-byte
     * bootloader_app_header_t struct is embedded.
     *
     * ESP-IDF image layout:
     *   0x00   esp_image_header_t          (24 bytes)
     *   0x18   esp_image_segment_header_t  (8 bytes)
     *   0x20   esp_app_desc_t              (256 bytes, ends at 0x11F)
     *   0x120  .rodata_custom_desc         <-- app_header lives here
     *
     * The application's app_header.c places the struct into the
     * .rodata_custom_desc linker section.  The post-link tool
     * (hex2bin.py --arch esp32) patches the CRC fields at this offset
     * and recomputes the ESP-IDF image checksum and SHA256 hash.
     */
    #define APP_HEADER_OFFSET          0x00000120UL

    /* ================================================================== */
    /* Board pin assignments -- ESP32 DevKit V1                            */
    /* ================================================================== */

        /** GPIO for the built-in blue LED (positive logic). */
    #define BOOTLOADER_LED_PIN         2

        /** GPIO for the BOOT button (active LOW, held at reset to enter bootloader). */
    #define BOOTLOADER_BOOT_BUTTON_PIN 0

    /* ================================================================== */
    /* Lifecycle                                                           */
    /* ================================================================== */

        /**
         *     Checks the reset reason and clears shared RTC RAM on power-on or
         *     brownout.  Initialises the TWAI (CAN) peripheral.
         */
    extern void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request);

        /**
         *     Checks whether the bootloader should stay in bootloader mode.
         *
         *     @return BOOTLOADER_NOT_REQUESTED      -- normal boot
         *             BOOTLOADER_REQUESTED_BY_APP    -- app set RTC RAM magic flag
         *             BOOTLOADER_REQUESTED_BY_BUTTON -- BOOT button held at reset
         */
    extern bootloader_request_t BootloaderDriversOpenlcb_is_bootloader_requested(void);

        /**
         *     Sets the application OTA partition as the next boot target and
         *     calls esp_restart().  Does not return.
         *
         *     Works for both cold-boot (valid app, no update) and post-update
         *     paths.  The ROM second-stage bootloader performs the actual
         *     partition switch on the next reset.
         */
    extern void BootloaderDriversOpenlcb_jump_to_application(void);

        /**
         *     Calls esp_restart().  Does not return.
         */
    extern void BootloaderDriversOpenlcb_reboot(void);

        /**
         *     Stops and uninstalls the TWAI driver before handing off.
         */
    extern void BootloaderDriversOpenlcb_cleanup_before_handoff(void);

    /* ================================================================== */
    /* Flash operations                                                    */
    /* ================================================================== */

        /**
         *     Returns the memory-mapped address range of the inactive OTA
         *     partition and the location of the embedded app_header struct.
         *     Caches the partition pointer used by all subsequent flash calls.
         *
         *     @param flash_min   output: mapped start of the update partition
         *     @param flash_max   output: mapped last byte of the update partition
         *     @param app_header  output: pointer to the app_header in the partition
         */
    extern void BootloaderDriversOpenlcb_get_flash_boundaries(uint32_t *flash_min, uint32_t *flash_max, uint32_t *app_header);

        /**
         *     Returns the start address and size of the 4 KB sector that
         *     contains the given mapped address.
         *
         *     @param address            mapped address within the sector
         *     @param page_start         output: sector-aligned mapped address
         *     @param page_length_bytes  output: sector size (4096)
         */
    extern void BootloaderDriversOpenlcb_get_flash_page_info(uint32_t address, uint32_t *page_start, uint32_t *page_length_bytes);

        /**
         *     Erases the 4 KB sector that contains the given mapped address
         *     using esp_partition_erase_range().
         *
         *     @param address  mapped address within the sector to erase
         *     @return 0 on success, ERROR_PERMANENT on failure
         */
    extern uint16_t BootloaderDriversOpenlcb_erase_flash_page(uint32_t address);

        /**
         *     Writes firmware bytes to the update partition using
         *     esp_partition_write().  Writes are rounded up to FLASH_WRITE_ALIGN
         *     (4 bytes); any trailing remainder is padded with 0xFF.
         *
         *     @param address     mapped destination address (4-byte aligned)
         *     @param data        source firmware image data
         *     @param size_bytes  number of bytes to write
         *     @return 0 on success, ERROR_PERMANENT on failure
         */
    extern uint16_t BootloaderDriversOpenlcb_write_flash_bytes(uint32_t address, const uint8_t *data, uint32_t size_bytes);

        /**
         *     Called after all firmware data has been written.  Verifies
         *     the triple-CRC checksum embedded at APP_HEADER_OFFSET, then
         *     calls esp_ota_set_boot_partition() to make the update
         *     partition the next boot target.
         *
         *     @return 0 on success, ERROR_PERMANENT on failure
         */
    extern uint16_t BootloaderDriversOpenlcb_finalize_flash(compute_checksum_func_t compute_checksum_helper);

    /* ================================================================== */
    /* Flash read                                                          */
    /* ================================================================== */

        /**
         *     Reads bytes from the inactive OTA partition via
         *     esp_partition_read().  The inactive partition is NOT
         *     memory-mapped, so memcpy cannot be used.
         *
         *     @param flash_addr  mapped address of the first byte to read
         *     @param dest_ram    destination RAM buffer
         *     @param size_bytes  number of bytes to copy
         */
    extern void BootloaderDriversOpenlcb_read_flash_bytes(uint32_t flash_addr, void *dest_ram, uint32_t size_bytes);

    /* ================================================================== */
    /* Checksum                                                            */
    /* ================================================================== */

        /**
         *     Computes a triple-CRC16-IBM checksum over a flash region.
         *     Reads the inactive partition in chunks via esp_partition_read()
         *     and accumulates CRCs incrementally.
         *
         *     @param flash_addr  mapped start address of the flash region
         *     @param size        number of bytes in the region
         *     @param checksum    output: four uint32_t values (three CRCs + zero pad)
         */
    extern void BootloaderDriversOpenlcb_compute_checksum(uint32_t flash_addr, uint32_t size, uint32_t *checksum);

    /* ================================================================== */
    /* Timer                                                               */
    /* ================================================================== */

        /**
         *     Returns a value that increments every 100 ms.
         *     Derived from Arduino millis() / 100, cast to uint8_t (wraps at 255).
         *
         *     @return 100 ms tick count
         */
    extern uint8_t BootloaderDriversOpenlcb_get_100ms_timer_tick(void);

    /* ================================================================== */
    /* Identity                                                            */
    /* ================================================================== */

        /**
         *     Returns the 48-bit OpenLCB node ID for this board.
         *     Reads from NVS (namespace "openlcb", key "node_id").
         *     Falls back to a hardcoded default if NVS is not programmed.
         *
         *     @return node ID (upper 16 bits zero)
         */
    extern uint64_t BootloaderDriversOpenlcb_get_persistent_node_id(void);

    /* ================================================================== */
    /* LED                                                                 */
    /* ================================================================== */

        /**
         *     Drives the bootloader status LED on BOOTLOADER_LED_PIN (GPIO 2).
         *
         *     @param led    which logical LED function is being set
         *     @param value  true = LED on, false = LED off
         */
    extern void BootloaderDriversOpenlcb_set_status_led(bootloader_led_enum led, bool value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESP32_BOOTLOADER_DRIVERS_OPENLCB__ */
