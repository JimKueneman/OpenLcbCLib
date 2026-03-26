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
 * @file bootloader_crc.c
 *
 * CRC-16-IBM checksum implementation for the standalone bootloader.
 * Ported from OpenMRN Crc.cxx (Balazs Racz, 2014).
 *
 * Table-driven nibble-at-a-time algorithm. Two 16-entry tables = 64 bytes ROM.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include "bootloader_crc.h"

/** Translation table for CRC-16-IBM, high nibble. */
static const uint16_t _CRC16_IBM_HI[16] = {

    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,

};

/** Translation table for CRC-16-IBM, low nibble. */
static const uint16_t _CRC16_IBM_LO[16] = {

    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,

};

/**
 *     Appends one byte to the CRC-16-IBM state.
 *
 *     @param state pointer to current CRC state (modified in place)
 *     @param data byte to add
 */
static void _crc16_ibm_add(uint16_t *state, uint8_t data) {

    *state ^= data;
    *state = (*state >> 8) ^ _CRC16_IBM_LO[*state & 0x0F] ^
            _CRC16_IBM_HI[(*state >> 4) & 0x0F];

}

uint16_t BootloaderCrc_crc16_ibm(const void *data, uint32_t length_bytes) {

    const uint8_t *payload = (const uint8_t *) data;
    uint16_t state = 0x0000;

    for (uint32_t i = 0; i < length_bytes; i++) {

        _crc16_ibm_add(&state, payload[i]);

    }

    return state;

}

void BootloaderCrc_crc3_crc16_ibm(const void *data, uint32_t length_bytes, uint16_t *checksum) {

    const uint8_t *payload = (const uint8_t *) data;
    uint16_t state_all = 0x0000;
    uint16_t state_odd = 0x0000;
    uint16_t state_even = 0x0000;

    for (uint32_t i = 1; i <= length_bytes; i++) {

        _crc16_ibm_add(&state_all, payload[i - 1]);

        if (i & 1) {

            _crc16_ibm_add(&state_odd, payload[i - 1]);

        } else {

            _crc16_ibm_add(&state_even, payload[i - 1]);

        }

    }

    checksum[0] = state_all;
    checksum[1] = state_odd;
    checksum[2] = state_even;

}
