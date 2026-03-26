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
 * @file bootloader_shared_ram.c
 *
 * Shared RAM variable definitions for the bootloader/application handshake
 * on dsPIC33EP512MC506.
 *
 * =====================================================================
 * WHY THIS FILE EXISTS -- SURVIVING A SOFTWARE RESET
 * =====================================================================
 *
 * When the bootloader wants to update firmware it normally stays resident
 * across a power cycle.  But on dsPIC33 the preferred drop-back path is
 * for the RUNNING APPLICATION to initiate the reset:
 *
 *   1. OpenLCB Config Tool sends a Freeze command to the application.
 *   2. Application's freeze callback (callbacks_config_mem.c) prepares
 *      the handshake data and calls  asm("RESET").
 *   3. The CPU resets and the bootloader starts fresh.
 *   4. The bootloader reads the handshake data and immediately enters
 *      firmware-update mode -- without the Config Tool needing to send
 *      Freeze again.
 *
 * The problem: how does data survive step 2 -> step 3?
 *
 * On a hardware power-on or brownout reset the SRAM contents are random
 * and cannot be trusted.  But on a SOFTWARE reset (asm RESET) the CPU
 * resets while SRAM is left completely intact.  Variables placed in SRAM
 * retain their values across the reset boundary.
 *
 * XC16 provides __attribute__((persistent)) to prevent the C startup
 * code from zeroing a variable.  Normally startup zeros all BSS and
 * copies initialized data from flash.  A persistent variable is skipped
 * by that process, so it holds whatever value was last written -- even
 * across a software reset.
 *
 * =====================================================================
 * FIXED ADDRESS ASSIGNMENT
 * =====================================================================
 *
 * Both the bootloader and the application are separate binaries.  They
 * are linked independently by two separate linker scripts.  For them to
 * share a variable they must agree on its exact RAM address -- the linker
 * for each project must place the variable at the SAME physical location.
 *
 * We use __attribute__((persistent, address(0xNNNN))) to pin each
 * variable to a specific address.  Both projects compile this same
 * bootloader_shared_ram.c file, so both binaries place the variables at
 * identical addresses and see the same physical bytes of SRAM.
 *
 * RAM layout chosen (dsPIC33EP512MC506, SRAM starts at 0x1000):
 *
 *   0x1000  bootloader_cached_alias  (2 bytes) -- CAN alias from app
 *   0x1002  bootloader_request_flag  (4 bytes) -- magic handshake value
 *   0x1006  bootloader_vivt_jumptable           -- 7 x 4-byte fn ptrs
 *
 * These addresses are at the very bottom of SRAM so they do not conflict
 * with normal stack or heap usage (both grow from the top of SRAM).
 *
 * =====================================================================
 * POWER-ON SAFETY
 * =====================================================================
 *
 * After a true power-on or brownout reset, these locations in SRAM
 * contain random garbage.  A random value that happens to equal
 * BOOTLOADER_REQUEST_MAGIC (0xB00710AD) would falsely trigger bootloader
 * mode on every power-on.
 *
 * BootloaderDriversOpenlcb_initialize_hardware() checks RCONbits.POR
 * and RCONbits.BOR on startup.  If either is set it means we came from
 * a true power-off/brownout, so it zeros all shared RAM variables
 * explicitly before reading them.  This eliminates the garbage-match
 * false-positive.
 *
 * On a software reset neither POR nor BOR is set, so initialize_hardware
 * leaves the shared RAM untouched, allowing the bootloader to read the
 * values the application wrote just before calling asm("RESET").
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include <stdint.h>
#include "bootloader_shared_ram.h"

/*
 * CAN alias cached by the application before it resets into the bootloader.
 *
 * The application writes its current 12-bit CAN alias here just before
 * calling asm("RESET").  The bootloader reads this in
 * BootloaderDriversCan_get_cached_alias_passed_from_application() and
 * uses it to skip the alias-negotiation phase, reusing the alias the
 * network already knows about.  This makes the drop-back seamless from
 * the Config Tool's perspective.
 *
 * Cleared to 0 after the bootloader reads it so a subsequent reset does
 * not falsely reuse a stale alias.
 */
volatile uint16_t bootloader_cached_alias
    __attribute__((persistent, address(0x1000)));

/*
 * Magic handshake flag written by the application before reset.
 *
 * The application writes BOOTLOADER_REQUEST_MAGIC (0xB00710AD) here
 * and then calls asm("RESET").  The bootloader checks this value in
 * BootloaderDriversOpenlcb_is_bootloader_requested():
 *   - Matches magic --> clear the flag, return BOOTLOADER_REQUESTED_BY_APP
 *     (the Config Tool already sent Freeze; bootloader starts with
 *     firmware_active = 1 and the CT can begin data transfer immediately).
 *   - Does not match --> BOOTLOADER_NOT_REQUESTED (normal boot, jump to app).
 *
 * Using a 32-bit magic value (not a single bit) makes a random power-on
 * SRAM match statistically negligible (1 in 2^32).
 */
volatile uint32_t bootloader_request_flag
    __attribute__((persistent, address(0x1002)));

/*
 * Virtual Interrupt Vector Table (VIVT).
 *
 * dsPIC33 has a fixed hardware IVT owned by the bootloader.  After
 * jump_to_application() the application code runs but all hardware
 * interrupts still fire into the bootloader's ISR stubs.  Each stub
 * checks the corresponding slot here and calls through if non-NULL.
 *
 * The application fills these slots during startup with global interrupts
 * disabled.  The bootloader clears them on POR/BOR (in initialize_hardware)
 * to prevent random SRAM from triggering a call to garbage code.
 *
 * The application MUST also clear them (again with interrupts disabled)
 * before calling asm("RESET") to drop back into the bootloader, so the
 * bootloader's ISR stubs see NULL during the firmware-update phase.
 * See CallbacksConfigMem_freeze() in the application project for the
 * full drop-back sequence.
 */
volatile vivt_jumptable_t bootloader_vivt_jumptable
    __attribute__((persistent, address(0x1006)));
