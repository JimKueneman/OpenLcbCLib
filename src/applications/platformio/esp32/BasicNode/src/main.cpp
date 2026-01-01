
#include <Arduino.h>

#include "stdio.h"
#include "unistd.h"

#include "node_parameters.h"
#include "callbacks.h"

#include "src/application_drivers/esp32_can_drivers.h"
#include "src/application_drivers/esp32_drivers.h"

#include "src/drivers/canbus/can_main_statemachine.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/openlcb_login_statemachine.h"
#include "src/openlcb/openlcb_node.h"
#include "src/node_definition/dependency_injection.h"
#include "src/node_definition/dependency_injection_canbus.h"

#define NODE_ID 0x050101010788

void setup()
{

  Serial.begin(921600);
  Serial.println("Can Statemachine init.....");

  Esp32CanDriver_setup();
  Esp32Drivers_setup();

  DependencyInjectionCanBus_initialize();
  DependencyInjection_initialize();

  Callbacks_initialize();

  Serial.println("Creating Node.....");

  OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);
}

void loop()
{

  CanMainStateMachine_run();
  OpenLcbLoginMainStatemachine_run();
  OpenLcbMainStatemachine_run();
}
