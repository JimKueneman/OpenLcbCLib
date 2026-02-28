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
 * @brief IEEE 754 half-precision (float16) conversion utilities.
 *
 * @details Encodes/decodes OpenLCB train speed as binary16.  Sign bit = direction
 * (0 forward, 1 reverse).  All conversions use integer-only bit manipulation
 * for embedded targets.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __OPENLCB_FLOAT16__
#define __OPENLCB_FLOAT16__

#include <stdbool.h>
#include <stdint.h>

#ifdef	__cplusplus
  extern "C" {
#endif /* __cplusplus */

/** @brief float16 value representing positive zero (forward, stopped). */
#define FLOAT16_POSITIVE_ZERO  0x0000

/** @brief float16 value representing negative zero (reverse, stopped). */
#define FLOAT16_NEGATIVE_ZERO  0x8000

/** @brief float16 NaN value (speed not available). */
#define FLOAT16_NAN            0x7E00

/** @brief Bit mask for the sign/direction bit (bit 15). */
#define FLOAT16_SIGN_MASK      0x8000

/** @brief Bit mask for the 5-bit exponent field (bits 14-10). */
#define FLOAT16_EXPONENT_MASK  0x7C00

/** @brief Bit mask for the 10-bit mantissa field (bits 9-0). */
#define FLOAT16_MANTISSA_MASK  0x03FF

        /**
         * @brief Converts a 32-bit float to a float16 bit pattern.
         *
         * @details Rounds toward zero.  Overflow is clamped.  NaN produces FLOAT16_NAN.
         *
         * @param value  32-bit float to convert.
         *
         * @return 16-bit float16 bit pattern.
         */
    extern uint16_t OpenLcbFloat16_from_float(float value);

        /** @brief Converts a float16 bit pattern to a 32-bit float. */
    extern float OpenLcbFloat16_to_float(uint16_t half);

        /** @brief Flips the sign/direction bit of a float16 value and returns the result. */
    extern uint16_t OpenLcbFloat16_negate(uint16_t half);

        /** @brief Returns true if the float16 bit pattern represents NaN (speed not available). */
    extern bool OpenLcbFloat16_is_nan(uint16_t half);

        /** @brief Returns true if the float16 bit pattern represents positive or negative zero. */
    extern bool OpenLcbFloat16_is_zero(uint16_t half);

        /** @brief Encodes a speed magnitude and direction into a float16 bit pattern. */
    extern uint16_t OpenLcbFloat16_speed_with_direction(float speed, bool reverse);

        /** @brief Returns the speed magnitude from a float16 bit pattern (ignores direction). */
    extern float OpenLcbFloat16_get_speed(uint16_t half);

        /** @brief Returns true if the direction bit is set (reverse). */
    extern bool OpenLcbFloat16_get_direction(uint16_t half);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_FLOAT16__ */
