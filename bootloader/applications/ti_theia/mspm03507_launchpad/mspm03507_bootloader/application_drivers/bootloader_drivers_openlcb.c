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
 * MSPM0G3507 hardware driver implementation for the standalone bootloader
 * OpenLCB layer.  Implements flash operations, lifecycle control, LED,
 * timer, identity, and checksum functions.  All functions are public so
 * main.c can wire them into the bootloader_openlcb_driver_t DI struct.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "../Debug/ti_msp_dl_config.h"
#include "bootloader_drivers_openlcb.h"

#include "../src/crc/bootloader_crc.h"

/* ====================================================================== */
/* Lifecycle                                                               */
/* ====================================================================== */

void BootloaderDriversOpenlcb_initialize_hardware(void) {

    __disable_irq();
    SYSCFG_DL_init();
    __enable_irq();

}

bool BootloaderDriversOpenlcb_is_bootloader_requested(void) {

    uint32_t pin_state = DL_GPIO_readPins(BOOTLOADER_BUTTON_PORT, BOOTLOADER_BUTTON_PIN);

    if (pin_state == 0) {

        return true;

    }

    return false;

}

void BootloaderDriversOpenlcb_jump_to_application(void) {

    __disable_irq();

    uint32_t *app_vectors = (uint32_t *) APP_FLASH_START;

    SCB->VTOR = APP_FLASH_START;
    __set_MSP(app_vectors[0]);

    void (*app_reset)(void) = (void (*)(void)) app_vectors[1];
    app_reset();

    while (1) {
    }

}

void BootloaderDriversOpenlcb_reboot(void) {

    NVIC_SystemReset();

    while (1) {
    }

}

/* ====================================================================== */
/* Flash operations                                                        */
/* ====================================================================== */

void BootloaderDriversOpenlcb_get_flash_boundaries(const void **flash_min, const void **flash_max, const struct bootloader_app_header **app_header) {

    *flash_min = (const void *) APP_FLASH_START;
    *flash_max = (const void *) APP_FLASH_END;
    *app_header = (const struct bootloader_app_header *) APP_HEADER_ADDRESS;

}

void BootloaderDriversOpenlcb_get_flash_page_info(const void *address, const void **page_start, uint32_t *page_length_bytes) {

    uintptr_t addr = (uintptr_t) address;
    *page_start = (const void *) (addr & ~(FLASH_SECTOR_SIZE - 1));
    *page_length_bytes = FLASH_SECTOR_SIZE;

}

uint16_t BootloaderDriversOpenlcb_erase_flash_page(const void *address) {

    // DL_FlashCTL_executeClearStatus(FLASHCTL);
    // DL_FLASHCTL_COMMAND_STATUS status =
    //         DL_FlashCTL_eraseMemoryFromRAM(FLASHCTL, (uint32_t) address,
    //                 DL_FLASHCTL_COMMAND_SIZE_SECTOR);

    // if (status == DL_FLASHCTL_COMMAND_STATUS_FAILED) {

    //     return ERROR_PERMANENT;

    // }

    // if (!DL_FlashCTL_waitForCmdDone(FLASHCTL)) {

    //     return ERROR_PERMANENT;

    // }

    return 0;

}

uint16_t BootloaderDriversOpenlcb_write_flash(const void *address, const void *data, uint32_t size_bytes) {

    // uint32_t dest = (uint32_t) address;
    // const uint8_t *src = (const uint8_t *) data;
    // uint32_t remaining = size_bytes;

    // while (remaining >= FLASH_WRITE_ALIGN) {

    //     DL_FlashCTL_executeClearStatus(FLASHCTL);
    //     DL_FLASHCTL_COMMAND_STATUS status =
    //             DL_FlashCTL_programMemoryFromRAM64WithECCGenerated(
    //                     FLASHCTL, dest, (uint32_t *) src);

    //     if (status == DL_FLASHCTL_COMMAND_STATUS_FAILED) {

    //         return ERROR_PERMANENT;

    //     }

    //     if (!DL_FlashCTL_waitForCmdDone(FLASHCTL)) {

    //         return ERROR_PERMANENT;

    //     }

    //     dest += FLASH_WRITE_ALIGN;
    //     src += FLASH_WRITE_ALIGN;
    //     remaining -= FLASH_WRITE_ALIGN;

    // }

    // if (remaining > 0) {

    //     uint8_t pad_buffer[FLASH_WRITE_ALIGN];
    //     memset(pad_buffer, 0xFF, sizeof(pad_buffer));
    //     memcpy(pad_buffer, src, remaining);

    //     DL_FlashCTL_executeClearStatus(FLASHCTL);
    //     DL_FLASHCTL_COMMAND_STATUS status =
    //             DL_FlashCTL_programMemoryFromRAM64WithECCGenerated(
    //                     FLASHCTL, dest, (uint32_t *) pad_buffer);

    //     if (status == DL_FLASHCTL_COMMAND_STATUS_FAILED) {

    //         return ERROR_PERMANENT;

    //     }

    //     if (!DL_FlashCTL_waitForCmdDone(FLASHCTL)) {

    //         return ERROR_PERMANENT;

    //     }

    // }

    return 0;

}

uint16_t BootloaderDriversOpenlcb_finalize_flash(compute_checksum_func_t compute_checksum_helper) {

    const bootloader_app_header_t *app_header =
            (const bootloader_app_header_t *) APP_HEADER_ADDRESS;

    bootloader_app_header_t header_copy;
    memcpy(&header_copy, app_header, sizeof(header_copy));

    uint32_t flash_size = APP_FLASH_END - APP_FLASH_START;

    if (header_copy.app_size > flash_size) { return ERROR_PERMANENT; }

    /* Pre-checksum: flash_min to app_header. */
    uint32_t pre_size = (uint32_t) ((const uint8_t *) app_header - (const uint8_t *) APP_FLASH_START);

    uint32_t checksum[BOOTLOADER_CHECKSUM_COUNT];
    memset(checksum, 0, sizeof(checksum));

    compute_checksum_helper((const void *) APP_FLASH_START, pre_size, checksum);

    if (memcmp(header_copy.checksum_pre, checksum, sizeof(checksum)) != 0) {

        return ERROR_PERMANENT;

    }

    /* Post-checksum: after app_header to app_size. */
    uint32_t post_offset = (uint32_t) (sizeof(bootloader_app_header_t) + pre_size);
    uint32_t post_size = 0;

    if (post_offset < header_copy.app_size) {

        post_size = header_copy.app_size - post_offset;

    }

    memset(checksum, 0, sizeof(checksum));
    compute_checksum_helper((const uint8_t *) app_header + sizeof(bootloader_app_header_t), post_size, checksum);

    if (memcmp(header_copy.checksum_post, checksum, sizeof(checksum)) != 0) {

        return ERROR_PERMANENT;

    }

    return 0;

}

/* ====================================================================== */
/* Checksum                                                                */
/* ====================================================================== */

void BootloaderDriversOpenlcb_compute_checksum(const void *data, uint32_t size, uint32_t *checksum) {

    uint16_t crc3[3];
    BootloaderCrc_crc3_crc16_ibm(data, size, crc3);

    checksum[0] = (uint32_t) crc3[0];
    checksum[1] = (uint32_t) crc3[1];
    checksum[2] = (uint32_t) crc3[2];
    checksum[3] = 0;

}

/* ====================================================================== */
/* Timer                                                                   */
/* ====================================================================== */

static volatile uint8_t _tick_counter = 0;

void SysTick_Handler(void) {

    _tick_counter++;

}

uint8_t BootloaderDriversOpenlcb_get_100ms_timer_tick(void) {

    return _tick_counter;

}

/* ====================================================================== */
/* Identity                                                                */
/* ====================================================================== */

uint64_t BootloaderDriversOpenlcb_get_persistent_node_id(void) {

    /* TODO: Read from protected flash sector at NODEID_FLASH_ADDRESS
     * once production programming is in place. Hardcoded for bringup. */
    return 0x050101012200ULL;

}

/* ====================================================================== */
/* LED                                                                     */
/* ====================================================================== */

void BootloaderDriversOpenlcb_set_status_led(bootloader_led_enum led, bool value) {

    (void) led;

    if (value) {

        DL_GPIO_clearPins(BOOTLOADER_LED_PORT, BOOTLOADER_LED_PIN);

    } else {

        DL_GPIO_setPins(BOOTLOADER_LED_PORT, BOOTLOADER_LED_PIN);

    }

}
