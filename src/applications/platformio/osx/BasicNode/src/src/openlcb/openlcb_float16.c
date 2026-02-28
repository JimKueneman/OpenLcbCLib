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
 * @file openlcb_float16.c
 * @brief IEEE 754 half-precision (float16) conversion utilities
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "openlcb_float16.h"

#include <string.h>


    /** @brief Reinterprets a float as its raw uint32_t bit pattern. */
static uint32_t _float_to_bits(float f) {

    uint32_t bits;

    memcpy(&bits, &f, sizeof(bits));

    return bits;

}

    /** @brief Reinterprets a raw uint32_t bit pattern as a float. */
static float _bits_to_float(uint32_t bits) {

    float f;

    memcpy(&f, &bits, sizeof(f));

    return f;

}


    /**
    * @brief Converts a 32-bit float to a float16 bit pattern.
    *
    * @details Algorithm:
    * -# Extract sign, exponent, mantissa from IEEE 754 single
    * -# Handle special cases: zero, NaN, infinity
    * -# Clamp overflow to max finite half value
    * -# Normal range: rebias exponent, truncate mantissa
    * -# Subnormal: shift mantissa into denormalized form
    * -# Underflow: flush to signed zero
    *
    * @verbatim
    * @param value 32-bit float to convert
    * @endverbatim
    *
    * @return 16-bit float16 bit pattern
    */
uint16_t OpenLcbFloat16_from_float(float value) {

    uint32_t fbits = _float_to_bits(value);

    uint16_t sign = (uint16_t) ((fbits >> 16) & 0x8000);
    int32_t exponent = (int32_t) ((fbits >> 23) & 0xFF) - 127;
    uint32_t mantissa = fbits & 0x007FFFFF;

    // Handle special cases

    // Zero (positive or negative)
    if (exponent == -127 && mantissa == 0) {

        return sign;

    }

    // NaN
    if (exponent == 128 && mantissa != 0) {

        return (uint16_t) (sign | 0x7E00);

    }

    // Infinity
    if (exponent == 128) {

        return (uint16_t) (sign | 0x7C00);

    }

    // Overflow — clamp to max finite half value
    if (exponent > 15) {

        return (uint16_t) (sign | 0x7BFF);

    }

    // Normal range for half
    if (exponent >= -14) {

        uint16_t h_exp = (uint16_t) ((exponent + 15) << 10);
        uint16_t h_man = (uint16_t) (mantissa >> 13);

        return (uint16_t) (sign | h_exp | h_man);

    }

    // Subnormal — exponent too small for half normal
    if (exponent >= -24) {

        mantissa |= 0x00800000;

        uint32_t shift = (uint32_t) (-14 - exponent);
        uint16_t h_man = (uint16_t) (mantissa >> (13 + shift));

        return (uint16_t) (sign | h_man);

    }

    // Too small — flush to zero
    return sign;

}


    /**
    * @brief Converts a float16 bit pattern to a 32-bit float.
    *
    * @details Algorithm:
    * -# Extract sign, exponent, mantissa from half-precision fields
    * -# Handle zero, subnormal (normalize), infinity/NaN, and normal cases
    * -# Rebias exponent and expand mantissa to single-precision format
    *
    * @verbatim
    * @param half 16-bit float16 bit pattern
    * @endverbatim
    *
    * @return 32-bit float
    */
float OpenLcbFloat16_to_float(uint16_t half) {

    uint32_t sign = ((uint32_t) half & 0x8000) << 16;
    uint32_t exponent = (half >> 10) & 0x1F;
    uint32_t mantissa = half & 0x03FF;

    // Zero
    if (exponent == 0 && mantissa == 0) {

        return _bits_to_float(sign);

    }

    // Subnormal
    if (exponent == 0) {

        // Normalize: shift mantissa up until leading 1 is in bit 10
        while ((mantissa & 0x0400) == 0) {

            mantissa <<= 1;
            exponent--;

        }

        exponent++;
        mantissa &= 0x03FF;

        uint32_t f_exp = (exponent + 127 - 15) << 23;
        uint32_t f_man = mantissa << 13;

        return _bits_to_float(sign | f_exp | f_man);

    }

    // Infinity or NaN
    if (exponent == 0x1F) {

        uint32_t f_exp = 0xFFUL << 23;
        uint32_t f_man = mantissa << 13;

        return _bits_to_float(sign | f_exp | f_man);

    }

    // Normal
    uint32_t f_exp = (exponent + 127 - 15) << 23;
    uint32_t f_man = mantissa << 13;

    return _bits_to_float(sign | f_exp | f_man);

}


    /** @brief Flips the sign/direction bit of a float16 value. */
uint16_t OpenLcbFloat16_negate(uint16_t half) {

    return half ^ FLOAT16_SIGN_MASK;

}


    /** @brief Returns true if the float16 bit pattern represents NaN. */
bool OpenLcbFloat16_is_nan(uint16_t half) {

    uint16_t exp = half & FLOAT16_EXPONENT_MASK;
    uint16_t man = half & FLOAT16_MANTISSA_MASK;

    return (exp == FLOAT16_EXPONENT_MASK) && (man != 0);

}


    /** @brief Returns true if the float16 bit pattern represents positive or negative zero. */
bool OpenLcbFloat16_is_zero(uint16_t half) {

    return (half & 0x7FFF) == 0;

}


    /** @brief Encodes a speed magnitude and direction into a float16 bit pattern. */
uint16_t OpenLcbFloat16_speed_with_direction(float speed, bool reverse) {

    // Ensure speed is non-negative for conversion
    if (speed < 0.0f) {

        speed = -speed;

    }

    uint16_t half = OpenLcbFloat16_from_float(speed);

    // Clear sign bit then set direction
    half &= 0x7FFF;

    if (reverse) {

        half |= FLOAT16_SIGN_MASK;

    }

    return half;

}


    /** @brief Returns the speed magnitude from a float16 bit pattern (ignores direction). */
float OpenLcbFloat16_get_speed(uint16_t half) {

    // Clear sign bit to get absolute speed
    uint16_t abs_half = half & 0x7FFF;

    return OpenLcbFloat16_to_float(abs_half);

}


    /** @brief Returns true if the direction bit is set (reverse). */
bool OpenLcbFloat16_get_direction(uint16_t half) {

    return (half & FLOAT16_SIGN_MASK) != 0;

}
