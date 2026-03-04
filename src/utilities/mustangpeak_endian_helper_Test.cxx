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
 * @file mustangpeak_endian_helper_Test.cxx
 * @brief Test suite for byte-order swap helper functions
 *
 * Test Organization:
 * - Section 1: swap_endian8 Tests
 * - Section 2: swap_endian16 Tests
 * - Section 3: swap_endian32 Tests
 * - Section 4: swap_endian64 Tests
 * - Section 5: Round-trip (double-swap) Tests
 *
 * @author Jim Kueneman
 * @date 4 Mar 2026
 */

#include "test/main_Test.hxx"

#include "utilities/mustangpeak_endian_helper.h"


// ============================================================================
// Section 1: swap_endian8 Tests
// ============================================================================

TEST(EndianHelper, swap8_returns_same_value)
{

    EXPECT_EQ(swap_endian8(0x00), 0x00);
    EXPECT_EQ(swap_endian8(0xFF), 0xFF);
    EXPECT_EQ(swap_endian8(0xA5), 0xA5);
    EXPECT_EQ(swap_endian8(0x01), 0x01);
    EXPECT_EQ(swap_endian8(0x80), 0x80);

}


// ============================================================================
// Section 2: swap_endian16 Tests
// ============================================================================

TEST(EndianHelper, swap16_swaps_bytes)
{

    EXPECT_EQ(swap_endian16(0x0102), 0x0201);
    EXPECT_EQ(swap_endian16(0xAABB), 0xBBAA);

}

TEST(EndianHelper, swap16_zero)
{

    EXPECT_EQ(swap_endian16(0x0000), 0x0000);

}

TEST(EndianHelper, swap16_all_ones)
{

    EXPECT_EQ(swap_endian16(0xFFFF), 0xFFFF);

}

TEST(EndianHelper, swap16_single_byte_high)
{

    EXPECT_EQ(swap_endian16(0xFF00), 0x00FF);

}

TEST(EndianHelper, swap16_single_byte_low)
{

    EXPECT_EQ(swap_endian16(0x00FF), 0xFF00);

}


// ============================================================================
// Section 3: swap_endian32 Tests
// ============================================================================

TEST(EndianHelper, swap32_swaps_bytes)
{

    EXPECT_EQ(swap_endian32(0x01020304), 0x04030201U);
    EXPECT_EQ(swap_endian32(0xAABBCCDD), 0xDDCCBBAAU);

}

TEST(EndianHelper, swap32_zero)
{

    EXPECT_EQ(swap_endian32(0x00000000), 0x00000000U);

}

TEST(EndianHelper, swap32_all_ones)
{

    EXPECT_EQ(swap_endian32(0xFFFFFFFF), 0xFFFFFFFFU);

}

TEST(EndianHelper, swap32_single_byte_positions)
{

    EXPECT_EQ(swap_endian32(0xFF000000), 0x000000FFU);
    EXPECT_EQ(swap_endian32(0x00FF0000), 0x0000FF00U);
    EXPECT_EQ(swap_endian32(0x0000FF00), 0x00FF0000U);
    EXPECT_EQ(swap_endian32(0x000000FF), 0xFF000000U);

}


// ============================================================================
// Section 4: swap_endian64 Tests
// ============================================================================

TEST(EndianHelper, swap64_swaps_bytes)
{

    EXPECT_EQ(swap_endian64(0x0102030405060708ULL), 0x0807060504030201ULL);
    EXPECT_EQ(swap_endian64(0xAABBCCDDEEFF1122ULL), 0x2211FFEEDDCCBBAAULL);

}

TEST(EndianHelper, swap64_zero)
{

    EXPECT_EQ(swap_endian64(0x0000000000000000ULL), 0x0000000000000000ULL);

}

TEST(EndianHelper, swap64_all_ones)
{

    EXPECT_EQ(swap_endian64(0xFFFFFFFFFFFFFFFFULL), 0xFFFFFFFFFFFFFFFFULL);

}

TEST(EndianHelper, swap64_single_byte_positions)
{

    EXPECT_EQ(swap_endian64(0xFF00000000000000ULL), 0x00000000000000FFULL);
    EXPECT_EQ(swap_endian64(0x00000000000000FFULL), 0xFF00000000000000ULL);
    EXPECT_EQ(swap_endian64(0x000000FF00000000ULL), 0x00000000FF000000ULL);
    EXPECT_EQ(swap_endian64(0x00000000FF000000ULL), 0x000000FF00000000ULL);

}


// ============================================================================
// Section 5: Round-trip (double-swap) Tests
// ============================================================================

TEST(EndianHelper, swap16_roundtrip)
{

    uint16_t original = 0x1234;

    EXPECT_EQ(swap_endian16(swap_endian16(original)), original);

}

TEST(EndianHelper, swap32_roundtrip)
{

    uint32_t original = 0xDEADBEEF;

    EXPECT_EQ(swap_endian32(swap_endian32(original)), original);

}

TEST(EndianHelper, swap64_roundtrip)
{

    uint64_t original = 0x0123456789ABCDEFULL;

    EXPECT_EQ(swap_endian64(swap_endian64(original)), original);

}
