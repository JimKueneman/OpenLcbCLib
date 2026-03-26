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
 * @file bootloader_isr_redirect.c
 *
 * Virtual Interrupt Vector Table (VIVT) for the dsPIC33 bootloader.
 *
 * =====================================================================
 * WHY THIS FILE EXISTS -- THE dsPIC33 INTERRUPT PROBLEM
 * =====================================================================
 *
 * ARM Cortex-M processors (STM32, MSPM0, etc.) have a Vector Table
 * Offset Register (VTOR).  When the bootloader jumps to the application
 * it simply writes the application's vector table address into VTOR, and
 * from that moment on ALL interrupts are handled entirely by the
 * application.  Clean, simple, no special tricks needed.
 *
 * dsPIC33 has NO equivalent of VTOR.  The hardware Interrupt Vector
 * Table (IVT) lives at a fixed address in program flash (starting near
 * 0x000004).  Whichever binary is programmed at the start of flash -- in
 * our case the bootloader -- permanently owns the IVT for the life of
 * the power cycle.  Even after jump_to_application() causes the CPU to
 * execute application code, ALL interrupts still vector through the
 * bootloader's IVT.  The application cannot directly receive hardware
 * interrupts.
 *
 * =====================================================================
 * THE SOLUTION: VIRTUAL INTERRUPT VECTOR TABLE (VIVT)
 * =====================================================================
 *
 * Instead of handling each interrupt themselves, the bootloader's ISR
 * stubs look up a corresponding function pointer in a shared-RAM table
 * (bootloader_vivt_jumptable, defined in bootloader_shared_ram.c).
 *
 *   - If the pointer is NULL  --> the bootloader handles the interrupt
 *     itself (spin forever on a trap, or do nothing on a peripheral
 *     interrupt during firmware-update mode).
 *
 *   - If the pointer is non-NULL --> call it.  This forwards the
 *     interrupt transparently to whatever the application registered.
 *
 * The application fills in these pointers during its startup sequence
 * with global interrupts disabled so there is no race between a hardware
 * interrupt firing and the pointer being written half-way.
 *
 * =====================================================================
 * VIVT COMPLETE LIFECYCLE
 * =====================================================================
 *
 * 1. Power-on or brownout reset
 *    - Physical SRAM contents are random.
 *    - BootloaderDriversOpenlcb_initialize_hardware() detects POR/BOR
 *      via RCON and zeros every VIVT slot, plus the magic flag and
 *      cached alias.  Random SRAM cannot accidentally point a VIVT slot
 *      at garbage code.
 *
 * 2. Bootloader runs (firmware_active = 0 or 1)
 *    - All VIVT slots remain NULL.
 *    - Trap ISRs spin (no app registered yet -- a trap here is fatal).
 *    - Peripheral ISRs (TMR2) are no-ops.  MCC's own timer bookkeeping
 *      still runs so get_100ms_timer_tick() works normally.
 *
 * 3. jump_to_application()
 *    - The bootloader calls the application's reset vector.
 *    - CPU now executes application code.
 *    - Hardware IVT still points at bootloader stubs.
 *    - All VIVT slots are still NULL.
 *
 * 4. Application startup (interrupts globally disabled by startup code)
 *    - Application writes its handler addresses into the VIVT table.
 *    - Application re-enables global interrupts.
 *    - From this point all hardware interrupts are forwarded to the
 *      application transparently through the VIVT.
 *
 * 5. Application receives a Freeze command (firmware update requested)
 *    - callbacks_config_mem.c::CallbacksConfigMem_freeze() runs.
 *    - CRITICAL SEQUENCE (must happen in this order):
 *        a) Disable global interrupts  (__builtin_disi(0x3FFF))
 *        b) Zero every VIVT slot
 *        c) Cache the current CAN alias in bootloader_cached_alias
 *        d) Set bootloader_request_flag = BOOTLOADER_REQUEST_MAGIC
 *        e) asm("RESET")
 *
 *    WHY zeroing the VIVT before RESET is mandatory:
 *    A software reset (asm RESET) preserves SRAM.  If the VIVT slots
 *    still hold the application's addresses when the bootloader restarts,
 *    the very first interrupt that fires during alias negotiation (e.g.
 *    the TMR2 100ms tick) will call into dead application code -- likely
 *    crashing or silently corrupting state.  The VIVT MUST be clean.
 *
 *    WHY initialize_hardware() does NOT clear the VIVT on software reset:
 *    It deliberately skips the clear on software reset so it can read
 *    the magic flag (step d) and recover the cached alias (step c) that
 *    the application just wrote.  Those values are also in SRAM and
 *    would be wiped if we cleared everything.  The application clearing
 *    the VIVT (step b) before writing the magic flag (step d) is what
 *    makes this safe.
 *
 * =====================================================================
 * TMR2: WHY WE USE TMR2_SetInterruptHandler() INSTEAD OF _T2Interrupt
 * =====================================================================
 *
 * MCC generates a strong (non-weak) _T2Interrupt symbol in tmr2.c.
 * We cannot define our own _T2Interrupt in a separate file because the
 * linker would see two strong definitions and produce a duplicate-symbol
 * error.
 *
 * However, MCC's _T2Interrupt already routes through a swappable
 * function pointer (TMR2_InterruptHandler) via TMR2_SetInterruptHandler().
 * We install _vivt_t2_proxy as that handler in
 * BootloaderIsrRedirect_initialize().
 *
 * The resulting call chain on each TMR2 interrupt:
 *
 *   hardware IVT --> MCC _T2Interrupt --> _vivt_t2_proxy
 *                                              |
 *                        NULL?  no-op          |   non-NULL?
 *                        (bootloader phase)    +--> app handler
 *                                                   (app running phase)
 *
 * MCC's _T2Interrupt unconditionally updates tmr2_obj.timerElapsed
 * AFTER calling the handler, so the bootloader's 100ms tick counter
 * (TMR2_SoftwareCounterGet) works correctly even while the proxy is
 * a no-op.
 *
 * =====================================================================
 * CAN (_C1Interrupt): HANDLED DIFFERENTLY
 * =====================================================================
 *
 * MCC's _C1Interrupt does not expose a swappable function-pointer hook
 * the way TMR2 does.  We therefore hand-edited
 * mcc_generated_files/can1.c to add a VIVT check near the top of
 * _C1Interrupt.  See that file for the exact three lines and the
 * comment marking the hand-edit so it can be re-applied after MCC
 * regeneration.
 *
 * The bootloader polls CAN (does not need _C1Interrupt at all), so we
 * also unchecked the CAN interrupt in MCC Easy Setup.  This prevents
 * MCC from regenerating _C1Interrupt in can1.c, freeing the symbol for
 * the application's own dspic33_can_drivers.c to define it with VIVT
 * support built in.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include <xc.h>

#include "bootloader_isr_redirect.h"
#include "../../shared/bootloader_shared_ram.h"
#include "../mcc_generated_files/tmr2.h"

/* ====================================================================== */
/* TMR2 VIVT proxy                                                         */
/*                                                                         */
/* Installed as MCC's TMR2_InterruptHandler via TMR2_SetInterruptHandler() */
/* in BootloaderIsrRedirect_initialize() below.                            */
/*                                                                         */
/* When the VIVT slot is NULL (bootloader phase): returns immediately.     */
/* MCC's _T2Interrupt still updates tmr2_obj.timerElapsed after this       */
/* proxy returns, so the 100ms tick counter continues to work.             */
/*                                                                         */
/* When the VIVT slot is non-NULL (app running): calls the application's   */
/* registered handler transparently.                                       */
/* ====================================================================== */

static void _vivt_t2_proxy(void) {

    if (bootloader_vivt_jumptable.timer_2_handler) {

        bootloader_vivt_jumptable.timer_2_handler();

    }

}

/* ====================================================================== */
/* Initialization                                                          */
/*                                                                         */
/* Call once from BootloaderDriversOpenlcb_initialize_hardware() before    */
/* any interrupts are enabled.  Wires the TMR2 proxy into MCC's function   */
/* pointer slot so the hardware _T2Interrupt flows through our VIVT.       */
/* ====================================================================== */

void BootloaderIsrRedirect_initialize(void) {

    TMR2_SetInterruptHandler(_vivt_t2_proxy);

}

/* ====================================================================== */
/* Trap overrides                                                          */
/*                                                                         */
/* MCC marks all trap handlers with __attribute__((weak)) so we can define */
/* strong replacements here without touching generated files.              */
/*                                                                         */
/* Each trap stub:                                                         */
/*   1. Clears the hardware trap flag.  On dsPIC33 a trap re-fires        */
/*      immediately if its flag is not cleared before returning from the   */
/*      ISR -- so we clear it first regardless of which path we take.     */
/*   2. If the application registered a handler, call it.  The application */
/*      can log the trap, blink an LED, attempt recovery, etc.            */
/*   3. If no handler is registered (bootloader phase), spin forever.     */
/*      A trap during bootloader execution is unrecoverable; spinning      */
/*      safely halts the device so a watchdog or power cycle can recover.  */
/* ====================================================================== */

void __attribute__((interrupt, no_auto_psv)) _OscillatorFail(void) {

    INTCON1bits.OSCFAIL = 0;

    if (bootloader_vivt_jumptable.oscillatorfail_handler) {

        bootloader_vivt_jumptable.oscillatorfail_handler();

    } else {

        while (1);

    }

}

void __attribute__((interrupt, no_auto_psv)) _AddressError(void) {

    INTCON1bits.ADDRERR = 0;

    if (bootloader_vivt_jumptable.addresserror_handler) {

        bootloader_vivt_jumptable.addresserror_handler();

    } else {

        while (1);

    }

}

void __attribute__((interrupt, no_auto_psv)) _StackError(void) {

    INTCON1bits.STKERR = 0;

    if (bootloader_vivt_jumptable.stackerror_handler) {

        bootloader_vivt_jumptable.stackerror_handler();

    } else {

        while (1);

    }

}

void __attribute__((interrupt, no_auto_psv)) _MathError(void) {

    INTCON1bits.MATHERR = 0;

    if (bootloader_vivt_jumptable.matherror_handler) {

        bootloader_vivt_jumptable.matherror_handler();

    } else {

        while (1);

    }

}

void __attribute__((interrupt, no_auto_psv)) _DMACError(void) {

    INTCON1bits.DMACERR = 0;

    if (bootloader_vivt_jumptable.dmacerror_handler) {

        bootloader_vivt_jumptable.dmacerror_handler();

    } else {

        while (1);

    }

}
