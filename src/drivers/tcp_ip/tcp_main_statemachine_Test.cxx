/*******************************************************************************
 * File: tcp_main_statemachine_Test.cxx
 *
 * Description:
 *   Test suite for TCP Main State Machine -- verifies link-up/down state
 *   transitions, login orchestration, link control run-loop, and callback
 *   invocation.
 *
 * Module Under Test:
 *   TcpMainStatemachine -- orchestrates TCP link lifecycle
 *
 * Test Coverage:
 *   - Initialize sets DISCONNECTED state
 *   - link_up transitions to LOGGING_IN and calls login_link_up
 *   - link_up clears pending bits
 *   - run() drives login and transitions to RUNNING on completion
 *   - run() stays in LOGGING_IN while login is busy
 *   - run() drives link control pending replies
 *   - link_down transitions to DISCONNECTED
 *   - NULL on_link_status_changed does not crash
 *
 * Author: Test Suite
 * Date: 2026-04-05
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_main_statemachine.h"

// =============================================================================
// Mock state
// =============================================================================

static bool _login_link_up_called = false;
static tcp_login_state_enum _mock_login_state = TCP_LOGIN_IDLE;
static bool _login_run_returns_busy = false;
static int _login_run_count = 0;
static uint8_t _mock_tick = 0;

static bool _status_changed_called = false;
static bool _status_is_up = false;

static int _link_control_run_count = 0;
static bool _link_control_run_returns_busy = false;
static tcp_statemachine_info_t *_link_control_run_info = NULL;

// =============================================================================
// Mock functions
// =============================================================================

static void _mock_login_link_up(void)
{

    _login_link_up_called = true;

}

static tcp_login_state_enum _mock_login_get_state(void)
{

    return _mock_login_state;

}

static bool _mock_login_run(void)
{

    _login_run_count++;

    if (_login_run_returns_busy) {

        return true;

    }

    _mock_login_state = TCP_LOGIN_COMPLETE;
    return false;

}

static bool _mock_link_control_run(tcp_statemachine_info_t *info)
{

    _link_control_run_count++;
    _link_control_run_info = info;
    return _link_control_run_returns_busy;

}

static uint8_t _mock_get_tick(void)
{

    return _mock_tick;

}

static void _mock_on_link_status_changed(bool is_up)
{

    _status_changed_called = true;
    _status_is_up = is_up;

}

// =============================================================================
// Test setup
// =============================================================================

static interface_tcp_main_statemachine_t _interface;

static void reset_mocks(void)
{

    _login_link_up_called = false;
    _mock_login_state = TCP_LOGIN_IDLE;
    _login_run_returns_busy = false;
    _login_run_count = 0;
    _mock_tick = 0;
    _status_changed_called = false;
    _status_is_up = false;
    _link_control_run_count = 0;
    _link_control_run_returns_busy = false;
    _link_control_run_info = NULL;

}

static void setup_test(void)
{

    reset_mocks();

    _interface.login_link_up          = &_mock_login_link_up;
    _interface.login_run              = &_mock_login_run;
    _interface.login_get_state        = &_mock_login_get_state;
    _interface.link_control_run       = &_mock_link_control_run;
    _interface.get_current_tick       = &_mock_get_tick;
    _interface.on_link_status_changed = &_mock_on_link_status_changed;

    TcpMainStatemachine_initialize(&_interface);

}

// =============================================================================
// Initialize
// =============================================================================

TEST(TCP_MainStatemachine, initialize_sets_disconnected)
{

    setup_test();

    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_DISCONNECTED);

}

TEST(TCP_MainStatemachine, initialize_clears_statemachine_info)
{

    setup_test();

    tcp_statemachine_info_t *info = TcpMainStatemachine_get_statemachine_info();
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->openlcb_node, nullptr);
    EXPECT_EQ(info->login_state, TCP_LOGIN_IDLE);
    EXPECT_EQ(info->link_state, TCP_LINK_STATE_DISCONNECTED);
    EXPECT_EQ(info->current_tick, 0);

}

// =============================================================================
// Link Up -- transitions to LOGGING_IN
// =============================================================================

TEST(TCP_MainStatemachine, link_up_triggers_login)
{

    setup_test();

    TcpMainStatemachine_link_up();

    EXPECT_TRUE(_login_link_up_called);

}

TEST(TCP_MainStatemachine, link_up_transitions_to_logging_in)
{

    setup_test();

    TcpMainStatemachine_link_up();

    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_LOGGING_IN);

}

TEST(TCP_MainStatemachine, link_up_notifies_status_changed_true)
{

    setup_test();

    TcpMainStatemachine_link_up();

    EXPECT_TRUE(_status_changed_called);
    EXPECT_TRUE(_status_is_up);

}

TEST(TCP_MainStatemachine, link_up_clears_pending_bits)
{

    setup_test();

    // Set pending bits before link_up
    tcp_statemachine_info_t *info = TcpMainStatemachine_get_statemachine_info();
    info->pending_status_reply = 1;
    info->pending_drop_reply = 1;

    TcpMainStatemachine_link_up();

    EXPECT_EQ(info->pending_status_reply, (uint8_t) 0);
    EXPECT_EQ(info->pending_drop_reply, (uint8_t) 0);

}

// =============================================================================
// Run -- drives login and transitions to RUNNING
// =============================================================================

TEST(TCP_MainStatemachine, run_completes_login_transitions_to_running)
{

    setup_test();

    TcpMainStatemachine_link_up();

    // login_run will set state to COMPLETE and return false
    bool busy = TcpMainStatemachine_run();

    EXPECT_EQ(_login_run_count, 1);
    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_RUNNING);
    EXPECT_FALSE(busy);

}

TEST(TCP_MainStatemachine, run_stays_logging_in_while_login_busy)
{

    setup_test();

    TcpMainStatemachine_link_up();

    _login_run_returns_busy = true;

    bool busy = TcpMainStatemachine_run();

    EXPECT_TRUE(busy);
    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_LOGGING_IN);

}

// =============================================================================
// Run -- drives link control
// =============================================================================

TEST(TCP_MainStatemachine, run_drives_link_control)
{

    setup_test();

    // Get past login
    TcpMainStatemachine_link_up();
    TcpMainStatemachine_run();
    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_RUNNING);

    _link_control_run_count = 0;

    TcpMainStatemachine_run();

    EXPECT_EQ(_link_control_run_count, 1);
    EXPECT_NE(_link_control_run_info, nullptr);

}

TEST(TCP_MainStatemachine, run_returns_busy_when_link_control_busy)
{

    setup_test();

    // Get past login
    TcpMainStatemachine_link_up();
    TcpMainStatemachine_run();

    _link_control_run_returns_busy = true;

    bool busy = TcpMainStatemachine_run();

    EXPECT_TRUE(busy);

}

// =============================================================================
// Link Down
// =============================================================================

TEST(TCP_MainStatemachine, link_down_transitions_to_disconnected)
{

    setup_test();

    TcpMainStatemachine_link_up();
    TcpMainStatemachine_run();
    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_RUNNING);

    _status_changed_called = false;
    TcpMainStatemachine_link_down();

    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_DISCONNECTED);

}

TEST(TCP_MainStatemachine, link_down_notifies_status_changed_false)
{

    setup_test();

    TcpMainStatemachine_link_up();
    TcpMainStatemachine_run();
    _status_changed_called = false;

    TcpMainStatemachine_link_down();

    EXPECT_TRUE(_status_changed_called);
    EXPECT_FALSE(_status_is_up);

}

// =============================================================================
// NULL Callback -- No Crash
// =============================================================================

TEST(TCP_MainStatemachine, null_status_callback_no_crash)
{

    reset_mocks();

    _interface.login_link_up          = &_mock_login_link_up;
    _interface.login_run              = &_mock_login_run;
    _interface.login_get_state        = &_mock_login_get_state;
    _interface.link_control_run       = &_mock_link_control_run;
    _interface.get_current_tick       = &_mock_get_tick;
    _interface.on_link_status_changed = NULL;

    TcpMainStatemachine_initialize(&_interface);

    TcpMainStatemachine_link_up();
    TcpMainStatemachine_link_down();

    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_DISCONNECTED);

}

// =============================================================================
// get_statemachine_info returns valid pointer
// =============================================================================

TEST(TCP_MainStatemachine, get_statemachine_info_not_null)
{

    setup_test();

    EXPECT_NE(TcpMainStatemachine_get_statemachine_info(), nullptr);

}
