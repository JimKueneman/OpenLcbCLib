
#if NATIVE == 0

#include <Arduino.h>

#endif

#include "stdio.h"
#include "unistd.h"
#include "src/drivers/common/can_main_statemachine.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/drivers/driver_mcu.h"
#include "src/drivers/driver_can.h"
#include "src/drivers/driver_100ms_clock.h"
#include "src/openlcb/openlcb_node.h"
#include "node_parameters.h"
#include "osx_drivers.h"
#include "osx_can_drivers.h"


int main(void)
{

  printf("Initializing...\n");

  CanMainStatemachine_initialize(
      &OSxCanDriver_setup,
      &OSxCanDriver_transmit_raw_can_frame,
      &OSxCanDriver_is_can_tx_buffer_clear,
      &OSxCanDriver_pause_can_rx,
      &OSxCanDriver_resume_can_rx);

  MainStatemachine_initialize(
      &OSxDrivers_setup,
      &OSxDrivers_reboot,
      &OSxDrivers_config_mem_read,
      &OSxDrivers_config_mem_write,
      &OSxDrivers_pause_100ms_timer,
      &OSxDrivers_resume_100ms_timer);

  printf("Waiting for CAN and 100ms Timer Drivers to connect\n");

  while (!OSxDrivers_100ms_is_connected() || !OSxCanDriver_is_connected())
  {

    printf("sleeping\n");

    sleep(2);
  }

  printf("Creating Node\n");
  Node_allocate(0x0501010107FF, &NodeParameters_main_node);
  printf("Logging in.....\n");

  while (1)
  {

    CanMainStateMachine_run();

    usleep(500);
  }

  printf("exiting\n");
}
