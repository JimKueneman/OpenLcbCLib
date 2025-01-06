
#include "src/drivers/common/can_main_statemachine.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/drivers/driver_mcu.h"
#include "src/openlcb/openlcb_node.h"
#include "node_parameters.h"
#include "esp32_can_drivers.h"
#include "esp32_drivers.h"


void setup() {
  // put your setup code here, to run once:

  CanMainStatemachine_initialize(
      &Esp32CanDriver_setup,
      &Esp32CanDriver_transmit_raw_can_frame,
      &Esp32CanDriver_is_can_tx_buffer_clear,
      &Esp32CanDriver_pause_can_rx,
      &Esp32CanDriver_resume_can_rx);
  MainStatemachine_initialize(
      &Esp32Drivers_setup,
      &Esp32Drivers_reboot,
      &Esp32Drivers_config_mem_read,
      &Esp32Drivers_config_mem_write,
      &Esp32Drivers_pause_100ms_timer,
      &Esp32Drivers_resume_100ms_timer);

  Node_allocate(0x0501010107FF, &NodeParameters_main_node);

}

void loop() {
  // put your main code here, to run repeatedly

  CanMainStateMachine_run();

}
