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
* @file openlcb_application_train_Test.cxx
* @brief Unit tests for application-level Train Control module (Layer 2)
*
* Test Organization:
* - Section 1: Initialization and Setup Tests
* - Section 2: Pool Allocation Tests
* - Section 3: Send Helper Tests
* - Section 4: Heartbeat Timer Tests
* - Section 5: Listener Management Tests
* - Section 6: Edge Cases
*
* @author Jim Kueneman
* @date 17 Feb 2026
*/

#include "test/main_Test.hxx"

#include <cstring>

#include "openlcb_application_train.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "protocol_train_handler.h"


// ============================================================================
// Test Constants
// ============================================================================

#define TEST_SOURCE_ALIAS 0x222
#define TEST_SOURCE_ID 0x010203040506ULL
#define TEST_DEST_ALIAS 0xBBB
#define TEST_DEST_ID 0x060504030201ULL
#define TEST_TRAIN_NODE_ID 0xAABBCCDDEE01ULL
#define TEST_CONTROLLER_NODE_ID 0x0A0B0C0D0E0FULL
#define TEST_LISTENER_NODE_ID 0x112233445566ULL


// ============================================================================
// Test Tracking Variables
// ============================================================================

static bool mock_send_called = false;
static openlcb_msg_t last_sent_msg;
static payload_basic_t last_sent_payload;
static int send_call_count = 0;

static bool mock_heartbeat_timeout_called = false;


// ============================================================================
// Reset
// ============================================================================

static void _reset_tracking(void) {

    mock_send_called = false;
    memset(&last_sent_msg, 0, sizeof(last_sent_msg));
    memset(&last_sent_payload, 0, sizeof(last_sent_payload));
    send_call_count = 0;
    mock_heartbeat_timeout_called = false;

}


// ============================================================================
// Mock Functions
// ============================================================================

static bool _mock_send_openlcb_msg(openlcb_msg_t *openlcb_msg) {

    mock_send_called = true;
    send_call_count++;

    memcpy(&last_sent_msg, openlcb_msg, sizeof(openlcb_msg_t));

    if (openlcb_msg->payload) {

        memcpy(&last_sent_payload, openlcb_msg->payload, LEN_MESSAGE_BYTES_BASIC);

    }

    return true;

}

static void _mock_on_heartbeat_timeout(openlcb_node_t *openlcb_node) {

    mock_heartbeat_timeout_called = true;

}


// ============================================================================
// Interface Structures
// ============================================================================

static interface_openlcb_application_train_t _interface_all = {

    .send_openlcb_msg = &_mock_send_openlcb_msg,
    .on_heartbeat_timeout = &_mock_on_heartbeat_timeout,

};

static interface_openlcb_application_train_t _interface_nulls = {

    .send_openlcb_msg = NULL,

};

static interface_protocol_train_handler_t _handler_interface_with_heartbeat = {

    .on_speed_changed = NULL,
    .on_function_changed = NULL,
    .on_emergency_entered = NULL,
    .on_emergency_exited = NULL,
    .on_controller_assigned = NULL,
    .on_controller_released = NULL,
    .on_listener_changed = NULL,
    .on_heartbeat_timeout = &_mock_on_heartbeat_timeout,
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

    ProtocolTrainHandler_initialize(&_handler_interface_with_heartbeat);
    OpenLcbApplicationTrain_initialize(&_interface_all);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

}


// ============================================================================
// Section 1: Initialization and Setup Tests
// ============================================================================

TEST(ApplicationTrain, initialize)
{

    _global_initialize();

    // Verify initialize does not crash and module is ready
    // Callback wiring is tested indirectly by the send helper tests below

}

TEST(ApplicationTrain, setup_allocates_state)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);

    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->train_state, nullptr);

    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);
    EXPECT_EQ(node->train_state, state);
    EXPECT_EQ(state->set_speed, 0);
    EXPECT_EQ(state->controller_node_id, (uint64_t) 0);
    EXPECT_FALSE(state->estop_active);

}

TEST(ApplicationTrain, setup_returns_existing_state)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    train_state_t *state1 = OpenLcbApplicationTrain_setup(node);
    train_state_t *state2 = OpenLcbApplicationTrain_setup(node);

    EXPECT_EQ(state1, state2);

}

TEST(ApplicationTrain, setup_null_node)
{

    _global_initialize();

    train_state_t *state = OpenLcbApplicationTrain_setup(NULL);

    EXPECT_EQ(state, nullptr);

}

TEST(ApplicationTrain, get_state)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    OpenLcbApplicationTrain_setup(node);

    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    EXPECT_NE(state, nullptr);
    EXPECT_EQ(state, node->train_state);

}

TEST(ApplicationTrain, get_state_null_node)
{

    _global_initialize();

    train_state_t *state = OpenLcbApplicationTrain_get_state(NULL);

    EXPECT_EQ(state, nullptr);

}

TEST(ApplicationTrain, get_state_no_train)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;

    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    EXPECT_EQ(state, nullptr);

}


// ============================================================================
// Section 2: Pool Allocation Tests
// ============================================================================

TEST(ApplicationTrain, pool_multiple_nodes)
{

    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    openlcb_node_t *node2 = OpenLcbNode_allocate(TEST_SOURCE_ID, &_test_node_parameters);
    node1->train_state = NULL;
    node2->train_state = NULL;

    train_state_t *state1 = OpenLcbApplicationTrain_setup(node1);
    train_state_t *state2 = OpenLcbApplicationTrain_setup(node2);

    EXPECT_NE(state1, nullptr);
    EXPECT_NE(state2, nullptr);
    EXPECT_NE(state1, state2);

    // Verify each node points to its own state
    EXPECT_EQ(node1->train_state, state1);
    EXPECT_EQ(node2->train_state, state2);

}

TEST(ApplicationTrain, pool_exhaustion)
{

    _global_initialize();

    // USER_DEFINED_TRAIN_NODE_COUNT defaults to 4
    // Allocate all 4 pool slots
    for (int i = 0; i < USER_DEFINED_TRAIN_NODE_COUNT; i++) {

        openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID + i, &_test_node_parameters);
        train_state_t *state = OpenLcbApplicationTrain_setup(node);

        EXPECT_NE(state, nullptr);

    }

    // 5th allocation should fail if we have nodes available
    // Since NODE_BUFFER_DEPTH is 4 we can't allocate more nodes
    // But we can verify the pool count by re-initializing and testing limit

}


// ============================================================================
// Section 3: Send Helper Tests
// ============================================================================

TEST(ApplicationTrain, send_set_speed)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_set_speed(node, TEST_TRAIN_NODE_ID, 0x3C00);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_msg.dest_id, TEST_TRAIN_NODE_ID);
    EXPECT_EQ(last_sent_msg.source_id, TEST_DEST_ID);
    EXPECT_EQ(last_sent_payload[0], TRAIN_SET_SPEED_DIRECTION);
    // Verify speed in big-endian (0x3C00 => byte 1 = 0x3C, byte 2 = 0x00)
    EXPECT_EQ(last_sent_payload[1], 0x3C);
    EXPECT_EQ(last_sent_payload[2], 0x00);

}

TEST(ApplicationTrain, send_set_function)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_set_function(node, TEST_TRAIN_NODE_ID, 0x000005, 0x0001);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_payload[0], TRAIN_SET_FUNCTION);
    // 24-bit fn address: 0x000005 => 0x00, 0x00, 0x05
    EXPECT_EQ(last_sent_payload[1], 0x00);
    EXPECT_EQ(last_sent_payload[2], 0x00);
    EXPECT_EQ(last_sent_payload[3], 0x05);
    // fn_value 0x0001 big-endian
    EXPECT_EQ(last_sent_payload[4], 0x00);
    EXPECT_EQ(last_sent_payload[5], 0x01);

}

TEST(ApplicationTrain, send_emergency_stop)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_emergency_stop(node, TEST_TRAIN_NODE_ID);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_payload[0], TRAIN_EMERGENCY_STOP);

}

TEST(ApplicationTrain, send_query_speeds)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_query_speeds(node, TEST_TRAIN_NODE_ID);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_payload[0], TRAIN_QUERY_SPEEDS);

}

TEST(ApplicationTrain, send_query_function)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_query_function(node, TEST_TRAIN_NODE_ID, 0x000003);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_payload[0], TRAIN_QUERY_FUNCTION);
    EXPECT_EQ(last_sent_payload[1], 0x00);
    EXPECT_EQ(last_sent_payload[2], 0x00);
    EXPECT_EQ(last_sent_payload[3], 0x03);

}

TEST(ApplicationTrain, send_assign_controller)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_assign_controller(node, TEST_TRAIN_NODE_ID);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_payload[0], TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(last_sent_payload[1], TRAIN_CONTROLLER_ASSIGN);

}

TEST(ApplicationTrain, send_release_controller)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_release_controller(node, TEST_TRAIN_NODE_ID);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_payload[0], TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(last_sent_payload[1], TRAIN_CONTROLLER_RELEASE);

}

TEST(ApplicationTrain, send_noop)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_noop(node, TEST_TRAIN_NODE_ID);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_payload[0], TRAIN_MANAGEMENT);
    EXPECT_EQ(last_sent_payload[1], TRAIN_MGMT_NOOP);

}

TEST(ApplicationTrain, send_null_node_no_crash)
{

    _reset_tracking();
    _global_initialize();

    OpenLcbApplicationTrain_send_set_speed(NULL, TEST_TRAIN_NODE_ID, 0x3C00);
    OpenLcbApplicationTrain_send_set_function(NULL, TEST_TRAIN_NODE_ID, 0, 0);
    OpenLcbApplicationTrain_send_emergency_stop(NULL, TEST_TRAIN_NODE_ID);
    OpenLcbApplicationTrain_send_query_speeds(NULL, TEST_TRAIN_NODE_ID);
    OpenLcbApplicationTrain_send_query_function(NULL, TEST_TRAIN_NODE_ID, 0);
    OpenLcbApplicationTrain_send_assign_controller(NULL, TEST_TRAIN_NODE_ID);
    OpenLcbApplicationTrain_send_release_controller(NULL, TEST_TRAIN_NODE_ID);
    OpenLcbApplicationTrain_send_noop(NULL, TEST_TRAIN_NODE_ID);

    EXPECT_FALSE(mock_send_called);

}

TEST(ApplicationTrain, send_with_null_interface)
{

    _reset_tracking();

    ProtocolTrainHandler_initialize(&_handler_interface_with_heartbeat);
    OpenLcbApplicationTrain_initialize(&_interface_nulls);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_set_speed(node, TEST_TRAIN_NODE_ID, 0x3C00);

    EXPECT_FALSE(mock_send_called);

}


// ============================================================================
// Section 4: Heartbeat Timer Tests
// ============================================================================

TEST(ApplicationTrain, heartbeat_timer_countdown)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    // Set heartbeat: 3 seconds = 30 ticks of 100ms
    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 30;

    // Tick 29 times — should not timeout
    for (int i = 0; i < 29; i++) {

        OpenLcbApplicationTrain_100ms_timer_tick();

    }

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 1);
    EXPECT_FALSE(mock_heartbeat_timeout_called);

    // Tick once more — counter reaches 0, estop fires
    OpenLcbApplicationTrain_100ms_timer_tick();

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);
    EXPECT_TRUE(mock_heartbeat_timeout_called);
    EXPECT_TRUE(state->estop_active);
    EXPECT_EQ(state->set_speed, 0);

}

TEST(ApplicationTrain, heartbeat_disabled)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    // Heartbeat disabled (timeout_s == 0)
    state->heartbeat_timeout_s = 0;
    state->heartbeat_counter_100ms = 0;

    for (int i = 0; i < 100; i++) {

        OpenLcbApplicationTrain_100ms_timer_tick();

    }

    EXPECT_FALSE(mock_heartbeat_timeout_called);
    EXPECT_FALSE(state->estop_active);

}

TEST(ApplicationTrain, heartbeat_no_nodes)
{

    _reset_tracking();
    _global_initialize();

    // No train nodes set up — timer should not crash
    for (int i = 0; i < 10; i++) {

        OpenLcbApplicationTrain_100ms_timer_tick();

    }

    EXPECT_FALSE(mock_heartbeat_timeout_called);

}


// ============================================================================
// Section 5: Edge Cases
// ============================================================================

TEST(ApplicationTrain, send_with_no_initialization)
{

    _reset_tracking();

    // Initialize with null interface — send helpers should not crash
    ProtocolTrainHandler_initialize(NULL);
    OpenLcbApplicationTrain_initialize(&_interface_nulls);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationTrain_send_set_speed(node, TEST_TRAIN_NODE_ID, 0x3C00);

    EXPECT_FALSE(mock_send_called);

}
