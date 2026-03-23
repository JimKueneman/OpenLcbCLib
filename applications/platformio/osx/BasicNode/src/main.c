



#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include "strings.h"
#include "pthread.h"

#include "application_callbacks/callbacks_can.h"
#include "application_callbacks/callbacks_olcb.h"
#include "application_callbacks/callbacks_config_mem.h"
#include "openlcb_user_config.h"
#include "application_drivers/osx_drivers.h"
#include "application_drivers/osx_can_drivers.h"

#include "openlcb_c_lib/drivers/canbus/can_config.h"
#include "openlcb_c_lib/openlcb/openlcb_config.h"

uint64_t node_id_base = 0x0507010100BB;

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &OSxCanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &OSxCanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &OSxCanDriver_pause_can_rx,
    .unlock_shared_resources = &OSxCanDriver_resume_can_rx,
    .on_rx                   = &CallbacksCan_on_rx,
    .on_tx                   = &CallbacksCan_on_tx,
    .on_alias_change         = &CallbacksCan_on_alias_change,
};

static const openlcb_config_t openlcb_config = {
    .lock_shared_resources   = &OSxCanDriver_pause_can_rx,
    .unlock_shared_resources = &OSxCanDriver_resume_can_rx,
    .config_mem_read         = &OSxDrivers_config_mem_read,
    .config_mem_write        = &OSxDrivers_config_mem_write,
    .reboot                  = &OSxDrivers_reboot,
    .factory_reset           = &CallbacksConfigMem_factory_reset,
    .freeze                  = &CallbacksConfigMem_freeze,
    .unfreeze                = &CallbacksConfigMem_unfreeze,
    .firmware_write          = &CallbacksConfigMem_firmware_write,
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,
};

int main(int argc, char *argv[])
{

  printf("Initializing...\n");


  OSxDrivers_initialize();
  OSxCanDriver_initialize();

  CanConfig_initialize(&can_config);
  OpenLcb_initialize(&openlcb_config);

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

  openlcb_node_t *node = OpenLcb_create_node(nodeid, &OpenLcbUserConfig_node_parameters);
  printf("Allocated.....\n");

  int idle_count = 0;

  while (1)
  {

    OpenLcb_run();

    if (OSxCanDriver_data_was_received())
        idle_count = 0;
    else
        idle_count++;

    if (idle_count > 100)
        usleep(50);

  }
}
