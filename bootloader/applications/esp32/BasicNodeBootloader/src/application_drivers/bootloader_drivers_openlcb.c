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
 *   bootloader_shared_ram.request_flag and bootloader_shared_ram.cached_alias are placed in RTC
 *   slow memory using RTC_NOINIT_ATTR.  This region survives both software
 *   reset and deep sleep, giving the same magic-value handshake behaviour as
 *   the .noinit section on STM32 and Cortex-M targets.  Definitions live in
 *   ../../shared/bootloader_shared_ram.c (compiled into both projects).
 *
 * Flash reads:
 *   The inactive OTA partition is NOT memory-mapped by the ESP32 MMU --
 *   only the running partition is.  All flash reads use esp_partition_read()
 *   with a partition-relative offset, not memcpy().
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include <string.h>

#include "Arduino.h"
#include "esp_system.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "driver/twai.h"

#include "bootloader_drivers_openlcb.h"
#include "../../shared/bootloader_shared_ram.h"
#include "../openlcb_c_lib_bootloader/crc/bootloader_crc.h"

/* ====================================================================== */
/* OTA partition cache                                                     */
/* ====================================================================== */

/*
 * Resolved once by get_flash_boundaries() and reused by every subsequent
 * flash call.  NULL until get_flash_boundaries() has been called.
 *
 * _ensure_ota_partition() provides lazy initialisation for code paths
 * that run before get_flash_boundaries() (e.g. finalize_flash on cold boot).
 */
static const esp_partition_t *_ota_partition = NULL;

static void _ensure_ota_partition(void) {

    if (_ota_partition == NULL) {

        _ota_partition = esp_ota_get_next_update_partition(NULL);

    }

}

    /**
     *     Converts a library-facing mapped address to a partition-relative
     *     byte offset suitable for esp_partition_read/write/erase.
     */
static uint32_t _mapped_to_offset(uint32_t mapped_addr) {

    return mapped_addr - ESP32_FLASH_MMAP_BASE - _ota_partition->address;

}

/* ====================================================================== */
/* Lifecycle                                                               */
/* ====================================================================== */

void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request) {

    /* Centralized .noinit shared RAM cleanup.
     * bootloader_shared_ram.request_flag was already consumed by is_bootloader_requested()
     * before this function runs; clear it unconditionally as belt-and-suspenders.
     * bootloader_shared_ram.cached_alias is only meaningful on app drop-back -- the library
     * CAN state machine reads it via get_cached_alias() during INIT_PICK_ALIAS
     * and clears it after pickup.  On any other reset path it is stale garbage. */
    bootloader_shared_ram.request_flag = 0;

    if (request != BOOTLOADER_REQUESTED_BY_APP) {

        bootloader_shared_ram.cached_alias = 0;

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
    if (bootloader_shared_ram.request_flag == BOOTLOADER_REQUEST_MAGIC) {

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
     * reset.
     *
     * For BOTH the cold-boot path (valid app, no update requested) and
     * the post-update path, we must tell the ROM bootloader which
     * partition to boot.  _ota_partition points to the "next update"
     * partition (the one the bootloader would write to), which IS the
     * application partition when the bootloader itself is the running
     * image.
     *
     * If finalize_flash() already called esp_ota_set_boot_partition(),
     * calling it again is harmless (idempotent).
     */
    _ensure_ota_partition();

    esp_ota_set_boot_partition(_ota_partition);
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

    _ensure_ota_partition();

#ifndef NO_CHECKSUM

    /*
     * Read the app header from the freshly written flash image.
     * The inactive partition is not memory-mapped, so use
     * esp_partition_read() with a partition-relative offset.
     */
    bootloader_app_header_t header;
    uint32_t flash_min = ESP32_FLASH_MMAP_BASE + _ota_partition->address;

    esp_partition_read(_ota_partition, APP_HEADER_OFFSET,
                       &header, sizeof(header));

    /* Validate app_size is within partition bounds. */
    if (header.app_size > _ota_partition->size) { return ERROR_PERMANENT; }

    /* Pre-checksum: partition start to app_header. */
    uint32_t pre_size = APP_HEADER_OFFSET;

    uint32_t checksum[BOOTLOADER_CHECKSUM_COUNT];
    memset(checksum, 0, sizeof(checksum));
    compute_checksum_helper(flash_min, pre_size, checksum);

    if (memcmp(header.checksum_pre, checksum, sizeof(checksum)) != 0) {

        return ERROR_PERMANENT;

    }

    /* Post-checksum: after app_header to end of image. */
    uint32_t post_start = flash_min + APP_HEADER_OFFSET +
                          (uint32_t) sizeof(bootloader_app_header_t);
    uint32_t post_size = 0;

    if ((pre_size + (uint32_t) sizeof(bootloader_app_header_t)) < header.app_size) {

        post_size = header.app_size - pre_size -
                    (uint32_t) sizeof(bootloader_app_header_t);

    }

    memset(checksum, 0, sizeof(checksum));
    compute_checksum_helper(post_start, post_size, checksum);

    if (memcmp(header.checksum_post, checksum, sizeof(checksum)) != 0) {

        return ERROR_PERMANENT;

    }

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
     * The bootloader reads the INACTIVE OTA partition, which is NOT
     * memory-mapped by the ESP32 MMU (only the running partition is).
     * Use esp_partition_read() with a partition-relative offset.
     */
    _ensure_ota_partition();

    uint32_t part_offset = _mapped_to_offset(flash_addr);

    esp_partition_read(_ota_partition, part_offset, dest_ram, size_bytes);

}

/* ====================================================================== */
/* Checksum                                                                */
/* ====================================================================== */

/*
 * Local CRC-16-IBM nibble tables -- duplicated from bootloader_crc.c so
 * the chunk-based compute_checksum can run incrementally without changing
 * the core library API.
 */
static const uint16_t _CRC16_IBM_HI[16] = {

    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,

};

static const uint16_t _CRC16_IBM_LO[16] = {

    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,

};

static void _crc16_ibm_add(uint16_t *state, uint8_t data) {

    *state ^= data;
    *state = (*state >> 8) ^ _CRC16_IBM_LO[*state & 0x0F] ^
            _CRC16_IBM_HI[(*state >> 4) & 0x0F];

}

#define CHECKSUM_CHUNK_SIZE 256U

void BootloaderDriversOpenlcb_compute_checksum(uint32_t flash_addr, uint32_t size, uint32_t *checksum) {

    /*
     * The inactive OTA partition is not memory-mapped, so we read flash
     * in fixed-size chunks via esp_partition_read() and feed each byte
     * into three incremental CRC-16-IBM accumulators (all, odd, even).
     * This keeps stack usage bounded to CHECKSUM_CHUNK_SIZE bytes
     * regardless of image size.
     */
    _ensure_ota_partition();

    uint16_t state_all  = 0x0000;
    uint16_t state_odd  = 0x0000;
    uint16_t state_even = 0x0000;

    uint32_t part_offset  = _mapped_to_offset(flash_addr);
    uint32_t remaining    = size;
    uint32_t byte_counter = 1;
    uint8_t  chunk[CHECKSUM_CHUNK_SIZE];

    while (remaining > 0) {

        uint32_t to_read = (remaining < CHECKSUM_CHUNK_SIZE) ? remaining : CHECKSUM_CHUNK_SIZE;

        esp_partition_read(_ota_partition, part_offset, chunk, to_read);

        for (uint32_t i = 0; i < to_read; i++) {

            _crc16_ibm_add(&state_all, chunk[i]);

            if (byte_counter & 1) {

                _crc16_ibm_add(&state_odd, chunk[i]);

            } else {

                _crc16_ibm_add(&state_even, chunk[i]);

            }

            byte_counter++;

        }

        part_offset += to_read;
        remaining   -= to_read;

    }

    checksum[0] = (uint32_t) state_all;
    checksum[1] = (uint32_t) state_odd;
    checksum[2] = (uint32_t) state_even;
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

    /*
     * Read the 48-bit node ID from NVS.  The application is responsible
     * for writing this value during production programming (namespace
     * "openlcb", key "node_id", stored as uint64_t).
     *
     * Falls back to a hardcoded default if NVS has not been programmed.
     */
    nvs_flash_init();

    nvs_handle_t handle;
    uint64_t node_id = 0x050101012201ULL;  /* fallback */

    if (nvs_open("openlcb", NVS_READONLY, &handle) == ESP_OK) {

        uint64_t stored;

        if (nvs_get_u64(handle, "node_id", &stored) == ESP_OK) {

            node_id = stored;

        }

        nvs_close(handle);

    }

    return node_id;

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
