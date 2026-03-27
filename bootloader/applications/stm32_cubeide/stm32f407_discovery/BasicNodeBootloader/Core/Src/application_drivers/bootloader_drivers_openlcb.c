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
 * STM32F407 hardware driver implementation for the standalone bootloader
 * OpenLCB layer.  Implements flash operations, lifecycle control, LED,
 * timer, identity, and checksum functions.  All functions are public so
 * main.c can wire them into the bootloader_openlcb_driver_t DI struct.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include <string.h>

/* #include "../Debug/ti_msp_dl_config.h" */  /* TI DriverLib — not used on STM32 */
#include "main.h"
#include "bootloader_drivers_openlcb.h"

extern CAN_HandleTypeDef hcan1;
#include "../../../../shared/bootloader_shared_ram.h"
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
     * B1 (blue button) on PA0 is active-high on the STM32F4 Discovery.
     * Hold it during power-on to force bootloader mode even if no
     * application has requested it via the magic RAM flag.
     * The bootloader starts as a normal node (firmware_active = 0)
     * and the CT must send Freeze before transferring firmware.
     */
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {

        return BOOTLOADER_REQUESTED_BY_BUTTON;

    }

    return BOOTLOADER_NOT_REQUESTED;

}

    /**
     * @brief Tears down all peripherals and ARM core state before handing off to the other binary.
     *
     * @details Algorithm:
     * -# De-initialise the CAN1 peripheral
     * -# Stop SysTick timer and clear its reload and current value registers
     * -# Disable all NVIC interrupts and clear all pending interrupt flags
     */
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {

    HAL_CAN_DeInit(&hcan1);

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICPR[0] = 0xFFFFFFFF;

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

    HAL_NVIC_SystemReset();

    while (1) {
    }

}

/* ====================================================================== */
/* Flash sector table (STM32F407, single bank, non-uniform sectors)        */
/* ====================================================================== */

typedef struct {

    uint32_t start;
    uint32_t size;
    uint32_t sector_number;

} flash_sector_entry_t;

static const flash_sector_entry_t _flash_sectors[] = {
    { 0x08000000U,  16384U, FLASH_SECTOR_0  },
    { 0x08004000U,  16384U, FLASH_SECTOR_1  },
    { 0x08008000U,  16384U, FLASH_SECTOR_2  },
    { 0x0800C000U,  16384U, FLASH_SECTOR_3  },
    { 0x08010000U,  65536U, FLASH_SECTOR_4  },
    { 0x08020000U, 131072U, FLASH_SECTOR_5  },
    { 0x08040000U, 131072U, FLASH_SECTOR_6  },
    { 0x08060000U, 131072U, FLASH_SECTOR_7  },
    { 0x08080000U, 131072U, FLASH_SECTOR_8  },
    { 0x080A0000U, 131072U, FLASH_SECTOR_9  },
    { 0x080C0000U, 131072U, FLASH_SECTOR_10 },
    { 0x080E0000U, 131072U, FLASH_SECTOR_11 },
};

#define FLASH_SECTOR_COUNT (sizeof(_flash_sectors) / sizeof(_flash_sectors[0]))

static const flash_sector_entry_t *_find_sector(uint32_t address) {

    for (uint32_t sector_index = 0; sector_index < FLASH_SECTOR_COUNT; sector_index++) {

        if (address >= _flash_sectors[sector_index].start &&
            address <  _flash_sectors[sector_index].start + _flash_sectors[sector_index].size) {

            return &_flash_sectors[sector_index];

        }

    }

    return NULL;

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

    const flash_sector_entry_t *s = _find_sector(address);

    if (s == NULL) {

        *page_start        = address;
        *page_length_bytes = 0;
        return;

    }

    *page_start        = s->start;
    *page_length_bytes = s->size;

}

uint16_t BootloaderDriversOpenlcb_erase_flash_page(uint32_t address) {

    const flash_sector_entry_t *s = _find_sector(address);

    if (s == NULL) {

        return ERROR_PERMANENT;

    }

    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;  /* 2.7 V–3.6 V: 32-bit parallelism */
    erase.Sector       = s->sector_number;
    erase.NbSectors    = 1;

    uint32_t sector_error;

    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &sector_error);
    HAL_FLASH_Lock();

    return (status == HAL_OK) ? 0 : ERROR_PERMANENT;

}

void BootloaderDriversOpenlcb_read_flash_bytes(uint32_t flash_addr, void *dest_ram, uint32_t size_bytes) {

    /*
     * STM32F407 Von Neumann architecture: flash is memory-mapped starting at
     * 0x08000000.  The CPU can read it with a normal load instruction, so a
     * plain memcpy() is all we need.
     *
     * On a Harvard architecture (dsPIC33), flash and SRAM are SEPARATE
     * address spaces.  A data pointer cannot reach flash -- you must use the
     * TBLRDL/TBLRDH table-read instructions via FLASH_ReadWord24().  The DI
     * function pointer exists precisely to let the same bootloader.c work on
     * both architectures by injecting the correct implementation here.
     */
    memcpy(dest_ram, (const void *)(uintptr_t) flash_addr, size_bytes);

}

uint16_t BootloaderDriversOpenlcb_write_flash_bytes(uint32_t address, const uint8_t *data, uint32_t size_bytes) {

    uint32_t       dest        = address;
    const uint8_t *source_data = data;
    uint32_t       remaining   = size_bytes;

    HAL_FLASH_Unlock();

    while (remaining >= FLASH_WRITE_ALIGN) {

        uint32_t word;
        memcpy(&word, source_data, FLASH_WRITE_ALIGN);

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest, (uint64_t) word) != HAL_OK) {

            HAL_FLASH_Lock();
            return ERROR_PERMANENT;

        }

        dest        += FLASH_WRITE_ALIGN;
        source_data += FLASH_WRITE_ALIGN;
        remaining   -= FLASH_WRITE_ALIGN;

    }

    if (remaining > 0) {

        uint8_t  pad[FLASH_WRITE_ALIGN];
        uint32_t word;
        memset(pad, 0xFF, sizeof(pad));
        memcpy(pad, source_data, remaining);
        memcpy(&word, pad, FLASH_WRITE_ALIGN);

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest, (uint64_t) word) != HAL_OK) {

            HAL_FLASH_Lock();
            return ERROR_PERMANENT;

        }

    }

    HAL_FLASH_Lock();
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

uint8_t BootloaderDriversOpenlcb_get_100ms_timer_tick(void) {

    /*
     * HAL_GetTick() returns milliseconds since HAL_Init().
     * Dividing by 100 gives a value that increments every 100ms, matching
     * the bootloader library's expected unit.  Casting to uint8_t gives
     * the same wrapping behaviour as the TI SysTick counter approach.
     *
     * stm32f4xx_it.c owns SysTick_Handler and calls HAL_IncTick() there —
     * no separate ISR is needed here.
     */
    return (uint8_t)(HAL_GetTick() / 100U);

}

/* ====================================================================== */
/* Identity                                                                */
/* ====================================================================== */

uint64_t BootloaderDriversOpenlcb_get_persistent_node_id(void) {

    /* TODO: Read from protected flash sector at NODEID_FLASH_ADDRESS
     * once production programming is in place. Hardcoded for bringup. */
     
    return 0x050101012201ULL;

}

/* ====================================================================== */
/* LED                                                                     */
/* ====================================================================== */

void BootloaderDriversOpenlcb_set_status_led(bootloader_led_enum led, bool value) {

    GPIO_PinState state = value ? GPIO_PIN_SET : GPIO_PIN_RESET;

    switch (led) {

        case BOOTLOADER_LED_ACTIVE:
            HAL_GPIO_WritePin(MAIN_LOOP_GREEN_LED_GPIO_Port, MAIN_LOOP_GREEN_LED_Pin, state);
            break;

        case BOOTLOADER_LED_WRITING:
            HAL_GPIO_WritePin(CAN_TX_LED_BLUE_GPIO_Port, CAN_TX_LED_BLUE_Pin, state);
            break;

        case BOOTLOADER_LED_CSUM_ERROR:
        case BOOTLOADER_LED_FRAME_LOST:
            HAL_GPIO_WritePin(_100MS_TIMER_LED_RED_GPIO_Port, _100MS_TIMER_LED_RED_Pin, state);
            break;

        case BOOTLOADER_LED_REQUEST:
            HAL_GPIO_WritePin(CAN_RX_ORANGE_LED_GPIO_Port, CAN_RX_ORANGE_LED_Pin, state);
            break;

        default:
            break;

    }

}

