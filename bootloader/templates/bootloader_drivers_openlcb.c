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
 * dsPIC33EP512MC506 hardware driver implementation for the standalone
 * bootloader OpenLCB layer.  Implements flash operations, lifecycle control,
 * LED, timer, identity, and checksum functions.  All functions are public so
 * bootloader_main.c can wire them into the bootloader_openlcb_driver_t DI
 * struct.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include <xc.h>
#include <string.h>

#include "bootloader_drivers_openlcb.h"
#include "bootloader_isr_redirect.h"

#include "../../../shared/bootloader_shared_ram.h"
#include "../../mcc_generated_files/pin_manager.h"
#include "../../mcc_generated_files/memory/flash.h"
#include "../../mcc_generated_files/tmr2.h"
#include "../../mcc_generated_files/can1.h"

/* ====================================================================== */
/* Shared RAM flag for app-to-bootloader drop-back                         */
/* ====================================================================== */

/*
 * The shared RAM variables (bootloader_request_flag, bootloader_cached_alias,
 * bootloader_vivt_jumptable) are defined in ../shared/bootloader_shared_ram.c,
 * compiled into both the bootloader and the application.  Both linker scripts
 * map the .noinit section to the same SRAM region so the two binaries see the
 * same physical addresses.
 *
 * See bootloader_shared_ram.c for porting notes.
 */

/* ====================================================================== */
/* Lifecycle                                                               */
/* ====================================================================== */

void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request) {

    /*
     * The `request` parameter tells us WHY the bootloader was entered,
     * allowing reset-reason-specific cleanup.
     *
     * VIVT (Virtual Interrupt Vector Table):
     *   Always cleared unconditionally.  After any reset the bootloader
     *   owns the hardware IVT; stale app handler pointers in persistent
     *   RAM must never be called.
     *
     * CAN interrupt (app drop-back only):
     *   SYSTEM_Initialize() already ran and enabled C1IE.  With the VIVT
     *   now cleared, a pending CAN interrupt would vector into NULL.
     *   Disable C1IE before global interrupts can deliver it.
     *   BootloaderDriversCan_initialize() re-enables CAN cleanly.
     */

    /* Centralized .noinit shared RAM cleanup.
     * bootloader_request_flag was already consumed by is_bootloader_requested()
     * before this function runs; clear it unconditionally as belt-and-suspenders.
     * bootloader_cached_alias is only meaningful on app drop-back -- the library
     * CAN state machine reads it via get_cached_alias() during INIT_PICK_ALIAS
     * and clears it after pickup.  On any other reset path it is stale garbage. */
    bootloader_request_flag = 0;

    /* VIVT: always clear unconditionally regardless of reset reason. */
    bootloader_vivt_jumptable.oscillatorfail_handler = 0;
    bootloader_vivt_jumptable.addresserror_handler   = 0;
    bootloader_vivt_jumptable.stackerror_handler     = 0;
    bootloader_vivt_jumptable.matherror_handler      = 0;
    bootloader_vivt_jumptable.dmacerror_handler      = 0;
    bootloader_vivt_jumptable.timer_2_handler        = 0;
    bootloader_vivt_jumptable.can1_handler           = 0;

    if (request == BOOTLOADER_REQUESTED_BY_APP) {

        IEC2bits.C1IE = 0;
        IFS2bits.C1IF = 0;
        /* bootloader_cached_alias is left intact here -- the library
         * CAN state machine reads it via get_cached_alias() during
         * INIT_PICK_ALIAS and clears it after pickup. */

    } else {

        bootloader_cached_alias = 0;

    }

    BootloaderDriversCan_initialize();

    /* VIVT is safe, CAN interrupt is clean -- enable global interrupts.
     * SYSTEM_Initialize() no longer enables GIE (hand-edited out). */
    _GIE = 1;

}

bootloader_request_t BootloaderDriversOpenlcb_is_bootloader_requested(void) {

    /*
     * Check 1: Did the application ask us to enter bootloader mode?
     *
     * The application writes BOOTLOADER_REQUEST_MAGIC into the shared RAM
     * flag and then calls asm("RESET").  SRAM survives the soft reset, so
     * the magic value is still here when we start up.  The flag is cleared
     * centrally by initialize_hardware(), not here.
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
     * IO_BOOT (RA10, active-high) is sampled at power-on.
     * Hold it during reset to force bootloader mode even if no
     * application has requested it via the magic RAM flag.
     * The bootloader starts as a normal node (firmware_active = 0)
     * and the CT must send Freeze before transferring firmware.
     */
    if (IO_BOOT_GetValue()) {

        return BOOTLOADER_REQUESTED_BY_BUTTON;

    }

    return BOOTLOADER_NOT_REQUESTED;

}

void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {

    IEC2bits.C1IE = 0;   /* Disable CAN1 interrupt  */
    IEC0bits.T2IE = 0;   /* Disable TMR2 interrupt  */

}

void BootloaderDriversOpenlcb_jump_to_application(void) {

    /*
     * Cast the application start PC address to a function pointer and call
     * it.  The application's __reset handler sets up its own stack pointer
     * and never returns, so the call never unwinds.
     *
     * XC16 generates a CALL instruction here.  Using GOTO would require
     * inline assembly; the function-pointer approach is cleaner and
     * produces equivalent behaviour.
     */
    void (*app)(void) = (void (*)(void)) APP_FLASH_START;
    app();

    while (1) { }   /* unreachable -- silence compiler */

}

void BootloaderDriversOpenlcb_reboot(void) {

    asm("RESET");

    while (1) { }   /* unreachable -- silence compiler */

}

/* ====================================================================== */
/* Flash operations                                                        */
/* ====================================================================== */

void BootloaderDriversOpenlcb_get_flash_boundaries(const void **flash_min, const void **flash_max, const struct bootloader_app_header **app_header) {

    *flash_min  = (const void *) APP_FLASH_START;
    *flash_max  = (const void *) APP_FLASH_END;
    *app_header = (const struct bootloader_app_header *) APP_HEADER_ADDRESS;

}

void BootloaderDriversOpenlcb_get_flash_page_info(const void *address, const void **page_start, uint32_t *page_length_bytes) {

    uint32_t addr  = (uint32_t)(uintptr_t) address;

    *page_start        = (const void *)(uintptr_t) FLASH_GetErasePageAddress(addr);
    *page_length_bytes = (uint32_t) FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS * 4U;

}

uint16_t BootloaderDriversOpenlcb_erase_flash_page(const void *address) {

    uint32_t addr = (uint32_t)(uintptr_t) address;

    FLASH_Unlock(FLASH_UNLOCK_KEY);
    bool result = FLASH_ErasePage(addr);
    FLASH_Lock();

    return result ? 0 : ERROR_PERMANENT;

}

uint16_t BootloaderDriversOpenlcb_write_flash_bytes(const void *address, const void *data, uint32_t size_bytes) {

    /*
     * dsPIC33 instruction encoding in the firmware image binary:
     *   byte[0] = instruction[7:0]
     *   byte[1] = instruction[15:8]
     *   byte[2] = instruction[23:16]
     *   byte[3] = 0x00  (phantom byte -- ignored)
     *
     * FLASH_WriteDoubleWord24 writes 2 instructions per call:
     *   Data0 → PC address dest     (first instruction)
     *   Data1 → PC address dest + 2 (second instruction)
     * PC address advances by 4 (2 instructions × 2 PC units each).
     * Image pointer advances by 8 (2 instructions × 4 image bytes each).
     */
    uint32_t       dest        = (uint32_t)(uintptr_t) address;
    const uint8_t *source_data = (const uint8_t *) data;
    uint32_t       remaining   = size_bytes;

    FLASH_Unlock(FLASH_UNLOCK_KEY);

    while (remaining >= 8U) {

        uint32_t instr0 = (uint32_t) source_data[0]
                        | ((uint32_t) source_data[1] << 8)
                        | ((uint32_t) source_data[2] << 16);

        uint32_t instr1 = (uint32_t) source_data[4]
                        | ((uint32_t) source_data[5] << 8)
                        | ((uint32_t) source_data[6] << 16);

        if (!FLASH_WriteDoubleWord24(dest, instr0, instr1)) {

            FLASH_Lock();
            return ERROR_PERMANENT;

        }

        dest        += 4U;
        source_data += 8U;
        remaining   -= 8U;

    }

    /* Pad any trailing sub-8-byte remainder with 0xFF (erased flash value). */
    if (remaining > 0U) {

        uint8_t pad[8];
        memset(pad, 0xFF, sizeof(pad));
        memcpy(pad, source_data, remaining);

        uint32_t instr0 = (uint32_t) pad[0]
                        | ((uint32_t) pad[1] << 8)
                        | ((uint32_t) pad[2] << 16);

        uint32_t instr1 = (uint32_t) pad[4]
                        | ((uint32_t) pad[5] << 8)
                        | ((uint32_t) pad[6] << 16);

        if (!FLASH_WriteDoubleWord24(dest, instr0, instr1)) {

            FLASH_Lock();
            return ERROR_PERMANENT;

        }

    }

    FLASH_Lock();
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

    /*
     * dsPIC33 program flash is a separate address space -- data pointers
     * cannot be used to read it.  FLASH_ReadWord24 issues tblrdl/tblrdh
     * instructions and returns a 24-bit instruction word as a uint32_t:
     *   bits[15:0]  = instruction bits [15:0]
     *   bits[23:16] = instruction bits [23:16]
     *   bits[31:24] = undefined (phantom -- do not use)
     *
     * Each instruction provides 3 data bytes; the PC address advances
     * by 2 units per instruction.
     */
    uint8_t  *dest      = (uint8_t *) dest_ram;
    uint32_t  addr      = flash_addr;
    uint32_t  remaining = size_bytes;

    while (remaining >= 3U) {

        uint32_t word = FLASH_ReadWord24(addr);
        *dest++ = (uint8_t)(word        & 0xFFU);
        *dest++ = (uint8_t)((word >> 8) & 0xFFU);
        *dest++ = (uint8_t)((word >> 16) & 0xFFU);
        remaining -= 3U;
        addr      += 2U;

    }

    /* Handle a trailing 1 or 2-byte remainder. */
    if (remaining > 0U) {

        uint32_t word = FLASH_ReadWord24(addr);

        if (remaining >= 1U) {

            *dest++ = (uint8_t)(word & 0xFFU);

        }

        if (remaining >= 2U) {

            *dest++ = (uint8_t)((word >> 8) & 0xFFU);

        }

    }

}

/* ====================================================================== */
/* Streaming CRC-16-IBM helpers (dsPIC33-specific, Harvard architecture)  */
/* ====================================================================== */

/*
 * These tables and functions are private to this file.  They exist because
 * the dsPIC33 Harvard architecture cannot read program flash through a data
 * pointer -- bytes must be fed one at a time from FLASH_ReadWord24().
 *
 * On Von Neumann targets (STM32, MSPM0) the BootloaderCrc_crc3_crc16_ibm()
 * block function in bootloader_crc.c is used instead, and these helpers are
 * not needed there.
 */

static const uint16_t _crc16_ibm_hi[16] = {
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};

static const uint16_t _crc16_ibm_lo[16] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
};

static void _crc16_ibm_add(uint16_t *state, uint8_t data) {

    *state ^= data;
    *state = (*state >> 8) ^ _crc16_ibm_lo[*state & 0x0F] ^
             _crc16_ibm_hi[(*state >> 4) & 0x0F];

}

typedef struct {

    uint16_t state_all;
    uint16_t state_odd;
    uint16_t state_even;
    uint32_t count;

} _crc3_state_t;

static void _crc3_init(_crc3_state_t *crc_state) {

    crc_state->state_all = 0x0000;
    crc_state->state_odd = 0x0000;
    crc_state->state_even = 0x0000;
    crc_state->count = 0;

}

static void _crc3_add_byte(_crc3_state_t *crc_state, uint8_t byte) {

    crc_state->count++;
    _crc16_ibm_add(&crc_state->state_all, byte);

    if (crc_state->count & 1U) {

        _crc16_ibm_add(&crc_state->state_odd, byte);

    } else {

        _crc16_ibm_add(&crc_state->state_even, byte);

    }

}

static void _crc3_finish(const _crc3_state_t *crc_state, uint16_t *checksum) {

    checksum[0] = crc_state->state_all;
    checksum[1] = crc_state->state_odd;
    checksum[2] = crc_state->state_even;

}

/* ====================================================================== */
/* Checksum                                                                */
/* ====================================================================== */

void BootloaderDriversOpenlcb_compute_checksum(const void *data, uint32_t size, uint32_t *checksum) {

    /*
     * On dsPIC33 the library passes a PC address disguised as a const void*.
     * size is the PC-unit span of the region (from pointer arithmetic in
     * bootloader.c); each pair of PC units = one 24-bit instruction = 3 data
     * bytes.  Feed bytes from flash into the streaming CRC one instruction
     * at a time so no large RAM buffer is needed.
     */
    uint32_t addr             = (uint32_t)(uintptr_t) data;
    uint32_t num_instructions = size / 2U;

    _crc3_state_t state;
    _crc3_init(&state);

    for (uint32_t instruction_index = 0U; instruction_index < num_instructions; instruction_index++) {

        uint32_t word = FLASH_ReadWord24(addr);
        _crc3_add_byte(&state, (uint8_t)(word         & 0xFFU));
        _crc3_add_byte(&state, (uint8_t)((word >> 8)  & 0xFFU));
        _crc3_add_byte(&state, (uint8_t)((word >> 16) & 0xFFU));
        addr += 2U;

    }

    uint16_t crc3[3];
    _crc3_finish(&state, crc3);

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
     * TMR2 is configured with a 100 ms period.  MCC's _T2Interrupt
     * increments tmr2_obj.timerElapsed each period and the software
     * counter (TMR2_SoftwareCounterGet) tracks the total elapsed
     * periods.  Casting to uint8_t provides the same wrapping behaviour
     * (0–255) that the bootloader library expects.
     */
    return (uint8_t) TMR2_SoftwareCounterGet();

}

/* ====================================================================== */
/* Identity                                                                */
/* ====================================================================== */

uint64_t BootloaderDriversOpenlcb_get_persistent_node_id(void) {

    /* TODO: Read from protected flash or EEPROM once production programming
     * is in place.  Hardcoded for bringup. */
    return 0x050101012201ULL;

}

/* ====================================================================== */
/* LED                                                                     */
/* ====================================================================== */

void BootloaderDriversOpenlcb_set_status_led(bootloader_led_enum led, bool value) {

    switch (led) {

        case BOOTLOADER_LED_ACTIVE:
            if (value) { IO_BOOTLOADER_LED_ACTIVE_SetHigh(); }
            else        { IO_BOOTLOADER_LED_ACTIVE_SetLow();  }
            break;

        case BOOTLOADER_LED_WRITING:
            if (value) { IO_BOOTLOADER_LED_WRITING_SetHigh(); }
            else        { IO_BOOTLOADER_LED_WRITING_SetLow();  }
            break;

        case BOOTLOADER_LED_CSUM_ERROR:
        case BOOTLOADER_LED_FRAME_LOST:
        case BOOTLOADER_LED_REQUEST:
        default:
            break;

    }

}
