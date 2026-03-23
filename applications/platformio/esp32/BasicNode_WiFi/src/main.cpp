
#include <Arduino.h>

#include "stdio.h"
#include "unistd.h"

#include "application_callbacks/callbacks_can.h"
#include "application_callbacks/callbacks_olcb.h"
#include "application_callbacks/callbacks_config_mem.h"
#include "openlcb_user_config.h"
#include "application_drivers/esp32_drivers.h"
#include "application_drivers/esp32_wifi_gridconnect_drivers.h"
#include "application_drivers/wifi_tools.h"
#include "application_drivers/wifi_tools_debug.h"

#include "openlcb_c_lib/drivers/canbus/can_config.h"
#include "openlcb_c_lib/openlcb/openlcb_config.h"

#include "openlcb_c_lib/drivers/canbus/alias_mappings.h"
#include "openlcb_c_lib/openlcb/openlcb_node.h"

// put function declarations here:

#define LED_BUILTIN 2
#define TEST_PIN 15
#define NODE_ID 0x0501010107DD

#define SSID "sonoita01"
#define PASSWORD "KylieKaelyn"
#define SERVER_IP "10.255.255.10"
#define SERVER_PORT 12021
#define SERVER_CONNECT_RETRY_TIME_MICROSECONDS 5000000

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &Esp32WiFiGridconnectDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &Esp32WiFiGridconnectDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &Esp32Drivers_lock_shared_resources,
    .unlock_shared_resources = &Esp32Drivers_unlock_shared_resources,
    .on_rx                   = &CallbacksCan_on_rx,
    .on_tx                   = &CallbacksCan_on_tx,
    .on_alias_change         = &CallbacksCan_on_alias_change,
};

static const openlcb_config_t openlcb_config = {
    .lock_shared_resources   = &Esp32Drivers_lock_shared_resources,
    .unlock_shared_resources = &Esp32Drivers_unlock_shared_resources,
    .config_mem_read         = &Esp32Drivers_config_mem_read,
    .config_mem_write        = &Esp32Drivers_config_mem_write,
    .reboot                  = &Esp32Drivers_reboot,
    .factory_reset           = &CallbacksConfigMem_factory_reset,
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,
};

void setup()
{

    Serial.begin(921600);

    Serial.println("Setting up Drivers.....");
    Esp32WiFiGridconnectDriver_setup();
    Esp32Drivers_initialize();

    CanConfig_initialize(&can_config);
    OpenLcb_initialize(&openlcb_config);

    CallbacksOlcb_initialize();

    Serial.println("Creating Node.....");
    OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);

    Serial.println("Logging into Network..");
    WiFiTools_log_events(true);
    WiFiTools_connect_to_access_point(SSID, PASSWORD);
}

void loop()
{

    int socket = -1;

    if (WiFiTools_is_connected_to_access_point())
    {

        if (WiFiTools_is_connected_to_server())
        {

            OpenLcb_run();
        } else {

            delayMicroseconds(SERVER_CONNECT_RETRY_TIME_MICROSECONDS);

            printf("Connecting to Server.....");
            socket = WiFiTools_connect_to_server(SERVER_IP, SERVER_PORT);

            if (socket > 0)
            {

                printf("Success connecting to Server, Socket Handle: %d\n", socket);
                AliasMappings_flush();
                OpenLcbNode_reset_state();
                Esp32WiFiGridconnectDriver_start(&socket);
            }
        }
    }
}
