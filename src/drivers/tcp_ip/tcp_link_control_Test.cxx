/*******************************************************************************
 * File: tcp_link_control_Test.cxx
 *
 * Description:
 *   Test suite for TCP Link Control module -- verifies handling and generation
 *   of link control messages (status request/reply, drop link request/reply).
 *
 * Module Under Test:
 *   TcpLinkControl -- handles link-level messages (flags bit 15 = 0)
 *
 * Test Coverage:
 *   - Handle Status Request sets pending_status_reply bit
 *   - Handle Status Reply calls on_link_status_changed(true)
 *   - Handle Drop Link Request sets pending_drop_reply bit
 *   - Handle Drop Link Reply calls on_link_status_changed(false)
 *   - run() sends Status Reply and clears pending bit
 *   - run() sends Drop Reply, clears pending bit, fires on_link_drop_requested
 *   - run() retries on send failure
 *   - Send Status Request
 *   - Send Drop Link Request
 *   - Unknown link control type ignored
 *   - Body too short ignored
 *
 * Author: Test Suite
 * Date: 2026-04-05
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_link_control.h"
#include "tcp_utilities.h"

// =============================================================================
// Mock state
// =============================================================================

static uint16_t _sent_flags = 0;
static uint8_t _sent_body[64];
static uint16_t _sent_body_len = 0;
static int _send_count = 0;
static bool _send_returns_false = false;

static bool _link_drop_requested = false;
static bool _link_status_changed_called = false;
static bool _link_status_is_up = false;

static tcp_statemachine_info_t _mock_info;

// =============================================================================
// Mock functions
// =============================================================================

static bool _mock_send_link_control(uint16_t flags, const uint8_t *body, uint16_t body_len)
{

    if (_send_returns_false) {

        return false;

    }

    _sent_flags = flags;
    _sent_body_len = body_len;
    _send_count++;

    if (body_len > 0 && body_len <= sizeof(_sent_body))
        memcpy(_sent_body, body, body_len);

    return true;

}

static bool _statemachine_info_returns_null = false;

static tcp_statemachine_info_t *_mock_get_statemachine_info(void)
{

    if (_statemachine_info_returns_null)
        return NULL;

    return &_mock_info;

}

static void _mock_on_link_drop_requested(void)
{

    _link_drop_requested = true;

}

static void _mock_on_link_status_changed(bool is_up)
{

    _link_status_changed_called = true;
    _link_status_is_up = is_up;

}

// =============================================================================
// Test setup
// =============================================================================

static const interface_tcp_link_control_t _interface = {

    .send_link_control       = &_mock_send_link_control,
    .get_statemachine_info   = &_mock_get_statemachine_info,
    .on_link_drop_requested  = &_mock_on_link_drop_requested,
    .on_link_status_changed  = &_mock_on_link_status_changed,

};

static void reset_mocks(void)
{

    _sent_flags = 0;
    _sent_body_len = 0;
    _send_count = 0;
    _send_returns_false = false;
    memset(_sent_body, 0, sizeof(_sent_body));

    _link_drop_requested = false;
    _link_status_changed_called = false;
    _link_status_is_up = false;

    _statemachine_info_returns_null = false;
    memset(&_mock_info, 0, sizeof(_mock_info));

}

static void setup_test(void)
{

    reset_mocks();
    TcpLinkControl_initialize(&_interface);

}

// =============================================================================
// Handle Status Request -- sets pending bit
// =============================================================================

TEST(TCP_LinkControl, handle_status_request_sets_pending_bit)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_STATUS_REQUEST);

    TcpLinkControl_handle(0x0000, body, 2);

    // Should NOT send immediately
    EXPECT_EQ(_send_count, 0);

    // Should set pending bit
    EXPECT_EQ(_mock_info.pending_status_reply, (uint8_t) 1);

}

// =============================================================================
// Handle Status Request + run() sends reply
// =============================================================================

TEST(TCP_LinkControl, run_sends_status_reply)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_STATUS_REQUEST);

    TcpLinkControl_handle(0x0000, body, 2);

    // Run flushes the pending reply
    bool busy = TcpLinkControl_run(&_mock_info);

    EXPECT_FALSE(busy);
    EXPECT_EQ(_send_count, 1);

    uint16_t reply_type = ((uint16_t) _sent_body[0] << 8) | _sent_body[1];
    EXPECT_EQ(reply_type, TCP_LINK_CONTROL_STATUS_REPLY);

    // Pending bit should be cleared
    EXPECT_EQ(_mock_info.pending_status_reply, (uint8_t) 0);

}

// =============================================================================
// Handle Status Reply -- calls on_link_status_changed(true)
// =============================================================================

TEST(TCP_LinkControl, handle_status_reply_notifies_link_up)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_STATUS_REPLY);

    TcpLinkControl_handle(0x0000, body, 2);

    EXPECT_TRUE(_link_status_changed_called);
    EXPECT_TRUE(_link_status_is_up);
    EXPECT_EQ(_send_count, 0);

}

// =============================================================================
// Handle Drop Link Request -- sets pending bit
// =============================================================================

TEST(TCP_LinkControl, handle_drop_link_request_sets_pending_bit)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_DROP_REQUEST);

    TcpLinkControl_handle(0x0000, body, 2);

    // Should NOT send immediately and NOT call drop callback
    EXPECT_EQ(_send_count, 0);
    EXPECT_FALSE(_link_drop_requested);

    // Should set pending bit
    EXPECT_EQ(_mock_info.pending_drop_reply, (uint8_t) 1);

}

// =============================================================================
// Handle Drop Link Request + run() sends reply and fires callback
// =============================================================================

TEST(TCP_LinkControl, run_sends_drop_reply_and_notifies)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_DROP_REQUEST);

    TcpLinkControl_handle(0x0000, body, 2);

    // Run flushes the pending reply
    bool busy = TcpLinkControl_run(&_mock_info);

    EXPECT_FALSE(busy);
    EXPECT_EQ(_send_count, 1);

    uint16_t reply_type = ((uint16_t) _sent_body[0] << 8) | _sent_body[1];
    EXPECT_EQ(reply_type, TCP_LINK_CONTROL_DROP_REPLY);

    // Pending bit should be cleared
    EXPECT_EQ(_mock_info.pending_drop_reply, (uint8_t) 0);

    // Drop callback fires only AFTER successful send
    EXPECT_TRUE(_link_drop_requested);

}

// =============================================================================
// run() retries status reply on send failure
// =============================================================================

TEST(TCP_LinkControl, run_retries_status_reply_on_send_failure)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_STATUS_REQUEST);
    TcpLinkControl_handle(0x0000, body, 2);

    // First run: send fails
    _send_returns_false = true;

    bool busy = TcpLinkControl_run(&_mock_info);

    EXPECT_TRUE(busy);
    EXPECT_EQ(_send_count, 0);
    EXPECT_EQ(_mock_info.pending_status_reply, (uint8_t) 1);

    // Second run: send succeeds
    _send_returns_false = false;

    busy = TcpLinkControl_run(&_mock_info);

    EXPECT_FALSE(busy);
    EXPECT_EQ(_send_count, 1);
    EXPECT_EQ(_mock_info.pending_status_reply, (uint8_t) 0);

}

// =============================================================================
// run() retries drop reply on send failure, callback fires only after send
// =============================================================================

TEST(TCP_LinkControl, run_retries_drop_reply_on_send_failure)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_DROP_REQUEST);
    TcpLinkControl_handle(0x0000, body, 2);

    // First run: send fails
    _send_returns_false = true;

    bool busy = TcpLinkControl_run(&_mock_info);

    EXPECT_TRUE(busy);
    EXPECT_EQ(_send_count, 0);
    EXPECT_EQ(_mock_info.pending_drop_reply, (uint8_t) 1);
    EXPECT_FALSE(_link_drop_requested);

    // Second run: send succeeds
    _send_returns_false = false;

    busy = TcpLinkControl_run(&_mock_info);

    EXPECT_FALSE(busy);
    EXPECT_EQ(_send_count, 1);
    EXPECT_EQ(_mock_info.pending_drop_reply, (uint8_t) 0);
    EXPECT_TRUE(_link_drop_requested);

}

// =============================================================================
// run() with NULL info returns false
// =============================================================================

TEST(TCP_LinkControl, run_null_info_returns_false)
{

    setup_test();

    bool busy = TcpLinkControl_run(NULL);

    EXPECT_FALSE(busy);

}

// =============================================================================
// run() with no pending returns false
// =============================================================================

TEST(TCP_LinkControl, run_no_pending_returns_false)
{

    setup_test();

    bool busy = TcpLinkControl_run(&_mock_info);

    EXPECT_FALSE(busy);
    EXPECT_EQ(_send_count, 0);

}

// =============================================================================
// Handle Drop Link Reply -- calls on_link_status_changed(false)
// =============================================================================

TEST(TCP_LinkControl, handle_drop_link_reply_notifies_link_down)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_DROP_REPLY);

    TcpLinkControl_handle(0x0000, body, 2);

    EXPECT_TRUE(_link_status_changed_called);
    EXPECT_FALSE(_link_status_is_up);
    EXPECT_EQ(_send_count, 0);

}

// =============================================================================
// Send Status Request
// =============================================================================

TEST(TCP_LinkControl, send_status_request)
{

    setup_test();

    bool result = TcpLinkControl_send_status_request();

    EXPECT_TRUE(result);
    EXPECT_EQ(_send_count, 1);
    EXPECT_EQ(_sent_body_len, 2);

    uint16_t type = ((uint16_t) _sent_body[0] << 8) | _sent_body[1];
    EXPECT_EQ(type, TCP_LINK_CONTROL_STATUS_REQUEST);

}

// =============================================================================
// Send Drop Link Request
// =============================================================================

TEST(TCP_LinkControl, send_drop_link_request)
{

    setup_test();

    bool result = TcpLinkControl_send_drop_link_request();

    EXPECT_TRUE(result);
    EXPECT_EQ(_send_count, 1);
    EXPECT_EQ(_sent_body_len, 2);

    uint16_t type = ((uint16_t) _sent_body[0] << 8) | _sent_body[1];
    EXPECT_EQ(type, TCP_LINK_CONTROL_DROP_REQUEST);

}

// =============================================================================
// Unknown Link Control Type -- Ignored
// =============================================================================

TEST(TCP_LinkControl, unknown_type_ignored)
{

    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, 0x00FF);

    TcpLinkControl_handle(0x0000, body, 2);

    EXPECT_EQ(_send_count, 0);
    EXPECT_FALSE(_link_drop_requested);
    EXPECT_FALSE(_link_status_changed_called);

}

// =============================================================================
// Body Too Short -- Ignored
// =============================================================================

TEST(TCP_LinkControl, body_too_short_ignored)
{

    setup_test();

    uint8_t body[1] = {0x00};

    TcpLinkControl_handle(0x0000, body, 1);

    EXPECT_EQ(_send_count, 0);
    EXPECT_FALSE(_link_drop_requested);
    EXPECT_FALSE(_link_status_changed_called);

}

// =============================================================================
// Empty Body -- Ignored
// =============================================================================

TEST(TCP_LinkControl, empty_body_ignored)
{

    setup_test();

    TcpLinkControl_handle(0x0000, NULL, 0);

    EXPECT_EQ(_send_count, 0);

}

// =============================================================================
// NULL Callbacks -- No Crash
// =============================================================================

TEST(TCP_LinkControl, null_callbacks_no_crash)
{

    static const interface_tcp_link_control_t interface_no_callbacks = {

        .send_link_control       = &_mock_send_link_control,
        .get_statemachine_info   = &_mock_get_statemachine_info,
        .on_link_drop_requested  = NULL,
        .on_link_status_changed  = NULL,

    };

    reset_mocks();
    TcpLinkControl_initialize(&interface_no_callbacks);

    // Status Reply with NULL on_link_status_changed -- should not crash
    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_STATUS_REPLY);
    TcpLinkControl_handle(0x0000, body, 2);
    EXPECT_EQ(_send_count, 0);

    // Drop Link Reply with NULL on_link_status_changed -- should not crash
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_DROP_REPLY);
    TcpLinkControl_handle(0x0000, body, 2);
    EXPECT_EQ(_send_count, 0);

    // Drop Link Request sets pending bit
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_DROP_REQUEST);
    TcpLinkControl_handle(0x0000, body, 2);
    EXPECT_EQ(_mock_info.pending_drop_reply, (uint8_t) 1);

    // run() sends reply with NULL on_link_drop_requested -- should not crash
    TcpLinkControl_run(&_mock_info);
    EXPECT_EQ(_send_count, 1);

}

// =============================================================================
// NULL Statemachine Info -- No Crash
// =============================================================================

TEST(TCP_LinkControl, null_statemachine_info_no_crash)
{

    setup_test();
    _statemachine_info_returns_null = true;

    // Status Request with NULL info -- should not crash or set pending bit
    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_STATUS_REQUEST);
    TcpLinkControl_handle(0x0000, body, 2);
    EXPECT_EQ(_send_count, 0);

    // Drop Link Request with NULL info -- should not crash or set pending bit
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_DROP_REQUEST);
    TcpLinkControl_handle(0x0000, body, 2);
    EXPECT_EQ(_send_count, 0);

}
