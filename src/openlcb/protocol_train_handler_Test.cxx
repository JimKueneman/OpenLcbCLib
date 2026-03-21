/** \copyright
* Copyright (c) 2026, Jim Kueneman
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
* @file protocol_train_handler_Test.cxx
* @brief Unit tests for Train Control Protocol handler (Layer 1)
*
* Test Organization:
* - Section 1: Initialization Tests
* - Section 2: Set Speed / Emergency Stop (state update + notifier)
* - Section 3: Set Function (state storage + notifier)
* - Section 4: Query Speeds / Query Function (auto-reply)
* - Section 5: Controller Config (assign/release/query/changed)
* - Section 6: Listener Config (attach/detach/query)
* - Section 7: Management (reserve/release/noop)
* - Section 8: Reply Dispatch Tests (throttle side)
* - Section 9: NULL Callback Safety Tests
* - Section 10: Edge Cases
* - Section 12: Global Emergency Events (event-based estop/eoff)
* - Section 13: Conformance Test Sequences (TN Section 2.2 - 2.11)
*
* @author Jim Kueneman
* @date 17 Feb 2026
*/

#include "test/main_Test.hxx"

#include <cstring>

#include "protocol_train_handler.h"
#include "openlcb_application_train.h"
#include "openlcb_float16.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"


// ============================================================================
// Test Constants
// ============================================================================

#define TEST_SOURCE_ALIAS 0x222
#define TEST_SOURCE_ID 0x010203040506ULL
#define TEST_DEST_ALIAS 0xBBB
#define TEST_DEST_ID 0x060504030201ULL
#define TEST_CONTROLLER_NODE_ID 0x0A0B0C0D0E0FULL
#define TEST_CONTROLLER_NODE_ID_2 0x0F0E0D0C0B0AULL
#define TEST_LISTENER_NODE_ID 0x112233445566ULL


// ============================================================================
// Test Tracking Variables
// ============================================================================

static int notifier_called = 0;

static uint16_t last_speed_float16 = 0;
static uint32_t last_fn_address = 0;
static uint16_t last_fn_value = 0;
static uint64_t last_node_id = 0;
static uint8_t last_flags = 0;
static uint8_t last_result = 0;
static uint8_t last_status = 0;
static uint16_t last_set_speed = 0;
static uint16_t last_commanded_speed = 0;
static uint16_t last_actual_speed = 0;
static uint8_t last_count = 0;
static uint8_t last_index = 0;
static uint32_t last_timeout = 0;
static openlcb_node_t *last_notified_node = NULL;

// Decision callback return values (configurable per test)
static bool decision_assign_result = true;
static bool decision_changed_result = true;
// Emergency callback tracking
static train_emergency_type_enum last_emergency_type = TRAIN_EMERGENCY_TYPE_ESTOP;
static int emergency_entered_called = 0;
static int emergency_exited_called = 0;


// ============================================================================
// Reset
// ============================================================================

static void _reset_tracking(void) {

    notifier_called = 0;
    last_speed_float16 = 0;
    last_fn_address = 0;
    last_fn_value = 0;
    last_node_id = 0;
    last_flags = 0;
    last_result = 0;
    last_status = 0;
    last_set_speed = 0;
    last_commanded_speed = 0;
    last_actual_speed = 0;
    last_count = 0;
    last_index = 0;
    last_timeout = 0;
    last_notified_node = NULL;
    decision_assign_result = true;
    decision_changed_result = true;
    last_emergency_type = TRAIN_EMERGENCY_TYPE_ESTOP;
    emergency_entered_called = 0;
    emergency_exited_called = 0;

}


// ============================================================================
// Mock Callbacks — Train-node side: notifiers
// ============================================================================

static void _mock_on_speed_changed(openlcb_node_t *openlcb_node, uint16_t speed_float16) {

    notifier_called = 1;
    last_notified_node = openlcb_node;
    last_speed_float16 = speed_float16;

}

static void _mock_on_function_changed(openlcb_node_t *openlcb_node,
        uint32_t fn_address, uint16_t fn_value) {

    notifier_called = 2;
    last_notified_node = openlcb_node;
    last_fn_address = fn_address;
    last_fn_value = fn_value;

}

static void _mock_on_emergency_entered(openlcb_node_t *openlcb_node,
        train_emergency_type_enum emergency_type) {

    notifier_called = 3;
    last_notified_node = openlcb_node;
    last_emergency_type = emergency_type;
    emergency_entered_called++;

}

static void _mock_on_emergency_exited(openlcb_node_t *openlcb_node,
        train_emergency_type_enum emergency_type) {

    notifier_called = 8;
    last_notified_node = openlcb_node;
    last_emergency_type = emergency_type;
    emergency_exited_called++;

}

static void _mock_on_controller_assigned(openlcb_node_t *openlcb_node,
        uint64_t controller_node_id) {

    notifier_called = 4;
    last_notified_node = openlcb_node;
    last_node_id = controller_node_id;

}

static void _mock_on_controller_released(openlcb_node_t *openlcb_node) {

    notifier_called = 5;
    last_notified_node = openlcb_node;

}

static void _mock_on_listener_changed(openlcb_node_t *openlcb_node) {

    notifier_called = 6;
    last_notified_node = openlcb_node;

}

static void _mock_on_heartbeat_timeout(openlcb_node_t *openlcb_node) {

    notifier_called = 7;
    last_notified_node = openlcb_node;

}


// ============================================================================
// Mock Callbacks — Train-node side: decision callbacks
// ============================================================================

static bool _mock_on_controller_assign_request(openlcb_node_t *openlcb_node,
        uint64_t current_controller, uint64_t requesting_controller) {

    last_notified_node = openlcb_node;
    return decision_assign_result;

}

static bool _mock_on_controller_changed_request(openlcb_node_t *openlcb_node,
        uint64_t new_controller) {

    last_notified_node = openlcb_node;
    return decision_changed_result;

}

// ============================================================================
// Mock Callbacks — Throttle side: notifiers (receiving replies from train)
// ============================================================================

static void _mock_on_query_speeds_reply(openlcb_node_t *openlcb_node,
        uint16_t set_speed, uint8_t status,
        uint16_t commanded_speed, uint16_t actual_speed) {

    notifier_called = 101;
    last_set_speed = set_speed;
    last_status = status;
    last_commanded_speed = commanded_speed;
    last_actual_speed = actual_speed;

}

static void _mock_on_query_function_reply(openlcb_node_t *openlcb_node,
        uint32_t fn_address, uint16_t fn_value) {

    notifier_called = 102;
    last_fn_address = fn_address;
    last_fn_value = fn_value;

}

static void _mock_on_controller_assign_reply(openlcb_node_t *openlcb_node,
        uint8_t result, node_id_t current_controller) {

    notifier_called = 103;
    last_result = result;
    last_node_id = current_controller;

}

static void _mock_on_controller_query_reply(openlcb_node_t *openlcb_node,
        uint8_t flags, uint64_t controller_node_id) {

    notifier_called = 104;
    last_flags = flags;
    last_node_id = controller_node_id;

}

static void _mock_on_controller_changed_notify_reply(openlcb_node_t *openlcb_node,
        uint8_t result) {

    notifier_called = 105;
    last_result = result;

}

static void _mock_on_listener_attach_reply(openlcb_node_t *openlcb_node,
        uint64_t node_id, uint8_t result) {

    notifier_called = 106;
    last_node_id = node_id;
    last_result = result;

}

static void _mock_on_listener_detach_reply(openlcb_node_t *openlcb_node,
        uint64_t node_id, uint8_t result) {

    notifier_called = 107;
    last_node_id = node_id;
    last_result = result;

}

static void _mock_on_listener_query_reply(openlcb_node_t *openlcb_node,
        uint8_t count, uint8_t index, uint8_t flags, uint64_t node_id) {

    notifier_called = 108;
    last_count = count;
    last_index = index;
    last_flags = flags;
    last_node_id = node_id;

}

static void _mock_on_reserve_reply(openlcb_node_t *openlcb_node, uint8_t result) {

    notifier_called = 109;
    last_result = result;

}

static void _mock_on_heartbeat_request(openlcb_node_t *openlcb_node,
        uint32_t timeout_seconds) {

    notifier_called = 110;
    last_timeout = timeout_seconds;

}


// ============================================================================
// Interface Structures
// ============================================================================

static interface_protocol_train_handler_t _interface_all = {

    // Train-node side: notifiers
    .on_speed_changed = &_mock_on_speed_changed,
    .on_function_changed = &_mock_on_function_changed,
    .on_emergency_entered = &_mock_on_emergency_entered,
    .on_emergency_exited = &_mock_on_emergency_exited,
    .on_controller_assigned = &_mock_on_controller_assigned,
    .on_controller_released = &_mock_on_controller_released,
    .on_listener_changed = &_mock_on_listener_changed,
    .on_heartbeat_timeout = &_mock_on_heartbeat_timeout,

    // Train-node side: decision callbacks
    .on_controller_assign_request = &_mock_on_controller_assign_request,
    .on_controller_changed_request = &_mock_on_controller_changed_request,
    // Throttle side: notifiers
    .on_query_speeds_reply = &_mock_on_query_speeds_reply,
    .on_query_function_reply = &_mock_on_query_function_reply,
    .on_controller_assign_reply = &_mock_on_controller_assign_reply,
    .on_controller_query_reply = &_mock_on_controller_query_reply,
    .on_controller_changed_notify_reply = &_mock_on_controller_changed_notify_reply,
    .on_listener_attach_reply = &_mock_on_listener_attach_reply,
    .on_listener_detach_reply = &_mock_on_listener_detach_reply,
    .on_listener_query_reply = &_mock_on_listener_query_reply,
    .on_reserve_reply = &_mock_on_reserve_reply,
    .on_heartbeat_request = &_mock_on_heartbeat_request,

};

static interface_protocol_train_handler_t _interface_nulls = {

    .on_speed_changed = NULL,
    .on_function_changed = NULL,
    .on_emergency_entered = NULL,
    .on_emergency_exited = NULL,
    .on_controller_assigned = NULL,
    .on_controller_released = NULL,
    .on_listener_changed = NULL,
    .on_heartbeat_timeout = NULL,

    .on_controller_assign_request = NULL,
    .on_controller_changed_request = NULL,
    .on_query_speeds_reply = NULL,
    .on_query_function_reply = NULL,
    .on_controller_assign_reply = NULL,
    .on_controller_query_reply = NULL,
    .on_controller_changed_notify_reply = NULL,
    .on_listener_attach_reply = NULL,
    .on_listener_detach_reply = NULL,
    .on_listener_query_reply = NULL,
    .on_reserve_reply = NULL,
    .on_heartbeat_request = NULL,

};

static interface_openlcb_application_train_t _interface_app_train = {

    .send_openlcb_msg = NULL,

};

static interface_openlcb_node_t _interface_openlcb_node = {};

static node_parameters_t _test_node_parameters = {

    .snip = {
        .mfg_version = 4,
        .name = "Test Train Node",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

    .configuration_options = {
        .high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY
    },

};


// ============================================================================
// Test Helpers
// ============================================================================

static void _global_initialize(void) {

    ProtocolTrainHandler_initialize(&_interface_all);
    OpenLcbApplicationTrain_initialize(&_interface_app_train);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

}

static void _global_initialize_with_nulls(void) {

    ProtocolTrainHandler_initialize(&_interface_nulls);
    OpenLcbApplicationTrain_initialize(&_interface_app_train);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

}

static void _setup_statemachine(openlcb_statemachine_info_t *sm, openlcb_node_t *node,
        openlcb_msg_t *incoming, openlcb_msg_t *outgoing) {

    sm->openlcb_node = node;
    sm->incoming_msg_info.msg_ptr = incoming;
    sm->incoming_msg_info.enumerate = false;
    sm->outgoing_msg_info.msg_ptr = outgoing;
    sm->outgoing_msg_info.enumerate = false;
    sm->outgoing_msg_info.valid = false;

    incoming->source_id = TEST_SOURCE_ID;
    incoming->source_alias = TEST_SOURCE_ALIAS;
    incoming->dest_id = TEST_DEST_ID;
    incoming->dest_alias = TEST_DEST_ALIAS;

}

static openlcb_node_t* _create_train_node(void) {

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    OpenLcbApplicationTrain_setup(node);

    return node;

}


// ============================================================================
// Section 1: Initialization Tests
// ============================================================================

TEST(ProtocolTrainHandler, initialize)
{

    _global_initialize();

    // Verify initialize does not crash and handler is ready
    // Callback wiring is tested indirectly by the command/reply tests below

}

TEST(ProtocolTrainHandler, initialize_with_nulls)
{

    _global_initialize_with_nulls();

    // Verify initialize with NULL callbacks does not crash

}


// ============================================================================
// Section 2: Set Speed / Emergency Stop (state update + notifier)
// ============================================================================

TEST(ProtocolTrainHandler, command_set_speed_updates_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Speed: byte 0 = 0x00, bytes 1-2 = float16 speed (0x3C00 = 1.0)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Verify state updated
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    EXPECT_NE(state, nullptr);
    EXPECT_EQ(state->set_speed, 0x3C00);
    EXPECT_FALSE(state->estop_active);

    // Verify notifier fired
    EXPECT_EQ(notifier_called, 1);
    EXPECT_EQ(last_speed_float16, 0x3C00);
    EXPECT_EQ(last_notified_node, node);

}

TEST(ProtocolTrainHandler, command_set_speed_clears_estop)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->estop_active = true;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x4000, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(state->estop_active);
    EXPECT_EQ(state->set_speed, 0x4000);

}

TEST(ProtocolTrainHandler, command_emergency_stop_updates_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    // Set a forward speed first
    state->set_speed = 0x3C00;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Verify estop active
    EXPECT_TRUE(state->estop_active);
    // Direction preserved (forward), speed zeroed
    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);

    // Verify notifier fired
    EXPECT_EQ(notifier_called, 3);
    EXPECT_EQ(last_notified_node, node);

}

TEST(ProtocolTrainHandler, command_emergency_stop_preserves_reverse)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    // Set a reverse speed (sign bit set)
    state->set_speed = 0xBC00;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Direction preserved (reverse), speed zeroed
    EXPECT_TRUE(state->estop_active);
    EXPECT_EQ(state->set_speed, FLOAT16_NEGATIVE_ZERO);

}


// ============================================================================
// Section 3: Set Function (state storage + notifier)
// ============================================================================

TEST(ProtocolTrainHandler, command_set_function_fires_notifier)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x05, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(notifier_called, 2);
    EXPECT_EQ(last_fn_address, (uint32_t) 5);
    EXPECT_EQ(last_fn_value, 0x0001);
    EXPECT_EQ(last_notified_node, node);

}

TEST(ProtocolTrainHandler, command_set_function_large_address)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Function address 0x123456
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x12, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x34, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x56, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0xFFFF, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(notifier_called, 2);
    EXPECT_EQ(last_fn_address, (uint32_t) 0x123456);
    EXPECT_EQ(last_fn_value, 0xFFFF);

}

TEST(ProtocolTrainHandler, command_set_function_stores_in_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Function F5 = 1
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x05, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->functions[5], 0x0001);

}

TEST(ProtocolTrainHandler, command_set_function_stores_f28)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Function F28 = 0xABCD
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 28, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0xABCD, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->functions[28], 0xABCD);

}

TEST(ProtocolTrainHandler, command_set_function_out_of_bounds_no_crash)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Function F30 (out of default bounds of 29) = 0x0001
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 30, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Notifier fires but no state storage
    EXPECT_EQ(notifier_called, 2);
    EXPECT_EQ(last_fn_address, (uint32_t) 30);
    EXPECT_EQ(last_fn_value, 0x0001);

}

TEST(ProtocolTrainHandler, command_query_function_reads_stored_value)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    // Pre-set a function value in state
    state->functions[5] = 0x00FF;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x05, 3);
    incoming->payload_count = 4;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply contains value from stored state (no callback needed)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x00FF);

}

TEST(ProtocolTrainHandler, command_set_then_query_function_roundtrip)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Function F10 = 0x1234
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 10, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x1234, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reset outgoing for query
    sm.outgoing_msg_info.valid = false;

    // Query Function F10
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 10, 3);
    incoming->payload_count = 4;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Verify round-trip consistency
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x1234);

}


// ============================================================================
// Section 4: Query Speeds / Query Function (auto-reply)
// ============================================================================

TEST(ProtocolTrainHandler, command_query_speeds_auto_reply)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    state->set_speed = 0x3C00;
    state->estop_active = true;
    state->commanded_speed = 0x3E00;
    state->actual_speed = 0x3A00;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Verify reply was built
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing->mti, MTI_TRAIN_REPLY);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_QUERY_SPEEDS);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), 0x3C00);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0x01);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x3E00);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 6), 0x3A00);

}

TEST(ProtocolTrainHandler, command_query_speeds_no_estop)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    state->set_speed = 0x4000;
    state->estop_active = false;
    state->commanded_speed = FLOAT16_NAN;
    state->actual_speed = FLOAT16_NAN;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0x00);

}

TEST(ProtocolTrainHandler, command_query_function_default_returns_zero)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x05, 3);
    incoming->payload_count = 4;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply built with default value 0
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x0000);

}


// ============================================================================
// Section 5: Controller Config (assign/release/query/changed)
// ============================================================================

TEST(ProtocolTrainHandler, command_controller_assign_no_existing)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    EXPECT_EQ(state->controller_node_id, (uint64_t) 0);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // State updated
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);

    // Reply built with result=0 (accept) — 3 bytes, no Node ID appended
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_CONTROLLER_ASSIGN);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);
    EXPECT_EQ(outgoing->payload_count, 3);

    // Notifier fired
    EXPECT_EQ(notifier_called, 4);
    EXPECT_EQ(last_node_id, TEST_CONTROLLER_NODE_ID);

}

TEST(ProtocolTrainHandler, command_controller_assign_same_controller)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Accept (same controller)
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);
    EXPECT_EQ(notifier_called, 4);

}

TEST(ProtocolTrainHandler, command_controller_assign_different_accept)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    // Decision callback returns true (accept)
    decision_assign_result = true;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // New controller accepted
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID_2);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);
    EXPECT_EQ(notifier_called, 4);

}

TEST(ProtocolTrainHandler, command_controller_assign_different_reject)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    // Decision callback returns false (reject)
    decision_assign_result = false;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Original controller preserved
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);

    // Reply: 9 bytes — 3 header + 6-byte Node ID of current controller (TN §2.8)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_CONTROLLER_ASSIGN);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0xFF);
    EXPECT_EQ(outgoing->payload_count, 9);

    // Node ID in reply matches current controller
    node_id_t reply_controller =
            OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 3);
    EXPECT_EQ(reply_controller, TEST_CONTROLLER_NODE_ID);

    // Notifier NOT called when rejected
    EXPECT_NE(notifier_called, 4);

}

TEST(ProtocolTrainHandler, command_controller_release_matching)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Controller cleared
    EXPECT_EQ(state->controller_node_id, (uint64_t) 0);
    // Notifier fired
    EXPECT_EQ(notifier_called, 5);

}

TEST(ProtocolTrainHandler, command_controller_release_non_matching)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Release with a different node ID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Controller NOT cleared (non-matching)
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);
    // Notifier NOT fired
    EXPECT_NE(notifier_called, 5);

}

TEST(ProtocolTrainHandler, command_controller_query_with_controller)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_QUERY, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Auto-reply with flags=0x01 and controller ID
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_CONTROLLER_QUERY);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x01);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 3), TEST_CONTROLLER_NODE_ID);

}

TEST(ProtocolTrainHandler, command_controller_query_no_controller)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = 0;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_QUERY, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // flags=0, node_id=0
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 3), (uint64_t) 0);

}

TEST(ProtocolTrainHandler, command_controller_changed_accept)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    decision_changed_result = true;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with result=0 (accepted)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_CONTROLLER_CHANGED);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

}

TEST(ProtocolTrainHandler, command_controller_changed_reject)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    decision_changed_result = false;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0xFF);

}

TEST(ProtocolTrainHandler, command_controller_unknown_sub)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFF, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(notifier_called, 0);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}


// ============================================================================
// Section 6: Listener Config (attach/detach/query)
// ============================================================================

TEST(ProtocolTrainHandler, command_listener_attach_success)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_FLAG_REVERSE, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Listener added to state
    EXPECT_EQ(state->listener_count, 1);
    EXPECT_EQ(state->listeners[0].node_id, TEST_LISTENER_NODE_ID);
    EXPECT_EQ(state->listeners[0].flags, TRAIN_LISTENER_FLAG_REVERSE);

    // Reply built with result=0 (success)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_LISTENER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_LISTENER_ATTACH);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 2), TEST_LISTENER_NODE_ID);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);

    // Notifier fired
    EXPECT_EQ(notifier_called, 6);

}

TEST(ProtocolTrainHandler, command_listener_detach_success)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    // Pre-populate a listener directly in state
    state->listeners[0].node_id = TEST_LISTENER_NODE_ID;
    state->listeners[0].flags = 0x00;
    state->listener_count = 1;
    EXPECT_EQ(state->listener_count, 1);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Listener removed
    EXPECT_EQ(state->listener_count, 0);

    // Reply with result=0
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);

    // Notifier fired
    EXPECT_EQ(notifier_called, 6);

}

TEST(ProtocolTrainHandler, command_listener_detach_not_found)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with result=0xFF (failure)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0xFF);

    // Notifier NOT fired on failure
    EXPECT_NE(notifier_called, 6);

}

TEST(ProtocolTrainHandler, command_listener_query_with_listeners)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    // Pre-populate two listeners directly in state
    state->listeners[0].node_id = TEST_LISTENER_NODE_ID;
    state->listeners[0].flags = TRAIN_LISTENER_FLAG_LINK_F0;
    state->listeners[1].node_id = 0xAABBCCDDEEFFULL;
    state->listeners[1].flags = 0x00;
    state->listener_count = 2;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // Listener index = 0
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with first listener entry
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_LISTENER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_LISTENER_QUERY);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 2);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4), TRAIN_LISTENER_FLAG_LINK_F0);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 5), TEST_LISTENER_NODE_ID);

}

TEST(ProtocolTrainHandler, command_listener_query_no_listeners)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // Listener index = 0
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with count=0
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4), 0);

}

TEST(ProtocolTrainHandler, command_listener_unknown_sub)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFF, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(notifier_called, 0);

}


// ============================================================================
// Section 7: Management (reserve/release/noop)
// ============================================================================

TEST(ProtocolTrainHandler, command_management_reserve)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    EXPECT_EQ(state->reserved_node_count, 0);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // State updated
    EXPECT_EQ(state->reserved_node_count, 1);

    // Reply built
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_MANAGEMENT);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_MGMT_RESERVE);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

}

TEST(ProtocolTrainHandler, command_management_reserve_same_source_succeeds)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;
    incoming->source_id = 0x050701010099;

    // First reserve succeeds
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 1);
    EXPECT_EQ(state->reserved_by_node_id, 0x050701010099);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Second reserve from same source succeeds
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 1);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

}

TEST(ProtocolTrainHandler, command_management_reserve_different_source_fails)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;
    incoming->source_id = 0x050701010099;

    // First reserve succeeds
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 1);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Second reserve from different source fails
    sm.outgoing_msg_info.valid = false;
    incoming->source_id = 0x050701010088;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 1);
    EXPECT_NE(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

}

TEST(ProtocolTrainHandler, command_management_release)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->reserved_node_count = 1;
    state->reserved_by_node_id = 0x050701010099;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 0);
    EXPECT_EQ(state->reserved_by_node_id, (node_id_t) 0);

    // No reply for release
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(ProtocolTrainHandler, command_management_release_at_zero)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->reserved_node_count = 0;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Should not underflow
    EXPECT_EQ(state->reserved_node_count, 0);

}

TEST(ProtocolTrainHandler, command_management_noop_resets_heartbeat)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 5;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_NOOP, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Heartbeat counter reset to timeout_s * 10
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 100);

}

TEST(ProtocolTrainHandler, command_management_noop_heartbeat_disabled)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 0;
    state->heartbeat_counter_100ms = 0;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_NOOP, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter stays at 0 when heartbeat disabled
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);

}

TEST(ProtocolTrainHandler, command_management_unknown_sub)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFF, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, command_unknown_instruction)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFF, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(notifier_called, 0);

}


// ============================================================================
// Section 8: Reply Dispatch Tests (throttle side)
// ============================================================================

TEST(ProtocolTrainHandler, reply_query_speeds)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3E00, 4);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3A00, 6);
    incoming->payload_count = 8;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 101);
    EXPECT_EQ(last_set_speed, 0x3C00);
    EXPECT_EQ(last_status, 0x01);
    EXPECT_EQ(last_commanded_speed, 0x3E00);
    EXPECT_EQ(last_actual_speed, 0x3A00);

}

TEST(ProtocolTrainHandler, reply_query_function)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x0A, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 102);
    EXPECT_EQ(last_fn_address, (uint32_t) 0x00010A);
    EXPECT_EQ(last_fn_value, 0x0001);

}

TEST(ProtocolTrainHandler, reply_controller_assign)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Accept reply — 3 bytes, no Node ID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 103);
    EXPECT_EQ(last_result, 0x00);
    EXPECT_EQ(last_node_id, (uint64_t) 0);  // No controller Node ID on accept

}

TEST(ProtocolTrainHandler, reply_controller_assign_reject_with_node_id)
{
    // Throttle receives a reject reply with 6-byte Node ID of current controller

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Reject reply — 9 bytes: 3 header + 6-byte Node ID of current controller
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFF, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 103);
    EXPECT_EQ(last_result, 0xFF);
    EXPECT_EQ(last_node_id, TEST_CONTROLLER_NODE_ID);

}

TEST(ProtocolTrainHandler, reply_controller_query)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 104);
    EXPECT_EQ(last_flags, 0x01);
    EXPECT_EQ(last_node_id, TEST_CONTROLLER_NODE_ID);

}

TEST(ProtocolTrainHandler, reply_controller_changed_notify)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 105);
    EXPECT_EQ(last_result, 0x00);

}

TEST(ProtocolTrainHandler, reply_listener_attach)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 8);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 106);
    EXPECT_EQ(last_node_id, TEST_LISTENER_NODE_ID);
    EXPECT_EQ(last_result, 0x00);

}

TEST(ProtocolTrainHandler, reply_listener_detach)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 8);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 107);
    EXPECT_EQ(last_node_id, TEST_LISTENER_NODE_ID);
    EXPECT_EQ(last_result, 0x01);

}

TEST(ProtocolTrainHandler, reply_listener_query)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 3, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 1, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_FLAG_LINK_F0, 4);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 5);
    incoming->payload_count = 11;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 108);
    EXPECT_EQ(last_count, 3);
    EXPECT_EQ(last_index, 1);
    EXPECT_EQ(last_flags, TRAIN_LISTENER_FLAG_LINK_F0);
    EXPECT_EQ(last_node_id, TEST_LISTENER_NODE_ID);

}

TEST(ProtocolTrainHandler, reply_management_reserve)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 109);
    EXPECT_EQ(last_result, 0x00);

}

TEST(ProtocolTrainHandler, reply_management_heartbeat)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_NOOP, 1);
    // 3-byte timeout: 10 seconds = 0x00, 0x00, 0x0A
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x0A, 4);
    incoming->payload_count = 5;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 110);
    EXPECT_EQ(last_timeout, (uint32_t) 10);

}

TEST(ProtocolTrainHandler, reply_unknown_instruction)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFF, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}


// ============================================================================
// Section 9: NULL Callback Safety Tests
// ============================================================================

TEST(ProtocolTrainHandler, null_callbacks_commands_no_crash)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Speed with NULL notifier — should still update state
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    EXPECT_EQ(state->set_speed, 0x3C00);
    EXPECT_EQ(notifier_called, 0);

    // Set Function with NULL notifier
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);
    EXPECT_EQ(notifier_called, 0);

    // Emergency Stop with NULL notifier — still updates state
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);
    EXPECT_TRUE(state->estop_active);
    EXPECT_EQ(notifier_called, 0);

    // Controller assign with NULL decision and NULL notifier — default accept
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(ProtocolTrainHandler, null_reply_callbacks_no_crash)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Query Speeds Reply with NULL callback
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 8;

    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

    // Controller config reply with NULL callbacks
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

    // Listener config reply with NULL callbacks
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

    // Management reply with NULL callbacks
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

}


// ============================================================================
// Section 10: Edge Cases
// ============================================================================

TEST(ProtocolTrainHandler, null_statemachine_info)
{

    _reset_tracking();
    _global_initialize();

    ProtocolTrainHandler_handle_train_command(NULL);
    ProtocolTrainHandler_handle_train_reply(NULL);

    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_incoming_msg)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    sm.openlcb_node = node;
    sm.incoming_msg_info.msg_ptr = NULL;
    sm.outgoing_msg_info.msg_ptr = outgoing;
    sm.outgoing_msg_info.valid = false;

    ProtocolTrainHandler_handle_train_command(&sm);
    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, command_no_train_state)
{

    _reset_tracking();
    _global_initialize();

    // Node without train state
    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set speed on node with no train state — should not crash
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Notifier still fires (the handler checks state before update but fires notifier regardless)
    EXPECT_EQ(notifier_called, 1);
    EXPECT_EQ(last_speed_float16, 0x3C00);

}

TEST(ProtocolTrainHandler, query_speeds_no_train_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply built with defaults (all zeros / NaN)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), 0x0000);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0x00);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), FLOAT16_NAN);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 6), FLOAT16_NAN);

}


// ============================================================================
// Section 11: Global Emergency Events (event-based estop/eoff)
// ============================================================================

TEST(ProtocolTrainHandler, global_emergency_stop_sets_flag) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    EXPECT_FALSE(node->train_state->global_estop_active);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);

    EXPECT_TRUE(node->train_state->global_estop_active);

}

TEST(ProtocolTrainHandler, clear_global_emergency_stop) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    node->train_state->global_estop_active = true;

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);

    EXPECT_FALSE(node->train_state->global_estop_active);

}

TEST(ProtocolTrainHandler, global_emergency_off_sets_flag) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    EXPECT_FALSE(node->train_state->global_eoff_active);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);

    EXPECT_TRUE(node->train_state->global_eoff_active);

}

TEST(ProtocolTrainHandler, clear_global_emergency_off) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    node->train_state->global_eoff_active = true;

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);

    EXPECT_FALSE(node->train_state->global_eoff_active);

}

TEST(ProtocolTrainHandler, global_emergency_does_not_change_set_speed) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    node->train_state->set_speed = 0x3C00;  // 1.0 float16

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);

    // Set speed must NOT be changed by global emergency (per spec)
    EXPECT_EQ(node->train_state->set_speed, 0x3C00);
    EXPECT_TRUE(node->train_state->global_estop_active);

}

TEST(ProtocolTrainHandler, global_emergency_off_does_not_change_functions) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    // Set some functions to non-zero values
    node->train_state->functions[0] = 1;    // F0 (headlight)
    node->train_state->functions[1] = 1;    // F1
    node->train_state->functions[5] = 0x0A; // F5

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);

    // Per spec: Emergency Off de-energizes outputs but does NOT change
    // the stored function values.  The app layer checks global_eoff_active
    // and de-energizes.  Upon clearing, functions restore to these values.
    EXPECT_TRUE(node->train_state->global_eoff_active);
    EXPECT_EQ(node->train_state->functions[0], 1);
    EXPECT_EQ(node->train_state->functions[1], 1);
    EXPECT_EQ(node->train_state->functions[5], 0x0A);

}

TEST(ProtocolTrainHandler, overlapping_emergency_states_independent) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    // Activate all three emergency states
    node->train_state->estop_active = true;
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);

    EXPECT_TRUE(node->train_state->estop_active);
    EXPECT_TRUE(node->train_state->global_estop_active);
    EXPECT_TRUE(node->train_state->global_eoff_active);

    // Clear global estop — other two remain
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);

    EXPECT_TRUE(node->train_state->estop_active);
    EXPECT_FALSE(node->train_state->global_estop_active);
    EXPECT_TRUE(node->train_state->global_eoff_active);

    // Clear global eoff — point-to-point estop remains
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);

    EXPECT_TRUE(node->train_state->estop_active);
    EXPECT_FALSE(node->train_state->global_estop_active);
    EXPECT_FALSE(node->train_state->global_eoff_active);

}

TEST(ProtocolTrainHandler, query_speeds_status_ignores_global_estop) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    node->train_state->set_speed = 0x3C00;
    node->train_state->global_estop_active = true;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    // Status bit reports only point-to-point estop, not global states
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x00);

}

TEST(ProtocolTrainHandler, query_speeds_status_ignores_global_eoff) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    node->train_state->set_speed = 0x3C00;
    node->train_state->global_eoff_active = true;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    // Status bit reports only point-to-point estop, not global states
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x00);

}

TEST(ProtocolTrainHandler, global_emergency_null_train_state_no_crash) {

    _reset_tracking();
    _global_initialize();

    openlcb_node_t node;
    memset(&node, 0, sizeof(node));
    node.train_state = NULL;

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, &node, &incoming, &outgoing);

    // Should not crash
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);

}

TEST(ProtocolTrainHandler, global_emergency_unknown_event_no_crash) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    // Unknown event ID — should not crash or change state
    ProtocolTrainHandler_handle_emergency_event(&sm, 0x0100000000001234ULL);

    EXPECT_FALSE(node->train_state->global_estop_active);
    EXPECT_FALSE(node->train_state->global_eoff_active);

}

TEST(ProtocolTrainHandler, global_emergency_null_statemachine_no_crash) {

    _reset_tracking();
    _global_initialize();

    // Should not crash
    ProtocolTrainHandler_handle_emergency_event(NULL, EVENT_ID_EMERGENCY_STOP);

}


// ============================================================================
// Section 13: Conformance Test Sequences (TN Section 2.2 - 2.11)
// ============================================================================

// TN 2.2 — Check set and query speeds
//
// Verifies that forward/reverse direction is independent of speed,
// particularly at zero.
//
// 1. Set speed 0.75 reverse
// 2. Query → 0.75 reverse
// 3. Set speed 0 reverse
// 4. Query → 0 reverse
// 5. Set speed 0.75 forward
// 6. Query → 0.75 forward
// 7. Set speed 0 forward
// 8. Query → 0 forward

TEST(ProtocolTrainHandler, conformance_2_2_check_set_and_query_speeds) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    uint16_t speed_0_75_fwd = OpenLcbFloat16_from_float(0.75f);
    uint16_t speed_0_75_rev = speed_0_75_fwd | 0x8000;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Set speed 0.75 reverse
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_75_rev, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_75_rev);

    // Step 2: Query → 0.75 reverse
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_75_rev);

    // Step 3: Set speed 0 reverse
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, FLOAT16_NEGATIVE_ZERO, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, FLOAT16_NEGATIVE_ZERO);

    // Step 4: Query → 0 reverse
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), FLOAT16_NEGATIVE_ZERO);

    // Step 5: Set speed 0.75 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_75_fwd, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_75_fwd);

    // Step 6: Query → 0.75 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_75_fwd);

    // Step 7: Set speed 0 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, FLOAT16_POSITIVE_ZERO, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);

    // Step 8: Query → 0 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), FLOAT16_POSITIVE_ZERO);

}

// TN 2.3 — Check set and query of functions
//
// Tests F0 set to on, query on, set to off, query off.
//
// 1. Set F0 to on
// 2. Query F0 → on
// 3. Set F0 to off
// 4. Query F0 → off

TEST(ProtocolTrainHandler, conformance_2_3_check_set_and_query_functions) {

    _reset_tracking();
    _global_initialize_with_nulls();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Set F0 to on (address 0x000000, value 0x0001)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->functions[0], 0x0001);

    // Step 2: Query F0 → on
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    incoming->payload_count = 4;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x0001);

    // Step 3: Set F0 to off (value 0x0000)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0000, 4);
    incoming->payload_count = 6;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->functions[0], 0x0000);

    // Step 4: Query F0 → off
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    incoming->payload_count = 4;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x0000);

}

// TN 2.4 — Check Emergency Stop (point-to-point cmd 0x02)
//
// 1. Set speed 0.1 reverse
// 2. Query → 0.1 reverse
// 3. Emergency stop command
// 4. Query → 0 reverse  (Set Speed IS changed)
// 5. Set speed 0.1 forward  (clears estop)
// 6. Query → 0.1 forward
// 7. Set speed 0 forward

TEST(ProtocolTrainHandler, conformance_2_4_check_emergency_stop) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    uint16_t speed_0_1_fwd = OpenLcbFloat16_from_float(0.1f);
    uint16_t speed_0_1_rev = speed_0_1_fwd | 0x8000;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Set speed 0.1 reverse
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_1_rev, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_1_rev);

    // Step 2: Query → 0.1 reverse
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_rev);

    // Step 3: Emergency stop command (point-to-point, cmd 0x02)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(state->estop_active);

    // Step 4: Query → 0 reverse  (Set Speed changed to zero, direction preserved)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), FLOAT16_NEGATIVE_ZERO);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x01);

    // Step 5: Set speed 0.1 forward (clears estop)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_1_fwd, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(state->estop_active);
    EXPECT_EQ(state->set_speed, speed_0_1_fwd);

    // Step 6: Query → 0.1 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_fwd);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x00);

    // Step 7: Set speed 0 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, FLOAT16_POSITIVE_ZERO, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);

}

// TN 2.5 — Check Global Emergency Stop
//
// 1. Set speed 0.1 reverse
// 2. Query → 0.1 reverse
// 3. Produce Emergency Stop All event
// 4. Query → 0.1 reverse  (Set Speed NOT changed)
// 5. Set speed 0.1 forward  (accepted even during global estop)
// 6. Query → 0.1 forward
// 7. Produce Clear Emergency Stop event
// 8. Set speed 0 forward

TEST(ProtocolTrainHandler, conformance_2_5_check_global_emergency_stop) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    uint16_t speed_0_1_fwd = OpenLcbFloat16_from_float(0.1f);
    uint16_t speed_0_1_rev = speed_0_1_fwd | 0x8000;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Set speed 0.1 reverse
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_1_rev, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_1_rev);

    // Step 2: Query → 0.1 reverse
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_rev);

    // Step 3: Produce Emergency Stop All event
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);

    EXPECT_TRUE(state->global_estop_active);

    // Step 4: Query → 0.1 reverse  (Set Speed NOT changed by global estop)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_rev);
    // Status bit reports only point-to-point estop, not global states
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x00);

    // Step 5: Set speed 0.1 forward (accepted during global estop)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_1_fwd, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_1_fwd);
    EXPECT_TRUE(state->global_estop_active);

    // Step 6: Query → 0.1 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_fwd);

    // Step 7: Produce Clear Emergency Stop event
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);

    EXPECT_FALSE(state->global_estop_active);

    // Step 8: Set speed 0 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, FLOAT16_POSITIVE_ZERO, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);

}

// TN 2.6 — Check Global Emergency Off
//
// 1. Set speed 0.1 reverse
// 2. Query → 0.1 reverse
// 3. Produce Emergency Off All event
// 4. Query → 0.1 reverse  (Set Speed NOT changed)
// 5. Set speed 0.1 forward  (accepted even during global eoff)
// 6. Query → 0.1 forward
// 7. Produce Clear Emergency Off event
// 8. Set speed 0 forward

TEST(ProtocolTrainHandler, conformance_2_6_check_global_emergency_off) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    uint16_t speed_0_1_fwd = OpenLcbFloat16_from_float(0.1f);
    uint16_t speed_0_1_rev = speed_0_1_fwd | 0x8000;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Set speed 0.1 reverse
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_1_rev, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_1_rev);

    // Step 2: Query → 0.1 reverse
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_rev);

    // Step 3: Produce Emergency Off All event
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);

    EXPECT_TRUE(state->global_eoff_active);

    // Step 4: Query → 0.1 reverse  (Set Speed NOT changed by global eoff)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_rev);
    // Status bit reports only point-to-point estop, not global states
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x00);

    // Step 5: Set speed 0.1 forward (accepted during global eoff)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_1_fwd, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_1_fwd);
    EXPECT_TRUE(state->global_eoff_active);

    // Step 6: Query → 0.1 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_fwd);

    // Step 7: Produce Clear Emergency Off event
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);

    EXPECT_FALSE(state->global_eoff_active);

    // Step 8: Set speed 0 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, FLOAT16_POSITIVE_ZERO, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);

}

// TN 2.8 — Check function to/from memory space connection
//
// Verifies that a write to the 0xF9 memory space at address 0 is
// reflected when querying function 0 via the Train Control protocol.
//
// 1. Set F0 off via Set Function command
// 2. Write byte 0 in 0xF9 space to 1 (simulated by direct state update)
// 3. Query F0 → on
//
// The 0xF9 write handler is tested in protocol_config_mem_write_handler_Test.
// This test verifies the shared state: both paths use state->functions[].

TEST(ProtocolTrainHandler, conformance_2_8_function_memory_space_connection) {

    _reset_tracking();
    _global_initialize_with_nulls();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Set F0 off via Set Function command
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0000, 4);
    incoming->payload_count = 6;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->functions[0], 0x0000);

    // Step 2: Write byte 0 in 0xF9 space to 1.
    // The 0xF9 write handler maps address 0 to functions[0] high byte
    // (big-endian).  Per TN, the 0xF9 space holds one byte per function.
    // A non-zero value means "on".  Simulate the write handler result:
    state->functions[0] = 0x0001;

    // Step 3: Query F0 → on
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    incoming->payload_count = 4;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_NE(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x0000);

}

// TN 2.9 — Check Controller Configuration command and response
//
// 1. Set Speed 0
// 2. Assign Controller → OK (flags = 0)
// 3. Query Controller → checker's Node ID in Active Controller field
// 4. Release Controller
// 5. Query Controller → zero Node ID in Active Controller field
//
// Ends with Release Controller for cleanup.

TEST(ProtocolTrainHandler, conformance_2_9_check_controller_configuration) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Set speed 0
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, FLOAT16_POSITIVE_ZERO, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Step 2: Assign Controller with checker's Node ID
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_SOURCE_ID, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Check for Assign reply with OK (result = 0)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_CONTROLLER_ASSIGN);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Step 4: Query Controller
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_QUERY, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Step 5: Check query reply has checker's Node ID
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_CONTROLLER_QUERY);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 3), TEST_SOURCE_ID);

    // Step 6: Release Controller
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_SOURCE_ID, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Step 7: Controller should now be zero
    EXPECT_EQ(state->controller_node_id, 0);

    // Step 8: Query Controller
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_QUERY, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Step 9: Check query reply has zero Node ID
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 3), (uint64_t) 0);

    // Cleanup: Release Controller (idempotent)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_SOURCE_ID, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

}

// TN 2.10 — Check Train Control Management command and response
//
// 1. Reserve from A → OK
// 2. Reserve again from A (same source) → OK
// 3. Release
// 4. Reserve from A → OK
// 5. Reserve from B (different source, already reserved) → fail
// 6. Release (cleanup)

TEST(ProtocolTrainHandler, conformance_2_10_check_management_reserve_release) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Reserve from source A → OK
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;
    incoming->source_id = TEST_SOURCE_ID;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_MANAGEMENT);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_MGMT_RESERVE);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Step 2: Reserve again from same source A → OK
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Step 3: Release (no response expected)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(state->reserved_node_count, 0);

    // Step 4: Reserve from A → OK
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Step 5: Reserve from different source B → fail
    sm.outgoing_msg_info.valid = false;
    incoming->source_id = 0xAABBCCDDEEFFULL;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_NE(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);
    EXPECT_EQ(state->reserved_node_count, 1);

    // Step 6: Release (cleanup, no response expected)
    sm.outgoing_msg_info.valid = false;
    incoming->source_id = TEST_SOURCE_ID;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(state->reserved_node_count, 0);

}

// TN 2.11 — Check Listener Configuration command and response
//
// Per Train Control Standard Section 6.5:
//   - Attach adds a listener with flags; reply echoes node_id + result 0 (OK)
//   - Detach removes a listener; reply echoes node_id + result 0 (OK)
//   - Query returns total count, the requested index entry (flags + node_id)
//   - Detach of a non-existent listener returns non-zero result
//
// Sequence:
//  1. Query Listeners (index 0) → count=0
//  2. Attach Listener A with REVERSE flag → OK
//  3. Query Listeners (index 0) → count=1, A with REVERSE
//  4. Attach Listener B with LINK_F0 flag → OK
//  5. Query Listeners (index 0) → count=2, A with REVERSE
//  6. Query Listeners (index 1) → count=2, B with LINK_F0
//  7. Detach Listener A → OK
//  8. Query Listeners (index 0) → count=1, B with LINK_F0
//  9. Detach non-existent Listener A again → fail (non-zero)
// 10. Detach Listener B → OK
// 11. Query Listeners (index 0) → count=0

TEST(ProtocolTrainHandler, conformance_2_11_check_listener_configuration) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    node_id_t listener_a = 0x112233445566ULL;
    node_id_t listener_b = 0xAABBCCDDEEFFULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Query Listeners (index 0) → count=0
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // Listener index = 0
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_LISTENER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_LISTENER_QUERY);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0);  // count = 0

    // Step 2: Attach Listener A with REVERSE flag → OK
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_FLAG_REVERSE, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, listener_a, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_LISTENER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_LISTENER_ATTACH);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 2), listener_a);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);  // OK

    // Step 3: Query Listeners (index 0) → count=1, A with REVERSE
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // index 0
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 1);  // count
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0);  // index
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4), TRAIN_LISTENER_FLAG_REVERSE);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 5), listener_a);

    // Step 4: Attach Listener B with LINK_F0 flag → OK
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_FLAG_LINK_F0, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, listener_b, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);  // OK

    // Step 5: Query Listeners (index 0) → count=2, A with REVERSE
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // index 0
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 2);  // count
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0);  // index
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4), TRAIN_LISTENER_FLAG_REVERSE);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 5), listener_a);

    // Step 6: Query Listeners (index 1) → count=2, B with LINK_F0
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 2);  // index 1
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 2);  // count
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 1);  // index
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4), TRAIN_LISTENER_FLAG_LINK_F0);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 5), listener_b);

    // Step 7: Detach Listener A → OK
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, listener_a, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_LISTENER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_LISTENER_DETACH);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 2), listener_a);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);  // OK

    // Step 8: Query Listeners (index 0) → count=1, B with LINK_F0
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // index 0
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 1);  // count
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0);  // index
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4), TRAIN_LISTENER_FLAG_LINK_F0);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 5), listener_b);

    // Step 9: Detach non-existent Listener A again → fail (non-zero)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, listener_a, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_NE(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);  // fail

    // Step 10: Detach Listener B → OK
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, listener_b, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);  // OK

    // Step 11: Query Listeners (index 0) → count=0
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // index 0
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0);  // count = 0

}


// ============================================================================
// Section 14: Listener Forwarding Tests
// ============================================================================

#define TEST_LISTENER_A 0x112233445566ULL
#define TEST_LISTENER_B 0xAABBCCDDEEFFULL
#define TEST_LISTENER_C 0x010203040506ULL  // Same as TEST_SOURCE_ID for source-skip tests

    /** @brief Helper: attach a listener to a train node via the protocol handler. */
static void _attach_test_listener(openlcb_statemachine_info_t *sm, openlcb_msg_t *incoming,
        node_id_t listener_id, uint8_t flags) {

    sm->outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, flags, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, listener_id, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(sm);

}


TEST(ProtocolTrainHandler, forwarding_speed_to_single_listener)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener A (no special flags)
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, 0);

    // Send SET_SPEED command
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // State should be updated locally
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    EXPECT_EQ(state->set_speed, 0x3C00);

    // First forwarded message should be valid
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_TRUE(sm.outgoing_msg_info.enumerate);
    EXPECT_TRUE(sm.incoming_msg_info.enumerate);

    // Outgoing should be addressed to listener A
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_alias, 0);

    // Payload should have P bit set
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0),
            TRAIN_SET_SPEED_DIRECTION | TRAIN_INSTRUCTION_P_BIT);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), 0x3C00);

    // Source should be the train node
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->source_id, node->id);

    // Re-dispatch — should complete enumeration (no more listeners)
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.incoming_msg_info.enumerate);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(ProtocolTrainHandler, forwarding_speed_to_two_listeners)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach two listeners
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, 0);
    _attach_test_listener(&sm, incoming, TEST_LISTENER_B, 0);

    // Send SET_SPEED command
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x4000, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // First forwarded message: listener A
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0),
            TRAIN_SET_SPEED_DIRECTION | TRAIN_INSTRUCTION_P_BIT);

    // Re-dispatch for listener B
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_B);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0),
            TRAIN_SET_SPEED_DIRECTION | TRAIN_INSTRUCTION_P_BIT);

    // Final re-dispatch — done
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.incoming_msg_info.enumerate);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(ProtocolTrainHandler, forwarding_speed_reverse_flag_flips_direction)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener with REVERSE flag
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, TRAIN_LISTENER_FLAG_REVERSE);

    // Send forward speed 0x3C00 (positive = forward)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Forwarded speed should have direction flipped (0x3C00 ^ 0x8000 = 0xBC00)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), 0xBC00);

}

TEST(ProtocolTrainHandler, forwarding_function_f0_link_f0_flag)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener A with LINK_F0 flag
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, TRAIN_LISTENER_FLAG_LINK_F0);

    // Send SET_FUNCTION for F0 (address = 0)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);  // addr hi
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // addr mid
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // addr lo = F0
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // F0 should be forwarded because LINK_F0 is set
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0),
            TRAIN_SET_FUNCTION | TRAIN_INSTRUCTION_P_BIT);

}

TEST(ProtocolTrainHandler, forwarding_function_f0_no_link_f0_skipped)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener without LINK_F0 (only LINK_FN)
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, TRAIN_LISTENER_FLAG_LINK_FN);

    // Send SET_FUNCTION for F0 (address = 0)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // F0
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // F0 should NOT be forwarded (no LINK_F0 flag)
    // Enumeration should have started and immediately completed
    EXPECT_FALSE(sm.incoming_msg_info.enumerate);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(ProtocolTrainHandler, forwarding_function_fn_link_fn_flag)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener with LINK_FN flag
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, TRAIN_LISTENER_FLAG_LINK_FN);

    // Send SET_FUNCTION for F5 (address = 5)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x05, 3);  // F5
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // F5 should be forwarded because LINK_FN is set
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);

}

TEST(ProtocolTrainHandler, forwarding_function_fn_no_link_fn_skipped)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener with LINK_F0 only (not LINK_FN)
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, TRAIN_LISTENER_FLAG_LINK_F0);

    // Send SET_FUNCTION for F5 (address = 5)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x05, 3);  // F5
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // F5 should NOT be forwarded (no LINK_FN flag)
    EXPECT_FALSE(sm.incoming_msg_info.enumerate);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(ProtocolTrainHandler, forwarding_estop_to_listeners)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, 0);

    // Send EMERGENCY_STOP
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Local estop should be active
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    EXPECT_TRUE(state->estop_active);

    // Forwarded estop to listener
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0),
            TRAIN_EMERGENCY_STOP | TRAIN_INSTRUCTION_P_BIT);

}

TEST(ProtocolTrainHandler, forwarding_source_skip)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener C whose node_id matches TEST_SOURCE_ID
    _attach_test_listener(&sm, incoming, TEST_LISTENER_C, 0);
    // Attach listener A (different)
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, 0);

    // Send SET_SPEED from TEST_SOURCE_ID
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;
    incoming->source_id = TEST_SOURCE_ID;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Should skip listener C (matches source) and forward to listener A
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);

    // Complete enumeration
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.incoming_msg_info.enumerate);

}

TEST(ProtocolTrainHandler, forwarding_no_listeners_no_enumeration)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // No listeners attached — send SET_SPEED
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // No forwarding — enumerate should remain false
    EXPECT_FALSE(sm.incoming_msg_info.enumerate);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

    // Speed should still be updated locally
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    EXPECT_EQ(state->set_speed, 0x3C00);

}

TEST(ProtocolTrainHandler, forwarding_p_bit_is_forwarded)
{
    // P=1 messages (from chained consist) MUST be forwarded per
    // TrainControlS §6.5.  Loop prevention uses source-skip, not P bit.

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach a listener (different from source — no source-skip)
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, 0);

    // Send a forwarded command (P bit already set) — simulates receiving
    // a consist-forwarded speed command from another consist member
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming,
            TRAIN_SET_SPEED_DIRECTION | TRAIN_INSTRUCTION_P_BIT, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // State should be updated locally (P bit masked, dispatched to _handle_set_speed)
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    EXPECT_EQ(state->set_speed, 0x3C00);

    // P=1 MUST trigger forwarding to listeners (TrainControlS §6.5)
    EXPECT_TRUE(sm.incoming_msg_info.enumerate);
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    // Forwarded message should be addressed to the listener
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);

    // Forwarded payload should have P bit set
    uint8_t forwarded_instruction =
            OpenLcbUtilities_extract_byte_from_openlcb_payload(
                    sm.outgoing_msg_info.msg_ptr, 0);
    EXPECT_TRUE(forwarded_instruction & TRAIN_INSTRUCTION_P_BIT);

}

TEST(ProtocolTrainHandler, forwarding_query_not_forwarded)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach a listener
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, 0);

    // Send QUERY_SPEEDS (not a forwardable command)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Query reply should be generated, but NO forwarding enumeration
    EXPECT_TRUE(sm.outgoing_msg_info.valid);  // Query reply
    EXPECT_FALSE(sm.incoming_msg_info.enumerate);

}

TEST(ProtocolTrainHandler, forwarding_mixed_flags_selective)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Listener A: LINK_F0 only
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, TRAIN_LISTENER_FLAG_LINK_F0);
    // Listener B: LINK_FN only
    _attach_test_listener(&sm, incoming, TEST_LISTENER_B, TRAIN_LISTENER_FLAG_LINK_FN);

    // Send SET_FUNCTION F0 (address 0) — should only go to A (has LINK_F0)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // F0
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Should forward to listener A (LINK_F0)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);

    // Re-dispatch — should skip B (no LINK_F0) and finish
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.incoming_msg_info.enumerate);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

    // Now send SET_FUNCTION F5 (address 5) — should only go to B (has LINK_FN)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x05, 3);  // F5
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Should skip A (no LINK_FN) and forward to B
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_B);

    // Re-dispatch — done
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.incoming_msg_info.enumerate);

}

TEST(ProtocolTrainHandler, forwarding_speed_always_forwarded_regardless_of_flags)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener with no link flags (only REVERSE or nothing)
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, 0);

    // Send SET_SPEED — should still be forwarded (speed is always forwarded)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->dest_id, TEST_LISTENER_A);

}

TEST(ProtocolTrainHandler, forwarding_reverse_flag_no_effect_on_estop)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener with REVERSE flag
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, TRAIN_LISTENER_FLAG_REVERSE);

    // Send EMERGENCY_STOP
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Estop is forwarded, REVERSE flag does not affect it (only 1 byte payload)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0),
            TRAIN_EMERGENCY_STOP | TRAIN_INSTRUCTION_P_BIT);
    EXPECT_EQ(outgoing->payload_count, 1);

}

TEST(ProtocolTrainHandler, forwarding_all_listeners_source_skipped)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach only the source node as a listener
    _attach_test_listener(&sm, incoming, TEST_LISTENER_C, 0);  // TEST_LISTENER_C == TEST_SOURCE_ID

    // Send SET_SPEED from TEST_SOURCE_ID
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;
    incoming->source_id = TEST_SOURCE_ID;

    ProtocolTrainHandler_handle_train_command(&sm);

    // All listeners skipped (source match) — enumeration done immediately
    EXPECT_FALSE(sm.incoming_msg_info.enumerate);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}


// ============================================================================
// Section 15: Missing Branch Coverage Tests
// ============================================================================

// ---------------------------------------------------------------------------
// _attach_listener: NULL state (line 73 TRUE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_attach_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with result=0xFF (state is NULL, attach fails)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0xFF);

    // on_listener_changed NOT fired on failure
    EXPECT_NE(notifier_called, 6);

}

// ---------------------------------------------------------------------------
// _attach_listener: zero node_id (line 73 — node_id == 0 path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_attach_zero_node_id)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach with node_id = 0
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, (uint64_t) 0, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Attach should fail with result 0xFF
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0xFF);

    // No listener added
    EXPECT_EQ(state->listener_count, 0);

}

// ---------------------------------------------------------------------------
// _attach_listener: update existing listener flags (line 82 TRUE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_attach_update_existing_flags)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener first time
    _attach_test_listener(&sm, incoming, TEST_LISTENER_NODE_ID, 0x00);

    EXPECT_EQ(state->listener_count, 1);
    EXPECT_EQ(state->listeners[0].flags, 0x00);

    // Attach same listener with different flags — should update, not add
    _attach_test_listener(&sm, incoming, TEST_LISTENER_NODE_ID, TRAIN_LISTENER_FLAG_REVERSE);

    EXPECT_EQ(state->listener_count, 1);
    EXPECT_EQ(state->listeners[0].flags, TRAIN_LISTENER_FLAG_REVERSE);

}

// ---------------------------------------------------------------------------
// _attach_listener: capacity full (line 92 TRUE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_attach_capacity_full)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Fill all listener slots (USER_DEFINED_MAX_LISTENERS_PER_TRAIN = 6)
    for (uint8_t i = 0; i < USER_DEFINED_MAX_LISTENERS_PER_TRAIN; i++) {

        _attach_test_listener(&sm, incoming, 0x010000000001ULL + i, 0x00);

    }

    EXPECT_EQ(state->listener_count, USER_DEFINED_MAX_LISTENERS_PER_TRAIN);

    // Try attaching one more — should fail
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, 0xFFFFFFFFFFFFULL, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with result=0xFF (full)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0xFF);

    // Count did not increase
    EXPECT_EQ(state->listener_count, USER_DEFINED_MAX_LISTENERS_PER_TRAIN);

}

// ---------------------------------------------------------------------------
// _detach_listener: NULL state (line 109 TRUE path — !state)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_detach_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with result=0xFF
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0xFF);

}

// ---------------------------------------------------------------------------
// _detach_listener: zero node_id (line 109 — node_id == 0 path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_detach_zero_node_id)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, (uint64_t) 0, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with result=0xFF (detach of zero node_id fails)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0xFF);

}

// ---------------------------------------------------------------------------
// _forward_to_next_listener: NULL train_state (line 439 TRUE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, forward_to_next_listener_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *saved_state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach a listener so forwarding begins
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, 0);

    // Send SET_SPEED command to start forwarding
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // First forward succeeds — enumerate should be true
    EXPECT_TRUE(sm.incoming_msg_info.enumerate);

    // Now NULL the train_state before re-dispatch
    node->train_state = NULL;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Should abort gracefully
    EXPECT_FALSE(sm.incoming_msg_info.enumerate);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

    // Restore for cleanup
    node->train_state = saved_state;

}

// ---------------------------------------------------------------------------
// _handle_set_speed: NULL on_speed_changed (line 514 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, set_speed_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x4200, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // State updated even though callback is NULL
    EXPECT_EQ(state->set_speed, 0x4200);
    // Notifier NOT fired
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _handle_set_function: fn_address >= USER_DEFINED_MAX_TRAIN_FUNCTIONS
//   (line 531 FALSE path — state && fn_address < max is FALSE)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, set_function_out_of_bounds_state_not_stored)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Function F29 (== USER_DEFINED_MAX_TRAIN_FUNCTIONS, out of bounds)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 29, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0xABCD, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Notifier fires with address and value
    EXPECT_EQ(notifier_called, 2);
    EXPECT_EQ(last_fn_address, (uint32_t) 29);
    EXPECT_EQ(last_fn_value, 0xABCD);

}

// ---------------------------------------------------------------------------
// _handle_set_function: NULL on_function_changed (line 537 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, set_function_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x03, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // State updated
    EXPECT_EQ(state->functions[3], 0x0001);
    // Notifier NOT fired
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _handle_emergency_stop: NULL state (line 551 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, emergency_stop_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Notifier still fires even with NULL state
    EXPECT_EQ(notifier_called, 3);
    EXPECT_EQ(last_emergency_type, TRAIN_EMERGENCY_TYPE_ESTOP);

}

// ---------------------------------------------------------------------------
// _handle_emergency_stop: NULL on_emergency_entered (line 561 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, emergency_stop_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // State updated
    EXPECT_TRUE(state->estop_active);
    // Notifier NOT fired
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _handle_query_function: fn_address >= max (line 601 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, query_function_out_of_range)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Query function address 30 (>= USER_DEFINED_MAX_TRAIN_FUNCTIONS which is 29)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 30, 3);
    incoming->payload_count = 4;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply built with value 0 (out-of-range returns default)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x0000);

}

// ---------------------------------------------------------------------------
// _handle_controller_config ASSIGN: NULL state (line 627 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, controller_assign_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply built (accepted = true, since state is NULL, accepted stays default true)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // on_controller_assigned still fires
    EXPECT_EQ(notifier_called, 4);

}

// ---------------------------------------------------------------------------
// _handle_controller_config ASSIGN: different controller, NULL assign_request
//   callback (line 637 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, controller_assign_different_null_decision_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // NULL assign_request callback => accepted stays true by default
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID_2);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // on_controller_assigned is NULL — notifier not called
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _handle_controller_config ASSIGN: accepted but NULL on_controller_assigned
//   (line 655 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, controller_assign_accepted_null_assigned_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Accepted
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // on_controller_assigned is NULL — notifier not called
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _handle_controller_config RELEASE: wrong releasing_id (line 669 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, controller_release_wrong_id)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Release with wrong node ID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Controller NOT cleared
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);
    // on_controller_released NOT fired
    EXPECT_NE(notifier_called, 5);

}

// ---------------------------------------------------------------------------
// _handle_controller_config RELEASE: NULL on_controller_released (line 673 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, controller_release_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Controller cleared
    EXPECT_EQ(state->controller_node_id, (uint64_t) 0);
    // Notifier NOT fired (NULL callback)
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _handle_controller_config QUERY: NULL state (line 690 — state is NULL)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, controller_query_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_QUERY, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with flags=0, node_id=0
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 3), (uint64_t) 0);

}

// ---------------------------------------------------------------------------
// _handle_controller_config CHANGED: NULL on_controller_changed_request
//   (line 713 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, controller_changed_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // NULL callback => accepted stays true => result=0x00
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

}

// ---------------------------------------------------------------------------
// _handle_listener_config ATTACH: NULL on_listener_changed (line 767 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_attach_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Listener added
    EXPECT_EQ(state->listener_count, 1);

    // Reply success
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);

    // on_listener_changed is NULL — notifier not called
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _handle_listener_config DETACH: NULL on_listener_changed (line 803 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_detach_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    // Pre-populate a listener
    state->listeners[0].node_id = TEST_LISTENER_NODE_ID;
    state->listeners[0].flags = 0x00;
    state->listener_count = 1;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Listener removed
    EXPECT_EQ(state->listener_count, 0);

    // Reply success
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);

    // on_listener_changed is NULL — notifier not called
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _handle_listener_config QUERY: NULL state (line 824 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_query_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with count=0 (state is NULL)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0);

}

// ---------------------------------------------------------------------------
// _handle_listener_config QUERY: index out of range (line 830 TRUE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, listener_query_index_out_of_range)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    // Add one listener
    state->listeners[0].node_id = TEST_LISTENER_NODE_ID;
    state->listeners[0].flags = TRAIN_LISTENER_FLAG_LINK_F0;
    state->listener_count = 1;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Query index 5 (only 1 listener exists, so index >= count)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 5, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply with count=1, index=5, flags=0, node_id=0
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 1);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 5);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4), 0);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing, 5), (uint64_t) 0);

}

// ---------------------------------------------------------------------------
// _handle_management RESERVE: NULL state (line 880 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, management_reserve_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Reply built with result=0 (state is NULL, result stays 0)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

}

// ---------------------------------------------------------------------------
// _handle_management RELEASE: NULL state (line 902 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, management_release_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // No crash — no reply for release
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ---------------------------------------------------------------------------
// _handle_management NOOP: NULL state (line 914 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, management_noop_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_NOOP, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // No crash
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for query_speeds (line 940 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_query_speeds_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3E00, 4);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3A00, 6);
    incoming->payload_count = 8;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for query_function (line 958 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_query_function_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x05, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for controller_assign_reply (line 983 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_controller_assign_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for controller_query_reply (line 994 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_controller_query_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for controller_changed_notify_reply (line 1006 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_controller_changed_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: unknown controller config sub-command (line 1017 default)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_controller_config_unknown_sub)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for listener_attach_reply (line 1035 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_listener_attach_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 8);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for listener_detach_reply (line 1047 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_listener_detach_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 8);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for listener_query_reply (line 1059 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_listener_query_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 3, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 1, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 4);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 5);
    incoming->payload_count = 11;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: unknown listener config sub-command (line 1073 default)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_listener_config_unknown_sub)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for reserve_reply (line 1091 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_management_reserve_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: NULL callback for heartbeat_request (line 1102 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_management_heartbeat_null_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_NOOP, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x0A, 4);
    incoming->payload_count = 5;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Reply dispatch: unknown management sub-command (line 1117 default)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, reply_management_unknown_sub)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Emergency event: NULL on_emergency_entered for ESTOP (line 1351 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, emergency_event_estop_null_entered_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);

    // Flag set
    EXPECT_TRUE(node->train_state->global_estop_active);
    // Notifier NOT fired (NULL callback)
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Emergency event: NULL on_emergency_exited for CLEAR_ESTOP (line 1363 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, emergency_event_clear_estop_null_exited_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    node->train_state->global_estop_active = true;

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);

    // Flag cleared
    EXPECT_FALSE(node->train_state->global_estop_active);
    // Notifier NOT fired (NULL callback)
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Emergency event: NULL on_emergency_entered for EOFF (line 1375 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, emergency_event_eoff_null_entered_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);

    // Flag set
    EXPECT_TRUE(node->train_state->global_eoff_active);
    // Notifier NOT fired (NULL callback)
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// Emergency event: NULL on_emergency_exited for CLEAR_EOFF (line 1387 FALSE path)
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, emergency_event_clear_eoff_null_exited_callback)
{

    _reset_tracking();
    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    node->train_state->global_eoff_active = true;

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming, outgoing;
    memset(&incoming, 0, sizeof(incoming));
    memset(&outgoing, 0, sizeof(outgoing));
    _setup_statemachine(&sm, node, &incoming, &outgoing);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);

    // Flag cleared
    EXPECT_FALSE(node->train_state->global_eoff_active);
    // Notifier NOT fired (NULL callback)
    EXPECT_EQ(notifier_called, 0);

}

// ---------------------------------------------------------------------------
// _load_forwarded_command: REVERSE flag with SET_SPEED (line 407 TRUE path fully)
// This tests that when all three sub-conditions are true, the direction bit flips.
// The existing test only verifies the direction is flipped; this ensures
// payload_count >= 3 condition is exercised along with instruction match.
// ---------------------------------------------------------------------------

TEST(ProtocolTrainHandler, forwarding_reverse_flag_flips_direction_forward_to_reverse)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Attach listener with REVERSE flag
    _attach_test_listener(&sm, incoming, TEST_LISTENER_A, TRAIN_LISTENER_FLAG_REVERSE);

    // Send forward speed 0x3C00 (forward = bit 15 clear)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Forwarded message should have direction bit flipped (0x3C00 ^ 0x8000 = 0xBC00)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    uint16_t fwd_speed = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1);
    EXPECT_EQ(fwd_speed, 0xBC00);

}

// ---------------------------------------------------------------------------
// _handle_listener_config ATTACH: failed attach returns 0xFF (line 753 TRUE path)
// This path is reached when _attach_listener returns false (state non-NULL but
// attach fails, e.g., capacity full or zero node_id).
// The capacity_full test above covers this via attach failing after max.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// _handle_listener_config DETACH: state non-NULL but detach fails
//   (line 785-793 — _detach_listener returns false)
// Already covered by command_listener_detach_not_found test above.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// _get_listener_count NULL state and _get_listener_by_index NULL/out-of-range
// These are covered indirectly through the listener_query_null_state and
// listener_query_index_out_of_range tests above, which force the paths
// through the _handle_listener_config QUERY code that calls these functions.
// ---------------------------------------------------------------------------


// ============================================================================
// Section 16: NULL _interface pointer tests
//
// Tests where ProtocolTrainHandler_initialize(NULL) is called so _interface
// itself is NULL, covering the `_interface &&` sub-branch FALSE path in all
// compound callback checks.
// ============================================================================

static void _global_initialize_null_interface(void) {

    ProtocolTrainHandler_initialize(NULL);
    OpenLcbApplicationTrain_initialize(&_interface_app_train);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

}

TEST(ProtocolTrainHandler, null_interface_set_speed)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x4200, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, 0x4200);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_set_function)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->functions[1], 0x0001);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_set_function_out_of_bounds)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 29, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0xABCD, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_emergency_stop)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(state->estop_active);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_query_function_null_state)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    incoming->payload_count = 4;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(ProtocolTrainHandler, null_interface_controller_assign_different)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // No decision callback → default accept
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID_2);
    // No on_controller_assigned callback → notifier_called stays 0
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_controller_release)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->controller_node_id, (uint64_t) 0);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_controller_changed)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Default accept — result=0x00
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

}

TEST(ProtocolTrainHandler, null_interface_listener_attach)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_listener_detach)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // First attach a listener
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Now detach
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_LISTENER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 8), 0x00);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_reply_query_speeds)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 8;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_reply_query_function)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_reply_controller_config)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // ASSIGN reply
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

    // QUERY reply
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_QUERY, 1);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

    // CHANGED reply
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_reply_listener_config)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // ATTACH reply
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_ATTACH, 1);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

    // DETACH reply
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_DETACH, 1);
    incoming->payload_count = 9;
    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

    // QUERY reply
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    incoming->payload_count = 11;
    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_reply_management)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // RESERVE reply
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

    // NOOP/heartbeat reply
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_NOOP, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x0A, 4);
    incoming->payload_count = 5;
    ProtocolTrainHandler_handle_train_reply(&sm);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, null_interface_emergency_events)
{

    _reset_tracking();
    _global_initialize_null_interface();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    openlcb_statemachine_info_t sm;
    openlcb_msg_t incoming_msg, outgoing_msg;
    memset(&sm, 0, sizeof(sm));
    memset(&incoming_msg, 0, sizeof(incoming_msg));
    memset(&outgoing_msg, 0, sizeof(outgoing_msg));
    sm.openlcb_node = node;
    sm.incoming_msg_info.msg_ptr = &incoming_msg;
    sm.outgoing_msg_info.msg_ptr = &outgoing_msg;

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);
    EXPECT_TRUE(state->global_estop_active);
    EXPECT_EQ(notifier_called, 0);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);
    EXPECT_FALSE(state->global_estop_active);
    EXPECT_EQ(notifier_called, 0);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);
    EXPECT_TRUE(state->global_eoff_active);
    EXPECT_EQ(notifier_called, 0);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);
    EXPECT_FALSE(state->global_eoff_active);
    EXPECT_EQ(notifier_called, 0);

}

TEST(ProtocolTrainHandler, forwarding_reverse_short_payload_no_flip)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    _attach_test_listener(&sm, incoming, TEST_LISTENER_NODE_ID, TRAIN_LISTENER_FLAG_REVERSE);

    // Send SET_SPEED with only 2 bytes payload (missing speed word)
    sm.outgoing_msg_info.valid = false;
    sm.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x42, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Forwarded but direction NOT flipped (payload_count < 3)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), 0x42);

}

TEST(ProtocolTrainHandler, set_function_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Callback fires even with NULL state
    EXPECT_EQ(notifier_called, 2);
    EXPECT_EQ(last_fn_address, (uint32_t) 1);

}

TEST(ProtocolTrainHandler, controller_release_null_state)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    node->train_state = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // flags (reserved)
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // No crash, no callback
    EXPECT_EQ(notifier_called, 0);

}


// ============================================================================
// Section 13: Heartbeat counter behavior (TrainControlS 6.6)
// ============================================================================

TEST(ProtocolTrainHandler, heartbeat_any_command_resets_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 50;  // Partially counted down

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Send Set Function command (not speed, not NOOP)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter should be reset to full (10 * 10 = 100)
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 100);

}

TEST(ProtocolTrainHandler, heartbeat_query_speeds_resets_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 5;
    state->heartbeat_counter_100ms = 20;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Send Query Speeds
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter reset to full (5 * 10 = 50)
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 50);

}

TEST(ProtocolTrainHandler, heartbeat_query_function_resets_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 5;
    state->heartbeat_counter_100ms = 20;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Send Query Function
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    incoming->payload_count = 4;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 50);

}

TEST(ProtocolTrainHandler, heartbeat_noop_resets_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 10;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Send NOOP
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_NOOP, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter reset to full (3 * 10 = 30)
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 30);

}

TEST(ProtocolTrainHandler, heartbeat_no_reset_when_counter_zero)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 5;
    state->heartbeat_counter_100ms = 0;  // Heartbeat inactive

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Send Set Function — should NOT restart counter from 0
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0001, 4);
    incoming->payload_count = 6;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter stays at 0 — only Set Speed non-zero starts it
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);

}

TEST(ProtocolTrainHandler, heartbeat_no_reset_when_disabled)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 0;  // Heartbeat disabled
    state->heartbeat_counter_100ms = 0;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter untouched — heartbeat disabled
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);

}

TEST(ProtocolTrainHandler, heartbeat_set_speed_zero_stops_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 80;  // Active

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Speed to zero
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x0000, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter stopped — no heartbeat when speed is zero
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);

}

TEST(ProtocolTrainHandler, heartbeat_set_speed_nonzero_restarts_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 0;  // Inactive (speed was zero)

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Set Speed to non-zero (1.0 forward = 0x3C00)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x3C00, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter started
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 100);

}

TEST(ProtocolTrainHandler, heartbeat_estop_stops_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 80;
    state->set_speed = 0x3C00;  // Non-zero

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Send Emergency Stop
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_EMERGENCY_STOP, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter stopped
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);
    EXPECT_TRUE(state->estop_active);

}

TEST(ProtocolTrainHandler, heartbeat_controller_release_stops_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 80;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Release controller — source must match controller
    incoming->source_id = TEST_CONTROLLER_NODE_ID;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Counter stopped — no controller means no heartbeat
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);
    EXPECT_EQ(state->controller_node_id, (uint64_t) 0);

}

TEST(ProtocolTrainHandler, heartbeat_controller_release_wrong_id_keeps_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 80;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Release with wrong ID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 3);
    incoming->payload_count = 9;

    ProtocolTrainHandler_handle_train_command(&sm);

    // General dispatch reset still happens (counter was > 0)
    // but controller is NOT released since IDs don't match
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);
    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 100);

}
