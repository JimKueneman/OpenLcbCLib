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
 * @file bootloader_drivers_openlcb.c
 *
 * ESP32 hardware driver implementation for the standalone bootloader OpenLCB
 * layer.  Implements flash operations, lifecycle control, LED, timer, identity,
 * and checksum functions.  All functions are public so main.cpp can wire them
 * into the bootloader_openlcb_driver_t DI struct.
 *
 * Flash model:
 *   The inactive OTA partition is used as the write target.
 *   get_flash_boundaries() resolves and caches the partition pointer.
 *   erase_flash_page() and write_flash_bytes() translate the mapped address
 *   back to a partition-relative offset:
 *     part_offset = mapped_addr - ESP32_FLASH_MMAP_BASE - partition->address
 *   finalize_flash() calls esp_ota_set_boot_partition() to commit the update.
 *   jump_to_application() calls esp_restart() -- the ROM second-stage
 *   bootloader performs the actual partition switch.
 *
 * Shared RAM:
 *   bootloader_request_flag and bootloader_cached_alias are placed in RTC
 *   slow memory using RTC_NOINIT_ATTR.  This region survives both software
 *   reset and deep sleep, giving the same magic-value handshake behaviour as
 *   the .noinit section on STM32 and Cortex-M targets.
 *
 *   TODO: move these definitions into a shared/ folder (shared between the
 *   bootloader project and the application project) once the BasicNode
 *   application side is implemented, following the same pattern as the STM32
 *   port.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include <string.h>

#include "Arduino.h"
#include "esp_attr.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "driver/gpio.h"
#include "driver/twai.h"

#include "bootloader_drivers_openlcb.h"
#include "../../shared/bootloader_shared_ram.h"
#include "../openlcb_c_lib_bootloader/crc/bootloader_crc.h"

/* ====================================================================== */
/* Shared RTC RAM -- survives software reset and deep sleep               */
/* ====================================================================== */

/*
 * bootloader_request_flag and bootloader_cached_alias are declared extern in
 * bootloader_shared_ram.h and DEFINED here with RTC_NOINIT_ATTR so they
 * survive esp_restart().  Both the bootloader and the application project
 * compile this file; the ESP-IDF linker script already provides the
 * rtc_noinit segment so no linker script changes are needed.
 *
 * See ../../shared/bootloader_shared_ram.c for platform porting notes.
 */
RTC_NOINIT_ATTR volatile uint32_t bootloader_request_flag;
RTC_NOINIT_ATTR volatile uint16_t bootloader_cached_alias;

/* ====================================================================== */
/* OTA partition cache                                                     */
/* ====================================================================== */

/*
 * Resolved once by get_flash_boundaries() and reused by every subsequent
 * flash call.  NULL until get_flash_boundaries() has been called.
 */
static const esp_partition_t *_ota_partition = NULL;

/* ====================================================================== */
/* Lifecycle                                                               */
/* ====================================================================== */

void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request) {

    /* Centralized .noinit shared RAM cleanup.
     * bootloader_request_flag was already consumed by is_bootloader_requested()
     * before this function runs; clear it unconditionally as belt-and-suspenders.
     * bootloader_cached_alias is only meaningful on app drop-back -- the library
     * CAN state machine reads it via get_cached_alias() during INIT_PICK_ALIAS
     * and clears it after pickup.  On any other reset path it is stale garbage. */
    bootloader_request_flag = 0;

    if (request != BOOTLOADER_REQUESTED_BY_APP) {

        bootloader_cached_alias = 0;

    }

    /*
     * Initialise the TWAI (CAN) peripheral in normal mode at 125 kbit/s.
     * TX = GPIO 21, RX = GPIO 22 -- same pin assignment as the BasicNode
     * application so a single hardware build works for both binaries.
     *
     * Accept all frames; the bootloader protocol layer filters by OpenLCB
     * alias and MTI.
     */
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
    twai_timing_config_t  t_config = TWAI_TIMING_CONFIG_125KBITS();
    twai_filter_config_t  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    twai_driver_install(&g_config, &t_config, &f_config);
    twai_start();

    /*
     * Configure the boot button (GPIO 0, active LOW) as an input with
     * internal pull-up so it reads HIGH when not pressed.
     */
    gpio_set_direction((gpio_num_t) BOOTLOADER_BOOT_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t) BOOTLOADER_BOOT_BUTTON_PIN, GPIO_PULLUP_ONLY);

    /* Configure the status LED output (GPIO 2). */
    gpio_set_direction((gpio_num_t) BOOTLOADER_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t) BOOTLOADER_LED_PIN, 0);

}

bootloader_request_t BootloaderDriversOpenlcb_is_bootloader_requested(void) {

    /*
     * Check 1: Did the application ask us to enter bootloader mode?
     *
     * The application writes BOOTLOADER_REQUEST_MAGIC into the RTC NOINIT
     * flag and then calls esp_restart().  The RTC slow memory survives the
     * reset, so the magic value is still here when we start up.  The flag
     * is cleared centrally by initialize_hardware(), not here.
     *
     * This tells the library that the CT already sent Freeze to the
     * application, so the bootloader starts with firmware_active = 1
     * (PIP reports Firmware Upgrade Active) and the CT can proceed
     * directly to data transfer without sending Freeze again.
     */
    if (bootloader_request_flag == BOOTLOADER_REQUEST_MAGIC) {

        /* Flag is cleared centrally by initialize_hardware(). */
        return BOOTLOADER_REQUESTED_BY_APP;

    }

    /*
     * Check 2: Is the BOOT button (GPIO 0, active LOW) held at reset?
     *
     * Hold it during power-on to force bootloader mode even if no
     * application has requested it via the magic RAM flag.
     * The bootloader starts as a normal node (firmware_active = 0)
     * and the CT must send Freeze before transferring firmware.
     */
    if (gpio_get_level((gpio_num_t) BOOTLOADER_BOOT_BUTTON_PIN) == 0) {

        return BOOTLOADER_REQUESTED_BY_BUTTON;

    }

    return BOOTLOADER_NOT_REQUESTED;

}

void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {

    twai_stop();
    twai_driver_uninstall();

}

void BootloaderDriversOpenlcb_jump_to_application(void) {

    /*
     * On ESP32 there is no bare-metal vector-table jump.  The ROM
     * second-stage bootloader selects the active OTA partition on every
     * reset.  finalize_flash() must have already called
     * esp_ota_set_boot_partition() to point at the newly written image.
     * All we do here is trigger the reset.
     *
     * TODO: For the cold-boot path (no update requested, jump straight to
     * the existing application), esp_ota_set_boot_partition() should be
     * called here with the already-valid application partition before
     * restarting.  This requires resolving which slot holds the valid app,
     * which depends on the partition layout agreed between the bootloader
     * and application projects.
     */
    esp_restart();

    while (1) { }  /* unreachable -- silence compiler */

}

void BootloaderDriversOpenlcb_reboot(void) {

    esp_restart();

    while (1) { }  /* unreachable -- silence compiler */

}

/* ====================================================================== */
/* Flash operations                                                        */
/* ====================================================================== */

void BootloaderDriversOpenlcb_get_flash_boundaries(uint32_t *flash_min, uint32_t *flash_max, uint32_t *app_header) {

    /*
     * Resolve and cache the inactive OTA partition.
     * esp_ota_get_next_update_partition(NULL) returns whichever OTA slot
     * (ota_0 or ota_1) is NOT the currently running partition.
     */
    _ota_partition = esp_ota_get_next_update_partition(NULL);

    *flash_min  = ESP32_FLASH_MMAP_BASE + _ota_partition->address;
    *flash_max  = ESP32_FLASH_MMAP_BASE + _ota_partition->address + _ota_partition->size - 1U;
    *app_header = *flash_min + APP_HEADER_OFFSET;

}

void BootloaderDriversOpenlcb_get_flash_page_info(uint32_t address, uint32_t *page_start, uint32_t *page_length_bytes) {

    /*
     * ESP32 flash erase granularity is one 4 KB sector.
     * Align the address down to the nearest sector boundary.
     */
    *page_start        = address & ~((uint32_t)(ESP32_FLASH_SECTOR_SIZE - 1U));
    *page_length_bytes = ESP32_FLASH_SECTOR_SIZE;

}

uint16_t BootloaderDriversOpenlcb_erase_flash_page(uint32_t address) {

    uint32_t page_aligned = address & ~(ESP32_FLASH_SECTOR_SIZE - 1U);
    uint32_t part_offset  = page_aligned - ESP32_FLASH_MMAP_BASE - _ota_partition->address;

    esp_err_t err = esp_partition_erase_range(_ota_partition, part_offset, ESP32_FLASH_SECTOR_SIZE);

    return (err == ESP_OK) ? 0 : ERROR_PERMANENT;

}

uint16_t BootloaderDriversOpenlcb_write_flash_bytes(uint32_t address, const uint8_t *data, uint32_t size_bytes) {

    uint32_t       part_offset = address - ESP32_FLASH_MMAP_BASE - _ota_partition->address;
    const uint8_t *source_data = data;
    uint32_t       remaining   = size_bytes;

    while (remaining >= FLASH_WRITE_ALIGN) {

        esp_err_t err = esp_partition_write(_ota_partition, part_offset, source_data, FLASH_WRITE_ALIGN);

        if (err != ESP_OK) {

            return ERROR_PERMANENT;

        }

        part_offset += FLASH_WRITE_ALIGN;
        source_data += FLASH_WRITE_ALIGN;
        remaining   -= FLASH_WRITE_ALIGN;

    }

    /* Pad any trailing sub-4-byte remainder with 0xFF (erased flash value). */
    if (remaining > 0U) {

        uint8_t pad[FLASH_WRITE_ALIGN];
        memset(pad, 0xFF, sizeof(pad));
        memcpy(pad, source_data, remaining);

        esp_err_t err = esp_partition_write(_ota_partition, part_offset, pad, FLASH_WRITE_ALIGN);

        if (err != ESP_OK) {

            return ERROR_PERMANENT;

        }

    }

    return 0;

}

uint16_t BootloaderDriversOpenlcb_finalize_flash(compute_checksum_func_t compute_checksum_helper) {

#ifndef NO_CHECKSUM

    /* TODO: implement checksum verification using compute_checksum_helper
     * once the post-link tool populates app_header in the firmware image. */
    (void) compute_checksum_helper;

#else

    (void) compute_checksum_helper;

#endif /* NO_CHECKSUM */

    /*
     * Commit the update: tell the ROM second-stage bootloader to select
     * the newly written partition on the next reset.
     */
    esp_err_t err = esp_ota_set_boot_partition(_ota_partition);

    return (err == ESP_OK) ? 0 : ERROR_PERMANENT;

}

/* ====================================================================== */
/* Flash read                                                              */
/* ====================================================================== */

void BootloaderDriversOpenlcb_read_flash_bytes(uint32_t flash_addr, void *dest_ram, uint32_t size_bytes) {

    /*
     * ESP32 flash is memory-mapped (read-only) via the MMU into the DROM
     * window at ESP32_FLASH_MMAP_BASE.  The CPU can read it with a normal
     * load instruction, so a plain memcpy() is sufficient.
     *
     * This is identical to the STM32 Von Neumann approach.  On a Harvard
     * architecture (dsPIC33), TBLRD instructions would be needed instead.
     */
    memcpy(dest_ram, (const void *)(uintptr_t) flash_addr, size_bytes);

}

/* ====================================================================== */
/* Checksum                                                                */
/* ====================================================================== */

void BootloaderDriversOpenlcb_compute_checksum(uint32_t flash_addr, uint32_t size, uint32_t *checksum) {

    /*
     * Flash is memory-mapped and CPU-readable, so the block CRC function
     * can operate directly on the mapped pointer -- no byte-by-byte
     * table-read loop is needed (contrast with the dsPIC33 Harvard port).
     */
    uint16_t crc3[3];
    BootloaderCrc_crc3_crc16_ibm((const void *)(uintptr_t) flash_addr, size, crc3);

    checksum[0] = (uint32_t) crc3[0];
    checksum[1] = (uint32_t) crc3[1];
    checksum[2] = (uint32_t) crc3[2];
    checksum[3] = 0;

}

/* ====================================================================== */
/* Timer                                                                   */
/* ====================================================================== */

uint8_t BootloaderDriversOpenlcb_get_100ms_timer_tick(void) {

    /*
     * Arduino millis() returns milliseconds since boot.
     * Dividing by 100 gives a counter that increments every 100 ms.
     * Casting to uint8_t provides the same 0-255 wrapping behaviour
     * expected by the bootloader library.
     */
    return (uint8_t)(millis() / 100UL);

}

/* ====================================================================== */
/* Identity                                                                */
/* ====================================================================== */

uint64_t BootloaderDriversOpenlcb_get_persistent_node_id(void) {

    /* TODO: Read from NVS once production programming is in place.
     * Hardcoded for bringup. */
    return 0x050101012201ULL;

}

/* ====================================================================== */
/* LED                                                                     */
/* ====================================================================== */

void BootloaderDriversOpenlcb_set_status_led(bootloader_led_enum led, bool value) {

    /*
     * ESP32 DevKit V1 has one built-in LED on GPIO 2.  All logical LED
     * functions are multiplexed onto it: any active state turns it on.
     *
     * TODO: If the target board has multiple LEDs, add GPIO assignments
     * here and map each bootloader_led_enum to its own pin.
     */
    switch (led) {

        case BOOTLOADER_LED_ACTIVE:
        case BOOTLOADER_LED_WRITING:
        case BOOTLOADER_LED_CSUM_ERROR:
        case BOOTLOADER_LED_FRAME_LOST:
        case BOOTLOADER_LED_REQUEST:
            gpio_set_level((gpio_num_t) BOOTLOADER_LED_PIN, value ? 1 : 0);
            break;

        default:
            break;

    }

}
