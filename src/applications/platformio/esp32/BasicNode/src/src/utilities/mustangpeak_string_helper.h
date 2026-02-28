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
 * @file mustangpeak_string_helper.h
 * @brief Dynamic string allocation helpers using malloc.
 *
 * @details Provides convenience wrappers around malloc for creating new
 * null-terminated C strings and for concatenating two strings into a newly
 * allocated buffer.  Every string returned by these functions must be freed
 * by the caller with free().
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.

#ifndef __MUSTANGPEAK_STRING_HELPER__
#define __MUSTANGPEAK_STRING_HELPER__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
         * @brief Allocates a new uninitialized string buffer.
         *
         * @details Allocates char_count + 1 bytes via malloc (the extra byte is for
         * the null terminator).  The contents of the buffer are uninitialized.
         * The caller is responsible for calling free() when finished.
         *
         * @param char_count  Number of usable characters (excluding the null terminator).
         *
         * @return Pointer to the allocated buffer, or NULL if malloc fails.
         *
         * @warning The caller must free() the returned pointer when finished.
         */
    extern char *strnew(int char_count);

        /**
         * @brief Allocates a new zero-initialized string buffer.
         *
         * @details Allocates char_count + 1 bytes via malloc and fills every byte
         * with '\\0'.  The caller is responsible for calling free() when finished.
         *
         * @param char_count  Number of usable characters (excluding the null terminator).
         *
         * @return Pointer to the allocated and zeroed buffer, or NULL if malloc fails.
         *
         * @warning The caller must free() the returned pointer when finished.
         */
    extern char *strnew_initialized(int char_count);

        /**
         * @brief Concatenates two strings into a newly allocated buffer.
         *
         * @details Allocates a new buffer large enough to hold the concatenation of
         * str1 and str2 plus a null terminator, copies both strings into it, and
         * returns the result.  The caller is responsible for calling free() when
         * finished.
         *
         * @param str1  Pointer to the first null-terminated string.
         * @param str2  Pointer to the second null-terminated string.
         *
         * @return Pointer to the newly allocated concatenated string, or NULL if
         *         malloc fails.
         *
         * @warning The caller must free() the returned pointer when finished.
         * @warning NULL pointers for either argument cause undefined behavior.
         */
    extern char *strcatnew(char *str1, char *str2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MUSTANGPEAK_STRING_HELPER__ */
