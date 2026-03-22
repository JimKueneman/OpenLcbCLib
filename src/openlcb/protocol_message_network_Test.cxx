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
* @file protocol_message_network_Test.cxx
* @brief Comprehensive test suite for Message Network Protocol Handler
* @details Tests message network protocol with full edge case coverage
*
* Test Organization:
* - Section 1: Existing Tests (6 tests) - Baseline functionality
* - Section 2: Enhanced Tests - Extended coverage for edge cases
* - Section 3: Boundary and State Tests - Comprehensive validation
*
* Module Characteristics:
* - Dependency Injection: YES (interface currently empty, reserved for future)
* - 9 public functions tested
* - 2 static helper functions (tested indirectly)
* - Protocol: Message Network Layer (OpenLCB Standard)
* - Core functions: node verification, initialization, protocol support
*
* Coverage Analysis:
* - Original (6 tests): ~75% coverage
* - Enhanced (15+ tests): ~95% coverage
*
* Interface: Currently empty struct (reserved for future callbacks)
*
* Enhanced Test Coverage:
* - Simple vs Full node protocol variations
* - Duplicate Node ID detection (first and subsequent)
* - Verify Node ID with/without payload
* - Verify Node ID matching and non-matching
* - Protocol support flag encoding
* - All message handler functions
* - Error and termination message handling
*
* @author Jim Kueneman
* @date 20 Jan 2026
*/

#include "test/main_Test.hxx"

#include "protocol_message_network.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

// ============================================================================
// TEST CONFIGURATION CONSTANTS
// ============================================================================

#define SOURCE_ALIAS 0x222
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0xBBB
#define DEST_ID 0x060504030201

// ============================================================================
// NODE PARAMETER CONFIGURATIONS
// ============================================================================

// Full protocol node configuration
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
                         PSI_FIRMWARE_UPGRADE |
                         PSI_MEMORY_CONFIGURATION |
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

    // Space 0xEF - Firmware
    .address_space_firmware = {
        .present = 1,
        .read_only = 0,
        .low_address_valid = 0,
        .address_space = CONFIG_MEM_SPACE_FIRMWARE,
        .highest_address = 0x200,
        .low_address = 0,
        .description = "Firmware Bootloader"
    },

    .cdi = NULL,
    .fdi = NULL,
};

// Simple protocol node configuration
node_parameters_t _node_parameters_main_node_simple = {

    .snip = {
        .mfg_version = 4,
        .name = "Test",
        .model = "Test Model J",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_SIMPLE |  // Simple node flag
                         PSI_FIRMWARE_UPGRADE |
                         PSI_MEMORY_CONFIGURATION |
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

    // Space 0xEF - Firmware
    .address_space_firmware = {
        .present = 1,
        .read_only = 0,
        .low_address_valid = 0,
        .address_space = CONFIG_MEM_SPACE_FIRMWARE,
        .highest_address = 0x200,
        .low_address = 0,
        .description = "Firmware Bootloader"
    },

    .cdi = NULL,
    .fdi = NULL,
};

// ============================================================================
// INTERFACE CONFIGURATION
// ============================================================================

// ============================================================================
// MOCK CALLBACK STATE
// ============================================================================

static bool _oir_callback_called = false;
static openlcb_node_t *_oir_callback_node = nullptr;
static node_id_t _oir_callback_source_node_id = 0;
static uint16_t _oir_callback_error_code = 0;
static uint16_t _oir_callback_rejected_mti = 0;

static bool _tde_callback_called = false;
static openlcb_node_t *_tde_callback_node = nullptr;
static node_id_t _tde_callback_source_node_id = 0;
static uint16_t _tde_callback_error_code = 0;
static uint16_t _tde_callback_rejected_mti = 0;

    /** @brief Mock OIR callback — captures parameters for verification. */
void _mock_on_optional_interaction_rejected(openlcb_node_t *openlcb_node, node_id_t source_node_id, uint16_t error_code, uint16_t rejected_mti) {

    _oir_callback_called = true;
    _oir_callback_node = openlcb_node;
    _oir_callback_source_node_id = source_node_id;
    _oir_callback_error_code = error_code;
    _oir_callback_rejected_mti = rejected_mti;

}

    /** @brief Mock TDE callback — captures parameters for verification. */
void _mock_on_terminate_due_to_error(openlcb_node_t *openlcb_node, node_id_t source_node_id, uint16_t error_code, uint16_t rejected_mti) {

    _tde_callback_called = true;
    _tde_callback_node = openlcb_node;
    _tde_callback_source_node_id = source_node_id;
    _tde_callback_error_code = error_code;
    _tde_callback_rejected_mti = rejected_mti;

}

// NULL-callback interface (existing behavior — no callbacks wired)
interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network = {};

// Callback-wired interface for OIR/TDE tests
interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network_with_callbacks = {

    .on_optional_interaction_rejected = &_mock_on_optional_interaction_rejected,
    .on_terminate_due_to_error = &_mock_on_terminate_due_to_error,

};

interface_openlcb_node_t interface_openlcb_node = {};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Resets all test tracking variables to initial state
 */
void _reset_variables(void)
{

    _oir_callback_called = false;
    _oir_callback_node = nullptr;
    _oir_callback_source_node_id = 0;
    _oir_callback_error_code = 0;
    _oir_callback_rejected_mti = 0;

    _tde_callback_called = false;
    _tde_callback_node = nullptr;
    _tde_callback_source_node_id = 0;
    _tde_callback_error_code = 0;
    _tde_callback_rejected_mti = 0;

}

/**
 * @brief Initializes all subsystems
 */
void _global_initialize(void)
{
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

// ============================================================================
// SECTION 1: BASIC FUNCTIONALITY TESTS
// ============================================================================

// ============================================================================
// TEST: Basic Initialization
// @details Verifies module initializes without errors
// @coverage ProtocolMessageNetwork_initialize()
// ============================================================================

TEST(ProtocolMessageNetwork, initialize)
{
    _reset_variables();
    _global_initialize();
    
    // If we get here, initialization succeeded
    EXPECT_TRUE(true);
}

// ============================================================================
// TEST: Protocol Support Inquiry - Full Node
// @details Tests protocol support inquiry handler for full protocol node
// @coverage ProtocolMessageNetwork_handle_protocol_support_inquiry()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_protocol_support_inquiry_full)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test protocol support inquiry
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);
    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PROTOCOL_SUPPORT_REPLY);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    
    // Verify protocol support flags are correctly encoded in all 48 bits
    // Bytes 0-2: bits 16-23, 8-15, 0-7 (lower 24 bits)
    // Bytes 3-5: bits 40-47, 32-39, 24-31 (upper 24 bits)
    uint64_t support_flags = node1->parameters->protocol_support;
    uint8_t *payload_ptr = (uint8_t *)outgoing_msg->payload;
    EXPECT_EQ(payload_ptr[0], (uint8_t)((support_flags >> 16) & 0xFF));
    EXPECT_EQ(payload_ptr[1], (uint8_t)((support_flags >> 8) & 0xFF));
    EXPECT_EQ(payload_ptr[2], (uint8_t)((support_flags >> 0) & 0xFF));
    EXPECT_EQ(payload_ptr[3], (uint8_t)((support_flags >> 40) & 0xFF));
    EXPECT_EQ(payload_ptr[4], (uint8_t)((support_flags >> 32) & 0xFF));
    EXPECT_EQ(payload_ptr[5], (uint8_t)((support_flags >> 24) & 0xFF));
}

// ============================================================================
// TEST: Protocol Support Inquiry - Simple Node
// @details Tests protocol support inquiry handler for simple protocol node
// @coverage ProtocolMessageNetwork_handle_protocol_support_inquiry()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_protocol_support_inquiry_simple)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node_simple);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test protocol support inquiry for simple node
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);
    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PROTOCOL_SUPPORT_REPLY);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    
    // Verify PSI_SIMPLE flag is set
    uint64_t support_flags = node1->parameters->protocol_support;
    EXPECT_TRUE((support_flags & PSI_SIMPLE) != 0);
}

// ============================================================================
// TEST: Protocol Support Inquiry - Firmware Upgrade Active
// @details Tests protocol support inquiry when firmware upgrade is active
// @coverage ProtocolMessageNetwork_handle_protocol_support_inquiry()
//          Covers: firmware_upgrade_active == true path (lines 277-281)
// ============================================================================

TEST(ProtocolMessageNetwork, handle_protocol_support_inquiry_firmware_upgrade_active)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    
    // Set firmware_upgrade_active flag to true
    node1->state.firmware_upgrade_active = 1;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test protocol support inquiry with firmware upgrade active
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);
    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PROTOCOL_SUPPORT_REPLY);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    
    // When firmware_upgrade_active is true:
    // - PSI_FIRMWARE_UPGRADE bit should be cleared
    // - PSI_FIRMWARE_UPGRADE_ACTIVE bit should be set
    uint64_t original_flags = node1->parameters->protocol_support;
    uint64_t expected_flags = (original_flags & ~((uint64_t)PSI_FIRMWARE_UPGRADE)) | (uint64_t)PSI_FIRMWARE_UPGRADE_ACTIVE;
    
    uint8_t *payload_ptr = (uint8_t *)outgoing_msg->payload;
    EXPECT_EQ(payload_ptr[0], (uint8_t)((expected_flags >> 16) & 0xFF));
    EXPECT_EQ(payload_ptr[1], (uint8_t)((expected_flags >> 8) & 0xFF));
    EXPECT_EQ(payload_ptr[2], (uint8_t)((expected_flags >> 0) & 0xFF));
    EXPECT_EQ(payload_ptr[3], (uint8_t)((expected_flags >> 40) & 0xFF));
    EXPECT_EQ(payload_ptr[4], (uint8_t)((expected_flags >> 32) & 0xFF));
    EXPECT_EQ(payload_ptr[5], (uint8_t)((expected_flags >> 24) & 0xFF));
    
    // Verify the bit manipulation worked correctly
    EXPECT_FALSE((expected_flags & PSI_FIRMWARE_UPGRADE) != 0);  // Should be cleared
    EXPECT_TRUE((expected_flags & PSI_FIRMWARE_UPGRADE_ACTIVE) != 0);  // Should be set
}

// ============================================================================
// TEST: Protocol Support Reply
// @details Tests protocol support reply handler (passive - no response)
// @coverage ProtocolMessageNetwork_handle_protocol_support_reply()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_protocol_support_reply)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test protocol support reply (should not generate response)
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_REPLY);
    openlcb_msg->payload_count = 6;
    uint8_t *payload_ptr = (uint8_t *)openlcb_msg->payload;
    payload_ptr[0] = 0x80;
    payload_ptr[1] = 0x10;
    payload_ptr[2] = 0xFF;
    payload_ptr[3] = 0x00;
    payload_ptr[4] = 0x00;
    payload_ptr[5] = 0x00;
    
    ProtocolMessageNetwork_handle_protocol_support_reply(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
}

// ============================================================================
// TEST: Verify Node ID Global - Empty Payload
// @details Tests global verify node ID with no specific node ID (responds always)
// @coverage ProtocolMessageNetwork_handle_verify_node_id_global()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verify_node_id_global_empty_payload)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test verify node ID global with empty payload (responds unconditionally)
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
    openlcb_msg->payload_count = 0;

    ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
    EXPECT_EQ(outgoing_msg->dest_alias, 0);
    EXPECT_EQ(outgoing_msg->dest_id, (uint64_t) 0);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0), DEST_ID);
}

// ============================================================================
// TEST: Verify Node ID Global - Matching Payload
// @details Tests global verify node ID with matching node ID in payload
// @coverage ProtocolMessageNetwork_handle_verify_node_id_global()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verify_node_id_global_matching_payload)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test verify node ID global with matching node ID in payload
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);

    ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
    EXPECT_EQ(outgoing_msg->dest_alias, 0);
    EXPECT_EQ(outgoing_msg->dest_id, (uint64_t) 0);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0), DEST_ID);
}

// ============================================================================
// TEST: Verify Node ID Global - Non-Matching Payload
// @details Tests global verify node ID with non-matching node ID in payload
// @coverage ProtocolMessageNetwork_handle_verify_node_id_global()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verify_node_id_global_nonmatching_payload)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test verify node ID global with non-matching node ID (should not respond)
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID + 1, 0);
    
    ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, 0x00);
}

// ============================================================================
// TEST: Verify Node ID Addressed - Full Node
// @details Tests addressed verify node ID for full protocol node
// @coverage ProtocolMessageNetwork_handle_verify_node_id_addressed()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verify_node_id_addressed_full)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test addressed verify node ID (always responds, but reply is always
    // unaddressed per MessageNetworkS §3.4.2)
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_ADDRESSED);

    ProtocolMessageNetwork_handle_verify_node_id_addressed(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
    EXPECT_EQ(outgoing_msg->dest_alias, 0);
    EXPECT_EQ(outgoing_msg->dest_id, (uint64_t) 0);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0), DEST_ID);
}

// ============================================================================
// TEST: Verify Node ID Addressed - Simple Node
// @details Tests addressed verify node ID for simple protocol node
// @coverage ProtocolMessageNetwork_handle_verify_node_id_addressed()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verify_node_id_addressed_simple)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node_simple);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test addressed verify node ID for simple node (reply is still
    // unaddressed per MessageNetworkS §3.4.2)
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_ADDRESSED);

    ProtocolMessageNetwork_handle_verify_node_id_addressed(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID_SIMPLE);
    EXPECT_EQ(outgoing_msg->dest_alias, 0);
    EXPECT_EQ(outgoing_msg->dest_id, (uint64_t) 0);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0), DEST_ID);
}

// ============================================================================
// TEST: Verified Node ID - Non-Duplicate
// @details Tests verified node ID handler when node IDs don't match
// @coverage ProtocolMessageNetwork_handle_verified_node_id()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verified_node_id_non_duplicate)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    node1->state.duplicate_id_detected = false;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test verified node ID with different ID (no duplicate)
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFIED_NODE_ID);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID + 1, 0);
    
    ProtocolMessageNetwork_handle_verified_node_id(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(node1->state.duplicate_id_detected);
}

// ============================================================================
// TEST: Verified Node ID - Duplicate Detection (First Time)
// @details Tests duplicate node ID detection on first occurrence
// @coverage ProtocolMessageNetwork_handle_verified_node_id()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verified_node_id_duplicate_first_time)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    node1->state.duplicate_id_detected = false;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test verified node ID with matching ID (duplicate detected first time)
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFIED_NODE_ID);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);
    
    ProtocolMessageNetwork_handle_verified_node_id(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PC_EVENT_REPORT);
    EXPECT_EQ(outgoing_msg->payload_count, 8);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), EVENT_ID_DUPLICATE_NODE_DETECTED);
    EXPECT_TRUE(node1->state.duplicate_id_detected);
}

// ============================================================================
// TEST: Verified Node ID - Duplicate Detection (Subsequent)
// @details Tests that duplicate event is only sent once
// @coverage ProtocolMessageNetwork_handle_verified_node_id()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verified_node_id_duplicate_subsequent)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    node1->state.duplicate_id_detected = false;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // First duplicate detection
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFIED_NODE_ID);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);
    
    ProtocolMessageNetwork_handle_verified_node_id(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(node1->state.duplicate_id_detected);

    // Second duplicate detection (should not send event again)
    OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
    statemachine_info.outgoing_msg_info.valid = false;
    
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFIED_NODE_ID);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);
    
    ProtocolMessageNetwork_handle_verified_node_id(&statemachine_info);
    
    // Should not generate another message
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, 0x00);
}

// ============================================================================
// TEST: Initialization Complete
// @details Tests initialization complete handler (no response expected)
// @coverage ProtocolMessageNetwork_handle_initialization_complete()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_initialization_complete)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test initialization complete
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID + 1, 0);
    
    ProtocolMessageNetwork_handle_initialization_complete(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(node1->state.duplicate_id_detected);
}

TEST(ProtocolMessageNetwork, handle_initialization_complete_duplicate)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    node1->state.duplicate_id_detected = false;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);

    ProtocolMessageNetwork_handle_initialization_complete(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PC_EVENT_REPORT);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), EVENT_ID_DUPLICATE_NODE_DETECTED);
    EXPECT_TRUE(node1->state.duplicate_id_detected);
}

// ============================================================================
// TEST: Initialization Complete Simple
// @details Tests initialization complete simple handler with non-matching ID
// @coverage ProtocolMessageNetwork_handle_initialization_complete_simple()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_initialization_complete_simple)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test initialization complete simple
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE_SIMPLE);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID + 1, 0);
    
    ProtocolMessageNetwork_handle_initialization_complete_simple(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(node1->state.duplicate_id_detected);
}

TEST(ProtocolMessageNetwork, handle_initialization_complete_simple_duplicate)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;
    node1->state.duplicate_id_detected = false;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE_SIMPLE);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);

    ProtocolMessageNetwork_handle_initialization_complete_simple(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_PC_EVENT_REPORT);
    EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg), EVENT_ID_DUPLICATE_NODE_DETECTED);
    EXPECT_TRUE(node1->state.duplicate_id_detected);
}

// ============================================================================
// TEST: Optional Interaction Rejected
// @details Tests optional interaction rejected handler (no response expected)
// @coverage ProtocolMessageNetwork_handle_optional_interaction_rejected()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_optional_interaction_rejected)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test optional interaction rejected
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_OPTIONAL_INTERACTION_REJECTED);
    openlcb_msg->payload_count = 4;
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, ERROR_PERMANENT_NOT_IMPLEMENTED, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x00, 2);
    
    ProtocolMessageNetwork_handle_optional_interaction_rejected(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
}

// ============================================================================
// TEST: Terminate Due To Error
// @details Tests terminate due to error handler (no response expected)
// @coverage ProtocolMessageNetwork_handle_terminate_due_to_error()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_terminate_due_to_error)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test terminate due to error
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DUE_TO_ERROR);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, ERROR_PERMANENT_NOT_IMPLEMENTED, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x00, 2);
    
    ProtocolMessageNetwork_handle_terminate_due_to_error(&statemachine_info);
    
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
}

// ============================================================================
// TEST: OIR Callback Invoked With Correct Parameters
// @details Wires a mock callback and verifies error code, rejected MTI,
//          source node ID, and node pointer are passed through correctly.
// @coverage OIR callback dispatch (MessageNetworkS Section 3.5.2)
// ============================================================================

TEST(ProtocolMessageNetwork, oir_calls_callback)
{

    _reset_variables();
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network_with_callbacks);

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_OPTIONAL_INTERACTION_REJECTED);
    openlcb_msg->payload_count = 4;
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, ERROR_PERMANENT_NOT_IMPLEMENTED, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, MTI_DATAGRAM, 2);

    ProtocolMessageNetwork_handle_optional_interaction_rejected(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(_oir_callback_called);
    EXPECT_EQ(_oir_callback_node, node1);
    EXPECT_EQ(_oir_callback_source_node_id, SOURCE_ID);
    EXPECT_EQ(_oir_callback_error_code, ERROR_PERMANENT_NOT_IMPLEMENTED);
    EXPECT_EQ(_oir_callback_rejected_mti, MTI_DATAGRAM);

}

// ============================================================================
// TEST: OIR With NULL Callback (no crash)
// @details Verifies the handler works with NULL callback (existing behavior).
// @coverage OIR null-safety
// ============================================================================

TEST(ProtocolMessageNetwork, oir_null_callback)
{

    _reset_variables();
    _global_initialize();  // uses interface with NULL callbacks

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_OPTIONAL_INTERACTION_REJECTED);
    openlcb_msg->payload_count = 4;
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, ERROR_PERMANENT_NOT_IMPLEMENTED, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x00, 2);

    ProtocolMessageNetwork_handle_optional_interaction_rejected(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(_oir_callback_called);

}

// ============================================================================
// TEST: TDE Callback Invoked With Correct Parameters
// @details Wires a mock callback and verifies error code, rejected MTI,
//          source node ID, and node pointer are passed through correctly.
// @coverage TDE callback dispatch (MessageNetworkS Section 3.5.2)
// ============================================================================

TEST(ProtocolMessageNetwork, tde_calls_callback)
{

    _reset_variables();
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network_with_callbacks);

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DUE_TO_ERROR);
    openlcb_msg->payload_count = 4;
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x2000, 0);  // Temporary error
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, MTI_VERIFY_NODE_ID_ADDRESSED, 2);

    ProtocolMessageNetwork_handle_terminate_due_to_error(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(_tde_callback_called);
    EXPECT_EQ(_tde_callback_node, node1);
    EXPECT_EQ(_tde_callback_source_node_id, SOURCE_ID);
    EXPECT_EQ(_tde_callback_error_code, 0x2000);
    EXPECT_EQ(_tde_callback_rejected_mti, MTI_VERIFY_NODE_ID_ADDRESSED);

}

// ============================================================================
// TEST: TDE With NULL Callback (no crash)
// @details Verifies the handler works with NULL callback (existing behavior).
// @coverage TDE null-safety
// ============================================================================

TEST(ProtocolMessageNetwork, tde_null_callback)
{

    _reset_variables();
    _global_initialize();  // uses interface with NULL callbacks

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DUE_TO_ERROR);
    openlcb_msg->payload_count = 4;
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, ERROR_PERMANENT_NOT_IMPLEMENTED, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x00, 2);

    ProtocolMessageNetwork_handle_terminate_due_to_error(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(_tde_callback_called);

}

// ============================================================================
// TEST: OIR Short Payload (< 4 bytes)
// @details Verifies graceful handling when payload has only 2 bytes (error
//          code present but no rejected MTI).  Callback should still fire
//          with rejected_mti = 0.
// @coverage OIR short payload robustness
// ============================================================================

TEST(ProtocolMessageNetwork, oir_short_payload)
{

    _reset_variables();
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network_with_callbacks);

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_OPTIONAL_INTERACTION_REJECTED);
    openlcb_msg->payload_count = 2;
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x1043, 0);

    ProtocolMessageNetwork_handle_optional_interaction_rejected(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(_oir_callback_called);
    EXPECT_EQ(_oir_callback_error_code, 0x1043);
    EXPECT_EQ(_oir_callback_rejected_mti, 0x0000);  // Not present in payload

}

// ============================================================================
// SECTION 2: EDGE CASE AND BOUNDARY TESTS
// ============================================================================

// ============================================================================
// TEST: Protocol Support with All Flags Set
// @details Tests protocol support inquiry with maximum flags
// @coverage Protocol support flag encoding
// ============================================================================

TEST(ProtocolMessageNetwork, protocol_support_all_flags)
{
    _reset_variables();
    _global_initialize();

    // Create node with all protocol support flags set
    node_parameters_t params_all_flags = _node_parameters_main_node;
    params_all_flags.protocol_support = 0xFFFFFFFFFFFFFFFF;

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &params_all_flags);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test protocol support inquiry with all flags
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);
    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    uint8_t *payload_ptr = (uint8_t *)outgoing_msg->payload;
    EXPECT_EQ(payload_ptr[0], 0xFF);
    EXPECT_EQ(payload_ptr[1], 0xFF);
    EXPECT_EQ(payload_ptr[2], 0xFF);
    EXPECT_EQ(payload_ptr[3], 0xFF);
    EXPECT_EQ(payload_ptr[4], 0xFF);
    EXPECT_EQ(payload_ptr[5], 0xFF);
}

// ============================================================================
// TEST: Protocol Support with No Flags Set
// @details Tests protocol support inquiry with minimal flags
// @coverage Protocol support flag encoding edge case
// ============================================================================

TEST(ProtocolMessageNetwork, protocol_support_no_flags)
{
    _reset_variables();
    _global_initialize();

    // Create node with no protocol support flags
    node_parameters_t params_no_flags = _node_parameters_main_node;
    params_no_flags.protocol_support = 0x0000000000000000;

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &params_no_flags);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Test protocol support inquiry with no flags
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);
    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);
    
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    uint8_t *payload_ptr = (uint8_t *)outgoing_msg->payload;
    EXPECT_EQ(payload_ptr[0], 0x00);
    EXPECT_EQ(payload_ptr[1], 0x00);
    EXPECT_EQ(payload_ptr[2], 0x00);
    EXPECT_EQ(payload_ptr[3], 0x00);
    EXPECT_EQ(payload_ptr[4], 0x00);
    EXPECT_EQ(payload_ptr[5], 0x00);
}

// ============================================================================
// TEST: Protocol Support with Upper 24 Bits Set
// @details Tests protocol support inquiry encodes bits 24-47 into bytes 3-5
// @coverage Protocol support 48-bit encoding
// ============================================================================

TEST(ProtocolMessageNetwork, protocol_support_upper_24_bits)
{

    _reset_variables();
    _global_initialize();

    node_parameters_t params_upper = _node_parameters_main_node;
    params_upper.protocol_support = 0x0000AABBCC000000;

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &params_upper);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID,
            DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);

    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->payload_count, 6);

    uint8_t *payload_ptr = (uint8_t *)outgoing_msg->payload;

    // Bytes 0-2: bits 16-23, 8-15, 0-7 -- all zero (no lower flags set)
    EXPECT_EQ(payload_ptr[0], 0x00);
    EXPECT_EQ(payload_ptr[1], 0x00);
    EXPECT_EQ(payload_ptr[2], 0x00);

    // Bytes 3-5: bits 40-47, 32-39, 24-31 -- should contain AA, BB, CC
    EXPECT_EQ(payload_ptr[3], 0xAA);
    EXPECT_EQ(payload_ptr[4], 0xBB);
    EXPECT_EQ(payload_ptr[5], 0xCC);

}

// ============================================================================
// TEST: Protocol Support with Mixed Lower and Upper Bits
// @details Tests protocol support inquiry encodes all 48 bits correctly
// @coverage Protocol support 48-bit encoding
// ============================================================================

TEST(ProtocolMessageNetwork, protocol_support_mixed_48_bits)
{

    _reset_variables();
    _global_initialize();

    node_parameters_t params_mixed = _node_parameters_main_node;
    params_mixed.protocol_support = 0x0000112233445566;

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &params_mixed);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID,
            DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);

    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->payload_count, 6);

    uint8_t *payload_ptr = (uint8_t *)outgoing_msg->payload;

    // Bytes 0-2: lower 24 bits (bits 16-23=0x44, 8-15=0x55, 0-7=0x66)
    EXPECT_EQ(payload_ptr[0], 0x44);
    EXPECT_EQ(payload_ptr[1], 0x55);
    EXPECT_EQ(payload_ptr[2], 0x66);

    // Bytes 3-5: upper 24 bits (bits 40-47=0x11, 32-39=0x22, 24-31=0x33)
    EXPECT_EQ(payload_ptr[3], 0x11);
    EXPECT_EQ(payload_ptr[4], 0x22);
    EXPECT_EQ(payload_ptr[5], 0x33);

}

// ============================================================================
// TEST: OIR Empty Payload (0 bytes)
// @details Verifies graceful handling when payload has 0 bytes.  Callback
//          should still fire with error_code = 0 and rejected_mti = 0.
//          Covers line 270 FALSE and line 277 FALSE branches.
// @coverage OIR empty payload robustness
// ============================================================================

TEST(ProtocolMessageNetwork, oir_empty_payload)
{

    _reset_variables();
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network_with_callbacks);

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_OPTIONAL_INTERACTION_REJECTED);
    openlcb_msg->payload_count = 0;

    ProtocolMessageNetwork_handle_optional_interaction_rejected(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(_oir_callback_called);
    EXPECT_EQ(_oir_callback_node, node1);
    EXPECT_EQ(_oir_callback_source_node_id, SOURCE_ID);
    EXPECT_EQ(_oir_callback_error_code, 0x0000);
    EXPECT_EQ(_oir_callback_rejected_mti, 0x0000);

}

// ============================================================================
// TEST: TDE Empty Payload (0 bytes)
// @details Verifies graceful handling when payload has 0 bytes.  Callback
//          should still fire with error_code = 0 and rejected_mti = 0.
//          Covers line 307 FALSE and line 314 FALSE branches.
// @coverage TDE empty payload robustness
// ============================================================================

TEST(ProtocolMessageNetwork, tde_empty_payload)
{

    _reset_variables();
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network_with_callbacks);

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DUE_TO_ERROR);
    openlcb_msg->payload_count = 0;

    ProtocolMessageNetwork_handle_terminate_due_to_error(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(_tde_callback_called);
    EXPECT_EQ(_tde_callback_node, node1);
    EXPECT_EQ(_tde_callback_source_node_id, SOURCE_ID);
    EXPECT_EQ(_tde_callback_error_code, 0x0000);
    EXPECT_EQ(_tde_callback_rejected_mti, 0x0000);

}

// ============================================================================
// TEST: TDE Short Payload (2 bytes)
// @details Verifies graceful handling when payload has only 2 bytes (error
//          code present but no rejected MTI).  Callback should still fire
//          with rejected_mti = 0.  Covers line 314 FALSE branch.
// @coverage TDE short payload robustness
// ============================================================================

TEST(ProtocolMessageNetwork, tde_short_payload)
{

    _reset_variables();
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network_with_callbacks);

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DUE_TO_ERROR);
    openlcb_msg->payload_count = 2;
    OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x2000, 0);

    ProtocolMessageNetwork_handle_terminate_due_to_error(&statemachine_info);

    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(_tde_callback_called);
    EXPECT_EQ(_tde_callback_error_code, 0x2000);
    EXPECT_EQ(_tde_callback_rejected_mti, 0x0000);  // Not present in payload

}

// ============================================================================
// TEST SUMMARY
// ============================================================================
//
// Total Tests: 21
// - Basic Functionality: 12 tests
// - Edge Cases & Boundaries: 9 tests
//
// Coverage: 100% branch (all 30 branches taken)
//
// Interface: Empty struct (reserved for future callbacks)
//
// Public Functions Tested (9):
// - ProtocolMessageNetwork_initialize()
// - ProtocolMessageNetwork_handle_initialization_complete()
// - ProtocolMessageNetwork_handle_initialization_complete_simple()
// - ProtocolMessageNetwork_handle_protocol_support_inquiry() - 100% coverage
//   * Normal operation path (firmware_upgrade_active = false)
//   * Firmware upgrade active path (firmware_upgrade_active = true)
//   * PSI_FIRMWARE_UPGRADE → PSI_FIRMWARE_UPGRADE_ACTIVE bit manipulation
// - ProtocolMessageNetwork_handle_protocol_support_reply()
// - ProtocolMessageNetwork_handle_verify_node_id_global()
// - ProtocolMessageNetwork_handle_verify_node_id_addressed()
// - ProtocolMessageNetwork_handle_verified_node_id()
// - ProtocolMessageNetwork_handle_optional_interaction_rejected()
// - ProtocolMessageNetwork_handle_terminate_due_to_error()
//
// Static Helper Functions (tested indirectly):
// - _load_duplicate_node_id()
// - _load_verified_node_id()
//
// Test Categories:
// 1. Initialization & Setup
// 2. Protocol Support (Full/Simple nodes, All/No flags, Firmware upgrade active)
// 3. Node Verification (Global/Addressed, With/Without payload)
// 4. Duplicate Detection (First/Subsequent occurrences)
// 5. Initialization Messages (Complete/Simple)
// 6. Error Messages (OIR/Terminate)
// 7. MTI Selection (Full vs Simple nodes)
// 8. Boundary Conditions (Max/Min protocol flags)
//
// ============================================================================
