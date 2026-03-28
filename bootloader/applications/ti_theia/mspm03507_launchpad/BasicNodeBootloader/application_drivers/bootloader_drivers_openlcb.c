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

    /* SYSCFG_DL_init() configures clocks, GPIO, MCAN, and SysTick via the
     * TI SysConfig-generated code.  Interrupts are disabled around it to
     * prevent any peripheral ISR from firing before the full configuration
     * is complete.
     *
     * No explicit MCAN interrupt disable is needed here.  Unlike the
     * application (which enables MCAN RX interrupts), the bootloader uses
     * polling -- it never enables the MCAN NVIC line.  SYSCFG_DL_init()
     * configures the peripheral but does not enable its NVIC interrupt,
     * so no CAN ISR can fire during or after init. */
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
    if (bootloader_shared_ram.request_flag == BOOTLOADER_REQUEST_MAGIC) {

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
     * @brief Tears down all peripherals and ARM core state before handing
     *        off to the application (or before a reboot).
     *
     * @details The application expects to start from a clean hardware state,
     *          as if after a power-on reset.  Every peripheral the bootloader
     *          touched must be returned to its reset-default condition so the
     *          application's own SYSCFG_DL_init() sequence works correctly.
     */
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {

    /* Stop SysTick completely.  The bootloader uses it for the 100ms timer.
     * If left running, the SysTick interrupt would fire into the application's
     * vector table before its own SysTick_Handler is ready, potentially
     * calling into uninitialised code or faulting if the handler address is
     * still erased flash (0xFFFFFFFF). */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Disable all NVIC interrupt sources and clear any pending flags.
     * MSPM0G3507 has fewer than 32 interrupts, so a single ICER/ICPR
     * register pair covers all of them.  (If porting to a chip with more
     * than 32 IRQs, loop over ICER[0..N] and ICPR[0..N] as STM32 does.) */
    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICPR[0] = 0xFFFFFFFF;

    /* Reset the MCAN peripheral to its power-on default configuration so
     * the application can re-init it with its own filter/interrupt settings.
     * Without this the peripheral retains the bootloader's configuration. */
    DL_MCAN_reset(MCAN0_INST);

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

    /* Unreachable -- the application's reset handler does not return. */
    while (1) {
    }

}

void BootloaderDriversOpenlcb_reboot(void) {

    /* NVIC_SystemReset resets the entire CPU and all peripherals.  The CMSIS
     * implementation includes a __DSB() barrier that flushes pending writes
     * before the reset fires.  No peripheral teardown is needed -- everything
     * returns to power-on state. */
    NVIC_SystemReset();

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

    *page_start        = address & ~((uint32_t)(FLASH_SECTOR_SIZE - 1));
    *page_length_bytes = FLASH_SECTOR_SIZE;

}

uint16_t BootloaderDriversOpenlcb_erase_flash_page(uint32_t address) {

    /* Interrupts must be disabled during flash erase.  On MSPM0 the flash
     * controller executes erase/program operations from a RAM trampoline
     * (eraseMemoryFromRAM).  If an interrupt fires while the flash controller
     * is busy, the ISR's instruction fetches from flash would stall or fault.
     * Disabling interrupts prevents this. */
    __disable_irq();

    /* Clear any leftover error flags from a previous operation. */
    DL_FlashCTL_executeClearStatus(FLASHCTL);

    /* The MSPM0 flash has per-sector write/erase protection.  Unprotect the
     * target sector before erasing.  Protection is restored automatically
     * after the operation completes. */
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

    /* Interrupts must be disabled during flash programming for the same
     * reason as erase: the flash controller runs from a RAM trampoline
     * and instruction fetches from flash would stall or fault. */
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

        /* Read-back verification: the flash controller returning success
         * only means the command was accepted, not that the bits landed
         * correctly.  Flash can fail silently due to wear, voltage droop,
         * or marginal cells.  Reading back catches these failures before
         * the bootloader declares success and reboots into a corrupted
         * image. */
        uint32_t readback[2];
        memcpy(readback, (const void *)(uintptr_t) dest, FLASH_WRITE_ALIGN);
        if (readback[0] != aligned_data[0] || readback[1] != aligned_data[1]) {

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

        /* Read-back verification for the partial tail chunk. */
        uint32_t tail_readback[2];
        memcpy(tail_readback, (const void *)(uintptr_t) dest, FLASH_WRITE_ALIGN);
        if (tail_readback[0] != pad_aligned[0] || tail_readback[1] != pad_aligned[1]) {

            __enable_irq();
            return ERROR_PERMANENT;

        }

    }

    __enable_irq();
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

/* SysTick is configured by SYSCFG_DL_init() to fire every 100ms.
 * Each interrupt increments a uint8_t counter that wraps at 256,
 * matching the bootloader library's expected interface for protocol
 * timeouts (alias negotiation, datagram retries). */
static volatile uint8_t _tick_counter = 0;

void SysTick_Handler(void) {

    _tick_counter++;

}

uint8_t BootloaderDriversOpenlcb_get_100ms_timer_tick(void) {

    /* Return the raw counter value.  The library compares successive
     * readings to detect elapsed time; the uint8_t wrap is intentional
     * and handled correctly by unsigned subtraction in the caller. */
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

    /* The MSPM0G3507 Launchpad has a single user LED on PA0 with negative
     * logic (active-low): clearPins turns it ON, setPins turns it OFF.
     * The led parameter is ignored -- all logical LED functions map to the
     * same physical LED.  On boards with multiple LEDs, add a switch on
     * the led enum (see the STM32 demo for an example). */
    (void) led;

    if (value) {

        DL_GPIO_clearPins(BOOTLOADER_LED_PORT, BOOTLOADER_LED_PIN);

    } else {

        DL_GPIO_setPins(BOOTLOADER_LED_PORT, BOOTLOADER_LED_PIN);

    }

}
