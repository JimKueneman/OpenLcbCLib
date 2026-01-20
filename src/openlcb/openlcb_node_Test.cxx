/** \copyright
* Copyright (c) 2024, Jim Kueneman
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
* @file openlcb_node_Test.cxx
* @brief Comprehensive test suite for OpenLCB node management
* @details Tests cover all public functions, edge cases, and dependency injection scenarios
*
* Test Organization:
* - Section 1: Existing Tests (16 tests) - Active and validated
* - Section 2: New Dependency Injection Tests (2 tests) - Commented, test incrementally
* - Section 3: New Detailed Coverage Tests (11 tests) - Commented, test incrementally
*
* Expected Coverage with All Tests: ~99-100%
*
* @author Jim Kueneman
* @date 20 Jan 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_node.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"

// ============================================================================
// Test Node Parameters
// ============================================================================

node_parameters_t _node_parameters_main_node = {

    // Force overruns for test (autocreate more than buffer allows)
    .consumer_count_autocreate = USER_DEFINED_CONSUMER_COUNT + 1,
    .producer_count_autocreate = USER_DEFINED_PRODUCER_COUNT + 1,

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
    .configuration_options.description = "These are options that defined the memory space capabilities",

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

// ============================================================================
// Test Control Variables
// ============================================================================

bool on_100ms_timer_tick_called = false;

// ============================================================================
// Mock Interface Functions
// ============================================================================

/**
 * @brief Mock callback for 100ms timer tick
 * @details Sets flag to verify callback was invoked
 */
void _on_100ms_timer_tick(void)
{
    on_100ms_timer_tick_called = true;
}

/**
 * @brief Interface with valid callback function
 * @details Used for testing normal operation with dependency injection
 */
const interface_openlcb_node_t interface_with_callback = {
    .on_100ms_timer_tick = &_on_100ms_timer_tick
};

/**
 * @brief Interface with NULL callback function
 * @details Used for testing NULL callback handling
 */
const interface_openlcb_node_t interface_with_null_callback = {
    .on_100ms_timer_tick = nullptr
};

// ============================================================================
// Test Helpers
// ============================================================================

/**
 * @brief Initialize module with valid interface callback
 */
void _global_initialize(void)
{
    OpenLcbNode_initialize(&interface_with_callback);
}

/**
 * @brief Initialize module with NULL callback
 */
void _global_initialize_null_callback(void)
{
    OpenLcbNode_initialize(&interface_with_null_callback);
}

/**
 * @brief Initialize module with NULL interface pointer
 */
void _global_initialize_null_interface(void)
{
    OpenLcbNode_initialize(nullptr);
}

/**
 * @brief Reset test control variables
 */
void _reset_variables(void)
{
    on_100ms_timer_tick_called = false;
}

// ============================================================================
// SECTION 1: EXISTING TESTS - Active and Validated (16 tests)
// ============================================================================

// ============================================================================
// TEST: Initialization with Valid Interface
// @details Verifies basic initialization, allocation, and enumeration
// ============================================================================

TEST(OpenLcbNode, initialization)
{
    _global_initialize();
    _reset_variables();

    // Verify empty node list after initialization
    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), nullptr);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), nullptr);

    // Allocate two nodes
    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0xAAA;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x777;

    // Verify enumeration works
    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_2), node1);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_2), node2);

    // Verify invalid key returns NULL
    EXPECT_EQ(OpenLcbNode_get_first(MAX_NODE_ENUM_KEY_VALUES), nullptr);
    EXPECT_EQ(OpenLcbNode_get_next(MAX_NODE_ENUM_KEY_VALUES), nullptr);
}

// ============================================================================
// TEST: Buffer Full Condition
// @details Verifies allocation fails when buffer is full
// ============================================================================

TEST(OpenLcbNode, buffer_full)
{
    _global_initialize();
    _reset_variables();

    node_id_t node_id = 0x010203040506;
    openlcb_node_t *openlcb_node;

    // Allocate until buffer is full
    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++)
    {
        openlcb_node = OpenLcbNode_allocate(node_id, &_node_parameters_main_node);
        EXPECT_NE(openlcb_node, nullptr);
        node_id++;
    }

    // Next allocation should fail (buffer full)
    openlcb_node = OpenLcbNode_allocate(node_id, &_node_parameters_main_node);
    EXPECT_EQ(openlcb_node, nullptr);
}

// ============================================================================
// TEST: 100ms Timer Tick with Valid Callback
// @details Verifies timer ticks increment and callback is invoked
// ============================================================================

TEST(OpenLcbNode, timer_100ms_tick)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0xAAA;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x777;

    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_3), node1);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_4), node2);

    // Initial timer ticks should be zero
    EXPECT_EQ(node1->timerticks, 0);
    EXPECT_EQ(node2->timerticks, 0);

    // Call timer tick 5 times
    OpenLcbNode_100ms_timer_tick();
    EXPECT_TRUE(on_100ms_timer_tick_called);
    OpenLcbNode_100ms_timer_tick();
    OpenLcbNode_100ms_timer_tick();
    OpenLcbNode_100ms_timer_tick();
    OpenLcbNode_100ms_timer_tick();

    // Verify timer ticks incremented for both nodes
    EXPECT_EQ(node1->timerticks, 5);
    EXPECT_EQ(node2->timerticks, 5);
}

// ============================================================================
// TEST: Find by Alias
// @details Verifies node lookup by CAN alias
// ============================================================================

TEST(OpenLcbNode, find_by_alias)
{
    _global_initialize();
    _reset_variables();

    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_4), nullptr);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_4), nullptr);

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0xAAA;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x777;

    // Find existing aliases
    EXPECT_EQ(OpenLcbNode_find_by_alias(0xAAA), node1);
    EXPECT_EQ(OpenLcbNode_find_by_alias(0x777), node2);
    
    // Find non-existent alias
    EXPECT_EQ(OpenLcbNode_find_by_alias(0x766), nullptr);
}

// ============================================================================
// TEST: Find by Node ID
// @details Verifies node lookup by 48-bit node ID
// ============================================================================

TEST(OpenLcbNode, find_by_node_id)
{
    _global_initialize();
    _reset_variables();

    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), nullptr);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), nullptr);

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0xAAA;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x777;

    // Find existing node IDs
    EXPECT_EQ(OpenLcbNode_find_by_node_id(0x010203040506), node1);
    EXPECT_EQ(OpenLcbNode_find_by_node_id(0x010203040507), node2);
    
    // Find non-existent node ID
    EXPECT_EQ(OpenLcbNode_find_by_node_id(0x010203040511), nullptr);
}

// ============================================================================
// TEST: Reset State
// @details Verifies all nodes reset to INIT state
// ============================================================================

TEST(OpenLcbNode, reset_state)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);

    // Set nodes to RUN state
    node1->state.run_state = RUNSTATE_RUN;
    node1->state.permitted = true;
    node1->state.initialized = true;

    node2->state.run_state = RUNSTATE_RUN;
    node2->state.permitted = true;
    node2->state.initialized = true;

    // Reset all nodes
    OpenLcbNode_reset_state();

    // Verify all nodes reset to INIT state
    EXPECT_EQ(node1->state.run_state, RUNSTATE_INIT);
    EXPECT_FALSE(node1->state.permitted);
    EXPECT_FALSE(node1->state.initialized);

    EXPECT_EQ(node2->state.run_state, RUNSTATE_INIT);
    EXPECT_FALSE(node2->state.permitted);
    EXPECT_FALSE(node2->state.initialized);
}

// ============================================================================
// TEST: Get First with Invalid Key
// @details Verifies invalid enumeration key handling
// ============================================================================

TEST(OpenLcbNode, get_first_invalid_key)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node, nullptr);

    // Test with key at boundary
    EXPECT_EQ(OpenLcbNode_get_first(MAX_NODE_ENUM_KEY_VALUES), nullptr);
    
    // Test with key beyond boundary
    EXPECT_EQ(OpenLcbNode_get_first(MAX_NODE_ENUM_KEY_VALUES + 1), nullptr);
}

// ============================================================================
// TEST: Get Next with Invalid Key
// @details Verifies invalid enumeration key handling
// ============================================================================

TEST(OpenLcbNode, get_next_invalid_key)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node, nullptr);

    // Test with key at boundary
    EXPECT_EQ(OpenLcbNode_get_next(MAX_NODE_ENUM_KEY_VALUES), nullptr);
    
    // Test with key beyond boundary
    EXPECT_EQ(OpenLcbNode_get_next(MAX_NODE_ENUM_KEY_VALUES + 1), nullptr);
}

// ============================================================================
// TEST: Get First with Empty Node List
// @details Verifies get_first returns NULL when no nodes allocated
// ============================================================================

TEST(OpenLcbNode, get_first_empty_list)
{
    _global_initialize();
    _reset_variables();

    // No nodes allocated - should return NULL
    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), nullptr);
}

// ============================================================================
// TEST: Get Next at End of List
// @details Verifies get_next returns NULL at end of node list
// ============================================================================

TEST(OpenLcbNode, get_next_end_of_list)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);

    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), node1);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), node2);
    
    // Try to get next beyond end of list
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), nullptr);
}

// ============================================================================
// TEST: Find by Alias - Empty List
// @details Verifies find returns NULL when no nodes exist
// ============================================================================

TEST(OpenLcbNode, find_by_alias_empty_list)
{
    _global_initialize();
    _reset_variables();

    // No nodes allocated - should return NULL
    EXPECT_EQ(OpenLcbNode_find_by_alias(0xAAA), nullptr);
}

// ============================================================================
// TEST: Find by Node ID - Empty List
// @details Verifies find returns NULL when no nodes exist
// ============================================================================

TEST(OpenLcbNode, find_by_node_id_empty_list)
{
    _global_initialize();
    _reset_variables();

    // No nodes allocated - should return NULL
    EXPECT_EQ(OpenLcbNode_find_by_node_id(0x010203040506), nullptr);
}

// ============================================================================
// TEST: Multiple Independent Enumerations
// @details Verifies different enumeration keys work independently
// ============================================================================

TEST(OpenLcbNode, multiple_enumerations)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    openlcb_node_t *node3 = OpenLcbNode_allocate(0x010203040508, &_node_parameters_main_node);

    // Start enumeration with key 0
    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), node1);
    
    // Start independent enumeration with key 1
    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_2), node1);
    
    // Continue key 0 enumeration
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), node2);
    
    // Continue key 1 enumeration (should be independent)
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_2), node2);
    
    // Finish both
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), node3);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_2), node3);
}

// ============================================================================
// TEST: Timer Tick with No Nodes
// @details Verifies timer tick doesn't crash with empty node list
// ============================================================================

TEST(OpenLcbNode, timer_tick_no_nodes)
{
    _global_initialize();
    _reset_variables();

    // Should not crash with no nodes allocated
    OpenLcbNode_100ms_timer_tick();
    EXPECT_TRUE(on_100ms_timer_tick_called);
}

// ============================================================================
// TEST: Reset State with No Nodes
// @details Verifies reset doesn't crash with empty node list
// ============================================================================

TEST(OpenLcbNode, reset_state_no_nodes)
{
    _global_initialize();
    _reset_variables();

    // Should not crash with no nodes allocated
    OpenLcbNode_reset_state();
}

// ============================================================================
// TEST: Allocate Duplicate Node ID
// @details Verifies multiple nodes can have same ID (virtual nodes)
// ============================================================================

TEST(OpenLcbNode, allocate_duplicate_node_id)
{
    _global_initialize();
    _reset_variables();

    uint64_t same_id = 0x010203040506;
    
    openlcb_node_t *node1 = OpenLcbNode_allocate(same_id, &_node_parameters_main_node);
    ASSERT_NE(node1, nullptr);
    
    // Should be able to allocate another node with same ID (different virtual node)
    openlcb_node_t *node2 = OpenLcbNode_allocate(same_id, &_node_parameters_main_node);
    EXPECT_NE(node2, nullptr);
    EXPECT_NE(node1, node2);  // Different node pointers
    EXPECT_EQ(node1->id, node2->id);  // Same node ID
}

// ============================================================================
// SECTION 2: NEW DEPENDENCY INJECTION TESTS (2 tests)
// @details Tests NULL interface and NULL callback handling
// @note Uncomment one test at a time to validate
// ============================================================================

/*
// ============================================================================
// TEST: Initialization with NULL Interface Pointer
// @details Tests that module handles NULL interface pointer safely
// @coverage Covers _interface NULL check in OpenLcbNode_100ms_timer_tick
// ============================================================================

TEST(OpenLcbNode, initialize_null_interface)
{
    _global_initialize_null_interface();
    _reset_variables();

    // Should initialize successfully even with NULL interface
    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), nullptr);

    // Allocate a node
    openlcb_node_t *node = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node, nullptr);

    // Timer tick should work without crashing (NULL interface check)
    OpenLcbNode_100ms_timer_tick();
    
    // Callback should NOT be called (NULL interface)
    EXPECT_FALSE(on_100ms_timer_tick_called);
    
    // Timer ticks should still increment
    EXPECT_EQ(node->timerticks, 1);
}
*/

/*
// ============================================================================
// TEST: Timer Tick with NULL Callback Function
// @details Tests that module handles NULL callback function safely
// @coverage Covers on_100ms_timer_tick NULL check in OpenLcbNode_100ms_timer_tick
// ============================================================================

TEST(OpenLcbNode, timer_tick_null_callback)
{
    _global_initialize_null_callback();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node, nullptr);

    // Initial timer tick
    EXPECT_EQ(node->timerticks, 0);

    // Timer tick should work without crashing (NULL callback check)
    OpenLcbNode_100ms_timer_tick();
    
    // Callback should NOT be called (NULL callback)
    EXPECT_FALSE(on_100ms_timer_tick_called);
    
    // Timer ticks should still increment
    EXPECT_EQ(node->timerticks, 1);
}
*/

// ============================================================================
// SECTION 3: NEW DETAILED COVERAGE TESTS (11 tests)
// @details Additional tests for comprehensive branch and edge case coverage
// @note Uncomment one test at a time to validate
// ============================================================================

/*
// ============================================================================
// TEST: Node Allocation - Verify Initial State
// @details Verifies all node fields are properly initialized after allocation
// @coverage Tests that _clear_node properly initializes all fields
// ============================================================================

TEST(OpenLcbNode, allocate_verify_initial_state)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x0A0B0C0D0E0F, &_node_parameters_main_node);
    ASSERT_NE(node, nullptr);

    // Verify node is properly initialized
    EXPECT_EQ(node->id, 0x0A0B0C0D0E0F);
    EXPECT_EQ(node->alias, 0);  // Not assigned yet
    EXPECT_EQ(node->state.run_state, RUNSTATE_INIT);
    EXPECT_TRUE(node->state.allocated);
    EXPECT_FALSE(node->state.initialized);
    EXPECT_FALSE(node->state.permitted);
    EXPECT_FALSE(node->state.duplicate_id_detected);
    EXPECT_EQ(node->timerticks, 0);
    EXPECT_EQ(node->parameters, &_node_parameters_main_node);
}
*/

/*
// ============================================================================
// TEST: Node Allocation - Verify Index Assignment
// @details Verifies that node index is correctly assigned sequentially
// @coverage Tests index assignment in OpenLcbNode_allocate
// ============================================================================

TEST(OpenLcbNode, allocate_verify_index)
{
    _global_initialize();
    _reset_variables();

    // Allocate multiple nodes and verify index assignment
    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    ASSERT_NE(node1, nullptr);
    EXPECT_EQ(node1->index, 0);

    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    ASSERT_NE(node2, nullptr);
    EXPECT_EQ(node2->index, 1);

    openlcb_node_t *node3 = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    ASSERT_NE(node3, nullptr);
    EXPECT_EQ(node3->index, 2);
}
*/

/*
// ============================================================================
// TEST: Get First - Verify Index Reset
// @details Verifies that get_first resets enumeration index each time
// @coverage Tests _node_enum_index_array reset in OpenLcbNode_get_first
// ============================================================================

TEST(OpenLcbNode, get_first_resets_index)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);

    // First enumeration
    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), node1);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), node2);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), nullptr);

    // Second enumeration should restart from beginning
    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), node1);
    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), node2);
}
*/

/*
// ============================================================================
// TEST: Find by Alias - First Node Match
// @details Verifies finding first node in list by alias
// @coverage Tests early return path in OpenLcbNode_find_by_alias
// ============================================================================

TEST(OpenLcbNode, find_by_alias_first_node)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node1, nullptr);
    node1->alias = 0x111;
    
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    ASSERT_NE(node2, nullptr);
    node2->alias = 0x222;

    // Find first node by alias
    EXPECT_EQ(OpenLcbNode_find_by_alias(0x111), node1);
}
*/

/*
// ============================================================================
// TEST: Find by Alias - Last Node Match
// @details Verifies finding last node in list by alias
// @coverage Tests full iteration in OpenLcbNode_find_by_alias
// ============================================================================

TEST(OpenLcbNode, find_by_alias_last_node)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node1, nullptr);
    node1->alias = 0x111;
    
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    ASSERT_NE(node2, nullptr);
    node2->alias = 0x222;
    
    openlcb_node_t *node3 = OpenLcbNode_allocate(0x010203040508, &_node_parameters_main_node);
    ASSERT_NE(node3, nullptr);
    node3->alias = 0x333;

    // Find last node by alias - ensures we iterate through all nodes
    EXPECT_EQ(OpenLcbNode_find_by_alias(0x333), node3);
}
*/

/*
// ============================================================================
// TEST: Find by Node ID - First Node Match
// @details Verifies finding first node in list by ID
// @coverage Tests early return path in OpenLcbNode_find_by_node_id
// ============================================================================

TEST(OpenLcbNode, find_by_node_id_first_node)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node1, nullptr);
    
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    ASSERT_NE(node2, nullptr);

    // Find first node by ID - ensures early return works
    EXPECT_EQ(OpenLcbNode_find_by_node_id(0x010203040506), node1);
    
    // Also verify second node is findable
    EXPECT_EQ(OpenLcbNode_find_by_node_id(0x010203040507), node2);
}
*/

/*
// ============================================================================
// TEST: Find by Node ID - Last Node Match
// @details Verifies finding last node in list by ID
// @coverage Tests full iteration in OpenLcbNode_find_by_node_id
// ============================================================================

TEST(OpenLcbNode, find_by_node_id_last_node)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node1, nullptr);
    
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    ASSERT_NE(node2, nullptr);
    
    openlcb_node_t *node3 = OpenLcbNode_allocate(0x010203040508, &_node_parameters_main_node);
    ASSERT_NE(node3, nullptr);

    // Find last node by ID - ensures we iterate through all nodes
    EXPECT_EQ(OpenLcbNode_find_by_node_id(0x010203040508), node3);
}
*/

/*
// ============================================================================
// TEST: Reset State - Verify Only Specific Fields Reset
// @details Verifies that reset_state only clears run_state, permitted, initialized
// @coverage Tests selective field reset in OpenLcbNode_reset_state
// ============================================================================

TEST(OpenLcbNode, reset_state_partial_reset)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node, nullptr);
    
    // Set various node fields
    node->state.run_state = RUNSTATE_RUN;
    node->state.permitted = true;
    node->state.initialized = true;
    node->state.allocated = true;  // Should NOT be cleared
    node->alias = 0xAAA;  // Should NOT be cleared
    node->id = 0x010203040506;  // Should NOT be cleared
    node->timerticks = 100;  // Should NOT be cleared

    // Reset state
    OpenLcbNode_reset_state();

    // Verify only specific fields were reset
    EXPECT_EQ(node->state.run_state, RUNSTATE_INIT);
    EXPECT_FALSE(node->state.permitted);
    EXPECT_FALSE(node->state.initialized);
    
    // Verify these fields were NOT changed
    EXPECT_TRUE(node->state.allocated);
    EXPECT_EQ(node->alias, 0xAAA);
    EXPECT_EQ(node->id, 0x010203040506);
    EXPECT_EQ(node->timerticks, 100);
}
*/

/*
// ============================================================================
// TEST: Timer Tick - Multiple Calls Accumulate
// @details Verifies that timer ticks accumulate correctly over multiple calls
// @coverage Tests timer tick increment logic in OpenLcbNode_100ms_timer_tick
// ============================================================================

TEST(OpenLcbNode, timer_tick_accumulation)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node, nullptr);

    // Call timer tick multiple times and verify accumulation
    for (int i = 1; i <= 10; i++)
    {
        OpenLcbNode_100ms_timer_tick();
        EXPECT_EQ(node->timerticks, i);
    }
}
*/

/*
// ============================================================================
// TEST: Enumeration with All Keys
// @details Verifies that all enumeration keys work independently
// @coverage Tests all valid key values in OpenLcbNode_get_first/get_next
// ============================================================================

TEST(OpenLcbNode, enumerate_all_keys)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node, nullptr);

    // Test all valid enumeration keys
    for (uint8_t key = 0; key < MAX_NODE_ENUM_KEY_VALUES; key++)
    {
        EXPECT_EQ(OpenLcbNode_get_first(key), node);
        EXPECT_EQ(OpenLcbNode_get_next(key), nullptr);
    }
}
*/
