/** \copyright
 * Copyright (c) 2025, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \file main.c
 *
 *  main
 *
 * @author Jim Kueneman
 * @date 4 April 2025
 */

// DSPIC33EP512GP504 Configuration Bit Settings

// 'C' source line config statements

// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

// FPOR
#pragma config ALTI2C1 = OFF            // Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins)
#pragma config ALTI2C2 = OFF            // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
#pragma config WDTWIN = WIN25           // Watchdog Window Select bits (WDT Window is 25% of WDT period)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON              // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FOSC
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = OFF            // Peripheral pin select configuration (Allow multiple reconfigurations)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = PRIPLL           // Oscillator Source Selection (Primary Oscillator with PLL module (XT + PLL, HS + PLL, EC + PLL))
#pragma config PWMLOCK = OFF            // PWM Lock Enable bit (PWM registers may be written without key sequence)
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)


// Output so write to the latch
#define _25AAxxx_CS _LATB4
#define _25AAxxx_CS_TRIS _TRISB4

#define TEST_PIN_1401_TRIS _TRISA11
#define TEST_PIN_1401 _RA11

#define TEST_PIN_1402_TRIS _TRISB14
#define TEST_PIN_1402 _LATB14

#define TEST_PIN_1403_TRIS _TRISG9
#define TEST_PIN_1403 _LATG9

#define TEST_PIN_1404_TRIS _TRISA12
#define TEST_PIN_1404 _LATA12

#define LED_BLUE_TRIS _TRISA7
#define LED_BLUE _LATA7

#define LED_YELLOW_TRIS _TRISC5
#define LED_YELLOW _LATC5

#define LED_GREEN_TRIS _TRISA0
#define LED_GREEN _LATA0

#include <libpic30.h>

#include "xc.h"
#include "stdio.h"  // printf
#include "string.h"
#include "stdlib.h"


#include "../../../drivers/alias_mappings.h"

#include "../../../drivers/common/can_types.h"
#include "../../../drivers/common/can_utilities.h"
#include "../../../drivers/common/can_buffer_store.h"
#include "../../../drivers/common/can_buffer_fifo.h"
#include "../../../drivers/common/can_login_message_handler.h"
#include "../../../drivers/common/can_login_statemachine.h"
#include "../../../drivers/common/can_rx_message_handler.h"
#include "../../../drivers/common/can_rx_statemachine.h"
#include "../../../drivers/common/can_tx_message_handler.h"
#include "../../../drivers/common/can_tx_statemachine.h"
#include "../../../drivers/common/can_main_statemachine.h"

#include "../../../openlcb/openlcb_defines.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_buffer_store.h"
#include "../../../openlcb/openlcb_buffer_list.h"
#include "../../../openlcb/openlcb_buffer_fifo.h"
#include "../../../openlcb/openlcb_node.h"
#include "../../../openlcb/protocol_message_network.h"
#include "../../../openlcb/protocol_event_transport.h"
#include "../../../openlcb/protocol_snip.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/protocol_datagram_handler.h"
#include "../../../openlcb/openlcb_login_statemachine.h"
#include "../../../openlcb/openlcb_login_statemachine_handler.h"
#include "../../../openlcb/protocol_config_mem_read_handler.h"
#include "../../../openlcb/protocol_config_mem_write_handler.h"
#include "../../../openlcb/protocol_config_mem_operations_handler.h"

#include "../../../openlcb/openlcb_application.h"

#include "node_parameters.h"
#include "drivers.h"
#include "../dsPIC_Common/ecan1_helper.h"


#define NODE_ID 0x0501010107AA

//static uint64_t node_id_base = NODE_ID;

static uint16_t count = 0;

void _initialize_io_early_for_test(void) {


    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELE = 0x00;


    LED_BLUE_TRIS = 0;
    LED_GREEN_TRIS = 0;
    LED_YELLOW_TRIS = 0;

    TEST_PIN_1401_TRIS = 0;
    TEST_PIN_1402_TRIS = 0;
    TEST_PIN_1403_TRIS = 0;
    TEST_PIN_1404_TRIS = 0;

}

void _on_100ms_timer_callback(void) {

    // Calls back every 100ms... don't do anything crazy here as it is in the context of the interrupt

    count++;

    if (count > 10) {

        count = 0;

        LED_BLUE = 0;
        LED_YELLOW = 0;
    }

}

void _on_can_rx_callback(can_msg_t* can_msg) {

    LED_BLUE = 1;

}

void _on_can_tx_callback(can_msg_t* can_msg) {

    LED_YELLOW = 1;

}

void _alias_change_callback(uint16_t new_alias, node_id_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    printf("NodeID: 0x%06llX\n\n", node_id);

}

static void _operations_request_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    printf("Factory Reset: NodeID = 0x%06llX\n", OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0));

}

static void _operations_request_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    printf("\n\n\nFactory Reboot............\n\n\n");

    BasicNodeDrivers_reboot();

}

void _on_event_with_payload(openlcb_node_t* node, event_id_t* event_id, uint16_t count, event_payload_t* payload) {

}

#define TRANSMIT_CAN_FRAME_FUNC &Ecan1Helper_transmit_can_frame
#define IS_TX_BUFFER_EMPTY_FUNC &Ecan1Helper_is_can_tx_buffer_clear
#define LOCK_SHARED_RESOURCES_FUNC &Ecan1Helper_pause_can_rx
#define UNLOCK_SHARED_RESOURCES_FUNC &Ecan1Helper_resume_can_rx
#define CONFIG_MEM_READ_FUNC &BasicNodeDrivers_config_mem_read
#define CONFIG_MEM_WRITE_FUNC &BasicNodeDrivers_config_mem_write
#define OPERATIONS_REBOOT_FUNC &_operations_request_reboot
#define OPERATIONS_FACTORY_RESET_FUNC &_operations_request_factory_reset

const interface_can_login_message_handler_t interface_can_login_message_handler = {

    .alias_mapping_register = &AliasMappings_register,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    // Callback events
    .on_alias_change = &_alias_change_callback

};

const interface_can_login_state_machine_t interface_can_login_state_machine = {

    .state_init = &CanLoginMessageHandler_state_init,
    .state_generate_seed = &CanLoginMessageHandler_state_generate_seed,
    .state_generate_alias = &CanLoginMessageHandler_state_generate_alias,
    .state_load_cid07 = &CanLoginMessageHandler_state_load_cid07,
    .state_load_cid06 = &CanLoginMessageHandler_state_load_cid06,
    .state_load_cid05 = &CanLoginMessageHandler_state_load_cid05,
    .state_load_cid04 = &CanLoginMessageHandler_state_load_cid04,
    .state_wait_200ms = &CanLoginMessageHandler_state_wait_200ms,
    .state_load_rid = &CanLoginMessageHandler_state_load_rid,
    .state_load_amd = &CanLoginMessageHandler_state_load_amd

};

const interface_can_rx_message_handler_t interface_can_rx_message_handler = {

    .openlcb_buffer_store_allocate_buffer = &OpenLcbBufferStore_allocate_buffer,
    .can_buffer_store_allocate_buffer = &CanBufferStore_allocate_buffer,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    .alias_mapping_find_mapping_by_node_id = &AliasMappings_find_mapping_by_node_id,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_set_has_duplicate_alias_flag = &AliasMappings_set_has_duplicate_alias_flag

};

const interface_can_rx_statemachine_t interface_can_rx_statemachine = {

    .handle_can_legacy_snip = &CanRxMessageHandler_can_legacy_snip,
    .handle_single_frame = &CanRxMessageHandler_single_frame,
    .handle_first_frame = &CanRxMessageHandler_first_frame,
    .handle_middle_frame = &CanRxMessageHandler_middle_frame,
    .handle_last_frame = &CanRxMessageHandler_last_frame,
    .handle_stream_frame = &CanRxMessageHandler_stream_frame,
    .handle_rid_frame = CanRxMessageHandler_rid_frame,
    .handle_amd_frame = CanRxMessageHandler_amd_frame,
    .handle_ame_frame = CanRxMessageHandler_ame_frame,
    .handle_amr_frame = CanRxMessageHandler_amr_frame,
    .handle_error_info_report_frame = CanRxMessageHandler_error_info_report_frame,
    .handle_cid_frame = CanRxMessageHandler_cid_frame,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    // Callback events
    .on_receive = &_on_can_rx_callback

};

const interface_can_tx_message_handler_t interface_can_tx_message_handler = {

    .transmit_can_frame = TRANSMIT_CAN_FRAME_FUNC, //  HARDWARE INTERFACE
    // Callback events
    .on_transmit = &_on_can_tx_callback

};

const interface_can_tx_statemachine_t interface_can_tx_statemachine = {

    .is_tx_buffer_empty = IS_TX_BUFFER_EMPTY_FUNC, //  HARDWARE INTERFACE
    .handle_addressed_msg_frame = &CanTxMessageHandler_addressed_msg_frame,
    .handle_unaddressed_msg_frame = &CanTxMessageHandler_unaddressed_msg_frame,
    .handle_datagram_frame = &CanTxMessageHandler_datagram_frame,
    .handle_stream_frame = &CanTxMessageHandler_stream_frame,
    .handle_can_frame = &CanTxMessageHandler_can_frame

};

const interface_can_main_statemachine_t interface_can_main_statemachine = {

    .lock_shared_resources = LOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE
    .unlock_shared_resources = UNLOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE
    .send_can_message = &CanTxStatemachine_send_can_message,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .openlcb_node_find_by_alias = &OpenLcbNode_find_by_alias,
    .login_statemachine_run = &CanLoginStateMachine_run,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_unregister = &AliasMappings_unregister,

    .handle_duplicate_aliases = &CanMainStatemachine_handle_duplicate_aliases,
    .handle_outgoing_can_message = &CanMainStatemachine_handle_outgoing_can_message,
    .handle_login_outgoing_can_message = &CanMainStatemachine_handle_login_outgoing_can_message,
    .handle_try_enumerate_first_node = &CanMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &CanMainStatemachine_handle_try_enumerate_next_node

};

const interface_openlcb_node_t interface_openlcb_node = {

    // Callback events
    .on_100ms_timer_tick = &_on_100ms_timer_callback

};

const interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network = {

};

const interface_openlcb_protocol_event_transport_t interface_openlcb_protocol_event_transport = {

    // Callback events
    .on_consumer_range_identified = NULL,
    .on_consumer_identified_unknown = NULL,
    .on_consumer_identified_set = NULL,
    .on_consumer_identified_clear = NULL,
    .on_consumer_identified_reserved = NULL,
    .on_producer_range_identified = NULL,
    .on_producer_identified_unknown = NULL,
    .on_producer_identified_set = NULL,
    .on_producer_identified_clear = NULL,
    .on_producer_identified_reserved = NULL,
    .on_event_learn = NULL,
    .on_pc_event_report = NULL,
    .on_pc_event_report_with_payload = &_on_event_with_payload

};

const interface_openlcb_login_message_handler_t interface_openlcb_login_message_handler = {

    .extract_consumer_event_state_mti = &ProtocolEventTransport_extract_consumer_event_status_mti,
    .extract_producer_event_state_mti = &ProtocolEventTransport_extract_producer_event_status_mti

};

const interface_openlcb_login_state_machine_t interface_openlcb_login_state_machine = {

    .load_initialization_complete = &OpenLcbLoginMessageHandler_load_initialization_complete,
    .load_producer_events = &OpenLcbLoginMessageHandler_load_producer_event,
    .load_consumer_events = &OpenLcbLoginMessageHandler_load_consumer_event,

    .send_openlcb_msg = &CanTxStatemachine_send_openlcb_message,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,

    .process_login_statemachine = &OpenLcbLoginStateMachine_process,
    .handle_outgoing_openlcb_message = &OpenLcbLoginStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &OpenLcbLoginStatemachine_handle_try_reenumerate,
    .handle_try_enumerate_first_node = &OpenLcbLoginStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &OpenLcbLoginStatemachine_handle_try_enumerate_next_node
};

const interface_openlcb_main_statemachine_t interface_openlcb_main_statemachine = {

    // MESSAGE NETWORK
    .message_network_initialization_complete = ProtocolMessageNetwork_handle_initialization_complete,
    .message_network_initialization_complete_simple = ProtocolMessageNetwork_handle_initialization_complete_simple,
    .message_network_verify_node_id_addressed = &ProtocolMessageNetwork_handle_verify_node_id_addressed,
    .message_network_verify_node_id_global = &ProtocolMessageNetwork_handle_verify_node_id_global,
    .message_network_verified_node_id = &ProtocolMessageNetwork_handle_verified_node_id,
    .message_network_optional_interaction_rejected = &ProtocolMessageNetwork_handle_optional_interaction_rejected,
    .message_network_terminate_due_to_error = &ProtocolMessageNetwork_handle_terminate_due_to_error,

    // PROTOCOL SUPPORT
    .message_network_protocol_support_inquiry = &ProtocolMessageNetwork_handle_protocol_support_inquiry,
    .message_network_protocol_support_reply = &ProtocolMessageNetwork_handle_protocol_support_reply,

    // SNIP
    .snip_simple_node_info_request = &ProtocolSnip_handle_simple_node_info_request,
    .snip_simple_node_info_reply = &ProtocolSnip_handle_simple_node_info_reply,

    // EVENTS
    .event_transport_consumer_identify = &ProtocolEventTransport_handle_consumer_identify,
    .event_transport_consumer_range_identified = &ProtocolEventTransport_handle_consumer_range_identified,
    .event_transport_consumer_identified_unknown = &ProtocolEventTransport_handle_consumer_identified_unknown,
    .event_transport_consumer_identified_set = &ProtocolEventTransport_handle_consumer_identified_set,
    .event_transport_consumer_identified_clear = &ProtocolEventTransport_handle_consumer_identified_clear,
    .event_transport_consumer_identified_reserved = &ProtocolEventTransport_handle_consumer_identified_reserved,
    .event_transport_producer_identify = &ProtocolEventTransport_handle_producer_identify,
    .event_transport_producer_range_identified = &ProtocolEventTransport_handle_producer_range_identified,
    .event_transport_producer_identified_unknown = &ProtocolEventTransport_handle_producer_identified_unknown,
    .event_transport_producer_identified_set = &ProtocolEventTransport_handle_producer_identified_set,
    .event_transport_producer_identified_clear = &ProtocolEventTransport_handle_producer_identified_clear,
    .event_transport_producer_identified_reserved = &ProtocolEventTransport_handle_producer_identified_reserved,
    .event_transport_identify_dest = &ProtocolEventTransport_handle_events_identify_dest,
    .event_transport_identify = &ProtocolEventTransport_handle_events_identify,
    .event_transport_learn = &ProtocolEventTransport_handle_event_learn,
    .event_transport_pc_report = &ProtocolEventTransport_handle_pc_event_report,
    .event_transport_pc_report_with_payload = &ProtocolEventTransport_handle_pc_event_report_with_payload,

    // TRACTION
    .traction_control_command = NULL,
    .traction_control_reply = NULL,

    // TRACTION SNIP
    .simple_train_node_ident_info_request = NULL,
    .simple_train_node_ident_info_reply = NULL,

    // DATAGRAM
    .datagram = ProtocolDatagramHandler_datagram,
    .datagram_ok_reply = ProtocolDatagramHandler_datagram_received_ok,
    .datagram_rejected_reply = ProtocolDatagramHandler_datagram_rejected,

    // STREAM
    .stream_initiate_request = NULL,
    .stream_initiate_reply = NULL,
    .stream_send_data = NULL,
    .stream_data_proceed = NULL,
    .stream_data_complete = NULL,

    // required
    .lock_shared_resources = LOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE
    .unlock_shared_resources = UNLOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE
    .send_openlcb_msg = &CanTxStatemachine_send_openlcb_message,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .load_interaction_rejected = &OpenLcbMainStatemachine_load_interaction_rejected,

    .handle_outgoing_openlcb_message = &OpenLcbMainStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &OpenLcbMainStatemachine_handle_try_reenumerate,
    .handle_try_enumerate_first_node = &OpenLcbMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &OpenLcbMainStatemachine_handle_try_enumerate_next_node,
    .handle_try_pop_next_incoming_openlcb_message = &OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message,

    // for test injection, leave null to use the default functions
    .process_main_statemachine = OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = &OpenLcbMainStatemachine_does_node_process_msg,

};

const interface_openlcb_protocol_snip_t interface_openlcb_protocol_snip = {

    .configuration_memory_read = CONFIG_MEM_READ_FUNC,
    .configuration_memory_write = CONFIG_MEM_WRITE_FUNC

};

const interface_protocol_config_mem_read_handler_t interface_protocol_config_mem_read_handler = {

    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message,
    .config_memory_read = CONFIG_MEM_READ_FUNC,

    .snip_load_manufacturer_version_id = &ProtocolSnip_load_manufacturer_version_id,
    .snip_load_name = &ProtocolSnip_load_name,
    .snip_load_model = &ProtocolSnip_load_model,
    .snip_load_hardware_version = &ProtocolSnip_load_hardware_version,
    .snip_load_software_version = &ProtocolSnip_load_software_version,
    .snip_load_user_version_id = &ProtocolSnip_load_user_version_id,
    .snip_load_user_name = &ProtocolSnip_load_user_name,
    .snip_load_user_description = &ProtocolSnip_load_user_description,

    .read_request_config_definition_info = &ProtocolConfigMemReadHandler_read_request_config_definition_info,
    .read_request_all = NULL,
    .read_request_config_mem = &ProtocolConfigMemReadHandler_read_request_config_mem,
    .read_request_acdi_manufacturer = &ProtocolConfigMemReadHandler_read_request_acdi_manufacturer,
    .read_request_acdi_user = &ProtocolConfigMemReadHandler_read_request_acdi_user,
    .read_request_traction_function_config_definition_info = NULL,
    .read_request_traction_function_config_memory = NULL,

    .delayed_reply_time = NULL

};

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler = {

    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message,
    .config_memory_write = CONFIG_MEM_WRITE_FUNC,
    .snip_user_name_write = &ProtocolSnip_write_user_name,
    .snip_user_description_write = ProtocolSnip_write_user_description,

    .write_request_config_definition_info = NULL,
    .write_request_all = NULL,
    .write_request_config_mem = &ProtocolConfigMemWriteHandler_write_request_config_mem,
    .write_request_acdi_manufacturer = NULL,
    .write_request_acdi_user = &ProtocolConfigMemWriteHandler_write_request_acdi_user,
    .write_request_traction_function_config_definition_info = NULL,
    .write_request_traction_function_config_memory = NULL,

    .delayed_reply_time = NULL

};

const interface_protocol_config_mem_operations_handler_t interface_protocol_config_mem_operations_handler = {

    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message,

    .operations_request_options_cmd = &ProtocolConfigMemOperationsHandler_request_options_cmd,
    .operations_request_options_cmd_reply = NULL,
    .operations_request_get_address_space_info = ProtocolConfigMemOperationsHandler_request_get_address_space_info,
    .operations_request_get_address_space_info_reply_present = NULL,
    .operations_request_get_address_space_info_reply_not_present = NULL,
    .operations_request_reserve_lock = ProtocolConfigMemOperationsHandler_request_reserve_lock,
    .operations_request_reserve_lock_reply = NULL,
    .operations_request_get_unique_id = NULL,
    .operations_request_get_unique_id_reply = NULL,
    .operations_request_freeze = NULL,
    .operations_request_unfreeze = NULL,
    .operations_request_update_complete = NULL,
    .operations_request_reset_reboot = OPERATIONS_REBOOT_FUNC, // HARDWARE INTERFACE
    .operations_request_factory_reset = OPERATIONS_FACTORY_RESET_FUNC, // HARDWARE INTERFACE

};

const interface_openlcb_application_t interface_openlcb_application = {

    .transmit_openlcb_message = NULL,
    .configuration_memory_read = NULL,
    .configuration_memory_write = NULL

};

const interface_protocol_datagram_handler_t interface_protocol_datagram_handler = {

    // Config Memory Read 
    .memory_read_space_config_description_info = &ProtocolConfigMemReadHandler_read_space_config_description_info,
    .memory_read_space_all = &ProtocolConfigMemReadHandler_read_space_all,
    .memory_read_space_configuration_memory = &ProtocolConfigMemReadHandler_read_space_config_memory,
    .memory_read_space_acdi_manufacturer = &ProtocolConfigMemReadHandler_read_space_acdi_manufacturer,
    .memory_read_space_acdi_user = &ProtocolConfigMemReadHandler_read_space_acdi_user,
    .memory_read_space_traction_function_definition_info = &ProtocolConfigMemReadHandler_read_space_traction_function_definition_info,
    .memory_read_space_traction_function_config_memory = &ProtocolConfigMemReadHandler_read_space_traction_function_config_memory,

    // Config Memory Read Reply Ok 
    .memory_read_space_config_description_info_reply_ok = NULL,
    .memory_read_space_all_reply_ok = NULL,
    .memory_read_space_configuration_memory_reply_ok = NULL,
    .memory_read_space_acdi_manufacturer_reply_ok = NULL,
    .memory_read_space_acdi_user_reply_ok = NULL,
    .memory_read_space_traction_function_definition_info_reply_ok = NULL,
    .memory_read_space_traction_function_config_memory_reply_ok = NULL,

    // Config Memory Read Reply Failed
    .memory_read_space_config_description_info_reply_fail = NULL,
    .memory_read_space_all_reply_fail = NULL,
    .memory_read_space_configuration_memory_reply_fail = NULL,
    .memory_read_space_acdi_manufacturer_reply_fail = NULL,
    .memory_read_space_acdi_user_reply_fail = NULL,
    .memory_read_space_traction_function_definition_info_reply_fail = NULL,
    .memory_read_space_traction_function_config_memory_reply_fail = NULL,

    // Config Memory Stream Read
    .memory_read_stream_space_config_description_info = NULL,
    .memory_read_stream_space_all = NULL,
    .memory_read_stream_space_configuration_memory = NULL,
    .memory_read_stream_space_acdi_manufacturer = NULL,

    // Config Memory Stream Read Reply = Ok
    .memory_read_stream_space_config_description_info_reply_ok = NULL,
    .memory_read_stream_space_all_reply_ok = NULL,
    .memory_read_stream_space_configuration_memory_reply_ok = NULL,
    .memory_read_stream_space_acdi_manufacturer_reply_ok = NULL,
    .memory_read_stream_space_acdi_user_reply_ok = NULL,
    .memory_read_stream_space_traction_function_definition_info_reply_ok = NULL,
    .memory_read_stream_space_traction_function_config_memory_reply_ok = NULL,

    // Config Memory Stream Read Reply = Failed
    .memory_read_stream_space_config_description_info_reply_fail = NULL,
    .memory_read_stream_space_all_reply_fail = NULL,
    .memory_read_stream_space_configuration_memory_reply_fail = NULL,
    .memory_read_stream_space_acdi_manufacturer_reply_fail = NULL,
    .memory_read_stream_space_acdi_user_reply_fail = NULL,
    .memory_read_stream_space_traction_function_definition_info_reply_fail = NULL,
    .memory_read_stream_space_traction_function_config_memory_reply_fail = NULL,

    // Config Memory Write 
    .memory_write_space_config_description_info = NULL,
    .memory_write_space_all = NULL,
    .memory_write_space_configuration_memory = NULL,
    .memory_write_space_acdi_manufacturer = NULL,
    .memory_write_space_acdi_user = NULL,
    .memory_write_space_traction_function_definition_info = NULL,
    .memory_write_space_traction_function_config_memory = NULL,
    .memory_write_space_firmware_upgrade = NULL,

    // Config Memory Write Reply Ok 
    .memory_write_space_config_description_info_reply_ok = NULL,
    .memory_write_space_all_reply_ok = NULL,
    .memory_write_space_configuration_memory_reply_ok = NULL,
    .memory_write_space_acdi_manufacturer_reply_ok = NULL,
    .memory_write_space_acdi_user_reply_ok = NULL,
    .memory_write_space_traction_function_definition_info_reply_ok = NULL,
    .memory_write_space_traction_function_config_memory_reply_ok = NULL,

    // Config Memory Write Reply Fail
    .memory_write_space_config_description_info_reply_fail = NULL,
    .memory_write_space_all_reply_fail = NULL,
    .memory_write_space_configuration_memory_reply_fail = NULL,
    .memory_write_space_acdi_manufacturer_reply_fail = NULL,
    .memory_write_space_acdi_user_reply_fail = NULL,
    .memory_write_space_traction_function_definition_info_reply_fail = NULL,
    .memory_write_space_traction_function_config_memory_reply_fail = NULL,

    // Config Memory Write Under Mask
    .memory_write_under_mask_space_config_description_info = NULL,
    .memory_write_under_mask_space_all = NULL,
    .memory_write_under_mask_space_configuration_memory = NULL,
    .memory_write_under_mask_space_acdi_manufacturer = NULL,
    .memory_write_under_mask_space_acdi_user = NULL,
    .memory_write_under_mask_space_traction_function_definition_info = NULL,
    .memory_write_under_mask_space_traction_function_config_memory = NULL,
    .memory_write_under_mask_space_firmware_upgrade = NULL,

    // Config Memory Stream Write
    .memory_write_stream_space_config_description_info = NULL,
    .memory_write_stream_space_all = NULL,
    .memory_write_stream_space_configuration_memory = NULL,
    .memory_write_stream_space_acdi_manufacturer = NULL,
    .memory_write_stream_space_acdi_user = NULL,
    .memory_write_stream_space_traction_function_definition_info = NULL,
    .memory_write_stream_space_traction_function_config_memory = NULL,
    .memory_write_stream_space_firmware_upgrade = NULL,

    // Config Memory Stream Write Reply = Ok
    .memory_write_stream_space_config_description_info_reply_ok = NULL,
    .memory_write_stream_space_all_reply_ok = NULL,
    .memory_write_stream_space_configuration_memory_reply_ok = NULL,
    .memory_write_stream_space_acdi_manufacturer_reply_ok = NULL,
    .memory_write_stream_space_acdi_user_reply_ok = NULL,
    .memory_write_stream_space_traction_function_definition_info_reply_ok = NULL,
    .memory_write_stream_space_traction_function_config_memory_reply_ok = NULL,

    // Config Memory Stream Write Reply = Failed
    .memory_write_stream_space_config_description_info_reply_fail = NULL,
    .memory_write_stream_space_all_reply_fail = NULL,
    .memory_write_stream_space_configuration_memory_reply_fail = NULL,
    .memory_write_stream_space_acdi_manufacturer_reply_fail = NULL,
    .memory_write_stream_space_acdi_user_reply_fail = NULL,
    .memory_write_stream_space_traction_function_definition_info_reply_fail = NULL,
    .memory_write_stream_space_traction_function_config_memory_reply_fail = NULL,

    // Config Memory Commands
    .memory_options_cmd = &ProtocolConfigMemOperationsHandler_options_cmd,
    .memory_options_reply = &ProtocolConfigMemOperationsHandler_options_reply,
    .memory_get_address_space_info = &ProtocolConfigMemOperationsHandler_get_address_space_info,
    .memory_get_address_space_info_reply_not_present = &ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present,
    .memory_get_address_space_info_reply_present = &ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present,
    .memory_reserve_lock = &ProtocolConfigMemOperationsHandler_reserve_lock,
    .memory_reserve_lock_reply = &ProtocolConfigMemOperationsHandler_reserve_lock_reply,
    .memory_get_unique_id = ProtocolConfigMemOperationsHandler_get_unique_id,
    .memory_get_unique_id_reply = &ProtocolConfigMemOperationsHandler_get_unique_id_reply,
    .memory_unfreeze = &ProtocolConfigMemOperationsHandler_unfreeze,
    .memory_freeze = &ProtocolConfigMemOperationsHandler_freeze,
    .memory_update_complete = &ProtocolConfigMemOperationsHandler_update_complete,
    .memory_reset_reboot = &ProtocolConfigMemOperationsHandler_reset_reboot,
    .memory_factory_reset = &ProtocolConfigMemOperationsHandler_factory_reset,

    .lock_shared_resources = LOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE
    .unlock_shared_resources = UNLOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE

};

int main(void) {

    _initialize_io_early_for_test();

    Ecan1Helper_initialize();
    BasicNodeDrivers_initialize();

    printf("MCU Initialized\n");

    CanBufferStore_initialize();
    CanBufferFifo_initialize();

    CanRxMessageHandler_initialize(&interface_can_rx_message_handler);
    CanRxStatemachine_initialize(&interface_can_rx_statemachine);

    CanTxMessageHandler_initialize(&interface_can_tx_message_handler);
    CanTxStatemachine_initialize(&interface_can_tx_statemachine);

    CanLoginMessageHandler_initialize(&interface_can_login_message_handler);
    CanLoginStateMachine_initialize(&interface_can_login_state_machine);
    CanMainStatemachine_initialize(&interface_can_main_statemachine);

    AliasMappings_initialize();

    OpenLcbBufferStore_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbBufferFifo_initialize();

    ProtocolSnip_initialize(&interface_openlcb_protocol_snip);
    ProtocolDatagramHandler_initialize(&interface_protocol_datagram_handler);
    ProtocolEventTransport_initialize(&interface_openlcb_protocol_event_transport);
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network);
    ProtocolConfigMemReadHandler_initialize(&interface_protocol_config_mem_read_handler);
    ProtocolConfigMemWriteHandler_initialize(&interface_protocol_config_mem_write_handler);
    ProtocolConfigMemOperationsHandler_initialize(&interface_protocol_config_mem_operations_handler);

    OpenLcbNode_initialize(&interface_openlcb_node);

    OpenLcbLoginMessageHandler_initialize(&interface_openlcb_login_message_handler);
    OpenLcbLoginStateMachine_initialize(&interface_openlcb_login_state_machine);
    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine);

    OpenLcbApplication_initialize(&interface_openlcb_application);

    OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);

    printf("Does it work?\n");

    while (1) {

        // Run the main Openlcb/LCC engine
        CanMainStateMachine_run();
        OpenLcbLoginMainStatemachine_run();
        OpenLcbMainStatemachine_run();

    }
}
