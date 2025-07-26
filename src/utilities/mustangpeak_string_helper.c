
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
 * \file mustangpeak_string_helper.c
 *
 * Implements the core buffers for normal, snip, datagram, and stream length buffers.
 * The FIFO and List buffers are arrays of pointers to these core buffers that are 
 * allocated and freed through access.  The CAN Rx and 100ms timer access these buffers
 * so care must be taken to Pause and Resume those calls if the main loop needs to 
 * access the buffers.  
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "utilities/mustangpeak_string_helper.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

/// Magic number for error checking malloc and free.
#define STR_MALLOC_MAGIC 0x06fe11ddUL

/// Internal structure for handling the allocation checking.
typedef struct
{
    uint32_t magic; ///< magic number
    char str[0]; ///< user visible string
} StringHelper_Malloc;


char *strnew(size_t char_count)
{
    // Always add space for a null character.
#if defined(NDEBUG)
    return (char *)(malloc( (char_count + 1) * sizeof(char)) );
#else
    // Prepend magic number.
    StringHelper_Malloc *result = (StringHelper_Malloc *)
        malloc( sizeof(uint32_t) + ((char_count + 1) * sizeof(char)) );
    result->magic = STR_MALLOC_MAGIC;
    return result->str;
#endif
}

char *strnew_initialized(size_t char_count)
{
    char *result = strnew(char_count);
    for (int i = 0; i < char_count + 1; i++)
      result[i] = '\0';
    return result;
}

char *strcatnew(const char *str1, const char *str2)
{
    unsigned long len = strlen(str1) + strlen(str2);
    char *temp1 = strnew(len);
    strcpy(temp1, str1);
    strcat(temp1, str2);
    temp1[len] = '\0';
    return temp1;
}

void strfree(char *str)
{
#if defined(NDEBUG)
    free(str);
#else
    StringHelper_Malloc *magic =
        (StringHelper_Malloc *)(str - sizeof(uint32_t));
    assert(magic->magic == STR_MALLOC_MAGIC);
    magic->magic = 0;
    free(magic);
#endif
}