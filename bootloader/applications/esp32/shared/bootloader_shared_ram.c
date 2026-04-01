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
 * Shared RAM variable definition for the bootloader/application handshake
 * on ESP32.
 *
 * RTC_NOINIT_ATTR places the struct in the RTC slow memory "rtc_noinit"
 * segment.  This region is:
 *   - NOT zeroed by the C startup code (unlike normal BSS)
 *   - Preserved across esp_restart() (software reset)
 *   - Preserved across deep sleep wake
 *   - Random/undefined after a true power-on or brownout
 *
 * The bootloader's initialize_hardware() clears the struct on cold boot
 * (power-on) to prevent random values from triggering bootloader mode.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include "esp_attr.h"
#include "bootloader_shared_ram.h"

/**
 * Single instance of the handshake struct, placed in RTC NOINIT memory.
 * Both the bootloader and application projects compile this file so they
 * share the same struct layout at the same physical address.
 */
RTC_NOINIT_ATTR bootloader_shared_ram_t bootloader_shared_ram;
