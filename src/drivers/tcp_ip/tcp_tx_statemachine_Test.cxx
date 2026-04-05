/*******************************************************************************
 * File: tcp_tx_statemachine_Test.cxx
 *
 * Description:
 *   Test suite for TCP TX State Machine — verifies that openlcb_msg_t messages
 *   are correctly converted to TCP/IP wire format (preamble + body) and that
 *   link control messages are built correctly.
 *
 * Module Under Test:
 *   TcpTxStatemachine — builds TCP wire format from OpenLCB messages
 *
 * Test Coverage:
 *   - Unaddressed message encoding (no dest Node ID)
 *   - Addressed message encoding (with dest Node ID)
 *   - Payload data included in wire output
 *   - Preamble flags, length, originating Node ID, capture time
 *   - TX buffer busy rejection
 *   - Link control message encoding
 *   - on_tx callback invocation
 *
 * Author: Test Suite
 * Date: 2026-04-05
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_tx_statemachine.h"
#include "tcp_utilities.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_defines.h"

// =============================================================================
// Mock state
// =============================================================================

static bool _tx_buffer_clear = true;
static bool _transmit_should_fail = false;
static uint8_t _transmitted_data[1024];
static uint16_t _transmitted_len = 0;
static bool _transmit_called = false;

static node_id_t _local_node_id = 0x050101012200ULL;
static uint64_t _capture_time = 1000;

static uint8_t _on_tx_data[1024];
static uint16_t _on_tx_len = 0;
static bool _on_tx_called = false;

// =============================================================================
// Mock functions
// =============================================================================

static bool _mock_transmit(uint8_t *data, uint16_t len)
{
    _transmit_called = true;
    _transmitted_len = len;
    if (len <= sizeof(_transmitted_data))
        memcpy(_transmitted_data, data, len);
    return !_transmit_should_fail;
}

static bool _mock_is_tx_buffer_clear(void)
{
    return _tx_buffer_clear;
}

static node_id_t _mock_get_local_node_id(void)
{
    return _local_node_id;
}

static uint64_t _mock_get_capture_time_ms(void)
{
    return _capture_time;
}

static void _mock_on_tx(uint8_t *data, uint16_t len)
{
    _on_tx_called = true;
    _on_tx_len = len;
    if (len <= sizeof(_on_tx_data))
        memcpy(_on_tx_data, data, len);
}

// =============================================================================
// Test setup
// =============================================================================

static const interface_tcp_tx_statemachine_t _interface = {
    .transmit_raw_tcp_data = &_mock_transmit,
    .is_tx_buffer_clear    = &_mock_is_tx_buffer_clear,
    .get_local_node_id     = &_mock_get_local_node_id,
    .get_capture_time_ms   = &_mock_get_capture_time_ms,
    .on_tx                 = &_mock_on_tx,
};

static const interface_tcp_tx_statemachine_t _interface_no_on_tx = {
    .transmit_raw_tcp_data = &_mock_transmit,
    .is_tx_buffer_clear    = &_mock_is_tx_buffer_clear,
    .get_local_node_id     = &_mock_get_local_node_id,
    .get_capture_time_ms   = &_mock_get_capture_time_ms,
    .on_tx                 = NULL,
};

static void reset_mocks(void)
{
    _tx_buffer_clear = true;
    _transmit_should_fail = false;
    _transmitted_len = 0;
    _transmit_called = false;
    memset(_transmitted_data, 0, sizeof(_transmitted_data));

    _on_tx_called = false;
    _on_tx_len = 0;
    memset(_on_tx_data, 0, sizeof(_on_tx_data));

    _local_node_id = 0x050101012200ULL;
    _capture_time = 1000;
}

static void setup_test(void)
{
    reset_mocks();
    OpenLcbBufferStore_initialize();
    TcpTxStatemachine_initialize(&_interface);
}

// =============================================================================
// Send OpenLCB Message — Unaddressed
// =============================================================================

TEST(TCP_TxStatemachine, send_unaddressed_message)
{
    setup_test();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0490;  // Verify Node ID Global (unaddressed)
    msg->source_id = 0x010203040506ULL;
    msg->dest_id = 0;
    msg->payload_count = 0;

    bool result = TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_transmit_called);

    // Total: 17 preamble + 2 MTI + 6 source = 25
    EXPECT_EQ(_transmitted_len, 25);

    // Verify flags: bit 15 set (OpenLCB message)
    uint16_t flags = TcpUtilities_decode_flags(_transmitted_data);
    EXPECT_EQ(flags, TCP_FLAGS_MESSAGE);

    // Verify length field: body_len = 2 + 6 = 8, encoded = 8 + 12 = 20
    uint32_t length = TcpUtilities_decode_length(_transmitted_data);
    EXPECT_EQ(length, 20u);

    // Verify originating Node ID
    node_id_t orig = TcpUtilities_decode_originating_node_id(_transmitted_data);
    EXPECT_EQ(orig, _local_node_id);

    // Verify capture time
    uint64_t capture = TcpUtilities_decode_capture_time(_transmitted_data);
    EXPECT_EQ(capture, _capture_time);

    // Verify MTI in body
    uint16_t mti = TcpUtilities_decode_mti(&_transmitted_data[TCP_PREAMBLE_LEN]);
    EXPECT_EQ(mti, 0x0490);

    // Verify source Node ID in body
    node_id_t src = TcpUtilities_decode_node_id(
            &_transmitted_data[TCP_PREAMBLE_LEN + TCP_BODY_MTI_LEN]);
    EXPECT_EQ(src, 0x010203040506ULL);

    OpenLcbBufferStore_free_buffer(msg);
}

// =============================================================================
// Send OpenLCB Message — Addressed (with dest Node ID)
// =============================================================================

TEST(TCP_TxStatemachine, send_addressed_message)
{
    setup_test();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0828;  // Protocol Support Inquiry (addressed, MASK_DEST_ADDRESS_PRESENT)
    msg->source_id = 0x010203040506ULL;
    msg->dest_id = 0x0A0B0C0D0E0FULL;
    msg->payload_count = 0;

    bool result = TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_TRUE(result);

    // Total: 17 preamble + 2 MTI + 6 source + 6 dest = 31
    EXPECT_EQ(_transmitted_len, 31);

    // Verify dest Node ID in body
    node_id_t dest = TcpUtilities_decode_node_id(
            &_transmitted_data[TCP_PREAMBLE_LEN + TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN]);
    EXPECT_EQ(dest, 0x0A0B0C0D0E0FULL);

    OpenLcbBufferStore_free_buffer(msg);
}

// =============================================================================
// Send OpenLCB Message — With Payload
// =============================================================================

TEST(TCP_TxStatemachine, send_message_with_payload)
{
    setup_test();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0100;  // Initialization Complete (unaddressed)
    msg->source_id = 0x010203040506ULL;
    msg->dest_id = 0;

    // 6-byte payload (node ID)
    uint8_t payload_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    memcpy(msg->payload, payload_data, 6);
    msg->payload_count = 6;

    bool result = TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_TRUE(result);

    // Total: 17 preamble + 2 MTI + 6 source + 6 payload = 31
    EXPECT_EQ(_transmitted_len, 31);

    // Verify payload bytes at the end
    uint16_t payload_offset = TCP_PREAMBLE_LEN + TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN;
    EXPECT_EQ(memcmp(&_transmitted_data[payload_offset], payload_data, 6), 0);

    OpenLcbBufferStore_free_buffer(msg);
}

// =============================================================================
// TX Buffer Busy
// =============================================================================

TEST(TCP_TxStatemachine, tx_buffer_busy_returns_false)
{
    setup_test();
    _tx_buffer_clear = false;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0490;
    msg->source_id = 0x010203040506ULL;
    msg->payload_count = 0;

    bool result = TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(_transmit_called);

    OpenLcbBufferStore_free_buffer(msg);
}

// =============================================================================
// on_tx Callback
// =============================================================================

TEST(TCP_TxStatemachine, on_tx_callback_invoked)
{
    setup_test();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0490;
    msg->source_id = 0x010203040506ULL;
    msg->payload_count = 0;

    TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_TRUE(_on_tx_called);
    EXPECT_EQ(_on_tx_len, _transmitted_len);

    OpenLcbBufferStore_free_buffer(msg);
}

// =============================================================================
// Link Control Message
// =============================================================================

TEST(TCP_TxStatemachine, send_link_control_status_request)
{
    setup_test();

    uint8_t body[2];
    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_STATUS_REQUEST);

    bool result = TcpTxStatemachine_send_link_control(0x0000, body, 2);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_transmit_called);

    // Total: 17 preamble + 2 body = 19
    EXPECT_EQ(_transmitted_len, 19);

    // Verify flags: bit 15 clear (link control)
    uint16_t flags = TcpUtilities_decode_flags(_transmitted_data);
    EXPECT_FALSE(TcpUtilities_is_openlcb_message(flags));

    // Verify body contains status request type
    EXPECT_EQ(_transmitted_data[TCP_PREAMBLE_LEN], 0x00);
    EXPECT_EQ(_transmitted_data[TCP_PREAMBLE_LEN + 1], 0x00);
}

TEST(TCP_TxStatemachine, send_link_control_busy_returns_false)
{
    setup_test();
    _tx_buffer_clear = false;

    uint8_t body[2] = {0x00, 0x00};

    bool result = TcpTxStatemachine_send_link_control(0x0000, body, 2);

    EXPECT_FALSE(result);
    EXPECT_FALSE(_transmit_called);
}

// =============================================================================
// Buffer Overflow Guards
// =============================================================================

TEST(TCP_TxStatemachine, send_message_overflow_returns_false)
{
    setup_test();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0490;
    msg->source_id = 0x010203040506ULL;
    msg->payload_count = 1008;  // total = 17 + 2 + 6 + 1008 = 1033 > 1024

    bool result = TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_FALSE(result);
    EXPECT_FALSE(_transmit_called);

    OpenLcbBufferStore_free_buffer(msg);
}

TEST(TCP_TxStatemachine, send_link_control_overflow_returns_false)
{
    setup_test();

    bool result = TcpTxStatemachine_send_link_control(0x0000, NULL, 1008);

    EXPECT_FALSE(result);
    EXPECT_FALSE(_transmit_called);
}

// =============================================================================
// Payload / Body Edge Cases
// =============================================================================

TEST(TCP_TxStatemachine, send_message_null_payload_with_nonzero_count_skips_copy)
{
    setup_test();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0490;
    msg->source_id = 0x010203040506ULL;
    msg->payload_count = 4;
    msg->payload = NULL;

    bool result = TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_transmit_called);
    EXPECT_EQ(_transmitted_len, 25);  // 17 + 2 + 6, payload not copied

    OpenLcbBufferStore_free_buffer(msg);
}

TEST(TCP_TxStatemachine, send_link_control_null_body_with_nonzero_len_skips_copy)
{
    setup_test();

    bool result = TcpTxStatemachine_send_link_control(0x0000, NULL, 2);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_transmit_called);
    EXPECT_EQ(_transmitted_len, 17);  // preamble only
}

TEST(TCP_TxStatemachine, send_link_control_empty_body)
{
    setup_test();

    bool result = TcpTxStatemachine_send_link_control(0x0000, NULL, 0);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_transmit_called);
    EXPECT_EQ(_transmitted_len, 17);  // preamble only
}

// =============================================================================
// Transmit Failure
// =============================================================================

TEST(TCP_TxStatemachine, send_message_transmit_failure_skips_on_tx)
{
    setup_test();
    _transmit_should_fail = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0490;
    msg->source_id = 0x010203040506ULL;
    msg->payload_count = 0;

    bool result = TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_FALSE(result);
    EXPECT_TRUE(_transmit_called);
    EXPECT_FALSE(_on_tx_called);

    OpenLcbBufferStore_free_buffer(msg);
}

TEST(TCP_TxStatemachine, send_link_control_transmit_failure_skips_on_tx)
{
    setup_test();
    _transmit_should_fail = true;

    uint8_t body[2] = {0x00, 0x00};

    bool result = TcpTxStatemachine_send_link_control(0x0000, body, 2);

    EXPECT_FALSE(result);
    EXPECT_TRUE(_transmit_called);
    EXPECT_FALSE(_on_tx_called);
}

// =============================================================================
// on_tx NULL Callback
// =============================================================================

TEST(TCP_TxStatemachine, send_message_on_tx_null_does_not_crash)
{
    reset_mocks();
    OpenLcbBufferStore_initialize();
    TcpTxStatemachine_initialize(&_interface_no_on_tx);

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    msg->mti = 0x0490;
    msg->source_id = 0x010203040506ULL;
    msg->payload_count = 0;

    bool result = TcpTxStatemachine_send_openlcb_message(msg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_transmit_called);
    EXPECT_FALSE(_on_tx_called);

    OpenLcbBufferStore_free_buffer(msg);
}

TEST(TCP_TxStatemachine, send_link_control_on_tx_null_does_not_crash)
{
    reset_mocks();
    OpenLcbBufferStore_initialize();
    TcpTxStatemachine_initialize(&_interface_no_on_tx);

    uint8_t body[2] = {0x00, 0x00};

    bool result = TcpTxStatemachine_send_link_control(0x0000, body, 2);

    EXPECT_TRUE(result);
    EXPECT_TRUE(_transmit_called);
    EXPECT_FALSE(_on_tx_called);
}
