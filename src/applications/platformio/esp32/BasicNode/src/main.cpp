
#include <Arduino.h>

#include "stdio.h"
#include "unistd.h"

#include "esp32_drivers.h"
#include "esp32_can_drivers.h"

#include "node_parameters.h"
#include "dependency_injection.h"
#include "dependency_injectors.h"

#include "src/drivers/canbus/can_main_statemachine.h"

#include "src/openlcb/openlcb_node.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/openlcb_login_statemachine.h"

#define NODE_ID 0x050101010788

void setup()
{

  Serial.begin(921600);

  Serial.println("Can Statemachine init.....");
  Esp32CanDriver_setup();
  Esp32Drivers_setup();

  DependencyInjectors_initialize();
  DependencyInjection_initialize();

  Serial.println("Creating Node.....");

  OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);
}

void loop()
{

  CanMainStateMachine_run();
  OpenLcbLoginMainStatemachine_run();
  OpenLcbMainStatemachine_run();
}
