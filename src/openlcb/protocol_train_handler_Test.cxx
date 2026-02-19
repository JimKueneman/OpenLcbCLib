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
static uint8_t decision_assign_result = 0;
static uint8_t decision_changed_result = 0;
static uint16_t decision_fn_value = 0;


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
    decision_assign_result = 0;
    decision_changed_result = 0;
    decision_fn_value = 0;

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

static void _mock_on_emergency_stopped(openlcb_node_t *openlcb_node) {

    notifier_called = 3;
    last_notified_node = openlcb_node;

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

static uint8_t _mock_on_controller_assign_request(openlcb_node_t *openlcb_node,
        uint64_t current_controller, uint64_t requesting_controller) {

    last_notified_node = openlcb_node;
    return decision_assign_result;

}

static uint8_t _mock_on_controller_changed_request(openlcb_node_t *openlcb_node,
        uint64_t new_controller) {

    last_notified_node = openlcb_node;
    return decision_changed_result;

}

static uint16_t _mock_on_query_function_request(openlcb_node_t *openlcb_node,
        uint32_t fn_address) {

    last_notified_node = openlcb_node;
    last_fn_address = fn_address;
    return decision_fn_value;

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
        uint8_t result) {

    notifier_called = 103;
    last_result = result;

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
    .on_emergency_stopped = &_mock_on_emergency_stopped,
    .on_controller_assigned = &_mock_on_controller_assigned,
    .on_controller_released = &_mock_on_controller_released,
    .on_listener_changed = &_mock_on_listener_changed,
    .on_heartbeat_timeout = &_mock_on_heartbeat_timeout,

    // Train-node side: decision callbacks
    .on_controller_assign_request = &_mock_on_controller_assign_request,
    .on_controller_changed_request = &_mock_on_controller_changed_request,
    .on_query_function_request = &_mock_on_query_function_request,

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
    .on_emergency_stopped = NULL,
    .on_controller_assigned = NULL,
    .on_controller_released = NULL,
    .on_listener_changed = NULL,
    .on_heartbeat_timeout = NULL,

    .on_controller_assign_request = NULL,
    .on_controller_changed_request = NULL,
    .on_query_function_request = NULL,

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

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

    .snip.mfg_version = 4,
    .snip.name = "Test Train Node",
    .snip.model = "Test Model",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2,

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,

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
    EXPECT_EQ(state->estop_active, 0);

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
    state->estop_active = 1;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 0x4000, 1);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->estop_active, 0);
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
    EXPECT_EQ(state->estop_active, 1);
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
    EXPECT_EQ(state->estop_active, 1);
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
    state->estop_active = 1;
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
    state->estop_active = 0;
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

TEST(ProtocolTrainHandler, command_query_function_with_callback)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();

    // Configure decision callback to return value 0x0042 for any function
    decision_fn_value = 0x0042;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Query Function: fn address = 3
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x03, 3);
    incoming->payload_count = 4;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Verify reply was built with value from decision callback
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing->mti, MTI_TRAIN_REPLY);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_QUERY_FUNCTION);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3), 0x03);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 4), 0x0042);

    // Verify decision callback received correct fn_address
    EXPECT_EQ(last_fn_address, (uint32_t) 3);

}

TEST(ProtocolTrainHandler, command_query_function_null_callback_returns_zero)
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
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 2);
    incoming->payload_count = 8;

    ProtocolTrainHandler_handle_train_command(&sm);

    // State updated
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);

    // Reply built with result=0 (accept)
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_CONTROLLER_ASSIGN);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

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
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 2);
    incoming->payload_count = 8;

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

    // Decision callback returns 0 (accept)
    decision_assign_result = 0;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 2);
    incoming->payload_count = 8;

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

    // Decision callback returns non-zero (reject)
    decision_assign_result = 0x01;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 2);
    incoming->payload_count = 8;

    ProtocolTrainHandler_handle_train_command(&sm);

    // Original controller preserved
    EXPECT_EQ(state->controller_node_id, TEST_CONTROLLER_NODE_ID);
    // Reply has non-zero result
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x01);
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
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 2);
    incoming->payload_count = 8;

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
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID_2, 2);
    incoming->payload_count = 8;

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

    decision_changed_result = 0;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 2);
    incoming->payload_count = 8;

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

    decision_changed_result = 0x02;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 2);
    incoming->payload_count = 8;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x02);

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

    // Pre-attach a listener
    ProtocolTrainHandler_attach_listener(state, TEST_LISTENER_NODE_ID, 0x00);
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

    // Add two listeners
    ProtocolTrainHandler_attach_listener(state, TEST_LISTENER_NODE_ID, TRAIN_LISTENER_FLAG_LINK_F0);
    ProtocolTrainHandler_attach_listener(state, 0xAABBCCDDEEFFULL, 0x00);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // NodeCount (ignored)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // NodeIndex = 0
    incoming->payload_count = 4;

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
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // NodeCount (ignored)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // NodeIndex = 0
    incoming->payload_count = 4;

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

TEST(ProtocolTrainHandler, command_management_reserve_when_already_reserved_fails)
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

    // First reserve succeeds
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 1);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Second reserve fails (already reserved)
    sm.outgoing_msg_info.valid = false;
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

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 0);

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

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    incoming->payload_count = 3;

    ProtocolTrainHandler_handle_train_reply(&sm);

    EXPECT_EQ(notifier_called, 103);
    EXPECT_EQ(last_result, 0x00);

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
    EXPECT_EQ(state->estop_active, 1);
    EXPECT_EQ(notifier_called, 0);

    // Controller assign with NULL decision and NULL notifier — default accept
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_CONTROLLER_NODE_ID, 2);
    incoming->payload_count = 8;

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
// Section 11: Listener Management (unit tests for ProtocolTrainHandler_*_listener)
// ============================================================================

TEST(ProtocolTrainHandler, listener_attach)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    EXPECT_NE(state, nullptr);
    EXPECT_EQ(state->listener_count, 0);

    bool result = ProtocolTrainHandler_attach_listener(state, TEST_LISTENER_NODE_ID, TRAIN_LISTENER_FLAG_REVERSE);

    EXPECT_TRUE(result);
    EXPECT_EQ(state->listener_count, 1);
    EXPECT_EQ(state->listeners[0].node_id, TEST_LISTENER_NODE_ID);
    EXPECT_EQ(state->listeners[0].flags, TRAIN_LISTENER_FLAG_REVERSE);

}

TEST(ProtocolTrainHandler, listener_attach_multiple)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    uint64_t id1 = 0x010203040501ULL;
    uint64_t id2 = 0x010203040502ULL;
    uint64_t id3 = 0x010203040503ULL;

    EXPECT_TRUE(ProtocolTrainHandler_attach_listener(state, id1, 0x00));
    EXPECT_TRUE(ProtocolTrainHandler_attach_listener(state, id2, TRAIN_LISTENER_FLAG_LINK_F0));
    EXPECT_TRUE(ProtocolTrainHandler_attach_listener(state, id3, TRAIN_LISTENER_FLAG_REVERSE));

    EXPECT_EQ(state->listener_count, 3);
    EXPECT_EQ(state->listeners[0].node_id, id1);
    EXPECT_EQ(state->listeners[1].node_id, id2);
    EXPECT_EQ(state->listeners[2].node_id, id3);

}

TEST(ProtocolTrainHandler, listener_attach_duplicate_updates_flags)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    ProtocolTrainHandler_attach_listener(state, TEST_LISTENER_NODE_ID, 0x00);

    EXPECT_EQ(state->listener_count, 1);
    EXPECT_EQ(state->listeners[0].flags, 0x00);

    // Attach same node again with different flags — should update, not add
    bool result = ProtocolTrainHandler_attach_listener(state, TEST_LISTENER_NODE_ID, TRAIN_LISTENER_FLAG_REVERSE);

    EXPECT_TRUE(result);
    EXPECT_EQ(state->listener_count, 1);
    EXPECT_EQ(state->listeners[0].flags, TRAIN_LISTENER_FLAG_REVERSE);

}

TEST(ProtocolTrainHandler, listener_attach_full)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    // Fill all slots
    for (int i = 0; i < USER_DEFINED_MAX_LISTENERS_PER_TRAIN; i++) {

        bool result = ProtocolTrainHandler_attach_listener(state, 0x010203040500ULL + i, 0x00);
        EXPECT_TRUE(result);

    }

    EXPECT_EQ(state->listener_count, USER_DEFINED_MAX_LISTENERS_PER_TRAIN);

    // One more should fail
    bool result = ProtocolTrainHandler_attach_listener(state, 0xAABBCCDDEEFFULL, 0x00);

    EXPECT_FALSE(result);
    EXPECT_EQ(state->listener_count, USER_DEFINED_MAX_LISTENERS_PER_TRAIN);

}

TEST(ProtocolTrainHandler, listener_attach_null_state)
{

    _reset_tracking();
    _global_initialize();

    bool result = ProtocolTrainHandler_attach_listener(NULL, TEST_LISTENER_NODE_ID, 0x00);

    EXPECT_FALSE(result);

}

TEST(ProtocolTrainHandler, listener_attach_zero_node_id)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    bool result = ProtocolTrainHandler_attach_listener(state, 0, 0x00);

    EXPECT_FALSE(result);
    EXPECT_EQ(state->listener_count, 0);

}

TEST(ProtocolTrainHandler, listener_detach)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    ProtocolTrainHandler_attach_listener(state, TEST_LISTENER_NODE_ID, TRAIN_LISTENER_FLAG_REVERSE);

    EXPECT_EQ(state->listener_count, 1);

    bool result = ProtocolTrainHandler_detach_listener(state, TEST_LISTENER_NODE_ID);

    EXPECT_TRUE(result);
    EXPECT_EQ(state->listener_count, 0);

}

TEST(ProtocolTrainHandler, listener_detach_middle)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    uint64_t id1 = 0x010203040501ULL;
    uint64_t id2 = 0x010203040502ULL;
    uint64_t id3 = 0x010203040503ULL;

    ProtocolTrainHandler_attach_listener(state, id1, 0x00);
    ProtocolTrainHandler_attach_listener(state, id2, 0x02);
    ProtocolTrainHandler_attach_listener(state, id3, 0x04);

    // Detach the middle one
    bool result = ProtocolTrainHandler_detach_listener(state, id2);

    EXPECT_TRUE(result);
    EXPECT_EQ(state->listener_count, 2);
    EXPECT_EQ(state->listeners[0].node_id, id1);
    EXPECT_EQ(state->listeners[0].flags, 0x00);
    EXPECT_EQ(state->listeners[1].node_id, id3);
    EXPECT_EQ(state->listeners[1].flags, 0x04);

}

TEST(ProtocolTrainHandler, listener_detach_not_found)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    ProtocolTrainHandler_attach_listener(state, TEST_LISTENER_NODE_ID, 0x00);

    bool result = ProtocolTrainHandler_detach_listener(state, 0xAABBCCDDEEFFULL);

    EXPECT_FALSE(result);
    EXPECT_EQ(state->listener_count, 1);

}

TEST(ProtocolTrainHandler, listener_detach_null_state)
{

    _reset_tracking();
    _global_initialize();

    bool result = ProtocolTrainHandler_detach_listener(NULL, TEST_LISTENER_NODE_ID);

    EXPECT_FALSE(result);

}

TEST(ProtocolTrainHandler, listener_find)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    ProtocolTrainHandler_attach_listener(state, TEST_LISTENER_NODE_ID, TRAIN_LISTENER_FLAG_LINK_F0);

    train_listener_entry_t *entry = ProtocolTrainHandler_find_listener(state, TEST_LISTENER_NODE_ID);

    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->node_id, TEST_LISTENER_NODE_ID);
    EXPECT_EQ(entry->flags, TRAIN_LISTENER_FLAG_LINK_F0);

}

TEST(ProtocolTrainHandler, listener_find_not_found)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    train_listener_entry_t *entry = ProtocolTrainHandler_find_listener(state, TEST_LISTENER_NODE_ID);

    EXPECT_EQ(entry, nullptr);

}

TEST(ProtocolTrainHandler, listener_find_null_state)
{

    _reset_tracking();
    _global_initialize();

    train_listener_entry_t *entry = ProtocolTrainHandler_find_listener(NULL, TEST_LISTENER_NODE_ID);

    EXPECT_EQ(entry, nullptr);

}

TEST(ProtocolTrainHandler, listener_get_count)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    EXPECT_EQ(ProtocolTrainHandler_get_listener_count(state), 0);

    ProtocolTrainHandler_attach_listener(state, 0x010203040501ULL, 0x00);

    EXPECT_EQ(ProtocolTrainHandler_get_listener_count(state), 1);

    ProtocolTrainHandler_attach_listener(state, 0x010203040502ULL, 0x00);

    EXPECT_EQ(ProtocolTrainHandler_get_listener_count(state), 2);

    EXPECT_EQ(ProtocolTrainHandler_get_listener_count(NULL), 0);

}

TEST(ProtocolTrainHandler, listener_get_by_index)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    uint64_t id1 = 0x010203040501ULL;
    uint64_t id2 = 0x010203040502ULL;

    ProtocolTrainHandler_attach_listener(state, id1, 0x00);
    ProtocolTrainHandler_attach_listener(state, id2, TRAIN_LISTENER_FLAG_REVERSE);

    train_listener_entry_t *entry0 = ProtocolTrainHandler_get_listener_by_index(state, 0);
    train_listener_entry_t *entry1 = ProtocolTrainHandler_get_listener_by_index(state, 1);
    train_listener_entry_t *entry2 = ProtocolTrainHandler_get_listener_by_index(state, 2);

    EXPECT_NE(entry0, nullptr);
    EXPECT_EQ(entry0->node_id, id1);

    EXPECT_NE(entry1, nullptr);
    EXPECT_EQ(entry1->node_id, id2);
    EXPECT_EQ(entry1->flags, TRAIN_LISTENER_FLAG_REVERSE);

    // Out of bounds
    EXPECT_EQ(entry2, nullptr);

    // Null state
    EXPECT_EQ(ProtocolTrainHandler_get_listener_by_index(NULL, 0), nullptr);

}


// ============================================================================
// Section 12: Global Emergency Events (event-based estop/eoff)
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

    EXPECT_EQ(node->train_state->global_estop_active, 0);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);

    EXPECT_EQ(node->train_state->global_estop_active, 1);

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

    node->train_state->global_estop_active = 1;

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);

    EXPECT_EQ(node->train_state->global_estop_active, 0);

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

    EXPECT_EQ(node->train_state->global_eoff_active, 0);

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);

    EXPECT_EQ(node->train_state->global_eoff_active, 1);

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

    node->train_state->global_eoff_active = 1;

    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);

    EXPECT_EQ(node->train_state->global_eoff_active, 0);

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
    EXPECT_EQ(node->train_state->global_estop_active, 1);

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
    EXPECT_EQ(node->train_state->global_eoff_active, 1);
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
    node->train_state->estop_active = 1;
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_STOP);
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_EMERGENCY_OFF);

    EXPECT_EQ(node->train_state->estop_active, 1);
    EXPECT_EQ(node->train_state->global_estop_active, 1);
    EXPECT_EQ(node->train_state->global_eoff_active, 1);

    // Clear global estop — other two remain
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);

    EXPECT_EQ(node->train_state->estop_active, 1);
    EXPECT_EQ(node->train_state->global_estop_active, 0);
    EXPECT_EQ(node->train_state->global_eoff_active, 1);

    // Clear global eoff — point-to-point estop remains
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);

    EXPECT_EQ(node->train_state->estop_active, 1);
    EXPECT_EQ(node->train_state->global_estop_active, 0);
    EXPECT_EQ(node->train_state->global_eoff_active, 0);

}

TEST(ProtocolTrainHandler, query_speeds_status_reflects_global_estop) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    node->train_state->set_speed = 0x3C00;
    node->train_state->global_estop_active = 1;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x01);

}

TEST(ProtocolTrainHandler, query_speeds_status_reflects_global_eoff) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();

    node->train_state->set_speed = 0x3C00;
    node->train_state->global_eoff_active = 1;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x01);

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

    EXPECT_EQ(node->train_state->global_estop_active, 0);
    EXPECT_EQ(node->train_state->global_eoff_active, 0);

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

    EXPECT_EQ(state->estop_active, 1);

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

    EXPECT_EQ(state->estop_active, 0);
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

    EXPECT_EQ(state->global_estop_active, 1);

    // Step 4: Query → 0.1 reverse  (Set Speed NOT changed by global estop)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_rev);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x01);

    // Step 5: Set speed 0.1 forward (accepted during global estop)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_1_fwd, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_1_fwd);
    EXPECT_EQ(state->global_estop_active, 1);

    // Step 6: Query → 0.1 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_fwd);

    // Step 7: Produce Clear Emergency Stop event
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_STOP);

    EXPECT_EQ(state->global_estop_active, 0);

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

    EXPECT_EQ(state->global_eoff_active, 1);

    // Step 4: Query → 0.1 reverse  (Set Speed NOT changed by global eoff)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_rev);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 3) & 0x01, 0x01);

    // Step 5: Set speed 0.1 forward (accepted during global eoff)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, speed_0_1_fwd, 1);
    incoming->payload_count = 3;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->set_speed, speed_0_1_fwd);
    EXPECT_EQ(state->global_eoff_active, 1);

    // Step 6: Query → 0.1 forward
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_QUERY_SPEEDS, 0);
    incoming->payload_count = 1;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 1), speed_0_1_fwd);

    // Step 7: Produce Clear Emergency Off event
    ProtocolTrainHandler_handle_emergency_event(&sm, EVENT_ID_CLEAR_EMERGENCY_OFF);

    EXPECT_EQ(state->global_eoff_active, 0);

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
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_SOURCE_ID, 2);
    incoming->payload_count = 8;
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
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_SOURCE_ID, 2);
    incoming->payload_count = 8;
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
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(incoming, TEST_SOURCE_ID, 2);
    incoming->payload_count = 8;
    ProtocolTrainHandler_handle_train_command(&sm);

}

// TN 2.10 — Check Train Control Management command and response
//
// 1. Reserve → OK
// 2. Release (no response)
// 3. Reserve → OK
// 4. Reserve again (already reserved) → fail
// 5. Release (cleanup)

TEST(ProtocolTrainHandler, conformance_2_10_check_management_reserve_release) {

    _reset_tracking();
    _global_initialize();
    openlcb_node_t *node = _create_train_node();
    train_state_t *state = node->train_state;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Step 1: Reserve → OK
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Step 2: Check OK
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 0), TRAIN_MANAGEMENT);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 1), TRAIN_MGMT_RESERVE);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Step 3: Release (no response expected)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(state->reserved_node_count, 0);

    // Step 4: Reserve → OK
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Step 5: Check OK
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);

    // Step 6: Reserve again (already reserved) → fail
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RESERVE, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    // Step 7: Check fail code
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_NE(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0x00);
    EXPECT_EQ(state->reserved_node_count, 1);

    // Step 8: Release (cleanup, no response expected)
    sm.outgoing_msg_info.valid = false;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(state->reserved_node_count, 0);

}

// TN 2.11 — Check Listener Configuration command and response
//
// Per Train Control Standard Section 6.4:
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
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);  // NodeCount (ignored)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // NodeIndex = 0
    incoming->payload_count = 4;
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
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // index 0
    incoming->payload_count = 4;
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
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // index 0
    incoming->payload_count = 4;
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
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x01, 3);  // index 1
    incoming->payload_count = 4;
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
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);  // index 0
    incoming->payload_count = 4;
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
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x00, 3);
    incoming->payload_count = 4;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 2), 0);  // count = 0

}
