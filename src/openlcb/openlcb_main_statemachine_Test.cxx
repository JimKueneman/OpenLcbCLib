#include "test/main_Test.hxx"

#include "openlcb_main_statemachine.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

bool lock_node_list_called = false;
bool unlock_node_list_called = false;
bool allow_successful_transmit = true;

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

    return true;
}

bool _ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    return true;
}

bool _ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    return true;
}

bool _ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    return true;
}

bool _ProtocolMessageNetwork_handle_verified_node_id(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    return true;
}

bool _ProtocolEventTransport_handle_consumer_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_consumer_identify_range(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_consumer_identified_set(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_consumer_identified_clear(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_producer_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_producer_identify_range(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_producer_identified_unknown(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_producer_identified_set(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_producer_identified_clear(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_producer_identified_reserved(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_identify_dest(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_event_learn(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_pc_event_report(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolDatagram_handle_datagram(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _Protocol_Datagram_handle_datagram_ok_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolDatagram_handle_datagram_rejected_reply(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
    return true;
}

bool _ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    return true;
}

void _ProtocolMessageNetwork_send_interaction_rejected(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{
}

void _ExampleDrivers_lock_can_buffer_fifo(void)
{
}

void _ExampleDrivers_unlock_can_buffer_fifo(void)
{
}

openlcb_node_t *_OpenLcbNode_get_first(uint8_t key)
{

    return OpenLcbNode_get_first(key);
}
openlcb_node_t *_OpenLcbNode_get_next(uint8_t key)
{

    return OpenLcbNode_get_next(key);
}

bool _CanTxStatemachine_transmit_openlcb_message(openlcb_msg_t *openlcb_msg)
{

    if (allow_successful_transmit)
    {
        return true;
    }
    else
    {

        return false;
    }
}

bool _OpenLcbMainStatemachine_process_main_statemachine(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg)
{

    return OpenLcbMainStatemachine_process_main_statemachine(openlcb_node, incoming_msg, outgoing_msg);
}

bool _OpenLcbMainStatemachine_does_node_process_msg(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg)
{

    return OpenLcbMainStatemachine_does_node_process_msg(openlcb_node, openlcb_msg);
}

openlcb_msg_t *_OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(openlcb_msg_t *active_incoming_msg)
{

    return OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(active_incoming_msg);
}

bool _OpenLcbMainStatemachine_try_reprocess_active_node(openlcb_node_t *active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg)
{

    return OpenLcbMainStatemachine_try_reprocess_active_node(active_node, active_incoming_msg, active_outgoing_msg);
}

bool _OpenLcbMainStatemachine_process_node(openlcb_node_t *active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg)
{

    return OpenLcbMainStatemachine_process_node(active_node, active_incoming_msg, active_outgoing_msg);
}

bool _OpenLcbMainStatemachine_try_process_first_node(openlcb_node_t **active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg)
{

    return OpenLcbMainStatemachine_try_process_first_node(active_node, active_incoming_msg, active_outgoing_msg);
}

bool _OpenLcbMainStatemachine_try_process_next_node(openlcb_node_t **active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg)
{

    return OpenLcbMainStatemachine_try_process_next_node(active_node, active_incoming_msg, active_outgoing_msg);
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

    .snip_simple_node_info_request = &_ProtocolSnip_handle_simple_node_info_request,
    .message_network_protocol_support_inquiry = &_ProtocolMessageNetwork_handle_protocol_support_inquiry,
    .message_network_verify_node_id_addressed = &_ProtocolMessageNetwork_handle_verify_node_id_addressed,
    .message_network_verify_node_id_global = &_ProtocolMessageNetwork_handle_verify_node_id_global,
    .message_network_verified_node_id_addressed = &_ProtocolMessageNetwork_handle_verified_node_id,
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
    .datagram = &_ProtocolDatagram_handle_datagram,
    .datagram_ok_reply = &_Protocol_Datagram_handle_datagram_ok_reply,
    .datagram_rejected_reply = &_ProtocolDatagram_handle_datagram_rejected_reply,
    .optional_interaction_rejected = &_ProtocolMessageNetwork_handle_optional_interaction_rejected,
    .send_interaction_rejected = &_ProtocolMessageNetwork_send_interaction_rejected,
    .node_get_first = &_OpenLcbNode_get_first,
    .node_get_next = &_OpenLcbNode_get_next,
    .transmit_openlcb_message = &_CanTxStatemachine_transmit_openlcb_message,
    .lock_openlcb_buffer_fifo = &_ExampleDrivers_lock_can_buffer_fifo,
    .unlock_openlcb_buffer_fifo = &_ExampleDrivers_unlock_can_buffer_fifo,

    // use the internal default functions
    .process_main_statemachine = _OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = _OpenLcbMainStatemachine_does_node_process_msg,
    .try_free_current_and_pop_next_incoming_msg = _OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg,
    .try_reprocess_active_node = _OpenLcbMainStatemachine_try_reprocess_active_node,
    .process_node = _OpenLcbMainStatemachine_process_node,
    .try_process_first_node = _OpenLcbMainStatemachine_try_process_first_node,
    .try_process_next_node = _OpenLcbMainStatemachine_try_process_next_node};

interface_openlcb_node_t interface_openlcb_node = {

    .lock_node_list = &lock_node_list,
    .unlock_node_list = &unlock_node_list};

void _reset_variables(void)
{

    lock_node_list_called = false;
    unlock_node_list_called = false;
    allow_successful_transmit = true;
}

void _global_initialize(void)
{

    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(OpenLcbMainStatemachine, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(OpenLcbMainStatemachine, does_node_process_msg)
{

#define SOURCE_ALIAS 0x455
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0x455
#define DEST_ID 0x060504030201

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

TEST(OpenLcbMainStatemachine, try_transmit_active_msg)
{

#define SOURCE_ALIAS 0x455
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0x455
#define DEST_ID 0x060504030201

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Initization Complete
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x100, 0);
        allow_successful_transmit = true;
        EXPECT_TRUE(OpenLcbMainStatemachine_try_transmit_active_msg(openlcb_msg));

        // Initization Complete
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, 0x100, 0);
        allow_successful_transmit = false;
        EXPECT_FALSE(OpenLcbMainStatemachine_try_transmit_active_msg(openlcb_msg));
        allow_successful_transmit = true;
        EXPECT_TRUE(OpenLcbMainStatemachine_try_transmit_active_msg(openlcb_msg));

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbMainStatemachine, try_free_current_and_pop_next_incoming_msg)
{

#define SOURCE_ALIAS 0x455
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0x455
#define DEST_ID 0x060504030201

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
    // OpenLcbMainStatemachine_run();
    // ************************************************************************

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0xAAA;

    // ************************************************************************
    // One in Node List
    // ************************************************************************

    //  OpenLcbMainStatemachine_run();
    //  ************************************************************************

    // TODO: as of Aug 23 this function does nothing
}

TEST(OpenLcbMainStatemachine, run_single_node)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0xAAA;

    //  EXPECT_NE(node1, nullptr);

    // OpenLcbMainStatemachine_run_single_node(node1);

    // // TODO: as of Aug 23 this function does nothing
}
