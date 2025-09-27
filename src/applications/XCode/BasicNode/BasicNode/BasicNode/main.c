

#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include "strings.h"
#include "pthread.h"



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
#include "src/drivers/common/can_main_statemachine_handler.h"
#include "src/drivers/common/can_main_statemachine.h"

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
#include "src/openlcb/openlcb_application.h"

#include "src/drivers/alias_mappings.h"



#include "node_parameters.h"
#include "osx_can_drivers.h"
#include "osx_drivers.h"



uint64_t node_id_base = 0x0507010100AA;

const interface_can_login_message_handler_t interface_can_login_message_handler = {
    
    .extract_producer_event_state_mti = &ProtocolEventTransport_extract_producer_event_status_mti,
    .extract_consumer_event_state_mti = &ProtocolEventTransport_extract_consumer_event_status_mti,
    .alias_mapping_register = &AliasMappings_register,
    // Callback events
    .on_alias_change = NULL
    
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
    .load_consumer_events = &CanLoginMessageHandler_load_consumer_events
    
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
        .handle_stream = &CanRxMessageHandler_stream,
        .handle_rid_frame = CanRxMessageHandler_rid_frame,
        .handle_amd_frame = CanRxMessageHandler_amd_frame,
        .handle_ame_frame = CanRxMessageHandler_ame_frame,
        .handle_amr_frame = CanRxMessageHandler_amr_frame,
        .handle_error_info_report_frame = CanRxMessageHandler_error_info_report_frame,
        .handle_cid_frame = CanRxMessageHandler_cid_frame,
        // Callback events
        .on_receive = NULL
    
};

const interface_can_tx_message_handler_t interface_can_tx_message_handler = {
    
    .transmit_can_frame = OSxCanDriver_transmit_raw_can_frame, //  HARDWARE INTERFACE
    // Callback events
    .on_transmit = NULL
    
};

const interface_can_tx_statemachine_t interface_can_tx_statemachine = {
    
    .is_tx_buffer_empty = OSxCanDriver_is_can_tx_buffer_clear, //  HARDWARE INTERFACE
    .handle_addressed_msg_frame = &CanTxMessageHandler_addressed_msg_frame,
    .handle_unaddressed_msg_frame = &CanTxMessageHandler_unaddressed_msg_frame,
    .handle_datagram_frame = &CanTxMessageHandler_datagram_frame,
    .handle_stream_frame = &CanTxMessageHandler_stream_frame,
    .handle_can_frame = &CanTxMessageHandler_can_frame
    
};

const interface_can_main_statemachine_t interface_can_main_statemachine = {
    
    .lock_shared_resources = OSxCanDriver_pause_can_rx,   //  HARDWARE INTERFACE
    .unlock_shared_resources = OSxCanDriver_resume_can_rx, //  HARDWARE INTERFACE
    .send_can_message = &CanTxStatemachine_send_can_message,
    .send_openlcb_message = &CanTxStatemachine_send_openlcb_message,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .openlcb_node_find_by_alias = &OpenLcbNode_find_by_alias,
    .login_statemachine_run = &CanLoginStateMachine_run,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_unregister = &AliasMappings_unregister
    
};


const interface_openlcb_node_t interface_openlcb_node = {
    
  
};

const interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network = {
    
};


// TODO Complete
const  interface_openlcb_protocol_event_transport_t interface_openlcb_protocol_event_transport = {
  
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
    .on_pc_event_report_with_payload = NULL
    
};


// TODO Complete
const interface_openlcb_main_statemachine_t interface_openlcb_main_statemachine = {
    
    // SNIP
    .snip_simple_node_info_request = &ProtocolSnip_handle_simple_node_info_request,
    .snip_simple_node_info_reply = &ProtocolSnip_handle_simple_node_info_reply,
    
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
    .lock_shared_resources = OSxCanDriver_pause_can_rx,   //  HARDWARE INTERFACE
    .unlock_shared_resources = OSxCanDriver_resume_can_rx,   //  HARDWARE INTERFACE
    .send_openlcb_msg = &CanTxStatemachine_send_openlcb_message,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .load_interaction_rejected = OpenLcbMainStatemachine_load_interaction_rejected,
    
    // for test injection, leave null to use the default functions
    .process_main_statemachine = OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = &OpenLcbMainStatemachine_does_node_process_msg,
    
};

const interface_openlcb_protocol_snip_t interface_openlcb_protocol_snip = {
    
    .configuration_memory_read = &OSxDrivers_config_mem_read
    
};

const interface_alias_mappings_t interface_alias_mappings = {
    
    
};

int main(int argc, char *argv[])
{

  printf("Initializing...\n");
    
    CanBufferStore_initialize();
    CanBufferFifo_initialize();

    CanLoginMessageHandler_initialize(&interface_can_login_message_handler);
    CanLoginStateMachine_initialize(&interface_can_login_state_machine);

    CanRxMessageHandler_initialize(&interface_can_rx_message_handler);
    CanRxStatemachine_initialize(&interface_can_rx_statemachine);

    CanTxMessageHandler_initialize(&interface_can_tx_message_handler);
    CanTxStatemachine_initialize(&interface_can_tx_statemachine);

    CanMainStatemachine_initialize(&interface_can_main_statemachine);


    OpenLcbBufferStore_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbBufferFifo_initialize();
    
    OpenLcbNode_initialize(&interface_openlcb_node);
    
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network);
    ProtocolEventTransport_initialize(&interface_openlcb_protocol_event_transport);
    ProtocolSnip_initialize(&interface_openlcb_protocol_snip);

    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine);
    
    AliasMappings_initialize(&interface_alias_mappings);
    
    OSxDrivers_setup();
    OSxCanDriver_setup();
    
  
  printf("Waiting for CAN and 100ms Timer Drivers to connect\n");

  while (!(OSxDrivers_100ms_is_connected() && OSxCanDriver_is_connected() && OSxDrivers_input_is_connected()))
  {
    printf("Waiting for Threads\n");
    sleep(2);
  }

  printf("Allocating Node\n");
  #ifdef PLATFORMIO
  uint64_olcb_t nodeid = 0x0501010107DD;
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
        
        usleep(25);
        
        CanMainStateMachine_run();
        OpenLcbMainStatemachine_run();
        
    }
    
  }
