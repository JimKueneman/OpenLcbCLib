#include "test/main_Test.hxx"

#include "openlcb_login_statemachine_handler.h"

#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

#define SOURCE_ALIAS 0x222
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0xBBB
#define DEST_ID 0x060504030201

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
                         PSI_FIRMWARE_UPGRADE |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,

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
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0,       // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,         // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
    .address_space_firmware.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_firmware.low_address = 0,         // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
        },

};

uint16_t _extract_producer_event_state_mti(openlcb_node_t *openlcb_node, uint16_t event_index)
{

    return MTI_PRODUCER_IDENTIFIED_UNKNOWN;
}

uint16_t _extract_consumer_event_state_mti(openlcb_node_t *openlcb_node, uint16_t event_index)
{

    return MTI_CONSUMER_IDENTIFIED_UNKNOWN;
}

interface_openlcb_login_message_handler_t interface_openlcb_login_message_handler = {

    .extract_producer_event_state_mti = &_extract_producer_event_state_mti,
    .extract_consumer_event_state_mti = &_extract_consumer_event_state_mti

};

interface_openlcb_node_t interface_openlcb_node = {};

void _reset_variables(void)
{
}

void _global_initialize(void)
{

    OpenLcbLoginMessageHandler_initialize(&interface_openlcb_login_message_handler);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(OpenLcbLoginMessageHandler, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(OpenLcbLoginMessageHandler, load_initialization_complete)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_login_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_INITIALIZATION_COMPLETE);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_id, node1->id);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_alias, node1->alias);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 6);
    EXPECT_TRUE(statemachine_info.openlcb_node->state.initialized);
    EXPECT_TRUE(statemachine_info.openlcb_node->producers.enumerator.running);
    EXPECT_TRUE(statemachine_info.openlcb_node->state.initialized);
    EXPECT_FALSE(statemachine_info.openlcb_node->consumers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 0);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 0);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    _node_parameters_main_node.protocol_support = (PSI_DATAGRAM |
                                                   PSI_SIMPLE |
                                                   PSI_EVENT_EXCHANGE);

    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_INITIALIZATION_COMPLETE_SIMPLE);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_id, node1->id);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_alias, node1->alias);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 6);
    EXPECT_TRUE(statemachine_info.openlcb_node->state.initialized);
    EXPECT_TRUE(statemachine_info.openlcb_node->producers.enumerator.running);
    EXPECT_TRUE(statemachine_info.openlcb_node->state.initialized);
    EXPECT_FALSE(statemachine_info.openlcb_node->consumers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 0);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 0);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    _node_parameters_main_node.protocol_support = (PSI_DATAGRAM |
                                                   PSI_FIRMWARE_UPGRADE |
                                                   PSI_MEMORY_CONFIGURATION |
                                                   PSI_EVENT_EXCHANGE |
                                                   PSI_ABBREVIATED_DEFAULT_CDI |
                                                   PSI_SIMPLE_NODE_INFORMATION |
                                                   PSI_CONFIGURATION_DESCRIPTION_INFO);
}

TEST(OpenLcbLoginMessageHandler, load_producer_event)
{

    // ************************************************************************
    // Special case where the node does not support any events
    // ************************************************************************

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_login_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    // ************************************************************************
    // Create some events in the node
    // ************************************************************************

    _node_parameters_main_node.consumer_count_autocreate = 2;
    _node_parameters_main_node.producer_count_autocreate = 2;

    _reset_variables();
    _global_initialize();

    node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_ID << 16);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), (DEST_ID << 16) + 1);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 0;
}

TEST(OpenLcbLoginMessageHandler, load_consumer_event)
{

    // ************************************************************************
    // Special case where the node does not support any events
    // ************************************************************************

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_login_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_RUN);

    // ************************************************************************
    // Create some events in the node
    // ************************************************************************

    _node_parameters_main_node.consumer_count_autocreate = 2;
    _node_parameters_main_node.producer_count_autocreate = 2;

    _reset_variables();
    _global_initialize();

    node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_ID << 16);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), (DEST_ID << 16) + 1);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_RUN);

    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 0;
}
