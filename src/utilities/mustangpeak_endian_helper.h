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
 * @file mustangpeak_endian_helper.h
 * @brief Byte-order swap helpers for 8-, 16-, 32-, and 64-bit unsigned integers.
 *
 * @details Provides portable byte-swap functions used throughout the library
 * wherever multi-byte values must be converted between host and network (big-endian)
 * byte order.  On GCC/Clang the implementations use compiler built-ins for optimal
 * code generation; on other compilers a portable bit-shift fallback is used.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.

#ifndef __MUSTANGPEAK_ENDIAN_HELPER__
#define __MUSTANGPEAK_ENDIAN_HELPER__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
         * @brief Returns the input byte unchanged (no-op for API symmetry).
         *
         * @details Provided so that generic code can call swap_endianN() for any
         * width without special-casing the single-byte case.
         *
         * @param x  Byte value to "swap".
         *
         * @return The same byte value, unchanged.
         */
    extern uint8_t swap_endian8(uint8_t x);

        /**
         * @brief Reverses the byte order of a 16-bit unsigned integer.
         *
         * @details Swaps the high and low bytes.  Uses __builtin_bswap16 when
         * available for single-instruction code generation.
         *
         * @param x  16-bit value to byte-swap.
         *
         * @return The byte-swapped 16-bit value.
         */
    extern uint16_t swap_endian16(uint16_t x);

        /**
         * @brief Reverses the byte order of a 32-bit unsigned integer.
         *
         * @details Swaps all four bytes end-for-end.  Uses __builtin_bswap32 when
         * available for single-instruction code generation.
         *
         * @param x  32-bit value to byte-swap.
         *
         * @return The byte-swapped 32-bit value.
         */
    extern uint32_t swap_endian32(uint32_t x);

        /**
         * @brief Reverses the byte order of a 64-bit unsigned integer.
         *
         * @details Swaps all eight bytes end-for-end.  Uses __builtin_bswap64 when
         * available for single-instruction code generation.
         *
         * @param x  64-bit value to byte-swap.
         *
         * @return The byte-swapped 64-bit value.
         */
    extern uint64_t swap_endian64(uint64_t x);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MUSTANGPEAK_ENDIAN_HELPER__ */
