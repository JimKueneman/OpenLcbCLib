/*******************************************************************************
 * File: can_login_message_handler_Test.cxx
 * 
 * Description:
 *   Comprehensive test suite for the CAN Login Message Handler module.
 *   Tests the OpenLCB alias allocation sequence according to the specification.
 *
 * Module Under Test:
 *   CanLoginMessageHandler - Manages node alias allocation and login sequence
 *
 * Test Coverage:
 *   - Module initialization
 *   - State initialization (seed from Node ID)
 *   - Seed generation (LFSR algorithm)
 *   - Alias generation (12-bit from 48-bit Node ID)
 *   - CID frame generation (CID7, CID6, CID5, CID4)
 *   - 200ms wait timer management
 *   - RID frame generation
 *   - AMD frame generation and permitted state
 *   - Optional alias change callback
 *
 * OpenLCB Alias Allocation Sequence:
 *   1. INIT - Seed LFSR with Node ID
 *   2. GENERATE_SEED - Iterate LFSR
 *   3. GENERATE_ALIAS - Extract 12-bit alias from seed
 *   4. LOAD_CID07 - Send Check ID frame with bits 47-36 of Node ID
 *   5. LOAD_CID06 - Send Check ID frame with bits 35-24 of Node ID
 *   6. LOAD_CID05 - Send Check ID frame with bits 23-12 of Node ID
 *   7. LOAD_CID04 - Send Check ID frame with bits 11-0 of Node ID
 *   8. WAIT_200ms - Wait for collision detection
 *   9. LOAD_RID - Reserve ID frame
 *  10. LOAD_AMD - Alias Map Definition (enters permitted state)
 *  11. INIT_COMPLETE - Node is now permitted to communicate
 *
 * Design Notes:
 *   The CID sequence spreads the 48-bit Node ID across 4 frames in the
 *   variable field of the CAN identifier. This allows other nodes to detect
 *   alias collisions by comparing Node IDs.
 *
 * Author: [Your Name]
 * Date: 2026-01-19
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "can_login_message_handler.h"
#include "can_types.h"
#include "can_utilities.h"
#include "can_rx_message_handler.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "alias_mappings.h"

#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_node.h"

/*******************************************************************************
 * Test Constants
 ******************************************************************************/
#define NODE_ID 0x010203040506
#define ALIAS 0xAAA

/*******************************************************************************
 * Test Fixtures and Mock Interfaces
 ******************************************************************************/

bool on_alias_change_called = false;

// Simplified node parameters for testing
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

// Mock alias mapping for testing
alias_mapping_t alias_mapping = {
    .alias = 0, 
    .node_id = 0, 
    .is_duplicate = false, 
    .is_permitted = false
};

/**
 * Mock: Alias mapping register
 * Stores alias/node_id mapping for testing
 */
alias_mapping_t *_alias_mapping_register(uint16_t alias, node_id_t node_id)
{
    alias_mapping.alias = alias;
    alias_mapping.node_id = node_id;
    alias_mapping.is_duplicate = false;
    alias_mapping.is_permitted = false;
    return &alias_mapping;
}

/**
 * Mock: Alias mapping find by alias
 * Returns stored mapping if alias matches
 */
alias_mapping_t *_alias_mapping_find_mapping_by_alias(uint16_t alias)
{
    if (alias == alias_mapping.alias)
    {
        return &alias_mapping;
    }
    return nullptr;
}

/**
 * Mock: Alias change callback
 * Sets flag when called to verify callback invocation
 */
void _on_alias_change(uint16_t alias, node_id_t node_id)
{
    on_alias_change_called = true;
}

// Interface without callback
const interface_openlcb_node_t interface_openlcb_node = {};

const interface_can_login_message_handler_t interface_can_login_message_handler = {
    .alias_mapping_find_mapping_by_alias = &_alias_mapping_find_mapping_by_alias,
    .alias_mapping_register = &_alias_mapping_register,
    .on_alias_change = nullptr
};

// Interface with callback
const interface_can_login_message_handler_t interface_can_login_message_handler_on_alias_callback = {
    .alias_mapping_find_mapping_by_alias = &_alias_mapping_find_mapping_by_alias,
    .alias_mapping_register = &_alias_mapping_register,
    .on_alias_change = &_on_alias_change
};

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

/**
 * Initialize all subsystems for testing
 */
void setup_test(const interface_can_login_message_handler_t *interface)
{
    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);
    
    CanLoginMessageHandler_initialize(interface);
}

/**
 * Reset test state variables
 */
void reset_test_variables(void)
{
    on_alias_change_called = false;
}

// CAN message buffers for testing
can_msg_t outgoing_can_msg;
can_msg_t login_can_msg;

/**
 * Initialize state machine info structure for testing
 */
void initialize_statemachine_info(can_statemachine_info_t *info)
{
    info->openlcb_node = OpenLcbNode_allocate(NODE_ID, &_node_parameters_main_node);
    info->openlcb_node->alias = ALIAS;
    
    info->enumerating = false;
    info->login_outgoing_can_msg_valid = false;
    
    CanUtilities_clear_can_message(&outgoing_can_msg);
    CanUtilities_clear_can_message(&login_can_msg);
    
    info->login_outgoing_can_msg = &login_can_msg;
    info->outgoing_can_msg = &outgoing_can_msg;
}

/*******************************************************************************
 * Basic Functionality Tests
 ******************************************************************************/

/**
 * Test: Module initialization
 * Verifies that the handler can be initialized without errors
 */
TEST(CanLoginMessageHandler, initialize)
{
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
}

/**
 * Test: State init
 * Verifies that initialization sets seed to Node ID and transitions to generate alias
 * 
 * OpenLCB Spec:
 * The seed is initially set equal to the Node ID to begin the LFSR sequence
 */
TEST(CanLoginMessageHandler, init)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_init(&info);
    
    // Seed should equal Node ID
    EXPECT_EQ(info.openlcb_node->seed, NODE_ID);
    
    // Should transition to generate alias state
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_GENERATE_ALIAS);
    
    // Not yet permitted or initialized
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/**
 * Test: Generate seed
 * Verifies LFSR seed generation algorithm
 * 
 * The LFSR (Linear Feedback Shift Register) is used to generate pseudo-random
 * alias values from the Node ID seed
 */
TEST(CanLoginMessageHandler, generate_seed)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_init(&info);
    
    uint64_t original_seed = info.openlcb_node->seed;
    
    CanLoginMessageHandler_state_generate_seed(&info);
    
    // Seed should have changed via LFSR
    EXPECT_NE(info.openlcb_node->seed, original_seed);
    
    // Should transition to generate alias
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_GENERATE_ALIAS);
    
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/**
 * Test: Generate alias
 * Verifies 12-bit alias generation from seed and callback invocation
 * 
 * Tests both with and without the optional alias change callback
 */
TEST(CanLoginMessageHandler, generate_alias)
{
    can_statemachine_info_t info;
    
    // Test without callback
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_init(&info);
    
    info.openlcb_node->alias = 0x00;
    
    CanLoginMessageHandler_state_generate_alias(&info);
    
    // Alias should be generated (12-bit, non-zero)
    EXPECT_NE(info.openlcb_node->alias, 0x00);
    EXPECT_LE(info.openlcb_node->alias, 0xFFF);
    
    // Should transition to load CID7
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
    
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
    
    // Test with callback
    setup_test(&interface_can_login_message_handler_on_alias_callback);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_init(&info);
    
    info.openlcb_node->alias = 0x00;
    
    CanLoginMessageHandler_state_generate_alias(&info);
    
    EXPECT_NE(info.openlcb_node->alias, 0x00);
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
    
    // Callback should have been invoked
    EXPECT_TRUE(on_alias_change_called);
}

/**
 * Test: Generate alias rejects 0x000
 * Verifies that a seed producing alias 0x000 advances the LFSR
 * and regenerates until a non-zero alias is obtained.
 *
 * Seed 0x050101000151 produces alias 0x000 from the XOR formula,
 * so the handler must detect and retry.
 */
TEST(CanLoginMessageHandler, generate_alias_rejects_zero)
{
    can_statemachine_info_t info;

    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);

    // Force a seed that produces alias 0x000
    info.openlcb_node->seed = 0x050101000151ULL;
    info.openlcb_node->alias = 0x00;

    CanLoginMessageHandler_state_generate_alias(&info);

    // Alias must be non-zero (the handler should have advanced the seed)
    EXPECT_NE(info.openlcb_node->alias, 0x000);
    EXPECT_LE(info.openlcb_node->alias, 0xFFF);

    // Seed must have been advanced at least once
    EXPECT_NE(info.openlcb_node->seed, (uint64_t) 0x050101000151ULL);

    // Should still transition to CID7
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
}

/*******************************************************************************
 * CID Frame Generation Tests
 ******************************************************************************/

/**
 * Test: Load CID7 frame
 * Verifies Check ID frame 7 contains bits 47-36 of Node ID
 * 
 * CID7 format: Variable field = (bits 47-36 of Node ID) | alias
 */
TEST(CanLoginMessageHandler, load_cid07)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_load_cid07(&info);
    
    // CID frames have no payload
    EXPECT_EQ(info.login_outgoing_can_msg->payload_count, 0);
    
    // Verify identifier format
    uint32_t expected_id = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | 
                          (((info.openlcb_node->id >> 24) & 0xFFF000) | 
                           info.openlcb_node->alias);
    EXPECT_EQ(info.login_outgoing_can_msg->identifier, expected_id);
    
    // Should transition to CID6
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_06);
    
    EXPECT_TRUE(info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/**
 * Test: Load CID6 frame
 * Verifies Check ID frame 6 contains bits 35-24 of Node ID
 */
TEST(CanLoginMessageHandler, load_cid06)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_load_cid06(&info);
    
    EXPECT_EQ(info.login_outgoing_can_msg->payload_count, 0);
    
    uint32_t expected_id = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | 
                          (((info.openlcb_node->id >> 12) & 0xFFF000) | 
                           info.openlcb_node->alias);
    EXPECT_EQ(info.login_outgoing_can_msg->identifier, expected_id);
    
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_05);
    EXPECT_TRUE(info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/**
 * Test: Load CID5 frame
 * Verifies Check ID frame 5 contains bits 23-12 of Node ID
 */
TEST(CanLoginMessageHandler, load_cid05)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_load_cid05(&info);
    
    EXPECT_EQ(info.login_outgoing_can_msg->payload_count, 0);
    
    uint32_t expected_id = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | 
                          ((info.openlcb_node->id & 0xFFF000) | 
                           info.openlcb_node->alias);
    EXPECT_EQ(info.login_outgoing_can_msg->identifier, expected_id);
    
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_04);
    EXPECT_TRUE(info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/**
 * Test: Load CID4 frame
 * Verifies Check ID frame 4 contains bits 11-0 of Node ID
 * Also verifies timer is reset for 200ms wait
 */
TEST(CanLoginMessageHandler, load_cid04)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_load_cid04(&info);
    
    EXPECT_EQ(info.login_outgoing_can_msg->payload_count, 0);
    
    uint32_t expected_id = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | 
                          (((info.openlcb_node->id << 12) & 0xFFF000) | 
                           info.openlcb_node->alias);
    EXPECT_EQ(info.login_outgoing_can_msg->identifier, expected_id);
    
    // Should transition to wait state and reset timer
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
    EXPECT_EQ(info.openlcb_node->timerticks, 0);
    
    EXPECT_TRUE(info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/*******************************************************************************
 * Timing Tests
 ******************************************************************************/

/**
 * Test: 200ms wait timer
 * Verifies that the state machine waits for 3 timer ticks (300ms)
 * before transitioning to reserve ID
 * 
 * OpenLCB Spec:
 * Must wait minimum 200ms after last CID frame before sending RID
 * Implementation uses 3x 100ms ticks = 300ms for safety margin
 */
TEST(CanLoginMessageHandler, wait_200ms)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    info.openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;
    
    // Tick 0 - should still be waiting
    CanLoginMessageHandler_state_wait_200ms(&info);
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
    EXPECT_EQ(info.openlcb_node->timerticks, 0);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
    
    // Tick 1 (100ms)
    OpenLcbNode_100ms_timer_tick();
    CanLoginMessageHandler_state_wait_200ms(&info);
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
    EXPECT_EQ(info.openlcb_node->timerticks, 1);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
    
    // Tick 2 (200ms)
    OpenLcbNode_100ms_timer_tick();
    CanLoginMessageHandler_state_wait_200ms(&info);
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);
    EXPECT_EQ(info.openlcb_node->timerticks, 2);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
    
    // Tick 3 (300ms) - should transition
    OpenLcbNode_100ms_timer_tick();
    CanLoginMessageHandler_state_wait_200ms(&info);
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_RESERVE_ID);
    EXPECT_EQ(info.openlcb_node->timerticks, 3);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/*******************************************************************************
 * Alias Reservation Tests
 ******************************************************************************/

/**
 * Test: Load RID frame
 * Verifies Reserve ID frame generation
 * 
 * RID frame claims the alias for this node
 */
TEST(CanLoginMessageHandler, load_rid)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_load_rid(&info);
    
    EXPECT_EQ(info.login_outgoing_can_msg->payload_count, 0);
    
    uint32_t expected_id = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | 
                          info.openlcb_node->alias;
    EXPECT_EQ(info.login_outgoing_can_msg->identifier, expected_id);
    
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_ALIAS_MAP_DEFINITION);
    EXPECT_TRUE(info.login_outgoing_can_msg_valid);
    EXPECT_FALSE(info.openlcb_node->state.permitted);
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/**
 * Test: Load AMD frame
 * Verifies Alias Map Definition frame generation and permitted state
 * 
 * AMD frame announces the alias-to-Node ID mapping to the network
 * After AMD, node enters permitted state and can communicate
 */
TEST(CanLoginMessageHandler, load_amd)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    // Setup alias mapping for lookup
    alias_mapping.alias = info.openlcb_node->alias;
    alias_mapping.node_id = info.openlcb_node->id;
    
    CanLoginMessageHandler_state_load_amd(&info);
    
    // AMD carries full 6-byte Node ID in payload
    EXPECT_EQ(info.login_outgoing_can_msg->payload_count, 6);
    
    uint32_t expected_id = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | 
                          info.openlcb_node->alias;
    EXPECT_EQ(info.login_outgoing_can_msg->identifier, expected_id);
    
    // Verify Node ID in payload
    EXPECT_EQ(CanUtilities_extract_can_payload_as_node_id(info.login_outgoing_can_msg), 
              NODE_ID);
    
    EXPECT_EQ(info.openlcb_node->state.run_state, RUNSTATE_LOAD_INITIALIZATION_COMPLETE);
    EXPECT_TRUE(info.login_outgoing_can_msg_valid);
    
    // Node is now permitted to communicate
    EXPECT_TRUE(info.openlcb_node->state.permitted);
    
    // But not yet initialized (needs to send Init Complete)
    EXPECT_FALSE(info.openlcb_node->state.initialized);
}

/*******************************************************************************
 * Additional Coverage Tests (Currently Commented Out)
 ******************************************************************************/

// Uncomment and validate these tests one at a time after validating the above

/*
 * Test: Seed generation produces different values
 * Verifies LFSR doesn't get stuck in a loop
 *
 * */
TEST(CanLoginMessageHandler, seed_generation_varies)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_init(&info);
    
    uint64_t seed1 = info.openlcb_node->seed;
    CanLoginMessageHandler_state_generate_seed(&info);
    uint64_t seed2 = info.openlcb_node->seed;
    CanLoginMessageHandler_state_generate_seed(&info);
    uint64_t seed3 = info.openlcb_node->seed;
    
    // All three seeds should be different
    EXPECT_NE(seed1, seed2);
    EXPECT_NE(seed2, seed3);
    EXPECT_NE(seed1, seed3);
}

/*
 * Test: Alias generation produces valid 12-bit values
 * Verifies alias is within valid range (0x001-0xFFF)
 *
 */
TEST(CanLoginMessageHandler, alias_within_valid_range)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_init(&info);
    
    // Generate multiple aliases
    for (int i = 0; i < 10; i++)
    {
        CanLoginMessageHandler_state_generate_seed(&info);
        info.openlcb_node->alias = 0x00;
        CanLoginMessageHandler_state_generate_alias(&info);
        
        // Alias must be non-zero and ≤ 0xFFF
        EXPECT_GT(info.openlcb_node->alias, 0x000);
        EXPECT_LE(info.openlcb_node->alias, 0xFFF);
    }
}

/*
 * Test: CID sequence contains complete Node ID
 * Verifies that all 48 bits of Node ID are present across CID frames
 *
 */
TEST(CanLoginMessageHandler, cid_sequence_complete_node_id)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    // Collect all CID frame identifiers
    CanLoginMessageHandler_state_load_cid07(&info);
    uint32_t cid7 = info.login_outgoing_can_msg->identifier;
    
    CanLoginMessageHandler_state_load_cid06(&info);
    uint32_t cid6 = info.login_outgoing_can_msg->identifier;
    
    CanLoginMessageHandler_state_load_cid05(&info);
    uint32_t cid5 = info.login_outgoing_can_msg->identifier;
    
    CanLoginMessageHandler_state_load_cid04(&info);
    uint32_t cid4 = info.login_outgoing_can_msg->identifier;
    
    // Extract Node ID fragments from variable field
    uint64_t reconstructed_id = 0;
    reconstructed_id |= (uint64_t)((cid7 >> 12) & 0xFFF) << 36;
    reconstructed_id |= (uint64_t)((cid6 >> 12) & 0xFFF) << 24;
    reconstructed_id |= (uint64_t)((cid5 >> 12) & 0xFFF) << 12;
    reconstructed_id |= (uint64_t)((cid4 >> 12) & 0xFFF) << 0;
    
    EXPECT_EQ(reconstructed_id, NODE_ID);
}

/*
 * Test: Timer reset on CID4
 * Verifies timer is explicitly set to 0 when entering wait state
 *
 */
TEST(CanLoginMessageHandler, timer_reset_on_cid04)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    // Set timer to non-zero
    info.openlcb_node->timerticks = 99;
    
    CanLoginMessageHandler_state_load_cid04(&info);
    
    // Timer should be reset
    EXPECT_EQ(info.openlcb_node->timerticks, 0);
}

/*
 * Test: Alias registration stores correct values
 * Verifies alias mapping is updated during alias generation
 *
 */
TEST(CanLoginMessageHandler, alias_registration)
{
    can_statemachine_info_t info;
    
    setup_test(&interface_can_login_message_handler);
    reset_test_variables();
    initialize_statemachine_info(&info);
    
    CanLoginMessageHandler_state_init(&info);
    info.openlcb_node->alias = 0x00;
    
    // Clear mapping
    alias_mapping.alias = 0;
    alias_mapping.node_id = 0;
    
    CanLoginMessageHandler_state_generate_alias(&info);
    
    // Mapping should be registered
    EXPECT_EQ(alias_mapping.alias, info.openlcb_node->alias);
    EXPECT_EQ(alias_mapping.node_id, info.openlcb_node->id);
    EXPECT_FALSE(alias_mapping.is_duplicate);
    EXPECT_FALSE(alias_mapping.is_permitted);
}

/*******************************************************************************
 * End of Test Suite
 ******************************************************************************/
