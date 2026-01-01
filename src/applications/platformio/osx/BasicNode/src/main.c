

#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include "strings.h"
#include "pthread.h"

#include "callbacks.h"
#include "node_parameters.h"
#include "src/application_drivers/osx_drivers.h"
#include "src/application_drivers/osx_can_drivers.h"
#include "src/node_definition/dependency_injection.h"
#include "src/node_definition/dependency_injection_canbus.h"

#include "src/drivers/canbus/can_main_statemachine.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/openlcb_login_statemachine.h"
#include "src/openlcb/openlcb_node.h"

uint64_t node_id_base = 0x0507010100BB;

int main(int argc, char *argv[])
{

  printf("Initializing...\n");


  OSxDrivers_setup();
  OSxCanDriver_setup();

  DependencyInjectionCanBus_initialize();
  DependencyInjection_initialize();

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

  openlcb_node_t *node = OpenLcbNode_allocate(nodeid, &NodeParameters_main_node);
  printf("Allocated.....\n");

  while (1)
  {

    CanMainStateMachine_run();
    OpenLcbLoginMainStatemachine_run();
    OpenLcbMainStatemachine_run();
  }
}
