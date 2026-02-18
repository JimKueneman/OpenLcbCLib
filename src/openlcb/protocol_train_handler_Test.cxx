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

    const interface_protocol_train_handler_t *iface = ProtocolTrainHandler_get_interface();

    EXPECT_NE(iface, nullptr);
    EXPECT_EQ(iface->on_speed_changed, &_mock_on_speed_changed);
    EXPECT_EQ(iface->on_emergency_stopped, &_mock_on_emergency_stopped);
    EXPECT_EQ(iface->on_reserve_reply, &_mock_on_reserve_reply);

}

TEST(ProtocolTrainHandler, initialize_with_nulls)
{

    _global_initialize_with_nulls();

    const interface_protocol_train_handler_t *iface = ProtocolTrainHandler_get_interface();

    EXPECT_NE(iface, nullptr);
    EXPECT_EQ(iface->on_speed_changed, nullptr);
    EXPECT_EQ(iface->on_emergency_stopped, nullptr);

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
    OpenLcbApplicationTrain_attach_listener(state, TEST_LISTENER_NODE_ID, 0x00);
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
    OpenLcbApplicationTrain_attach_listener(state, TEST_LISTENER_NODE_ID, TRAIN_LISTENER_FLAG_LINK_F0);
    OpenLcbApplicationTrain_attach_listener(state, 0xAABBCCDDEEFFULL, 0x00);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_LISTENER_QUERY, 1);
    incoming->payload_count = 2;

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
    incoming->payload_count = 2;

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

TEST(ProtocolTrainHandler, command_management_reserve_multiple)
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

    ProtocolTrainHandler_handle_train_command(&sm);
    sm.outgoing_msg_info.valid = false;
    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 2);

}

TEST(ProtocolTrainHandler, command_management_release)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    train_state_t *state = OpenLcbApplicationTrain_get_state(node);
    state->reserved_node_count = 2;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, TRAIN_MGMT_RELEASE, 1);
    incoming->payload_count = 2;

    ProtocolTrainHandler_handle_train_command(&sm);

    EXPECT_EQ(state->reserved_node_count, 1);

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
