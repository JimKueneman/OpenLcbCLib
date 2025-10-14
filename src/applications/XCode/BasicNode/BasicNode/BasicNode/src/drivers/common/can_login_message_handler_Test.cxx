#include "test/main_Test.hxx"

#include "can_login_message_handler.h"

#include "can_types.h"
#include "can_utilities.h"
#include "can_rx_message_handler.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"

#include "../alias_mappings.h"

#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_node.h"

#define NODE_ID 0x010203040506
#define ALIAS 0xAAA

bool on_alias_change_called = true;

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

alias_mapping_t alias_mapping = {.alias = 0, .node_id = 0, .is_duplicate = false, .is_permitted = false};

alias_mapping_t *_alias_mapping_register(uint16_t alias, node_id_t node_id)
{

    alias_mapping.alias = alias;
    alias_mapping.node_id = node_id;
    alias_mapping.is_duplicate = false;
    alias_mapping.is_permitted = false;

    return &alias_mapping;
}

alias_mapping_t *_alias_mapping_find_mapping_by_alias(uint16_t alias)
{

    if (alias == alias_mapping.alias)
    {

        return (&alias_mapping);
    }

    return nullptr;
}

void _on_alias_change(uint16_t alias, node_id_t node_id)
{

    on_alias_change_called = true;
}

const interface_openlcb_node_t interface_openlcb_node = {

};

const interface_can_login_message_handler_t interface_can_login_message_handler = {

    .alias_mapping_find_mapping_by_alias = &_alias_mapping_find_mapping_by_alias,
    .alias_mapping_register = &_alias_mapping_register,
    .on_alias_change = nullptr};

const interface_can_login_message_handler_t interface_can_login_message_handler_on_alias_callback = {

    .alias_mapping_find_mapping_by_alias = &_alias_mapping_find_mapping_by_alias,
    .alias_mapping_register = &_alias_mapping_register,
    .on_alias_change = &_on_alias_change};

void _global_initialize(const interface_can_login_message_handler_t *interface_can_login_message_handler)
{

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);

    CanLoginMessageHandler_initialize(interface_can_login_message_handler);
}

void _reset_variables(void)
{

    on_alias_change_called = false;
}

can_msg_t outgoing_can_msg;
can_msg_t login_can_msg;

void _initialize_can_statemachine_info(can_statemachine_info_t *can_statemachine_info)
{

    can_statemachine_info->openlcb_node = OpenLcbNode_allocate(NODE_ID, &_node_parameters_main_node);
    can_statemachine_info->openlcb_node->alias = ALIAS;

    can_statemachine_info->enumerating = false;
    can_statemachine_info->login_outgoing_can_msg_valid = false;
    CanUtilities_clear_can_message(&outgoing_can_msg);
    CanUtilities_clear_can_message(&login_can_msg);

    can_statemachine_info->login_outgoing_can_msg = &login_can_msg;
    can_statemachine_info->outgoing_can_msg = &outgoing_can_msg;
}

TEST(CanLoginMessageHandler, intitialize)
{

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();
}

TEST(CanLoginMessageHandler, init)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_init(&can_statemachine_info);

    EXPECT_NE(can_statemachine_info.openlcb_node->seed, 0x00);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_GENERATE_ALIAS);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
}

TEST(CanLoginMessageHandler, generate_seed)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_init(&can_statemachine_info);

    uint64_t old_seed = can_statemachine_info.openlcb_node->seed;

    CanLoginMessageHandler_state_generate_seed(&can_statemachine_info);

    EXPECT_NE(can_statemachine_info.openlcb_node->seed, old_seed);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_GENERATE_ALIAS);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
}

TEST(CanLoginMessageHandler, generate_alias)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_init(&can_statemachine_info);

    can_statemachine_info.openlcb_node->alias = 0x00;

    CanLoginMessageHandler_state_generate_alias(&can_statemachine_info);

    EXPECT_NE(can_statemachine_info.openlcb_node->alias, 0x00);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);

    _global_initialize(&interface_can_login_message_handler_on_alias_callback);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_init(&can_statemachine_info);

    can_statemachine_info.openlcb_node->alias = 0x00;

    CanLoginMessageHandler_state_generate_alias(&can_statemachine_info);

    EXPECT_NE(can_statemachine_info.openlcb_node->alias, 0x00);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
    EXPECT_TRUE(on_alias_change_called);
}

TEST(CanLoginMessageHandler, load_cid07)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_load_cid07(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((can_statemachine_info.openlcb_node->id >> 24) & 0xFFF000) | can_statemachine_info.openlcb_node->alias));
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_06);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
}

TEST(CanLoginMessageHandler, load_cid06)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_load_cid06(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((can_statemachine_info.openlcb_node->id >> 12) & 0xFFF000) | can_statemachine_info.openlcb_node->alias));
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_05);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
}

TEST(CanLoginMessageHandler, load_cid05)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_load_cid05(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((can_statemachine_info.openlcb_node->id & 0xFFF000) | can_statemachine_info.openlcb_node->alias));
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_04);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
}

TEST(CanLoginMessageHandler, load_cid04)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_load_cid04(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((can_statemachine_info.openlcb_node->id << 12) & 0xFFF000) | can_statemachine_info.openlcb_node->alias));
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
}

TEST(CanLoginMessageHandler, wait_200ms)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;

    CanLoginMessageHandler_state_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
    EXPECT_EQ(can_statemachine_info.openlcb_node->timerticks, 0);

    OpenLcbNode_100ms_timer_tick();
    CanLoginMessageHandler_state_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
    EXPECT_EQ(can_statemachine_info.openlcb_node->timerticks, 1);

    OpenLcbNode_100ms_timer_tick();
    CanLoginMessageHandler_state_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
    EXPECT_EQ(can_statemachine_info.openlcb_node->timerticks, 2);

    OpenLcbNode_100ms_timer_tick();
    CanLoginMessageHandler_state_wait_200ms(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_RESERVE_ID);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
    EXPECT_EQ(can_statemachine_info.openlcb_node->timerticks, 3);
}

TEST(CanLoginMessageHandler, load_rid)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    CanLoginMessageHandler_state_load_rid(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 0);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_statemachine_info.openlcb_node->alias);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_ALIAS_MAP_DEFINITION);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
}

TEST(CanLoginMessageHandler, load_amd)
{

    can_statemachine_info_t can_statemachine_info;

    _global_initialize(&interface_can_login_message_handler);
    _reset_variables();

    _initialize_can_statemachine_info(&can_statemachine_info);

    alias_mapping.alias = can_statemachine_info.openlcb_node->alias;
    alias_mapping.node_id = can_statemachine_info.openlcb_node->id;

    CanLoginMessageHandler_state_load_amd(&can_statemachine_info);

    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(can_statemachine_info.login_outgoing_can_msg->identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_statemachine_info.openlcb_node->alias);
    EXPECT_EQ(can_statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_INITIALIZATION_COMPLETE);
    EXPECT_EQ(CanUtilities_extract_can_payload_as_node_id(can_statemachine_info.login_outgoing_can_msg), NODE_ID);
    EXPECT_TRUE(can_statemachine_info.login_outgoing_can_msg_valid);
    EXPECT_TRUE(can_statemachine_info.openlcb_node->state.permitted);
    EXPECT_FALSE(can_statemachine_info.openlcb_node->state.initalized);
}