#include "test/main_Test.hxx"

#include "openlcb_main_statemachine.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

uint16_t handler_mti = 0x00;
bool load_interaction_rejected_called = false;
bool reply_to_protocol_support_inquiry = false;
bool force_process_statemachine_to_fail = false;
bool transmit_openlcb_msg_called = false;
bool try_free_current_and_pop_next_incoming_msg_called = false;
bool process_statemachine_called = false;
bool node_get_first_called = false;
bool node_get_next_called = false;
bool does_node_process_msg = false;
openlcb_node_t *node_get_first = nullptr;
openlcb_node_t *node_get_next = nullptr;
bool lock_node_list_called = false;
bool unlock_node_list_called = false;
bool allow_successful_transmit = true;
openlcb_node_t *openlcb_node_callback = nullptr;
openlcb_msg_t *openlcb_msg_incoming_callback = nullptr;
openlcb_msg_t *openlcb_msg_outgoing_callback = nullptr;

node_parameters_t _node_parameters_main_node = {

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Test",
    .snip.model = "Test Model J",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (PSI_DATAGRAM |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .configuration_options.high_address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,

    .configuration_options.read_from_manufacturer_space_0xfc_supported = 1,
    .configuration_options.read_from_user_space_0xfb_supported = 1,
    .configuration_options.stream_read_write_supported = 0,
    .configuration_options.unaligned_reads_supported = 1,
    .configuration_options.unaligned_writes_supported = 1,
    .configuration_options.write_to_user_space_0xfb_supported = 1,
    .configuration_options.write_under_mask_supported = 1,
    .configuration_options.description = "These are options that defined the memory space capabilities",

    // Space 0xFF
    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so
    //    make sure the CDI maps these 2 items to the first 128 bytes as well
    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 0,
    .address_space_configuration_definition.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,         // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 0x200, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
    .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0,       // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = ADDRESS_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,         // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    .cdi =
        {
            // </cdi>
        },

};

bool _ProtocolSnip_handle_simple_node_info_request(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolSnip_handle_simple_node_info_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolMessageNetwork_initialization_complete(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolMessageNetwork_initialization_complete_simple(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    fprintf(stderr, "\n_ProtocolMessageNetwork_handle_protocol_support_inquiry = called\n");

    handler_mti = incoming_msg->mti;

    if (reply_to_protocol_support_inquiry)
    {

        fprintf(stderr, "\nreply_to_protocol_support_inquiry = true\n");

        outgoing_msg->mti = MTI_PROTOCOL_SUPPORT_REPLY;
    }

    return true;
}

bool _ProtocolMessageNetwork_handle_protocol_support_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolMessageNetwork_handle_verified_node_id(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolMessageNetwork_handle_terminate_due_to_error(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_consumer_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_consumer_identify_range(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_consumer_identified_set(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_consumer_identified_clear(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_producer_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_producer_identify_range(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_producer_identified_unknown(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_producer_identified_set(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_producer_identified_clear(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_producer_identified_reserved(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_identify_dest(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_event_learn(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_pc_event_report(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolTractionControl_command(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolTractionControl_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolSimpleTrainNodeIdentInfo_request(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolSimpleTrainNodeIdentInfo_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolDatagram_handle_datagram(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _Protocol_Datagram_handle_datagram_ok_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolDatagram_handle_datagram_rejected_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

void _OpenLcbUtilities_load_interaction_rejected(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    load_interaction_rejected_called = true;
}

bool _ProtocolStream_initiate_request(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolStream_initiate_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolStream_send_data(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolStream_data_proceed(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

bool _ProtocolStream_data_complete(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    handler_mti = incoming_msg->mti;

    return true;
}

void _ExampleDrivers_lock_can_buffer_fifo(void)
{
}

void _ExampleDrivers_unlock_can_buffer_fifo(void)
{
}

openlcb_node_t *_OpenLcbNode_get_first(uint8_t key)
{
    openlcb_node_t *result = OpenLcbNode_get_first(key);

    node_get_first_called = true;
    node_get_first = result;

    return result;
}
openlcb_node_t *_OpenLcbNode_get_next(uint8_t key)
{

    openlcb_node_t *result = OpenLcbNode_get_next(key);

    node_get_next_called = true;
    node_get_next = result;

    return result;
}

bool _CanTxStatemachine_transmit_openlcb_message(openlcb_msg_t *openlcb_msg)
{
    transmit_openlcb_msg_called = true;

    if (allow_successful_transmit)
    {
        fprintf(stderr, "CanTxStatemachine_transmit_openlcb_message, YES\n");

        return true;
    }
    else
    {

        fprintf(stderr, "CanTxStatemachine_transmit_openlcb_message, NO\n");

        return false;
    }
}

bool _OpenLcbMainStatemachine_process_main_statemachine(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    bool result = false;

    if (!force_process_statemachine_to_fail)
    {

        result = OpenLcbMainStatemachine_process_main_statemachine(openlcb_node, incoming_msg, outgoing_msg);
    }

    openlcb_node_callback = openlcb_node;
    openlcb_msg_incoming_callback = incoming_msg;
    openlcb_msg_outgoing_callback = outgoing_msg;
    process_statemachine_called = true;

    fprintf(stderr, "OpenLcbMainStatemachine_process_main_statemachine: %d, force_process_statemachine_to_fail: %d\n", result, force_process_statemachine_to_fail);

    return result;
}

bool _OpenLcbMainStatemachine_does_node_process_msg(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg)
{

    bool result = OpenLcbMainStatemachine_does_node_process_msg(openlcb_node, openlcb_msg);

    does_node_process_msg = result;

    fprintf(stderr, "OpenLcbMainStatemachine_does_node_process_msg: %d\n", result);

    return result;
}

openlcb_msg_t *_OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(openlcb_msg_t *active_incoming_msg)
{

    openlcb_msg_t *result = OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(active_incoming_msg);

    fprintf(stderr, "OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg: %p\n", result);

    try_free_current_and_pop_next_incoming_msg_called = true;

    return result;
}

void lock_node_list(void)
{

    lock_node_list_called = true;
}

void unlock_node_list(void)
{

    unlock_node_list_called = true;
}

const interface_openlcb_main_statemachine_t interface_openlcb_main_statemachine = {

    // Optinal handlers
    .snip_simple_node_info_request = &_ProtocolSnip_handle_simple_node_info_request,
    .snip_simple_node_info_reply = &_ProtocolSnip_handle_simple_node_info_request,

    .message_network_initialization_complete = &_ProtocolMessageNetwork_initialization_complete,
    .message_network_initialization_complete_simple = &_ProtocolMessageNetwork_initialization_complete_simple,
    .message_network_verify_node_id_addressed = &_ProtocolMessageNetwork_handle_verify_node_id_addressed,
    .message_network_verify_node_id_global = &_ProtocolMessageNetwork_handle_verify_node_id_global,
    .message_network_verified_node_id_addressed = &_ProtocolMessageNetwork_handle_verified_node_id,
    .message_network_optional_interaction_rejected = &_ProtocolMessageNetwork_handle_optional_interaction_rejected,
    .message_network_terminate_due_to_error = &_ProtocolMessageNetwork_handle_terminate_due_to_error,

    .message_network_protocol_support_inquiry = &_ProtocolMessageNetwork_handle_protocol_support_inquiry,
    .message_network_protocol_support_reply = &_ProtocolMessageNetwork_handle_protocol_support_reply,

    .event_transport_consumer_identify = &_ProtocolEventTransport_handle_consumer_identify,
    .event_transport_consumer_identify_range = &_ProtocolEventTransport_handle_consumer_identify_range,
    .event_transport_consumer_identified_unknown = &_ProtocolEventTransport_handle_consumer_identified_unknown,
    .event_transport_consumer_identified_set = &_ProtocolEventTransport_handle_consumer_identified_set,
    .event_transport_consumer_identified_clear = &_ProtocolEventTransport_handle_consumer_identified_clear,
    .event_transport_consumer_identified_reserved = &_ProtocolEventTransport_handle_consumer_identified_reserved,
    .event_transport_producer_identify = &_ProtocolEventTransport_handle_producer_identify,
    .event_transport_producer_identify_range = &_ProtocolEventTransport_handle_producer_identify_range,
    .event_transport_producer_identified_unknown = &_ProtocolEventTransport_handle_producer_identified_unknown,
    .event_transport_producer_identified_set = &_ProtocolEventTransport_handle_producer_identified_set,
    .event_transport_producer_identified_clear = &_ProtocolEventTransport_handle_producer_identified_clear,
    .event_transport_producer_identified_reserved = &_ProtocolEventTransport_handle_producer_identified_reserved,
    .event_transport_identify_dest = &_ProtocolEventTransport_handle_identify_dest,
    .event_transport_identify = &_ProtocolEventTransport_handle_identify,
    .event_transport_learn = &_ProtocolEventTransport_handle_event_learn,
    .event_transport_pc_report = &_ProtocolEventTransport_handle_pc_event_report,
    .event_transport_pc_report_with_payload = &_ProtocolEventTransport_handle_pc_event_report_with_payload,

    .traction_control_command = &_ProtocolTractionControl_command,
    .traction_control_reply = &_ProtocolTractionControl_reply,

    .simple_train_node_ident_info_request = &_ProtocolSimpleTrainNodeIdentInfo_request,
    .simple_train_node_ident_info_reply = &_ProtocolSimpleTrainNodeIdentInfo_reply,

    .datagram = &_ProtocolDatagram_handle_datagram,
    .datagram_ok_reply = &_Protocol_Datagram_handle_datagram_ok_reply,
    .datagram_rejected_reply = &_ProtocolDatagram_handle_datagram_rejected_reply,
    .load_interaction_rejected = &_OpenLcbUtilities_load_interaction_rejected,

    .stream_initiate_request = &_ProtocolStream_initiate_request,
    .stream_initiate_reply = &_ProtocolStream_initiate_reply,
    .stream_send_data = &_ProtocolStream_send_data,
    .stream_data_proceed = &_ProtocolStream_data_proceed,
    .stream_data_complete = &_ProtocolStream_data_complete,

    // required functions
    .node_get_first = &_OpenLcbNode_get_first,
    .node_get_next = &_OpenLcbNode_get_next,
    .transmit_openlcb_msg = &_CanTxStatemachine_transmit_openlcb_message,
    .lock_openlcb_buffer_fifo = &_ExampleDrivers_lock_can_buffer_fifo,
    .unlock_openlcb_buffer_fifo = &_ExampleDrivers_unlock_can_buffer_fifo,

    // use the internal default functions
    .process_main_statemachine = _OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = _OpenLcbMainStatemachine_does_node_process_msg,
    .try_free_current_and_pop_next_incoming_msg = _OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg};

const interface_openlcb_main_statemachine_t interface_openlcb_main_statemachine_null_handlers = {

    // Optinal handlers
    .snip_simple_node_info_request = nullptr,
    .snip_simple_node_info_reply = nullptr,

    .message_network_initialization_complete = nullptr,
    .message_network_initialization_complete_simple = nullptr,
    .message_network_verify_node_id_addressed = nullptr,
    .message_network_verify_node_id_global = nullptr,
    .message_network_verified_node_id_addressed = nullptr,
    .message_network_optional_interaction_rejected = nullptr,
    .message_network_terminate_due_to_error = nullptr,

    .message_network_protocol_support_inquiry = nullptr,
    .message_network_protocol_support_reply = nullptr,

    .event_transport_consumer_identify = nullptr,
    .event_transport_consumer_identify_range = nullptr,
    .event_transport_consumer_identified_unknown = nullptr,
    .event_transport_consumer_identified_set = nullptr,
    .event_transport_consumer_identified_clear = nullptr,
    .event_transport_consumer_identified_reserved = nullptr,
    .event_transport_producer_identify = nullptr,
    .event_transport_producer_identify_range = nullptr,
    .event_transport_producer_identified_unknown = nullptr,
    .event_transport_producer_identified_set = nullptr,
    .event_transport_producer_identified_clear = nullptr,
    .event_transport_producer_identified_reserved = nullptr,
    .event_transport_identify_dest = nullptr,
    .event_transport_identify = nullptr,
    .event_transport_learn = nullptr,
    .event_transport_pc_report = nullptr,
    .event_transport_pc_report_with_payload = nullptr,

    .traction_control_command = nullptr,
    .traction_control_reply = nullptr,

    .simple_train_node_ident_info_request = nullptr,
    .simple_train_node_ident_info_reply = nullptr,

    .datagram = nullptr,
    .datagram_ok_reply = nullptr,
    .datagram_rejected_reply = nullptr,

    .stream_initiate_request = nullptr,
    .stream_initiate_reply = nullptr,
    .stream_send_data = nullptr,
    .stream_data_proceed = nullptr,
    .stream_data_complete = nullptr,

    // required functions
    .node_get_first = &_OpenLcbNode_get_first,
    .node_get_next = &_OpenLcbNode_get_next,
    .transmit_openlcb_msg = &_CanTxStatemachine_transmit_openlcb_message,
    .lock_openlcb_buffer_fifo = &_ExampleDrivers_lock_can_buffer_fifo,
    .unlock_openlcb_buffer_fifo = &_ExampleDrivers_unlock_can_buffer_fifo,
    .load_interaction_rejected = &_OpenLcbUtilities_load_interaction_rejected,

    // use the internal default functions
    .process_main_statemachine = _OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = _OpenLcbMainStatemachine_does_node_process_msg,
    .try_free_current_and_pop_next_incoming_msg = _OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg};

interface_openlcb_node_t interface_openlcb_node = {

    .lock_node_list = &lock_node_list,
    .unlock_node_list = &unlock_node_list};

void _reset_variables(void)
{

    load_interaction_rejected_called = false;
    reply_to_protocol_support_inquiry = false;
    transmit_openlcb_msg_called = false;
    try_free_current_and_pop_next_incoming_msg_called = false;
    process_statemachine_called = false;
    node_get_first_called = false;
    node_get_next_called = false;
    does_node_process_msg = false;
    lock_node_list_called = false;
    unlock_node_list_called = false;
    allow_successful_transmit = true;
    openlcb_node_callback = nullptr;
    openlcb_msg_incoming_callback = nullptr;
    openlcb_msg_outgoing_callback = nullptr;
    node_get_first = nullptr;
    node_get_next = nullptr;
    force_process_statemachine_to_fail = false;
    handler_mti = 0x00;
}

void _global_initialize(void)
{

    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_null_handlers(void)
{

    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine_null_handlers);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

#define SOURCE_ALIAS 0x222
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0xBBB
#define DEST_ID 0x060504030201

TEST(OpenLcbMainStatemachine, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(OpenLcbMainStatemachine, does_node_process_msg)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Initization Complete but node not in initialized state
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x100, 0);
        EXPECT_FALSE(OpenLcbMainStatemachine_does_node_process_msg(node1, openlcb_msg));

        // Initization Complete but node in initialized state
        node1->state.initalized = true;
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x100, 0);
        EXPECT_TRUE(OpenLcbMainStatemachine_does_node_process_msg(node1, openlcb_msg));

        // Verify ID Addressed to us
        node1->state.initalized = true;
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x488, 0);
        EXPECT_TRUE(OpenLcbMainStatemachine_does_node_process_msg(node1, openlcb_msg));

        // Verify ID not Addressed to us
        node1->state.initalized = true;
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS + 1, DEST_ID + 1, 0x488, 0);
        EXPECT_FALSE(OpenLcbMainStatemachine_does_node_process_msg(node1, openlcb_msg));

        // Datagram for us
        node1->state.initalized = true;
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x1C48, 0);
        EXPECT_TRUE(OpenLcbMainStatemachine_does_node_process_msg(node1, openlcb_msg));

        // Datagram not for us
        node1->state.initalized = true;
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS + 2, DEST_ID + 2, 0x1C48, 0);
        EXPECT_FALSE(OpenLcbMainStatemachine_does_node_process_msg(node1, openlcb_msg));

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbMainStatemachine, try_free_current_and_pop_next_incoming_msg)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *openlcb_msg2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *active_msg = nullptr;

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg1, nullptr);
    EXPECT_NE(openlcb_msg2, nullptr);

    if (openlcb_msg1 && openlcb_msg2)
    {
        // pass in a null and the FIFO is empty
        active_msg = OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(active_msg);
        EXPECT_EQ(active_msg, nullptr);

        OpenLcbBufferFifo_push(openlcb_msg1);
        EXPECT_FALSE(OpenLcbBufferFifo_is_empty());

        // pass in a null and the FIFO contains 1 message
        active_msg = OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(active_msg);
        EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
        EXPECT_EQ(active_msg, openlcb_msg1);

        // pass in a message and the FIFO is empty
        EXPECT_TRUE(openlcb_msg1->state.allocated);
        EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
        active_msg = OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(openlcb_msg1);
        EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
        EXPECT_EQ(active_msg, nullptr);
        EXPECT_FALSE(openlcb_msg1->state.allocated);

        openlcb_msg_t *openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);

        // pass in a message and the FIFO has 1
        EXPECT_TRUE(openlcb_msg1->state.allocated);
        EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
        OpenLcbBufferFifo_push(openlcb_msg1);
        EXPECT_FALSE(OpenLcbBufferFifo_is_empty());

        active_msg = OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(openlcb_msg2);
        EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
        EXPECT_EQ(active_msg, openlcb_msg1);
        EXPECT_FALSE(openlcb_msg2->state.allocated);

        OpenLcbBufferStore_free_buffer(openlcb_msg1);
        // openlcb_msg2 has been freed already
    }
}

TEST(OpenLcbMainStatemachine, run)
{

    _reset_variables();
    _global_initialize();

    // ************************************************************************
    // empty Node List
    // ************************************************************************
    OpenLcbMainStatemachine_run();
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_TRUE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_FALSE(node_get_next_called);
    _reset_variables();
    // ************************************************************************

    // ************************************************************************
    // empty Node List but messages are coming in
    // ************************************************************************

    fprintf(stderr, "\n****************** no node: one message ********************\n");

    openlcb_msg_t *openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x100, 0); // Initization Complete
    OpenLcbBufferFifo_push(openlcb_msg1);

    OpenLcbMainStatemachine_run();
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());                   // should have popped it
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1); // but not freed it yet
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_TRUE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_FALSE(node_get_next_called); // should have called but should not have found anything
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_EQ(node_get_next, nullptr);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0); // it should be freed
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_TRUE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_EQ(node_get_next, nullptr);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->state.initalized = true;
    node1->alias = 0xAAA;

    // ************************************************************************
    //  Node List has one node and messages are coming in
    // ************************************************************************
    openlcb_msg_t *openlcb_msg2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg2, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg2, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x100, 0); // Initization Complete
    OpenLcbBufferFifo_push(openlcb_msg2);

    fprintf(stderr, "\n****************** one node: one message ********************\n");
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());                   // should have popped it
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1); // but not freed it yet
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, node1);
    EXPECT_EQ(node_get_next, nullptr);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_FALSE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_TRUE(node_get_next_called); // should have called but should not have found anything
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_EQ(node_get_next, nullptr);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    _reset_variables();
    OpenLcbMainStatemachine_run();

    fprintf(stderr, "\n****************** one node: no message  ******************\n");
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0); // it should be freed
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_TRUE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_EQ(node_get_next, nullptr);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    _reset_variables();
    OpenLcbMainStatemachine_run();

    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x100, 0); // Initization Complete
    OpenLcbBufferFifo_push(openlcb_msg1);

    fprintf(stderr, "\n****************** one node: one message  ******************\n");
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_TRUE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, node1);
    EXPECT_EQ(node_get_next, nullptr);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    _reset_variables();
    OpenLcbMainStatemachine_run();

    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040599, &_node_parameters_main_node);
    node2->state.initalized = true;
    node2->alias = 0xBBB;

    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x488, 0); // Verify Node ID addressed
    OpenLcbBufferFifo_push(openlcb_msg1);

    fprintf(stderr, "\n****************** two nodes: one message addressed to the second node  ******************\n");
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_TRUE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(does_node_process_msg);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, node1);
    EXPECT_EQ(node_get_next, nullptr);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_FALSE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_TRUE(does_node_process_msg);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_TRUE(node_get_next_called);
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_EQ(node_get_next, node2);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x488, 0); // Verify Node ID addressed
    OpenLcbBufferFifo_push(openlcb_msg1);

    fprintf(stderr, "\n****************** two nodes: one message addressed to the second node process statemachine fails  ******************\n");

    // Pops the next message and tries the first node in the list but it is not for that node
    fprintf(stderr, "pop message and test first node in the list...\n");
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_TRUE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(does_node_process_msg);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, node1);
    EXPECT_EQ(node_get_next, nullptr);

    // gets the second node in the list but the process statemachine fails
    fprintf(stderr, "test second node in the list, message is for this node but Process Main Statemachine fails...\n");
    _reset_variables();
    force_process_statemachine_to_fail = true;
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_FALSE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_TRUE(does_node_process_msg);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_TRUE(node_get_next_called);
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_EQ(node_get_next, node2);

    fprintf(stderr, "should have set reprocess and this is a direct call to Process Statemachine (that still should fail)...\n");
    _reset_variables();
    force_process_statemachine_to_fail = true;
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_FALSE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(does_node_process_msg);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_EQ(node_get_next, nullptr);
    EXPECT_TRUE(process_statemachine_called);

    // The reprocess_active_node flag should be cleared and the same node should should succeed
    _reset_variables();
    force_process_statemachine_to_fail = false;
    fprintf(stderr, "should have set reprocess and this is a direct call to Process Statemachine that now succeeds...\n");
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_FALSE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(does_node_process_msg);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_EQ(node_get_next, nullptr);
    EXPECT_TRUE(process_statemachine_called);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    _reset_variables();
    OpenLcbMainStatemachine_run();

    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS + 1, DEST_ID, 0x488, 0); // Verify Node ID addressed
    OpenLcbBufferFifo_push(openlcb_msg1);

    fprintf(stderr, "\n****************** two nodes: one message addressed to neither ******************\n");
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_TRUE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(does_node_process_msg);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_FALSE(node_get_next_called);
    EXPECT_EQ(node_get_first, node1);
    EXPECT_EQ(node_get_next, nullptr);
    EXPECT_FALSE(process_statemachine_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_FALSE(try_free_current_and_pop_next_incoming_msg_called);
    EXPECT_FALSE(does_node_process_msg);
    EXPECT_FALSE(node_get_first_called);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    EXPECT_TRUE(node_get_next_called);
    EXPECT_EQ(node_get_first, nullptr);
    EXPECT_FALSE(process_statemachine_called);
    EXPECT_EQ(node_get_next, node2);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

    // ************************************************************************

    // ************************************************************************
    // One in Node List
    // ************************************************************************

    //  OpenLcbMainStatemachine_run();
    //  ************************************************************************

    // TODO: as of Aug 23 this function does nothing
}

TEST(OpenLcbMainStatemachine, null_handlers)
{

    _reset_variables();
    _global_initialize_null_handlers();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->state.initalized = true;
    node1->alias = 0xBBB;

    // ************************************************************************
    // MTI_SIMPLE_NODE_INFO_REQUEST
    // ************************************************************************
    openlcb_msg_t *openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_TRUE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_SIMPLE_NODE_INFO_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(SNIP);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_INITIALIZATION_COMPLETE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_INITIALIZATION_COMPLETE_SIMPLE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE_SIMPLE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_VERIFY_NODE_ID_ADDRESSED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_ADDRESSED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_VERIFY_NODE_ID_GLOBAL
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_VERIFIED_NODE_ID
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFIED_NODE_ID, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_OPTIONAL_INTERACTION_REJECTED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_OPTIONAL_INTERACTION_REJECTED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_TERMINATE_DO_TO_ERROR
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DO_TO_ERROR, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PROTOCOL_SUPPORT_INQUIRY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PROTOCOL_SUPPORT_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFY_RANGE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY_RANGE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFY_RANGE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_UNKNOWN, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFIED_SET
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_SET, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFIED_CLEAR
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_CLEAR, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFIED_RESERVED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_RESERVED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFY_RANGE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFY_RANGE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFY_RANGE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_UNKNOWN, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFIED_SET
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_SET, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFIED_CLEAR
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_CLEAR, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFIED_RESERVED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_RESERVED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_EVENTS_IDENTIFY_DEST
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY_DEST, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_EVENTS_IDENTIFY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_EVENT_LEARN
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENT_LEARN, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PC_EVENT_REPORT
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PC_EVENT_REPORT_WITH_PAYLOAD
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT_WITH_PAYLOAD, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_TRACTION_PROTOCOL
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TRACTION_PROTOCOL, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_TRUE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_TRACTION_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TRACTION_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_SIMPLE_TRAIN_INFO_REQUEST
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_TRAIN_INFO_REQUEST, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_TRUE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_SIMPLE_TRAIN_INFO_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_TRAIN_INFO_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_DATAGRAM
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_DATAGRAM_OK_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM_OK_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_DATAGRAM_REJECTED_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM_REJECTED_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_DATAGRAM_REJECTED_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM_REJECTED_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_INIT_REQUEST
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REQUEST, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_INIT_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_SEND
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_PROCEED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_PROCEED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_COMPLETE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_COMPLETE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, 0x00);
    EXPECT_FALSE(load_interaction_rejected_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************
}

TEST(OpenLcbMainStatemachine, callback_handlers)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->state.initalized = true;
    node1->alias = 0xBBB;

    // ************************************************************************
    // MTI_SIMPLE_NODE_INFO_REQUEST
    // ************************************************************************
    openlcb_msg_t *openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_SIMPLE_NODE_INFO_REQUEST);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_SIMPLE_NODE_INFO_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(SNIP);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_SIMPLE_NODE_INFO_REPLY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_INITIALIZATION_COMPLETE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_INITIALIZATION_COMPLETE);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_INITIALIZATION_COMPLETE_SIMPLE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE_SIMPLE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_INITIALIZATION_COMPLETE_SIMPLE);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_VERIFY_NODE_ID_ADDRESSED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_ADDRESSED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_VERIFY_NODE_ID_ADDRESSED);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_VERIFY_NODE_ID_GLOBAL
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_VERIFY_NODE_ID_GLOBAL);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_VERIFIED_NODE_ID
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFIED_NODE_ID, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_VERIFIED_NODE_ID);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_OPTIONAL_INTERACTION_REJECTED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_OPTIONAL_INTERACTION_REJECTED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_OPTIONAL_INTERACTION_REJECTED);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_TERMINATE_DO_TO_ERROR
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DO_TO_ERROR, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_TERMINATE_DO_TO_ERROR);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PROTOCOL_SUPPORT_INQUIRY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PROTOCOL_SUPPORT_INQUIRY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PROTOCOL_SUPPORT_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PROTOCOL_SUPPORT_REPLY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_CONSUMER_IDENTIFY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFY_RANGE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY_RANGE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_CONSUMER_IDENTIFY_RANGE);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFY_RANGE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_UNKNOWN, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFIED_SET
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_SET, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_CONSUMER_IDENTIFIED_SET);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFIED_CLEAR
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_CLEAR, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_CONSUMER_IDENTIFIED_CLEAR);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_CONSUMER_IDENTIFIED_RESERVED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_RESERVED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_CONSUMER_IDENTIFIED_RESERVED);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PRODUCER_IDENTIFY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFY_RANGE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFY_RANGE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PRODUCER_IDENTIFY_RANGE);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFY_RANGE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_UNKNOWN, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFIED_SET
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_SET, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PRODUCER_IDENTIFIED_SET);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFIED_CLEAR
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_CLEAR, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PRODUCER_IDENTIFIED_CLEAR);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PRODUCER_IDENTIFIED_RESERVED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_RESERVED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PRODUCER_IDENTIFIED_RESERVED);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_EVENTS_IDENTIFY_DEST
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY_DEST, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_EVENTS_IDENTIFY_DEST);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_EVENTS_IDENTIFY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_EVENTS_IDENTIFY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_EVENT_LEARN
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENT_LEARN, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_EVENT_LEARN);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PC_EVENT_REPORT
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PC_EVENT_REPORT);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_PC_EVENT_REPORT_WITH_PAYLOAD
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT_WITH_PAYLOAD, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PC_EVENT_REPORT_WITH_PAYLOAD);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_TRACTION_PROTOCOL
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TRACTION_PROTOCOL, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_TRACTION_PROTOCOL);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_TRACTION_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TRACTION_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_TRACTION_REPLY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_SIMPLE_TRAIN_INFO_REQUEST
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_TRAIN_INFO_REQUEST, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_SIMPLE_TRAIN_INFO_REQUEST);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_SIMPLE_TRAIN_INFO_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_TRAIN_INFO_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_SIMPLE_TRAIN_INFO_REPLY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_DATAGRAM
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_DATAGRAM);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_DATAGRAM_OK_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM_OK_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_DATAGRAM_OK_REPLY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_DATAGRAM_REJECTED_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM_REJECTED_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_DATAGRAM_REJECTED_REPLY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_DATAGRAM_REJECTED_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM_REJECTED_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_DATAGRAM_REJECTED_REPLY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_INIT_REQUEST
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REQUEST, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_STREAM_INIT_REQUEST);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_INIT_REPLY
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_STREAM_INIT_REPLY);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_SEND
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_STREAM_SEND);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_PROCEED
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_PROCEED, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_STREAM_PROCEED);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************

    // ************************************************************************
    // MTI_STREAM_COMPLETE
    // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_COMPLETE, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_STREAM_COMPLETE);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    // ************************************************************************
}

TEST(OpenLcbMainStatemachine, handler_returns_openlcb_msg_to_transmit)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->state.initalized = true;
    node1->alias = 0xBBB;

    // // ************************************************************************
    // // MTI_PROTOCOL_SUPPORT_INQUIRY
    // // ************************************************************************
    openlcb_msg_t *openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    reply_to_protocol_support_inquiry = true;
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PROTOCOL_SUPPORT_INQUIRY);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_TRUE(transmit_openlcb_msg_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_FALSE(transmit_openlcb_msg_called);

    // // ************************************************************************
    // // MTI_PROTOCOL_SUPPORT_INQUIRY
    // // ************************************************************************
    openlcb_msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg1, nullptr);
    OpenLcbUtilities_load_openlcb_message(openlcb_msg1, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY, 0);
    OpenLcbBufferFifo_push(openlcb_msg1);

    _reset_variables();
    reply_to_protocol_support_inquiry = true;
    OpenLcbMainStatemachine_run();
    EXPECT_EQ(handler_mti, MTI_PROTOCOL_SUPPORT_INQUIRY);
    EXPECT_FALSE(transmit_openlcb_msg_called);
    _reset_variables();
    allow_successful_transmit = false;
    OpenLcbMainStatemachine_run();
    EXPECT_TRUE(transmit_openlcb_msg_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_TRUE(transmit_openlcb_msg_called);
    _reset_variables();
    OpenLcbMainStatemachine_run();
    EXPECT_FALSE(transmit_openlcb_msg_called);
    // ************************************************************************
}
