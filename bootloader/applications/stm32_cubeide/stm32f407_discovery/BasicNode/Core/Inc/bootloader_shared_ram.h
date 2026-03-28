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
 * @file bootloader_shared_ram.h
 *
 * Shared definitions for the bootloader <-> application RAM handshake.
 *
 * This header is included by BOTH the bootloader (via bootloader_types.h)
 * and the application.  It contains only the magic value constant and the
 * extern declarations for the shared RAM variables.  No bootloader internals.
 *
 * The actual variable definitions (with platform-specific .noinit attributes)
 * live in the matching bootloader_shared_ram.c, which is compiled into both
 * projects.
 *
 * @author Jim Kueneman
 * @date 24 Mar 2026
 */

#ifndef __BOOTLOADER_SHARED_RAM__
#define __BOOTLOADER_SHARED_RAM__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* ================================================================== */
    /* Application-to-bootloader drop-back ("magic value" handshake)       */
    /* ================================================================== */

    /*
     * HOW THIS WORKS — read this if you are new to bootloaders:
     *
     * Problem:
     *   The application is running and the user wants to update firmware.
     *   The bootloader is the only code that can write flash, so we need
     *   to restart the chip and land in the bootloader instead of the app.
     *   But how does the bootloader know "the app asked me to run" vs.
     *   "this is a normal power-on and I should jump to the app"?
     *
     * Solution — a "magic value" in RAM:
     *   Before resetting the chip, the application writes a specific
     *   32-bit number (the "magic value") into a known RAM variable.
     *   After the reset the bootloader checks that variable:
     *     - If it contains the magic value  → stay in bootloader mode
     *     - If it contains anything else    → normal boot (jump to app)
     *
     * Why does RAM survive a reset?
     *   On most microcontrollers, a software reset (NVIC_SystemReset on
     *   Cortex-M, asm("RESET") on dsPIC, watchdog reset on AVR) does NOT
     *   erase SRAM.  Only a full power-on or brown-out wipes RAM.  So a
     *   value written just before a software reset will still be there
     *   when the bootloader starts up after that reset.
     *
     * Why a 32-bit magic value instead of a single 1-bit flag?
     *   After a power-on reset, SRAM contains random garbage.  A single
     *   bit flag has a 50% chance of accidentally being 1, which would
     *   falsely tell the bootloader "the app asked me to run."  A 32-bit
     *   magic value like 0xB00710AD has only a 1-in-4-billion chance of
     *   appearing in random garbage.  Much safer.
     *
     * Platform notes:
     *   The variable must be placed in a RAM section that the C startup
     *   code does NOT zero-initialize.  The section name and attribute
     *   vary by compiler:
     *     - GCC (Cortex-M, AVR):  __attribute__((section(".noinit")))
     *     - XC16 (dsPIC):         __attribute__((persistent))
     *     - IAR:                  __no_init
     *     - TI CCS (ARM):        __attribute__((section(".noinit")))
     *   See bootloader_shared_ram.c for detailed per-platform examples.
     *
     * Usage in the application (on Freeze command):
     *
     *   #include "bootloader_shared_ram.h"
     *
     *   bootloader_shared_ram.cached_alias = my_current_alias;
     *   bootloader_shared_ram.request_flag = BOOTLOADER_REQUEST_MAGIC;
     *   NVIC_SystemReset();   // or asm("RESET") on dsPIC, etc.
     */

    /**
     *     Magic value written to bootloader_shared_ram.request_flag by the application
     *     before a software reset to request bootloader mode.  The hex
     *     digits spell "BOOTLOAD" (0xB00710AD) as a mnemonic.
     */
    #define BOOTLOADER_REQUEST_MAGIC 0xB00710ADUL

    /**
     *     Shared RAM struct for application-to-bootloader communication.
     *
     *     Both variables are wrapped in a single struct so the linker
     *     produces identical memory layout in both binaries.  Without the
     *     struct, the linker is free to reorder the individual variables
     *     within the .noinit section -- and it DOES: Debug vs Release
     *     builds, different object file link order, or different
     *     optimisation levels can all cause the variables to swap
     *     positions.  The app writes the alias at one address and the
     *     bootloader reads it from a different address.  The handshake
     *     silently fails.
     *
     *     request_flag: The application writes BOOTLOADER_REQUEST_MAGIC
     *     here before issuing a software reset.  The bootloader's
     *     is_bootloader_requested() driver checks this value on startup,
     *     clears it, and returns BOOTLOADER_REQUESTED_BY_APP if it
     *     matches.
     *
     *     cached_alias: The application writes its current 12-bit CAN
     *     alias here before issuing a software reset alongside
     *     BOOTLOADER_REQUEST_MAGIC.  The bootloader's get_cached_alias()
     *     driver reads this value so it can skip the full CID negotiation
     *     and 200ms wait -- the alias is still valid on the bus.  Set to 0
     *     if no alias is available (cold boot, button entry, or corrupted
     *     app).  The bootloader will then negotiate a new alias from
     *     scratch.
     *
     *     Defined (with platform-specific .noinit placement) in
     *     bootloader_shared_ram.c.
     */
    typedef struct {

        volatile uint32_t request_flag;
        volatile uint16_t cached_alias;

    } bootloader_shared_ram_t;

    extern bootloader_shared_ram_t bootloader_shared_ram;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_SHARED_RAM__ */
