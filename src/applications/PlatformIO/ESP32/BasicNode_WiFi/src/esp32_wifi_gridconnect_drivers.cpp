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
 * @date 15 Nov 2025
 */

#include "esp32_wifi_gridconnect_drivers.h"

#include "Arduino.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_mac.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_system.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "nvs_flash.h"
#include "ping/ping_sock.h"
#include "driver/gpio.h"

#include "wifi_tools.h"
#include "wifi_tools_debug.h"

#include "src/drivers/common/can_rx_statemachine.h"
#include "src/drivers/common/can_types.h"
#include "src/openlcb/openlcb_gridconnect.h"
#include "src/utilities/mustangpeak_string_helper.h"
#include "src/openlcb/openlcb_gridconnect.h"

#define INCLUDE_vTaskSuspend 1

static TaskHandle_t _receive_task_handle = NULL;

static void _receive_task(void *arg)
{

  can_msg_t can_message;
  gridconnect_buffer_t gridconnect_buffer;
  char *msg;
  char next_char;
  bool connected = WiFiTools_is_connected_to_server();
  int socket = WifiTools_get_socket();
  bool do_delay = true;

  if (socket <= 0) {

    return;
  }

  while (1)
  {
       int bytes_received = recv(socket, &next_char, 1, MSG_DONTWAIT);

      // bytes_received is 0 on connection close and errno gets set to ENOTCONN (128)
      // bytes_received is -1 when no data is received and errno is set to EAGAIN (11)
      // bytes_received is > 0 when data is received and errno is set to EAGAIN (11) still.

      if (bytes_received > 0) {

        do_delay = false; // may be more coming

        if (OpenLcbGridConnect_copy_out_gridconnect_when_done(next_char, &gridconnect_buffer)) {

          OpenLcbGridConnect_to_can_msg(&gridconnect_buffer, &can_message);

          printf("[R] %s\n", (char *)&gridconnect_buffer);
  
          CanRxStatemachine_incoming_can_driver_callback(&can_message);

        }
      
      } else if (bytes_received == 0) {  // error found...socket closed?

        printf("return %d. errno %d\n", bytes_received, errno);

        WiFiTools_close_server();
        _receive_task_handle = NULL;

        vTaskDelete(NULL);

      } else {

        do_delay = true;  // returned no bytes in receive so restart the delay

      }
  
      if (do_delay) {

        vTaskDelay(1000 / portTICK_PERIOD_MS);

      }

  }

}

bool Esp32WiFiGridconnectDriver_is_can_tx_buffer_clear(void)
{

  return true;
}

bool Esp32WiFiGridconnectDriver_transmit_raw_can_frame(can_msg_t *msg)
{

  gridconnect_buffer_t gridconnect_buffer;
  ssize_t result = 0;

  if (!WiFiTools_is_connected_to_server) {

    return false;
  }

  OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, msg);

  printf("[S] %s\n", (char *)&gridconnect_buffer);

  return (send(WifiTools_get_socket(), &gridconnect_buffer, strlen((char*)&gridconnect_buffer), 0) > 0);
}

void Esp32WiFiGridconnectDriver_pause_can_rx(void)
{
  if(_receive_task_handle) {

    vTaskSuspend(_receive_task_handle);

  }

}

void Esp32WiFiGridconnectDriver_resume_can_rx(void)
{

  if (_receive_task_handle) {

    vTaskResume(_receive_task_handle);

  }

}

void Esp32WiFiGridconnectDriver_start(int *socket)
{

  xTaskCreate(
      _receive_task,        // [IN] function to call
      "receive_task",       // [IN] user identifier
      2048,                 // [IN] Stack Size
      NULL,                 // [IN] Parameter to pass
      10,                   // [IN] Task Priority
      &_receive_task_handle // [OUT] Task Handle send pointer to a TaskHandle_t variable
  );

}
