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
 * ESP32.  This header is compiled into BOTH the bootloader project and the
 * application project.
 *
 * On ESP32, variables are placed in RTC slow memory using RTC_NOINIT_ATTR.
 * This region survives both software reset (esp_restart) and deep sleep.
 * The ESP-IDF linker script already defines the rtc_noinit segment, so no
 * linker script changes are needed.
 *
 * A struct is used (rather than bare globals) to prevent the linker from
 * reordering the fields independently in each binary.  Both projects
 * compile this same file, so the struct layout is identical and the
 * bootloader reads exactly what the application wrote.
 *
 * HOW THE MAGIC VALUE WORKS:
 *   Before issuing esp_restart() to enter the bootloader, the application
 *   writes BOOTLOADER_REQUEST_MAGIC into request_flag and its current CAN
 *   alias into cached_alias.  After the reset the bootloader checks
 *   request_flag; if it matches it clears the flag and enters firmware-
 *   update mode using the cached alias.  A 32-bit magic value (not a
 *   single bit) makes a random power-on match statistically negligible.
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

/** Written to request_flag by the application before reset. */
#define BOOTLOADER_REQUEST_MAGIC  0xB00710ADUL

/* ====================================================================== */
/* Shared RAM struct                                                       */
/* ====================================================================== */

/**
 * Bootloader/application handshake data.
 *
 * Wrapping the fields in a struct prevents the linker from reordering
 * them independently between the bootloader and application builds.
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
