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
#include "../../../drivers/common/can_main_statemachine_handler.h"
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
#include "../../../openlcb/protocol_datagram_config_mem_handler.h"
#include "../../../openlcb/protocol_datagram_handler.h"

#include "../../../openlcb/openlcb_application.h"

#include "node_parameters.h"
#include "drivers.h"
#include "../dsPIC_Common/ecan1_helper.h"



uint64_t node_id_base = 0x0507010100AA;

uint16_t count = 0;

void _100ms_timer_callback(void) {

   // Calls back every 100ms... don't do anything crazy here as it is in the context of the interrupt
    
    count++;
    
    if (count > 10) {
        
        count = 0;
        
        LED_BLUE = 0;
        LED_YELLOW = 0;
    }

}

void _can_rx_callback(can_msg_t* can_msg) {

    // Called when a CAN message is received
    
//    printf("Rx: 0x%08lX [", can_msg->identifier);
//    
//    for (int i = 0; i < can_msg->payload_count; i++) {
//        
//        printf("0x%04X ", can_msg->payload[i]);
//        
//    }
//    
//    printf("]\n");
    
    LED_BLUE = 1;

}

void _can_tx_callback(can_msg_t* can_msg) {

    // Called when a CAN message is transmitted

//    printf("Tx: 0x%08lX\n", can_msg->identifier);
    
    LED_YELLOW = 1;
       
}

void _alias_change_callback(uint16_t new_alias, node_id_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    printf("NodeID: 0x%04X%04X%04X\n\n", (uint16_t) (node_id >> 32), (uint16_t) (node_id >> 16), (uint16_t) node_id);

}

void _event_with_payload(openlcb_node_t* node, event_id_t* event_id, uint16_t count, event_payload_t* payload) {
    
//    printf("event with payload: data count = %d; Total count = %d\n", count, count + sizeof(*event_id));
//    printf("event_id = 0x%08llX\n", *event_id);
//    
//    uint16_t i = 1;
//    
//    printf("[ ");
//    while (i <= count) {
//        
//        if (i % 8 == 0) {
//            
//            printf("0x%02X ] ", (*payload)[i-1]);
//            printf("\n");
//            printf("[ ");
//            
//        } else {
//
//          printf("0x%02X, ", (*payload)[i-1]);
//          
//        }
//          
//        i++;
//        
//    }
//
//    printf("\n\n");
    
}

const interface_can_login_message_handler_t interface_can_login_message_handler = {
    
    .extract_producer_event_state_mti = &ProtocolEventTransport_extract_producer_event_status_mti,
    .extract_consumer_event_state_mti = &ProtocolEventTransport_extract_consumer_event_status_mti,
    .alias_mapping_register = &AliasMappings_register,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    // Callback events
    .on_alias_change = &_alias_change_callback
    
};

const interface_can_login_state_machine_t interface_can_login_state_machine = {
    
    .init = &CanLoginMessageHandler_init,
    .generate_seed = &CanLoginMessageHandler_generate_seed,
    .generate_alias = &CanLoginMessageHandler_generate_alias,
    .load_cid07 = &CanLoginMessageHandler_load_cid07,
    .load_cid06 = &CanLoginMessageHandler_load_cid06,
    .load_cid05 = &CanLoginMessageHandler_load_cid05,
    .load_cid04 = &CanLoginMessageHandler_load_cid04,
    .wait_200ms = &CanLoginMessageHandler_wait_200ms,
    .load_rid = &CanLoginMessageHandler_load_rid,
    .load_amd = &CanLoginMessageHandler_load_amd,
    .load_initialization_complete = &CanLoginMessageHandler_load_initialization_complete,
    .load_producer_events = &CanLoginMessageHandler_load_producer_events,
    .load_consumer_events = &CanLoginMessageHandler_load_consumer_events, 
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
    .on_receive = &_can_rx_callback
    
};

const interface_can_tx_message_handler_t interface_can_tx_message_handler = {
    
    .transmit_can_frame = &Ecan1Helper_transmit_can_frame, //  HARDWARE INTERFACE
    // Callback events
    .on_transmit = &_can_tx_callback
    
};

const interface_can_tx_statemachine_t interface_can_tx_statemachine = {
    
    .is_tx_buffer_empty = Ecan1Helper_is_can_tx_buffer_clear, //  HARDWARE INTERFACE
    .handle_addressed_msg_frame = &CanTxMessageHandler_addressed_msg_frame,
    .handle_unaddressed_msg_frame = &CanTxMessageHandler_unaddressed_msg_frame,
    .handle_datagram_frame = &CanTxMessageHandler_datagram_frame,
    .handle_stream_frame = &CanTxMessageHandler_stream_frame,
    .handle_can_frame = &CanTxMessageHandler_can_frame
    
};


const interface_can_main_statemachine_handler_t interface_can_main_statemachine_handler = {
    
    
};

const interface_can_main_statemachine_t interface_can_main_statemachine = {
    
    .lock_shared_resources = Ecan1Helper_pause_can_rx,   //  HARDWARE INTERFACE
    .unlock_shared_resources = Ecan1Helper_resume_can_rx, //  HARDWARE INTERFACE
    .send_can_message = &CanTxStatemachine_send_can_message,
    .send_openlcb_message = &CanTxStatemachine_send_openlcb_message,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .openlcb_node_find_by_alias = &OpenLcbNode_find_by_alias,
    .login_statemachine_run = &CanLoginStateMachine_run,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_unregister = &AliasMappings_unregister
    
};

const interface_alias_mappings_t interface_alias_mappings = {
    
    
};


const interface_openlcb_node_t interface_openlcb_node = {
    
    // Callback events
    .on_100ms_timer_tick = &_100ms_timer_callback
  
};

const interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network = {
    
};


// TODO Complete
const  interface_openlcb_protocol_event_transport_t interface_openlcb_protocol_event_transport = {
  
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
    .on_pc_event_report_with_payload = &_event_with_payload
    
};

// TODO Complete
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
    .event_transport_consumer_identified_unknown =&ProtocolEventTransport_handle_consumer_identified_unknown,
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
    .datagram = NULL,
    .datagram_ok_reply = NULL,
    .datagram_rejected_reply = NULL,
    
    // STREAM
    .stream_initiate_request = NULL,
    .stream_initiate_reply = NULL,
    .stream_send_data = NULL,
    .stream_data_proceed = NULL,
    .stream_data_complete = NULL,
    
    // required
    .lock_shared_resources = Ecan1Helper_pause_can_rx,   //  HARDWARE INTERFACE
    .unlock_shared_resources = Ecan1Helper_resume_can_rx,   //  HARDWARE INTERFACE
    .send_openlcb_msg = &CanTxStatemachine_send_openlcb_message,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .load_interaction_rejected = OpenLcbMainStatemachine_load_interaction_rejected,
    
    // for test injection, leave null to use the default functions
    .process_main_statemachine = OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = &OpenLcbMainStatemachine_does_node_process_msg,
    
};

const interface_openlcb_protocol_snip_t interface_openlcb_protocol_snip = {
    
    .configuration_memory_read = &BasicNodeDrivers_config_mem_read
    
};

// TODO Finish these interfaces
// -------------------------------

const interface_protocol_datagram_handler_t interface_protocol_datagram_handler = {
    
    // Memory Read Address Space
    .memory_read_space_config_description_info_message = NULL,
    .memory_read_space_all_message = NULL,
    .memory_read_space_configuration_memory_message = NULL,
    .memory_read_space_acdi_manufacturer_message = NULL,
    .memory_read_space_acdi_user_message = NULL,
    .memory_read_space_traction_function_definition_info_message = NULL,
    .memory_read_space_traction_function_config_memory_message = NULL,
   
    // Memory Read Ok Ok Address Space
    .memory_read_space_config_description_info_reply_ok_message = NULL,
    .memory_read_space_all_reply_ok_message = NULL,
    .memory_read_space_configuration_memory_reply_ok_message = NULL,
    .memory_read_space_acdi_manufacturer_reply_ok_message = NULL,
    .memory_read_space_acdi_user_reply_ok_message = NULL,
    .memory_read_space_traction_function_definition_info_reply_ok_message = NULL,
    .memory_read_space_traction_function_config_memory_reply_ok_message = NULL,
  
    // Memory Read Failed Reply Address Space
    .memory_read_space_config_description_info_reply_fail_message = NULL,
    .memory_read_space_all_reply_fail_message = NULL,
    .memory_read_space_configuration_memory_reply_fail_message = NULL,
    .memory_read_space_acdi_manufacturer_reply_fail_message = NULL,
    .memory_read_space_acdi_user_reply_fail_message = NULL,
    .memory_read_space_traction_function_definition_info_reply_fail_message = NULL,
    .memory_read_space_traction_function_config_memory_reply_fail_message = NULL,

    // Memory Write Address Space
    .memory_write_space_config_description_info_message = NULL,
    .memory_write_space_all_message = NULL,
    .memory_write_space_configuration_memory_message = NULL,
    .memory_write_space_acdi_manufacturer_message = NULL,
    .memory_write_space_acdi_user_message = NULL,
    .memory_write_space_traction_function_definition_info_message = NULL,
    .memory_write_space_traction_function_config_memory_message = NULL,
    .memory_write_space_firmware_upgrade_message = NULL,
   
    // Memory Write Ok Reply Address Space
    .memory_write_space_config_description_info_reply_ok_message = NULL,
    .memory_write_space_all_reply_ok_message = NULL,
    .memory_write_space_configuration_memory_reply_ok_message = NULL,
    .memory_write_space_acdi_manufacturer_reply_ok_message = NULL,
    .memory_write_space_acdi_user_reply_ok_message = NULL,
    .memory_write_space_traction_function_definition_info_reply_ok_message = NULL,
    .memory_write_space_traction_function_config_memory_reply_ok_message = NULL,
   
    // Memory Write Fail Reply Address Space
    .memory_write_space_config_description_info_reply_fail_message = NULL,
    .memory_write_space_all_reply_fail_message = NULL,
    .memory_write_space_configuration_memory_reply_fail_message = NULL,
    .memory_write_space_acdi_manufacturer_reply_fail_message = NULL,
    .memory_write_space_acdi_user_reply_fail_message = NULL,
    .memory_write_space_traction_function_definition_info_reply_fail_message = NULL,
    .memory_write_space_traction_function_config_memory_reply_fail_message = NULL,
    
    // Memory Write Under Mask Address Space
    .memory_write_under_mask_space_config_description_info_message = NULL,
    .memory_write_under_mask_space_all_message = NULL,
    .memory_write_under_mask_space_configuration_memory_message = NULL,
    .memory_write_under_mask_space_acdi_manufacturer_message = NULL,
    .memory_write_under_mask_space_acdi_user_message = NULL,
    .memory_write_under_mask_space_traction_function_definition_info_message = NULL,
    .memory_write_under_mask_space_traction_function_config_memory_message = NULL,
    .memory_write_under_mask_space_firmware_upgrade_message = NULL,
    
    // Commands
    .memory_options_cmd_message = NULL,
    .memory_options_reply_message = NULL,
    .memory_get_address_space_info_message = NULL,
    .memory_get_address_space_info_reply_not_present_message = NULL,
    .memory_get_address_space_info_reply_present_message = NULL,
    .memory_reserve_lock_message = NULL,
    .memory_get_unique_id_message = NULL,
    .memory_unfreeze_message = NULL,
    .memory_freeze_message = NULL,
    .memory_update_complete_message = NULL,
    .memory_reset_reboot_message = NULL,
    .memory_factory_reset_message = NULL,
    .send_datagram_rejected_reply = NULL
  
};

const interface_openlcb_application_t interface_openlcb_application = {
    
    .transmit_openlcb_message = NULL,
    .configuration_memory_read = NULL,
    .configuration_memory_write = NULL
    
};

interface_openlcb_protocol_datagram_config_mem_handler_t interface_openlcb_protocol_datagram_config_mem_handler = {
    
    .configuration_memory_read = NULL,
    .configuration_memory_write = NULL,
    .reboot = NULL,
    .configuration_memory_factory_reset = NULL,
    .snip_load_manufacturer_version_id = NULL,
    .snip_load_name = NULL,
    .snip_load_model = NULL,
    .snip_load_hardware_version = NULL,
    .snip_load_software_version = NULL,
    .snip_load_user_version_id = NULL,
    .snip_load_user_name = NULL,
    .snip_load_user_description = NULL,
    // Callback events
    .on_configuration_memory_factory_reset = NULL,
    .on_config_mem_write = NULL,
    .on_config_mem_freeze_firmware_update = NULL,
    .on_config_mem_unfreeze_firmware_update = NULL
    
};


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

    CanMainStatemachineHandler_initialize(&interface_can_main_statemachine_handler);
    CanMainStatemachine_initialize(&interface_can_main_statemachine);
    
    AliasMappings_initialize(&interface_alias_mappings);

    OpenLcbBufferStore_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbBufferFifo_initialize();
    
    ProtocolSnip_initialize(&interface_openlcb_protocol_snip);
    ProtocolDatagramConfigMemHandler_initialize(&interface_openlcb_protocol_datagram_config_mem_handler);
    ProtocolDatagramHandler_initialize(&interface_protocol_datagram_handler);
    ProtocolEventTransport_initialize(&interface_openlcb_protocol_event_transport);
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network);
    
    OpenLcbNode_initialize(&interface_openlcb_node);
    
    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine);
    
    OpenLcbApplication_initialize(&interface_openlcb_application);

    // We always boot and reallocate the alias
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101010707, &NodeParameters_main_node);

    printf("Does it work?\n");
   
    while (1) {

        // Run the main Openlcb/LCC engine
        CanMainStateMachine_run();
        OpenLcbMainStatemachine_run();

    }
}
