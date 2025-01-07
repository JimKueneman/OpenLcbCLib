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
 * \file BasicNode.ino
 *
 * This sketh will create a very basic OpenLcb Node.  It needs the Config Memory handlers and a reset impementation finished (esp32_drivers.c)
 * Connect the CAN transciever Tx pin to GPIO 21 adn the Rx pin to GPIO 22 on the ESP32 Dev Board.
 *
 * @author Jim Kueneman
 * @date 7 Jan 2025
 */

#include "src/drivers/common/can_main_statemachine.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/drivers/driver_mcu.h"
#include "src/openlcb/openlcb_node.h"
#include "node_parameters.h"
#include "esp32_can_drivers.h"
#include "esp32_drivers.h"

#include "esp_pm.h"

#define LED_BUILTIN 2

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, 1);

  Serial.begin(9600);

  Serial.println("Can Statemachine init.....");
 
  CanMainStatemachine_initialize(
      &Esp32CanDriver_setup,
      &Esp32CanDriver_transmit_raw_can_frame,
      &Esp32CanDriver_is_can_tx_buffer_clear,
      &Esp32CanDriver_pause_can_rx,
      &Esp32CanDriver_resume_can_rx);

      Serial.println("Main Statemachine init.....");

  MainStatemachine_initialize(
      &Esp32Drivers_setup,
      &Esp32Drivers_reboot,
      &Esp32Drivers_config_mem_read,
      &Esp32Drivers_config_mem_write,
      &Esp32Drivers_pause_100ms_timer,
      &Esp32Drivers_resume_100ms_timer);

Serial.println("Creating Node.....");

  Node_allocate(0x0501010107FF, &NodeParameters_main_node);

}

void loop() {
  // put your main code here, to run repeatedly
  CanMainStateMachine_run();

}
