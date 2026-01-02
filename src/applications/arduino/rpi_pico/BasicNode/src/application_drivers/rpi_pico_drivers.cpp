
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
 * \odx_drivers.c
 *
 *
 *
 * @author Jim Kueneman
 * @date 7 Nov 2025
 */

#define ARDUINO_COMPATIBLE

#include "rpi_pico_drivers.h"
#include "rpi_pico_can_drivers.h"

#include "../openlcb/openlcb_types.h"
#include "../openlcb/openlcb_defines.h"
#include "../utilities/mustangpeak_string_helper.h"
#include "../openlcb/openlcb_node.h"
#include "../openlcb/protocol_datagram_handler.h"

#ifdef ARDUINO_COMPATIBLE
// TODO:  include any header files the Raspberry Pi Pico need to compile under Arduino/PlatformIO
#include "Arduino.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdio.h>
#endif

// Create a Timer interrupt or task and call the following
struct repeating_timer timer;
static bool timer_enabled = false;
static bool timer_unhandled_tick = false;

void _handle_timer_tick(void) {

  OpenLcbNode_100ms_timer_tick();
  ProtocolDatagramHandler_100ms_timer_tick();
}

bool timer_task_or_interrupt(__unused struct repeating_timer *timer) {
  // You can access user data if needed:
  // int* data = (int*) t->user_data;
  // printf("Timer callback fired, data value: %d\n", *data);

  // a Timer interrupt or task (if using RTOS) and call the following every 100ms or so (not critical)
  if (timer_enabled) {

    _handle_timer_tick();

  } else {

    timer_unhandled_tick = true;
  }
  // printf("Timer fired!\n");
  return true;  // Keep the timer running
}

void RPiPicoDriver_setup(void) {
  // Initialize Raspberry Pi Pico interrupt and any features needed for config memory read/writes

  timer_enabled = add_repeating_timer_ms(-100, timer_task_or_interrupt, NULL, &timer);
}

void RPiPicoDrivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

  watchdog_enable(1, 1);  // Enable watchdog with a 1ms timeout and pause on debug
  while (1)
    ;  // Enter infinite loop, watchdog will trigger reset
}


static char str_name[] = "Raspberry Pi Pico";
static char str_desc[] = "This is my RPi Pico Test Bed with OpenLcbCLib";

uint16_t RPiPicoDrivers_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

  // TODO: Read to EEPROM/FLASH/FRAM/........

  switch (address) {

    case CONFIG_MEM_USER_MODEL_ADDRESS:
      {
        for (int i = 0; i < count; i++) {

          *buffer[i] = str_name[i];

          if (*buffer[i] == 0x00) {

            return i;
          }
        }

        break;
      }

    case CONFIG_MEM_USER_DESCRIPTION_ADDRESS:
      {

        for (int i = 0; i < count; i++) {

          *buffer[i] = str_desc[i];

          if (*buffer[i] == 0x00) {

            return i;
          }
        }

        break;
      }
  }

  return 0;
}

uint16_t RPiPicoDrivers_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

  // TODO: Write to EEPROM/FLASH/FRAM/........

  switch (address) {

    case CONFIG_MEM_USER_MODEL_ADDRESS:
      {
        for (int i = 0; i < count; i++) {

          str_name[i] = *buffer[i];

          if (*buffer[i] == 0x00) {

            return i;
          }
        }

        break;
      }

    case CONFIG_MEM_USER_DESCRIPTION_ADDRESS:
      {

        for (int i = 0; i < count; i++) {

          str_desc[i] = *buffer[i];

          if (*buffer[i] == 0x00) {

            return i;
          }
        }

        break;
      }
  }

  return 0;
}

void RPiPicoDrivers_lock_shared_resources(void) {

  // Pause the 100ms Timer here
  timer_enabled = false;
  // Pause the CAN Rx thread
  RPiPicoCanDriver_pause_can_rx();
}

void RPiPicoDrivers_unlock_shared_resources(void) {
  // TODO: Resume the 100ms Timer here
  timer_enabled = true;
  // Resume the CAN Rx thread
  RPiPicoCanDriver_resume_can_rx();

  if (timer_unhandled_tick) {

    timer_unhandled_tick = false;
    _handle_timer_tick();
  }
}
