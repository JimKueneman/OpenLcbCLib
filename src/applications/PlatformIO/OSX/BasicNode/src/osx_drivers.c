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
uint8_olcb_t _is_input_running = FALSE;

pthread_mutex_t OSxDdrivers_input_mutex;

parameterless_callback_t _100ms_timer_sink_func = (void *)0;

uint8_olcb_t OSxDrivers_input_is_connected(void)
{

    return _is_input_running;
}

void *thread_function_input(void *arg)
{

    int thread_id = *((int *)arg); // Access argument passed to thread

    char key;

    printf("Input Thread %d started\n", thread_id);

    _is_input_running = TRUE;

    while (1)
    {
        scanf("%c", &key);

        pthread_mutex_lock(&OSxDdrivers_input_mutex);

        switch (key)
        {

            // use this to handle any keyboard input
        }

        pthread_mutex_unlock(&OSxDdrivers_input_mutex);

        usleep(100000);
    }
}

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

uint8_olcb_t OSxDrivers_100ms_is_connected(void)
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

    printf("Mutex initialization for Input - Result Code: %d\n", pthread_mutex_init(&OSxDdrivers_input_mutex, NULL));

    pthread_t thread3;

    int thread_num3 = 3;

    pthread_create(&thread3, NULL, thread_function_input, &thread_num3);
}

void OSxDrivers_reboot(void)
{
}

uint16_olcb_t OSxDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer)
{

    //  printf("configmem read count: %d\n", count);
    //  printf("configmem read address: %08lX\n", address);

    // printf("configmem read address: %02X\n", (*buffer)[0]);

    // Null out the buffer in case we have no data
    for (int i = 0; i < sizeof(*buffer); i++)
        (*buffer)[i] = '\0';

    FILE *_file;

    _file = fopen("../config_mem.dat", "rb"); // read binary

    if (_file)
    {

        if (fseek(_file, address, SEEK_SET) == 0)
        {

            if (fread(buffer, 1, count, _file) == count)
            {

                fclose(_file);
                return count;
            }
        }

        fclose(_file);
    }

    return count; // just returning 0's
}

uint16_olcb_t OSxDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer)
{

    //  printf("configmem write\n");

    if (count == 0)
        return 0;

    FILE *_file;

    _file = fopen("../config_mem.dat", "r+b"); // append will ONLY add to the EOF not past it but r+ will fail if the file does not exist
    if (!_file)
        _file = fopen("../config_mem.dat", "w+b"); // append binary

    if (_file)
    {

        if (fseek(_file, address, SEEK_SET) == 0)
        {

            if (fwrite(buffer, 1, count, _file) == count)
            {

                fclose(_file);
                return count;
            }
        }

        fclose(_file);
    }

    return 0;
}

void OSxDrivers_pause_100ms_timer(void)
{
    _timer_pause = TRUE;
}

void OSxDrivers_resume_100ms_timer(void)
{
    _timer_pause = FALSE;
}
