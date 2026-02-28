
#include <Arduino.h>

#include "stdio.h"
#include "unistd.h"

#include "node_parameters.h"
#include "callbacks.h"

#include "application_drivers/esp32_can_drivers.h"
#include "application_drivers/esp32_drivers.h"

#include "src/drivers/canbus/can_config.h"
#include "src/openlcb/openlcb_config.h"

#define NODE_ID 0x050101010788

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
    .firmware_write          = &Callbacks_write_firmmware,
    .factory_reset           = &Callbacks_operations_request_factory_reset,
    .on_100ms_timer          = &Callbacks_on_100ms_timer_callback,
};

void setup()
{

  Serial.begin(921600);
  Serial.println("Can Statemachine init.....");

  Esp32CanDriver_setup();
  Esp32Drivers_setup();

  CanConfig_initialize(&can_config);
  OpenLcb_initialize(&openlcb_config);

  Callbacks_initialize();

  Serial.println("Creating Node.....");

  OpenLcb_create_node(NODE_ID, &NodeParameters_main_node);
}

void loop()
{

  OpenLcb_run();
}
