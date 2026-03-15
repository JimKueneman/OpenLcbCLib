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
 * @file mustangpeak_string_helper.c
 * @brief Implementation of the dynamic string allocation helpers.
 *
 * @details Uses malloc for all allocations.  Every buffer returned by these
 * functions must be freed by the caller with free().
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "mustangpeak_string_helper.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


    /**
     * @brief Allocates a new uninitialized string buffer.
     *
     * @details Algorithm:
     * -# Call malloc for char_count + 1 bytes (room for the null terminator).
     * -# Return the pointer (may be NULL on allocation failure).
     *
     * @verbatim
     * @param char_count  Number of usable characters (excluding the null terminator).
     * @endverbatim
     *
     * @return Pointer to the allocated buffer, or NULL if malloc fails.
     *
     * @warning The caller must free() the returned pointer when finished.
     */
char *strnew(int char_count) {

    return (char *)(malloc((char_count + 1) * sizeof(char)));

}

    /**
     * @brief Allocates a new zero-initialized string buffer.
     *
     * @details Algorithm:
     * -# Call malloc for char_count + 1 bytes.
     * -# Fill every byte with '\\0'.
     * -# Return the pointer.
     *
     * @verbatim
     * @param char_count  Number of usable characters (excluding the null terminator).
     * @endverbatim
     *
     * @return Pointer to the allocated and zeroed buffer, or NULL if malloc fails.
     *
     * @warning The caller must free() the returned pointer when finished.
     */
char *strnew_initialized(int char_count) {

    char *result = (char *)(malloc((char_count + 1) * sizeof(char)));

    for (int i = 0; i < char_count + 1; i++) {

        result[i] = '\0';

    }

    return result;

}

    /**
     * @brief Concatenates two strings into a newly allocated buffer.
     *
     * @details Algorithm:
     * -# Compute the combined length of str1 and str2.
     * -# Allocate a new buffer via strnew().
     * -# Copy str1 into the buffer with strcpy().
     * -# Append str2 with strcat().
     * -# Null-terminate and return.
     *
     * @verbatim
     * @param str1  Pointer to the first null-terminated string.
     * @param str2  Pointer to the second null-terminated string.
     * @endverbatim
     *
     * @return Pointer to the newly allocated concatenated string, or NULL if
     *         malloc fails.
     *
     * @warning The caller must free() the returned pointer when finished.
     * @warning NULL pointers for either argument cause undefined behavior.
     */
char *strcatnew(char *str1, char *str2) {

    int len = (int)(strlen(str1) + strlen(str2));
    char *temp1 = strnew(len);
    strcpy(temp1, str1);
    strcat(temp1, str2);
    temp1[len] = '\0';

    return temp1;

}
