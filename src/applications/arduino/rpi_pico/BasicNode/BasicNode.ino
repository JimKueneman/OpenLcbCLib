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

#include "callbacks.h"
#include "node_parameters.h"
#include "src/application_drivers/rpi_pico_drivers.h"
#include "src/application_drivers/rpi_pico_can_drivers.h"
#include "src/node_definition/dependency_injection.h"
#include "src/node_definition/dependency_injection_canbus.h"

#include "src/drivers/canbus/can_main_statemachine.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/openlcb_login_statemachine.h"
#include "src/openlcb/openlcb_node.h"


#define NODE_ID 0x050101010777

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);

  Serial.println("Can Statemachine init.....");
  
  RPiPicoCanDriver_setup();
  RPiPicoDriver_setup();

  DependencyInjectionCanBus_initialize();
  DependencyInjection_initialize();

  Callbacks_initialize();

  Serial.println("Creating Node.....");

  OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);
}

void loop()
{
  // put your main code here, to run repeatedly
  CanMainStateMachine_run();
  OpenLcbLoginMainStatemachine_run();
  OpenLcbMainStatemachine_run();
}
