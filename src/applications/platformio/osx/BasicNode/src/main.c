


#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include "strings.h"
#include "pthread.h"

#include "callbacks.h"
#include "node_parameters.h"
#include "application_drivers/osx_drivers.h"
#include "application_drivers/osx_can_drivers.h"

#include "src/drivers/canbus/can_config.h"
#include "src/openlcb/openlcb_config.h"

uint64_t node_id_base = 0x0507010100BB;

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &OSxCanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &OSxCanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &OSxCanDriver_pause_can_rx,
    .unlock_shared_resources = &OSxCanDriver_resume_can_rx,
    .on_rx                   = &Callbacks_on_can_rx_callback,
    .on_tx                   = &Callbacks_on_can_tx_callback,
    .on_alias_change         = &Callbacks_alias_change_callback,
};

static const openlcb_config_t openlcb_config = {
    .lock_shared_resources   = &OSxCanDriver_pause_can_rx,
    .unlock_shared_resources = &OSxCanDriver_resume_can_rx,
    .config_mem_read         = &OSxDrivers_config_mem_read,
    .config_mem_write        = &OSxDrivers_config_mem_write,
    .reboot                  = &OSxDrivers_reboot,
    .freeze                  = &Callbacks_freeze,
    .unfreeze                = &Callbacks_unfreeze,
    .firmware_write          = &Callbacks_write_firmmware,
    .factory_reset           = &Callbacks_operations_request_factory_reset,
    .on_100ms_timer          = &Callbacks_on_100ms_timer_callback,
};

int main(int argc, char *argv[])
{

  printf("Initializing...\n");


  OSxDrivers_setup();
  OSxCanDriver_setup();

  CanConfig_initialize(&can_config);
  OpenLcb_initialize(&openlcb_config, OPENLCB_PROFILE_STANDARD | OPENLCB_FEATURE_FIRMWARE_UPGRADE);

  printf("Waiting for CAN and 100ms Timer Drivers to connect\n");

  while (!(OSxDrivers_100ms_is_connected() && OSxCanDriver_is_connected() && OSxDrivers_input_is_connected()))
  {
    printf("Waiting for Threads\n");
    sleep(2);
  }

  printf("Allocating Node\n");
#ifdef PLATFORMIO
  uint64_t nodeid = 0x0501010107DD;
#else
  uint64_t nodeid = 0x050101010707;
#endif

  printf("NodeID: %12llX\n", nodeid);

  if (argc > 1)
  {
    printf("Creating with NodeID = %s\n", argv[1]);
    nodeid = strtol(argv[1], NULL, 0);

    printf("NodeID: %12llX\n", nodeid);
  }

  openlcb_node_t *node = OpenLcb_create_node(nodeid, &NodeParameters_main_node);
  printf("Allocated.....\n");

  while (1)
  {

    OpenLcb_run();
  }
}
