/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file callbacks_olcb.c
 *
 * Core node lifecycle callback implementations for the BasicNode demo.
 *
 * @author Jim Kueneman
 * @date 15 Mar 2026
 */

#include "callbacks_olcb.h"

#include <stdio.h>

#include "main.h"

#include "../openlcb_c_lib/openlcb/openlcb_defines.h"

static uint16_t _100ms_ticks = 0;

void CallbacksOlcb_initialize(void) {

}

void CallbacksOlcb_on_100ms_timer(void) {

    HAL_GPIO_TogglePin(_100MS_TIMER_LED_RED_GPIO_Port,
            _100MS_TIMER_LED_RED_Pin);

    if (_100ms_ticks > 5) {

        HAL_GPIO_WritePin(CAN_RX_ORANGE_LED_GPIO_Port, CAN_RX_ORANGE_LED_Pin,
                GPIO_PIN_RESET); // turn off
        HAL_GPIO_WritePin(CAN_TX_LED_BLUE_GPIO_Port, CAN_TX_LED_BLUE_Pin,
                GPIO_PIN_RESET);

        _100ms_ticks = 0;
    }

    _100ms_ticks++;

}

void CallbacksOlcb_firmware_write(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, write_result_t write_result) {

    printf("Firmware Write, buffer is in config_mem_write_request_info->writebuffer ");

    write_result(statemachine_info, config_mem_write_request_info, true);

}

void CallbacksOlcb_freeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        printf("Requesting Firmware update");
    }

}

void CallbacksOlcb_unfreeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        printf("Requesting Firmware firmware update complete, reboot");
    }

}
