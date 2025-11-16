
#include <Arduino.h>

#include "stdio.h"
#include "unistd.h"

#include "esp32_drivers.h"
#include "esp32_wifi_gridconnect_drivers.h"
#include "dependency_injection.h"
#include "wifi_tools.h"
#include "wifi_tools_debug.h"

#include "node_parameters.h"

#include "src/drivers/alias_mappings.h"

#include "src/drivers/common/can_types.h"
#include "src/drivers/common/can_utilities.h"
#include "src/drivers/common/can_buffer_store.h"
#include "src/drivers/common/can_buffer_fifo.h"
#include "src/drivers/common/can_login_message_handler.h"
#include "src/drivers/common/can_login_statemachine.h"
#include "src/drivers/common/can_rx_message_handler.h"
#include "src/drivers/common/can_rx_statemachine.h"
#include "src/drivers/common/can_tx_message_handler.h"
#include "src/drivers/common/can_tx_statemachine.h"
#include "src/drivers/common/can_main_statemachine.h"

#include "src/openlcb/openlcb_application.h"
#include "src/openlcb/openlcb_defines.h"
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/openlcb_utilities.h"
#include "src/openlcb/openlcb_buffer_store.h"
#include "src/openlcb/openlcb_buffer_list.h"
#include "src/openlcb/openlcb_buffer_fifo.h"
#include "src/openlcb/openlcb_node.h"
#include "src/openlcb/protocol_message_network.h"
#include "src/openlcb/protocol_event_transport.h"
#include "src/openlcb/protocol_snip.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/protocol_datagram_handler.h"
#include "src/openlcb/openlcb_login_statemachine.h"
#include "src/openlcb/openlcb_login_statemachine_handler.h"
#include "src/openlcb/protocol_config_mem_read_handler.h"
#include "src/openlcb/protocol_config_mem_write_handler.h"
#include "src/openlcb/protocol_config_mem_operations_handler.h"

// put function declarations here:

#define LED_BUILTIN 2
#define TEST_PIN 15
#define NODE_ID 0x0501010107DD

#define SSID "sonoita01"
#define PASSWORD "KylieKaelyn"
#define SERVER_IP "10.255.255.16"
#define SERVER_PORT 12021
#define SERVER_CONNECT_RETRY_TIME_MICROSECONDS 5000000

void _on_100ms_timer_callback(void)
{
}

static void _on_can_rx_callback(can_msg_t *can_msg)
{
}

static void _on_can_tx_callback(can_msg_t *can_msg)
{
}

static void _alias_change_callback(uint16_t new_alias, node_id_t node_id)
{

    printf("Alias Allocation: 0x%02X  ", new_alias);
    printf("NodeID: 0x%06llX\n\n", node_id);
}

void setup()
{
    pinMode(TEST_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(230400);

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
                Esp32WiFiGridconnectDriver_start(&socket);
                
            }

        }

    }

   // WiFiToolsDebug_log_status();
}
