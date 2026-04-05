/*******************************************************************************
 * File: tcp_rx_statemachine_Test.cxx
 *
 * Description:
 *   Test suite for TCP RX State Machine — verifies that raw byte streams are
 *   correctly parsed into openlcb_msg_t messages and pushed to the FIFO.
 *
 * Module Under Test:
 *   TcpRxStatemachine — accumulates bytes, parses preambles, reassembles
 *   multi-part messages, routes link control to handler
 *
 * Test Coverage:
 *   - Single complete unaddressed message
 *   - Single complete addressed message
 *   - Message with payload
 *   - Partial delivery (bytes arrive in chunks)
 *   - Two messages back-to-back in one buffer
 *   - Link control message routing
 *   - Multi-part message reassembly (first + last)
 *   - Buffer overflow reset
 *   - on_rx callback invocation
 *
 * Author: Test Suite
 * Date: 2026-04-05
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_rx_statemachine.h"
#include "tcp_utilities.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_defines.h"

// =============================================================================
// Mock state
// =============================================================================

static openlcb_msg_t *_last_pushed_msg = NULL;
static int _push_count = 0;

static uint16_t _link_control_flags = 0;
static uint8_t _link_control_data[64];
static uint16_t _link_control_len = 0;
static bool _link_control_called = false;

static bool _on_rx_called = false;
static uint16_t _on_rx_len = 0;

static bool _mock_allocate_returns_null = false;

// =============================================================================
// Mock functions
// =============================================================================

static openlcb_msg_t *_mock_allocate_buffer(payload_type_enum payload_type)
{
    if (_mock_allocate_returns_null)
        return NULL;
    return OpenLcbBufferStore_allocate_buffer(payload_type);
}

static void _mock_free_buffer(openlcb_msg_t *msg)
{
    OpenLcbBufferStore_free_buffer(msg);
}

static openlcb_msg_t *_mock_push_to_fifo(openlcb_msg_t *msg)
{
    _last_pushed_msg = msg;
    _push_count++;
    return msg;
}

static void _mock_handle_link_control(uint16_t flags, const uint8_t *data, uint16_t len)
{
    _link_control_called = true;
    _link_control_flags = flags;
    _link_control_len = len;
    if (len > 0 && len <= sizeof(_link_control_data))
        memcpy(_link_control_data, data, len);
}

static void _mock_lock(void) {}
static void _mock_unlock(void) {}

static void _mock_on_rx(uint8_t *data, uint16_t len)
{
    _on_rx_called = true;
    _on_rx_len = len;
}

// =============================================================================
// Test setup
// =============================================================================

static const interface_tcp_rx_statemachine_t _interface = {
    .allocate_buffer       = &_mock_allocate_buffer,
    .free_buffer           = &_mock_free_buffer,
    .push_to_fifo          = &_mock_push_to_fifo,
    .handle_link_control   = &_mock_handle_link_control,
    .lock_shared_resources = &_mock_lock,
    .unlock_shared_resources = &_mock_unlock,
    .on_rx                 = &_mock_on_rx,
};

static void reset_mocks(void)
{
    _last_pushed_msg = NULL;
    _push_count = 0;
    _link_control_called = false;
    _link_control_flags = 0;
    _link_control_len = 0;
    _on_rx_called = false;
    _on_rx_len = 0;
    _mock_allocate_returns_null = false;
    memset(_link_control_data, 0, sizeof(_link_control_data));
}

static void setup_test(void)
{
    reset_mocks();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    TcpRxStatemachine_initialize(&_interface);
}

/**
 * @brief Builds a complete TCP wire message (preamble + body) for an
 * unaddressed OpenLCB message with no payload.
 *
 * @return total bytes written to buf.
 */
static uint16_t build_unaddressed_msg(uint8_t *buf, uint16_t mti, node_id_t source_id)
{
    // body = 2 (MTI) + 6 (source) = 8
    uint16_t body_len = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN;
    uint16_t offset = TcpUtilities_encode_preamble(
            buf, TCP_FLAGS_MESSAGE, body_len, 0x050101012200ULL, 500);
    offset += TcpUtilities_encode_uint16(&buf[offset], mti);
    offset += TcpUtilities_encode_node_id(&buf[offset], source_id);
    return offset;
}

/**
 * @brief Builds a complete TCP wire message for an addressed OpenLCB message.
 */
static uint16_t build_addressed_msg(uint8_t *buf, uint16_t mti,
                                     node_id_t source_id, node_id_t dest_id,
                                     const uint8_t *payload, uint16_t payload_len)
{
    uint16_t body_len = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN + TCP_BODY_NODE_ID_LEN + payload_len;
    uint16_t offset = TcpUtilities_encode_preamble(
            buf, TCP_FLAGS_MESSAGE, body_len, 0x050101012200ULL, 500);
    offset += TcpUtilities_encode_uint16(&buf[offset], mti);
    offset += TcpUtilities_encode_node_id(&buf[offset], source_id);
    offset += TcpUtilities_encode_node_id(&buf[offset], dest_id);
    if (payload_len > 0 && payload) {
        memcpy(&buf[offset], payload, payload_len);
        offset += payload_len;
    }
    return offset;
}

/**
 * @brief Builds a multipart FIRST wire message with an addressed body.
 */
static uint16_t build_multipart_first_msg(uint8_t *buf, node_id_t orig_id,
    uint16_t mti, node_id_t source_id, node_id_t dest_id,
    const uint8_t *payload, uint16_t payload_len)
{
    uint16_t body_len = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN +
                         TCP_BODY_NODE_ID_LEN + payload_len;
    uint16_t offset = TcpUtilities_encode_preamble(
            buf, TCP_FLAGS_MESSAGE | TCP_FLAGS_MULTIPART_FIRST,
            body_len, orig_id, 500);
    offset += TcpUtilities_encode_uint16(&buf[offset], mti);
    offset += TcpUtilities_encode_node_id(&buf[offset], source_id);
    offset += TcpUtilities_encode_node_id(&buf[offset], dest_id);
    if (payload_len > 0 && payload) {
        memcpy(&buf[offset], payload, payload_len);
        offset += payload_len;
    }
    return offset;
}

/**
 * @brief Builds a multipart MIDDLE or LAST wire message (body-only, no MTI/NodeIDs).
 */
static uint16_t build_multipart_continuation_msg(uint8_t *buf,
    uint16_t multipart_flags, node_id_t orig_id,
    const uint8_t *payload, uint16_t payload_len)
{
    uint16_t offset = TcpUtilities_encode_preamble(
            buf, TCP_FLAGS_MESSAGE | multipart_flags,
            payload_len, orig_id, 600);
    if (payload_len > 0 && payload) {
        memcpy(&buf[offset], payload, payload_len);
        offset += payload_len;
    }
    return offset;
}

// =============================================================================
// Single Complete Unaddressed Message
// =============================================================================

TEST(TCP_RxStatemachine, receive_unaddressed_message)
{
    setup_test();

    uint8_t wire[64];
    uint16_t wire_len = build_unaddressed_msg(wire, 0x0490, 0x010203040506ULL);

    TcpRxStatemachine_incoming_data(wire, wire_len);

    EXPECT_EQ(_push_count, 1);
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->mti, 0x0490);
    EXPECT_EQ(_last_pushed_msg->source_id, 0x010203040506ULL);
    EXPECT_EQ(_last_pushed_msg->dest_id, (node_id_t)0);
    EXPECT_EQ(_last_pushed_msg->payload_count, 0);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Single Complete Addressed Message
// =============================================================================

TEST(TCP_RxStatemachine, receive_addressed_message)
{
    setup_test();

    uint8_t wire[64];
    uint16_t wire_len = build_addressed_msg(
            wire, 0x0828, 0x010203040506ULL, 0x0A0B0C0D0E0FULL, NULL, 0);

    TcpRxStatemachine_incoming_data(wire, wire_len);

    EXPECT_EQ(_push_count, 1);
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->mti, 0x0828);
    EXPECT_EQ(_last_pushed_msg->source_id, 0x010203040506ULL);
    EXPECT_EQ(_last_pushed_msg->dest_id, 0x0A0B0C0D0E0FULL);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Message With Payload
// =============================================================================

TEST(TCP_RxStatemachine, receive_message_with_payload)
{
    setup_test();

    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t wire[64];
    uint16_t wire_len = build_addressed_msg(
            wire, 0x1C48, 0x010203040506ULL, 0x0A0B0C0D0E0FULL,
            payload, 4);

    TcpRxStatemachine_incoming_data(wire, wire_len);

    EXPECT_EQ(_push_count, 1);
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->payload_count, 4);
    EXPECT_EQ(memcmp(_last_pushed_msg->payload, payload, 4), 0);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Partial Delivery (bytes arrive in chunks)
// =============================================================================

TEST(TCP_RxStatemachine, partial_delivery)
{
    setup_test();

    uint8_t wire[64];
    uint16_t wire_len = build_unaddressed_msg(wire, 0x0490, 0x010203040506ULL);

    // Feed first 10 bytes — not enough for a complete message
    TcpRxStatemachine_incoming_data(wire, 10);
    EXPECT_EQ(_push_count, 0);

    // Feed remaining bytes
    TcpRxStatemachine_incoming_data(&wire[10], wire_len - 10);
    EXPECT_EQ(_push_count, 1);
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->mti, 0x0490);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Two Messages Back-to-Back
// =============================================================================

TEST(TCP_RxStatemachine, two_messages_in_one_buffer)
{
    setup_test();

    uint8_t wire[128];
    uint16_t offset = build_unaddressed_msg(wire, 0x0490, 0x010203040506ULL);
    offset += build_unaddressed_msg(&wire[offset], 0x0100, 0xAABBCCDDEEFFULL);

    TcpRxStatemachine_incoming_data(wire, offset);

    EXPECT_EQ(_push_count, 2);

    // The last pushed message should be the second one
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->mti, 0x0100);
    EXPECT_EQ(_last_pushed_msg->source_id, 0xAABBCCDDEEFFULL);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Link Control Message Routing
// =============================================================================

TEST(TCP_RxStatemachine, link_control_routed_to_handler)
{
    setup_test();

    uint8_t wire[32];
    // Link control: flags bit 15 = 0, body = status request type (0x0000)
    uint16_t body_len = 2;
    uint16_t offset = TcpUtilities_encode_preamble(
            wire, 0x0000, body_len, 0x050101012200ULL, 500);
    TcpUtilities_encode_uint16(&wire[offset], TCP_LINK_CONTROL_STATUS_REQUEST);
    offset += 2;

    TcpRxStatemachine_incoming_data(wire, offset);

    EXPECT_TRUE(_link_control_called);
    EXPECT_EQ(_link_control_len, 2);
    EXPECT_EQ(_push_count, 0);  // Link control should NOT push to FIFO
}

// =============================================================================
// on_rx Callback
// =============================================================================

TEST(TCP_RxStatemachine, on_rx_callback_invoked)
{
    setup_test();

    uint8_t wire[64];
    uint16_t wire_len = build_unaddressed_msg(wire, 0x0490, 0x010203040506ULL);

    TcpRxStatemachine_incoming_data(wire, wire_len);

    EXPECT_TRUE(_on_rx_called);
    EXPECT_EQ(_on_rx_len, wire_len);

    if (_last_pushed_msg)
        OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Reset clears state
// =============================================================================

TEST(TCP_RxStatemachine, reset_clears_accumulation)
{
    setup_test();

    uint8_t wire[64];
    uint16_t wire_len = build_unaddressed_msg(wire, 0x0490, 0x010203040506ULL);

    // Feed partial data
    TcpRxStatemachine_incoming_data(wire, 10);
    EXPECT_EQ(_push_count, 0);

    // Reset
    TcpRxStatemachine_reset();

    // Feed complete message fresh
    TcpRxStatemachine_incoming_data(wire, wire_len);
    EXPECT_EQ(_push_count, 1);

    if (_last_pushed_msg)
        OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Multi-part Message Reassembly
// =============================================================================

TEST(TCP_RxStatemachine, multipart_first_and_last)
{
    setup_test();

    node_id_t orig_id = 0x050101012200ULL;
    node_id_t source_id = 0x010203040506ULL;
    node_id_t dest_id = 0x0A0B0C0D0E0FULL;

    // --- Build FIRST part ---
    uint8_t first_wire[128];
    uint8_t first_payload[] = {0x01, 0x02, 0x03, 0x04};
    // body: MTI(2) + source(6) + dest(6) + payload(4) = 18
    uint16_t first_body_len = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN +
                               TCP_BODY_NODE_ID_LEN + 4;
    uint16_t offset = TcpUtilities_encode_preamble(
            first_wire,
            TCP_FLAGS_MESSAGE | TCP_FLAGS_MULTIPART_FIRST,
            first_body_len, orig_id, 500);
    offset += TcpUtilities_encode_uint16(&first_wire[offset], 0x1C48);  // addressed MTI
    offset += TcpUtilities_encode_node_id(&first_wire[offset], source_id);
    offset += TcpUtilities_encode_node_id(&first_wire[offset], dest_id);
    memcpy(&first_wire[offset], first_payload, 4);
    offset += 4;

    TcpRxStatemachine_incoming_data(first_wire, offset);
    EXPECT_EQ(_push_count, 0);  // Not yet complete

    // --- Build LAST part ---
    uint8_t last_wire[64];
    uint8_t last_payload[] = {0x05, 0x06, 0x07, 0x08};
    uint16_t last_body_len = 4;
    uint16_t offset2 = TcpUtilities_encode_preamble(
            last_wire,
            TCP_FLAGS_MESSAGE | TCP_FLAGS_MULTIPART_LAST,
            last_body_len, orig_id, 600);
    memcpy(&last_wire[offset2], last_payload, 4);
    offset2 += 4;

    TcpRxStatemachine_incoming_data(last_wire, offset2);
    EXPECT_EQ(_push_count, 1);

    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->mti, 0x1C48);
    EXPECT_EQ(_last_pushed_msg->source_id, source_id);
    EXPECT_EQ(_last_pushed_msg->dest_id, dest_id);
    EXPECT_EQ(_last_pushed_msg->payload_count, 8);

    // Verify combined payload
    uint8_t expected[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    EXPECT_EQ(memcmp(_last_pushed_msg->payload, expected, 8), 0);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Body too short — silently dropped
// =============================================================================

TEST(TCP_RxStatemachine, body_too_short_dropped)
{
    setup_test();

    uint8_t wire[32];
    // body_len = 4 (less than minimum 8: MTI + source NodeID)
    uint16_t offset = TcpUtilities_encode_preamble(
            wire, TCP_FLAGS_MESSAGE, 4, 0x050101012200ULL, 500);
    // Write only 4 body bytes (incomplete)
    wire[offset++] = 0x04;
    wire[offset++] = 0x90;
    wire[offset++] = 0x00;
    wire[offset++] = 0x00;

    TcpRxStatemachine_incoming_data(wire, offset);

    EXPECT_EQ(_push_count, 0);  // Dropped, not pushed
}

// =============================================================================
// Datagram-size payload (17-72 bytes) — covers _select_payload_type DATAGRAM
// =============================================================================

TEST(TCP_RxStatemachine, receive_datagram_size_payload)
{
    setup_test();

    uint8_t payload[20];
    memset(payload, 0xAB, sizeof(payload));

    uint8_t wire[128];
    uint16_t wire_len = build_addressed_msg(
            wire, 0x1C48, 0x010203040506ULL, 0x0A0B0C0D0E0FULL,
            payload, 20);

    TcpRxStatemachine_incoming_data(wire, wire_len);

    EXPECT_EQ(_push_count, 1);
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->payload_count, 20);
    EXPECT_EQ(memcmp(_last_pushed_msg->payload, payload, 20), 0);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// SNIP-size payload (73-256 bytes) — covers _select_payload_type SNIP
// =============================================================================

TEST(TCP_RxStatemachine, receive_snip_size_payload)
{
    setup_test();

    uint8_t payload[100];
    memset(payload, 0xCD, sizeof(payload));

    uint8_t wire[256];
    uint16_t wire_len = build_addressed_msg(
            wire, 0x1C48, 0x010203040506ULL, 0x0A0B0C0D0E0FULL,
            payload, 100);

    TcpRxStatemachine_incoming_data(wire, wire_len);

    EXPECT_EQ(_push_count, 1);
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->payload_count, 100);
    EXPECT_EQ(memcmp(_last_pushed_msg->payload, payload, 100), 0);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// STREAM-size payload (>256 bytes) — covers _select_payload_type STREAM +
// truncation to LEN_MESSAGE_BYTES_STREAM (256)
// =============================================================================

TEST(TCP_RxStatemachine, receive_stream_size_payload_truncated)
{
    setup_test();

    uint8_t payload[260];
    memset(payload, 0xEF, sizeof(payload));

    uint8_t wire[400];
    uint16_t wire_len = build_addressed_msg(
            wire, 0x1C48, 0x010203040506ULL, 0x0A0B0C0D0E0FULL,
            payload, 260);

    TcpRxStatemachine_incoming_data(wire, wire_len);

    EXPECT_EQ(_push_count, 1);
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->payload_count, LEN_MESSAGE_BYTES_STREAM);
    EXPECT_EQ(memcmp(_last_pushed_msg->payload, payload, LEN_MESSAGE_BYTES_STREAM), 0);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Multipart FIRST — table full, third FIRST dropped
// =============================================================================

TEST(TCP_RxStatemachine, multipart_first_table_full_dropped)
{
    setup_test();

    node_id_t source = 0x010203040506ULL;
    node_id_t dest   = 0x0A0B0C0D0E0FULL;
    uint8_t pay[] = {0x01, 0x02};

    uint8_t wire[128];
    uint16_t len;

    // Fill slot 0
    len = build_multipart_first_msg(wire, 0xAAAAAAAAAAAAULL,
            0x1C48, source, dest, pay, 2);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Fill slot 1
    len = build_multipart_first_msg(wire, 0xBBBBBBBBBBBBULL,
            0x1C48, source, dest, pay, 2);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Third FIRST — table full, dropped
    len = build_multipart_first_msg(wire, 0xCCCCCCCCCCCCULL,
            0x1C48, source, dest, pay, 2);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Verify slot 0 still works — send LAST for it
    uint8_t last_pay[] = {0x03, 0x04};
    len = build_multipart_continuation_msg(wire, TCP_FLAGS_MULTIPART_LAST,
            0xAAAAAAAAAAAAULL, last_pay, 2);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 1);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
    TcpRxStatemachine_reset();
}

// =============================================================================
// Multipart MIDDLE with no matching slot — dropped
// =============================================================================

TEST(TCP_RxStatemachine, multipart_middle_no_slot_dropped)
{
    setup_test();

    uint8_t pay[] = {0x01, 0x02, 0x03};
    uint8_t wire[64];
    uint16_t len = build_multipart_continuation_msg(wire,
            TCP_FLAGS_MULTIPART_MIDDLE, 0xDDDDDDDDDDDDULL, pay, 3);

    TcpRxStatemachine_incoming_data(wire, len);

    EXPECT_EQ(_push_count, 0);
}

// =============================================================================
// Multipart LAST with no matching slot — dropped
// =============================================================================

TEST(TCP_RxStatemachine, multipart_last_no_slot_dropped)
{
    setup_test();

    uint8_t pay[] = {0x01, 0x02, 0x03};
    uint8_t wire[64];
    uint16_t len = build_multipart_continuation_msg(wire,
            TCP_FLAGS_MULTIPART_LAST, 0xEEEEEEEEEEEEULL, pay, 3);

    TcpRxStatemachine_incoming_data(wire, len);

    EXPECT_EQ(_push_count, 0);
}

// =============================================================================
// Addressed message body too short for destination Node ID
// =============================================================================

TEST(TCP_RxStatemachine, addressed_body_too_short_for_dest)
{
    setup_test();

    uint8_t wire[64];
    // body = 10 bytes: MTI(2) + source(6) + 2 extra (not enough for dest 6)
    // MTI 0x0828 has MASK_DEST_ADDRESS_PRESENT set
    uint16_t body_len = 10;
    uint16_t offset = TcpUtilities_encode_preamble(
            wire, TCP_FLAGS_MESSAGE, body_len, 0x050101012200ULL, 500);
    offset += TcpUtilities_encode_uint16(&wire[offset], 0x0828);
    offset += TcpUtilities_encode_node_id(&wire[offset], 0x010203040506ULL);
    wire[offset++] = 0x00;
    wire[offset++] = 0x00;

    TcpRxStatemachine_incoming_data(wire, offset);

    EXPECT_EQ(_push_count, 0);
}

// =============================================================================
// allocate_buffer returns NULL for single-part message — dropped
// =============================================================================

TEST(TCP_RxStatemachine, allocate_null_single_part_dropped)
{
    setup_test();
    _mock_allocate_returns_null = true;

    uint8_t wire[64];
    uint16_t wire_len = build_unaddressed_msg(wire, 0x0490, 0x010203040506ULL);

    TcpRxStatemachine_incoming_data(wire, wire_len);

    EXPECT_EQ(_push_count, 0);
}

// =============================================================================
// Multipart FIRST with body too short (< MTI + source NodeID)
// =============================================================================

TEST(TCP_RxStatemachine, multipart_first_body_too_short)
{
    setup_test();

    uint8_t wire[64];
    // body = 4 bytes (less than minimum 8 = MTI + source NodeID)
    uint16_t offset = TcpUtilities_encode_preamble(
            wire, TCP_FLAGS_MESSAGE | TCP_FLAGS_MULTIPART_FIRST,
            4, 0x050101012200ULL, 500);
    wire[offset++] = 0x04;
    wire[offset++] = 0x90;
    wire[offset++] = 0x00;
    wire[offset++] = 0x00;

    TcpRxStatemachine_incoming_data(wire, offset);

    EXPECT_EQ(_push_count, 0);

    TcpRxStatemachine_reset();
}

// =============================================================================
// Multipart FIRST — allocate_buffer returns NULL
// =============================================================================

TEST(TCP_RxStatemachine, multipart_first_allocate_null)
{
    setup_test();
    _mock_allocate_returns_null = true;

    uint8_t pay[] = {0x01, 0x02};
    uint8_t wire[128];
    uint16_t len = build_multipart_first_msg(wire, 0xAAAAAAAAAAAAULL,
            0x1C48, 0x010203040506ULL, 0x0A0B0C0D0E0FULL, pay, 2);

    TcpRxStatemachine_incoming_data(wire, len);

    EXPECT_EQ(_push_count, 0);
}

// =============================================================================
// Buffer overflow — accumulation buffer full resets _rx_count
// =============================================================================

TEST(TCP_RxStatemachine, buffer_overflow_resets)
{
    setup_test();

    // Build a preamble declaring a very large body (1100 bytes)
    // so the while loop waits for more data, and we keep feeding bytes
    uint8_t big_buf[1030];
    uint16_t offset = TcpUtilities_encode_preamble(
            big_buf, TCP_FLAGS_MESSAGE, 1100, 0x050101012200ULL, 500);
    memset(&big_buf[offset], 0xAA, sizeof(big_buf) - offset);

    // Feed 1020 bytes — fills accumulation buffer to 1020
    TcpRxStatemachine_incoming_data(big_buf, 1020);
    EXPECT_EQ(_push_count, 0);

    // Feed 10 more — at byte 1024 the overflow fires, resets _rx_count
    TcpRxStatemachine_incoming_data(&big_buf[1020], 10);
    EXPECT_EQ(_push_count, 0);

    // Verify system recovers — send a fresh valid message
    uint8_t wire[64];
    uint16_t wire_len = build_unaddressed_msg(wire, 0x0490, 0x010203040506ULL);
    TcpRxStatemachine_incoming_data(wire, wire_len);
    EXPECT_EQ(_push_count, 1);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Partial message — preamble complete but body incomplete (while loop return)
// =============================================================================

TEST(TCP_RxStatemachine, partial_preamble_complete_body_incomplete)
{
    setup_test();

    uint8_t wire[64];
    uint16_t wire_len = build_unaddressed_msg(wire, 0x0490, 0x010203040506ULL);

    // Send 20 bytes: enough for preamble (17) but not full message (25)
    // This enters the while loop but hits _rx_count < total_msg_len → return
    TcpRxStatemachine_incoming_data(wire, 20);
    EXPECT_EQ(_push_count, 0);

    // Send remaining bytes — message completes
    TcpRxStatemachine_incoming_data(&wire[20], wire_len - 20);
    EXPECT_EQ(_push_count, 1);
    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->mti, 0x0490);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Reset frees in-progress multipart assembly
// =============================================================================

TEST(TCP_RxStatemachine, reset_frees_in_progress_multipart)
{
    setup_test();

    uint8_t pay[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t wire[128];
    uint16_t len = build_multipart_first_msg(wire, 0xAAAAAAAAAAAAULL,
            0x1C48, 0x010203040506ULL, 0x0A0B0C0D0E0FULL, pay, 4);

    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Reset should free the in-progress multipart buffer
    TcpRxStatemachine_reset();

    // Verify system works after reset
    uint8_t wire2[64];
    uint16_t wire2_len = build_unaddressed_msg(wire2, 0x0490, 0x010203040506ULL);
    TcpRxStatemachine_incoming_data(wire2, wire2_len);
    EXPECT_EQ(_push_count, 1);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Multipart FIRST + MIDDLE + LAST — three-part reassembly
// =============================================================================

// =============================================================================
// Link control with zero body — covers (length > 12) false branch in line 189
// =============================================================================

TEST(TCP_RxStatemachine, link_control_zero_body)
{
    setup_test();

    uint8_t wire[32];
    uint16_t offset = TcpUtilities_encode_preamble(
            wire, 0x0000, 0, 0x050101012200ULL, 500);

    TcpRxStatemachine_incoming_data(wire, offset);

    EXPECT_TRUE(_link_control_called);
    EXPECT_EQ(_link_control_len, 0);
    EXPECT_EQ(_push_count, 0);
}

// =============================================================================
// OpenLCB message with zero body — covers (length > 12) false branch line 199
// =============================================================================

TEST(TCP_RxStatemachine, message_zero_body_dropped)
{
    setup_test();

    uint8_t wire[32];
    uint16_t offset = TcpUtilities_encode_preamble(
            wire, TCP_FLAGS_MESSAGE, 0, 0x050101012200ULL, 500);

    TcpRxStatemachine_incoming_data(wire, offset);

    EXPECT_EQ(_push_count, 0);
}

// =============================================================================
// Multipart FIRST with addressed MTI but body too short for dest node ID
// Covers lines 227 true, 228 false, 234 (payload_len=0), 252 false
// =============================================================================

TEST(TCP_RxStatemachine, multipart_first_addressed_body_short_for_dest)
{
    setup_test();

    uint8_t wire[64];
    // body = MTI(2) + source(6) = 8 bytes. Addressed MTI but no room for dest.
    uint16_t body_len = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN;
    uint16_t offset = TcpUtilities_encode_preamble(
            wire, TCP_FLAGS_MESSAGE | TCP_FLAGS_MULTIPART_FIRST,
            body_len, 0x050101012200ULL, 500);
    offset += TcpUtilities_encode_uint16(&wire[offset], 0x0828);  // addressed MTI
    offset += TcpUtilities_encode_node_id(&wire[offset], 0x010203040506ULL);

    TcpRxStatemachine_incoming_data(wire, offset);

    EXPECT_EQ(_push_count, 0);

    // Send LAST to complete and free the slot
    uint8_t last_wire[64];
    uint16_t last_len = build_multipart_continuation_msg(last_wire,
            TCP_FLAGS_MULTIPART_LAST, 0x050101012200ULL, NULL, 0);
    TcpRxStatemachine_incoming_data(last_wire, last_len);

    if (_last_pushed_msg)
        OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Multipart MIDDLE with zero-length body — covers line 267 false branch
// =============================================================================

TEST(TCP_RxStatemachine, multipart_middle_zero_body)
{
    setup_test();

    node_id_t orig = 0xAAAAAAAAAAAAULL;
    uint8_t pay[] = {0x01, 0x02};
    uint8_t wire[128];
    uint16_t len;

    // Send FIRST
    len = build_multipart_first_msg(wire, orig,
            0x1C48, 0x010203040506ULL, 0x0A0B0C0D0E0FULL, pay, 2);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Send MIDDLE with zero body
    len = build_multipart_continuation_msg(wire, TCP_FLAGS_MULTIPART_MIDDLE,
            orig, NULL, 0);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Send LAST to complete
    uint8_t last_pay[] = {0x03, 0x04};
    len = build_multipart_continuation_msg(wire, TCP_FLAGS_MULTIPART_LAST,
            orig, last_pay, 2);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 1);

    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->payload_count, 4);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Multipart payload overflow — SNIP buffer full, remaining=0
// Covers lines 270 (remaining=0), 272 (copy_len=0), 274 (copy_len=0 false)
// =============================================================================

TEST(TCP_RxStatemachine, multipart_payload_overflow_capped)
{
    setup_test();

    node_id_t orig = 0xAAAAAAAAAAAAULL;
    uint8_t wire[400];
    uint16_t len;

    // Send FIRST with 250-byte payload (close to SNIP limit of 256)
    uint8_t big_pay[250];
    memset(big_pay, 0xAA, sizeof(big_pay));
    len = build_multipart_first_msg(wire, orig,
            0x1C48, 0x010203040506ULL, 0x0A0B0C0D0E0FULL, big_pay, 250);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Send MIDDLE with 10 bytes — only 6 fit (256-250=6), remaining after = 0
    uint8_t mid_pay[10];
    memset(mid_pay, 0xBB, sizeof(mid_pay));
    len = build_multipart_continuation_msg(wire, TCP_FLAGS_MULTIPART_MIDDLE,
            orig, mid_pay, 10);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Send another MIDDLE — remaining = 0, copy_len = 0
    uint8_t mid_pay2[5];
    memset(mid_pay2, 0xCC, sizeof(mid_pay2));
    len = build_multipart_continuation_msg(wire, TCP_FLAGS_MULTIPART_MIDDLE,
            orig, mid_pay2, 5);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // Send LAST to complete
    len = build_multipart_continuation_msg(wire, TCP_FLAGS_MULTIPART_LAST,
            orig, NULL, 0);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 1);

    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->payload_count, LEN_MESSAGE_BYTES_SNIP);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}

// =============================================================================
// Multipart FIRST + MIDDLE + LAST — three-part reassembly
// =============================================================================

TEST(TCP_RxStatemachine, multipart_first_middle_last)
{
    setup_test();

    node_id_t orig = 0xAAAAAAAAAAAAULL;
    node_id_t source = 0x010203040506ULL;
    node_id_t dest = 0x0A0B0C0D0E0FULL;

    uint8_t wire[128];
    uint16_t len;

    // FIRST with 4-byte payload
    uint8_t p1[] = {0x01, 0x02, 0x03, 0x04};
    len = build_multipart_first_msg(wire, orig, 0x1C48, source, dest, p1, 4);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // MIDDLE with 4-byte payload
    uint8_t p2[] = {0x05, 0x06, 0x07, 0x08};
    len = build_multipart_continuation_msg(wire, TCP_FLAGS_MULTIPART_MIDDLE,
            orig, p2, 4);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 0);

    // LAST with 4-byte payload
    uint8_t p3[] = {0x09, 0x0A, 0x0B, 0x0C};
    len = build_multipart_continuation_msg(wire, TCP_FLAGS_MULTIPART_LAST,
            orig, p3, 4);
    TcpRxStatemachine_incoming_data(wire, len);
    EXPECT_EQ(_push_count, 1);

    ASSERT_NE(_last_pushed_msg, nullptr);
    EXPECT_EQ(_last_pushed_msg->mti, 0x1C48);
    EXPECT_EQ(_last_pushed_msg->source_id, source);
    EXPECT_EQ(_last_pushed_msg->dest_id, dest);
    EXPECT_EQ(_last_pushed_msg->payload_count, 12);

    uint8_t expected[] = {0x01, 0x02, 0x03, 0x04,
                          0x05, 0x06, 0x07, 0x08,
                          0x09, 0x0A, 0x0B, 0x0C};
    EXPECT_EQ(memcmp(_last_pushed_msg->payload, expected, 12), 0);

    OpenLcbBufferStore_free_buffer(_last_pushed_msg);
}
