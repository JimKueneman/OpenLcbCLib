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
 * \file openlcb_buffer_store.h
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

#ifndef __THREADSAFE_STRINGLIST_
#define __THREADSAFE_STRINGLIST_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "src/openlcb/openlcb_types.h"

#define MAX_STRINGS 100

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    typedef struct
    {
        char *strings[MAX_STRINGS];
        int count;
        int head, tail;
        pthread_mutex_t mutex;
    } StringList;

    // Initialize the string list
    extern void ThreadSafeStringList_init(StringList *list);

    // Add a string to the list
    extern void ThreadSafeStringList_add(StringList *list, const char *string);

    // Remove string from list
    extern void ThreadSafeStringList_remove(StringList *list, const char *str);

    // Add a string to the list
    extern uint8_olcb_t ThreadSafeStringList_push(StringList *list, const char *string);

    // Remove string from list; dont for get to free() the popped string
    extern char *ThreadSafeStringList_pop(StringList *list);

    // Print the list
    extern void ThreadSafeStringList_print(StringList *list);

    // Cleanup the list
    extern void ThreadSafeStringList_destroy(StringList *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __THREADSAFE_STRINGLIST_ */