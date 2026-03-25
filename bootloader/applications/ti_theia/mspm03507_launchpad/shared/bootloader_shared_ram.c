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
 * Shared RAM variable definitions for the bootloader/application handshake.
 *
 * This file is compiled into BOTH the bootloader project and the application
 * project.  It defines the variables declared as 'extern' in bootloader_types.h.
 * Both linker scripts must map the .noinit section to the same fixed SRAM
 * address (SHARED_NOINIT) so the bootloader and application see the same
 * physical memory.
 *
 * Variables in .noinit are NOT zeroed by C startup code, so their values
 * survive a software reset (NVIC_SystemReset).  See bootloader_types.h for
 * a full explanation of the magic value handshake.
 *
 * @author Jim Kueneman
 * @date 24 Mar 2026
 */

#include <stdint.h>

/*
 * bootloader_request_flag — the application writes BOOTLOADER_REQUEST_MAGIC
 * (0xB00710AD) here before calling NVIC_SystemReset().  The bootloader reads
 * it on startup to decide whether to stay in bootloader mode.
 *
 * bootloader_cached_alias — the application writes its current 12-bit CAN
 * alias here before resetting.  The bootloader reads it to skip the full
 * CID negotiation and reuse the alias for faster startup.
 *
 * Both variables are placed in the .noinit section.  The linker script in
 * each project maps .noinit to SHARED_NOINIT — a small fixed region at the
 * top of SRAM that both binaries agree on.
 *
 * =========================================================================
 * PLATFORM PORTING NOTES
 *
 * When porting to a different chip/compiler, replace the variable definitions
 * below with the appropriate attribute and update the linker script.  The
 * key requirement is:
 *   1. The variable must NOT be zeroed by C startup code
 *   2. Both binaries must place it at the SAME physical RAM address
 *
 * -------------------------------------------------------------------------
 * TI CCS / MSPM0 (tiarmclang — this file):
 *
 *   C source:
 *     volatile uint32_t bootloader_request_flag __attribute__((section(".noinit")));
 *
 *   Linker script (.cmd):
 *     SHARED_NOINIT (RW) : origin = 0x20207FF0, length = 0x00000010
 *     SECTIONS { .noinit : > SHARED_NOINIT }
 *
 * -------------------------------------------------------------------------
 * Any Cortex-M with GCC (arm-none-eabi-gcc):
 * Covers STM32, NXP LPC/i.MX RT, Nordic nRF52/nRF53, Renesas RA, etc.
 *
 *   C source:
 *     volatile uint32_t bootloader_request_flag __attribute__((section(".noinit")));
 *
 *   Linker script (.ld) — add after .bss, before .stack:
 *     .noinit (NOLOAD) :
 *     {
 *         KEEP(*(.noinit))
 *         KEEP(*(.noinit.*))
 *     } > SHARED_NOINIT
 *     (NOLOAD) tells the linker not to emit load-time initialization data.
 *
 * -------------------------------------------------------------------------
 * Any Cortex-M with IAR (EWARM):
 * Covers STM32, NXP, Nordic, Renesas RA, etc.
 *
 *   C source:
 *     __no_init volatile uint32_t bootloader_request_flag;
 *
 *   Linker script (.icf):
 *     define region SHARED_NOINIT = mem:[from 0x20007FF0 to 0x20007FFF];
 *     place in SHARED_NOINIT { section .noinit };
 *
 * -------------------------------------------------------------------------
 * AVR with avr-gcc:
 *
 *   C source:
 *     volatile uint32_t bootloader_request_flag __attribute__((section(".noinit")));
 *
 *   Linker script:
 *     No changes needed — avr-libc's default linker script already
 *     defines a .noinit section mapped to internal SRAM.
 *     To pin the address, use __attribute__((section(".noinit"), address(0x0100)))
 *     or define a custom memory region.
 *
 * -------------------------------------------------------------------------
 * dsPIC / PIC24 with XC16:
 *
 *   C source:
 *     volatile uint32_t bootloader_request_flag __attribute__((persistent address(0x1000)));
 *
 *   Linker script:
 *     No changes needed — XC16 supports address() in the attribute
 *     to pin the variable to a specific RAM address.
 *
 * -------------------------------------------------------------------------
 * PIC32 with XC32:
 *
 *   C source:
 *     volatile uint32_t bootloader_request_flag __attribute__((persistent));
 *
 *   Linker script:
 *     Pin the .persist section to a fixed address in both projects.
 *
 * -------------------------------------------------------------------------
 * ESP32 with ESP-IDF (xtensa-gcc or riscv-gcc):
 *
 *   C source:
 *     #include "esp_attr.h"
 *     RTC_NOINIT_ATTR volatile uint32_t bootloader_request_flag;
 *     This places the variable in RTC slow memory, which survives
 *     both software reset and deep sleep.
 *
 *   Linker script:
 *     No changes needed — ESP-IDF's default linker script already
 *     defines the rtc_noinit segment.
 *
 * -------------------------------------------------------------------------
 * Renesas RX with CC-RX:
 *
 *   C source:
 *     #pragma section B BNI
 *     volatile uint32_t bootloader_request_flag;
 *     #pragma section
 *
 *   Linker configuration (.lnk):
 *     -start=BNI/04000000
 *     Replace 04000000 with the start address of your RAM region.
 *     The BNI section must NOT appear in the initialization table.
 * =========================================================================
 */

volatile uint32_t bootloader_request_flag __attribute__((section(".noinit")));
volatile uint16_t bootloader_cached_alias __attribute__((section(".noinit")));
