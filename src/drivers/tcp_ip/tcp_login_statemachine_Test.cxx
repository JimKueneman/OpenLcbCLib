/*******************************************************************************
 * File: tcp_login_statemachine_Test.cxx
 * Description:
 *   Test suite for TCP Login State Machine. TCP has no transport-level login:
 *   link_up goes straight to COMPLETE and nothing is sent. Node login is the
 *   OpenLCB login statemachine's job (Initialization Complete first, per
 *   Message Network 3.4.1), so this module must never put a message on the
 *   wire, and in particular never a Verify Node ID Global with a zero source
 *   Node ID as an earlier version did.
 * Module Under Test:
 *   TcpLoginStatemachine
 * Test Coverage:
 *   - Initialize sets state to IDLE
 *   - link_up transitions directly to COMPLETE
 *   - run() returns false in every state (no pending work)
 *   - Re-initialize returns to IDLE
 *   - link_up after link_down/re-initialize completes again
 * Author: Test Suite
 * Date: 2026-09-25
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_login_statemachine.h"

// =============================================================================
// Test setup
// =============================================================================

static void setup_test(void)
{
    TcpLoginStatemachine_initialize();
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
// Link Up -- completes immediately, nothing to send
// =============================================================================

TEST(TCP_LoginStatemachine, link_up_completes_immediately)
{
    setup_test();
    TcpLoginStatemachine_link_up();
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);
}

// =============================================================================
// Run -- never has pending work
// =============================================================================

TEST(TCP_LoginStatemachine, run_returns_false_when_idle)
{
    setup_test();
    EXPECT_FALSE(TcpLoginStatemachine_run());
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_IDLE);
}

TEST(TCP_LoginStatemachine, run_returns_false_when_complete)
{
    setup_test();
    TcpLoginStatemachine_link_up();
    EXPECT_FALSE(TcpLoginStatemachine_run());
    EXPECT_FALSE(TcpLoginStatemachine_run());
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);
}

// =============================================================================
// Re-initialize resets state, and a second link_up completes again
// =============================================================================

TEST(TCP_LoginStatemachine, reinitialize_resets_to_idle)
{
    setup_test();
    TcpLoginStatemachine_link_up();
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);

    TcpLoginStatemachine_initialize();
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_IDLE);

    TcpLoginStatemachine_link_up();
    EXPECT_EQ(TcpLoginStatemachine_get_state(), TCP_LOGIN_COMPLETE);
}
