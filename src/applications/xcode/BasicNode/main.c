

#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include "strings.h"
#include "pthread.h"

#include "node_parameters.h"
#include "osx_can_drivers.h"
#include "osx_drivers.h"
#include "dependency_injection.h"
#include "dependency_injectors.h"

#include "src/drivers/canbus/can_main_statemachine.h"

#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/openlcb_login_statemachine.h"
#include "src/openlcb/openlcb_node.h"

#define NODE_ID 0x050701010033

int main(int argc, char *argv[])
{

  printf("Initializing...\n");

  DependencyInjection_initialize();
  DependencyInjectors_initialize();

  OSxDrivers_setup();
  OSxCanDriver_setup();

  printf("Waiting for CAN and 100ms Timer Drivers to connect\n");

  while (!(OSxDrivers_100ms_is_connected() && OSxCanDriver_is_connected() && OSxDrivers_input_is_connected()))
  {
    printf("Waiting for Threads\n");
    sleep(2);
  }

  OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);
  printf("Node Allocated.....\n");

  while (1)
  {

    usleep(2);

    CanMainStateMachine_run();
    OpenLcbLoginMainStatemachine_run();
    OpenLcbMainStatemachine_run();
  }
}
