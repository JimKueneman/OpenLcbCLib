
#include <Arduino.h>

#include "stdio.h"
#include "unistd.h"

#include "esp32_drivers.h"
#include "esp32_wifi_gridconnect_drivers.h"

#include "wifi_tools.h"
#include "wifi_tools_debug.h"
#include "node_parameters.h"
#include "dependency_injectors.h"
#include "dependency_injection.h"


#include "src/drivers/common/alias_mappings.h"
#include "src/drivers/common/can_main_statemachine.h"

#include "src/openlcb/openlcb_node.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/openlcb_login_statemachine.h"


// put function declarations here:

#define LED_BUILTIN 2
#define TEST_PIN 15
#define NODE_ID 0x0501010107DD

#define SSID "sonoita01"
#define PASSWORD "KylieKaelyn"
#define SERVER_IP "10.255.255.10"
#define SERVER_PORT 12021
#define SERVER_CONNECT_RETRY_TIME_MICROSECONDS 5000000


void setup()
{

    Serial.begin(921600);

    DependencyInjectors_initialize();
    DependencyInjection_initialize();

    Serial.println("Setting up Drivers.....");
    Esp32Drivers_setup();
    
    Serial.println("Creating Node.....");
    OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);

    Serial.println("Logging into Network..");
    WiFiTools_log_events(true);
    WiFiTools_connect_to_access_point(SSID, PASSWORD);

}

void loop()
{

    int socket = -1;

    if (WiFiTools_is_connected_to_access_point())
    {

        if (WiFiTools_is_connected_to_server()) {

           CanMainStateMachine_run();
           OpenLcbLoginMainStatemachine_run();
           OpenLcbMainStatemachine_run();

        } else {

            delayMicroseconds(SERVER_CONNECT_RETRY_TIME_MICROSECONDS);

            printf("Connecting to Server.....");
            socket = WiFiTools_connect_to_server(SERVER_IP, SERVER_PORT);

            if (socket > 0) {

                printf("Success connecting to Server, Socket Handle: %d\n", socket);
                AliasMappings_flush();
                OpenLcbNode_reset_state();
                Esp32WiFiGridconnectDriver_start(&socket);
            }

        }

    }

   // WiFiToolsDebug_log_status();
}
