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
#include "../shared/bootloader_shared_ram.h"
#include "../openlcb_c_lib_bootloader/crc/bootloader_crc.h"

/* ====================================================================== */
/* Shared RAM flag for app-to-bootloader drop-back                         */
/* ====================================================================== */

/*
 * The shared RAM variables (bootloader_request_flag, bootloader_cached_alias)
 * are defined in ../shared/bootloader_shared_ram.c, which is compiled into
 * both the bootloader and the application project.  Both linker scripts map
 * the .noinit section to the same fixed SRAM region (SHARED_NOINIT) so the
 * two binaries see the same physical addresses.
 *
 * See bootloader_shared_ram.c for platform porting notes covering TI CCS,
 * GCC, IAR, XC16, XC32, ESP-IDF, and Renesas CC-RX.
 */

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

    __disable_irq();
    SYSCFG_DL_init();
    __enable_irq();

}

bootloader_request_t BootloaderDriversOpenlcb_is_bootloader_requested(void) {

    /*
     * Check 1: Did the application ask us to enter bootloader mode?
     *
     * The application writes BOOTLOADER_REQUEST_MAGIC into the shared RAM
     * flag and then calls NVIC_SystemReset().  SRAM survives the soft
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
     * Check 2: Is the hardware bootloader button pressed?
     *
     * This is the manual fallback — hold the button during power-on to
     * force bootloader mode even if the application is not running.
     * The bootloader starts as a normal node (firmware_active = 0)
     * and the CT must send Freeze before transferring firmware.
     */
    uint32_t pin_state = DL_GPIO_readPins(BOOTLOADER_BUTTON_PORT, BOOTLOADER_BUTTON_PIN);

    if (pin_state == 0) {

        return BOOTLOADER_REQUESTED_BY_BUTTON;

    }

    return BOOTLOADER_NOT_REQUESTED;

}

    /**
     * @brief Tears down all peripherals and ARM core state before handing off to the other binary.
     *
     * @details Algorithm:
     * -# Stop SysTick timer and clear its reload and current value registers
     * -# Disable all NVIC interrupts and clear all pending interrupt flags
     * -# Reset the MCAN peripheral to its power-on default configuration
     */
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICPR[0] = 0xFFFFFFFF;

    DL_MCAN_reset(MCAN0_INST);

}

void BootloaderDriversOpenlcb_jump_to_application(void) {

    uint32_t *app_vectors = (uint32_t *) APP_FLASH_START;

    SCB->VTOR = APP_FLASH_START;

    /* Load the application's stack pointer and reset handler, then jump. */
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

void BootloaderDriversOpenlcb_get_flash_boundaries(uint32_t *flash_min, uint32_t *flash_max, uint32_t *app_header) {

    *flash_min  = APP_FLASH_START;
    *flash_max  = APP_FLASH_END;
    *app_header = APP_HEADER_ADDRESS;

}

void BootloaderDriversOpenlcb_get_flash_page_info(uint32_t address, uint32_t *page_start, uint32_t *page_length_bytes) {

    *page_start        = address & ~((uint32_t)(FLASH_SECTOR_SIZE - 1));
    *page_length_bytes = FLASH_SECTOR_SIZE;

}

uint16_t BootloaderDriversOpenlcb_erase_flash_page(uint32_t address) {

    __disable_irq();

    DL_FlashCTL_executeClearStatus(FLASHCTL);
    DL_FlashCTL_unprotectSector(FLASHCTL, address, DL_FLASHCTL_REGION_SELECT_MAIN);

    DL_FLASHCTL_COMMAND_STATUS status =
            DL_FlashCTL_eraseMemoryFromRAM(FLASHCTL, address,
                    DL_FLASHCTL_COMMAND_SIZE_SECTOR);

    if (status == DL_FLASHCTL_COMMAND_STATUS_FAILED) {

        __enable_irq();
        return ERROR_PERMANENT;

    }

    if (!DL_FlashCTL_waitForCmdDone(FLASHCTL)) {

        __enable_irq();
        return ERROR_PERMANENT;

    }

    __enable_irq();
    return 0;

}

uint16_t BootloaderDriversOpenlcb_write_flash_bytes(uint32_t address, const uint8_t *data, uint32_t size_bytes) {

    uint32_t       dest        = address;
    const uint8_t *source_data = data;
    uint32_t       remaining   = size_bytes;

    __disable_irq();

    while (remaining >= FLASH_WRITE_ALIGN) {

        /* Copy 8 bytes into a properly aligned uint32_t buffer.
         * The flash controller requires 4-byte aligned source data.
         * Cortex-M0+ does not support unaligned access, so casting
         * a uint8_t* to uint32_t* will hang if the buffer is not
         * naturally aligned. */
        uint32_t aligned_data[2];
        memcpy(aligned_data, source_data, FLASH_WRITE_ALIGN);

        DL_FlashCTL_executeClearStatus(FLASHCTL);
        DL_FlashCTL_unprotectSector(FLASHCTL, dest, DL_FLASHCTL_REGION_SELECT_MAIN);
        DL_FLASHCTL_COMMAND_STATUS status =
                DL_FlashCTL_programMemoryFromRAM64WithECCGenerated(
                        FLASHCTL, dest, aligned_data);

        if (status == DL_FLASHCTL_COMMAND_STATUS_FAILED) {

            __enable_irq();
            return ERROR_PERMANENT;

        }

        if (!DL_FlashCTL_waitForCmdDone(FLASHCTL)) {

            __enable_irq();
            return ERROR_PERMANENT;

        }

        dest        += FLASH_WRITE_ALIGN;
        source_data += FLASH_WRITE_ALIGN;
        remaining   -= FLASH_WRITE_ALIGN;

    }

    if (remaining > 0) {

        /* Pad the final partial chunk to 8 bytes with 0xFF (erased state)
         * and copy into an aligned buffer for the flash controller. */
        uint32_t pad_aligned[2];
        uint8_t pad_buffer[FLASH_WRITE_ALIGN];
        memset(pad_buffer, 0xFF, sizeof(pad_buffer));
        memcpy(pad_buffer, source_data, remaining);
        memcpy(pad_aligned, pad_buffer, FLASH_WRITE_ALIGN);

        DL_FlashCTL_executeClearStatus(FLASHCTL);
        DL_FlashCTL_unprotectSector(FLASHCTL, dest, DL_FLASHCTL_REGION_SELECT_MAIN);
        DL_FLASHCTL_COMMAND_STATUS status =
                DL_FlashCTL_programMemoryFromRAM64WithECCGenerated(
                        FLASHCTL, dest, pad_aligned);

        if (status == DL_FLASHCTL_COMMAND_STATUS_FAILED) {

            __enable_irq();
            return ERROR_PERMANENT;

        }

        if (!DL_FlashCTL_waitForCmdDone(FLASHCTL)) {

            __enable_irq();
            return ERROR_PERMANENT;

        }

    }

    __enable_irq();
    return 0;

}

uint16_t BootloaderDriversOpenlcb_finalize_flash(compute_checksum_func_t compute_checksum_helper) {

#ifndef NO_CHECKSUM

    /* TODO: implement checksum validation using compute_checksum_helper
     * once the post-link tool populates app_header in the firmware image. */
    (void) compute_checksum_helper;
    return 0;

#else

    (void) compute_checksum_helper;
    return 0;

#endif /* NO_CHECKSUM */

}

/* ====================================================================== */
/* Flash read                                                              */
/* ====================================================================== */

void BootloaderDriversOpenlcb_read_flash_bytes(uint32_t flash_addr, void *dest_ram, uint32_t size_bytes) {

    /* MSPM0G3507 is a Von Neumann ARM Cortex-M0+: flash is in the unified
     * address space and can be read directly via a data pointer. */
    memcpy(dest_ram, (const void *)(uintptr_t) flash_addr, size_bytes);

}

/* ====================================================================== */
/* Checksum                                                                */
/* ====================================================================== */

void BootloaderDriversOpenlcb_compute_checksum(uint32_t flash_addr, uint32_t size, uint32_t *checksum) {

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
