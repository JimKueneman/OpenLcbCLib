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
#include "openlcb_float16.h"


// ============================================================================
// Test Constants
// ============================================================================

#define TEST_SOURCE_ALIAS 0x222
#define TEST_SOURCE_ID 0x010203040506ULL
#define TEST_DEST_ALIAS 0xBBB
#define TEST_DEST_ID 0x060504030201ULL
#define TEST_TRAIN_NODE_ID 0xAABBCCDDEE01ULL
#define TEST_TRAIN_ALIAS 0x0AA
#define TEST_CONTROLLER_NODE_ID 0x0A0B0C0D0E0FULL
#define TEST_LISTENER_NODE_ID 0x112233445566ULL


// ============================================================================
// Test Tracking Variables
// ============================================================================

static bool mock_send_called = false;
static openlcb_msg_t last_sent_msg;
static payload_basic_t last_sent_payload;
static int send_call_count = 0;

#define MAX_SENT_MESSAGES 8
static openlcb_msg_t sent_msgs[MAX_SENT_MESSAGES];
static payload_basic_t sent_payloads[MAX_SENT_MESSAGES];

static bool mock_heartbeat_timeout_called = false;
static openlcb_node_t *mock_heartbeat_timeout_node = NULL;


// ============================================================================
// Reset
// ============================================================================

static void _reset_tracking(void) {

    mock_send_called = false;
    memset(&last_sent_msg, 0, sizeof(last_sent_msg));
    memset(&last_sent_payload, 0, sizeof(last_sent_payload));
    memset(sent_msgs, 0, sizeof(sent_msgs));
    memset(sent_payloads, 0, sizeof(sent_payloads));
    send_call_count = 0;
    mock_heartbeat_timeout_called = false;
    mock_heartbeat_timeout_node = NULL;

}


// ============================================================================
// Mock Functions
// ============================================================================

static bool _mock_send_openlcb_msg(openlcb_msg_t *openlcb_msg) {

    mock_send_called = true;

    if (send_call_count < MAX_SENT_MESSAGES) {

        memcpy(&sent_msgs[send_call_count], openlcb_msg, sizeof(openlcb_msg_t));

        if (openlcb_msg->payload) {

            memcpy(&sent_payloads[send_call_count], openlcb_msg->payload, LEN_MESSAGE_BYTES_BASIC);

        }

    }

    send_call_count++;

    memcpy(&last_sent_msg, openlcb_msg, sizeof(openlcb_msg_t));

    if (openlcb_msg->payload) {

        memcpy(&last_sent_payload, openlcb_msg->payload, LEN_MESSAGE_BYTES_BASIC);

    }

    return true;

}

static void _mock_on_heartbeat_timeout(openlcb_node_t *openlcb_node) {

    mock_heartbeat_timeout_called = true;
    mock_heartbeat_timeout_node = openlcb_node;

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

    /** @brief Interface with send wired but no heartbeat callback — exercises
     *  the on_heartbeat_timeout == NULL branch at line 354 */
static interface_openlcb_application_train_t _interface_send_no_callback = {

    .send_openlcb_msg = &_mock_send_openlcb_msg,
    .on_heartbeat_timeout = NULL,

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
    EXPECT_EQ(state->owner_node, node);
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

    OpenLcbApplicationTrain_send_set_speed(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID, 0x3C00);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_msg.dest_alias, TEST_TRAIN_ALIAS);
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

    OpenLcbApplicationTrain_send_set_function(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID, 0x000005, 0x0001);

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

    OpenLcbApplicationTrain_send_emergency_stop(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);

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

    OpenLcbApplicationTrain_send_query_speeds(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);

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

    OpenLcbApplicationTrain_send_query_function(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID, 0x000003);

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

    OpenLcbApplicationTrain_send_assign_controller(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);

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

    OpenLcbApplicationTrain_send_release_controller(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);

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

    OpenLcbApplicationTrain_send_noop(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(last_sent_payload[0], TRAIN_MANAGEMENT);
    EXPECT_EQ(last_sent_payload[1], TRAIN_MGMT_NOOP);

}

TEST(ApplicationTrain, send_null_node_no_crash)
{

    _reset_tracking();
    _global_initialize();

    OpenLcbApplicationTrain_send_set_speed(NULL, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID, 0x3C00);
    OpenLcbApplicationTrain_send_set_function(NULL, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID, 0, 0);
    OpenLcbApplicationTrain_send_emergency_stop(NULL, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);
    OpenLcbApplicationTrain_send_query_speeds(NULL, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);
    OpenLcbApplicationTrain_send_query_function(NULL, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID, 0);
    OpenLcbApplicationTrain_send_assign_controller(NULL, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);
    OpenLcbApplicationTrain_send_release_controller(NULL, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);
    OpenLcbApplicationTrain_send_noop(NULL, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID);

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

    OpenLcbApplicationTrain_send_set_speed(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID, 0x3C00);

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

        OpenLcbApplicationTrain_100ms_timer_tick((uint8_t)(i + 1));

    }

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 1);
    EXPECT_FALSE(mock_heartbeat_timeout_called);

    // Tick once more — counter reaches 0, estop fires
    OpenLcbApplicationTrain_100ms_timer_tick(30);

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);
    EXPECT_TRUE(mock_heartbeat_timeout_called);
    EXPECT_EQ(mock_heartbeat_timeout_node, node);
    EXPECT_TRUE(state->estop_active);
    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);

}

TEST(ApplicationTrain, heartbeat_timer_countdown_reverse)
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

    // Simulate reverse movement: set_speed has the sign bit set
    state->set_speed = FLOAT16_NEGATIVE_ZERO | 0x3C00;  // Reverse, ~1.0 mph

    // Tick 30 times -- counter reaches 0, estop fires
    for (int i = 0; i < 30; i++) {

        OpenLcbApplicationTrain_100ms_timer_tick((uint8_t)(i + 1));

    }

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);
    EXPECT_TRUE(mock_heartbeat_timeout_called);
    EXPECT_EQ(mock_heartbeat_timeout_node, node);
    EXPECT_TRUE(state->estop_active);

    // Direction must be preserved: reverse, stopped
    EXPECT_EQ(state->set_speed, FLOAT16_NEGATIVE_ZERO);

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

        OpenLcbApplicationTrain_100ms_timer_tick((uint8_t)(i + 1));

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

        OpenLcbApplicationTrain_100ms_timer_tick((uint8_t)(i + 1));

    }

    EXPECT_FALSE(mock_heartbeat_timeout_called);

}

TEST(ApplicationTrain, heartbeat_sends_request_at_halfway)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 100;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    // Tick 50 times to reach halfway (100 - 50 = 50 = halfway of 10s * 10 / 2)
    for (int i = 0; i < 50; i++) {

        OpenLcbApplicationTrain_100ms_timer_tick((uint8_t)(i + 1));

    }

    EXPECT_TRUE(mock_send_called);
    EXPECT_EQ(send_call_count, 1);
    EXPECT_EQ(last_sent_msg.mti, MTI_TRAIN_REPLY);
    EXPECT_EQ(last_sent_msg.dest_id, TEST_CONTROLLER_NODE_ID);
    EXPECT_EQ(last_sent_msg.source_id, TEST_DEST_ID);
    EXPECT_EQ(last_sent_payload[0], TRAIN_MANAGEMENT);
    EXPECT_EQ(last_sent_payload[1], TRAIN_MGMT_NOOP);
    // 3-byte timeout: 10 seconds = 0x00, 0x00, 0x0A
    EXPECT_EQ(last_sent_payload[2], 0x00);
    EXPECT_EQ(last_sent_payload[3], 0x00);
    EXPECT_EQ(last_sent_payload[4], 0x0A);

}

TEST(ApplicationTrain, heartbeat_no_request_without_controller)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 100;
    state->controller_node_id = 0;

    // Tick to halfway — no controller, should not send
    for (int i = 0; i < 50; i++) {

        OpenLcbApplicationTrain_100ms_timer_tick((uint8_t)(i + 1));

    }

    EXPECT_FALSE(mock_send_called);

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

    OpenLcbApplicationTrain_send_set_speed(node, TEST_TRAIN_ALIAS, TEST_TRAIN_NODE_ID, 0x3C00);

    EXPECT_FALSE(mock_send_called);

}


// ============================================================================
// Section 7: Pool Exhaustion — Line 115
// ============================================================================

TEST(ApplicationTrain, pool_exhaustion_returns_null)
{

    _global_initialize();

    // Allocate all 4 pool slots using the node allocator
    openlcb_node_t *nodes[USER_DEFINED_TRAIN_NODE_COUNT];

    for (int i = 0; i < USER_DEFINED_TRAIN_NODE_COUNT; i++) {

        nodes[i] = OpenLcbNode_allocate(TEST_DEST_ID + i, &_test_node_parameters);

        EXPECT_NE(nodes[i], nullptr);

        // _clear_node does not zero train_state, so force it NULL
        nodes[i]->train_state = NULL;

        train_state_t *state = OpenLcbApplicationTrain_setup(nodes[i]);

        EXPECT_NE(state, nullptr);

    }

    // NODE_BUFFER_DEPTH is also 4, so use a stack-local node for the 5th attempt
    openlcb_node_t extra_node;
    memset(&extra_node, 0, sizeof(openlcb_node_t));
    extra_node.id = 0xFFFFFFFFFF00ULL;
    extra_node.train_state = NULL;

    // Pool is full — must return NULL
    train_state_t *overflow = OpenLcbApplicationTrain_setup(&extra_node);

    EXPECT_EQ(overflow, nullptr);
    EXPECT_EQ(extra_node.train_state, nullptr);

}


// ============================================================================
// Section 8: Heartbeat Request Null Send Callback — Line 183
// ============================================================================

TEST(ApplicationTrain, heartbeat_request_null_send_callback)
{

    _reset_tracking();

    // Initialize with _interface_nulls so send_openlcb_msg is NULL
    ProtocolTrainHandler_initialize(&_handler_interface_with_heartbeat);
    OpenLcbApplicationTrain_initialize(&_interface_nulls);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 100;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    // Tick to halfway — send_openlcb_msg is NULL, should not crash
    for (int i = 0; i < 50; i++) {

        OpenLcbApplicationTrain_100ms_timer_tick((uint8_t)(i + 1));

    }

    // send was NULL so nothing should have been sent
    EXPECT_FALSE(mock_send_called);

}


// ============================================================================
// Section 9: Timer Tick Same Value Early Return — Line 241
// ============================================================================

TEST(ApplicationTrain, timer_tick_same_value_early_return)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 100;

    // First tick with value 5 — should process normally
    OpenLcbApplicationTrain_100ms_timer_tick(5);

    uint32_t counter_after_first = state->heartbeat_counter_100ms;

    EXPECT_EQ(counter_after_first, (uint32_t) 95);

    // Second tick with same value 5 — ticks_elapsed == 0, early return
    OpenLcbApplicationTrain_100ms_timer_tick(5);

    // Counter must not have changed
    EXPECT_EQ(state->heartbeat_counter_100ms, counter_after_first);

}


// ============================================================================
// Section 10: DCC Address / Speed Steps — Lines 624-681
// ============================================================================

TEST(ApplicationTrain, dcc_address_null_node)
{

    _global_initialize();

    // set with NULL node — should not crash
    OpenLcbApplicationTrain_set_dcc_address(NULL, 1234, true);

    // get with NULL node — returns 0
    EXPECT_EQ(OpenLcbApplicationTrain_get_dcc_address(NULL), (uint16_t) 0);

}

TEST(ApplicationTrain, dcc_address_null_train_state)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;

    // set with valid node but NULL train_state — should not crash
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, true);

    // get with valid node but NULL train_state — returns 0
    EXPECT_EQ(OpenLcbApplicationTrain_get_dcc_address(node), (uint16_t) 0);

}

TEST(ApplicationTrain, is_long_address_null_node)
{

    _global_initialize();

    // NULL node — returns false
    EXPECT_FALSE(OpenLcbApplicationTrain_is_long_address(NULL));

}

TEST(ApplicationTrain, is_long_address_null_train_state)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;

    // Valid node, NULL train_state — returns false
    EXPECT_FALSE(OpenLcbApplicationTrain_is_long_address(node));

}

TEST(ApplicationTrain, speed_steps_null_node)
{

    _global_initialize();

    // set with NULL node — should not crash
    OpenLcbApplicationTrain_set_speed_steps(NULL, 128);

    // get with NULL node — returns 0
    EXPECT_EQ(OpenLcbApplicationTrain_get_speed_steps(NULL), (uint8_t) 0);

}

TEST(ApplicationTrain, speed_steps_null_train_state)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;

    // set with valid node but NULL train_state — should not crash
    OpenLcbApplicationTrain_set_speed_steps(node, 128);

    // get with valid node but NULL train_state — returns 0
    EXPECT_EQ(OpenLcbApplicationTrain_get_speed_steps(node), (uint8_t) 0);

}

TEST(ApplicationTrain, dcc_address_normal_operation)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    // Set short address
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    EXPECT_EQ(OpenLcbApplicationTrain_get_dcc_address(node), (uint16_t) 42);
    EXPECT_FALSE(OpenLcbApplicationTrain_is_long_address(node));

    // Change to long address
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);

    EXPECT_EQ(OpenLcbApplicationTrain_get_dcc_address(node), (uint16_t) 9999);
    EXPECT_TRUE(OpenLcbApplicationTrain_is_long_address(node));

}

TEST(ApplicationTrain, speed_steps_normal_operation)
{

    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    // Default after setup should be 0
    EXPECT_EQ(OpenLcbApplicationTrain_get_speed_steps(node), (uint8_t) 0);

    OpenLcbApplicationTrain_set_speed_steps(node, 128);

    EXPECT_EQ(OpenLcbApplicationTrain_get_speed_steps(node), (uint8_t) 128);

    OpenLcbApplicationTrain_set_speed_steps(node, 28);

    EXPECT_EQ(OpenLcbApplicationTrain_get_speed_steps(node), (uint8_t) 28);

}


// ============================================================================
// Section 11: Heartbeat listener forwarding (TrainControlS 6.6)
// ============================================================================

TEST(ApplicationTrain, heartbeat_timeout_forwards_speed_zero_to_listeners)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    // Configure heartbeat
    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 1;  // About to expire
    state->set_speed = 0x3C00;           // Forward 1.0
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    // Add two listeners
    state->listeners[0].node_id = TEST_LISTENER_NODE_ID;
    state->listeners[0].flags = 0x00;
    state->listeners[1].node_id = 0x223344556677ULL;
    state->listeners[1].flags = 0x00;
    state->listener_count = 2;

    // Tick past expiry
    OpenLcbApplicationTrain_100ms_timer_tick(1);

    // Heartbeat request was sent at halfway (not here — counter was at 1)
    // Timeout fires: speed zeroed, listeners forwarded, callback fired
    EXPECT_TRUE(mock_heartbeat_timeout_called);
    EXPECT_TRUE(state->estop_active);
    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);

    // Should have sent: 2 listener forwards + (no heartbeat request since
    // counter jumped from 1 to 0, skipping halfway)
    EXPECT_EQ(send_call_count, 2);

    // First listener: Set Speed 0 with P bit
    EXPECT_EQ(sent_msgs[0].dest_id, TEST_LISTENER_NODE_ID);
    EXPECT_EQ(sent_msgs[0].mti, MTI_TRAIN_PROTOCOL);
    EXPECT_EQ(sent_payloads[0][0], (uint8_t) (TRAIN_SET_SPEED_DIRECTION | TRAIN_INSTRUCTION_P_BIT));
    EXPECT_EQ(sent_payloads[0][1], (uint8_t) 0x00);
    EXPECT_EQ(sent_payloads[0][2], (uint8_t) 0x00);

    // Second listener
    EXPECT_EQ(sent_msgs[1].dest_id, (uint64_t) 0x223344556677ULL);
    EXPECT_EQ(sent_payloads[1][0], (uint8_t) (TRAIN_SET_SPEED_DIRECTION | TRAIN_INSTRUCTION_P_BIT));

}

TEST(ApplicationTrain, heartbeat_timeout_respects_listener_reverse_flag)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 1;
    state->set_speed = 0x3C00;  // Forward
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    // Listener with REVERSE flag
    state->listeners[0].node_id = TEST_LISTENER_NODE_ID;
    state->listeners[0].flags = TRAIN_LISTENER_FLAG_REVERSE;
    state->listener_count = 1;

    OpenLcbApplicationTrain_100ms_timer_tick(1);

    EXPECT_TRUE(mock_heartbeat_timeout_called);
    EXPECT_EQ(send_call_count, 1);

    // Speed should be negative zero (direction flipped by reverse flag)
    uint16_t forwarded_speed = ((uint16_t) sent_payloads[0][1] << 8) | sent_payloads[0][2];
    EXPECT_EQ(forwarded_speed, (uint16_t) FLOAT16_NEGATIVE_ZERO);

}

TEST(ApplicationTrain, heartbeat_timeout_no_listeners_no_forwards)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 1;
    state->set_speed = 0x3C00;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;
    state->listener_count = 0;

    OpenLcbApplicationTrain_100ms_timer_tick(1);

    EXPECT_TRUE(mock_heartbeat_timeout_called);
    EXPECT_TRUE(state->estop_active);

    // No listeners — no forwards sent (only timeout callback)
    EXPECT_EQ(send_call_count, 0);

}

TEST(ApplicationTrain, heartbeat_timeout_preserves_reverse_direction)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 1;
    state->set_speed = 0xBC00;  // Reverse 1.0 (sign bit set)
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    // Listener without reverse flag
    state->listeners[0].node_id = TEST_LISTENER_NODE_ID;
    state->listeners[0].flags = 0x00;
    state->listener_count = 1;

    OpenLcbApplicationTrain_100ms_timer_tick(1);

    EXPECT_TRUE(mock_heartbeat_timeout_called);

    // Direction preserved: negative zero
    EXPECT_EQ(state->set_speed, FLOAT16_NEGATIVE_ZERO);

    // Forwarded message should also have negative zero
    uint16_t forwarded_speed = ((uint16_t) sent_payloads[0][1] << 8) | sent_payloads[0][2];
    EXPECT_EQ(forwarded_speed, (uint16_t) FLOAT16_NEGATIVE_ZERO);

}


// ============================================================================
// Section 12: Branch Coverage — Missing Branches
// ============================================================================

// ============================================================================
// TEST: Heartbeat timeout with NULL on_heartbeat_timeout callback — exercises
//       the false branch of (_interface->on_heartbeat_timeout) at line 354.
//       Send is wired so estop and listener forwarding still happen.
// ============================================================================

TEST(ApplicationTrain, heartbeat_timeout_null_callback_no_crash)
{

    _reset_tracking();

    ProtocolTrainHandler_initialize(&_handler_interface_with_heartbeat);
    OpenLcbApplicationTrain_initialize(&_interface_send_no_callback);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 1;
    state->set_speed = 0x3C00;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;
    state->listener_count = 0;

    // Tick past expiry — on_heartbeat_timeout is NULL, should not crash
    OpenLcbApplicationTrain_100ms_timer_tick(1);

    // Estop still fires even though callback is NULL
    EXPECT_TRUE(state->estop_active);
    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);

    // Callback was NOT called (it's NULL)
    EXPECT_FALSE(mock_heartbeat_timeout_called);

}

// ============================================================================
// TEST: Large tick jump saturates counter to zero — exercises the else branch
//       at line 328 where heartbeat_counter_100ms <= ticks_elapsed.
// ============================================================================

TEST(ApplicationTrain, heartbeat_large_tick_jump_saturates_counter)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 5;  // Only 5 ticks remaining
    state->set_speed = 0x3C00;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;
    state->listener_count = 0;

    // Jump by 20 ticks at once — counter (5) < ticks_elapsed (20)
    // Should saturate to 0 and trigger timeout
    OpenLcbApplicationTrain_100ms_timer_tick(20);

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);
    EXPECT_TRUE(state->estop_active);
    EXPECT_TRUE(mock_heartbeat_timeout_called);

}

// ============================================================================
// TEST: Heartbeat full timeout with NULL send — exercises _forward_estop_to_listeners
//       and _send_heartbeat_request returning early due to NULL send_openlcb_msg
//       at the actual timeout point (not just halfway).
// ============================================================================

TEST(ApplicationTrain, heartbeat_full_timeout_null_send)
{

    _reset_tracking();

    ProtocolTrainHandler_initialize(&_handler_interface_with_heartbeat);
    OpenLcbApplicationTrain_initialize(&_interface_nulls);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 1;
    state->set_speed = 0x3C00;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;

    // Add a listener so _forward_estop_to_listeners enters the loop guard
    state->listeners[0].node_id = TEST_LISTENER_NODE_ID;
    state->listeners[0].flags = 0x00;
    state->listener_count = 1;

    // Tick to timeout — send is NULL, should not crash
    OpenLcbApplicationTrain_100ms_timer_tick(1);

    // Estop fires, but no messages sent (send is NULL)
    EXPECT_TRUE(state->estop_active);
    EXPECT_FALSE(mock_send_called);
    EXPECT_EQ(send_call_count, 0);

}

// ============================================================================
// TEST: Heartbeat halfway + timeout in the same tick jump — exercises both
//       the halfway send and timeout in a single timer call.
// ============================================================================

TEST(ApplicationTrain, heartbeat_halfway_and_timeout_same_tick)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    // 10 second timeout, halfway = 50 ticks
    state->heartbeat_timeout_s = 10;
    state->heartbeat_counter_100ms = 60;  // above halfway
    state->set_speed = 0x3C00;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;
    state->listener_count = 0;

    // Jump by 100 ticks — passes halfway AND hits zero in one call
    OpenLcbApplicationTrain_100ms_timer_tick(100);

    // Both halfway request AND timeout should have fired
    EXPECT_TRUE(state->estop_active);
    EXPECT_TRUE(mock_heartbeat_timeout_called);

    // Heartbeat request sent at halfway crossing + no listener forwards
    EXPECT_GE(send_call_count, 1);

}

// ============================================================================
// TEST: Counter exactly equals ticks_elapsed — exercises the boundary
//       condition where counter == ticks_elapsed (takes else branch, sets 0).
// ============================================================================

TEST(ApplicationTrain, heartbeat_counter_equals_ticks_elapsed)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 10;  // exactly 10 ticks remaining
    state->set_speed = 0x3C00;
    state->controller_node_id = TEST_CONTROLLER_NODE_ID;
    state->listener_count = 0;

    // Jump by exactly 10 ticks — counter == ticks_elapsed, takes else branch
    OpenLcbApplicationTrain_100ms_timer_tick(10);

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);
    EXPECT_TRUE(state->estop_active);
    EXPECT_TRUE(mock_heartbeat_timeout_called);

}
