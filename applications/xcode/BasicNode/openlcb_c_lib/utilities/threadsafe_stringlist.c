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
 * \file threadsafe_stringlist.c
 *
 * Thread-safe string queue implementation.
 *
 * @author Jim Kueneman
 * @date 12 Dec 2024
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "threadsafe_stringlist.h"

void ThreadSafeStringList_init(StringList *list)
{

    list->head = 0;
    list->tail = 0;
    pthread_mutex_init(&list->mutex, NULL);
}

uint8_t ThreadSafeStringList_push(StringList *list, const char *string)
{
    pthread_mutex_lock(&list->mutex);

    uint8_t result = false;

    int next = list->head + 1;

    if (next >= MAX_STRINGS)
        next = 0;

    if (next != list->tail)
    {

        //     printf("push %s\n", string);

        list->strings[list->head] = strdup(string);
        list->head = next;
        result = true;
    }

    pthread_mutex_unlock(&list->mutex);

    return result;
}

char *ThreadSafeStringList_pop(StringList *list)
{
    char *result = (void *)0;

    pthread_mutex_lock(&list->mutex);

    if (list->head != list->tail)
    {

        result = list->strings[list->tail];

        //      printf("pop: %s\n", result);

        list->tail = list->tail + 1;
        if (list->tail >= MAX_STRINGS)
            list->tail = 0;
    }

    pthread_mutex_unlock(&list->mutex);

    return result;
};

