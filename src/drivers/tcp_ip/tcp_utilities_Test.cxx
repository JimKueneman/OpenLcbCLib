/*******************************************************************************
 * File: tcp_utilities_Test.cxx
 *
 * Description:
 *   Test suite for TCP Utilities module — preamble encode/decode, node ID
 *   encoding, flag helpers, and byte-order conversions.
 *
 * Module Under Test:
 *   TcpUtilities — stateless encode/decode helpers for TCP/IP OpenLCB wire format
 *
 * Test Coverage:
 *   - Preamble encode/decode round-trip
 *   - Individual field decode (flags, length, originating Node ID, capture time)
 *   - MTI encode/decode
 *   - Node ID encode/decode
 *   - uint16 / uint48 encode
 *   - Flag helpers (is_openlcb_message, is_chained, multipart_type)
 *   - Boundary values (zero, max 48-bit, max 24-bit length)
 *
 * Author: Test Suite
 * Date: 2026-04-05
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "tcp_utilities.h"

// =============================================================================
// Preamble Encode
// =============================================================================

TEST(TCP_Utilities, encode_preamble_returns_17)
{
    uint8_t buf[TCP_PREAMBLE_LEN];

    uint16_t len = TcpUtilities_encode_preamble(
            buf, TCP_FLAGS_MESSAGE, 100, 0x010203040506ULL, 0x0A0B0C0D0E0FULL);

    EXPECT_EQ(len, TCP_PREAMBLE_LEN);
}

TEST(TCP_Utilities, encode_preamble_flags)
{
    uint8_t buf[TCP_PREAMBLE_LEN];

    TcpUtilities_encode_preamble(buf, 0x8000, 0, 0, 0);

    EXPECT_EQ(buf[0], 0x80);
    EXPECT_EQ(buf[1], 0x00);
}

TEST(TCP_Utilities, encode_preamble_length_includes_12_byte_overhead)
{
    uint8_t buf[TCP_PREAMBLE_LEN];

    // body_len=100, so encoded length = 100 + 12 = 112 = 0x000070
    TcpUtilities_encode_preamble(buf, TCP_FLAGS_MESSAGE, 100, 0, 0);

    uint32_t length = ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 8) | buf[4];
    EXPECT_EQ(length, 112u);
}

TEST(TCP_Utilities, encode_preamble_originating_node_id)
{
    uint8_t buf[TCP_PREAMBLE_LEN];

    TcpUtilities_encode_preamble(buf, 0, 0, 0x010203040506ULL, 0);

    EXPECT_EQ(buf[5],  0x01);
    EXPECT_EQ(buf[6],  0x02);
    EXPECT_EQ(buf[7],  0x03);
    EXPECT_EQ(buf[8],  0x04);
    EXPECT_EQ(buf[9],  0x05);
    EXPECT_EQ(buf[10], 0x06);
}

TEST(TCP_Utilities, encode_preamble_capture_time)
{
    uint8_t buf[TCP_PREAMBLE_LEN];

    TcpUtilities_encode_preamble(buf, 0, 0, 0, 0x0A0B0C0D0E0FULL);

    EXPECT_EQ(buf[11], 0x0A);
    EXPECT_EQ(buf[12], 0x0B);
    EXPECT_EQ(buf[13], 0x0C);
    EXPECT_EQ(buf[14], 0x0D);
    EXPECT_EQ(buf[15], 0x0E);
    EXPECT_EQ(buf[16], 0x0F);
}

// =============================================================================
// Preamble Decode
// =============================================================================

TEST(TCP_Utilities, decode_flags)
{
    uint8_t buf[TCP_PREAMBLE_LEN] = {};
    buf[0] = 0xC0;
    buf[1] = 0x40;

    EXPECT_EQ(TcpUtilities_decode_flags(buf), 0xC040);
}

TEST(TCP_Utilities, decode_length)
{
    uint8_t buf[TCP_PREAMBLE_LEN] = {};
    buf[2] = 0x01;
    buf[3] = 0x02;
    buf[4] = 0x03;

    EXPECT_EQ(TcpUtilities_decode_length(buf), 0x010203u);
}

TEST(TCP_Utilities, decode_originating_node_id)
{
    uint8_t buf[TCP_PREAMBLE_LEN] = {};
    buf[5]  = 0xAA;
    buf[6]  = 0xBB;
    buf[7]  = 0xCC;
    buf[8]  = 0xDD;
    buf[9]  = 0xEE;
    buf[10] = 0xFF;

    EXPECT_EQ(TcpUtilities_decode_originating_node_id(buf), 0xAABBCCDDEEFFULL);
}

TEST(TCP_Utilities, decode_capture_time)
{
    uint8_t buf[TCP_PREAMBLE_LEN] = {};
    buf[11] = 0x11;
    buf[12] = 0x22;
    buf[13] = 0x33;
    buf[14] = 0x44;
    buf[15] = 0x55;
    buf[16] = 0x66;

    EXPECT_EQ(TcpUtilities_decode_capture_time(buf), 0x112233445566ULL);
}

// =============================================================================
// Preamble Round-Trip
// =============================================================================

TEST(TCP_Utilities, preamble_roundtrip)
{
    uint8_t buf[TCP_PREAMBLE_LEN];

    uint16_t flags = TCP_FLAGS_MESSAGE | TCP_FLAGS_CHAINING;
    uint32_t body_len = 500;
    node_id_t orig_id = 0x050301020304ULL;
    uint64_t capture = 0xFEDCBA987654ULL;

    TcpUtilities_encode_preamble(buf, flags, body_len, orig_id, capture);

    EXPECT_EQ(TcpUtilities_decode_flags(buf), flags);
    EXPECT_EQ(TcpUtilities_decode_length(buf), body_len + 12);
    EXPECT_EQ(TcpUtilities_decode_originating_node_id(buf), orig_id);
    EXPECT_EQ(TcpUtilities_decode_capture_time(buf), capture);
}

TEST(TCP_Utilities, preamble_roundtrip_zero_body)
{
    uint8_t buf[TCP_PREAMBLE_LEN];

    TcpUtilities_encode_preamble(buf, 0, 0, 0, 0);

    EXPECT_EQ(TcpUtilities_decode_flags(buf), 0);
    EXPECT_EQ(TcpUtilities_decode_length(buf), 12u);  // 0 body + 12 overhead
    EXPECT_EQ(TcpUtilities_decode_originating_node_id(buf), 0u);
    EXPECT_EQ(TcpUtilities_decode_capture_time(buf), 0u);
}

TEST(TCP_Utilities, preamble_roundtrip_max_24bit_length)
{
    uint8_t buf[TCP_PREAMBLE_LEN];

    // Max 24-bit body_len that doesn't overflow: 0xFFFFFF - 12
    uint32_t body_len = 0xFFFFFF - 12;

    TcpUtilities_encode_preamble(buf, TCP_FLAGS_MESSAGE, body_len, 0, 0);

    EXPECT_EQ(TcpUtilities_decode_length(buf), 0xFFFFFFu);
}

// =============================================================================
// Message Body Helpers
// =============================================================================

TEST(TCP_Utilities, decode_mti)
{
    uint8_t body[] = {0x04, 0x90};  // MTI 0x0490 = Verify Node ID Global

    EXPECT_EQ(TcpUtilities_decode_mti(body), 0x0490);
}

TEST(TCP_Utilities, decode_node_id)
{
    uint8_t buf[] = {0x05, 0x01, 0x01, 0x01, 0x22, 0x00};

    EXPECT_EQ(TcpUtilities_decode_node_id(buf), 0x050101012200ULL);
}

TEST(TCP_Utilities, encode_node_id)
{
    uint8_t buf[6] = {};

    uint8_t written = TcpUtilities_encode_node_id(buf, 0xAABBCCDDEEFFULL);

    EXPECT_EQ(written, 6);
    EXPECT_EQ(buf[0], 0xAA);
    EXPECT_EQ(buf[1], 0xBB);
    EXPECT_EQ(buf[2], 0xCC);
    EXPECT_EQ(buf[3], 0xDD);
    EXPECT_EQ(buf[4], 0xEE);
    EXPECT_EQ(buf[5], 0xFF);
}

TEST(TCP_Utilities, node_id_roundtrip)
{
    uint8_t buf[6];
    node_id_t original = 0x010203040506ULL;

    TcpUtilities_encode_node_id(buf, original);

    EXPECT_EQ(TcpUtilities_decode_node_id(buf), original);
}

TEST(TCP_Utilities, encode_uint16)
{
    uint8_t buf[2] = {};

    uint8_t written = TcpUtilities_encode_uint16(buf, 0x1234);

    EXPECT_EQ(written, 2);
    EXPECT_EQ(buf[0], 0x12);
    EXPECT_EQ(buf[1], 0x34);
}

TEST(TCP_Utilities, encode_uint48)
{
    uint8_t buf[6] = {};

    uint8_t written = TcpUtilities_encode_uint48(buf, 0x112233445566ULL);

    EXPECT_EQ(written, 6);
    EXPECT_EQ(buf[0], 0x11);
    EXPECT_EQ(buf[1], 0x22);
    EXPECT_EQ(buf[2], 0x33);
    EXPECT_EQ(buf[3], 0x44);
    EXPECT_EQ(buf[4], 0x55);
    EXPECT_EQ(buf[5], 0x66);
}

// =============================================================================
// Flag Helpers
// =============================================================================

TEST(TCP_Utilities, is_openlcb_message_true)
{
    EXPECT_TRUE(TcpUtilities_is_openlcb_message(TCP_FLAGS_MESSAGE));
    EXPECT_TRUE(TcpUtilities_is_openlcb_message(0xFFFF));
}

TEST(TCP_Utilities, is_openlcb_message_false)
{
    EXPECT_FALSE(TcpUtilities_is_openlcb_message(0x0000));
    EXPECT_FALSE(TcpUtilities_is_openlcb_message(0x7FFF));
}

TEST(TCP_Utilities, is_chained_true)
{
    EXPECT_TRUE(TcpUtilities_is_chained(TCP_FLAGS_CHAINING));
    EXPECT_TRUE(TcpUtilities_is_chained(TCP_FLAGS_MESSAGE | TCP_FLAGS_CHAINING));
}

TEST(TCP_Utilities, is_chained_false)
{
    EXPECT_FALSE(TcpUtilities_is_chained(0x0000));
    EXPECT_FALSE(TcpUtilities_is_chained(TCP_FLAGS_MESSAGE));
}

TEST(TCP_Utilities, multipart_type_single)
{
    EXPECT_EQ(TcpUtilities_multipart_type(TCP_FLAGS_MESSAGE), TCP_FLAGS_MULTIPART_SINGLE);
}

TEST(TCP_Utilities, multipart_type_first)
{
    EXPECT_EQ(TcpUtilities_multipart_type(TCP_FLAGS_MESSAGE | TCP_FLAGS_MULTIPART_FIRST),
              TCP_FLAGS_MULTIPART_FIRST);
}

TEST(TCP_Utilities, multipart_type_middle)
{
    EXPECT_EQ(TcpUtilities_multipart_type(TCP_FLAGS_MESSAGE | TCP_FLAGS_MULTIPART_MIDDLE),
              TCP_FLAGS_MULTIPART_MIDDLE);
}

TEST(TCP_Utilities, multipart_type_last)
{
    EXPECT_EQ(TcpUtilities_multipart_type(TCP_FLAGS_MESSAGE | TCP_FLAGS_MULTIPART_LAST),
              TCP_FLAGS_MULTIPART_LAST);
}
