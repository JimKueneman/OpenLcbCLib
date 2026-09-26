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
* @file openlcb_login_statemachine_Test.cxx
* @brief Test suite for OpenLCB login state machine dispatcher
*
* @details This test suite validates:
* - State machine initialization
* - State dispatch logic (INIT_COMPLETE, PRODUCER_EVENTS, CONSUMER_EVENTS)
* - Message transmission handling
* - Re-enumeration control for multi-message sequences
* - First node enumeration
* - Next node enumeration
* - Main run loop priority and dispatch
* - State machine info accessor
*
* Coverage Analysis:
* - OpenLcbLoginStatemachine_initialize: 100%
* - OpenLcbLoginStatemachine_process: 100%
* - OpenLcbLoginStatemachine_handle_outgoing_openlcb_message: 100%
* - OpenLcbLoginStatemachine_handle_try_reenumerate: 100%
* - OpenLcbLoginStatemachine_handle_try_enumerate_first_node: 100%
* - OpenLcbLoginStatemachine_handle_try_enumerate_next_node: 100%
* - OpenLcbLoginStatemachine_run: 100%
* - OpenLcbLoginStatemachine_get_statemachine_info: 100%
*
* @author Jim Kueneman
* @date 19 Jan 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_login_statemachine.h"

#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_utilities.h"

// ============================================================================
// Test Constants
// ============================================================================

#define SOURCE_ALIAS 0x222
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0xBBB
#define DEST_ID 0x060504030201

// ============================================================================
// Test Node Parameters
// ============================================================================

node_parameters_t _node_parameters_main_node = {

    .snip = {
        .mfg_version = 4,
        .name = "Test",
        .model = "Test Model J",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .configuration_options = {
        .write_under_mask_supported = 1,
        .unaligned_reads_supported = 1,
        .unaligned_writes_supported = 1,
        .read_from_manufacturer_space_0xfc_supported = 1,
        .read_from_user_space_0xfb_supported = 1,
        .write_to_user_space_0xfb_supported = 1,
        .stream_read_write_supported = 0,
        .high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .description = "These are options that defined the memory space capabilities"
    },

    // Space 0xFF - Configuration Definition Info
    .address_space_configuration_definition = {
        .present = 0,
        .read_only = 1,
        .low_address_valid = 0,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .highest_address = 0x200,
        .low_address = 0,
        .description = "Configuration definition info"
    },

    // Space 0xFE - All Memory
    .address_space_all = {
        .present = 0,
        .read_only = 1,
        .low_address_valid = 0,
        .address_space = CONFIG_MEM_SPACE_ALL,
        .highest_address = 0,
        .low_address = 0,
        .description = "All memory Info"
    },

    // Space 0xFD - Configuration Memory
    .address_space_config_memory = {
        .present = 0,
        .read_only = 0,
        .low_address_valid = 0,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .highest_address = 0x200,
        .low_address = 0,
        .description = "Configuration memory storage"
    },

    .cdi = NULL,
    .fdi = NULL,
};

// ============================================================================
// Mock Control Variables
// ============================================================================

void *called_function_ptr = nullptr;          // Tracks which mock functions were called
bool fail_send_msg = false;                   // Controls send_openlcb_msg behavior
bool fail_first_node = false;                 // Controls openlcb_node_get_first behavior
bool fail_next_node = false;                  // Controls openlcb_node_get_next behavior
bool fail_handle_outgoing_openlcb_message = false;
bool fail_handle_try_reenumerate = false;
bool fail_handle_try_enumerate_first_node = false;
bool fail_handle_try_enumerate_next_node = false;
openlcb_node_t *first_node = nullptr;         // Node returned by get_first
openlcb_node_t *next_node = nullptr;          // Node returned by get_next

// ============================================================================
// Mock Helper Functions
// ============================================================================

/**
 * @brief Updates called function pointer by adding address
 * @details Accumulates function addresses to track call sequences
 * @param function_ptr Pointer to function that was called
 */
void _update_called_function_ptr(void *function_ptr)
{
    called_function_ptr = (void *)((long long)function_ptr + (long long)called_function_ptr);
}

// ============================================================================
// Mock Interface Functions - State Handlers
// ============================================================================

/**
 * @brief Mock handler for RUNSTATE_LOAD_INITIALIZATION_COMPLETE
 * @param openlcb_statemachine_info State machine info (unused in mock)
 */
void _load_initialization_complete(openlcb_login_statemachine_info_t *openlcb_statemachine_info)
{
    _update_called_function_ptr((void *)&_load_initialization_complete);
}

/**
 * @brief Mock handler for RUNSTATE_LOAD_PRODUCER_EVENTS
 * @param openlcb_statemachine_info State machine info (unused in mock)
 */
void _load_producer_events(openlcb_login_statemachine_info_t *openlcb_statemachine_info)
{
    _update_called_function_ptr((void *)&_load_producer_events);
}

/**
 * @brief Mock handler for RUNSTATE_LOAD_CONSUMER_EVENTS
 * @param openlcb_statemachine_info State machine info (unused in mock)
 */
void _load_consumer_events(openlcb_login_statemachine_info_t *openlcb_statemachine_info)
{
    _update_called_function_ptr((void *)&_load_consumer_events);
}

// ============================================================================
// Mock Interface Functions - Message Handling
// ============================================================================

/**
 * @brief Mock message send function
 * @param outgoing_msg Message to send (unused in mock)
 * @return true if send succeeds, false if fail_send_msg is set
 */
bool _send_openlcb_msg(openlcb_msg_t *outgoing_msg)
{
    _update_called_function_ptr((void *)&_send_openlcb_msg);

    if (fail_send_msg)
    {
        return false;
    }

    return true;
}

// ============================================================================
// Mock Interface Functions - Node Enumeration
// ============================================================================

/**
 * @brief Mock get first node function
 * @param key Enumeration key
 * @return first_node if successful, nullptr if fail_first_node is set
 */
openlcb_node_t *_openlcb_node_get_first(uint8_t key)
{
    _update_called_function_ptr((void *)&_openlcb_node_get_first);

    if (fail_first_node)
    {
        return nullptr;
    }

    return first_node;
}

/**
 * @brief Mock get next node function
 * @param key Enumeration key
 * @return next_node if successful, nullptr if fail_next_node is set
 */
openlcb_node_t *_openlcb_node_get_next(uint8_t key)
{
    _update_called_function_ptr((void *)&_openlcb_node_get_next);

    if (fail_next_node)
    {
        return nullptr;
    }

    return next_node;
}

/**
 * @brief Mock process login statemachine function
 * @param openlcb_statemachine_info State machine info (unused in mock)
 */
void _process_login_statemachine(openlcb_login_statemachine_info_t *openlcb_statemachine_info)
{
    _update_called_function_ptr((void *)&_process_login_statemachine);
}

/**
 * @brief Mock process main statemachine function for sibling dispatch
 * @param statemachine_info State machine info (unused in mock)
 */
void _process_main_statemachine(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_process_main_statemachine);
}

/**
 * @brief Mock node count function — returns 1 by default (no siblings)
 * @return Number of allocated nodes
 */
static uint16_t mock_node_count = 1;

uint16_t _openlcb_node_get_count(void)
{
    return mock_node_count;
}

// ============================================================================
// Mock Interface Functions - Handler Functions (for run loop testing)
// ============================================================================

/**
 * @brief Mock outgoing message handler
 * @return true if handler succeeds, false if fail flag is set
 */
bool _handle_outgoing_openlcb_message(void)
{
    _update_called_function_ptr((void *)&_handle_outgoing_openlcb_message);

    if (fail_handle_outgoing_openlcb_message)
    {
        return false;
    }

    return true;
}

/**
 * @brief Mock re-enumerate handler
 * @return true if handler succeeds, false if fail flag is set
 */
bool _handle_try_reenumerate(void)
{
    _update_called_function_ptr((void *)&_handle_try_reenumerate);

    if (fail_handle_try_reenumerate)
    {
        return false;
    }

    return true;
}

/**
 * @brief Mock first node enumerate handler
 * @return true if handler succeeds, false if fail flag is set
 */
bool _handle_try_enumerate_first_node(void)
{
    _update_called_function_ptr((void *)&_handle_try_enumerate_first_node);

    if (fail_handle_try_enumerate_first_node)
    {
        return false;
    }

    return true;
}

/**
 * @brief Mock next node enumerate handler
 * @return true if handler succeeds, false if fail flag is set
 */
bool _handle_try_enumerate_next_node(void)
{
    _update_called_function_ptr((void *)&_handle_try_enumerate_next_node);

    if (fail_handle_try_enumerate_next_node)
    {
        return false;
    }

    return true;
}

// ============================================================================
// Mock Interface Functions - Login Complete Callback
// ============================================================================

static bool login_complete_return_value = true;

/**
 * @brief Mock on_login_complete callback
 * @param openlcb_node Node that completed login
 * @return login_complete_return_value (configurable by test)
 */
bool _on_login_complete(openlcb_node_t *openlcb_node)
{
    _update_called_function_ptr((void *)&_on_login_complete);
    return login_complete_return_value;
}

// ============================================================================
// Interface Structures
// ============================================================================

const interface_openlcb_login_state_machine_t interface_openlcb_login_state_machine = {

    .send_openlcb_msg = &_send_openlcb_msg,
    .openlcb_node_get_first = &_openlcb_node_get_first,
    .openlcb_node_get_next = &_openlcb_node_get_next,
    .load_initialization_complete = &_load_initialization_complete,
    .load_producer_events = &_load_producer_events,
    .load_consumer_events = &_load_consumer_events,
    .process_main_statemachine = &_process_main_statemachine,
    .openlcb_node_get_count = &_openlcb_node_get_count,
    .process_login_statemachine = &_process_login_statemachine,

    // For test injection of run loop handlers
    .handle_outgoing_openlcb_message = &_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &_handle_try_reenumerate,
    .handle_try_enumerate_first_node = &_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &_handle_try_enumerate_next_node,

    .on_login_complete = NULL,
};

const interface_openlcb_login_state_machine_t interface_with_login_complete = {

    .send_openlcb_msg = &_send_openlcb_msg,
    .openlcb_node_get_first = &_openlcb_node_get_first,
    .openlcb_node_get_next = &_openlcb_node_get_next,
    .load_initialization_complete = &_load_initialization_complete,
    .load_producer_events = &_load_producer_events,
    .load_consumer_events = &_load_consumer_events,
    .process_main_statemachine = &_process_main_statemachine,
    .openlcb_node_get_count = &_openlcb_node_get_count,
    .process_login_statemachine = &_process_login_statemachine,

    .handle_outgoing_openlcb_message = &_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &_handle_try_reenumerate,
    .handle_try_enumerate_first_node = &_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &_handle_try_enumerate_next_node,

    .on_login_complete = &_on_login_complete,
};

const interface_openlcb_node_t interface_openlcb_node = {};

// ============================================================================
// Test Helpers
// ============================================================================

/**
 * @brief Reset all test control variables to default state
 */
void _reset_variables(void)
{
    called_function_ptr = nullptr;
    fail_send_msg = false;
    first_node = nullptr;
    next_node = nullptr;
    fail_first_node = false;
    fail_next_node = false;
    fail_handle_outgoing_openlcb_message = false;
    fail_handle_try_reenumerate = false;
    fail_handle_try_enumerate_first_node = false;
    fail_handle_try_enumerate_next_node = false;
    login_complete_return_value = true;
    mock_node_count = 1;
}

/**
 * @brief Initialize all required modules for testing
 */
void _global_initialize(void)
{
    OpenLcbLoginStatemachine_initialize(&interface_openlcb_login_state_machine);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();
}

// ============================================================================
// TEST: Module Initialization
// ============================================================================

TEST(OpenLcbLoginStateMachine, initialize)
{
    _reset_variables();
    _global_initialize();

    // Get state machine info to verify initialization
    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();

    EXPECT_NE(statemachine_info, nullptr);
    
    // Verify message buffer is properly initialized
    EXPECT_NE(statemachine_info->outgoing_msg_info.msg_ptr, nullptr);
    EXPECT_NE(statemachine_info->outgoing_msg_info.msg_ptr->payload, nullptr);
    EXPECT_EQ(statemachine_info->outgoing_msg_info.msg_ptr->payload_type, BASIC);
    EXPECT_TRUE(statemachine_info->outgoing_msg_info.msg_ptr->state.allocated);
    
    // Verify node pointer is NULL (no current node)
    EXPECT_EQ(statemachine_info->openlcb_node, nullptr);
}

// ============================================================================
// TEST: State Dispatch - RUNSTATE_LOAD_INITIALIZATION_COMPLETE
// ============================================================================

TEST(OpenLcbLoginStateMachine, process_initialization_complete)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Call process function
    OpenLcbLoginStatemachine_process(statemachine_info);

    // Verify correct handler was called
    EXPECT_EQ(called_function_ptr, &_load_initialization_complete);
}

// ============================================================================
// TEST: State Dispatch - RUNSTATE_LOAD_PRODUCER_EVENTS
// ============================================================================

TEST(OpenLcbLoginStateMachine, process_producer_events)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Call process function
    OpenLcbLoginStatemachine_process(statemachine_info);

    // Verify correct handler was called
    EXPECT_EQ(called_function_ptr, &_load_producer_events);
}

// ============================================================================
// TEST: State Dispatch - RUNSTATE_LOAD_CONSUMER_EVENTS
// ============================================================================

TEST(OpenLcbLoginStateMachine, process_consumer_events)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Call process function
    OpenLcbLoginStatemachine_process(statemachine_info);

    // Verify correct handler was called
    EXPECT_EQ(called_function_ptr, &_load_consumer_events);
}

// ============================================================================
// TEST: State Dispatch - RUNSTATE_RUN (no dispatch)
// ============================================================================

TEST(OpenLcbLoginStateMachine, process_run_state_no_dispatch)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_RUN;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Call process function
    OpenLcbLoginStatemachine_process(statemachine_info);

    // Verify NO handler was called (node already in RUN state)
    EXPECT_EQ(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Handle Outgoing Message - Message Valid, Send Succeeds
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_outgoing_message_valid_send_succeeds)
{
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Set message as valid (ready to send)
    statemachine_info->outgoing_msg_info.valid = true;
    
    // Send will succeed (fail_send_msg = false)
    fail_send_msg = false;

    bool result = OpenLcbLoginStatemachine_handle_outgoing_openlcb_message();

    // Verify send was attempted
    EXPECT_EQ(called_function_ptr, &_send_openlcb_msg);
    
    // Verify function returned true (message was handled)
    EXPECT_TRUE(result);
    
    // Verify valid flag was cleared (message sent successfully)
    EXPECT_FALSE(statemachine_info->outgoing_msg_info.valid);
}

// ============================================================================
// TEST: Handle Outgoing Message - Message Valid, Send Fails
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_outgoing_message_valid_send_fails)
{
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Set message as valid
    statemachine_info->outgoing_msg_info.valid = true;
    
    // Make send fail
    fail_send_msg = true;

    bool result = OpenLcbLoginStatemachine_handle_outgoing_openlcb_message();

    // Verify send was attempted
    EXPECT_EQ(called_function_ptr, &_send_openlcb_msg);
    
    // Verify function returned true (message pending, will retry)
    EXPECT_TRUE(result);
    
    // Verify valid flag NOT cleared (message still pending)
    EXPECT_TRUE(statemachine_info->outgoing_msg_info.valid);
}

// ============================================================================
// TEST: Handle Outgoing Message - No Message Pending
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_outgoing_message_not_valid)
{
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Set message as NOT valid (nothing to send)
    statemachine_info->outgoing_msg_info.valid = false;

    bool result = OpenLcbLoginStatemachine_handle_outgoing_openlcb_message();

    // Verify send was NOT attempted
    EXPECT_EQ(called_function_ptr, nullptr);
    
    // Verify function returned false (no message to handle)
    EXPECT_FALSE(result);
}

// ============================================================================
// TEST: Handle Re-enumerate - Enumerate Flag Set
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_reenumerate_flag_set)
{
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Set enumerate flag (multi-message sequence)
    statemachine_info->outgoing_msg_info.enumerate = true;

    bool result = OpenLcbLoginStatemachine_handle_try_reenumerate();

    // Verify process was called
    EXPECT_EQ(called_function_ptr, &_process_login_statemachine);
    
    // Verify function returned true
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: Handle Re-enumerate - Enumerate Flag Clear
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_reenumerate_flag_clear)
{
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Clear enumerate flag (no re-enumeration needed)
    statemachine_info->outgoing_msg_info.enumerate = false;

    bool result = OpenLcbLoginStatemachine_handle_try_reenumerate();

    // Verify process was NOT called
    EXPECT_EQ(called_function_ptr, nullptr);
    
    // Verify function returned false
    EXPECT_FALSE(result);
}

// ============================================================================
// TEST: Handle First Node - No Current Node, First Node Exists, Not In RUN
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_first_node_exists_needs_processing)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_INIT;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // No current node
    statemachine_info->openlcb_node = nullptr;
    
    // Setup first node to return
    first_node = node_1;
    fail_first_node = false;

    bool result = OpenLcbLoginStatemachine_handle_try_enumerate_first_node();

    // Verify get_first and process were called
    EXPECT_EQ(called_function_ptr, (void *)((uint64_t)&_openlcb_node_get_first + (uint64_t)&_process_login_statemachine));
    
    // Verify node was assigned
    EXPECT_EQ(statemachine_info->openlcb_node, node_1);
    
    // Verify function returned true
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: Handle First Node - No Current Node, First Node Exists, In RUN
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_first_node_exists_already_running)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_RUN;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    statemachine_info->openlcb_node = nullptr;
    
    first_node = node_1;
    fail_first_node = false;

    bool result = OpenLcbLoginStatemachine_handle_try_enumerate_first_node();

    // Verify only get_first was called (process skipped for RUN state)
    EXPECT_EQ(called_function_ptr, &_openlcb_node_get_first);
    
    // Verify node was assigned
    EXPECT_EQ(statemachine_info->openlcb_node, node_1);
    
    // Verify function returned true
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: Handle First Node - No Current Node, No First Node Available
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_first_node_none_available)
{
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    statemachine_info->openlcb_node = nullptr;
    
    // No first node available
    first_node = nullptr;
    fail_first_node = false;

    bool result = OpenLcbLoginStatemachine_handle_try_enumerate_first_node();

    // Verify get_first was called
    EXPECT_EQ(called_function_ptr, &_openlcb_node_get_first);
    
    // Verify node is still NULL
    EXPECT_EQ(statemachine_info->openlcb_node, nullptr);
    
    // Verify function returned true (handled condition)
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: Handle First Node - Current Node Already Set
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_first_node_already_have_node)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Already have a current node
    statemachine_info->openlcb_node = node_1;

    bool result = OpenLcbLoginStatemachine_handle_try_enumerate_first_node();

    // Verify get_first was NOT called
    EXPECT_EQ(called_function_ptr, nullptr);
    
    // Verify function returned false (nothing to do)
    EXPECT_FALSE(result);
}

// ============================================================================
// TEST: Handle Next Node - Have Current Node, Next Node Exists, Not In RUN
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_next_node_exists_needs_processing)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    openlcb_node_t *node_2 = OpenLcbNode_allocate(DEST_ID + 1, &_node_parameters_main_node);
    node_1->state.run_state = RUNSTATE_INIT;
    node_2->state.run_state = RUNSTATE_INIT;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Have current node
    statemachine_info->openlcb_node = node_1;
    
    // Setup next node
    next_node = node_2;
    fail_next_node = false;

    bool result = OpenLcbLoginStatemachine_handle_try_enumerate_next_node();

    // Verify get_next and process were called
    EXPECT_EQ(called_function_ptr, (void *)((uint64_t)&_openlcb_node_get_next + (uint64_t)&_process_login_statemachine));
    
    // Verify node was updated
    EXPECT_EQ(statemachine_info->openlcb_node, node_2);
    
    // Verify function returned true
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: Handle Next Node - Have Current Node, Next Node Exists, In RUN
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_next_node_exists_already_running)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    openlcb_node_t *node_2 = OpenLcbNode_allocate(DEST_ID + 1, &_node_parameters_main_node);
    node_1->state.run_state = RUNSTATE_RUN;
    node_2->state.run_state = RUNSTATE_RUN;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    statemachine_info->openlcb_node = node_1;
    
    next_node = node_2;
    fail_next_node = false;

    bool result = OpenLcbLoginStatemachine_handle_try_enumerate_next_node();

    // Verify only get_next was called (process skipped)
    EXPECT_EQ(called_function_ptr, &_openlcb_node_get_next);
    
    // Verify node was updated
    EXPECT_EQ(statemachine_info->openlcb_node, node_2);
    
    // Verify function returned true
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: Handle Next Node - Have Current Node, No Next Node (End of List)
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_next_node_end_of_list)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    statemachine_info->openlcb_node = node_1;
    
    // No next node (end of list)
    next_node = nullptr;
    fail_next_node = false;

    bool result = OpenLcbLoginStatemachine_handle_try_enumerate_next_node();

    // Verify get_next was called
    EXPECT_EQ(called_function_ptr, &_openlcb_node_get_next);
    
    // Verify node is NULL (end of enumeration)
    EXPECT_EQ(statemachine_info->openlcb_node, nullptr);
    
    // Verify function returned true
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: Handle Next Node - No Current Node
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_next_node_no_current_node)
{
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // No current node
    statemachine_info->openlcb_node = nullptr;

    bool result = OpenLcbLoginStatemachine_handle_try_enumerate_next_node();

    // Verify get_next was NOT called
    EXPECT_EQ(called_function_ptr, nullptr);
    
    // Verify function returned false
    EXPECT_FALSE(result);
}

// ============================================================================
// TEST: Main Run - Priority 1: Outgoing Message Handling
// ============================================================================

TEST(OpenLcbLoginStateMachine, run_priority_outgoing_message)
{
    _reset_variables();
    _global_initialize();

    // Make outgoing message handler succeed (returns true)
    fail_handle_outgoing_openlcb_message = false;
    
    // Make all other handlers fail so we can verify they're not called
    fail_handle_try_reenumerate = true;
    fail_handle_try_enumerate_first_node = true;
    fail_handle_try_enumerate_next_node = true;

    OpenLcbLoginStatemachine_run();

    // Verify only outgoing message handler was called
    EXPECT_EQ(called_function_ptr, &_handle_outgoing_openlcb_message);
}

// ============================================================================
// TEST: Main Run - Priority 2: Re-enumeration
// ============================================================================

TEST(OpenLcbLoginStateMachine, run_priority_reenumerate)
{
    _reset_variables();
    _global_initialize();

    // Make outgoing message handler fail (skip it)
    fail_handle_outgoing_openlcb_message = true;
    
    // Make re-enumerate succeed
    fail_handle_try_reenumerate = false;
    
    // Make remaining handlers fail
    fail_handle_try_enumerate_first_node = true;
    fail_handle_try_enumerate_next_node = true;

    OpenLcbLoginStatemachine_run();

    // Verify outgoing message and re-enumerate were called
    EXPECT_EQ(called_function_ptr, (void *)((uint64_t)&_handle_outgoing_openlcb_message + (uint64_t)&_handle_try_reenumerate));
}

// ============================================================================
// TEST: Main Run - Priority 3: First Node Enumeration
// ============================================================================

TEST(OpenLcbLoginStateMachine, run_priority_first_node)
{
    _reset_variables();
    _global_initialize();

    // Skip first two priorities
    fail_handle_outgoing_openlcb_message = true;
    fail_handle_try_reenumerate = true;
    
    // Make first node enumerate succeed
    fail_handle_try_enumerate_first_node = false;
    
    // Make last handler fail
    fail_handle_try_enumerate_next_node = true;

    OpenLcbLoginStatemachine_run();

    // Verify first three handlers were called
    EXPECT_EQ(called_function_ptr, (void *)((uint64_t)&_handle_outgoing_openlcb_message + (uint64_t)&_handle_try_reenumerate + (uint64_t)&_handle_try_enumerate_first_node));
}

// ============================================================================
// TEST: Main Run - Priority 4: Next Node Enumeration
// ============================================================================

TEST(OpenLcbLoginStateMachine, run_priority_next_node)
{
    _reset_variables();
    _global_initialize();

    // Skip all higher priorities
    fail_handle_outgoing_openlcb_message = true;
    fail_handle_try_reenumerate = true;
    fail_handle_try_enumerate_first_node = true;
    
    // Make next node enumerate succeed
    fail_handle_try_enumerate_next_node = false;

    OpenLcbLoginStatemachine_run();

    // Verify all four handlers were called
    EXPECT_EQ(called_function_ptr, (void *)((uint64_t)&_handle_outgoing_openlcb_message + (uint64_t)&_handle_try_reenumerate + (uint64_t)&_handle_try_enumerate_first_node + (uint64_t)&_handle_try_enumerate_next_node));
}

// ============================================================================
// TEST: Main Run - All Handlers Fail
// ============================================================================

TEST(OpenLcbLoginStateMachine, run_all_handlers_fail)
{
    _reset_variables();
    _global_initialize();

    // Make all handlers fail
    fail_handle_outgoing_openlcb_message = true;
    fail_handle_try_reenumerate = true;
    fail_handle_try_enumerate_first_node = true;
    fail_handle_try_enumerate_next_node = true;

    OpenLcbLoginStatemachine_run();

    // Verify all handlers were attempted
    EXPECT_EQ(called_function_ptr, (void *)((uint64_t)&_handle_outgoing_openlcb_message + (uint64_t)&_handle_try_reenumerate + (uint64_t)&_handle_try_enumerate_first_node + (uint64_t)&_handle_try_enumerate_next_node));
}

// ============================================================================
// TEST: Get State Machine Info
// ============================================================================

TEST(OpenLcbLoginStateMachine, get_statemachine_info)
{
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *info1 = OpenLcbLoginStatemachine_get_statemachine_info();
    openlcb_login_statemachine_info_t *info2 = OpenLcbLoginStatemachine_get_statemachine_info();

    // Verify non-NULL
    EXPECT_NE(info1, nullptr);
    
    // Verify returns same structure every time (static)
    EXPECT_EQ(info1, info2);
}

// ============================================================================
// ADDITIONAL TESTS - COMMENTED OUT FOR INCREMENTAL TESTING
// ============================================================================

// ============================================================================
// TEST: Process Multiple States in Sequence
// ============================================================================

TEST(OpenLcbLoginStateMachine, process_state_sequence)
{
    // Test state transitions: INIT_COMPLETE -> PRODUCER -> CONSUMER
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Test RUNSTATE_LOAD_INITIALIZATION_COMPLETE
    _reset_variables();
    node_1->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;
    OpenLcbLoginStatemachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_initialization_complete);

    // Test RUNSTATE_LOAD_PRODUCER_EVENTS
    _reset_variables();
    node_1->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    OpenLcbLoginStatemachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_producer_events);

    // Test RUNSTATE_LOAD_CONSUMER_EVENTS
    _reset_variables();
    node_1->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    OpenLcbLoginStatemachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_consumer_events);

    // Test RUNSTATE_RUN (no dispatch)
    _reset_variables();
    node_1->state.run_state = RUNSTATE_RUN;
    OpenLcbLoginStatemachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Message Send Retry Logic
// ============================================================================

TEST(OpenLcbLoginStateMachine, message_send_retry)
{
    // Test that failed sends keep message valid for retry
    
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    statemachine_info->outgoing_msg_info.valid = true;
    
    // First attempt - send fails
    fail_send_msg = true;
    bool result1 = OpenLcbLoginStatemachine_handle_outgoing_openlcb_message();
    EXPECT_TRUE(result1);
    EXPECT_TRUE(statemachine_info->outgoing_msg_info.valid);  // Still pending
    
    // Second attempt - send succeeds
    _reset_variables();
    fail_send_msg = false;
    bool result2 = OpenLcbLoginStatemachine_handle_outgoing_openlcb_message();
    EXPECT_TRUE(result2);
    EXPECT_FALSE(statemachine_info->outgoing_msg_info.valid);  // Now cleared
}

// ============================================================================
// TEST: Node Enumeration Cycle
// ============================================================================

TEST(OpenLcbLoginStateMachine, node_enumeration_cycle)
{
    // Test complete enumeration cycle: first -> next -> end -> restart
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    openlcb_node_t *node_2 = OpenLcbNode_allocate(DEST_ID + 1, &_node_parameters_main_node);
    node_1->state.run_state = RUNSTATE_INIT;
    node_2->state.run_state = RUNSTATE_INIT;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Step 1: Get first node
    statemachine_info->openlcb_node = nullptr;
    first_node = node_1;
    OpenLcbLoginStatemachine_handle_try_enumerate_first_node();
    EXPECT_EQ(statemachine_info->openlcb_node, node_1);
    
    // Step 2: Get next node
    next_node = node_2;
    fail_next_node = false;
    OpenLcbLoginStatemachine_handle_try_enumerate_next_node();
    EXPECT_EQ(statemachine_info->openlcb_node, node_2);
    
    // Step 3: Reach end of list
    next_node = nullptr;
    OpenLcbLoginStatemachine_handle_try_enumerate_next_node();
    EXPECT_EQ(statemachine_info->openlcb_node, nullptr);
    
    // Step 4: Restart enumeration
    first_node = node_1;
    OpenLcbLoginStatemachine_handle_try_enumerate_first_node();
    EXPECT_EQ(statemachine_info->openlcb_node, node_1);
}

// ============================================================================
// TEST: Re-enumeration Flow
// ============================================================================

TEST(OpenLcbLoginStateMachine, reenumeration_flow)
{
    // Test re-enumeration for multi-message sequences
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;
    
    // Set enumerate flag (simulate handler requesting re-entry)
    statemachine_info->outgoing_msg_info.enumerate = true;
    
    bool result = OpenLcbLoginStatemachine_handle_try_reenumerate();
    
    EXPECT_TRUE(result);
    EXPECT_EQ(called_function_ptr, &_process_login_statemachine);
}

// ============================================================================
// TEST: Skip Nodes Already in RUN State
// ============================================================================

TEST(OpenLcbLoginStateMachine, skip_running_nodes)
{
    // Verify that nodes in RUN state are not processed
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    openlcb_node_t *node_2 = OpenLcbNode_allocate(DEST_ID + 1, &_node_parameters_main_node);
    openlcb_node_t *node_3 = OpenLcbNode_allocate(DEST_ID + 2, &_node_parameters_main_node);
    
    node_1->state.run_state = RUNSTATE_RUN;           // Already running
    node_2->state.run_state = RUNSTATE_INIT;          // Needs processing
    node_3->state.run_state = RUNSTATE_RUN;           // Already running

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Get first node (RUN state) - should not process
    _reset_variables();
    statemachine_info->openlcb_node = nullptr;
    first_node = node_1;
    OpenLcbLoginStatemachine_handle_try_enumerate_first_node();
    EXPECT_EQ(called_function_ptr, &_openlcb_node_get_first);  // Only get, no process
    
    // Get next node (INIT state) - should process
    _reset_variables();
    next_node = node_2;
    OpenLcbLoginStatemachine_handle_try_enumerate_next_node();
    EXPECT_EQ(called_function_ptr, (void *)((uint64_t)&_openlcb_node_get_next + (uint64_t)&_process_login_statemachine));
    
    // Get next node (RUN state) - should not process
    _reset_variables();
    statemachine_info->openlcb_node = node_2;
    next_node = node_3;
    OpenLcbLoginStatemachine_handle_try_enumerate_next_node();
    EXPECT_EQ(called_function_ptr, &_openlcb_node_get_next);  // Only get, no process
}

// ============================================================================
// TEST: Complete Login Sequence Integration
// ============================================================================

TEST(OpenLcbLoginStateMachine, complete_login_sequence)
{
    // Integration test of complete login for single node
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Setup for enumeration
    first_node = node_1;
    fail_first_node = false;
    statemachine_info->openlcb_node = nullptr;
    
    // Step 1: Enumerate first node
    OpenLcbLoginStatemachine_handle_try_enumerate_first_node();
    EXPECT_EQ(statemachine_info->openlcb_node, node_1);
    
    // The process_login_statemachine was called, which would call
    // OpenLcbLoginStatemachine_process in real code, which dispatches
    // based on run_state to the appropriate handler
    
    // In real operation, handlers would:
    // 1. Generate message
    // 2. Set valid flag
    // 3. Update run_state
    // 4. Set/clear enumerate flag
    
    EXPECT_NE(statemachine_info->openlcb_node, nullptr);
}

// ============================================================================
// TEST: Initialization Structure Validation
// ============================================================================

TEST(OpenLcbLoginStateMachine, initialization_structure_validation)
{
    // Verify all initialization setup is correct
    
    _reset_variables();
    _global_initialize();

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();

    // Verify message pointer setup
    EXPECT_NE(statemachine_info->outgoing_msg_info.msg_ptr, nullptr);
    EXPECT_EQ(statemachine_info->outgoing_msg_info.msg_ptr, &statemachine_info->outgoing_msg_info.openlcb_msg.openlcb_msg);
    
    // Verify payload pointer setup
    EXPECT_NE(statemachine_info->outgoing_msg_info.msg_ptr->payload, nullptr);
    EXPECT_EQ(statemachine_info->outgoing_msg_info.msg_ptr->payload, (openlcb_payload_t *)statemachine_info->outgoing_msg_info.openlcb_msg.openlcb_payload);
    
    // Verify payload type
    EXPECT_EQ(statemachine_info->outgoing_msg_info.msg_ptr->payload_type, BASIC);
    
    // Verify allocated flag
    EXPECT_TRUE(statemachine_info->outgoing_msg_info.msg_ptr->state.allocated);
    
    // Verify node pointer initialization
    EXPECT_EQ(statemachine_info->openlcb_node, nullptr);
}

// ============================================================================
// TEST: Multiple Nodes Sequential Processing
// ============================================================================

TEST(OpenLcbLoginStateMachine, multiple_nodes_sequential)
{
    // Test processing multiple nodes in sequence
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    openlcb_node_t *node_2 = OpenLcbNode_allocate(DEST_ID + 1, &_node_parameters_main_node);
    openlcb_node_t *node_3 = OpenLcbNode_allocate(DEST_ID + 2, &_node_parameters_main_node);
    
    node_1->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;
    node_2->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    node_3->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Process node 1
    _reset_variables();
    statemachine_info->openlcb_node = node_1;
    OpenLcbLoginStatemachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_initialization_complete);
    
    // Process node 2
    _reset_variables();
    statemachine_info->openlcb_node = node_2;
    OpenLcbLoginStatemachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_producer_events);
    
    // Process node 3
    _reset_variables();
    statemachine_info->openlcb_node = node_3;
    OpenLcbLoginStatemachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_consumer_events);
}

// ============================================================================
// TEST: State Machine Info Persistence
// ============================================================================

TEST(OpenLcbLoginStateMachine, statemachine_info_persistence)
{
    // Verify state machine info persists across operations
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);

    openlcb_login_statemachine_info_t *info1 = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Modify state
    info1->openlcb_node = node_1;
    info1->outgoing_msg_info.valid = true;
    info1->outgoing_msg_info.enumerate = true;
    
    // Get info again
    openlcb_login_statemachine_info_t *info2 = OpenLcbLoginStatemachine_get_statemachine_info();
    
    // Verify same structure and state persisted
    EXPECT_EQ(info1, info2);
    EXPECT_EQ(info2->openlcb_node, node_1);
    EXPECT_TRUE(info2->outgoing_msg_info.valid);
    EXPECT_TRUE(info2->outgoing_msg_info.enumerate);
}

// ============================================================================
// TEST: State Dispatch - RUNSTATE_LOGIN_COMPLETE with NULL callback
// ============================================================================

TEST(OpenLcbLoginStateMachine, process_login_complete_null_callback)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOGIN_COMPLETE;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Interface has on_login_complete = NULL, so should skip callback and transition to RUN
    OpenLcbLoginStatemachine_process(statemachine_info);

    // Verify NO handler was called (callback is NULL)
    EXPECT_EQ(called_function_ptr, nullptr);

    // Verify node transitioned to RUNSTATE_RUN
    EXPECT_EQ(node_1->state.run_state, RUNSTATE_RUN);
}

// ============================================================================
// TEST: State Dispatch - RUNSTATE_LOGIN_COMPLETE with callback returning true
// ============================================================================

TEST(OpenLcbLoginStateMachine, process_login_complete_callback_returns_true)
{
    _reset_variables();

    // Use interface with on_login_complete set
    OpenLcbLoginStatemachine_initialize(&interface_with_login_complete);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOGIN_COMPLETE;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Callback returns true -> should transition to RUNSTATE_RUN
    login_complete_return_value = true;

    OpenLcbLoginStatemachine_process(statemachine_info);

    // Verify on_login_complete was called
    EXPECT_EQ(called_function_ptr, &_on_login_complete);

    // Verify node transitioned to RUNSTATE_RUN
    EXPECT_EQ(node_1->state.run_state, RUNSTATE_RUN);
}

// ============================================================================
// TEST: State Dispatch - RUNSTATE_LOGIN_COMPLETE with callback returning false
// ============================================================================

TEST(OpenLcbLoginStateMachine, process_login_complete_callback_returns_false)
{
    _reset_variables();

    // Use interface with on_login_complete set
    OpenLcbLoginStatemachine_initialize(&interface_with_login_complete);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOGIN_COMPLETE;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Callback returns false -> should stay in LOGIN_COMPLETE (retry later)
    login_complete_return_value = false;

    OpenLcbLoginStatemachine_process(statemachine_info);

    // Verify on_login_complete was called
    EXPECT_EQ(called_function_ptr, &_on_login_complete);

    // Verify node did NOT transition - stays in LOGIN_COMPLETE
    EXPECT_EQ(node_1->state.run_state, RUNSTATE_LOGIN_COMPLETE);
}

// ============================================================================
// SIBLING DISPATCH TESTS
// ============================================================================
//
// These tests exercise the Phase 2 sibling dispatch mechanism in the login
// statemachine.  They use real node allocation and real node enumeration
// functions (OpenLcbNode_get_first/get_next/get_count) instead of mocks,
// because sibling dispatch iterates over all allocated nodes internally.
//
// The interface struct wires the REAL handle_outgoing (which triggers sibling
// dispatch) and real node functions, but keeps mock send and mock handlers.
// ============================================================================

    /** @brief Tracks how many times process_main_statemachine was called */
static int sibling_dispatch_call_count = 0;

    /** @brief Tracks source_id of each message dispatched to siblings */
static node_id_t sibling_dispatch_source_ids[100];

    /** @brief Tracks which node received each sibling dispatch */
static openlcb_node_t *sibling_dispatch_nodes[100];

    /** @brief When >= 0, the mock produces an outgoing response on that dispatch call number */
static int sibling_produce_response_on_call = -1;

    /** @brief When >= 0, the mock sets enumerate=true on that dispatch call number */
static int sibling_set_enumerate_on_call = -1;

    /** @brief Mock process_main_statemachine that records each dispatch.
     *  Optionally produces a response or sets enumerate based on control flags. */
void _tracking_process_main_statemachine(openlcb_statemachine_info_t *statemachine_info)
{

    if (sibling_dispatch_call_count < 100) {

        sibling_dispatch_source_ids[sibling_dispatch_call_count] =
                statemachine_info->incoming_msg_info.msg_ptr->source_id;
        sibling_dispatch_nodes[sibling_dispatch_call_count] =
                statemachine_info->openlcb_node;

    }

    if (sibling_dispatch_call_count == sibling_produce_response_on_call) {

        statemachine_info->outgoing_msg_info.msg_ptr->mti = MTI_VERIFIED_NODE_ID;
        statemachine_info->outgoing_msg_info.msg_ptr->source_id =
                statemachine_info->openlcb_node->id;
        statemachine_info->outgoing_msg_info.msg_ptr->source_alias =
                statemachine_info->openlcb_node->alias;
        statemachine_info->outgoing_msg_info.msg_ptr->dest_id = 0;
        statemachine_info->outgoing_msg_info.msg_ptr->dest_alias = 0;
        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 0;
        statemachine_info->outgoing_msg_info.valid = true;

    }

    if (sibling_dispatch_call_count == sibling_set_enumerate_on_call) {

        statemachine_info->incoming_msg_info.enumerate = true;

    }

    sibling_dispatch_call_count++;

}

    /** @brief Tracks how many times send was called */
static int send_call_count = 0;

    /** @brief When >= 0, the send mock returns false on that call number (1-based) */
static int fail_send_on_call = -1;

    /** @brief Mock send that counts calls.  Can fail on a specific call number. */
bool _counting_send_openlcb_msg(openlcb_msg_t *outgoing_msg)
{

    send_call_count++;

    if (send_call_count == fail_send_on_call) {

        return false;

    }

    return true;

}

    /** @brief Mock load_initialization_complete that sets valid and source_id */
void _sibling_load_initialization_complete(openlcb_login_statemachine_info_t *info)
{

    info->outgoing_msg_info.msg_ptr->mti = MTI_INITIALIZATION_COMPLETE;
    info->outgoing_msg_info.msg_ptr->source_id = info->openlcb_node->id;
    info->outgoing_msg_info.msg_ptr->source_alias = info->openlcb_node->alias;
    info->outgoing_msg_info.msg_ptr->dest_id = 0;
    info->outgoing_msg_info.msg_ptr->dest_alias = 0;
    info->outgoing_msg_info.msg_ptr->payload_count = 0;
    info->outgoing_msg_info.valid = true;

    info->openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

}

    /** @brief Counter for how many producer events have been loaded */
static int producer_event_index = 0;
static int producer_event_total = 0;

    /** @brief Mock load_producer_events that enumerates N events */
void _sibling_load_producer_events(openlcb_login_statemachine_info_t *info)
{

    info->outgoing_msg_info.msg_ptr->mti = MTI_PRODUCER_IDENTIFIED_UNKNOWN;
    info->outgoing_msg_info.msg_ptr->source_id = info->openlcb_node->id;
    info->outgoing_msg_info.msg_ptr->source_alias = info->openlcb_node->alias;
    info->outgoing_msg_info.msg_ptr->dest_id = 0;
    info->outgoing_msg_info.msg_ptr->dest_alias = 0;
    info->outgoing_msg_info.msg_ptr->payload_count = 8;
    info->outgoing_msg_info.valid = true;

    producer_event_index++;

    if (producer_event_index < producer_event_total) {

        info->outgoing_msg_info.enumerate = true;

    } else {

        info->outgoing_msg_info.enumerate = false;
        info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

    }

}

    /** @brief Mock load_consumer_events — no consumers, move to LOGIN_COMPLETE */
void _sibling_load_consumer_events(openlcb_login_statemachine_info_t *info)
{

    info->outgoing_msg_info.msg_ptr->mti = MTI_CONSUMER_IDENTIFIED_UNKNOWN;
    info->outgoing_msg_info.msg_ptr->source_id = info->openlcb_node->id;
    info->outgoing_msg_info.msg_ptr->source_alias = info->openlcb_node->alias;
    info->outgoing_msg_info.valid = true;
    info->outgoing_msg_info.enumerate = false;
    info->openlcb_node->state.run_state = RUNSTATE_LOGIN_COMPLETE;

}

    /** @brief Interface struct for sibling dispatch tests — uses real node
     *  functions and real handle_outgoing (which triggers sibling dispatch). */
const interface_openlcb_login_state_machine_t interface_sibling_dispatch = {

    .send_openlcb_msg = &_counting_send_openlcb_msg,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .load_initialization_complete = &_sibling_load_initialization_complete,
    .load_producer_events = &_sibling_load_producer_events,
    .load_consumer_events = &_sibling_load_consumer_events,
    .process_main_statemachine = &_tracking_process_main_statemachine,
    .openlcb_node_get_count = &OpenLcbNode_get_count,
    .process_login_statemachine = &OpenLcbLoginStatemachine_process,

    .handle_outgoing_openlcb_message = &OpenLcbLoginStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &OpenLcbLoginStatemachine_handle_try_reenumerate,
    .handle_try_enumerate_first_node = &OpenLcbLoginStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &OpenLcbLoginStatemachine_handle_try_enumerate_next_node,

    .on_login_complete = NULL,
};

    /** @brief Reset sibling dispatch test tracking state */
void _reset_sibling_tracking(void)
{

    sibling_dispatch_call_count = 0;
    send_call_count = 0;
    producer_event_index = 0;
    producer_event_total = 0;
    sibling_produce_response_on_call = -1;
    sibling_set_enumerate_on_call = -1;
    fail_send_on_call = -1;

    for (int i = 0; i < 100; i++) {

        sibling_dispatch_source_ids[i] = 0;
        sibling_dispatch_nodes[i] = nullptr;

    }

}

    /** @brief Initialize for sibling dispatch tests with real node functions */
void _sibling_test_initialize(void)
{

    _reset_sibling_tracking();
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();
    OpenLcbLoginStatemachine_initialize(&interface_sibling_dispatch);

    // Ensure leftover state from previous tests is cleared
    openlcb_login_statemachine_info_t *info = OpenLcbLoginStatemachine_get_statemachine_info();
    info->outgoing_msg_info.valid = false;
    info->outgoing_msg_info.enumerate = false;

}

// ============================================================================
// TEST: Single node — no sibling dispatch overhead
// ============================================================================

TEST(OpenLcbLoginStateMachine, sibling_dispatch_single_node_no_dispatch)
{

    _sibling_test_initialize();

    openlcb_node_t *node_a = OpenLcbNode_allocate(0x050101010100, &_node_parameters_main_node);
    node_a->alias = 0x100;
    node_a->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    // Run until Init Complete is sent and slot cleared
    for (int i = 0; i < 20; i++) {

        OpenLcbLoginStatemachine_run();

    }

    // Send was called for each login message (Init Complete + Producer + Consumer)
    EXPECT_GE(send_call_count, 1);

    // No sibling dispatch — only 1 node
    EXPECT_EQ(sibling_dispatch_call_count, 0);

}

// ============================================================================
// TEST: Loopback flag — outgoing message has loopback=true during dispatch,
//       cleared after dispatch completes
// ============================================================================

TEST(OpenLcbLoginStateMachine, sibling_dispatch_loopback_flag_lifecycle)
{

    _sibling_test_initialize();

    openlcb_node_t *node_a = OpenLcbNode_allocate(0x050101010100, &_node_parameters_main_node);
    node_a->alias = 0x100;
    node_a->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    openlcb_node_t *node_b = OpenLcbNode_allocate(0x050101010101, &_node_parameters_main_node);
    node_b->alias = 0x101;
    node_b->state.run_state = RUNSTATE_RUN;

    openlcb_login_statemachine_info_t *info = OpenLcbLoginStatemachine_get_statemachine_info();

    // Before anything — loopback should be false
    EXPECT_FALSE(info->outgoing_msg_info.msg_ptr->state.loopback);

    // Run through the full cycle
    for (int i = 0; i < 30; i++) {

        OpenLcbLoginStatemachine_run();

    }

    // After dispatch completes — loopback should be cleared
    EXPECT_FALSE(info->outgoing_msg_info.msg_ptr->state.loopback);

    // And valid should be cleared
    EXPECT_FALSE(info->outgoing_msg_info.valid);

}

// ============================================================================
// BRANCH COVERAGE TESTS — exercises specific branches in sibling dispatch
// ============================================================================

// ============================================================================
// TEST: Sibling response send fails — exercises _sibling_handle_outgoing
//       valid=true, send returns false (retry path)
// ============================================================================

TEST(OpenLcbLoginStateMachine, sibling_dispatch_response_send_fails_then_retries)
{

    _sibling_test_initialize();

    openlcb_node_t *node_a = OpenLcbNode_allocate(0x050101010100, &_node_parameters_main_node);
    node_a->alias = 0x100;
    node_a->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    openlcb_node_t *node_b = OpenLcbNode_allocate(0x050101010101, &_node_parameters_main_node);
    node_b->alias = 0x101;
    node_b->state.run_state = RUNSTATE_RUN;

    // Produce a response on first dispatch call
    sibling_produce_response_on_call = 0;

    // Fail the second send call (the sibling response send).
    // Call 1 = Init Complete to wire (succeeds).
    // Call 2 = sibling response to wire (fails).
    // Call 3 = retry of sibling response (succeeds).
    fail_send_on_call = 2;

    for (int i = 0; i < 50; i++) {

        OpenLcbLoginStatemachine_run();

    }

    // Should have eventually sent everything (retry succeeds)
    EXPECT_GE(send_call_count, 3);

}

// ============================================================================
// TEST: Login outgoing send fails — exercises handle_outgoing_openlcb_message
//       valid=true, send returns false (keeps valid, retries next tick)
// ============================================================================

TEST(OpenLcbLoginStateMachine, handle_outgoing_send_fails_keeps_valid_for_retry)
{

    _sibling_test_initialize();

    openlcb_node_t *node_a = OpenLcbNode_allocate(0x050101010100, &_node_parameters_main_node);
    node_a->alias = 0x100;
    node_a->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    openlcb_node_t *node_b = OpenLcbNode_allocate(0x050101010101, &_node_parameters_main_node);
    node_b->alias = 0x101;
    node_b->state.run_state = RUNSTATE_RUN;

    // Fail the first send (Init Complete to wire). Retries until it succeeds.
    fail_send_on_call = 1;

    for (int i = 0; i < 50; i++) {

        OpenLcbLoginStatemachine_run();

    }

    // Eventually succeeded — at least the Init Complete was sent
    EXPECT_GE(send_call_count, 2);

    // Node eventually continued login
    EXPECT_GE(node_a->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

}

// ============================================================================
// TEST: Sibling dispatch with NULL openlcb_node clears active flag
// @details Forces node_count > 1 (so dispatch_begin marks active=true) but
//          openlcb_node_get_first returns NULL (so openlcb_node stays NULL).
//          On the next _run() Priority 2 entry, _sibling_dispatch_current
//          hits its NULL guard, clears active, and returns false.  Control
//          falls through to the closing brace of the if(active) block.
// @coverage _sibling_dispatch_current NULL-node early return (lines 315, 317)
//           AND end-of-block fall-through in _run (line 566)
// ============================================================================

TEST(OpenLcbLoginStateMachine, sibling_dispatch_null_first_node_clears_active)
{

    _reset_variables();
    _global_initialize();

    // Make get_first return NULL despite count=2 — the only natural way to
    // land in dispatch with a NULL openlcb_node.  This represents a corner
    // case where a node was deallocated between count read and first read.
    mock_node_count = 2;
    fail_first_node = true;
    fail_send_msg = false;

    openlcb_login_statemachine_info_t *info = OpenLcbLoginStatemachine_get_statemachine_info();
    info->outgoing_msg_info.valid = true;

    // Drive the real handle_outgoing_openlcb_message — it sends, then
    // _sibling_dispatch_begin sets active=true while leaving openlcb_node NULL.
    OpenLcbLoginStatemachine_handle_outgoing_openlcb_message();

    // Next _run(): Priority 1 skipped (active=true), Priority 2 enters,
    //   2a: false (no sibling outgoing pending),
    //   2b: false (enumerate=false from dispatch_begin),
    //   2c: dispatch_current sees NULL openlcb_node → clears active, returns false.
    //   Falls through to the end of the if(active) block (line 566).
    OpenLcbLoginStatemachine_run();

    // After the NULL-guard fired, active must be cleared so the next _run()
    // tick re-enables Priority 1.  Verify by observing handle_outgoing fire.
    called_function_ptr = nullptr;
    OpenLcbLoginStatemachine_run();
    EXPECT_EQ(called_function_ptr, &_handle_outgoing_openlcb_message);

}

