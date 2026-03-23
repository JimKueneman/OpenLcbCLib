

#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include "strings.h"
#include "pthread.h"

#include "application_callbacks/callbacks_can.h"
#include "application_callbacks/callbacks_olcb.h"
#include "application_callbacks/callbacks_config_mem.h"
#include "application_callbacks/callbacks_events.h"
#include "application_callbacks/callbacks_broadcast_time.h"
#include "application_callbacks/callbacks_train.h"

#include "openlcb_user_config.h"
#include "application_drivers/osx_drivers.h"
#include "application_drivers/osx_can_drivers.h"

#include "openlcb_c_lib/drivers/canbus/can_config.h"
#include "openlcb_c_lib/openlcb/openlcb_config.h"
#include "openlcb_c_lib/openlcb/openlcb_application_broadcast_time.h"

#define NODE_ID 0x050701010033

openlcb_node_t *node = NULL;

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &OSxCanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &OSxCanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &OSxDrivers_lock_shared_resources,
    .unlock_shared_resources = &OSxDrivers_unlock_shared_resources,
    .on_rx                   = &CallbacksCan_on_rx,
    .on_tx                   = &CallbacksCan_on_tx,
    .on_alias_change         = &CallbacksCan_on_alias_change,
};

static const openlcb_config_t openlcb_config = {
    // Required hardware
    .lock_shared_resources   = &OSxDrivers_lock_shared_resources,
    .unlock_shared_resources = &OSxDrivers_unlock_shared_resources,
    .config_mem_read         = &OSxDrivers_config_mem_read,
    .config_mem_write        = &OSxDrivers_config_mem_write,
    .reboot                  = &OSxDrivers_reboot,

    // Optional hardware extensions
    .factory_reset           = &CallbacksConfigMem_factory_reset,
    .freeze                  = &OSxDrivers_freeze,
    .unfreeze                = &OSxDrivers_unfreeze,
    .firmware_write          = &OSxDrivers_firmware_write,

    // Core application callbacks
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,
    .on_login_complete       = &CallbacksOlcb_on_login_complete,

    // Event transport callbacks
    .on_consumed_event_identified = &CallbacksEvents_on_consumed_event_identified,
    .on_consumed_event_pcer       = &CallbacksEvents_on_consumed_event_pcer,
    .on_event_learn               = &CallbacksEvents_on_event_learn,
    
    // Broadcast time callbacks
    .on_broadcast_time_changed    = &CallbacksBroadcastTime_on_time_changed,

    // Train callbacks
    .on_train_controller_assigned = &CallbacksTrain_on_controller_assigned,
    .on_train_controller_released = &CallbacksTrain_on_controller_released,
};


void *thread_function_char_read(void *arg) {
    
    char c;
    
    while (1) {

        int ch = getchar();
        if (ch == EOF)
            break;
        c = (char)ch;

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

    return NULL;
}

int main(int argc, char *argv[])
{

  printf("Initializing...\n");

  CanConfig_initialize(&can_config);
  OpenLcb_initialize(&openlcb_config);

  CallbacksOlcb_initialize();

  OSxDrivers_initialize();
  OSxCanDriver_initialize();

  printf("Waiting for CAN and 100ms Timer Drivers to connect\n");

  while (!(OSxDrivers_100ms_is_connected() && OSxCanDriver_is_connected() && OSxDrivers_input_is_connected()))
  {
    printf("Waiting for Threads\n");
    sleep(2);
  }

  node = OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);
  printf("Node Allocated.....\n");

  OpenLcbApplicationBroadcastTime_setup_consumer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    
  pthread_t thread4;
  int thread_num4 = 4;
  pthread_create(&thread4, NULL, thread_function_char_read, &thread_num4);

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
