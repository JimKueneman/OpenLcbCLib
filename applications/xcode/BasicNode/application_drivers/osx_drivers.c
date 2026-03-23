/** \copyright
 * Copyright (c) 2026 Jim Kueneman
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
 * \file osx_drivers.c
 *
 * macOS platform drivers for the BasicNode demo: 100ms timer thread,
 * keyboard input thread, configuration memory stubs, and firmware callbacks.
 *
 * @author Jim Kueneman
 * @date 1 Jan 2026
 */

#include "osx_drivers.h"

#include "osx_can_drivers.h"

#include "../openlcb_c_lib/openlcb/openlcb_types.h"
#include "../openlcb_c_lib/utilities/mustangpeak_string_helper.h"
#include "../openlcb_c_lib/openlcb/openlcb_config.h"
#include "../openlcb_c_lib/openlcb/openlcb_node.h"
#include "../openlcb_c_lib/openlcb/openlcb_application.h"
#include "../openlcb_c_lib/openlcb/openlcb_utilities.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

uint8_t _is_clock_running = false;
char *user_data;
uint8_t _is_input_running = false;

pthread_mutex_t OSxDrivers_input_mutex;

uint8_t OSxDrivers_input_is_connected(void) {

    return _is_input_running;
}

void *thread_function_input(void *arg) {

    int thread_id = *((int *)arg);

    char key;

    printf("Input Thread %d started\n", thread_id);

    _is_input_running = true;

    while (1) {

        if (scanf("%c", &key) == EOF)
            break;

        pthread_mutex_lock(&OSxDrivers_input_mutex);

        switch (key) {

            case 'r':
                printf("reboot\n");
                OpenLcbNode_reset_state();
                break;
        }

        pthread_mutex_unlock(&OSxDrivers_input_mutex);

        usleep(100000);
    }

    return NULL;
}

void *thread_function_timer(void *arg) {

    int thread_id = *((int *)arg);

    printf("100ms Timer Thread %d started\n", thread_id);

    _is_clock_running = true;

    while (1) {

        OpenLcb_100ms_timer_tick();

        usleep(100000);
    }
}

uint8_t OSxDrivers_100ms_is_connected(void) {

    return _is_clock_running;
}

void OSxDrivers_initialize(void) {

    user_data = strnew_initialized(LEN_SNIP_USER_NAME_BUFFER + LEN_SNIP_USER_DESCRIPTION_BUFFER + 1);

    pthread_t thread2;
    int thread_num2 = 2;
    pthread_create(&thread2, NULL, thread_function_timer, &thread_num2);

    printf("Mutex initialization for Input - Result Code: %d\n", pthread_mutex_init(&OSxDrivers_input_mutex, NULL));

    pthread_t thread3;
    int thread_num3 = 3;
    pthread_create(&thread3, NULL, thread_function_input, &thread_num3);
}

void OSxDrivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    printf("Reboot requested via protocol\n");
    OpenLcbNode_reset_state();
}

uint16_t OSxDrivers_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

    char str[] = "iMac M1 on XCode";

    for (int i = 0; i < count; i++) {
        (*buffer)[i] = 0x00;
    }

    switch (address) {

        case 0:
            for (int i = 0; i < count; i++) {
                (*buffer)[i] = str[i];
            }
            break;

        default:
            break;
    }

    return count;
}

uint16_t OSxDrivers_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

    printf("configmem write\n");

    if (count == 0) {
        return 0;
    }

    return count;
}

void OSxDrivers_lock_shared_resources(void) {

    OSxCanDriver_pause_can_rx();
}

void OSxDrivers_unlock_shared_resources(void) {

    OSxCanDriver_resume_can_rx();
}

void OSxDrivers_firmware_write(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, write_result_t write_result) {

    printf("Firmware Write, buffer is in config_mem_write_request_info->writebuffer ");

    write_result(statemachine_info, config_mem_write_request_info, true);
}

void OSxDrivers_freeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        printf("Requesting Firmware update\n");

        statemachine_info->openlcb_node->state.firmware_upgrade_active = true;

        OpenLcbApplication_send_initialization_event(statemachine_info->openlcb_node);
    }
}

void OSxDrivers_unfreeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        printf("Firmware update complete, reboot\n");

        statemachine_info->openlcb_node->state.firmware_upgrade_active = false;
    }
}
