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
 * \file wifi_tools.c
 *
 *
 * @author Jim Kueneman
 * @date 15 Nov 2025
 */

#define ARDUINO_COMPATIBLE

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "wifi_tools.h"

#include "wifi_tools_debug.h"

#ifdef ARDUINO_COMPATIBLE
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
#endif //  ARDUINO_COMPATIBLE

typedef struct
{

	bool event_logging_enabled;
	uint32_t reconnect_timer;
	bool should_reconnect;
	bool first_disconnect;

} esp32_wifi_info_t;

typedef struct
{

	uint32_t ip_address;
	uint16_t port;
	bool is_connected_to_access_point;
	bool is_connected_to_server;
	int sock;

} esp32_wifi_connection_info_t;

static esp32_wifi_info_t _esp32_wifi_info = {
	.event_logging_enabled = false,
	.reconnect_timer = 0,
	.should_reconnect = false,
	.first_disconnect = false

};

static esp32_wifi_connection_info_t _esp32_wifi_connection_info = {

	.ip_address = 0x00000000,
	.port = 12021,
	.is_connected_to_access_point = false,
	.is_connected_to_server = false,
	.sock = -1

};

static const char *TAG = "TCP SOCKET Client";
static const char *payload = "Message from ESP32 TCP Socket Client";

// callback called by the ESP WiFi module
static void _wifi_event_handler(WiFiEvent_t event, WiFiEventInfo_t info)
{

	if (_esp32_wifi_info.event_logging_enabled)
	{

		WifiToolsDebug_log_event(event, info);
	}

	if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
	{

		if (_esp32_wifi_connection_info.is_connected_to_access_point)
		{

			Serial.println("\n\tdisconnected...\n");
		}

		_esp32_wifi_connection_info.is_connected_to_access_point = false;
		_esp32_wifi_connection_info.is_connected_to_server = false;
		_esp32_wifi_connection_info.ip_address = 0x00000000;

		bool user_disconnected = info.wifi_sta_disconnected.reason == WIFI_REASON_ASSOC_LEAVE;

		_esp32_wifi_info.should_reconnect = !(user_disconnected || _esp32_wifi_info.first_disconnect);
		_esp32_wifi_info.first_disconnect = false;
	}

	if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP)
	{

		if (!_esp32_wifi_connection_info.is_connected_to_access_point)
		{

			_esp32_wifi_connection_info.ip_address = WiFi.localIP();
			String szRet = WiFi.localIP().toString();
			Serial.print("\n\tconnected: IP: ");
			Serial.println(szRet);
		}

		_esp32_wifi_connection_info.is_connected_to_access_point = true;
	}
}

void WiFiTools_reconnect_to_access_point()
{

	if ((millis() - _esp32_wifi_info.reconnect_timer > RECONNECT_INTERVAL) && _esp32_wifi_info.should_reconnect)
	{

		Serial.println("\n\treconnecting...\n");
		WiFi.reconnect();
		_esp32_wifi_info.reconnect_timer = millis();
	}
}

void WiFiTools_connect_to_access_point(const char *ssid, const char *pass)
{

	WiFi.setAutoReconnect(false); // we handle autoconnect better
	WiFi.persistent(false);		  // do not store my credentials in the ESP32
	WiFi.onEvent(&_wifi_event_handler);
	WiFi.begin(ssid, pass);
}

void WiFiTools_log_events(bool do_enable)
{

	_esp32_wifi_info.event_logging_enabled = do_enable;
}

bool WiFiTools_is_connected_to_access_point(void)
{

	return _esp32_wifi_connection_info.is_connected_to_access_point;
}

bool WiFiTools_is_connected_to_server(void)
{

	return _esp32_wifi_connection_info.is_connected_to_server;
}

void WiFiTools_close_server(void)
{

	if (_esp32_wifi_connection_info.sock > 0)
	{

		closesocket(_esp32_wifi_connection_info.sock);
	}

	_esp32_wifi_connection_info.is_connected_to_server = false;
}

int WifiTools_get_socket(void)
{

	return _esp32_wifi_connection_info.sock;
}

int WiFiTools_connect_to_server(const char *ip_address, const uint16_t port)
{

	if (!_esp32_wifi_connection_info.is_connected_to_server)
	{

		struct sockaddr_in dest_addr;
		int addr_family = 0;
		int ip_protocol = 0;

		inet_pton(AF_INET, ip_address, &dest_addr.sin_addr);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(port);

		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;

		printf("Creating a Socket...\n");

		_esp32_wifi_connection_info.sock = socket(addr_family, SOCK_STREAM, ip_protocol);

		if (_esp32_wifi_connection_info.sock < 0)
		{

			printf("Unable to create socket: errno %d\n", errno);
			_esp32_wifi_connection_info.is_connected_to_server = false;

			return -1;
		}

		printf("Socket Created\n");
		printf("Connecting to %s:%d\n", ip_address, port);

		int err = connect(_esp32_wifi_connection_info.sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

		if (err != 0)
		{
			printf("Socket unable to connect: errno %d\n", errno);
			_esp32_wifi_connection_info.is_connected_to_server = false;
			closesocket(_esp32_wifi_connection_info.sock);

			return -1;
		}

		printf("Successfully connected\n");

		_esp32_wifi_connection_info.is_connected_to_server = true;
	}

	return _esp32_wifi_connection_info.sock;
}