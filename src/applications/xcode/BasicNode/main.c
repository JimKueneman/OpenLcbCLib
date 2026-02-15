

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
#include "src/openlcb/openlcb_application_broadcast_time.h"

#include "function_injection_defines.h"

#define NODE_ID 0x050701010033

openlcb_node_t *node = NULL;


extern bool OpenLcbApplicationBroadcastTime_send_query(openlcb_node_t *openlcb_node);

// Controller send functions (any node can set a clock generator)

extern bool OpenLcbApplicationBroadcastTime_send_set_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute);
extern bool OpenLcbApplicationBroadcastTime_send_set_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day);
extern bool OpenLcbApplicationBroadcastTime_send_set_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year);
extern bool OpenLcbApplicationBroadcastTime_send_set_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate);
extern bool OpenLcbApplicationBroadcastTime_send_command_start(openlcb_node_t *openlcb_node, event_id_t clock_id);
extern bool OpenLcbApplicationBroadcastTime_send_command_stop(openlcb_node_t *openlcb_node, event_id_t clock_id);

void *thread_function_char_read(void *arg) {
    
    char c;
    
    while (1) {
        
        c = getchar();
        
        printf("Character received: %c\n", c);
        
        switch (c) {
                
            case '1':
                printf("Send Query\n");
                OpenLcbApplicationBroadcastTime_send_query(node);
            break;
                
            case '2':
                printf("Send Set Time/Date/Year\n");
                OpenLcbApplicationBroadcastTime_send_set_time(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 33);
                OpenLcbApplicationBroadcastTime_send_set_date(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 3, 7);
                OpenLcbApplicationBroadcastTime_send_set_year(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2008);
            break;
                
            case '3':
                printf("Send Start\n");
                OpenLcbApplicationBroadcastTime_send_command_start(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
            break;
                
            case '4':
                printf("Send Stop\n");
                OpenLcbApplicationBroadcastTime_send_command_stop(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
            break;
                
            case '5':
                printf("Send Rate\n");
                OpenLcbApplicationBroadcastTime_send_set_rate(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 4);
            break;
                
            case '6':
                
            break;
                
            case '7':
                
            break;
                
        }
        
    }
}


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

  node = OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);
  printf("Node Allocated.....\n");

    
  OpenLcbApplicationBroadcastTime_setup_consumer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    
  pthread_t thread4;
  int thread_num4 = 4;
  pthread_create(&thread4, NULL, thread_function_char_read, &thread_num4);

  while (1)
  {

    usleep(2);

    CanMainStateMachine_run();
    OpenLcbLoginMainStatemachine_run();
    OpenLcbMainStatemachine_run();
  }
}
