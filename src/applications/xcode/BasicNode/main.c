

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
#include "src/openlcb/openlcb_application_broadcast_time.h"

#define NODE_ID 0x050701010033

openlcb_node_t *node = NULL;

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &OSxCanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &OSxCanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &OSxDrivers_lock_shared_resources,
    .unlock_shared_resources = &OSxDrivers_unlock_shared_resources,
    .on_rx                   = &Callbacks_on_can_rx_callback,
    .on_tx                   = &Callbacks_on_can_tx_callback,
    .on_alias_change         = &Callbacks_alias_change_callback,
};

static const openlcb_config_t openlcb_config = {
    // Required hardware
    .lock_shared_resources   = &OSxDrivers_lock_shared_resources,
    .unlock_shared_resources = &OSxDrivers_unlock_shared_resources,
    .config_mem_read         = &OSxDrivers_config_mem_read,
    .config_mem_write        = &OSxDrivers_config_mem_write,
    .reboot                  = &OSxDrivers_reboot,

    // Optional hardware extensions
    .freeze                  = &OSxDrivers_freeze,
    .unfreeze                = &OSxDrivers_unfreeze,
    .firmware_write          = &OSxDrivers_write_firemware,
    .factory_reset           = &Callbacks_operations_request_factory_reset,

    // Core application callbacks
    .on_100ms_timer          = &Callbacks_on_100ms_timer_callback,
    .on_login_complete       = &Callbacks_on_login_complete,

    // Event transport callbacks
    .on_consumed_event_identified = &Callbacks_on_consumed_event_identified,
    .on_consumed_event_pcer       = &Callbacks_on_consumed_event_pcer,
    .on_event_learn               = &Callbacks_on_event_learn,
    
    // Broadcast time callbacks
    .on_broadcast_time_changed    = &Callbacks_on_broadcast_time_changed,
};


void *thread_function_char_read(void *arg) {
    
    char c;
    
    while (1) {
        
        c = getchar();
        
        printf("Character received: %c\n", c);
        
        switch (c) {
                
            case '1':
                printf("Send Query\n");
                OpenLcbApplicationBroadcastTime_send_query(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
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

  CanConfig_initialize(&can_config);
  OpenLcb_initialize(&openlcb_config, OPENLCB_PROFILE_STANDARD | OPENLCB_FEATURE_BROADCAST_TIME |OPENLCB_FEATURE_FIRMWARE_UPGRADE);

  Callbacks_initialize();

  OSxDrivers_setup();
  OSxCanDriver_setup();

  printf("Waiting for CAN and 100ms Timer Drivers to connect\n");

  while (!(OSxDrivers_100ms_is_connected() && OSxCanDriver_is_connected() && OSxDrivers_input_is_connected()))
  {
    printf("Waiting for Threads\n");
    sleep(2);
  }

  node = OpenLcb_create_node(NODE_ID, &NodeParameters_main_node);
  printf("Node Allocated.....\n");

  OpenLcbApplicationBroadcastTime_setup_consumer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    
  pthread_t thread4;
  int thread_num4 = 4;
  pthread_create(&thread4, NULL, thread_function_char_read, &thread_num4);

  while (1)
  {

    usleep(2);
    OpenLcb_run();
      
  }
    
}
