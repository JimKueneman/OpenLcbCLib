/*******************************************************************************
 * File: tcp_login_statemachine_Test.cxx
 *
 * Description:
 *   Test suite for TCP Login State Machine -- verifies the TCP login sequence:
 *   link_up transitions to SEND_VERIFY_GLOBAL, run() allocates buffer,
 *   sends Verify Node ID Global, transitions to COMPLETE.
 *
 * Module Under Test:
 *   TcpLoginStatemachine -- TCP-specific login (no CAN alias negotiation)
 *
 * Test Coverage:
 *   - Initialize sets state to IDLE
 *   - link_up transitions to SEND_VERIFY_GLOBAL (no send)
 *   - run() sends Verify Node ID Global and transitions to COMPLETE
 *   - run() retries on send failure
 *   - run() retries on allocation failure
 *   - run() returns false when IDLE or COMPLETE
 *
 * Author: Test Suite
 * Date: 2026-04-05
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_login_statemachine.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_defines.h"

// =============================================================================
// Mock state
// =============================================================================

static bool _send_called = false;
static uint16_t _sent_mti = 0;
static node_id_t _sent_source_id = 0;
static node_id_t _sent_dest_id = 0;
static bool _send_returns_false = false;

static int _allocate_count = 0;
static int _free_count = 0;
static bool _allocate_returns_null = false;

// =============================================================================
// Mock functions
// =============================================================================

static bool _mock_send(openlcb_msg_t *msg)
{

    if (_send_returns_false) {

        return false;

    }

    _send_called = true;
    _sent_mti = msg->mti;
    _sent_source_id = msg->source_id;
    _sent_dest_id = msg->dest_id;
    return true;

}

static openlcb_msg_t *_mock_allocate(payload_type_enum payload_type)
{

    _allocate_count++;

    if (_allocate_returns_null)
        return NULL;

    return OpenLcbBufferStore_allocate_buffer(payload_type);

}

static void _mock_free(openlcb_msg_t *msg)
{

    _free_count++;
    OpenLcbBufferStore_free_buffer(msg);

}

static void _mock_lock(void) {}
static void _mock_unlock(void) {}

// =============================================================================
// Test setup
// =============================================================================

static const interface_tcp_login_statemachine_t _interface = {

    .send_openlcb_msg      = &_mock_send,
    .allocate_buffer       = &_mock_allocate,
    .free_buffer           = &_mock_free,
    .lock_shared_resources = &_mock_lock,
    .unlock_shared_resources = &_mock_unlock,

};

static void reset_mocks(void)
{

    _send_called = false;
    _sent_mti = 0;
    _sent_source_id = 0;
    _sent_dest_id = 0;
    _send_returns_false = false;
    _allocate_count = 0;
    _free_count = 0;
    _allocate_returns_null = false;

}

static void setup_test(void)
{

    reset_mocks();
    OpenLcbBufferStore_initialize();
    TcpLoginStatemachine_initialize(&_interface);

}

// =============================================================================
// Initialize
// =============================================================================

TEST(TCP_LoginStatemachine, initialize_sets_idle)
{

    setup_test();

    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_IDLE);

}

// =============================================================================
// Link Up -- transitions to SEND_VERIFY_GLOBAL, does not send
// =============================================================================

TEST(TCP_LoginStatemachine, link_up_transitions_to_send_verify_global)
{

    setup_test();

    TcpLoginStatemachine_link_up();

    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_SEND_VERIFY_GLOBAL);
    EXPECT_FALSE(_send_called);
    EXPECT_EQ(_allocate_count, 0);

}

// =============================================================================
// Run -- sends Verify Node ID Global and completes
// =============================================================================

TEST(TCP_LoginStatemachine, run_sends_verify_and_completes)
{

    setup_test();

    TcpLoginStatemachine_link_up();

    bool busy = TcpLoginStatemachine_run();

    EXPECT_FALSE(busy);
    EXPECT_TRUE(_send_called);
    EXPECT_EQ(_sent_mti, MTI_VERIFY_NODE_ID_GLOBAL);
    EXPECT_EQ(_sent_source_id, (node_id_t) 0);
    EXPECT_EQ(_sent_dest_id, (node_id_t) 0);
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);
    EXPECT_EQ(_allocate_count, 1);
    EXPECT_EQ(_free_count, 1);

}

// =============================================================================
// Run -- retries on send failure
// =============================================================================

TEST(TCP_LoginStatemachine, run_retries_on_send_failure)
{

    setup_test();

    TcpLoginStatemachine_link_up();

    // First run: send fails
    _send_returns_false = true;

    bool busy = TcpLoginStatemachine_run();

    EXPECT_TRUE(busy);
    EXPECT_FALSE(_send_called);
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_SEND_VERIFY_GLOBAL);
    EXPECT_EQ(_allocate_count, 1);
    EXPECT_EQ(_free_count, 1);

    // Second run: send succeeds
    _send_returns_false = false;

    busy = TcpLoginStatemachine_run();

    EXPECT_FALSE(busy);
    EXPECT_TRUE(_send_called);
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);
    EXPECT_EQ(_allocate_count, 2);
    EXPECT_EQ(_free_count, 2);

}

// =============================================================================
// Run -- retries on allocation failure
// =============================================================================

TEST(TCP_LoginStatemachine, run_retries_on_allocation_failure)
{

    setup_test();

    TcpLoginStatemachine_link_up();

    // First run: allocation fails
    _allocate_returns_null = true;

    bool busy = TcpLoginStatemachine_run();

    EXPECT_TRUE(busy);
    EXPECT_FALSE(_send_called);
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_SEND_VERIFY_GLOBAL);
    EXPECT_EQ(_allocate_count, 1);
    EXPECT_EQ(_free_count, 0);

    // Second run: allocation succeeds, send succeeds
    _allocate_returns_null = false;

    busy = TcpLoginStatemachine_run();

    EXPECT_FALSE(busy);
    EXPECT_TRUE(_send_called);
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);
    EXPECT_EQ(_allocate_count, 2);
    EXPECT_EQ(_free_count, 1);

}

// =============================================================================
// Run -- returns false when IDLE
// =============================================================================

TEST(TCP_LoginStatemachine, run_returns_false_when_idle)
{

    setup_test();

    // State is IDLE, run should be a no-op
    bool busy = TcpLoginStatemachine_run();

    EXPECT_FALSE(busy);
    EXPECT_FALSE(_send_called);
    EXPECT_EQ(_allocate_count, 0);

}

// =============================================================================
// Run -- returns false when already COMPLETE
// =============================================================================

TEST(TCP_LoginStatemachine, run_returns_false_when_complete)
{

    setup_test();

    TcpLoginStatemachine_link_up();
    TcpLoginStatemachine_run();

    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);

    // Run again after completion
    reset_mocks();

    bool busy = TcpLoginStatemachine_run();

    EXPECT_FALSE(busy);
    EXPECT_FALSE(_send_called);
    EXPECT_EQ(_allocate_count, 0);

}

// =============================================================================
// Re-initialize resets state
// =============================================================================

TEST(TCP_LoginStatemachine, reinitialize_resets_to_idle)
{

    setup_test();

    TcpLoginStatemachine_link_up();
    TcpLoginStatemachine_run();
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);

    TcpLoginStatemachine_initialize(&_interface);
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_IDLE);

}
