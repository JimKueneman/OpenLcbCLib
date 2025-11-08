#include <stdbool.h>
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
 * \file esp32_can_drivers.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Jan 2025
 */

#include "esp32_can_drivers.h"

#include "src/drivers/common/can_rx_statemachine.h"
#include "src/drivers/common/can_types.h"
#include "src/openlcb/openlcb_gridconnect.h"
#include "src/utilities/mustangpeak_string_helper.h"

#include "Arduino.h"
#include "driver/gpio.h"
#include "driver/twai.h"

// So the mutex will function
#define INCLUDE_vTaskSuspend 1

bool _is_connected = false;
TaskHandle_t receive_task_handle = (void *)0;
SemaphoreHandle_t mutex;

void receive_task(void *arg) {

  SemaphoreHandle_t local_mutex = (SemaphoreHandle_t)arg;
  can_msg_t can_msg;
  can_msg.state.allocated = 1;

  while (1) {
    xSemaphoreTake(local_mutex, portMAX_DELAY);  //  INCLUDE_vTaskSuspend is defined as 1 above so this is a forever wait


    twai_message_t message;
    esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(100));
    if (err == ESP_OK) {
      if (message.extd)  // only accept extended format
      {
        can_msg.identifier = message.identifier;
        can_msg.payload_count = message.data_length_code;
        for (int i = 0; i < message.data_length_code; i++)
          can_msg.payload[i] = message.data[i];

        CanRxStatemachine_incoming_can_driver_callback(&can_msg);

        digitalWrite(2, !digitalRead(2));  // blink the onboard LED
      }
    } else if (err == ESP_ERR_TIMEOUT) {
      digitalWrite(2, 0);  // turn off the LED
    } else {
    }

    xSemaphoreGive(local_mutex);

   // vTaskDelay(10 / portTICK_PERIOD_MS);  // 10ms
  }
}

void Esp32CanDriver_config_mem_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

  printf("Resetting to Factory Defaults\n");

  // TODO: implement based on how Config Mem is stored
}

void Esp32CanDriver_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

  printf("Rebooting\n");

  // TODO: Implement
}

bool Esp32CanDriver_is_connected(void) {

  return _is_connected;
}

bool Esp32CanDriver_is_can_tx_buffer_clear(void) {
  // The Esp32CanDriver_transmit_raw_can_frame will return FALSE if it can't send it for now.
  // Keep looking for an answer......
  return true;
}

bool Esp32CanDriver_transmit_raw_can_frame(can_msg_t *msg) {

  //xSemaphoreTake(mutex, portMAX_DELAY);  //  INCLUDE_vTaskSuspend is defined as 1 above so this is a forever wait

  // Configure message to transmit
  twai_message_t message;
  message.identifier = msg->identifier;
  message.extd = 1;
  message.data_length_code = msg->payload_count;
  for (int i = 0; i < msg->payload_count; i++) {
    message.data[i] = msg->payload[i];
  }

  // Queue message for transmission
  if (twai_transmit(&message, pdMS_TO_TICKS(1)) == ESP_OK) {
    return true;
  } else {
    return false;
  }

  //xSemaphoreGive(mutex);  //  INCLUDE_vTaskSuspend is defined as 1 above so this is a forever wait
}

void Esp32CanDriver_pause_can_rx(void) {

  xSemaphoreTake(mutex, portMAX_DELAY);  //  INCLUDE_vTaskSuspend is defined as 1 above so this is a forever wait

}

void Esp32CanDriver_resume_can_rx(void) {

  xSemaphoreGive(mutex);
}

void Esp32CanDriver_setup(void) {

  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install CAN driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    // Start CAN driver
    if (twai_start() == ESP_OK) {
      _is_connected = true;

      mutex = xSemaphoreCreateMutex();

      xTaskCreate(
        receive_task,           // [IN] function to call
        "receive_task",         // [IN] user identifier
        1024,                   // [IN] Stack Size
        (void *)mutex,          // [IN] Paramter to pass
        10,                     // [IN] Task Priority
        &receive_task_handle);  // [OUT] Task Handle send pointer to a TaskHandle_t variable
    } else {
    }
  } else {
  }
}
