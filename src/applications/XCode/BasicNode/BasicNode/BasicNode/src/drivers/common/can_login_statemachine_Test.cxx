#include "test/main_Test.hxx"

#include "can_login_statemachine.h"

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

bool _init_called = false;
bool _generate_seed_called = false;
bool _generate_alias_called = false;
bool _load_cid07_called = false;
bool _load_cid06_called = false;
bool _load_cid05_called = false;
bool _load_cid04_called = false;
bool _wait_200ms_called = false;
bool _load_rid_called = false;
bool _load_amd_called = false;
bool _load_initialization_complete_called = false;
bool _load_producer_events_called = false;
bool _load_consumer_events_called = false;

uint16_t called_mti = 0;

void _init(can_statemachine_info_t *can_statemachine_info)
{
    _init_called = true;
}

void _generate_seed(can_statemachine_info_t *can_statemachine_info)
{
    _generate_seed_called = true;
}

void _generate_alias(can_statemachine_info_t *can_statemachine_info)
{

    _generate_alias_called = true;
}

void _load_cid07(can_statemachine_info_t *can_statemachine_info)
{
    _load_cid07_called = true;
}

void _load_cid06(can_statemachine_info_t *can_statemachine_info)
{
    _load_cid06_called = true;
}

void _load_cid05(can_statemachine_info_t *can_statemachine_info)
{
    _load_cid05_called = true;
}

void _load_cid04(can_statemachine_info_t *can_statemachine_info)
{

    _load_cid04_called = true;
}

void _wait_200ms(can_statemachine_info_t *can_statemachine_info)
{

    _wait_200ms_called = true;
}

void _load_rid(can_statemachine_info_t *can_statemachine_info)
{

    _load_rid_called = true;
}

void _load_amd(can_statemachine_info_t *can_statemachine_info)
{

    _load_amd_called = true;
}

void _load_initialization_complete(can_statemachine_info_t *can_statemachine_info)
{

    _load_initialization_complete_called = true;
}

void _load_producer_events(can_statemachine_info_t *can_statemachine_info)
{

    _load_producer_events_called = true;
}

void _load_consumer_events(can_statemachine_info_t *can_statemachine_info)
{

    _load_consumer_events_called = true;
}

const interface_can_login_state_machine_t interface_can_login_state_machine = {

    .generate_alias = &_generate_alias,
    .generate_seed = &_generate_seed,
    .init = &_init,
    .load_amd = &_load_amd,
    .load_cid04 = &_load_cid04,
    .load_cid05 = &_load_cid05,
    .load_cid06 = &_load_cid06,
    .load_cid07 = &_load_cid07,
    .load_consumer_events = &_load_consumer_events,
    .load_initialization_complete = &_load_initialization_complete,
    .load_producer_events = &_load_producer_events,
    .load_rid = &_load_rid,
    .wait_200ms = &_wait_200ms

};

const interface_openlcb_node_t interface_openlcb_node = {};

void _reset_variables(void)
{

    _init_called = false;
    _generate_seed_called = false;
    _generate_alias_called = false;
    _load_cid07_called = false;
    _load_cid06_called = false;
    _load_cid05_called = false;
    _load_cid04_called = false;
    _wait_200ms_called = false;
    _load_rid_called = false;
    _load_amd_called = false;
    _load_initialization_complete_called = false;
    _load_producer_events_called = false;
    _load_consumer_events_called = false;
}

void _initialize(void)
{

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);

    CanLoginStateMachine_initialize(&interface_can_login_state_machine);
}
void _initalize_can_statemachine_info(node_id_t node_id, node_parameters_t *node_parameters, can_statemachine_info_t *can_statemachine_info)
{

    can_statemachine_info->openlcb_node = OpenLcbNode_allocate(node_id, node_parameters);
    can_statemachine_info->incoming_msg = NULL;
    can_statemachine_info->outgoing_can_msg = CanBufferStore_allocate_buffer();
    can_statemachine_info->outgoing_can_msg_valid = false;
    can_statemachine_info->outgoing_openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    can_statemachine_info->outgoing_openlcb_msg_valid = false;
}

TEST(CanLoginStateMachine, initialize)
{

    _initialize();
    _reset_variables();
}

TEST(CanLoginStateMachine, run)
{

#define NODE_ID 0x010203040506

    _initialize();
    _reset_variables();

    can_statemachine_info_t can_statemachine_info;
    _initalize_can_statemachine_info(NODE_ID, &_node_parameters_main_node, &can_statemachine_info);

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_INIT;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_TRUE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_GENERATE_SEED;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_TRUE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_TRUE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_TRUE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_06;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_TRUE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_TRUE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_04;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_TRUE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_TRUE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_RESERVE_ID;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_TRUE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_ALIAS_MAP_DEFINITION;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_TRUE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_TRUE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_TRUE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_RUN;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_TRUE(_load_consumer_events_called);
    _reset_variables();

    can_statemachine_info.openlcb_node->state.run_state = 31; // Invalid
    CanLoginStateMachine_run(&can_statemachine_info);
    EXPECT_FALSE(_init_called);
    EXPECT_FALSE(_generate_seed_called);
    EXPECT_FALSE(_generate_alias_called);
    EXPECT_FALSE(_load_cid07_called);
    EXPECT_FALSE(_load_cid06_called);
    EXPECT_FALSE(_load_cid05_called);
    EXPECT_FALSE(_load_cid04_called);
    EXPECT_FALSE(_wait_200ms_called);
    EXPECT_FALSE(_load_rid_called);
    EXPECT_FALSE(_load_amd_called);
    EXPECT_FALSE(_load_initialization_complete_called);
    EXPECT_FALSE(_load_producer_events_called);
    EXPECT_FALSE(_load_consumer_events_called);
    _reset_variables();
}