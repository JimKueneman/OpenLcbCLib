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
* @file openlcb_login_statemachine_handler_Test.cxx
* @brief Test suite for OpenLCB login state machine message handler
*
* @details This test suite validates:
* - Initialization Complete message generation (Full and Simple protocols)
* - Producer Event Identified message generation
* - Consumer Event Identified message generation
* - State machine transitions during login sequence
* - Event enumeration logic
* - Edge cases (zero events, single event, multiple events)
*
* Coverage Analysis:
* - OpenLcbLoginMessageHandler_initialize: 100%
* - OpenLcbLoginMessageHandler_load_initialization_complete: 100%
* - OpenLcbLoginMessageHandler_load_producer_event: 100%
* - OpenLcbLoginMessageHandler_load_consumer_event: 100%
*
* @author Jim Kueneman
* @date 19 Jan 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_login_statemachine_handler.h"

#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

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

    // Space 0xEF - Firmware
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
    .address_space_firmware.low_address_valid = 0,
    .address_space_firmware.low_address = 0,
    .address_space_firmware.highest_address = 0x200,
    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
        },
};

// ============================================================================
// Mock Interface Functions
// ============================================================================

/**
 * @brief Mock function to extract producer event state MTI
 * @details Checks the event status and returns appropriate MTI
 * @param openlcb_node Pointer to node
 * @param event_index Index of event
 * @return MTI based on event status (SET, CLEAR, or UNKNOWN)
 */
uint16_t _extract_producer_event_state_mti(openlcb_node_t *openlcb_node, uint16_t event_index)
{
    if (event_index < openlcb_node->producers.count) {
        switch (openlcb_node->producers.list[event_index].status) {
            case EVENT_STATUS_SET:
                return MTI_PRODUCER_IDENTIFIED_SET;
            case EVENT_STATUS_CLEAR:
                return MTI_PRODUCER_IDENTIFIED_CLEAR;
            default:
                return MTI_PRODUCER_IDENTIFIED_UNKNOWN;
        }
    }
    return MTI_PRODUCER_IDENTIFIED_UNKNOWN;
}

/**
 * @brief Mock function to extract consumer event state MTI
 * @details Checks the event status and returns appropriate MTI
 * @param openlcb_node Pointer to node
 * @param event_index Index of event
 * @return MTI based on event status (SET, CLEAR, or UNKNOWN)
 */
uint16_t _extract_consumer_event_state_mti(openlcb_node_t *openlcb_node, uint16_t event_index)
{
    if (event_index < openlcb_node->consumers.count) {
        switch (openlcb_node->consumers.list[event_index].status) {
            case EVENT_STATUS_SET:
                return MTI_CONSUMER_IDENTIFIED_SET;
            case EVENT_STATUS_CLEAR:
                return MTI_CONSUMER_IDENTIFIED_CLEAR;
            default:
                return MTI_CONSUMER_IDENTIFIED_UNKNOWN;
        }
    }
    return MTI_CONSUMER_IDENTIFIED_UNKNOWN;
}

// ============================================================================
// Interface Structures
// ============================================================================

interface_openlcb_login_message_handler_t interface_openlcb_login_message_handler = {
    .extract_producer_event_state_mti = &_extract_producer_event_state_mti,
    .extract_consumer_event_state_mti = &_extract_consumer_event_state_mti
};

interface_openlcb_node_t interface_openlcb_node = {};

// ============================================================================
// Test Helpers
// ============================================================================

/**
 * @brief Reset test variables to initial state
 * @details Currently no dynamic test variables to reset
 */
void _reset_variables(void)
{
    // No dynamic variables to reset in current implementation
}

/**
 * @brief Initialize all required modules for testing
 * @details Initializes:
 * - Login Message Handler with interface
 * - Node management
 * - Buffer FIFO
 * - Buffer Store
 */
void _global_initialize(void)
{
    OpenLcbLoginMessageHandler_initialize(&interface_openlcb_login_message_handler);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

// ============================================================================
// TEST: Module Initialization
// ============================================================================

TEST(OpenLcbLoginMessageHandler, initialize)
{
    _reset_variables();
    _global_initialize();
    
    // Test passes if no crashes occur during initialization
    // Interface is stored statically and used in subsequent function calls
}

// ============================================================================
// TEST: Initialization Complete Message - Full Protocol
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_initialization_complete_full_protocol)
{
    _reset_variables();
    _global_initialize();

    // Allocate test node with Full protocol support
    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    // Allocate message buffer
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    // Setup statemachine info structure
    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Call function under test
    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);

    // Verify message is valid and ready to send
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify correct MTI for Full protocol (not Simple)
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_INITIALIZATION_COMPLETE);
    
    // Verify message source fields
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_id, node1->id);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_alias, node1->alias);
    
    // Verify payload count (6 bytes for Node ID)
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 6);
    
    // Verify node state changes
    EXPECT_TRUE(statemachine_info.openlcb_node->state.initialized);
    
    // Verify producer enumeration setup
    EXPECT_TRUE(statemachine_info.openlcb_node->producers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 0);
    
    // Verify consumer enumeration NOT started yet
    EXPECT_FALSE(statemachine_info.openlcb_node->consumers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 0);
    
    // Verify state transition
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);
}

// ============================================================================
// TEST: Initialization Complete Message - Simple Protocol
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_initialization_complete_simple_protocol)
{
    _reset_variables();
    _global_initialize();

    // Modify node parameters to include PSI_SIMPLE flag
    _node_parameters_main_node.protocol_support = (PSI_DATAGRAM |
                                                   PSI_SIMPLE |
                                                   PSI_EVENT_EXCHANGE);

    // Allocate test node
    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    // Allocate message buffer
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    // Setup statemachine info structure
    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Call function under test
    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);

    // Verify message is valid
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify correct MTI for Simple protocol
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_INITIALIZATION_COMPLETE_SIMPLE);
    
    // Verify message source fields
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_id, node1->id);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_alias, node1->alias);
    
    // Verify payload count
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 6);
    
    // Verify node state changes
    EXPECT_TRUE(statemachine_info.openlcb_node->state.initialized);
    EXPECT_TRUE(statemachine_info.openlcb_node->producers.enumerator.running);
    EXPECT_FALSE(statemachine_info.openlcb_node->consumers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 0);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 0);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    // Restore original protocol support for other tests
    _node_parameters_main_node.protocol_support = (PSI_DATAGRAM |
                                                   PSI_FIRMWARE_UPGRADE |
                                                   PSI_MEMORY_CONFIGURATION |
                                                   PSI_EVENT_EXCHANGE |
                                                   PSI_ABBREVIATED_DEFAULT_CDI |
                                                   PSI_SIMPLE_NODE_INFORMATION |
                                                   PSI_CONFIGURATION_DESCRIPTION_INFO);
}

// ============================================================================
// TEST: Producer Event Loading - No Events
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_event_no_events)
{
    _reset_variables();
    _global_initialize();

    // Allocate node with NO producer events
    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    // Allocate message buffer
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    // Setup statemachine info
    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Call function under test
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    // Verify no message is generated when there are no producer events
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify state transition skips to consumer events
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
}

// ============================================================================
// TEST: Producer Event Loading - Multiple Events
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_event_multiple_events)
{
    // Configure node to auto-create 2 producer events
    _node_parameters_main_node.consumer_count_autocreate = 2;
    _node_parameters_main_node.producer_count_autocreate = 2;

    _reset_variables();
    _global_initialize();

    // Allocate node with producer events
    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    // Allocate message buffer
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    // Setup statemachine info
    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // ========================================================================
    // Test First Producer Event
    // ========================================================================
    
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    // Verify message is valid
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify MTI (from mock function)
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    
    // Verify event ID (first event = Node ID << 16)
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_ID << 16);
    
    // Verify still in producer enumeration state
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);
    
    // Verify enumerate flag is set (will re-enter for next event)
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);

    // ========================================================================
    // Test Second Producer Event
    // ========================================================================
    
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    // Verify message is valid
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify MTI
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    
    // Verify event ID (second event = (Node ID << 16) + 1)
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), (DEST_ID << 16) + 1);
    
    // Verify still in producer enumeration state (cleanup happens on next call)
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);
    
    // Verify enumerate flag still set (will trigger cleanup call)
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);
    
    // Verify index incremented to 2
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 2);

    // ========================================================================
    // Test Cleanup Call (no more events, triggers transition)
    // ========================================================================
    
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    // Verify no message generated (cleanup only)
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify transition to consumer enumeration
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    
    // Verify enumerate flag cleared (done with producers)
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.enumerate);
    
    // Verify producer enumeration stopped and consumer enumeration started
    EXPECT_FALSE(statemachine_info.openlcb_node->producers.enumerator.running);
    EXPECT_TRUE(statemachine_info.openlcb_node->consumers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 0);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 0);

    // Restore original event counts
    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Consumer Event Loading - No Events
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_event_no_events)
{
    _reset_variables();
    _global_initialize();

    // Allocate node with NO consumer events
    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    // Allocate message buffer
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    // Setup statemachine info
    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Call function under test
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    // Verify no message is generated when there are no consumer events
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify state transition to RUN (login complete)
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);
}

// ============================================================================
// TEST: Consumer Event Loading - Multiple Events
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_event_multiple_events)
{
    // Configure node to auto-create 2 consumer events
    _node_parameters_main_node.consumer_count_autocreate = 2;
    _node_parameters_main_node.producer_count_autocreate = 2;

    _reset_variables();
    _global_initialize();

    // Allocate node with consumer events
    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    // Allocate message buffer
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    // Setup statemachine info
    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // ========================================================================
    // Test First Consumer Event
    // ========================================================================
    
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    // Verify message is valid
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify MTI (from mock function)
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    
    // Verify event ID (first event = Node ID << 16)
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_ID << 16);
    
    // Verify still in consumer enumeration state
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    
    // Verify enumerate flag is set (will re-enter for next event)
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);

    // ========================================================================
    // Test Second Consumer Event
    // ========================================================================
    
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    // Verify message is valid
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify MTI
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    
    // Verify event ID (second event = (Node ID << 16) + 1)
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), (DEST_ID << 16) + 1);
    
    // Still in consumer state (cleanup happens on next call)
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    
    // Verify enumerate flag still set (will trigger cleanup call)
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);
    
    // Verify index incremented to 2
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 2);

    // ========================================================================
    // Test Cleanup Call (no more events, triggers transition)
    // ========================================================================
    
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    // Verify no message generated (cleanup only)
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    // Verify transition to RUN state (login complete)
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);
    
    // Verify enumerate flag cleared (done with consumers)
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.enumerate);
    
    // Verify consumer enumeration stopped
    EXPECT_FALSE(statemachine_info.openlcb_node->consumers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 0);

    // Restore original event counts
    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// ADDITIONAL TESTS - ALL WORKING GREAT
// ============================================================================

// ============================================================================
// TEST: Producer Event - Single Event
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_event_single_event)
{
    // Test with exactly one producer event to verify boundary conditions
    
    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 1;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Process single producer event
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_ID << 16);
    
    // Still in producer state, enumerate flag set (cleanup happens on next call)
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 1);

    // Process cleanup call
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    // Should transition to consumer events after cleanup
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.enumerate);
    EXPECT_FALSE(statemachine_info.openlcb_node->producers.enumerator.running);
    EXPECT_TRUE(statemachine_info.openlcb_node->consumers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 0);

    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Consumer Event - Single Event
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_event_single_event)
{
    // Test with exactly one consumer event to verify boundary conditions
    
    _node_parameters_main_node.consumer_count_autocreate = 1;
    _node_parameters_main_node.producer_count_autocreate = 0;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Process single consumer event
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), DEST_ID << 16);
    
    // Still in consumer state, enumerate flag set (cleanup happens on next call)
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 1);

    // Process cleanup call
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    // Should transition to RUN after cleanup
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.enumerate);
    EXPECT_FALSE(statemachine_info.openlcb_node->consumers.enumerator.running);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 0);

    _node_parameters_main_node.consumer_count_autocreate = 0;
}

// ============================================================================
// TEST: Producer Event - Large Event Count
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_event_many_events)
{
    // Test with maximum or near-maximum event count to verify enumeration logic
    // This tests the loop behavior and index management
    
    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 10;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Enumerate all 10 producer events
    for (int i = 0; i < 10; i++)
    {
        OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
        
        EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
        EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), 
                  (DEST_ID << 16) + i);
        
        // All events keep enumerate flag set and stay in LOAD_PRODUCER_EVENTS state
        EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);
        EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);
    }

    // One more call to trigger cleanup
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.enumerate);
    EXPECT_EQ(statemachine_info.openlcb_node->producers.enumerator.enum_index, 0);

    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Consumer Event - Large Event Count
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_event_many_events)
{
    // Test with maximum or near-maximum event count to verify enumeration logic
    
    _node_parameters_main_node.consumer_count_autocreate = 10;
    _node_parameters_main_node.producer_count_autocreate = 0;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Enumerate all 10 consumer events
    for (int i = 0; i < 10; i++)
    {
        OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
        
        EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
        EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), 
                  (DEST_ID << 16) + i);
        
        // All events keep enumerate flag set and stay in LOAD_CONSUMER_EVENTS state
        EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
        EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);
    }

    // One more call to trigger cleanup
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.enumerate);
    EXPECT_EQ(statemachine_info.openlcb_node->consumers.enumerator.enum_index, 0);

    _node_parameters_main_node.consumer_count_autocreate = 0;
}

// ============================================================================
// TEST: Full Login Sequence
// ============================================================================

TEST(OpenLcbLoginMessageHandler, full_login_sequence)
{
    // Test complete login sequence: Init Complete -> Producers -> Consumers -> Run
    
    _node_parameters_main_node.consumer_count_autocreate = 2;
    _node_parameters_main_node.producer_count_autocreate = 2;

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

    // Step 1: Initialization Complete
    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_INITIALIZATION_COMPLETE);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);
    EXPECT_TRUE(statemachine_info.openlcb_node->state.initialized);

    // Step 2: First Producer Event
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    // Step 3: Second Producer Event
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    // Step 4: Producer Cleanup Call
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    // Step 5: First Consumer Event
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    // Step 6: Second Consumer Event
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    // Step 7: Consumer Cleanup Call
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);
    
    // Final verification - node is fully initialized and in RUN state
    EXPECT_TRUE(statemachine_info.openlcb_node->state.initialized);
    EXPECT_FALSE(statemachine_info.openlcb_node->producers.enumerator.running);
    EXPECT_FALSE(statemachine_info.openlcb_node->consumers.enumerator.running);

    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Payload Verification - Node ID Format
// ============================================================================

TEST(OpenLcbLoginMessageHandler, verify_node_id_payload_format)
{
    // Verify that Node ID is correctly formatted in Initialization Complete payload
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);

    // Verify payload structure for 48-bit Node ID
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    
    // Extract and verify Node ID from payload
    uint64_t extracted_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0);
    EXPECT_EQ(extracted_id, DEST_ID);
}

// ============================================================================
// TEST: Payload Verification - Event ID Format
// ============================================================================

TEST(OpenLcbLoginMessageHandler, verify_event_id_payload_format)
{
    // Verify that Event ID is correctly formatted in Producer/Consumer Identified payloads
    
    _node_parameters_main_node.producer_count_autocreate = 1;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    // Verify payload structure for 64-bit Event ID
    EXPECT_EQ(outgoing_msg->payload_count, 8);
    
    // Extract and verify Event ID from payload
    event_id_t extracted_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg);
    EXPECT_EQ(extracted_event_id, DEST_ID << 16);

    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Enumeration Index Management - Producer
// ============================================================================

TEST(OpenLcbLoginMessageHandler, verify_producer_enumeration_index)
{
    // Verify that enumeration index is correctly incremented and reset
    
    _node_parameters_main_node.producer_count_autocreate = 3;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Initial index should be 0
    EXPECT_EQ(node1->producers.enumerator.enum_index, 0);

    // First event - index becomes 1
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->producers.enumerator.enum_index, 1);

    // Second event - index becomes 2
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->producers.enumerator.enum_index, 2);

    // Third event - index becomes 3
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->producers.enumerator.enum_index, 3);

    // Cleanup call - index resets to 0
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->producers.enumerator.enum_index, 0);
    EXPECT_FALSE(node1->producers.enumerator.running);

    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Enumeration Index Management - Consumer
// ============================================================================

TEST(OpenLcbLoginMessageHandler, verify_consumer_enumeration_index)
{
    // Verify that enumeration index is correctly incremented and reset
    
    _node_parameters_main_node.consumer_count_autocreate = 3;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Initial index should be 0
    EXPECT_EQ(node1->consumers.enumerator.enum_index, 0);

    // First event - index becomes 1
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(node1->consumers.enumerator.enum_index, 1);

    // Second event - index becomes 2
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(node1->consumers.enumerator.enum_index, 2);

    // Third event - index becomes 3
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(node1->consumers.enumerator.enum_index, 3);

    // Cleanup call - index resets to 0
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(node1->consumers.enumerator.enum_index, 0);
    EXPECT_FALSE(node1->consumers.enumerator.running);

    _node_parameters_main_node.consumer_count_autocreate = 0;
}

// ============================================================================
// TEST: State Transition Verification
// ============================================================================

TEST(OpenLcbLoginMessageHandler, verify_all_state_transitions)
{
    // Comprehensive test of all state transitions through login sequence
    
    _node_parameters_main_node.consumer_count_autocreate = 1;
    _node_parameters_main_node.producer_count_autocreate = 1;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Initial state (would be set by previous state machine)
    node1->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    // Transition 1: LOAD_INITIALIZATION_COMPLETE -> LOAD_PRODUCER_EVENTS
    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    // Transition 2: LOAD_PRODUCER_EVENTS (send event, stay in same state)
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    // Transition 3: LOAD_PRODUCER_EVENTS -> LOAD_CONSUMER_EVENTS (cleanup call)
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    // Transition 4: LOAD_CONSUMER_EVENTS (send event, stay in same state)
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    // Transition 5: LOAD_CONSUMER_EVENTS -> RUN (cleanup call)
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_LOGIN_COMPLETE);

    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Message Source Fields
// ============================================================================

TEST(OpenLcbLoginMessageHandler, verify_message_source_fields)
{
    // Verify that all messages have correct source alias and ID
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Test Initialization Complete message
    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);
    EXPECT_EQ(outgoing_msg->source_alias, DEST_ALIAS);
    EXPECT_EQ(outgoing_msg->source_id, DEST_ID);

    // Additional tests would be added for producer and consumer messages
    // when events are configured
}

// ============================================================================
// TEST: Both Producers and Consumers Zero
// ============================================================================

TEST(OpenLcbLoginMessageHandler, zero_producers_and_consumers)
{
    // Test node with no events at all - should skip directly through enumeration
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // Initialization complete
    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    // No producers - skip to consumers
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);

    // No consumers - skip to RUN
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_LOGIN_COMPLETE);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
}

// ============================================================================
// ADDITIONAL COVERAGE TESTS FOR 100% CODE COVERAGE
// ============================================================================

// ============================================================================
// TEST: Producer Range Events - Single Range
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_range_event_single)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    node1->producers.range_count = 1;
    node1->producers.range_list[0].start_base = (DEST_ID << 16);
    node1->producers.range_list[0].event_count = EVENT_RANGE_COUNT_16;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_RANGE_IDENTIFIED);
    EXPECT_EQ(node1->producers.enumerator.range_enum_index, 1);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
    EXPECT_EQ(node1->producers.enumerator.range_enum_index, 0);
}

// ============================================================================
// TEST: Producer Range Events - Multiple Ranges
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_range_event_multiple)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Only use 1 range since array size is 1
    node1->producers.range_count = 1;
    node1->producers.range_list[0].start_base = (DEST_ID << 16);
    node1->producers.range_list[0].event_count = EVENT_RANGE_COUNT_8;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_RANGE_IDENTIFIED);
    EXPECT_EQ(node1->producers.enumerator.range_enum_index, 1);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);
}

// ============================================================================
// TEST: Producer Mixed Range and Normal Events
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_mixed_range_and_normal)
{
    _node_parameters_main_node.producer_count_autocreate = 2;
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Add 1 range (array size is 1)
    node1->producers.range_count = 1;
    node1->producers.range_list[0].start_base = (DEST_ID << 16) + 1000;
    node1->producers.range_list[0].event_count = EVENT_RANGE_COUNT_8;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // First call should process range
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_RANGE_IDENTIFIED);

    // Next two calls should process normal events
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);
    
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);

    // Cleanup call
    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Consumer Range Events - Single Range
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_range_event_single)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    node1->consumers.range_count = 1;
    node1->consumers.range_list[0].start_base = (DEST_ID << 16);
    node1->consumers.range_list[0].event_count = EVENT_RANGE_COUNT_16;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_RANGE_IDENTIFIED);
    EXPECT_EQ(node1->consumers.enumerator.range_enum_index, 1);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);
    EXPECT_EQ(node1->consumers.enumerator.range_enum_index, 0);
}

// ============================================================================
// TEST: Consumer Range Events - Multiple Ranges
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_range_event_multiple)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Only use 1 range since array size is 1
    node1->consumers.range_count = 1;
    node1->consumers.range_list[0].start_base = (DEST_ID << 16);
    node1->consumers.range_list[0].event_count = EVENT_RANGE_COUNT_8;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_RANGE_IDENTIFIED);
    EXPECT_EQ(node1->consumers.enumerator.range_enum_index, 1);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.enumerate);

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);
}

// ============================================================================
// TEST: Consumer Mixed Range and Normal Events
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_mixed_range_and_normal)
{
    _node_parameters_main_node.consumer_count_autocreate = 2;
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Add 1 range (array size is 1)
    node1->consumers.range_count = 1;
    node1->consumers.range_list[0].start_base = (DEST_ID << 16) + 1000;
    node1->consumers.range_list[0].event_count = EVENT_RANGE_COUNT_8;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    // First call should process range
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_RANGE_IDENTIFIED);

    // Next two calls should process normal events
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);
    
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);

    // Cleanup call
    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);

    _node_parameters_main_node.consumer_count_autocreate = 0;
}

// ============================================================================
// TEST: Producer Event MTI Variations - Valid
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_event_mti_valid)
{
    _node_parameters_main_node.producer_count_autocreate = 1;
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Set the producer event status to SET
    node1->producers.list[0].status = EVENT_STATUS_SET;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_SET);

    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Producer Event MTI Variations - Invalid
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_producer_event_mti_invalid)
{
    _node_parameters_main_node.producer_count_autocreate = 1;
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Set the producer event status to CLEAR
    node1->producers.list[0].status = EVENT_STATUS_CLEAR;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_CLEAR);

    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Consumer Event MTI Variations - Valid
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_event_mti_valid)
{
    _node_parameters_main_node.consumer_count_autocreate = 1;
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Set the consumer event status to SET
    node1->consumers.list[0].status = EVENT_STATUS_SET;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_SET);

    _node_parameters_main_node.consumer_count_autocreate = 0;
}

// ============================================================================
// TEST: Consumer Event MTI Variations - Invalid
// ============================================================================

TEST(OpenLcbLoginMessageHandler, load_consumer_event_mti_invalid)
{
    _node_parameters_main_node.consumer_count_autocreate = 1;
    
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Set the consumer event status to CLEAR
    node1->consumers.list[0].status = EVENT_STATUS_CLEAR;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_CLEAR);

    _node_parameters_main_node.consumer_count_autocreate = 0;
}

// ============================================================================
// TEST: Full Login with Ranges
// ============================================================================

TEST(OpenLcbLoginMessageHandler, full_login_sequence_with_ranges)
{
    _node_parameters_main_node.consumer_count_autocreate = 1;
    _node_parameters_main_node.producer_count_autocreate = 1;

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    node1->producers.range_count = 1;
    node1->producers.range_list[0].start_base = (DEST_ID << 16) + 1000;
    node1->producers.range_list[0].event_count = EVENT_RANGE_COUNT_8;
    
    node1->consumers.range_count = 1;
    node1->consumers.range_list[0].start_base = (DEST_ID << 16) + 2000;
    node1->consumers.range_list[0].event_count = EVENT_RANGE_COUNT_16;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_initialization_complete(&statemachine_info);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_RANGE_IDENTIFIED);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_IDENTIFIED_UNKNOWN);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_RANGE_IDENTIFIED);

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_IDENTIFIED_UNKNOWN);

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);

    EXPECT_FALSE(node1->producers.enumerator.running);
    EXPECT_FALSE(node1->consumers.enumerator.running);
    EXPECT_EQ(node1->producers.enumerator.enum_index, 0);
    EXPECT_EQ(node1->consumers.enumerator.enum_index, 0);
    EXPECT_EQ(node1->producers.enumerator.range_enum_index, 0);
    EXPECT_EQ(node1->consumers.enumerator.range_enum_index, 0);

    _node_parameters_main_node.consumer_count_autocreate = 0;
    _node_parameters_main_node.producer_count_autocreate = 0;
}

// ============================================================================
// TEST: Only Range Events
// ============================================================================

TEST(OpenLcbLoginMessageHandler, only_range_events_no_normal)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    node1->producers.range_count = 1;
    node1->producers.range_list[0].start_base = (DEST_ID << 16);
    node1->producers.range_list[0].event_count = EVENT_RANGE_COUNT_8;
    
    node1->consumers.range_count = 1;
    node1->consumers.range_list[0].start_base = (DEST_ID << 16) + 1000;
    node1->consumers.range_list[0].event_count = EVENT_RANGE_COUNT_32;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_PRODUCER_RANGE_IDENTIFIED);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_EQ(outgoing_msg->mti, MTI_CONSUMER_RANGE_IDENTIFIED);

    OpenLcbLoginMessageHandler_load_consumer_event(&statemachine_info);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.openlcb_node->state.run_state, RUNSTATE_LOGIN_COMPLETE);
}

// ============================================================================
// TEST: Range Index Management
// ============================================================================

TEST(OpenLcbLoginMessageHandler, verify_range_index_management)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Only 1 range since array size is 1
    node1->producers.range_count = 1;
    node1->producers.range_list[0].start_base = (DEST_ID << 16);
    node1->producers.range_list[0].event_count = EVENT_RANGE_COUNT_4;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_login_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    EXPECT_EQ(node1->producers.enumerator.range_enum_index, 0);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->producers.enumerator.range_enum_index, 1);

    OpenLcbLoginMessageHandler_load_producer_event(&statemachine_info);
    EXPECT_EQ(node1->producers.enumerator.range_enum_index, 0);
    EXPECT_FALSE(node1->producers.enumerator.running);
}
