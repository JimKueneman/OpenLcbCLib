#include "test/main_Test.hxx"

#include "can_login_message_handler.h"

#include "can_types.h"
#include "can_utilities.h"
#include "can_rx_message_handler.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_node.h"

#define NODE_ID 0x010203040506
#define ALIAS 0xAAA

bool _alias_change_callback_called = false;
uint16_t _alias_change_callback_alias = 0;
uint64_t _alias_change_callback_node_id = 0;

node_parameters_t _node_parameters_main_node = {

    .consumer_count_autocreate = 3,
    .producer_count_autocreate = 4,

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

node_parameters_t _node_parameters_main_node_no_events = {

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

uint16_t _extract_producer_event_state_mti(openlcb_node_t *openlcb_node, uint16_t event_index)
{

    return 0;
}
uint16_t _extract_consumer_event_state_mti(openlcb_node_t *openlcb_node, uint16_t event_index)
{

    return 0;
}

alias_mapping_t alias_mapping = {.alias = 0x00, .node_id = 0x00, .is_duplicate = false, .is_permitted = false};

alias_mapping_t *_alias_mapping_find_mapping_by_alias(uint16_t alias)
{

    if (alias == alias_mapping.alias)
    {

        return (&alias_mapping);
    }

    return NULL;
}

alias_mapping_t *_alias_mapping_register(uint16_t alias, node_id_t node_id)
{

    alias_mapping.alias = alias;
    alias_mapping.node_id = node_id;

    return &alias_mapping;
}

void on_alias_change(uint16_t alias, uint64_t node_id)
{
    _alias_change_callback_called = true;
    _alias_change_callback_alias = alias;
    _alias_change_callback_node_id = node_id;
}

const interface_can_login_message_handler_t interface_can_login_message_handler = {

    .extract_consumer_event_state_mti = &_extract_consumer_event_state_mti,
    .extract_producer_event_state_mti = &_extract_producer_event_state_mti,
    .alias_mapping_find_mapping_by_alias = &_alias_mapping_find_mapping_by_alias,
    .alias_mapping_register = &_alias_mapping_register,
    .on_alias_change = &on_alias_change

};

const interface_openlcb_node_t interface_openlcb_node = {

};

void _global_initialize(void)
{

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);

    CanLoginMessageHandler_initialize(&interface_can_login_message_handler);
}

void _global_reset_variables(void)
{

    _alias_change_callback_called = false;
    _alias_change_callback_alias = 0;
    _alias_change_callback_node_id = 0;
}

void _initalize_can_statemachine_info(node_id_t node_id, node_parameters_t *node_parameters, can_statemachine_info_t *can_statemachine_info)
{

    can_statemachine_info->openlcb_node = OpenLcbNode_allocate(node_id, node_parameters);
    can_statemachine_info->login_outgoing_can_msg = CanBufferStore_allocate_buffer();
    can_statemachine_info->login_outgoing_can_msg_valid = false;
    can_statemachine_info->login_outgoing_openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    can_statemachine_info->login_outgoing_openlcb_msg_valid = false;
}

TEST(CanLoginMessageHandler, init)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);

    CanLoginMessageHandler_init(&can_statemachine_info);
    EXPECT_EQ(can_statemachine_info.openlcb_node->seed, NODE_ID);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_GENERATE_ALIAS);
}

TEST(CanLoginMessageHandler, generate_seed)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);

    can_statemachine_info.openlcb_node->seed = NODE_ID;
    can_statemachine_info.openlcb_node->index = 2;
    CanLoginMessageHandler_generate_seed(&can_statemachine_info);
    EXPECT_NE(can_statemachine_info.openlcb_node->seed, NODE_ID);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_GENERATE_ALIAS);
}

TEST(CanLoginMessageHandler, generate_alias)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);

    can_statemachine_info.openlcb_node->seed = NODE_ID;
    can_statemachine_info.openlcb_node->alias = 0;

    CanLoginMessageHandler_generate_alias(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
    EXPECT_NE(can_statemachine_info.openlcb_node->alias, 0);
    EXPECT_TRUE(_alias_change_callback_called);

    _global_reset_variables();
    can_statemachine_info.openlcb_node->seed = NODE_ID;
    can_statemachine_info.openlcb_node->alias = 0;

    CanLoginMessageHandler_generate_alias(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
    EXPECT_NE(can_statemachine_info.openlcb_node->alias, 0);
    EXPECT_TRUE(_alias_change_callback_called);
    EXPECT_EQ(_alias_change_callback_alias, can_statemachine_info.openlcb_node->alias);
    EXPECT_EQ(_alias_change_callback_node_id, can_statemachine_info.openlcb_node->id);
}

TEST(CanLoginMessageHandler, load_cid07)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;
    can_statemachine_info.openlcb_node->alias = ALIAS;

    CanLoginMessageHandler_load_cid07(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((can_statemachine_info.openlcb_node->id >> 24) & 0xFFF000) | can_statemachine_info.openlcb_node->alias));
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_06);
}

TEST(CanLoginMessageHandler, load_cid06)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_06;

    CanLoginMessageHandler_load_cid06(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((can_statemachine_info.openlcb_node->id >> 12) & 0xFFF000) | can_statemachine_info.openlcb_node->alias));
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_05);
}

TEST(CanLoginMessageHandler, load_cid05)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;

    CanLoginMessageHandler_load_cid05(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((can_statemachine_info.openlcb_node->id & 0xFFF000) | can_statemachine_info.openlcb_node->alias));
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_04);
}

TEST(CanLoginMessageHandler, load_cid04)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_04;

    CanLoginMessageHandler_load_cid04(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((can_statemachine_info.openlcb_node->id << 12) & 0xFFF000) | can_statemachine_info.openlcb_node->alias));
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
}

TEST(CanLoginMessageHandler, wait_200ms)
{

    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;
    can_statemachine_info.openlcb_node->timerticks = 0;

    CanLoginMessageHandler_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

    can_statemachine_info.openlcb_node->timerticks++;
    CanLoginMessageHandler_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

    can_statemachine_info.openlcb_node->timerticks++;
    CanLoginMessageHandler_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

    can_statemachine_info.openlcb_node->timerticks++;
    CanLoginMessageHandler_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

    can_statemachine_info.openlcb_node->timerticks++;
    CanLoginMessageHandler_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_RESERVE_ID);
}

TEST(CanLoginMessageHandler, load_rid)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_RESERVE_ID;

    CanLoginMessageHandler_load_rid(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, (RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_statemachine_info.openlcb_node->alias));
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_ALIAS_MAP_DEFINITION);
}

TEST(CanLoginMessageHandler, load_amd)
{
    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_ALIAS_MAP_DEFINITION;

    CanLoginMessageHandler_load_amd(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(CanUtilities_extract_can_payload_as_node_id(can_statemachine_info.login_outgoing_can_msg), NODE_ID);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, (RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_statemachine_info.openlcb_node->alias));
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
    EXPECT_TRUE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_INITIALIZATION_COMPLETE);
}

TEST(CanLoginMessageHandler, load_initialization_complete)
{

    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->alias = ALIAS;
    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    CanLoginMessageHandler_load_initialization_complete(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_openlcb_msg->mti, MTI_INITIALIZATION_COMPLETE);
    EXPECT_EQ(can_statemachine_info.login_outgoing_openlcb_msg->payload_count, 6);
    EXPECT_EQ(can_statemachine_info.login_outgoing_openlcb_msg->source_alias, ALIAS);
    EXPECT_EQ(can_statemachine_info.login_outgoing_openlcb_msg->source_id, NODE_ID);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(can_statemachine_info.login_outgoing_openlcb_msg, 0), NODE_ID);
    EXPECT_TRUE(can_statemachine_info.openlcb_node->state.initalized);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_openlcb_msg_valid);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    _global_reset_variables();

    _node_parameters_main_node.protocol_support = _node_parameters_main_node.protocol_support | PSI_SIMPLE;
    CanLoginMessageHandler_load_initialization_complete(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_openlcb_msg->mti, MTI_INITIALIZATION_COMPLETE_SIMPLE);
    EXPECT_EQ(can_statemachine_info.login_outgoing_openlcb_msg->payload_count, 6);
    EXPECT_EQ(can_statemachine_info.login_outgoing_openlcb_msg->source_alias, ALIAS);
    EXPECT_EQ(can_statemachine_info.login_outgoing_openlcb_msg->source_id, NODE_ID);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(can_statemachine_info.login_outgoing_openlcb_msg, 0), NODE_ID);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_openlcb_msg_valid);
    _node_parameters_main_node.protocol_support = _node_parameters_main_node.protocol_support & ~PSI_SIMPLE;
}

TEST(CanLoginMessageHandler, load_producer_events)
{

    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->alias = ALIAS;
    can_statemachine_info.openlcb_node->producers.enumerator.running = true;
    can_statemachine_info.openlcb_node->producers.enumerator.enum_index = 0;
    can_statemachine_info.openlcb_node->state.initalized = true;
    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

    // *******************************************************************************************
    // Test with the send buffer free and get all events
    // *******************************************************************************************

    _global_reset_variables();

    uint16_t offset = 0;
    while (can_statemachine_info.openlcb_node->state.run_state == RUNSTATE_LOAD_PRODUCER_EVENTS)
    {

        CanLoginMessageHandler_load_producer_events(&can_statemachine_info);
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(can_statemachine_info.login_outgoing_openlcb_msg), (uint64_t)(NODE_ID << 16) + offset);
        EXPECT_TRUE(can_statemachine_info.login_outgoing_openlcb_msg_valid);
        offset++;
    }

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    _global_initialize();
    _global_reset_variables();

    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node_no_events, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->alias = ALIAS;
    can_statemachine_info.openlcb_node->consumers.enumerator.running = true;
    can_statemachine_info.openlcb_node->consumers.enumerator.enum_index = 0;
    can_statemachine_info.openlcb_node->state.initalized = true;
    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

    // *******************************************************************************************
    // Test with no events defined for the node
    // *******************************************************************************************
    CanLoginMessageHandler_load_producer_events(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    EXPECT_FALSE(can_statemachine_info.login_outgoing_openlcb_msg_valid);

    // *******************************************************************************************
}

TEST(CanLoginMessageHandler, load_consumer_events)
{

    _global_initialize();
    _global_reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->alias = ALIAS;
    can_statemachine_info.openlcb_node->consumers.enumerator.running = true;
    can_statemachine_info.openlcb_node->consumers.enumerator.enum_index = 0;
    can_statemachine_info.openlcb_node->state.initalized = true;
    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

    // *******************************************************************************************
    // Test with the send buffer free and get all events
    // *******************************************************************************************
    _global_reset_variables();

    uint16_t offset = 0;
    while (can_statemachine_info.openlcb_node->state.run_state == RUNSTATE_LOAD_CONSUMER_EVENTS)
    {

        CanLoginMessageHandler_load_consumer_events(&can_statemachine_info);
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(can_statemachine_info.login_outgoing_openlcb_msg), (uint64_t)(NODE_ID << 16) + offset);
        offset++;
    }

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_RUN);
    // *******************************************************************************************

    _global_initialize();
    _global_reset_variables();

    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node_no_events, &can_statemachine_info);
    CanLoginMessageHandler_generate_alias(&can_statemachine_info); // initialize the alias_mapping structure

    can_statemachine_info.openlcb_node->alias = ALIAS;
    can_statemachine_info.openlcb_node->consumers.enumerator.running = true;
    can_statemachine_info.openlcb_node->consumers.enumerator.enum_index = 0;
    can_statemachine_info.openlcb_node->state.initalized = true;
    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

    _global_reset_variables();
    // *******************************************************************************************
    // Test with no events defined for the node
    // *******************************************************************************************
    CanLoginMessageHandler_load_consumer_events(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_RUN);
    EXPECT_FALSE(can_statemachine_info.login_outgoing_openlcb_msg_valid);

    // *******************************************************************************************
}
