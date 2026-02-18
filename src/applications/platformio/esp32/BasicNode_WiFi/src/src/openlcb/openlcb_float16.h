/** \copyright
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
 * @file openlcb_float16.h
 * @brief IEEE 754 half-precision (float16) conversion utilities
 *
 * @details The OpenLCB Train Control Protocol encodes speed as IEEE 754
 * binary16 (half-precision float). The sign bit represents direction
 * (0 = forward, 1 = reverse). The value is speed in scale meters per second.
 *
 * Format: [S][EEEEE][MMMMMMMMMM]
 *   S     = sign/direction (bit 15)
 *   EEEEE = 5-bit exponent with bias 15 (bits 14-10)
 *   M...M = 10-bit mantissa (bits 9-0)
 *
 * Special values:
 *   0x0000 = positive zero (forward, stopped)
 *   0x8000 = negative zero (reverse, stopped)
 *   Exponent all 1s, mantissa != 0 = NaN (speed not available)
 *
 * All conversions use integer-only bit manipulation (no math.h dependency)
 * for embedded target compatibility.
 *
 * @author Jim Kueneman
 * @date 17 Feb 2026
 */

#ifndef __OPENLCB_FLOAT16__
#define __OPENLCB_FLOAT16__

#include <stdbool.h>
#include <stdint.h>

#define FLOAT16_POSITIVE_ZERO  0x0000
#define FLOAT16_NEGATIVE_ZERO  0x8000
#define FLOAT16_NAN            0x7E00

#define FLOAT16_SIGN_MASK      0x8000
#define FLOAT16_EXPONENT_MASK  0x7C00
#define FLOAT16_MANTISSA_MASK  0x03FF

#ifdef __cplusplus
extern "C" {
#endif

    extern uint16_t OpenLcbFloat16_from_float(float value);

    extern float OpenLcbFloat16_to_float(uint16_t half);

    extern uint16_t OpenLcbFloat16_negate(uint16_t half);

    extern bool OpenLcbFloat16_is_nan(uint16_t half);

    extern bool OpenLcbFloat16_is_zero(uint16_t half);

    extern uint16_t OpenLcbFloat16_speed_with_direction(float speed, bool reverse);

    extern float OpenLcbFloat16_get_speed(uint16_t half);

    extern bool OpenLcbFloat16_get_direction(uint16_t half);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_FLOAT16__ */
