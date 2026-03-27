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
 * @file bootloader_crc.h
 *
 * CRC-16-IBM checksum functions for the standalone bootloader.
 * Ported from OpenMRN Crc.hxx/Crc.cxx (Balazs Racz, 2014).
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_CRC_BOOTLOADER_CRC__
#define __BOOTLOADER_LIB_CRC_BOOTLOADER_CRC__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         *     Computes CRC-16-IBM (ANSI) over a block of data.
         *
         *     Polynomial 0xA001 (reversed), init 0x0000, no final XOR.
         *     Test vector: "123456789" -> 0xBB3D.
         *
         *     @param data pointer to data to checksum
         *     @param length_bytes number of bytes
         *     @return CRC-16-IBM value
         */
    extern uint16_t BootloaderCrc_crc16_ibm(const void *data, uint32_t length_bytes);

        /**
         *     Computes triple CRC-16-IBM over a block of data.
         *
         *     checksum[0] = CRC of all bytes
         *     checksum[1] = CRC of odd-indexed bytes (1st, 3rd, 5th, ...)
         *     checksum[2] = CRC of even-indexed bytes (2nd, 4th, 6th, ...)
         *
         *     Compatible with TI Tiva ROM_Crc16Array3() hardware CRC function.
         *
         *     @param data pointer to data to checksum
         *     @param length_bytes number of bytes
         *     @param checksum output array of 3 uint16_t values
         */
    extern void BootloaderCrc_crc3_crc16_ibm(const void *data, uint32_t length_bytes, uint16_t *checksum);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_CRC_BOOTLOADER_CRC__ */
