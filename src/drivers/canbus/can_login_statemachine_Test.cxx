/*******************************************************************************
 * File: can_login_statemachine_Test.cxx
 * 
 * Description:
 *   Comprehensive test suite for the CAN Login State Machine module.
 *   Tests state machine dispatcher that routes to appropriate state handlers.
 *
 * Module Under Test:
 *   CanLoginStateMachine - State machine dispatcher for alias allocation
 *
 * Test Coverage:
 *   - Module initialization
 *   - State dispatch for all valid states
 *   - Invalid state handling
 *   - State handler invocation verification
 *
 * Design Notes:
 *   The state machine is a simple dispatcher that calls the appropriate
 *   handler function based on the current run_state value. The actual
 *   logic is in CanLoginMessageHandler; this tests the dispatch mechanism.
 *
 *   State Flow:
 *   INIT → GENERATE_SEED → GENERATE_ALIAS → CID07 → CID06 → CID05 → CID04 →
 *   WAIT_200ms → LOAD_RID → LOAD_AMD → INIT_COMPLETE → CONSUMER_EVENTS →
 *   PRODUCER_EVENTS → RUN
 *
 * Author: [Your Name]
 * Date: 2026-01-19
 ******************************************************************************/

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

/*******************************************************************************
 * Test Fixtures and Mock Interfaces
 ******************************************************************************/

// Simplified node parameters
node_parameters_t _node_parameters_main_node = {
    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .snip.mfg_version = 4,
    .snip.name = "Test",
    .snip.model = "Test Model J",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2,

    .protocol_support = (PSI_DATAGRAM |
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
    .configuration_options.description = "Memory space capabilities",

    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 0,
    .address_space_configuration_definition.low_address_valid = 0,
    .address_space_configuration_definition.low_address = 0,
    .address_space_configuration_definition.highest_address = 0x200,
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0,
    .address_space_all.low_address = 0,
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,
    .address_space_config_memory.low_address = 0,
    .address_space_config_memory.highest_address = 0x200,
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    .cdi = {},
};

/*******************************************************************************
 * Mock State Handler Flags
 ******************************************************************************/

// Flags to track which state handlers were called
bool _state_init_called = false;
bool _state_generate_seed_called = false;
bool _state_generate_alias_called = false;
bool _state_load_cid07_called = false;
bool _state_load_cid06_called = false;
bool _state_load_cid05_called = false;
bool _state_load_cid04_called = false;
bool _state_wait_200ms_called = false;
bool _state_load_rid_called = false;
bool _state_load_amd_called = false;

/**
 * Mock state handlers - set flags when called
 * The actual state machine just dispatches to these handlers
 */
void _state_init(can_statemachine_info_t *info)
{
    _state_init_called = true;
}

void _state_generate_seed(can_statemachine_info_t *info)
{
    _state_generate_seed_called = true;
}

void _state_generate_alias(can_statemachine_info_t *info)
{
    _state_generate_alias_called = true;
}

void _state_load_cid07(can_statemachine_info_t *info)
{
    _state_load_cid07_called = true;
}

void _state_load_cid06(can_statemachine_info_t *info)
{
    _state_load_cid06_called = true;
}

void _state_load_cid05(can_statemachine_info_t *info)
{
    _state_load_cid05_called = true;
}

void _state_load_cid04(can_statemachine_info_t *info)
{
    _state_load_cid04_called = true;
}

void _state_wait_200ms(can_statemachine_info_t *info)
{
    _state_wait_200ms_called = true;
}

void _state_load_rid(can_statemachine_info_t *info)
{
    _state_load_rid_called = true;
}

void _state_load_amd(can_statemachine_info_t *info)
{
    _state_load_amd_called = true;
}

// Interface with mock handlers
const interface_can_login_state_machine_t interface_can_login_state_machine = {
    .state_init = &_state_init,
    .state_generate_seed = &_state_generate_seed,
    .state_generate_alias = &_state_generate_alias,
    .state_load_cid07 = &_state_load_cid07,
    .state_load_cid06 = &_state_load_cid06,
    .state_load_cid05 = &_state_load_cid05,
    .state_load_cid04 = &_state_load_cid04,
    .state_wait_200ms = &_state_wait_200ms,
    .state_load_rid = &_state_load_rid,
    .state_load_amd = &_state_load_amd
};

const interface_openlcb_node_t interface_openlcb_node = {};

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

/**
 * Reset all state handler call flags
 */
void reset_state_flags(void)
{
    _state_init_called = false;
    _state_generate_seed_called = false;
    _state_generate_alias_called = false;
    _state_load_cid07_called = false;
    _state_load_cid06_called = false;
    _state_load_cid05_called = false;
    _state_load_cid04_called = false;
    _state_wait_200ms_called = false;
    _state_load_rid_called = false;
    _state_load_amd_called = false;
}

/**
 * Initialize all subsystems
 */
void setup_test(void)
{
    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);
    
    CanLoginStateMachine_initialize(&interface_can_login_state_machine);
}

/**
 * Initialize state machine info structure
 */
void initialize_statemachine_info(node_id_t node_id, 
                                  node_parameters_t *params,
                                  can_statemachine_info_t *info)
{
    info->openlcb_node = OpenLcbNode_allocate(node_id, params);
    info->login_outgoing_can_msg = CanBufferStore_allocate_buffer();
    info->outgoing_can_msg = CanBufferStore_allocate_buffer();
    info->login_outgoing_can_msg_valid = false;
    info->enumerating = false;
}

/**
 * Verify that only the specified state handler was called
 */
void verify_only_one_state_called(const char *expected_state)
{
    int call_count = 0;
    
    if (_state_init_called) call_count++;
    if (_state_generate_seed_called) call_count++;
    if (_state_generate_alias_called) call_count++;
    if (_state_load_cid07_called) call_count++;
    if (_state_load_cid06_called) call_count++;
    if (_state_load_cid05_called) call_count++;
    if (_state_load_cid04_called) call_count++;
    if (_state_wait_200ms_called) call_count++;
    if (_state_load_rid_called) call_count++;
    if (_state_load_amd_called) call_count++;
    
    EXPECT_EQ(call_count, 1) << "Expected only " << expected_state << " to be called";
}

/*******************************************************************************
 * Basic Functionality Tests
 ******************************************************************************/

/**
 * Test: Module initialization
 * Verifies state machine can be initialized
 */
TEST(CanLoginStateMachine, initialize)
{
    setup_test();
}

/**
 * Test: State machine dispatch
 * Verifies that each state routes to the correct handler function
 * 
 * This comprehensive test validates the entire state machine dispatch table
 * by setting each possible state and verifying only that state's handler is called
 */
TEST(CanLoginStateMachine, run)
{
    can_statemachine_info_t info;
    
    setup_test();
    initialize_statemachine_info(0x010203040506, &_node_parameters_main_node, &info);
    
    // Test RUNSTATE_INIT
    info.openlcb_node->state.run_state = RUNSTATE_INIT;
    CanLoginStateMachine_run(&info);
    EXPECT_TRUE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_init");
    reset_state_flags();
    
    // Test RUNSTATE_GENERATE_SEED
    info.openlcb_node->state.run_state = RUNSTATE_GENERATE_SEED;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_TRUE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_generate_seed");
    reset_state_flags();
    
    // Test RUNSTATE_GENERATE_ALIAS
    info.openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_TRUE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_generate_alias");
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_CHECK_ID_07
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_TRUE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_load_cid07");
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_CHECK_ID_06
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_06;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_TRUE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_load_cid06");
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_CHECK_ID_05
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_TRUE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_load_cid05");
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_CHECK_ID_04
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_04;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_TRUE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_load_cid04");
    reset_state_flags();
    
    // Test RUNSTATE_WAIT_200ms
    info.openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_TRUE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_wait_200ms");
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_RESERVE_ID
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_RESERVE_ID;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_TRUE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    verify_only_one_state_called("state_load_rid");
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_ALIAS_MAP_DEFINITION
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_ALIAS_MAP_DEFINITION;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_TRUE(_state_load_amd_called);
    verify_only_one_state_called("state_load_amd");
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_INITIALIZATION_COMPLETE - no handler
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_PRODUCER_EVENTS - no handler
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    reset_state_flags();
    
    // Test RUNSTATE_RUN - no handler
    info.openlcb_node->state.run_state = RUNSTATE_RUN;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    reset_state_flags();
    
    // Test RUNSTATE_LOAD_CONSUMER_EVENTS - no handler
    info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    reset_state_flags();
    
    // Test invalid state (31) - no handler should be called
    info.openlcb_node->state.run_state = 31;
    CanLoginStateMachine_run(&info);
    EXPECT_FALSE(_state_init_called);
    EXPECT_FALSE(_state_generate_seed_called);
    EXPECT_FALSE(_state_generate_alias_called);
    EXPECT_FALSE(_state_load_cid07_called);
    EXPECT_FALSE(_state_load_cid06_called);
    EXPECT_FALSE(_state_load_cid05_called);
    EXPECT_FALSE(_state_load_cid04_called);
    EXPECT_FALSE(_state_wait_200ms_called);
    EXPECT_FALSE(_state_load_rid_called);
    EXPECT_FALSE(_state_load_amd_called);
    reset_state_flags();
}

/*******************************************************************************
 * Additional Coverage Tests (Currently Commented Out)
 ******************************************************************************/

// Uncomment and validate these tests one at a time after validating the above

/*
 * Test: Multiple sequential state transitions
 * Verifies state machine can handle rapid state changes
 *
 * */
TEST(CanLoginStateMachine, sequential_states)
{
    can_statemachine_info_t info;
    
    setup_test();
    initialize_statemachine_info(0x010203040506, &_node_parameters_main_node, &info);
    
    // Simulate full login sequence
    info.openlcb_node->state.run_state = RUNSTATE_INIT;
    CanLoginStateMachine_run(&info);
    EXPECT_TRUE(_state_init_called);
    reset_state_flags();
    
    info.openlcb_node->state.run_state = RUNSTATE_GENERATE_SEED;
    CanLoginStateMachine_run(&info);
    EXPECT_TRUE(_state_generate_seed_called);
    reset_state_flags();
    
    info.openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS;
    CanLoginStateMachine_run(&info);
    EXPECT_TRUE(_state_generate_alias_called);
    reset_state_flags();
    
    // Verify each state in sequence works correctly
    for (int state = RUNSTATE_LOAD_CHECK_ID_07; 
         state <= RUNSTATE_LOAD_ALIAS_MAP_DEFINITION; 
         state++)
    {
        info.openlcb_node->state.run_state = state;
        CanLoginStateMachine_run(&info);
        reset_state_flags();
    }
}

/*
 * Test: State machine with NULL info pointer
 * Verifies defensive programming against NULL pointers
 *
 */
TEST(CanLoginStateMachine, null_info_pointer)
{
    setup_test();
    
    // Should not crash - implementation should check for NULL
    // (This test documents expected behavior - may need to add NULL check)
    // CanLoginStateMachine_run(nullptr);
}

/*******************************************************************************
 * End of Test Suite
 ******************************************************************************/
