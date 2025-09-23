#include "test/main_Test.hxx"

#include "protocol_event_transport.h"

#include "protocol_message_network.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

#define AUTO_CREATE_EVENT_COUNT 10
#define DEST_EVENT_ID 0x0605040302010000
#define SOURCE_ALIAS 0x222
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0xBBB
#define DEST_ID 0x060504030201
#define SNIP_NAME_FULL "0123456789012345678901234567890123456789"
#define SNIP_MODEL "Test Model J"

#define CONFIG_MEM_START_ADDRESS 0x100
#define CONFIG_MEM_NODE_ADDRESS_ALLOCATION 0x200

bool lock_node_list_called = false;
bool unlock_node_list_called = false;

bool on_consumer_range_identified_called = false;
bool on_consumer_identified_unknown_called = false;
bool on_consumer_identified_set_called = false;
bool on_consumer_identified_clear_called = false;
bool on_consumer_identified_reserved_called = false;

bool on_producer_range_identified_called = false;
bool on_producer_identified_unknown_called = false;
bool on_producer_identified_set_called = false;
bool on_producer_identified_clear_called = false;
bool on_producer_identified_reserved_called = false;

bool on_event_learn_called = false;
bool on_pc_event_report_called = false;
bool on_pc_event_report_with_payload_called = false;

uint16_t event_with_payload_count;
event_payload_t event_with_payload;

node_parameters_t _node_parameters_main_node = {

    .consumer_count_autocreate = AUTO_CREATE_EVENT_COUNT,
    .producer_count_autocreate = AUTO_CREATE_EVENT_COUNT,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = SNIP_NAME_FULL,
    .snip.model = SNIP_MODEL,
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (PSI_DATAGRAM |
                         PSI_FIRMWARE_UPGRADE |
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
    .address_space_config_memory.low_address_valid = 0,                                // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,                                      // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = CONFIG_MEM_NODE_ADDRESS_ALLOCATION, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
    .address_space_firmware.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_firmware.low_address = 0,         // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = ADDRESS_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
        },

};

void lock_node_list(void)
{

    lock_node_list_called = true;
}

void unlock_node_list(void)
{

    unlock_node_list_called = true;
}

interface_openlcb_node_t interface_openlcb_node = {

    .lock_node_list = &lock_node_list,
    .unlock_node_list = &unlock_node_list};

void _on_consumer_range_identified(openlcb_node_t *openlcb_node, event_id_t *event_id)
{

    on_consumer_range_identified_called = true;
}

void _on_consumer_identified_unknown(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_consumer_identified_unknown_called = true;
}

void _on_consumer_identified_set(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_consumer_identified_set_called = true;
}

void _on_consumer_identified_clear(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_consumer_identified_clear_called = true;
}

void _on_consumer_identified_reserved(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_consumer_identified_reserved_called = true;
}

void _on_producer_range_identified(openlcb_node_t *openlcb_node, event_id_t *event_id)
{

    on_producer_range_identified_called = true;
}

void _on_producer_identified_unknown(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_producer_identified_unknown_called = true;
}

void _on_producer_identified_set(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_producer_identified_set_called = true;
}

void _on_producer_identified_clear(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_producer_identified_clear_called = true;
}

void _on_producer_identified_reserved(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_producer_identified_reserved_called = true;
}

void _on_event_learn(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_event_learn_called = true;
}

void _on_pc_event_report(openlcb_node_t *openlcb_node, event_id_t *event_id)
{
    on_pc_event_report_called = true;
}

void _on_pc_event_report_with_payload(openlcb_node_t *node, event_id_t *event_id, uint16_t count, event_payload_t *payload)
{
    on_pc_event_report_with_payload_called = true;

    event_with_payload_count = count;

    for (int i = 0; i < sizeof(event_payload_t); i++)
    {

        event_with_payload[i] = (*payload)[i];
    }
}

interface_openlcb_protocol_event_transport_t interface_openlcb_protocol_event_transport = {

    .on_consumer_range_identified = &_on_consumer_range_identified,
    .on_consumer_identified_unknown = &_on_consumer_identified_unknown,
    .on_consumer_identified_set = &_on_consumer_identified_set,
    .on_consumer_identified_clear = &_on_consumer_identified_clear,
    .on_consumer_identified_reserved = &_on_consumer_identified_reserved,
    .on_producer_range_identified = &_on_producer_range_identified,
    .on_producer_identified_unknown = &_on_producer_identified_unknown,
    .on_producer_identified_set = &_on_producer_identified_set,
    .on_producer_identified_clear = &_on_producer_identified_clear,
    .on_producer_identified_reserved = &_on_producer_identified_reserved,
    .on_event_learn = &_on_event_learn,
    .on_pc_event_report = &_on_pc_event_report,
    .on_pc_event_report_with_payload = &_on_pc_event_report_with_payload};

interface_openlcb_protocol_event_transport_t interface_openlcb_protocol_event_transport_null_callbacks = {

    .on_consumer_range_identified = NULL,
    .on_consumer_identified_unknown = NULL,
    .on_consumer_identified_set = NULL,
    .on_consumer_identified_clear = NULL,
    .on_producer_range_identified = NULL,
    .on_producer_identified_unknown = NULL,
    .on_producer_identified_set = NULL,
    .on_producer_identified_clear = NULL,
    .on_event_learn = NULL,
    .on_pc_event_report = NULL,
    .on_pc_event_report_with_payload = NULL};

void _reset_variables(void)
{

    lock_node_list_called = false;
    unlock_node_list_called = false;
    on_consumer_identified_unknown_called = false;
    on_consumer_identified_set_called = false;
    on_consumer_identified_clear_called = false;
    on_consumer_identified_reserved_called = false;
    on_producer_identified_unknown_called = false;
    on_producer_identified_reserved_called = false;
    on_producer_identified_set_called = false;
    on_producer_identified_clear_called = false;
    on_event_learn_called = false;
    on_pc_event_report_called = false;
    on_pc_event_report_with_payload_called = false;
    on_producer_range_identified_called = false;
    on_consumer_range_identified_called = false;
    on_consumer_identified_reserved_called = false;
    on_producer_identified_reserved_called = false;

    event_with_payload_count = 0x00;
    for (int i = 0; i < sizeof(event_id_t); i++)
    {

        event_with_payload[i] = 0x00;
    }
}

void _global_initialize(void)
{

    ProtocolEventTransport_initialize(&interface_openlcb_protocol_event_transport);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_null_callbacks(void)
{

    ProtocolEventTransport_initialize(&interface_openlcb_protocol_event_transport_null_callbacks);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(ProtocolEventTransport, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(ProtocolEventTransport, handle_consumer_identify)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;

        // ********************************************************************
        // Consumer Identify event for us
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_consumer_identify(&statemachine_info);
        EXPECT_EQ(outgoing_msg->payload_count, 8);
        // TODO HOW TO SET/CLEAR/UNKNOWN and check for reply message type
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_EVENT_ID);
        // ********************************************************************

        // ********************************************************************
        // Consumer Identify event for us
        // ********************************************************************
        _reset_variables();
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID + AUTO_CREATE_EVENT_COUNT - 1);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_consumer_identify(&statemachine_info);
        EXPECT_EQ(outgoing_msg->payload_count, 8);
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_EVENT_ID + AUTO_CREATE_EVENT_COUNT - 1);
        // ********************************************************************

        // ********************************************************************
        // Consumer Identify event NOT for us
        // ********************************************************************
        _reset_variables();
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID + AUTO_CREATE_EVENT_COUNT);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_consumer_identify(&statemachine_info);
        EXPECT_EQ(outgoing_msg->payload_count, 0);
        EXPECT_EQ(outgoing_msg->mti, 0x00);
        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, handle_producer_identify)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;

        // ********************************************************************
        // Producer Identify event for us
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_producer_identify(&statemachine_info);
        EXPECT_EQ(outgoing_msg->payload_count, 8);
        // TODO HOW TO SET/CLEAR/UNKNOWN and check for reply message type
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_EVENT_ID);
        // ********************************************************************

        // ********************************************************************
        // Producer Identify event for us
        // ********************************************************************
        _reset_variables();
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID + AUTO_CREATE_EVENT_COUNT - 1);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_producer_identify(&statemachine_info);
        EXPECT_EQ(outgoing_msg->payload_count, 8);
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_EVENT_ID + AUTO_CREATE_EVENT_COUNT - 1);
        // ********************************************************************

        // ********************************************************************
        // Producer Identify event NOT for us
        // ********************************************************************
        _reset_variables();
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFY, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID + AUTO_CREATE_EVENT_COUNT);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_producer_identify(&statemachine_info);
        EXPECT_EQ(outgoing_msg->payload_count, 0);
        EXPECT_EQ(outgoing_msg->mti, 0x00);
        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, consumer_xxx_identified)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {
        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;

        // ********************************************************************
        // ProtocolEventTransport_handle_consumer_range_identified
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_RANGE_IDENTIFIED, 0);
        ProtocolEventTransport_handle_consumer_range_identified(&statemachine_info);
        EXPECT_TRUE(on_consumer_range_identified_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_consumer_identified_unknown
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_UNKNOWN, 0);
        ProtocolEventTransport_handle_consumer_identified_unknown(&statemachine_info);
        EXPECT_TRUE(on_consumer_identified_unknown_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_consumer_identified_set
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_SET, 0);
        ProtocolEventTransport_handle_consumer_identified_set(&statemachine_info);
        EXPECT_TRUE(on_consumer_identified_set_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_consumer_identified_clear
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_CLEAR, 0);
        ProtocolEventTransport_handle_consumer_identified_clear(&statemachine_info);
        EXPECT_TRUE(on_consumer_identified_clear_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_consumer_identified_reserved
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_RESERVED, 0);
        ProtocolEventTransport_handle_consumer_identified_reserved(&statemachine_info);
        EXPECT_TRUE(on_consumer_identified_reserved_called);
        _reset_variables();
        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, producer_xxx_identified)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;

        // ********************************************************************
        // ProtocolEventTransport_handle_producer_range_identified
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_RANGE_IDENTIFIED, 0);
        ProtocolEventTransport_handle_producer_range_identified(&statemachine_info);
        EXPECT_TRUE(on_producer_range_identified_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_producer_identified_unknown
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_UNKNOWN, 0);
        ProtocolEventTransport_handle_producer_identified_unknown(&statemachine_info);
        EXPECT_TRUE(on_producer_identified_unknown_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_producer_identified_set
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_SET, 0);
        ProtocolEventTransport_handle_producer_identified_set(&statemachine_info);
        EXPECT_TRUE(on_producer_identified_set_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_producer_identified_clear
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_CLEAR, 0);
        ProtocolEventTransport_handle_producer_identified_clear(&statemachine_info);
        EXPECT_TRUE(on_producer_identified_clear_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_producer_identified_reserved
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_RESERVED, 0);
        ProtocolEventTransport_handle_producer_identified_reserved(&statemachine_info);
        EXPECT_TRUE(on_producer_identified_reserved_called);
        _reset_variables();
        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, handle_event_learn)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;

        // ********************************************************************
        // ProtocolEventTransport_handle_event_learn
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENT_LEARN, 0);
        ProtocolEventTransport_handle_event_learn(&statemachine_info);
        EXPECT_TRUE(on_event_learn_called);
        _reset_variables();
        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, handle_pc_event_report)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(SNIP);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;

        // ********************************************************************
        // ProtocolEventTransport_handle_pc_event_report
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT, 0);
        ProtocolEventTransport_handle_pc_event_report(&statemachine_info);
        EXPECT_TRUE(on_pc_event_report_called);
        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_pc_event_report_with_payload
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT_WITH_PAYLOAD, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);

        for (int i = sizeof(event_id_t); i < LEN_EVENT_PAYLOAD + sizeof(event_id_t); i++)
        {

            *openlcb_msg->payload[i] = i - sizeof(event_id_t);
        }

        openlcb_msg->payload_count = LEN_EVENT_PAYLOAD + sizeof(event_id_t);

        ProtocolEventTransport_handle_pc_event_report_with_payload(&statemachine_info);
        EXPECT_TRUE(on_pc_event_report_with_payload_called);
        EXPECT_EQ(event_with_payload_count, LEN_EVENT_PAYLOAD);

        for (int i = 0; i < event_with_payload_count; i++)
        {

            EXPECT_EQ(event_with_payload[i], i);
        }

        _reset_variables();

        // ********************************************************************
        // ProtocolEventTransport_handle_pc_event_report_with_payload too many bytes
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT_WITH_PAYLOAD, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);

        for (int i = sizeof(event_id_t); i < LEN_EVENT_PAYLOAD + sizeof(event_id_t); i++)
        {

            *openlcb_msg->payload[i] = i - sizeof(event_id_t);
        }

        openlcb_msg->payload_count = LEN_EVENT_PAYLOAD + sizeof(event_id_t) + 1; // too many should clip it

        ProtocolEventTransport_handle_pc_event_report_with_payload(&statemachine_info);
        EXPECT_TRUE(on_pc_event_report_with_payload_called);
        EXPECT_EQ(event_with_payload_count, LEN_EVENT_PAYLOAD);

        for (int i = 0; i < event_with_payload_count; i++)
        {

            EXPECT_EQ(event_with_payload[i], i);
        }

        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // ProtocolEventTransport_handle_pc_event_report_with_payload too few bytes
        // ********************************************************************
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT_WITH_PAYLOAD, 0);
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, DEST_EVENT_ID);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        openlcb_msg->payload_count = sizeof(event_id_t); // too few should fail

        ProtocolEventTransport_handle_pc_event_report_with_payload(&statemachine_info);
        EXPECT_FALSE(on_pc_event_report_with_payload_called);

        _reset_variables();
        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, set_consumer_event_status)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        // ********************************************************************
        // ProtocolEventTransport_set_consumer_event_status, ProtocolEventTransport_extract_consumer_event_status_mti
        // ********************************************************************

        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->consumers.list[i].status = EVENT_STATUS_SET;
            EXPECT_EQ(ProtocolEventTransport_extract_consumer_event_status_mti(node1, i), MTI_CONSUMER_IDENTIFIED_SET);
            node1->consumers.list[i].status = EVENT_STATUS_CLEAR;
            EXPECT_EQ(ProtocolEventTransport_extract_consumer_event_status_mti(node1, i), MTI_CONSUMER_IDENTIFIED_CLEAR);
            node1->consumers.list[i].status = EVENT_STATUS_UNKNOWN;
            EXPECT_EQ(ProtocolEventTransport_extract_consumer_event_status_mti(node1, i), MTI_CONSUMER_IDENTIFIED_UNKNOWN);
            node1->consumers.list[i].status = (event_status_enum)100; // Force an invalid value
            EXPECT_EQ(ProtocolEventTransport_extract_consumer_event_status_mti(node1, i), MTI_CONSUMER_IDENTIFIED_UNKNOWN);
        }

        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->producers.list[i].status = EVENT_STATUS_SET;
            EXPECT_EQ(ProtocolEventTransport_extract_producer_event_status_mti(node1, i), MTI_PRODUCER_IDENTIFIED_SET);
            node1->producers.list[i].status = EVENT_STATUS_CLEAR;
            EXPECT_EQ(ProtocolEventTransport_extract_producer_event_status_mti(node1, i), MTI_PRODUCER_IDENTIFIED_CLEAR);
            node1->producers.list[i].status = EVENT_STATUS_UNKNOWN;
            EXPECT_EQ(ProtocolEventTransport_extract_producer_event_status_mti(node1, i), MTI_PRODUCER_IDENTIFIED_UNKNOWN);
            node1->producers.list[i].status = (event_status_enum)100; // Force an invalid value
            EXPECT_EQ(ProtocolEventTransport_extract_producer_event_status_mti(node1, i), MTI_PRODUCER_IDENTIFIED_UNKNOWN);
        }

        _reset_variables();
        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, handle_identify)
{

    bool done = false;
    int counter = 0;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg) {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;
        

        // ********************************************************************
        // OpenLcbUtilities_load_openlcb_message, unknown
        // ********************************************************************
        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->consumers.list[i].status = EVENT_STATUS_UNKNOWN;
            node1->producers.list[i].status = EVENT_STATUS_UNKNOWN;
        }

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY, 0);

        counter = 0;
        done = false;
        while (!done)
        {

            OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
            ProtocolEventTransport_handle_events_identify(&statemachine_info);
            done = !statemachine_info.enumerating;

            if (counter == 0)
            {

                EXPECT_FALSE(done);
            }

            if (counter < AUTO_CREATE_EVENT_COUNT)
            {

                //        fprintf(stderr, "MTI: 0x%02X; 0x%02X\n", outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->producers.list[counter].event);
            }
            else if (counter < (AUTO_CREATE_EVENT_COUNT * 2))
            {
                EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->consumers.list[counter - AUTO_CREATE_EVENT_COUNT].event);
            }

            counter++;
        }

        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // OpenLcbUtilities_load_openlcb_message, set
        // ********************************************************************
        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->consumers.list[i].status = EVENT_STATUS_SET;
            node1->producers.list[i].status = EVENT_STATUS_SET;
        }

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY, 0);

        counter = 0;
        done = false;
        while (!done)
        {

            OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
            ProtocolEventTransport_handle_events_identify(&statemachine_info);
            done = !statemachine_info.enumerating;

            if (counter == 0)
            {

                EXPECT_FALSE(done);
            }

            if (counter < AUTO_CREATE_EVENT_COUNT)
            {

                EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_SET);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->producers.list[counter].event);
            }
            else if (counter < (AUTO_CREATE_EVENT_COUNT * 2))
            {
                EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_SET);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->consumers.list[counter - AUTO_CREATE_EVENT_COUNT].event);
            }

            counter++;
        }

        _reset_variables();

        // ********************************************************************

        // ********************************************************************
        // OpenLcbUtilities_load_openlcb_message, clear
        // ********************************************************************
        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->consumers.list[i].status = EVENT_STATUS_CLEAR;
            node1->producers.list[i].status = EVENT_STATUS_CLEAR;
        }

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY, 0);

        counter = 0;
        done = false;
        while (!done)
        {

            OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
            ProtocolEventTransport_handle_events_identify(&statemachine_info);
            done = !statemachine_info.enumerating;

            if (counter == 0)
            {

                EXPECT_FALSE(done);
            }

            if (counter < AUTO_CREATE_EVENT_COUNT)
            {

                EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_CLEAR);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->producers.list[counter].event);
            }
            else if (counter < (AUTO_CREATE_EVENT_COUNT * 2))
            {
                EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_CLEAR);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->consumers.list[counter - AUTO_CREATE_EVENT_COUNT].event);
            }

            counter++;
        }

        _reset_variables();

        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, handle_identify_with_dest)
{

    bool done = false;
    int counter = 0;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;
        

        // ********************************************************************
        // OpenLcbUtilities_load_openlcb_message, unknown
        // ********************************************************************
        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->consumers.list[i].status = EVENT_STATUS_UNKNOWN;
            node1->producers.list[i].status = EVENT_STATUS_UNKNOWN;
        }

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY_DEST, 0);

        counter = 0;
        done = false;
        while (!done)
        {

            if (counter == 0)
            {

                EXPECT_FALSE(done);
            }

            OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
            ProtocolEventTransport_handle_events_identify_dest(&statemachine_info);
            done = !statemachine_info.enumerating;;

            if (counter == 0)
            {

                EXPECT_FALSE(done);
            }

            if (counter < AUTO_CREATE_EVENT_COUNT)
            {

                //        fprintf(stderr, "MTI: 0x%02X; 0x%02X\n", outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->producers.list[counter].event);
            }
            else if (counter < (AUTO_CREATE_EVENT_COUNT * 2))
            {
                EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->consumers.list[counter - AUTO_CREATE_EVENT_COUNT].event);
            }

            counter++;
        }

        _reset_variables();
        // ********************************************************************

        // ********************************************************************
        // OpenLcbUtilities_load_openlcb_message, set
        // ********************************************************************
        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->consumers.list[i].status = EVENT_STATUS_SET;
            node1->producers.list[i].status = EVENT_STATUS_SET;
        }

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY, 0);

        counter = 0;
        done = false;
        while (!done)
        {

            if (counter == 0)
            {

                EXPECT_FALSE(done);
            }

            OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
            ProtocolEventTransport_handle_events_identify_dest(&statemachine_info);
            done = !statemachine_info.enumerating;;

            if (counter < AUTO_CREATE_EVENT_COUNT)
            {

                EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_SET);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->producers.list[counter].event);
            }
            else if (counter < (AUTO_CREATE_EVENT_COUNT * 2))
            {
                EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_SET);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->consumers.list[counter - AUTO_CREATE_EVENT_COUNT].event);
            }

            counter++;
        }

        _reset_variables();

        // ********************************************************************

        // ********************************************************************
        // OpenLcbUtilities_load_openlcb_message, clear
        // ********************************************************************
        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->consumers.list[i].status = EVENT_STATUS_CLEAR;
            node1->producers.list[i].status = EVENT_STATUS_CLEAR;
        }

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENTS_IDENTIFY, 0);

        counter = 0;
        done = false;
        while (!done)
        {

            OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
            ProtocolEventTransport_handle_events_identify_dest(&statemachine_info);
            done = !statemachine_info.enumerating;;

            if (counter == 0)
            {

                EXPECT_FALSE(done);
            }

            if (counter < AUTO_CREATE_EVENT_COUNT)
            {

                EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_CLEAR);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->producers.list[counter].event);
            }
            else if (counter < (AUTO_CREATE_EVENT_COUNT * 2))
            {
                EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_CLEAR);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->consumers.list[counter - AUTO_CREATE_EVENT_COUNT].event);
            }

            counter++;
        }

        _reset_variables();

        // ********************************************************************

        // ********************************************************************
        // OpenLcbUtilities_load_openlcb_message, unknown NOT ADDRESSED TO US
        // ********************************************************************
        for (int i = 0; i < AUTO_CREATE_EVENT_COUNT; i++)
        {

            node1->consumers.list[i].status = EVENT_STATUS_UNKNOWN;
            node1->producers.list[i].status = EVENT_STATUS_UNKNOWN;
        }

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS + 1, DEST_ID + 1, MTI_EVENTS_IDENTIFY_DEST, 0);

        counter = 0;
        done = false;
        while (!done)
        {

            if (counter == 0)
            {

                EXPECT_FALSE(done);
            }

            OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
            ProtocolEventTransport_handle_events_identify_dest(&statemachine_info);
            done = !statemachine_info.enumerating;;

            if (counter == 0)
            {

                EXPECT_TRUE(done);
                counter = AUTO_CREATE_EVENT_COUNT * 2;
            }

            if (counter < AUTO_CREATE_EVENT_COUNT)
            {

                //        fprintf(stderr, "MTI: 0x%02X; 0x%02X\n", outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->producers.list[counter].event);
            }
            else if (counter < (AUTO_CREATE_EVENT_COUNT * 2))
            {
                EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
                EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), node1->consumers.list[counter - AUTO_CREATE_EVENT_COUNT].event);
            }

            counter++;
        }

        _reset_variables();
        // ********************************************************************
    }
}

TEST(ProtocolEventTransport, null_callbacks)
{

    _reset_variables();
    _global_initialize_null_callbacks();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg = openlcb_msg;
        statemachine_info.outgoing_msg = outgoing_msg;
        statemachine_info.enumerating = false;
        statemachine_info.outgoing_msg_valid = false;
        

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_UNKNOWN, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_consumer_identified_unknown(&statemachine_info);
        EXPECT_FALSE(on_consumer_identified_unknown_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_SET, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_consumer_identified_set(&statemachine_info);
        EXPECT_FALSE(on_consumer_identified_set_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_CLEAR, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_consumer_identified_clear(&statemachine_info);
        EXPECT_FALSE(on_consumer_identified_clear_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_CONSUMER_IDENTIFIED_RESERVED, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_consumer_identified_reserved(&statemachine_info);
        EXPECT_FALSE(on_consumer_identified_reserved_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_UNKNOWN, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_producer_identified_unknown(&statemachine_info);
        EXPECT_FALSE(on_producer_identified_unknown_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_SET, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_producer_identified_set(&statemachine_info);
        EXPECT_FALSE(on_producer_identified_set_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_CLEAR, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_producer_identified_clear(&statemachine_info);
        EXPECT_FALSE(on_producer_identified_clear_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PRODUCER_IDENTIFIED_RESERVED, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_producer_identified_reserved(&statemachine_info);
        EXPECT_FALSE(on_producer_identified_reserved_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_EVENT_LEARN, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_event_learn(&statemachine_info);
        EXPECT_FALSE(on_event_learn_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_pc_event_report(&statemachine_info);
        EXPECT_FALSE(on_pc_event_report_called);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PC_EVENT_REPORT_WITH_PAYLOAD, 0);
        openlcb_msg->payload_count = 34;
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolEventTransport_handle_pc_event_report_with_payload(&statemachine_info);
        EXPECT_FALSE(on_pc_event_report_with_payload_called);
    }
}