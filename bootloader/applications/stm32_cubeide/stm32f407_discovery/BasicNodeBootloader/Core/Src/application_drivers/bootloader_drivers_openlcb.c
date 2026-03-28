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
 * The shared RAM variables (bootloader_shared_ram.request_flag, bootloader_shared_ram.cached_alias)
 * are defined in ../shared/bootloader_shared_ram.c, which is compiled into
 * both the bootloader and the application project.  Both linker scripts map
 * the .noinit section to the same fixed SRAM region (SHARED_NOINIT) so the
 * two binaries see the same physical addresses.
 *
 * See bootloader_shared_ram.c for platform porting notes covering TI CCS,
 * GCC, IAR, XC16, XC32, ESP-IDF, and Renesas CC-RX.
 */



/* ====================================================================== */
/* Flash sector table (STM32F407, single bank, non-uniform sectors)        */
/* ====================================================================== */

/*
 * The STM32F407 has 1 MB of flash organised in a single bank with NON-UNIFORM
 * sector sizes.  Unlike many microcontrollers where every page is the same
 * size, the STM32F4 uses a mixed layout:
 *
 *   Sectors 0-3  :  16 KB each  (0x08000000 - 0x0800FFFF)
 *   Sector  4    :  64 KB       (0x08010000 - 0x0801FFFF)
 *   Sectors 5-11 : 128 KB each  (0x08020000 - 0x080FFFFF)
 *
 * The HAL erase function requires a SECTOR NUMBER, not an address.  The
 * bootloader library works in byte addresses, so this table maps addresses
 * to sector numbers.  _find_sector() performs the lookup: given any byte
 * address in flash, it returns the sector's start address, size, and HAL
 * sector number.
 *
 * The non-uniform layout matters for two reasons:
 *
 *   1. Erase granularity -- the smallest erase unit is a full sector.
 *      Erasing sector 5 wipes 128 KB even if only one byte needs to change.
 *      The bootloader library calls get_flash_page_info() to learn the
 *      erase size so it can manage write boundaries correctly.
 *
 *   2. Memory partitioning -- the small 16 KB sectors at the bottom are
 *      used for the bootloader (sectors 0-1, 32 KB total).  The application
 *      starts at sector 2 (0x08008000).  Sector 11 is reserved for
 *      persistent data (node ID).  This layout lets the bootloader fit in
 *      the smallest sectors while giving the application the large sectors.
 *
 * FLASH_SECTOR_* constants come from the STM32 HAL headers and are the
 * values the HAL erase API expects.
 */

typedef struct {

    uint32_t start;
    uint32_t size;
    uint32_t sector_number;

} flash_sector_entry_t;

static const flash_sector_entry_t _flash_sectors[] = {
    { 0x08000000U,  16384U, FLASH_SECTOR_0  },  /* 16 KB - bootloader      */
    { 0x08004000U,  16384U, FLASH_SECTOR_1  },  /* 16 KB - bootloader      */
    { 0x08008000U,  16384U, FLASH_SECTOR_2  },  /* 16 KB - app start       */
    { 0x0800C000U,  16384U, FLASH_SECTOR_3  },  /* 16 KB - app             */
    { 0x08010000U,  65536U, FLASH_SECTOR_4  },  /* 64 KB - app             */
    { 0x08020000U, 131072U, FLASH_SECTOR_5  },  /* 128 KB - app            */
    { 0x08040000U, 131072U, FLASH_SECTOR_6  },  /* 128 KB - app            */
    { 0x08060000U, 131072U, FLASH_SECTOR_7  },  /* 128 KB - app            */
    { 0x08080000U, 131072U, FLASH_SECTOR_8  },  /* 128 KB - app            */
    { 0x080A0000U, 131072U, FLASH_SECTOR_9  },  /* 128 KB - app            */
    { 0x080C0000U, 131072U, FLASH_SECTOR_10 },  /* 128 KB - app            */
    { 0x080E0000U, 131072U, FLASH_SECTOR_11 },  /* 128 KB - node ID store  */
};

#define FLASH_SECTOR_COUNT (sizeof(_flash_sectors) / sizeof(_flash_sectors[0]))

/* Given a byte address anywhere in flash, return the sector entry that
 * contains it.  Returns NULL if the address is outside the flash region
 * (caller treats this as ERROR_PERMANENT). */
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
/* Lifecycle                                                               */
/* ====================================================================== */

void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request) {

    /* Clear the magic flag unconditionally.  is_bootloader_requested() already
     * consumed it; zeroing here prevents a stale flag from falsely re-entering
     * bootloader mode on the next reset. */
    bootloader_shared_ram.request_flag = 0;

    /* On an app drop-back the cached alias is valid -- the application wrote
     * it before resetting so the bootloader can reuse the same CAN alias
     * without re-negotiation.  The CAN state machine reads and clears it
     * during alias pickup.
     *
     * On any other reset path (power-on, button, brownout) the .noinit RAM
     * contains random garbage, so clear it to force normal alias negotiation. */
    if (request != BOOTLOADER_REQUESTED_BY_APP) {

        bootloader_shared_ram.cached_alias = 0;

    }

    /* No explicit CAN interrupt disable is needed here.  On Cortex-M the HAL
     * CAN peripheral requires an explicit HAL_CAN_Start() call before it can
     * generate interrupts.  MX_CAN1_Init() in main() only configures the
     * peripheral -- it does not start it.  The bootloader CAN driver performs
     * its own controlled start in read_received_frame() after the filter is
     * configured. */

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
    if (bootloader_shared_ram.request_flag == BOOTLOADER_REQUEST_MAGIC) {

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
     * @brief Tears down all peripherals and ARM core state before handing
     *        off to the application (or before a reboot).
     *
     * @details The application expects to start from a clean hardware state,
     *          as if after a power-on reset.  Every peripheral the bootloader
     *          touched must be returned to its reset-default condition so the
     *          application's own HAL_Init() / MX_*_Init() sequence works
     *          correctly.
     */
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {

    /* De-init CAN1 so the application can re-init it with its own filter
     * configuration and interrupt settings.  Without this the peripheral
     * retains the bootloader's filter bank setup and started state. */
    HAL_CAN_DeInit(&hcan1);

    /* Stop SysTick completely.  The bootloader uses it for HAL_GetTick()
     * (100ms timer).  If left running, the SysTick interrupt would fire
     * into the application's vector table before its own SysTick_Handler
     * is ready, potentially calling HAL_IncTick() on an uninitialised HAL
     * or faulting if the handler address is still erased flash (0xFFFFFFFF). */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Disable all NVIC interrupt sources and clear any pending flags.
     * STM32F407 has up to 82 interrupts spanning 3 ICER/ICPR registers
     * (IRQ 0-31, 32-63, 64-81).  Clearing all 3 ensures any future
     * peripheral additions above IRQ 31 are covered. */
    for (int i = 0; i < 3; i++) {

        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;

    }

}

void BootloaderDriversOpenlcb_jump_to_application(void) {

    /* The application's vector table starts at APP_FLASH_START.  Entry [0]
     * is the initial stack pointer, entry [1] is the reset handler address. */
    uint32_t *app_vectors = (uint32_t *) APP_FLASH_START;

    /* Relocate the vector table so all interrupts and faults route to the
     * application's handlers instead of the bootloader's.  The DSB ensures
     * the VTOR write completes before any subsequent instruction, and the
     * ISB flushes the pipeline so the CPU fetches vectors from the new
     * table -- not a stale cached copy of the bootloader's table. */
    SCB->VTOR = APP_FLASH_START;
    __DSB();
    __ISB();

    /* Load the application's stack pointer from its vector table entry [0].
     * The bootloader's stack is no longer needed. */
    __set_MSP(app_vectors[0]);

    /* Jump to the application's reset handler (vector table entry [1]).
     * This never returns -- the application runs its own startup code
     * (SystemInit, __libc_init_array, main) from this point forward. */
    void (*app_reset)(void) = (void (*)(void)) app_vectors[1];
    app_reset();

    while (1) {
    }

}

void BootloaderDriversOpenlcb_reboot(void) {

    /* NVIC_SystemReset resets the entire CPU and all peripherals.  The HAL
     * wrapper includes a __DSB() barrier that flushes pending writes before
     * the reset fires.  No peripheral teardown is needed -- everything
     * returns to power-on state. */
    HAL_NVIC_SystemReset();

    /* Unreachable -- the reset completes before this line executes. */
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

        /* Read-back verification: HAL_OK means the peripheral accepted
         * the command, not that the bits landed correctly.  Compare the
         * written word against flash to catch silent corruption. */
        uint32_t readback = *(volatile uint32_t *)(uintptr_t) dest;

        if (readback != word) {

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

        uint32_t readback = *(volatile uint32_t *)(uintptr_t) dest;

        if (readback != word) {

            HAL_FLASH_Lock();
            return ERROR_PERMANENT;

        }

    }

    HAL_FLASH_Lock();
    return 0;

}

uint16_t BootloaderDriversOpenlcb_finalize_flash(compute_checksum_func_t compute_checksum_helper) {

#ifndef NO_CHECKSUM

    /* Read the app header from the freshly written flash image. */
    bootloader_app_header_t header;
    memcpy(&header, (const void *) APP_HEADER_ADDRESS, sizeof(header));

    /* Validate app_size is within flash bounds. */
    uint32_t flash_size = APP_FLASH_END - APP_FLASH_START + 1U;

    if (header.app_size > flash_size) { return ERROR_PERMANENT; }

    /* Pre-checksum: flash_min to app_header. */
    uint32_t pre_size = APP_HEADER_ADDRESS - APP_FLASH_START;

    uint32_t checksum[BOOTLOADER_CHECKSUM_COUNT];
    memset(checksum, 0, sizeof(checksum));
    compute_checksum_helper(APP_FLASH_START, pre_size, checksum);

    if (memcmp(header.checksum_pre, checksum, sizeof(checksum)) != 0) {

        return ERROR_PERMANENT;

    }

    /* Post-checksum: after app_header to end of image. */
    uint32_t post_start = APP_HEADER_ADDRESS + (uint32_t) sizeof(bootloader_app_header_t);
    uint32_t post_size = 0;

    if ((pre_size + (uint32_t) sizeof(bootloader_app_header_t)) < header.app_size) {

        post_size = header.app_size - pre_size - (uint32_t) sizeof(bootloader_app_header_t);

    }

    memset(checksum, 0, sizeof(checksum));
    compute_checksum_helper(post_start, post_size, checksum);

    if (memcmp(header.checksum_post, checksum, sizeof(checksum)) != 0) {

        return ERROR_PERMANENT;

    }

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

