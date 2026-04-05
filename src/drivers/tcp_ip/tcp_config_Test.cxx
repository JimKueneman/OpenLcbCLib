/*******************************************************************************
 * File: tcp_config_Test.cxx
 *
 * Description:
 *   Test suite for TCP Config — verifies that TcpConfig_initialize() correctly
 *   wires all sub-modules and that the facade functions delegate properly.
 *
 * Module Under Test:
 *   TcpConfig — user-facing DI entry point and internal wiring
 *
 * Test Coverage:
 *   - TcpConfig_initialize completes without crash
 *   - TcpConfig_get_send_openlcb_msg returns a non-NULL function pointer
 *   - TcpConfig_link_up transitions main statemachine to RUNNING
 *   - TcpConfig_link_down transitions main statemachine to DISCONNECTED
 *   - TcpConfig_incoming_data routes to RX statemachine (messages pushed to FIFO)
 *   - End-to-end TX: send via returned function pointer → mock transmit called
 *
 * Author: Test Suite
 * Date: 2026-04-05
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_config.h"
#include "tcp_main_statemachine.h"
#include "tcp_utilities.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_defines.h"

// =============================================================================
// Mock state
// =============================================================================

static bool _transmit_called = false;
static uint8_t _transmitted_data[1024];
static uint16_t _transmitted_len = 0;
static bool _tx_buffer_clear = true;

static bool _link_status_called = false;
static bool _link_status_is_up = false;

static bool _drop_requested = false;

// =============================================================================
// Mock functions
// =============================================================================

static bool _mock_transmit(uint8_t *data, uint16_t len)
{
    _transmit_called = true;
    _transmitted_len = len;
    if (len <= sizeof(_transmitted_data))
        memcpy(_transmitted_data, data, len);
    return true;
}

static bool _mock_is_tx_clear(void)
{
    return _tx_buffer_clear;
}

static node_id_t _mock_get_node_id(void)
{
    return 0x050101012200ULL;
}

static uint64_t _mock_get_time(void)
{
    return 1000;
}

static void _mock_lock(void) {}
static void _mock_unlock(void) {}

static void _mock_on_link_status(bool is_up)
{
    _link_status_called = true;
    _link_status_is_up = is_up;
}

static void _mock_on_drop(void)
{
    _drop_requested = true;
}

// =============================================================================
// Test setup
// =============================================================================

static const tcp_config_t _tcp_config = {
    .transmit_raw_tcp_data   = &_mock_transmit,
    .is_tx_buffer_clear      = &_mock_is_tx_clear,
    .get_local_node_id       = &_mock_get_node_id,
    .get_capture_time_ms     = &_mock_get_time,
    .lock_shared_resources   = &_mock_lock,
    .unlock_shared_resources = &_mock_unlock,
    .on_rx                   = NULL,
    .on_tx                   = NULL,
    .on_link_status_changed  = &_mock_on_link_status,
    .on_link_drop_requested  = &_mock_on_drop,
};

static void reset_mocks(void)
{
    _transmit_called = false;
    _transmitted_len = 0;
    _tx_buffer_clear = true;
    memset(_transmitted_data, 0, sizeof(_transmitted_data));
    _link_status_called = false;
    _link_status_is_up = false;
    _drop_requested = false;
}

static void setup_test(void)
{
    reset_mocks();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    TcpConfig_initialize(&_tcp_config);
}

// =============================================================================
// Initialize
// =============================================================================

TEST(TCP_Config, initialize_succeeds)
{
    setup_test();

    // No crash = pass. Verify main statemachine is in disconnected state.
    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_DISCONNECTED);
}

// =============================================================================
// get_send_openlcb_msg
// =============================================================================

TEST(TCP_Config, get_send_openlcb_msg_returns_function_pointer)
{
    setup_test();

    auto fn = TcpConfig_get_send_openlcb_msg();
    EXPECT_NE(fn, nullptr);
}

// =============================================================================
// End-to-end TX via returned function pointer
// =============================================================================

TEST(TCP_Config, send_via_function_pointer)
{
    setup_test();

    auto send_fn = TcpConfig_get_send_openlcb_msg();
    ASSERT_NE(send_fn, nullptr);

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0490;  // Verify Node ID Global
    msg->source_id = 0x010203040506ULL;
    msg->dest_id = 0;
    msg->payload_count = 0;

    bool result = send_fn(msg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_transmit_called);
    EXPECT_GT(_transmitted_len, 0);

    // Verify preamble flags
    uint16_t flags = TcpUtilities_decode_flags(_transmitted_data);
    EXPECT_TRUE(TcpUtilities_is_openlcb_message(flags));

    OpenLcbBufferStore_free_buffer(msg);
}

// =============================================================================
// Link Up / Down
// =============================================================================

TEST(TCP_Config, link_up_transitions_to_running)
{
    setup_test();

    TcpConfig_link_up();

    // link_up transitions to LOGGING_IN
    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_LOGGING_IN);
    EXPECT_TRUE(_link_status_called);
    EXPECT_TRUE(_link_status_is_up);

    // run() drives login to completion, transitions to RUNNING
    TcpConfig_run();

    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_RUNNING);
}

TEST(TCP_Config, link_down_transitions_to_disconnected)
{
    setup_test();

    TcpConfig_link_up();
    reset_mocks();

    TcpConfig_link_down();

    EXPECT_EQ(TcpMainStatemachine_get_link_state(), TCP_LINK_STATE_DISCONNECTED);
    EXPECT_TRUE(_link_status_called);
    EXPECT_FALSE(_link_status_is_up);
}

// =============================================================================
// Incoming Data — routes to RX statemachine
// =============================================================================

TEST(TCP_Config, incoming_data_parses_message)
{
    setup_test();

    // Build a complete wire message
    uint8_t wire[64];
    uint16_t body_len = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN;
    uint16_t offset = TcpUtilities_encode_preamble(
            wire, TCP_FLAGS_MESSAGE, body_len, 0x050101012200ULL, 500);
    offset += TcpUtilities_encode_uint16(&wire[offset], 0x0490);
    offset += TcpUtilities_encode_node_id(&wire[offset], 0x010203040506ULL);

    TcpConfig_incoming_data(wire, offset);

    // The message should have been pushed to the OpenLCB FIFO
    openlcb_msg_t *msg = OpenLcbBufferFifo_pop();
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(msg->mti, 0x0490);
    EXPECT_EQ(msg->source_id, 0x010203040506ULL);

    OpenLcbBufferStore_free_buffer(msg);
}

// =============================================================================
// Link Up sends Verify Node ID Global
// =============================================================================

TEST(TCP_Config, link_up_sends_verify_node_id_global)
{
    setup_test();

    TcpConfig_link_up();

    // link_up does not send immediately
    EXPECT_FALSE(_transmit_called);

    // run() drives the login statemachine which sends Verify Node ID Global
    TcpConfig_run();

    EXPECT_TRUE(_transmit_called);

    // Verify the transmitted message has MTI_VERIFY_NODE_ID_GLOBAL
    uint16_t mti = TcpUtilities_decode_mti(&_transmitted_data[TCP_PREAMBLE_LEN]);
    EXPECT_EQ(mti, MTI_VERIFY_NODE_ID_GLOBAL);
}
