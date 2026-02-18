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
#include "esp32_drivers.h"
#include "esp32_can_drivers.h"

#include "src/drivers/canbus/can_config.h"
#include "src/openlcb/openlcb_config.h"


#define NODE_ID 0x050101010777

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &Esp32CanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &Esp32CanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &Esp32Drivers_lock_shared_resources,
    .unlock_shared_resources = &Esp32Drivers_unlock_shared_resources,
    .on_rx                   = &Callbacks_on_can_rx_callback,
    .on_tx                   = &Callbacks_on_can_tx_callback,
    .on_alias_change         = &Callbacks_alias_change_callback,
};

static const openlcb_config_t openlcb_config = {
    .lock_shared_resources   = &Esp32Drivers_lock_shared_resources,
    .unlock_shared_resources = &Esp32Drivers_unlock_shared_resources,
    .config_mem_read         = &Esp32Drivers_config_mem_read,
    .config_mem_write        = &Esp32Drivers_config_mem_write,
    .reboot                  = &Esp32Drivers_reboot,
    .freeze                  = &Callbacks_freeze,
    .unfreeze                = &Callbacks_unfreeze,
    .firmware_write          = &Callbacks_write_firemware,
    .factory_reset           = &Callbacks_operations_request_factory_reset,
    .on_100ms_timer          = &Callbacks_on_100ms_timer_callback,
};

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(9600);

  Serial.println("Can Statemachine init.....");

  Esp32CanDriver_setup();
  Esp32Drivers_setup();

  CanConfig_initialize(&can_config);
  OpenLcb_initialize(&openlcb_config, OPENLCB_PROFILE_STANDARD | OPENLCB_FEATURE_FIRMWARE_UPGRADE);

  Callbacks_initialize();

  Serial.println("Creating Node.....");

  OpenLcb_create_node(NODE_ID, &NodeParameters_main_node);
}

void loop()
{
  // put your main code here, to run repeatedly
  OpenLcb_run();
}
