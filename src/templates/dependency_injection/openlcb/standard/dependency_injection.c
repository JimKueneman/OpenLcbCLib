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
 * \file dependency_injection.c
 *
 *
 * @author Jim Kueneman
 * @date 27 Dec 2025
 */

#include "dependency_injection.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "../../function_injection_defines.h"

// OpenlcbCLib includes
#include "../openlcb/openlcb_application.h"
#include "../openlcb/openlcb_defines.h"
#include "../openlcb/openlcb_types.h"
#include "../openlcb/openlcb_utilities.h"
#include "../openlcb/openlcb_buffer_store.h"
#include "../openlcb/openlcb_buffer_list.h"
#include "../openlcb/openlcb_buffer_fifo.h"
#include "../openlcb/openlcb_node.h"
#include "../openlcb/protocol_message_network.h"
#include "../openlcb/protocol_event_transport.h"
#include "../openlcb/protocol_snip.h"
#include "../openlcb/openlcb_main_statemachine.h"
#include "../openlcb/protocol_datagram_handler.h"
#include "../openlcb/openlcb_login_statemachine.h"
#include "../openlcb/openlcb_login_statemachine_handler.h"
#include "../openlcb/protocol_config_mem_read_handler.h"
#include "../openlcb/protocol_config_mem_write_handler.h"
#include "../openlcb/protocol_config_mem_operations_handler.h"
#include "../openlcb/protocol_broadcast_time_handler.h"
#include "../openlcb/openlcb_application_broadcast_time.h"

const interface_openlcb_protocol_broadcast_time_handler_t interface_openlcb_protocol_broadcast_time_handler = {

    .on_time_received = ON_BROADCAST_TIME_RECEIVED,
    .on_date_received = ON_BROADCAST_DATE_RECEIVED,
    .on_year_received = ON_BROADCAST_YEAR_RECEIVED,
    .on_rate_received = ON_BROADCAST_RATE_RECEIVED,
    .on_clock_started = ON_BROADCAST_CLOCK_STARTED,
    .on_clock_stopped = ON_BROADCAST_CLOCK_STOPPED,
    .on_date_rollover = ON_BROADCAST_DATE_ROLLOVER,

};

const interface_openlcb_node_t interface_openlcb_node = {

    // Optional Callback function assignments
    .on_100ms_timer_tick = ON_100MS_TIMER_CALLBACK

};

const interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network = {

};

const interface_openlcb_protocol_event_transport_t interface_openlcb_protocol_event_transport = {

    // Optional Callback function assignments
    .on_consumed_event_identified = ON_CONSUMED_EVENT_IDENTIFIED,
    .on_consumed_event_pcer = ON_CONSUMED_EVENT_PCER,
    .on_event_learn = ON_EVENT_LEARN_CALLBACK,

};

const interface_openlcb_login_message_handler_t interface_openlcb_login_message_handler = {

    // Required function assignments
    .extract_producer_event_state_mti = &ProtocolEventTransport_extract_producer_event_status_mti,
    .extract_consumer_event_state_mti = &ProtocolEventTransport_extract_consumer_event_status_mti

};

const interface_openlcb_login_state_machine_t interface_openlcb_login_state_machine = {

    // Required function assignments
    .send_openlcb_msg = SEND_OPENLCB_MESSAGE_FUNC, // HARDWARE INTERFACE
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,

    // Required Handler function assignments
    .load_initialization_complete = &OpenLcbLoginMessageHandler_load_initialization_complete,
    .load_producer_events = &OpenLcbLoginMessageHandler_load_producer_event,
    .load_consumer_events = &OpenLcbLoginMessageHandler_load_consumer_event,

    // Required internal function assignments (for testability)
    .process_login_statemachine = &OpenLcbLoginStateMachine_process,
    .handle_outgoing_openlcb_message = &OpenLcbLoginStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &OpenLcbLoginStatemachine_handle_try_reenumerate,
    .handle_try_enumerate_first_node = &OpenLcbLoginStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &OpenLcbLoginStatemachine_handle_try_enumerate_next_node,

    // Optional
    .on_login_complete = ON_LOGIN_COMPLETE_CALLBACK

};

const interface_openlcb_main_statemachine_t interface_openlcb_main_statemachine = {

    // Required function assignments
    .lock_shared_resources = LOCK_SHARED_RESOURCES_FUNC,     // HARDWARE INTERFACE
    .unlock_shared_resources = UNLOCK_SHARED_RESOURCES_FUNC, // HARDWARE INTERFACE
    .send_openlcb_msg = SEND_OPENLCB_MESSAGE_FUNC,           // HARDWARE INTERFACE
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .load_interaction_rejected = &OpenLcbMainStatemachine_load_interaction_rejected,

    // Required Message Network Protocol Handler function assignments
    .message_network_initialization_complete = ProtocolMessageNetwork_handle_initialization_complete,
    .message_network_initialization_complete_simple = ProtocolMessageNetwork_handle_initialization_complete_simple,
    .message_network_verify_node_id_addressed = &ProtocolMessageNetwork_handle_verify_node_id_addressed,
    .message_network_verify_node_id_global = &ProtocolMessageNetwork_handle_verify_node_id_global,
    .message_network_verified_node_id = &ProtocolMessageNetwork_handle_verified_node_id,
    .message_network_optional_interaction_rejected = &ProtocolMessageNetwork_handle_optional_interaction_rejected,
    .message_network_terminate_due_to_error = &ProtocolMessageNetwork_handle_terminate_due_to_error,

    // Required Protocol Support Protocol (PIP) Handler function assignments
    .message_network_protocol_support_inquiry = &ProtocolMessageNetwork_handle_protocol_support_inquiry,
    .message_network_protocol_support_reply = &ProtocolMessageNetwork_handle_protocol_support_reply,

    // Required internal function assignments (for testability)
    .process_main_statemachine = OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = &OpenLcbMainStatemachine_does_node_process_msg,
    .handle_outgoing_openlcb_message = &OpenLcbMainStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &OpenLcbMainStatemachine_handle_try_reenumerate,
    .handle_try_pop_next_incoming_openlcb_message = &OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message,
    .handle_try_enumerate_first_node = &OpenLcbMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &OpenLcbMainStatemachine_handle_try_enumerate_next_node,

    // Optional SNIP Protocol Handler function assignments
    .snip_simple_node_info_request = &ProtocolSnip_handle_simple_node_info_request,
    .snip_simple_node_info_reply = &ProtocolSnip_handle_simple_node_info_reply,

    // Optional Event Transport Protocol Handler function assignments
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

    // Optional Traction Protocol Handler function assignments
    .traction_control_command = NULL,
    .traction_control_reply = NULL,

    // Optional Traction SNIP Protocol Handler function assignments
    .simple_train_node_ident_info_request = NULL,
    .simple_train_node_ident_info_reply = NULL,

    // Optional Datagram Protocol Handler function assignments
    .datagram = ProtocolDatagramHandler_datagram,
    .datagram_ok_reply = ProtocolDatagramHandler_datagram_received_ok,
    .datagram_rejected_reply = ProtocolDatagramHandler_datagram_rejected,

    // Optional Stream Protocol Handler function assignments
    .stream_initiate_request = NULL,
    .stream_initiate_reply = NULL,
    .stream_send_data = NULL,
    .stream_data_proceed = NULL,
    .stream_data_complete = NULL,

};

const interface_openlcb_protocol_snip_t interface_openlcb_protocol_snip = {

    // Required function assignments
    .config_memory_read = CONFIG_MEM_READ_FUNC, // HARDWARE INTERFACE

};

const interface_protocol_config_mem_read_handler_t interface_protocol_config_mem_read_handler = {

    // Required function assignments
    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message,
    .config_memory_read = CONFIG_MEM_READ_FUNC,

    // Optional function assignments (Required if Address Spaces 0xFB/0xFC ACDI Manufacturer/User Protocol is enabled)
    .snip_load_manufacturer_version_id = &ProtocolSnip_load_manufacturer_version_id,
    .snip_load_name = &ProtocolSnip_load_name,
    .snip_load_model = &ProtocolSnip_load_model,
    .snip_load_hardware_version = &ProtocolSnip_load_hardware_version,
    .snip_load_software_version = &ProtocolSnip_load_software_version,
    .snip_load_user_version_id = &ProtocolSnip_load_user_version_id,
    .snip_load_user_name = &ProtocolSnip_load_user_name,
    .snip_load_user_description = &ProtocolSnip_load_user_description,

    // Optional request function assignments (Encapsulated functions to read specific address spaces)
    .read_request_config_definition_info = &ProtocolConfigMemReadHandler_read_request_config_definition_info,
    .read_request_all = NULL, // Unimplemented
    .read_request_config_mem = &ProtocolConfigMemReadHandler_read_request_config_mem,
    .read_request_acdi_manufacturer = &ProtocolConfigMemReadHandler_read_request_acdi_manufacturer,
    .read_request_acdi_user = &ProtocolConfigMemReadHandler_read_request_acdi_user,
    .read_request_traction_function_config_definition_info = NULL, // Unimplemented
    .read_request_traction_function_config_memory = NULL,          // Unimplemented

    // Optional override to return the flag in the Datagram ACK to allow the client to say the reply will be coming 2^N seconds
    .delayed_reply_time = CONFIG_MEM_READ_DELAYED_REPLY_TIME_FUNC

};

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler = {

    // Required function assignments
    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message,
    .config_memory_write = CONFIG_MEM_WRITE_FUNC,

    // Optional request function assignments (Encapsulated functions to write specific address spaces)
    .write_request_config_definition_info = NULL, // Typically never implemented
    .write_request_all = NULL,                    // Typically never implemented
    .write_request_config_mem = &ProtocolConfigMemWriteHandler_write_request_config_mem,
    .write_request_acdi_manufacturer = NULL, // Typically never implemented
    .write_request_acdi_user = &ProtocolConfigMemWriteHandler_write_request_acdi_user,
    .write_request_traction_function_config_definition_info = NULL, // Typically never implemented
    .write_request_traction_function_config_memory = NULL,
    .write_request_firmware = FIRMWARE_WRITE_FUNC,

    // Optional override to return the flag in the Datagram ACK to allow the client to say the reply will be coming 2^N seconds
    .delayed_reply_time = CONFIG_MEM_WRITE_DELAYED_REPLY_TIME_FUNC

};

const interface_protocol_config_mem_operations_handler_t interface_protocol_config_mem_operations_handler = {

    // Required function assignments
    .load_datagram_received_ok_message = &ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message,

    // Optional request function assignments (Encapsulated functions to perform the particular Operations Task)
    .operations_request_options_cmd = &ProtocolConfigMemOperationsHandler_request_options_cmd,
    .operations_request_options_cmd_reply = NULL,
    .operations_request_get_address_space_info = ProtocolConfigMemOperationsHandler_request_get_address_space_info,
    .operations_request_get_address_space_info_reply_present = NULL,
    .operations_request_get_address_space_info_reply_not_present = NULL,
    .operations_request_reserve_lock = ProtocolConfigMemOperationsHandler_request_reserve_lock,
    .operations_request_reserve_lock_reply = NULL,
    .operations_request_get_unique_id = NULL,
    .operations_request_get_unique_id_reply = NULL,
    .operations_request_freeze = OPERATIONS_REQUEST_FREEZE_FUNC,
    .operations_request_unfreeze = OPERATIONS_REQUEST_UNFREEZE_FUNC,
    .operations_request_update_complete = NULL,
    .operations_request_reset_reboot = OPERATIONS_REQUEST_REBOOT_FUNC,        // HARDWARE INTERFACE
    .operations_request_factory_reset = OPERATIONS_REQUEST_FACTORY_RESET_FUNC // HARDWARE INTERFACE

};

const interface_openlcb_application_t interface_openlcb_application = {

    // Required function assignments
    .send_openlcb_msg = SEND_OPENLCB_MESSAGE_FUNC,
    .config_memory_read = CONFIG_MEM_READ_FUNC,
    .config_memory_write = CONFIG_MEM_WRITE_FUNC

};

const interface_protocol_datagram_handler_t interface_protocol_datagram_handler = {

    .lock_shared_resources = LOCK_SHARED_RESOURCES_FUNC,     //  HARDWARE INTERFACE
    .unlock_shared_resources = UNLOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE

    // Optional functions to implement Address Space access to Read Address Spaces, these are general functions that call the request functions defined in interface_protocol_config_mem_read_handler_t
    .memory_read_space_config_description_info = &ProtocolConfigMemReadHandler_read_space_config_description_info,
    .memory_read_space_all = &ProtocolConfigMemReadHandler_read_space_all,
    .memory_read_space_configuration_memory = &ProtocolConfigMemReadHandler_read_space_config_memory,
    .memory_read_space_acdi_manufacturer = &ProtocolConfigMemReadHandler_read_space_acdi_manufacturer,
    .memory_read_space_acdi_user = &ProtocolConfigMemReadHandler_read_space_acdi_user,
    .memory_read_space_traction_function_definition_info = &ProtocolConfigMemReadHandler_read_space_traction_function_definition_info,
    .memory_read_space_traction_function_config_memory = &ProtocolConfigMemReadHandler_read_space_traction_function_config_memory,

    // Optional functions to implement Datagram replies (Only required if the node is requesting a datagram from some other node and this is the OK reply)
    .memory_read_space_config_description_info_reply_ok = NULL,
    .memory_read_space_all_reply_ok = NULL,
    .memory_read_space_configuration_memory_reply_ok = NULL,
    .memory_read_space_acdi_manufacturer_reply_ok = NULL,
    .memory_read_space_acdi_user_reply_ok = NULL,
    .memory_read_space_traction_function_definition_info_reply_ok = NULL,
    .memory_read_space_traction_function_config_memory_reply_ok = NULL,

    // Optional functions to implement Address Space read replies (Only required if the node is requesting a datagram from some other node and this is the FAIL reply)
    .memory_read_space_config_description_info_reply_fail = NULL,
    .memory_read_space_all_reply_fail = NULL,
    .memory_read_space_configuration_memory_reply_fail = NULL,
    .memory_read_space_acdi_manufacturer_reply_fail = NULL,
    .memory_read_space_acdi_user_reply_fail = NULL,
    .memory_read_space_traction_function_definition_info_reply_fail = NULL,
    .memory_read_space_traction_function_config_memory_reply_fail = NULL,

    // Optional functions to implement Address Space access to Read Address Spaces through a Stream
    .memory_read_stream_space_config_description_info = NULL,
    .memory_read_stream_space_all = NULL,
    .memory_read_stream_space_configuration_memory = NULL,
    .memory_read_stream_space_acdi_manufacturer = NULL,
    .memory_read_stream_space_acdi_user = NULL,
    .memory_read_stream_space_traction_function_definition_info = NULL,
    .memory_read_stream_space_traction_function_config_memory = NULL,

    // Optional functions to implement Address Space read replies through a Stream (Only required if the node is requesting a datagram from some other node and this is the OK reply)
    .memory_read_stream_space_config_description_info_reply_ok = NULL,
    .memory_read_stream_space_all_reply_ok = NULL,
    .memory_read_stream_space_configuration_memory_reply_ok = NULL,
    .memory_read_stream_space_acdi_manufacturer_reply_ok = NULL,
    .memory_read_stream_space_acdi_user_reply_ok = NULL,
    .memory_read_stream_space_traction_function_definition_info_reply_ok = NULL,
    .memory_read_stream_space_traction_function_config_memory_reply_ok = NULL,

    // Optional functions to implement Address Space read replies through a Stream (Only required if the node is requesting a datagram from some other node and this is the FAIL reply)
    .memory_read_stream_space_config_description_info_reply_fail = NULL,
    .memory_read_stream_space_all_reply_fail = NULL,
    .memory_read_stream_space_configuration_memory_reply_fail = NULL,
    .memory_read_stream_space_acdi_manufacturer_reply_fail = NULL,
    .memory_read_stream_space_acdi_user_reply_fail = NULL,
    .memory_read_stream_space_traction_function_definition_info_reply_fail = NULL,
    .memory_read_stream_space_traction_function_config_memory_reply_fail = NULL,

    // Optional functions to implement Address Space access to Write Address Spaces, these are general functions that call the request functions defined in interface_protocol_config_mem_write_handler_t
    .memory_write_space_config_description_info = NULL, // Typically NULL as this a a read only space
    .memory_write_space_all = NULL,                     // Typically NULL as this a a read only space
    .memory_write_space_configuration_memory = &ProtocolConfigMemWriteHandler_write_space_config_memory,
    .memory_write_space_acdi_manufacturer = NULL, // Typically NULL as this a a read only space
    .memory_write_space_acdi_user = &ProtocolConfigMemWriteHandler_write_space_acdi_user,
    .memory_write_space_traction_function_definition_info = NULL, // Typically NULL as this a a read only space
    .memory_write_space_traction_function_config_memory = &ProtocolConfigMemWriteHandler_write_space_traction_function_config_memory,
    .memory_write_space_firmware_upgrade = &ProtocolConfigMemWriteHandler_write_space_firmware,

    // Optional functions to implement Address Space write replies (Only required if the node is requesting a datagram from some other node and this is the OK reply)
    .memory_write_space_config_description_info_reply_ok = NULL, // Typically never called as this a a read only space
    .memory_write_space_all_reply_ok = NULL,                     // Typically never called as this a a read only space
    .memory_write_space_configuration_memory_reply_ok = NULL,
    .memory_write_space_acdi_manufacturer_reply_ok = NULL, // Typically never called as this a a read only space
    .memory_write_space_acdi_user_reply_ok = NULL,
    .memory_write_space_traction_function_definition_info_reply_ok = NULL, // Typically never called as this a a read only space
    .memory_write_space_traction_function_config_memory_reply_ok = NULL,

    // Optional functions to implement Address Space write replies (Only required if the node is requesting a datagram from some other node and this is the FAIL reply)
    .memory_write_space_config_description_info_reply_fail = NULL,
    .memory_write_space_all_reply_fail = NULL,
    .memory_write_space_configuration_memory_reply_fail = NULL,
    .memory_write_space_acdi_manufacturer_reply_fail = NULL,
    .memory_write_space_acdi_user_reply_fail = NULL,
    .memory_write_space_traction_function_definition_info_reply_fail = NULL,
    .memory_write_space_traction_function_config_memory_reply_fail = NULL,

    // Optional functions to implement Address Space write under a mask
    .memory_write_under_mask_space_config_description_info = NULL, // Typically NULL as this a a read only space
    .memory_write_under_mask_space_all = NULL,                     // Typically NULL as this a a read only space
    .memory_write_under_mask_space_configuration_memory = NULL,
    .memory_write_under_mask_space_acdi_manufacturer = NULL, // Typically NULL as this a a read only space
    .memory_write_under_mask_space_acdi_user = NULL,
    .memory_write_under_mask_space_traction_function_definition_info = NULL, // Typically NULL as this a a read only space
    .memory_write_under_mask_space_traction_function_config_memory = NULL,
    .memory_write_under_mask_space_firmware_upgrade = NULL,

    // Optional functions to implement Address Space access to Write Address Spaces through a Stream
    .memory_write_stream_space_config_description_info = NULL, // Typically NULL as this a a read only space
    .memory_write_stream_space_all = NULL,                     // Typically NULL as this a a read only space
    .memory_write_stream_space_configuration_memory = NULL,
    .memory_write_stream_space_acdi_manufacturer = NULL, // Typically NULL as this a a read only space
    .memory_write_stream_space_acdi_user = NULL,
    .memory_write_stream_space_traction_function_definition_info = NULL, // Typically NULL as this a a read only space
    .memory_write_stream_space_traction_function_config_memory = NULL,
    .memory_write_stream_space_firmware_upgrade = NULL,

    // Optional functions to implement Address Space write replies through a Stream (Only required if the node is requesting a datagram from some other node and this is the OK reply)
    .memory_write_stream_space_config_description_info_reply_ok = NULL, // Typically never called as this a a read only space
    .memory_write_stream_space_all_reply_ok = NULL,                     // Typically never called as this a a read only space
    .memory_write_stream_space_configuration_memory_reply_ok = NULL,
    .memory_write_stream_space_acdi_manufacturer_reply_ok = NULL, // Typically never called as this a a read only space
    .memory_write_stream_space_acdi_user_reply_ok = NULL,
    .memory_write_stream_space_traction_function_definition_info_reply_ok = NULL, // Typically never called as this a a read only space
    .memory_write_stream_space_traction_function_config_memory_reply_ok = NULL,

    // Optional functions to implement Address Space write replies through a Stream (Only required if the node is requesting a datagram from some other node and this is the FAIL reply)
    .memory_write_stream_space_config_description_info_reply_fail = NULL,
    .memory_write_stream_space_all_reply_fail = NULL,
    .memory_write_stream_space_configuration_memory_reply_fail = NULL,
    .memory_write_stream_space_acdi_manufacturer_reply_fail = NULL,
    .memory_write_stream_space_acdi_user_reply_fail = NULL,
    .memory_write_stream_space_traction_function_definition_info_reply_fail = NULL,
    .memory_write_stream_space_traction_function_config_memory_reply_fail = NULL,

    // Optional functions to implement the commands in the Configuration Memory Operations, typically assigned the handlers in interface_protocol_config_mem_operations_handler_t.
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

};

void DependencyInjection_initialize(void)
{

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
    ProtocolBroadcastTime_initialize(&interface_openlcb_protocol_broadcast_time_handler);
    OpenLcbApplicationBroadcastTime_initialize();

    OpenLcbNode_initialize(&interface_openlcb_node);

    OpenLcbLoginMessageHandler_initialize(&interface_openlcb_login_message_handler);
    OpenLcbLoginStateMachine_initialize(&interface_openlcb_login_state_machine);
    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine);

    OpenLcbApplication_initialize(&interface_openlcb_application);
}