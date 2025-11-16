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

// So the mutex will function
#define INCLUDE_vTaskSuspend 1

static const char *TAG = "TCP SOCKET Client";
static const char *payload = "Message from ESP32 TCP Socket Client";
TaskHandle_t receive_task_handle = (void *)0;

void receive_task(void *arg)
{

  // //  SemaphoreHandle_t local_mutex = (SemaphoreHandle_t)arg;
  // can_msg_t can_msg;
  // can_msg.state.allocated = 1;

  // char rx_buffer[128];
  // char host_ip[] = "192.168.1.160"; // Server IP
  // int addr_family = 0;
  // int ip_protocol = 0;
  // uint16_t port = 12021;

  // struct sockaddr_in dest_addr;

  // inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
  // dest_addr.sin_family = AF_INET;
  // dest_addr.sin_port = htons(port);

  // addr_family = AF_INET;
  // ip_protocol = IPPROTO_IP;

  // int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
  // if (sock < 0)
  // {

  //   ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);

  // }

  // ESP_LOGI(TAG, "Socket created ");
  // ESP_LOGI(TAG, "Connecting to %s:%d", host_ip, PORT);

  // int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
  // if (err != 0)
  // {
  //   ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
    
  // }

  // ESP_LOGI(TAG, "Successfully connected");

  while (1)
  {

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

bool Esp32WiFiGridconnectDriver_is_connected_to_access_point(void)
{

  return WiFiTools_is_connected_to_access_point();
}

bool Esp32WiFiGridconnectDriver_is_connected_to_server(void)
{

  return WiFiTools_is_connected_to_server();
}

bool Esp32WiFiGridconnectDriver_is_can_tx_buffer_clear(void)
{

  return true;
}

bool Esp32WiFiGridconnectDriver_transmit_raw_can_frame(can_msg_t *msg)
{

  return true;
}

void Esp32WiFiGridconnectDriver_pause_can_rx(void)
{

  vTaskSuspend(receive_task_handle);
}

void Esp32WiFiGridconnectDriver_resume_can_rx(void)
{

  vTaskResume(receive_task_handle);
}

void Esp32WiFiGridconnectDriver_log_into_access_point(const char *ssid, const char *pass)
{


  xTaskCreate(
      receive_task,          // [IN] function to call
      "receive_task",        // [IN] user identifier
      1024,                  // [IN] Stack Size
      NULL,                  // [IN] Paramter to pass
      10,                    // [IN] Task Priority
      &receive_task_handle); // [OUT] Task Handle send pointer to a TaskHandle_t variable
}
