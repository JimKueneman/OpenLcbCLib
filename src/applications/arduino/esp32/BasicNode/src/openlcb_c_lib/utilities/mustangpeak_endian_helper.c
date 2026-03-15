/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * @file mustangpeak_endian_helper.c
 * @brief Implementation of the byte-order swap helpers.
 *
 * @details Each function reverses the byte order of its argument.  On GCC/Clang
 * the compiler built-in __builtin_bswapNN is used for optimal code generation;
 * on other compilers a portable bit-shift fallback is provided.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "mustangpeak_endian_helper.h"

#include <stdint.h>

    /**
     * @brief Returns the input byte unchanged (no-op for API symmetry).
     *
     * @verbatim
     * @param x  Byte value to "swap".
     * @endverbatim
     *
     * @return The same byte value, unchanged.
     */
uint8_t swap_endian8(uint8_t x) {

    return x;

}

    /**
     * @brief Reverses the byte order of a 16-bit unsigned integer.
     *
     * @details Algorithm:
     * -# If __builtin_bswap16 is available, use it directly.
     * -# Otherwise shift the low byte up 8 bits and OR with the high byte shifted down.
     *
     * @verbatim
     * @param x  16-bit value to byte-swap.
     * @endverbatim
     *
     * @return The byte-swapped 16-bit value.
     */
uint16_t swap_endian16(uint16_t x) {

#if defined(__GNUC__) || defined(__clang__)
#if defined(__has_builtin)
#if __has_builtin(__builtin_bswap16)
    return __builtin_bswap16(x);
#endif
#elif defined(__GNUC__)
    /* GCC provides __builtin_bswap16 since GCC 4.8 */
    return __builtin_bswap16(x);
#endif
#endif
    return (uint16_t)((x << 8) | (x >> 8));

}

    /**
     * @brief Reverses the byte order of a 32-bit unsigned integer.
     *
     * @details Algorithm:
     * -# If __builtin_bswap32 is available, use it directly.
     * -# Otherwise mask and shift each of the four bytes into its mirror position.
     *
     * @verbatim
     * @param x  32-bit value to byte-swap.
     * @endverbatim
     *
     * @return The byte-swapped 32-bit value.
     */
uint32_t swap_endian32(uint32_t x) {

#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(x);
#else
    return ((x & 0x000000FFU) << 24) |
           ((x & 0x0000FF00U) << 8) |
           ((x & 0x00FF0000U) >> 8) |
           ((x & 0xFF000000U) >> 24);
#endif

}

    /**
     * @brief Reverses the byte order of a 64-bit unsigned integer.
     *
     * @details Algorithm:
     * -# If __builtin_bswap64 is available, use it directly.
     * -# Otherwise mask and shift each of the eight bytes into its mirror position.
     *
     * @verbatim
     * @param x  64-bit value to byte-swap.
     * @endverbatim
     *
     * @return The byte-swapped 64-bit value.
     */
uint64_t swap_endian64(uint64_t x) {

#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(x);
#else
    return ((x & 0x00000000000000FFULL) << 56) |
           ((x & 0x000000000000FF00ULL) << 40) |
           ((x & 0x0000000000FF0000ULL) << 24) |
           ((x & 0x00000000FF000000ULL) << 8) |
           ((x & 0x000000FF00000000ULL) >> 8) |
           ((x & 0x0000FF0000000000ULL) >> 24) |
           ((x & 0x00FF000000000000ULL) >> 40) |
           ((x & 0xFF00000000000000ULL) >> 56);
#endif

}
