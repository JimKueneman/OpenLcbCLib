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
 * Shared RAM definitions for the bootloader <-> application handshake on
 * dsPIC33EP512MC506.  This header is compiled into BOTH the bootloader
 * project and the application project.
 *
 * RAM layout (dsPIC33EP512MC506, RAM starts at 0x1000):
 *
 *   0x1000  bootloader_cached_alias  (2 bytes)  -- CAN alias from app before reset
 *   0x1002  bootloader_request_flag  (4 bytes)  -- magic value 0xB00710AD
 *   0x1006  bootloader_vivt_jumptable            -- 7 x 4-byte function pointers
 *
 * All addresses are below 0x2000 (near data window) so no far attribute is
 * needed.  All variables use __attribute__((persistent)) so they are not
 * zeroed by the C startup code and survive a software reset.
 *
 * HOW THE MAGIC VALUE WORKS:
 *   Before issuing a software reset to enter the bootloader, the application
 *   writes BOOTLOADER_REQUEST_MAGIC into bootloader_request_flag and its
 *   current CAN alias into bootloader_cached_alias.  After the reset the
 *   bootloader checks bootloader_request_flag; if it matches it clears the
 *   flag and enters firmware-update mode using the cached alias.  A 32-bit
 *   magic value is used (not a single bit) so random SRAM garbage after
 *   power-on cannot accidentally trigger bootloader mode.
 *
 * HOW INTERRUPT REDIRECT (VIVT) WORKS:
 *   On dsPIC33 the hardware IVT is at a fixed flash address owned by the
 *   bootloader.  After jump_to_application() the application code runs but
 *   all interrupts still fire into the bootloader's ISR stubs.  Each stub
 *   checks its corresponding function pointer in bootloader_vivt_jumptable;
 *   if non-NULL it calls it.  The application fills in those pointers during
 *   its startup so normal interrupt-driven operation resumes transparently.
 *   A NULL pointer means the redirect is not yet set up -- no separate
 *   interrupt_redirect flag is required.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#ifndef __BOOTLOADER_SHARED_RAM__
#define __BOOTLOADER_SHARED_RAM__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ====================================================================== */
/* Magic value                                                             */
/* ====================================================================== */

/** Written to bootloader_request_flag by the application before reset. */
#define BOOTLOADER_REQUEST_MAGIC  0xB00710ADUL

/* ====================================================================== */
/* VIVT type                                                               */
/* ====================================================================== */

typedef void (*bootloader_isr_handler_t)(void);

/**
 * Virtual Interrupt Vector Table.
 * The application fills in these function pointers during its startup.
 * Each bootloader ISR stub calls through here when the pointer is non-NULL.
 * NULL means the redirect is not yet set up -- no separate flag needed.
 */
typedef struct {
    bootloader_isr_handler_t oscillatorfail_handler;
    bootloader_isr_handler_t addresserror_handler;
    bootloader_isr_handler_t stackerror_handler;
    bootloader_isr_handler_t matherror_handler;
    bootloader_isr_handler_t dmacerror_handler;
    bootloader_isr_handler_t timer_2_handler;
    bootloader_isr_handler_t can1_handler;
} vivt_jumptable_t;

/* ====================================================================== */
/* Shared RAM variable declarations                                        */
/* ====================================================================== */

extern volatile uint16_t          bootloader_cached_alias;
extern volatile uint32_t          bootloader_request_flag;
extern volatile vivt_jumptable_t  bootloader_vivt_jumptable;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_SHARED_RAM__ */
