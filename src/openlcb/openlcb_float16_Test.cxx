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
* @file openlcb_float16_Test.cxx
* @brief Unit tests for IEEE 754 half-precision float16 utilities
*
* Test Organization:
* - Section 1: Conversion from float to float16
* - Section 2: Conversion from float16 to float
* - Section 3: Round-trip accuracy
* - Section 4: Special values (zero, NaN, infinity)
* - Section 5: Direction/speed helpers
* - Section 6: Negate
*
* @author Jim Kueneman
* @date 17 Feb 2026
*/

#include "test/main_Test.hxx"

#include <cmath>

#include "openlcb_float16.h"


// ============================================================================
// Section 1: Conversion from float to float16
// ============================================================================

TEST(Float16, from_float_positive_zero)
{

    uint16_t result = OpenLcbFloat16_from_float(0.0f);

    EXPECT_EQ(result, FLOAT16_POSITIVE_ZERO);

}

TEST(Float16, from_float_negative_zero)
{

    uint16_t result = OpenLcbFloat16_from_float(-0.0f);

    EXPECT_EQ(result, FLOAT16_NEGATIVE_ZERO);

}

TEST(Float16, from_float_one)
{

    // 1.0 in half: sign=0, exp=15(0b01111), mantissa=0
    // = 0 01111 0000000000 = 0x3C00
    uint16_t result = OpenLcbFloat16_from_float(1.0f);

    EXPECT_EQ(result, 0x3C00);

}

TEST(Float16, from_float_minus_one)
{

    // -1.0 in half: sign=1, exp=15, mantissa=0
    // = 1 01111 0000000000 = 0xBC00
    uint16_t result = OpenLcbFloat16_from_float(-1.0f);

    EXPECT_EQ(result, 0xBC00);

}

TEST(Float16, from_float_half)
{

    // 0.5 in half: sign=0, exp=14(0b01110), mantissa=0
    // = 0 01110 0000000000 = 0x3800
    uint16_t result = OpenLcbFloat16_from_float(0.5f);

    EXPECT_EQ(result, 0x3800);

}

TEST(Float16, from_float_two)
{

    // 2.0 in half: sign=0, exp=16(0b10000), mantissa=0
    // = 0 10000 0000000000 = 0x4000
    uint16_t result = OpenLcbFloat16_from_float(2.0f);

    EXPECT_EQ(result, 0x4000);

}

TEST(Float16, from_float_100)
{

    // 100.0 in half: sign=0, exp=6+15=21(0b10101), mantissa=100/64-1=0.5625
    // 100 = 1.5625 * 2^6 => exp=21, mantissa=0.5625*1024=576=0x240
    // = 0 10101 1001000000 = 0x5640
    uint16_t result = OpenLcbFloat16_from_float(100.0f);

    EXPECT_EQ(result, 0x5640);

}

TEST(Float16, from_float_overflow_clamps)
{

    // 100000.0 overflows half max (65504) — should clamp to max finite
    uint16_t result = OpenLcbFloat16_from_float(100000.0f);

    EXPECT_EQ(result, 0x7BFF);

}

TEST(Float16, from_float_subnormal)
{

    // Very small value that becomes subnormal in half
    // Smallest normal half = 2^-14 ~ 6.1e-5
    // 2^-15 = 3.05e-5 should be subnormal
    uint16_t result = OpenLcbFloat16_from_float(3.0517578125e-5f);

    // Subnormal: exp=0, mantissa should be non-zero
    EXPECT_EQ(result & FLOAT16_EXPONENT_MASK, 0x0000);
    EXPECT_NE(result & FLOAT16_MANTISSA_MASK, 0x0000);

}

TEST(Float16, from_float_tiny_flushes_to_zero)
{

    // Extremely small value below half subnormal range
    uint16_t result = OpenLcbFloat16_from_float(1.0e-10f);

    EXPECT_EQ(result, FLOAT16_POSITIVE_ZERO);

}


// ============================================================================
// Section 2: Conversion from float16 to float
// ============================================================================

TEST(Float16, to_float_positive_zero)
{

    float result = OpenLcbFloat16_to_float(FLOAT16_POSITIVE_ZERO);

    EXPECT_EQ(result, 0.0f);

}

TEST(Float16, to_float_negative_zero)
{

    float result = OpenLcbFloat16_to_float(FLOAT16_NEGATIVE_ZERO);

    EXPECT_EQ(result, -0.0f);
    EXPECT_TRUE(std::signbit(result));

}

TEST(Float16, to_float_one)
{

    float result = OpenLcbFloat16_to_float(0x3C00);

    EXPECT_FLOAT_EQ(result, 1.0f);

}

TEST(Float16, to_float_minus_one)
{

    float result = OpenLcbFloat16_to_float(0xBC00);

    EXPECT_FLOAT_EQ(result, -1.0f);

}

TEST(Float16, to_float_100)
{

    float result = OpenLcbFloat16_to_float(0x5640);

    EXPECT_FLOAT_EQ(result, 100.0f);

}

TEST(Float16, to_float_max_finite)
{

    // 0x7BFF = max finite half = 65504
    float result = OpenLcbFloat16_to_float(0x7BFF);

    EXPECT_FLOAT_EQ(result, 65504.0f);

}

TEST(Float16, to_float_nan)
{

    float result = OpenLcbFloat16_to_float(FLOAT16_NAN);

    EXPECT_TRUE(std::isnan(result));

}

TEST(Float16, to_float_infinity)
{

    // 0x7C00 = positive infinity
    float result = OpenLcbFloat16_to_float(0x7C00);

    EXPECT_TRUE(std::isinf(result));
    EXPECT_GT(result, 0.0f);

}

TEST(Float16, to_float_negative_infinity)
{

    // 0xFC00 = negative infinity
    float result = OpenLcbFloat16_to_float(0xFC00);

    EXPECT_TRUE(std::isinf(result));
    EXPECT_LT(result, 0.0f);

}

TEST(Float16, to_float_subnormal)
{

    // Smallest subnormal: 0x0001 = 2^-24 ~ 5.96e-8
    float result = OpenLcbFloat16_to_float(0x0001);

    EXPECT_GT(result, 0.0f);
    EXPECT_LT(result, 1.0e-4f);

}


// ============================================================================
// Section 3: Round-trip accuracy
// ============================================================================

TEST(Float16, roundtrip_one)
{

    float original = 1.0f;
    float result = OpenLcbFloat16_to_float(OpenLcbFloat16_from_float(original));

    EXPECT_FLOAT_EQ(result, original);

}

TEST(Float16, roundtrip_100)
{

    float original = 100.0f;
    float result = OpenLcbFloat16_to_float(OpenLcbFloat16_from_float(original));

    EXPECT_FLOAT_EQ(result, original);

}

TEST(Float16, roundtrip_typical_speeds)
{

    // Test some typical model railroad speeds
    float speeds[] = {0.0f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 25.0f, 50.0f, 100.0f};

    for (float speed : speeds) {

        float result = OpenLcbFloat16_to_float(OpenLcbFloat16_from_float(speed));
        EXPECT_FLOAT_EQ(result, speed);

    }

}

TEST(Float16, roundtrip_negative)
{

    float original = -42.0f;
    float result = OpenLcbFloat16_to_float(OpenLcbFloat16_from_float(original));

    EXPECT_FLOAT_EQ(result, original);

}


// ============================================================================
// Section 4: Special value helpers
// ============================================================================

TEST(Float16, is_nan_true)
{

    EXPECT_TRUE(OpenLcbFloat16_is_nan(FLOAT16_NAN));
    EXPECT_TRUE(OpenLcbFloat16_is_nan(0x7C01));
    EXPECT_TRUE(OpenLcbFloat16_is_nan(0xFE00));

}

TEST(Float16, is_nan_false)
{

    EXPECT_FALSE(OpenLcbFloat16_is_nan(FLOAT16_POSITIVE_ZERO));
    EXPECT_FALSE(OpenLcbFloat16_is_nan(FLOAT16_NEGATIVE_ZERO));
    EXPECT_FALSE(OpenLcbFloat16_is_nan(0x3C00));
    EXPECT_FALSE(OpenLcbFloat16_is_nan(0x7C00));  // Infinity, not NaN

}

TEST(Float16, is_zero_true)
{

    EXPECT_TRUE(OpenLcbFloat16_is_zero(FLOAT16_POSITIVE_ZERO));
    EXPECT_TRUE(OpenLcbFloat16_is_zero(FLOAT16_NEGATIVE_ZERO));

}

TEST(Float16, is_zero_false)
{

    EXPECT_FALSE(OpenLcbFloat16_is_zero(0x3C00));
    EXPECT_FALSE(OpenLcbFloat16_is_zero(0x0001));
    EXPECT_FALSE(OpenLcbFloat16_is_zero(FLOAT16_NAN));

}


// ============================================================================
// Section 5: Direction/speed helpers
// ============================================================================

TEST(Float16, speed_with_direction_forward)
{

    uint16_t result = OpenLcbFloat16_speed_with_direction(50.0f, false);

    EXPECT_EQ(result & FLOAT16_SIGN_MASK, 0x0000);
    EXPECT_FLOAT_EQ(OpenLcbFloat16_get_speed(result), 50.0f);
    EXPECT_FALSE(OpenLcbFloat16_get_direction(result));

}

TEST(Float16, speed_with_direction_reverse)
{

    uint16_t result = OpenLcbFloat16_speed_with_direction(50.0f, true);

    EXPECT_NE(result & FLOAT16_SIGN_MASK, 0x0000);
    EXPECT_FLOAT_EQ(OpenLcbFloat16_get_speed(result), 50.0f);
    EXPECT_TRUE(OpenLcbFloat16_get_direction(result));

}

TEST(Float16, speed_with_direction_zero_forward)
{

    uint16_t result = OpenLcbFloat16_speed_with_direction(0.0f, false);

    EXPECT_EQ(result, FLOAT16_POSITIVE_ZERO);

}

TEST(Float16, speed_with_direction_zero_reverse)
{

    uint16_t result = OpenLcbFloat16_speed_with_direction(0.0f, true);

    EXPECT_EQ(result, FLOAT16_NEGATIVE_ZERO);

}

TEST(Float16, speed_with_direction_negative_input_made_positive)
{

    // Passing negative speed should still produce correct absolute speed
    uint16_t result = OpenLcbFloat16_speed_with_direction(-25.0f, false);

    EXPECT_FLOAT_EQ(OpenLcbFloat16_get_speed(result), 25.0f);
    EXPECT_FALSE(OpenLcbFloat16_get_direction(result));

}

TEST(Float16, get_speed_strips_direction)
{

    // Forward 10 m/s
    uint16_t fwd = OpenLcbFloat16_speed_with_direction(10.0f, false);
    // Reverse 10 m/s
    uint16_t rev = OpenLcbFloat16_speed_with_direction(10.0f, true);

    EXPECT_FLOAT_EQ(OpenLcbFloat16_get_speed(fwd), 10.0f);
    EXPECT_FLOAT_EQ(OpenLcbFloat16_get_speed(rev), 10.0f);

}

TEST(Float16, get_direction_forward)
{

    uint16_t half = OpenLcbFloat16_speed_with_direction(5.0f, false);

    EXPECT_FALSE(OpenLcbFloat16_get_direction(half));

}

TEST(Float16, get_direction_reverse)
{

    uint16_t half = OpenLcbFloat16_speed_with_direction(5.0f, true);

    EXPECT_TRUE(OpenLcbFloat16_get_direction(half));

}


// ============================================================================
// Section 6: Negate
// ============================================================================

TEST(Float16, negate_forward_to_reverse)
{

    uint16_t fwd = OpenLcbFloat16_speed_with_direction(10.0f, false);
    uint16_t rev = OpenLcbFloat16_negate(fwd);

    EXPECT_FALSE(OpenLcbFloat16_get_direction(fwd));
    EXPECT_TRUE(OpenLcbFloat16_get_direction(rev));
    EXPECT_FLOAT_EQ(OpenLcbFloat16_get_speed(fwd), OpenLcbFloat16_get_speed(rev));

}

TEST(Float16, negate_reverse_to_forward)
{

    uint16_t rev = OpenLcbFloat16_speed_with_direction(10.0f, true);
    uint16_t fwd = OpenLcbFloat16_negate(rev);

    EXPECT_TRUE(OpenLcbFloat16_get_direction(rev));
    EXPECT_FALSE(OpenLcbFloat16_get_direction(fwd));

}

TEST(Float16, negate_zero_forward_to_reverse)
{

    uint16_t result = OpenLcbFloat16_negate(FLOAT16_POSITIVE_ZERO);

    EXPECT_EQ(result, FLOAT16_NEGATIVE_ZERO);

}

TEST(Float16, negate_zero_reverse_to_forward)
{

    uint16_t result = OpenLcbFloat16_negate(FLOAT16_NEGATIVE_ZERO);

    EXPECT_EQ(result, FLOAT16_POSITIVE_ZERO);

}

TEST(Float16, negate_double_negate_identity)
{

    uint16_t original = OpenLcbFloat16_speed_with_direction(42.0f, false);
    uint16_t result = OpenLcbFloat16_negate(OpenLcbFloat16_negate(original));

    EXPECT_EQ(result, original);

}
