
#include <Arduino.h>

#include "stdio.h"
#include "unistd.h"

#include "openlcb_user_config.h"

#include "application_callbacks/callbacks_can.h"
#include "application_callbacks/callbacks_olcb.h"
#include "application_callbacks/callbacks_config_mem.h"

#include "application_drivers/esp32_can_drivers.h"
#include "application_drivers/esp32_drivers.h"

#include "openlcb_c_lib/drivers/canbus/can_config.h"
#include "openlcb_c_lib/openlcb/openlcb_config.h"

#define NODE_ID 0x050101010788

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &Esp32CanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &Esp32CanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &Esp32Drivers_lock_shared_resources,
    .unlock_shared_resources = &Esp32Drivers_unlock_shared_resources,
    .on_rx                   = &CallbacksCan_on_rx,
    .on_tx                   = &CallbacksCan_on_tx,
    .on_alias_change         = &CallbacksCan_on_alias_change,
};

static const openlcb_config_t openlcb_config = {
    .lock_shared_resources   = &Esp32Drivers_lock_shared_resources,
    .unlock_shared_resources = &Esp32Drivers_unlock_shared_resources,
    .config_mem_read         = &Esp32Drivers_config_mem_read,
    .config_mem_write        = &Esp32Drivers_config_mem_write,
    .reboot                  = &Esp32Drivers_reboot,
    .factory_reset           = &CallbacksConfigMem_factory_reset,
    .freeze                  = &CallbacksConfigMem_freeze,
    .unfreeze                = &CallbacksConfigMem_unfreeze,
    .firmware_write          = &CallbacksConfigMem_firmware_write,
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,
};

void setup()
{

  Serial.begin(921600);
  Serial.println("Can Statemachine init.....");

  Esp32CanDriver_initialize();
  Esp32Drivers_initialize();

  CanConfig_initialize(&can_config);
  OpenLcb_initialize(&openlcb_config);

  CallbacksOlcb_initialize();

  Serial.println("Creating Node.....");

  OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);
}

void loop()
{

  OpenLcb_run();
}
