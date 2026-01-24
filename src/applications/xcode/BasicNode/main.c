

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
#include "src/openlcb/openlcb_application.h"

#include "function_injection_defines.h"

#define NODE_ID 0x050701010033

int main(int argc, char *argv[])
{

  printf("Initializing...\n");
    
  DependencyInjectionCanBus_initialize();
  DependencyInjection_initialize();

  Callbacks_initialize();

  OSxDrivers_setup();
  OSxCanDriver_setup();

  printf("Waiting for CAN and 100ms Timer Drivers to connect\n");

  while (!(OSxDrivers_100ms_is_connected() && OSxCanDriver_is_connected() && OSxDrivers_input_is_connected()))
  {
    printf("Waiting for Threads\n");
    sleep(2);
  }

  openlcb_node_t *node = OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);
  printf("Node Allocated.....\n");
    
    OpenLcbApplication_clear_consumer_ranges(node);
    OpenLcbApplication_clear_producer_ranges(node);
    OpenLcbApplication_register_consumer_range(node,  0x0102030405060000, EVENT_RANGE_COUNT_64);
    OpenLcbApplication_register_producer_range(node,  0x0605040302010000, EVENT_RANGE_COUNT_128);

  while (1)
  {

    usleep(2);

    CanMainStateMachine_run();
    OpenLcbLoginMainStatemachine_run();
    OpenLcbMainStatemachine_run();
  }
}
