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
* - OpenLcbLoginStateMachine_initialize: 100%
* - OpenLcbLoginStateMachine_process: 100%
* - OpenLcbLoginStatemachine_handle_outgoing_openlcb_message: 100%
* - OpenLcbLoginStatemachine_handle_try_reenumerate: 100%
* - OpenLcbLoginStatemachine_handle_try_enumerate_first_node: 100%
* - OpenLcbLoginStatemachine_handle_try_enumerate_next_node: 100%
* - OpenLcbLoginMainStatemachine_run: 100%
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

    // Space 0xFF - Configuration Definition Info
    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 0,
    .address_space_configuration_definition.low_address_valid = 0,
    .address_space_configuration_definition.low_address = 0,
    .address_space_configuration_definition.highest_address = 0x200,
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE - All Memory
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0,
    .address_space_all.low_address = 0,
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD - Configuration Memory
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,
    .address_space_config_memory.low_address = 0,
    .address_space_config_memory.highest_address = 0x200,
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    .cdi =
        {
            // </cdi>
        },
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

    .load_initialization_complete = &_load_initialization_complete,
    .load_producer_events = &_load_producer_events,
    .load_consumer_events = &_load_consumer_events,

    .send_openlcb_msg = &_send_openlcb_msg,

    .openlcb_node_get_first = &_openlcb_node_get_first,
    .openlcb_node_get_next = &_openlcb_node_get_next,
    .process_login_statemachine = &_process_login_statemachine,

    // For test injection of run loop handlers
    .handle_outgoing_openlcb_message = &_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &_handle_try_reenumerate,
    .handle_try_enumerate_first_node = &_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &_handle_try_enumerate_next_node,

    .on_login_complete = NULL,
};

const interface_openlcb_login_state_machine_t interface_with_login_complete = {

    .load_initialization_complete = &_load_initialization_complete,
    .load_producer_events = &_load_producer_events,
    .load_consumer_events = &_load_consumer_events,

    .send_openlcb_msg = &_send_openlcb_msg,

    .openlcb_node_get_first = &_openlcb_node_get_first,
    .openlcb_node_get_next = &_openlcb_node_get_next,
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
}

/**
 * @brief Initialize all required modules for testing
 */
void _global_initialize(void)
{
    OpenLcbLoginStateMachine_initialize(&interface_openlcb_login_state_machine);
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
    OpenLcbLoginStateMachine_process(statemachine_info);

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
    OpenLcbLoginStateMachine_process(statemachine_info);

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
    OpenLcbLoginStateMachine_process(statemachine_info);

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
    OpenLcbLoginStateMachine_process(statemachine_info);

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

    OpenLcbLoginMainStatemachine_run();

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

    OpenLcbLoginMainStatemachine_run();

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

    OpenLcbLoginMainStatemachine_run();

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

    OpenLcbLoginMainStatemachine_run();

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

    OpenLcbLoginMainStatemachine_run();

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
    OpenLcbLoginStateMachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_initialization_complete);

    // Test RUNSTATE_LOAD_PRODUCER_EVENTS
    _reset_variables();
    node_1->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    OpenLcbLoginStateMachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_producer_events);

    // Test RUNSTATE_LOAD_CONSUMER_EVENTS
    _reset_variables();
    node_1->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    OpenLcbLoginStateMachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_consumer_events);

    // Test RUNSTATE_RUN (no dispatch)
    _reset_variables();
    node_1->state.run_state = RUNSTATE_RUN;
    OpenLcbLoginStateMachine_process(statemachine_info);
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
    // OpenLcbLoginStateMachine_process in real code, which dispatches
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
    OpenLcbLoginStateMachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_initialization_complete);
    
    // Process node 2
    _reset_variables();
    statemachine_info->openlcb_node = node_2;
    OpenLcbLoginStateMachine_process(statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_producer_events);
    
    // Process node 3
    _reset_variables();
    statemachine_info->openlcb_node = node_3;
    OpenLcbLoginStateMachine_process(statemachine_info);
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
    OpenLcbLoginStateMachine_process(statemachine_info);

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
    OpenLcbLoginStateMachine_initialize(&interface_with_login_complete);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOGIN_COMPLETE;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Callback returns true -> should transition to RUNSTATE_RUN
    login_complete_return_value = true;

    OpenLcbLoginStateMachine_process(statemachine_info);

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
    OpenLcbLoginStateMachine_initialize(&interface_with_login_complete);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node_1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node_1->alias = DEST_ALIAS;
    node_1->state.run_state = RUNSTATE_LOGIN_COMPLETE;

    openlcb_login_statemachine_info_t *statemachine_info = OpenLcbLoginStatemachine_get_statemachine_info();
    statemachine_info->openlcb_node = node_1;

    // Callback returns false -> should stay in LOGIN_COMPLETE (retry later)
    login_complete_return_value = false;

    OpenLcbLoginStateMachine_process(statemachine_info);

    // Verify on_login_complete was called
    EXPECT_EQ(called_function_ptr, &_on_login_complete);

    // Verify node did NOT transition - stays in LOGIN_COMPLETE
    EXPECT_EQ(node_1->state.run_state, RUNSTATE_LOGIN_COMPLETE);
}
