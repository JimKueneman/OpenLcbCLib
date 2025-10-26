

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


#include "src/drivers/alias_mappings.h"


#include "node_parameters.h"
#include "osx_can_drivers.h"
#include "osx_drivers.h"



uint64_t node_id_base = 0x0507010100AA;

void _100ms_timer_callback(void) {

    // Calls back every 100ms... don't do anything crazy here as it is in the context of the interrupt

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

}

void _can_tx_callback(can_msg_t* can_msg) {

    // Called when a CAN message is transmitted

    //    printf("Tx: 0x%08lX\n", can_msg->identifier);

}

void _alias_change_callback(uint16_t new_alias, node_id_t node_id) {

//    printf("Alias Allocation: 0x%02X  ", new_alias);
//    printf("NodeID: 0x%04X%04X%04X\n\n", (uint16_t) (node_id >> 32), (uint16_t) (node_id >> 16), (uint16_t) node_id);

}

void _event_with_payload(openlcb_node_t* node, event_id_t* event_id, uint16_t count, event_payload_t* payload) {


}


#define TRANSMIT_CAN_FRAME_FUNC &OSxCanDriver_transmit_raw_can_frame
#define IS_TX_BUFFER_EMPTY_FUNC &OSxCanDriver_is_can_tx_buffer_clear
#define LOCK_SHARED_RESOURCES_FUNC &OSxCanDriver_pause_can_rx
#define UNLOCK_SHARED_RESOURCES_FUNC &OSxCanDriver_resume_can_rx
#define CONFIG_MEM_READ_FUNC OSxDrivers_config_mem_read

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
    .on_receive = &_can_rx_callback

};

const interface_can_tx_message_handler_t interface_can_tx_message_handler = {

    .transmit_can_frame = TRANSMIT_CAN_FRAME_FUNC, //  HARDWARE INTERFACE
    // Callback events
    .on_transmit = &_can_tx_callback

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
    .on_100ms_timer_tick = &_100ms_timer_callback

};

const interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network = {

};


// TODO Complete
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
    .on_pc_event_report_with_payload = &_event_with_payload

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
    .datagram = ProtocolDatagramHandler_handle_datagram,
    .datagram_ok_reply = Protocol_DatagramHandler_handle_datagram_received_ok,
    .datagram_rejected_reply = ProtocolDatagramHandler_handle_datagram_rejected,

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

    .configuration_memory_read = CONFIG_MEM_READ_FUNC

};

const interface_protocol_config_mem_read_handler_t interface_protocol_config_mem_read_handler = {

    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message,
    
    .snip_load_manufacturer_version_id = &ProtocolSnip_load_manufacturer_version_id,
    .snip_load_name = &ProtocolSnip_load_name,
    .snip_load_model = &ProtocolSnip_load_model,
    .snip_load_hardware_version = &ProtocolSnip_load_hardware_version,
    .snip_load_software_version = &ProtocolSnip_load_software_version,
    .snip_load_user_version_id = &ProtocolSnip_load_user_version_id,
    .snip_load_user_name = &ProtocolSnip_load_user_name,
    .snip_load_user_description = &ProtocolSnip_load_user_description,
    
    // Callbacks
    .on_read_space_config_decscription_info = NULL,
    .on_read_space_all = NULL,
    .on_read_space_configuration_memory = NULL,
    .on_read_space_acdi_manufacturer = NULL,
    .on_read_space_acdi_user = NULL,
    .on_read_space_traction_config_decscription_info = NULL,
    .on_read_space_traction_config_memory = NULL

};

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler = {

    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message,
    
    // Callbacks
    .on_write_space_config_decscription_info = NULL,
    .on_write_space_all = NULL,
    .on_write_space_configuration_memory = NULL,
    .on_write_space_acdi_manufacturer = NULL,
    .on_write_space_acdi_user = NULL,
    .on_write_space_traction_config_decscription_info = NULL,
    .on_write_space_traction_config_memory = NULL

};

const interface_protocol_config_mem_operations_handler_t interface_protocol_config_mem_operations_handler = {

    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message

};

// TODO Finish these interfaces
// -------------------------------

const interface_openlcb_application_t interface_openlcb_application = {

    .transmit_openlcb_message = NULL,
    .configuration_memory_read = NULL,
    .configuration_memory_write = NULL

};

const interface_protocol_datagram_handler_t interface_protocol_datagram_handler = {

    // Config Memory Read
    .memory_read_space_config_description_info = &ProtocolConfigMemReadHandler_memory_read_space_config_description_info,
    .memory_read_space_all = &ProtocolConfigMemReadHandler_memory_read_space_all,
    .memory_read_space_configuration_memory = &ProtocolConfigMemReadHandler_memory_read_space_configuration_memory,
    .memory_read_space_acdi_manufacturer = &ProtocolConfigMemReadHandler_memory_read_space_acdi_manufacturer,
    .memory_read_space_acdi_user = &ProtocolConfigMemReadHandler_memory_read_space_acdi_user,
    .memory_read_space_traction_function_definition_info = &ProtocolConfigMemReadHandler_memory_read_space_traction_function_definition_info,
    .memory_read_space_traction_function_config_memory = &ProtocolConfigMemReadHandler_memory_read_space_traction_function_config_memory,

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
    .memory_options_cmd = &ProtocolConfigMemOperationsHandler_memory_options_cmd,
    .memory_options_reply = &ProtocolConfigMemOperationsHandler_memory_options_reply,
    .memory_get_address_space_info = &ProtocolConfigMemOperationsHandler_memory_get_address_space_info,
    .memory_get_address_space_info_reply_not_present = &ProtocolConfigMemOperationsHandler_memory_get_address_space_info_reply_not_present,
    .memory_get_address_space_info_reply_present = &ProtocolConfigMemOperationsHandler_memory_get_address_space_info_reply_present,
    .memory_reserve_lock = &ProtocolConfigMemOperationsHandler_memory_reserve_lock,
    .memory_reserve_lock_reply = &ProtocolConfigMemOperationsHandler_memory_reserve_lock_reply,
    .memory_get_unique_id = ProtocolConfigMemOperationsHandler_memory_get_unique_id,
    .memory_get_unique_id_reply = &ProtocolConfigMemOperationsHandler_memory_get_unique_id_reply,
    .memory_unfreeze = &ProtocolConfigMemOperationsHandler_memory_unfreeze,
    .memory_freeze = &ProtocolConfigMemOperationsHandler_memory_freeze,
    .memory_update_complete = &ProtocolConfigMemOperationsHandler_memory_update_complete,
    .memory_reset_reboot = &ProtocolConfigMemOperationsHandler_memory_reset_reboot,
    .memory_factory_reset = &ProtocolConfigMemOperationsHandler_memory_factory_reset,


    .lock_shared_resources = LOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE
    .unlock_shared_resources = UNLOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE

};

int main(int argc, char *argv[])
{

  printf("Initializing...\n");
    
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
        OpenLcbLoginMainStatemachine_run();
        OpenLcbMainStatemachine_run();
        
    }
    
  }
