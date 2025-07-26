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
 * \file mustangpeak_string_helper.h
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

// This is a guard condition so that contents of this file are not included
// more than once.

#ifndef __UTILITIES_MUSTANGPEAK_STRING_HELPER__
#define __UTILITIES_MUSTANGPEAK_STRING_HELPER__

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/// Create a new string by allocating memory for characters and null
/// termination. Resulting memory will be uninitialized.
/// @param char_count the number of characters to allocate space for, one will
///        be added to this cont for a null terminating character
/// @return space for the new string, uninitialized
extern char *strnew(size_t char_count);

/// Create a new string by allocating memory for characters and null
/// termination. Resulting memory will be initialized to all zeros ('\0)
/// @param char_count the number of characters to allocate space for, one will
///        be added to this cont for a null terminating character
/// @return space for the new string, initialized to all zeros ('\0')
extern char *strnew_initialized(size_t char_count);

/// Concatenate two strings. Memory will be allocated for the new string. The
/// memory for the passed in strings is not released, and remains under the
/// ownership of the caller.
/// @param str1 string to concatenate at the beginning
/// @param str2 string to concatenate at the end
/// @return new string in newly allocated memory that is str1 + str2
extern char *strcatnew(const char *str1, const char *str2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UTILITIES_MUSTANGPEAK_STRING_HELPER__ */