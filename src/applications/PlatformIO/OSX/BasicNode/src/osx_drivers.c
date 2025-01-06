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
 * \odx_drivers.c
 *
 *
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

#include "osx_drivers.h"

#include "src/openlcb/openlcb_types.h"
#include "src/utilities/mustangpeak_string_helper.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h> // read(), write(), close()

uint8_olcb_t _is_clock_running = FALSE;
uint8_olcb_t _timer_pause = FALSE;
char *user_data;

parameterless_callback_t _100ms_timer_sink_func = (void *)0;

void *thread_function_timer(void *arg)
{
    int thread_id = *((int *)arg); // Access argument passed to thread

    printf("100ms Timer Thread %d started\n", thread_id);

    _is_clock_running = TRUE;

    while (1)
    {

        if (_timer_pause == 0)
        {
            if (_100ms_timer_sink_func)
                _100ms_timer_sink_func();
        }

        usleep(100000);
    }
}

uint8_olcb_t OSxDrivers_100ms_is_connected()
{

    return _is_clock_running;
}

void OSxDrivers_setup(parameterless_callback_t _100ms_timer_sink)
{

    _100ms_timer_sink_func = _100ms_timer_sink;

    user_data = strnew_initialized(LEN_SNIP_USER_NAME + LEN_SNIP_USER_DESCRIPTION + 1); // add extra null since these are 2 null terminated strings

    pthread_t thread2;

    int thread_num2 = 2;

    pthread_create(&thread2, NULL, thread_function_timer, &thread_num2);
}

void OSxDrivers_reboot(void)
{
}

uint16_olcb_t OSxDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer)
{

    printf("configmem read count: %d\n", count);
    printf("configmem read address: %08lX\n", address);

    printf("configmem read address: %02X\n", (*buffer)[0]);

    uint32_olcb_t buffer_index = 0;

    for (int i = address; i < (address + count); i++)
    {
        //    (*buffer)[buffer_index] = user_data[i];

        (*buffer)[buffer_index] = '\0';
        buffer_index++;
    }
    return count;
}

uint16_olcb_t OSxDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer)
{

    printf("configmem write\n");

    if (count == 0)
        return 0;

    uint32_olcb_t buffer_index = 0;

    for (int i = address; i < (address + count); i++)
    {
        //      user_data[i] = (*buffer)[buffer_index];
        buffer_index++;
    }
    // the caller may or may not have added the null to the string
    // if (user_data[address + count - 1] != '\0')
    //   user_data[address + count] = '\0';

    return count;
}

void OSxDrivers_pause_100ms_timer()
{
    _timer_pause = TRUE;
}

void OSxDrivers_resume_100ms_timer()
{
    _timer_pause = FALSE;
}