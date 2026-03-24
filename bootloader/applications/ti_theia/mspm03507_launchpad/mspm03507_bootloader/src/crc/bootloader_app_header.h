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
 * @file bootloader_app_header.h
 *
 * Application header structure and checksum validation for the standalone
 * bootloader. The app_header is embedded in the application firmware image
 * at a linker-defined address and contains pre/post checksums that the
 * bootloader validates before jumping to the application.
 *
 * Ported from OpenMRN ApplicationChecksum.hxx (Balazs Racz, 2017).
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_CRC_BOOTLOADER_APP_HEADER__
#define __BOOTLOADER_LIB_CRC_BOOTLOADER_APP_HEADER__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /** Number of 32-bit words in one checksum block. */
    #define BOOTLOADER_CHECKSUM_COUNT 4

    /**
     *     Application header embedded in the firmware image.
     *
     *     The linker places this struct at a known address in the application
     *     flash region. A post-link tool computes the checksums and patches
     *     them into the binary.
     *
     *     checksum_pre covers bytes from flash_min to the start of this struct.
     *     checksum_post covers bytes from end of this struct to app_size.
     */
    typedef struct {

        uint32_t app_size;
        uint32_t checksum_pre[BOOTLOADER_CHECKSUM_COUNT];
        uint32_t checksum_post[BOOTLOADER_CHECKSUM_COUNT];

    } bootloader_app_header_t;

    /**
     *     Validates the application checksum in flash.
     *
     *     Calls BootloaderDriver_get_flash_boundaries() to locate the flash
     *     region and app_header, then calls BootloaderDriver_checksum_data()
     *     to compute and compare pre/post checksums.
     *
     *     @return true if the application checksum is valid, false otherwise
     */
    extern bool Bootloader_check_application_checksum(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_CRC_BOOTLOADER_APP_HEADER__ */
