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

#define ARDUINO_COMPATIBLE

#include "esp32_can_drivers.h"

#include "../drivers/canbus/can_rx_statemachine.h"
#include "../drivers/canbus/can_types.h"
#include "../openlcb/openlcb_gridconnect.h"
#include "../utilities/mustangpeak_string_helper.h"

#ifdef ARDUINO_COMPATIBLE
#include "Arduino.h"
#include "driver/gpio.h"
#include "driver/twai.h"
#endif

// So the mutex will function
#define INCLUDE_vTaskSuspend 1

bool _is_connected = false;
TaskHandle_t receive_task_handle = (void *)0;
int _tx_queue_len = 0;

void receive_task(void *arg)
{

  //  SemaphoreHandle_t local_mutex = (SemaphoreHandle_t)arg;
  can_msg_t can_msg;
  can_msg.state.allocated = 1;

  while (1)
  {

    twai_message_t message;
    esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(100));

    switch (err)
    {

    case ESP_OK:

      if (message.extd) // only accept extended format
      {

        can_msg.identifier = message.identifier;
        can_msg.payload_count = message.data_length_code;
        for (int i = 0; i < message.data_length_code; i++)
        {

          can_msg.payload[i] = message.data[i];
        }

        CanRxStatemachine_incoming_can_driver_callback(&can_msg);
      }

      break;

    case ESP_ERR_TIMEOUT:

      break;

    default:

      break;
    }
  }
}

bool Esp32CanDriver_is_connected(void)
{

  return _is_connected;
}

bool Esp32CanDriver_is_can_tx_buffer_clear(void)
{

  twai_status_info_t status;

  // This return value is broken, twai_get_status_info returns TWAI_STATE_STOPPED (also equal to ESP_OK = 0) for a valid system which is not right.
  // That said the value of status.state IS correct on the return of this call
  twai_get_status_info(&status);

  switch (status.state)
  {

  case TWAI_STATE_STOPPED: //  // = "0" Same value as ESP_OK

    return false;

    break;

  case TWAI_STATE_RUNNING: // = "1"

    return ((_tx_queue_len - status.msgs_to_tx) > 0);

    break;

  case TWAI_STATE_BUS_OFF: // = "2"

    twai_initiate_recovery();

    return false;

    break;

  case TWAI_STATE_RECOVERING: // = "3"

    return false;

    break;

  default:

    return false;

    break;
  }
}

bool Esp32CanDriver_transmit_raw_can_frame(can_msg_t *msg)
{

  // Configure message to transmit
  twai_message_t message;
  message.identifier = msg->identifier;
  message.extd = 1; // Standard vs extended format
  message.data_length_code = msg->payload_count;
  message.rtr = 0;          // Data vs RTR frame
  message.ss = 0;           // Whether the message is single shot (i.e., does not repeat on error)
  message.self = 0;         // Whether the message is a self reception request (loopback)
  message.dlc_non_comp = 0; // DLC is less than 8

  for (int i = 0; i < msg->payload_count; i++)
  {

    message.data[i] = msg->payload[i];
  }

  // Queue message for transmission
  if (twai_transmit(&message, 0) == ESP_OK)
  {

    return true;
  }
  else
  {

    return false;
  }
}

void Esp32CanDriver_pause_can_rx(void)
{

  vTaskSuspend(receive_task_handle);
}

void Esp32CanDriver_resume_can_rx(void)
{

  vTaskResume(receive_task_handle);
}

void Esp32CanDriver_setup(void)
{

  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  _tx_queue_len = g_config.tx_queue_len;

  // Install CAN driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
  {
    // Start CAN driver
    if (twai_start() == ESP_OK)
    {

      _is_connected = true;

      xTaskCreate(
          receive_task,          // [IN] function to call
          "receive_task",        // [IN] user identifier
          2048,                  // [IN] Stack Size
          NULL,                  // [IN] Paramter to pass
          10,                    // [IN] Task Priority
          &receive_task_handle); // [OUT] Task Handle send pointer to a TaskHandle_t variable
    }
    else
    {
    }
  }
  else
  {
  }
}
