/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Test suite for OpenLCB GridConnect Protocol
 * Refactored and enhanced for maximum code coverage
 */

#include "test/main_Test.hxx"

#include "openlcb_gridconnect.h"
#include "openlcb_types.h"

// ============================================================================
// EXISTING TESTS - Cleaned up and improved
// ============================================================================

/**
 * @brief Test GridConnect streaming parser with various valid and invalid inputs
 * 
 * Verifies:
 * - Parser handles uppercase hex digits
 * - Parser handles lowercase hex digits
 * - Parser handles messages with no data
 * - Parser rejects invalid hex characters
 * - Parser rejects messages with wrong header length
 * - Parser rejects odd number of data characters
 * - Parser rejects too many data characters
 * - State machine properly resets on errors
 */
TEST(OpenLcbGridConnect, copy_out_gridconnect_when_done)
{
    gridconnect_buffer_t gridconnect_buffer;

    // ========================================================================
    // Test 1: Valid message with uppercase hex letters
    // ========================================================================
    const char *gridconnect_pip = ":X19828BC7N06EB;";

    // Feed bytes one at a time - all should return false except last
    for (size_t i = 0; i < strlen(gridconnect_pip) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[i], &gridconnect_buffer))
            << "Byte " << i << " should not complete message";
    }

    // Last byte (';') should complete the message
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_pip[strlen(gridconnect_pip) - 1], &gridconnect_buffer))
        << "Final byte should complete message";
    
    EXPECT_STREQ(":X19828BC7N06EB;", (char *)&gridconnect_buffer);

    // ========================================================================
    // Test 2: Valid message with lowercase hex letters
    // ========================================================================
    const char *gridconnect_identify_producer = ":x19914bc7n06eb;";

    for (size_t i = 0; i < strlen(gridconnect_identify_producer) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_producer[i], &gridconnect_buffer))
            << "Byte " << i << " should not complete message";
    }

    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_producer[strlen(gridconnect_identify_producer) - 1], &gridconnect_buffer))
        << "Final byte should complete message";
    
    EXPECT_STREQ(":x19914bc7n06eb;", (char *)&gridconnect_buffer);

    // ========================================================================
    // Test 3: Valid message with no data bytes
    // ========================================================================
    const char *gridconnect_identify_events = ":X19970BC7N;";

    for (size_t i = 0; i < strlen(gridconnect_identify_events) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_events[i], &gridconnect_buffer))
            << "Byte " << i << " should not complete message";
    }

    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_identify_events[strlen(gridconnect_identify_events) - 1], &gridconnect_buffer))
        << "Final byte should complete message";
    
    EXPECT_STREQ(":X19970BC7N;", (char *)&gridconnect_buffer);

    // ========================================================================
    // Test 4: Invalid message - bad hex character 'G' in header
    // ========================================================================
    const char *gridconnect_bad_hex = ":X19970GC7N;";

    for (size_t i = 0; i < strlen(gridconnect_bad_hex); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_hex[i], &gridconnect_buffer))
            << "Invalid hex should cause rejection at byte " << i;
    }

    // ========================================================================
    // Test 5: Invalid message - header too long (9 chars instead of 8)
    // ========================================================================
    const char *gridconnect_bad_long_header = ":X19970C75FN;";

    for (size_t i = 0; i < strlen(gridconnect_bad_long_header); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_long_header[i], &gridconnect_buffer))
            << "Too-long header should cause rejection at byte " << i;
    }

    // ========================================================================
    // Test 6: Invalid message - odd number of data characters
    // ========================================================================
    const char *gridconnect_odd_number_of_data_characters = ":X19828BC7N6EB;";

    for (size_t i = 0; i < strlen(gridconnect_odd_number_of_data_characters); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_odd_number_of_data_characters[i], &gridconnect_buffer))
            << "Odd data length should cause rejection at byte " << i;
    }

    // ========================================================================
    // Test 7: Invalid message - header too short (7 chars instead of 8)
    // ========================================================================
    const char *gridconnect_too_few_header_characters = ":X9828BC7N6EB;";

    for (size_t i = 0; i < strlen(gridconnect_too_few_header_characters); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_too_few_header_characters[i], &gridconnect_buffer))
            << "Too-short header should cause rejection at byte " << i;
    }

    // ========================================================================
    // Test 8: Invalid message - bad hex character 'G' in data
    // ========================================================================
    const char *gridconnect_bad_hex_in_data = ":X19970BC7N06BE0G;";

    for (size_t i = 0; i < strlen(gridconnect_bad_hex_in_data); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_bad_hex_in_data[i], &gridconnect_buffer))
            << "Invalid hex in data should cause rejection at byte " << i;
    }

    // ========================================================================
    // Test 9: Invalid message - too many data characters (overflow)
    // ========================================================================
    const char *gridconnect_too_many_data_characters = ":X19970BC7N010203040506070800;";

    for (size_t i = 0; i < strlen(gridconnect_too_many_data_characters); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(gridconnect_too_many_data_characters[i], &gridconnect_buffer))
            << "Buffer overflow should cause rejection at byte " << i;
    }
}

/**
 * @brief Test GridConnect to CAN message conversion
 * 
 * Verifies:
 * - Conversion of message with no data
 * - Conversion of message with 2 data bytes
 * - Conversion of message with 8 data bytes (full CAN frame)
 * - Correct identifier extraction
 * - Correct payload extraction
 */
TEST(OpenLcbGridConnect, to_can_msg)
{
    can_msg_t can_msg;

    // ========================================================================
    // Test 1: Message with no data bytes
    // ========================================================================
    const char *gridconnect_pip_none = ":X19828BC7N;";

    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gridconnect_pip_none, &can_msg);
    
    EXPECT_EQ(can_msg.identifier, 0x19828BC7);
    EXPECT_EQ(can_msg.payload_count, 0);

    // ========================================================================
    // Test 2: Message with 2 data bytes
    // ========================================================================
    const char *gridconnect_pip_medium = ":X19828BC7N06EB;";

    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gridconnect_pip_medium, &can_msg);
    
    EXPECT_EQ(can_msg.identifier, 0x19828BC7);
    EXPECT_EQ(can_msg.payload_count, 2);
    EXPECT_EQ(can_msg.payload[0], 0x06);
    EXPECT_EQ(can_msg.payload[1], 0xEB);

    // ========================================================================
    // Test 3: Message with 8 data bytes (full CAN frame)
    // ========================================================================
    const char *gridconnect_pip_full = ":X19828BC7N0102030405060708;";

    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gridconnect_pip_full, &can_msg);
    
    EXPECT_EQ(can_msg.identifier, 0x19828BC7);
    EXPECT_EQ(can_msg.payload_count, 8);
    EXPECT_EQ(can_msg.payload[0], 0x01);
    EXPECT_EQ(can_msg.payload[1], 0x02);
    EXPECT_EQ(can_msg.payload[2], 0x03);
    EXPECT_EQ(can_msg.payload[3], 0x04);
    EXPECT_EQ(can_msg.payload[4], 0x05);
    EXPECT_EQ(can_msg.payload[5], 0x06);
    EXPECT_EQ(can_msg.payload[6], 0x07);
    EXPECT_EQ(can_msg.payload[7], 0x08);
}

/**
 * @brief Test CAN message to GridConnect conversion
 * 
 * Verifies:
 * - Conversion of CAN message with no data
 * - Conversion of CAN message with 2 data bytes
 * - Conversion of CAN message with 8 data bytes
 * - Correct format with leading zeros in identifier
 * - Uppercase hex output
 */
TEST(OpenLcbGridConnect, from_can_msg)
{
    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;

    // ========================================================================
    // Test 1: CAN message with no data bytes
    // ========================================================================
    can_msg.identifier = 0x19828BC7;
    can_msg.payload_count = 0;

    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    
    EXPECT_STREQ((char *)&gridconnect_buffer, ":X19828BC7N;");

    // ========================================================================
    // Test 2: CAN message with 2 data bytes
    // ========================================================================
    can_msg.identifier = 0x19828BC7;
    can_msg.payload_count = 2;
    can_msg.payload[0] = 0x06;
    can_msg.payload[1] = 0xEB;

    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    
    EXPECT_STREQ((char *)&gridconnect_buffer, ":X19828BC7N06EB;");

    // ========================================================================
    // Test 3: CAN message with 8 data bytes (full)
    // ========================================================================
    can_msg.identifier = 0x19828BC7;
    can_msg.payload_count = 8;
    can_msg.payload[0] = 0x01;
    can_msg.payload[1] = 0x02;
    can_msg.payload[2] = 0x03;
    can_msg.payload[3] = 0x04;
    can_msg.payload[4] = 0x05;
    can_msg.payload[5] = 0x06;
    can_msg.payload[6] = 0x07;
    can_msg.payload[7] = 0x08;

    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    
    EXPECT_STREQ((char *)&gridconnect_buffer, ":X19828BC7N0102030405060708;");
}

// ============================================================================
// NEW TESTS - For maximum coverage (ALL COMMENTED OUT)
// ============================================================================

/**
 * @brief Test parser state machine synchronization
 * 
 * Verifies:
 * - Parser ignores garbage before 'X'
 * - Parser synchronizes to first 'X'
 * - State machine properly transitions
 */
TEST(OpenLcbGridConnect, parser_synchronization)
{
    gridconnect_buffer_t gridconnect_buffer;
    
    const char *garbage_then_valid = "GARBAGE:X19828BC7N06EB;";
    
    // Feed garbage - should all return false
    for (size_t i = 0; i < 7; i++)  // "GARBAGE"
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(garbage_then_valid[i], &gridconnect_buffer))
            << "Garbage byte " << i << " should be ignored";
    }
    
    // Now feed the valid message starting with ':'
    for (size_t i = 7; i < strlen(garbage_then_valid) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(garbage_then_valid[i], &gridconnect_buffer))
            << "Valid byte " << i << " should not complete yet";
    }
    
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(garbage_then_valid[strlen(garbage_then_valid) - 1], &gridconnect_buffer))
        << "Final ';' should complete message";
    
    EXPECT_STREQ(":X19828BC7N06EB;", (char *)&gridconnect_buffer);
}

/**
 * @brief Test parser with multiple messages in sequence
 * 
 * Verifies:
 * - State resets after successful parse
 * - Can immediately parse next message
 * - No state contamination between messages
 */
TEST(OpenLcbGridConnect, sequential_messages)
{
    gridconnect_buffer_t gridconnect_buffer;
    
    // First message
    const char *msg1 = ":X19828BC7N06EB;";
    for (size_t i = 0; i < strlen(msg1) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(msg1[i], &gridconnect_buffer));
    }
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(msg1[strlen(msg1) - 1], &gridconnect_buffer));
    EXPECT_STREQ(":X19828BC7N06EB;", (char *)&gridconnect_buffer);
    
    // Second message - should work immediately
    const char *msg2 = ":X19970BC7N;";
    for (size_t i = 0; i < strlen(msg2) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(msg2[i], &gridconnect_buffer));
    }
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(msg2[strlen(msg2) - 1], &gridconnect_buffer));
    EXPECT_STREQ(":X19970BC7N;", (char *)&gridconnect_buffer);
    
    // Third message with data
    const char *msg3 = ":X19914BC7N0102;";
    for (size_t i = 0; i < strlen(msg3) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(msg3[i], &gridconnect_buffer));
    }
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(msg3[strlen(msg3) - 1], &gridconnect_buffer));
    EXPECT_STREQ(":X19914BC7N0102;", (char *)&gridconnect_buffer);
}

/**
 * @brief Test parser recovery from errors
 * 
 * Verifies:
 * - Parser recovers from invalid hex in header
 * - Parser can successfully parse next valid message
 * - State machine properly resets on error
 */
TEST(OpenLcbGridConnect, error_recovery)
{
    gridconnect_buffer_t gridconnect_buffer;
    
    // Send invalid message (bad hex 'G')
    const char *bad_msg = ":X19970GC7N;";
    for (size_t i = 0; i < strlen(bad_msg); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(bad_msg[i], &gridconnect_buffer))
            << "Bad message should never complete";
    }
    
    // Now send valid message - should parse successfully
    const char *good_msg = ":X19970BC7N06EB;";
    for (size_t i = 0; i < strlen(good_msg) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(good_msg[i], &gridconnect_buffer));
    }
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(good_msg[strlen(good_msg) - 1], &gridconnect_buffer));
    EXPECT_STREQ(":X19970BC7N06EB;", (char *)&gridconnect_buffer);
}

/**
 * @brief Test conversion with minimum identifier value
 * 
 * Verifies:
 * - Leading zeros preserved in identifier
 * - Smallest valid identifier (0x00000000) works
 */
TEST(OpenLcbGridConnect, min_identifier)
{
    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;
    
    // Test to_can_msg
    const char *gc_min = ":X00000000N;";
    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gc_min, &can_msg);
    EXPECT_EQ(can_msg.identifier, 0x00000000);
    EXPECT_EQ(can_msg.payload_count, 0);
    
    // Test from_can_msg
    can_msg.identifier = 0x00000000;
    can_msg.payload_count = 0;
    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    EXPECT_STREQ((char *)&gridconnect_buffer, ":X00000000N;");
}

/**
 * @brief Test conversion with maximum identifier value
 * 
 * Verifies:
 * - Maximum 29-bit CAN identifier works
 * - All bits set correctly
 */
TEST(OpenLcbGridConnect, max_identifier)
{
    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;
    
    // Test to_can_msg with max 29-bit value
    const char *gc_max = ":X1FFFFFFFN;";
    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gc_max, &can_msg);
    EXPECT_EQ(can_msg.identifier, 0x1FFFFFFF);
    EXPECT_EQ(can_msg.payload_count, 0);
    
    // Test from_can_msg
    can_msg.identifier = 0x1FFFFFFF;
    can_msg.payload_count = 0;
    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    EXPECT_STREQ((char *)&gridconnect_buffer, ":X1FFFFFFFN;");
}

/**
 * @brief Test conversion with single data byte
 * 
 * Verifies:
 * - Single byte payload works correctly
 * - Leading zero in byte value preserved
 */
TEST(OpenLcbGridConnect, single_data_byte)
{
    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;
    
    // Test to_can_msg
    const char *gc_single = ":X19828BC7N05;";
    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)gc_single, &can_msg);
    EXPECT_EQ(can_msg.identifier, 0x19828BC7);
    EXPECT_EQ(can_msg.payload_count, 1);
    EXPECT_EQ(can_msg.payload[0], 0x05);
    
    // Test from_can_msg
    can_msg.identifier = 0x19828BC7;
    can_msg.payload_count = 1;
    can_msg.payload[0] = 0x05;
    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    EXPECT_STREQ((char *)&gridconnect_buffer, ":X19828BC7N05;");
}

/**
 * @brief Test conversion with byte values needing leading zeros
 * 
 * Verifies:
 * - Small byte values (< 0x10) get leading zero
 * - All byte values 0x00-0x0F format correctly
 */
TEST(OpenLcbGridConnect, leading_zeros_in_bytes)
{
    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;
    
    // Test bytes 0x00 through 0x0F
    can_msg.identifier = 0x12345678;
    can_msg.payload_count = 4;
    can_msg.payload[0] = 0x00;
    can_msg.payload[1] = 0x01;
    can_msg.payload[2] = 0x0A;
    can_msg.payload[3] = 0x0F;
    
    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    EXPECT_STREQ((char *)&gridconnect_buffer, ":X12345678N00010A0F;");
    
    // Convert back and verify
    can_msg_t can_msg_back;
    OpenLcbGridConnect_to_can_msg(&gridconnect_buffer, &can_msg_back);
    EXPECT_EQ(can_msg_back.identifier, 0x12345678);
    EXPECT_EQ(can_msg_back.payload_count, 4);
    EXPECT_EQ(can_msg_back.payload[0], 0x00);
    EXPECT_EQ(can_msg_back.payload[1], 0x01);
    EXPECT_EQ(can_msg_back.payload[2], 0x0A);
    EXPECT_EQ(can_msg_back.payload[3], 0x0F);
}

/**
 * @brief Test round-trip conversion (CAN → GridConnect → CAN)
 * 
 * Verifies:
 * - Data integrity through conversion cycle
 * - No data loss or corruption
 */
TEST(OpenLcbGridConnect, roundtrip_conversion)
{
    can_msg_t can_msg_orig, can_msg_result;
    gridconnect_buffer_t gridconnect_buffer;
    
    // Original CAN message
    can_msg_orig.identifier = 0x19A5C123;
    can_msg_orig.payload_count = 6;
    can_msg_orig.payload[0] = 0xAB;
    can_msg_orig.payload[1] = 0xCD;
    can_msg_orig.payload[2] = 0xEF;
    can_msg_orig.payload[3] = 0x01;
    can_msg_orig.payload[4] = 0x23;
    can_msg_orig.payload[5] = 0x45;
    
    // Convert to GridConnect
    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg_orig);
    
    // Convert back to CAN
    OpenLcbGridConnect_to_can_msg(&gridconnect_buffer, &can_msg_result);
    
    // Verify match
    EXPECT_EQ(can_msg_result.identifier, can_msg_orig.identifier);
    EXPECT_EQ(can_msg_result.payload_count, can_msg_orig.payload_count);
    for (int i = 0; i < can_msg_orig.payload_count; i++)
    {
        EXPECT_EQ(can_msg_result.payload[i], can_msg_orig.payload[i])
            << "Byte " << i << " mismatch";
    }
}

/**
 * @brief Test parser with mixed case hex in same message
 * 
 * Verifies:
 * - Parser accepts mixed upper/lowercase hex
 * - Output preserves original case for 'x' and 'n'
 */
TEST(OpenLcbGridConnect, mixed_case_hex)
{
    gridconnect_buffer_t gridconnect_buffer;
    
    // Mixed case: lowercase x and n, mixed hex
    const char *mixed_case = ":x19AbCdEfn01Ef;";
    
    for (size_t i = 0; i < strlen(mixed_case) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(mixed_case[i], &gridconnect_buffer));
    }
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(mixed_case[strlen(mixed_case) - 1], &gridconnect_buffer));
    
    EXPECT_STREQ(":x19AbCdEfn01Ef;", (char *)&gridconnect_buffer);
}

/**
 * @brief Test to_can_msg with too-short message
 * 
 * Verifies:
 * - Function handles messages shorter than header length
 * - Returns zero identifier and zero count
 * - No crash or undefined behavior
 */
TEST(OpenLcbGridConnect, to_can_msg_short_message)
{
    can_msg_t can_msg;
    
    // Message too short (only 5 chars, need 12+)
    const char *too_short = ":X123";
    
    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)too_short, &can_msg);
    
    EXPECT_EQ(can_msg.identifier, 0);
    EXPECT_EQ(can_msg.payload_count, 0);
}

/**
 * @brief Test to_can_msg with empty string
 * 
 * Verifies:
 * - Function handles empty string gracefully
 * - No crash on zero-length input
 */
TEST(OpenLcbGridConnect, to_can_msg_empty_string)
{
    can_msg_t can_msg;
    
    const char *empty = "";
    
    OpenLcbGridConnect_to_can_msg((gridconnect_buffer_t *)empty, &can_msg);
    
    EXPECT_EQ(can_msg.identifier, 0);
    EXPECT_EQ(can_msg.payload_count, 0);
}

/**
 * @brief Test parser with N at wrong position
 * 
 * Verifies:
 * - Parser rejects N before position 10
 * - Parser rejects N after position 10
 * - State machine resets properly
 */
TEST(OpenLcbGridConnect, n_at_wrong_position)
{
    gridconnect_buffer_t gridconnect_buffer;
    
    // N too early (position 9 instead of 10)
    const char *n_too_early = ":X1234567N12;";
    for (size_t i = 0; i < strlen(n_too_early); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(n_too_early[i], &gridconnect_buffer))
            << "N too early should be rejected";
    }
    
    // N too late (position 11 instead of 10)
    const char *n_too_late = ":X123456789N;";
    for (size_t i = 0; i < strlen(n_too_late); i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(n_too_late[i], &gridconnect_buffer))
            << "N too late should be rejected";
    }
}

/**
 * @brief Test parser behavior with ':' not followed by 'X'
 * 
 * Verifies:
 * - Parser ignores ':' not followed by X/x
 * - Parser continues searching for valid start
 */
TEST(OpenLcbGridConnect, colon_without_x)
{
    gridconnect_buffer_t gridconnect_buffer;
    
    // Send ':' followed by non-X characters, then valid message
    const char *data = ":A:B:C:X19828BC7N06EB;";
    
    // All bytes before valid start should return false
    for (size_t i = 0; i < 6; i++)  // ":A:B:C"
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(data[i], &gridconnect_buffer));
    }
    
    // Now feed valid message
    for (size_t i = 6; i < strlen(data) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(data[i], &gridconnect_buffer));
    }
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(data[strlen(data) - 1], &gridconnect_buffer));
    
    EXPECT_STREQ(":X19828BC7N06EB;", (char *)&gridconnect_buffer);
}

/**
 * @brief Test parser with all valid hex values in data
 *
 * Verifies:
 * - Digits 0-9 accepted (first validation path)
 * - Uppercase A-F accepted (second validation path)
 * - Lowercase a-f accepted (third validation path)
 */
TEST(OpenLcbGridConnect, all_hex_digits_in_data)
{
    gridconnect_buffer_t gridconnect_buffer;

    // Message with all three hex digit types:
    // '0'-'9': 0, 1, 8, 9
    // 'A'-'F': A, B, C, D, E, F
    // 'a'-'f': a, b, c, d, e, f
    // Data: "0189AaBbCcDdEeFf" = 8 bytes (16 hex chars - fits in buffer)
    const char *mixed_hex = ":X12345678N0189AaBbCcDdEeFf;";

    for (size_t i = 0; i < strlen(mixed_hex) - 1; i++)
    {
        EXPECT_FALSE(OpenLcbGridConnect_copy_out_gridconnect_when_done(mixed_hex[i], &gridconnect_buffer));
    }
    EXPECT_TRUE(OpenLcbGridConnect_copy_out_gridconnect_when_done(mixed_hex[strlen(mixed_hex) - 1], &gridconnect_buffer));

    EXPECT_STREQ(":X12345678N0189AaBbCcDdEeFf;", (char *)&gridconnect_buffer);
}

/**
 * @brief Test from_can_msg with various payload counts
 * 
 * Verifies:
 * - All valid payload counts (0-8) work correctly
 * - Each produces correct GridConnect format
 */
TEST(OpenLcbGridConnect, all_payload_counts)
{
    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;
    
    can_msg.identifier = 0xABCDEF01;
    
    // Test each payload count from 0 to 8
    for (int count = 0; count <= 8; count++)
    {
        can_msg.payload_count = count;
        for (int i = 0; i < count; i++)
        {
            can_msg.payload[i] = 0x10 + i;  // Distinctive values
        }
        
        OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
        
        // Verify starts with correct header
        EXPECT_EQ(strncmp((char *)&gridconnect_buffer, ":XABCDEF01N", 11), 0)
            << "Header mismatch for count " << count;
        
        // Verify ends with ';'
        size_t len = strlen((char *)&gridconnect_buffer);
        EXPECT_EQ(gridconnect_buffer[len - 1], ';')
            << "Missing terminator for count " << count;
        
        // Verify data length
        EXPECT_EQ(len, 12 + count * 2)
            << "Length mismatch for count " << count;
    }
}

/**
 * @brief Test conversion with all zeros in payload
 * 
 * Verifies:
 * - Zero bytes format correctly with "00"
 * - No skipped or malformed bytes
 */
TEST(OpenLcbGridConnect, all_zero_payload)
{
    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;
    
    can_msg.identifier = 0x12345678;
    can_msg.payload_count = 4;
    can_msg.payload[0] = 0x00;
    can_msg.payload[1] = 0x00;
    can_msg.payload[2] = 0x00;
    can_msg.payload[3] = 0x00;
    
    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    EXPECT_STREQ((char *)&gridconnect_buffer, ":X12345678N00000000;");
    
    // Convert back and verify
    can_msg_t can_msg_back;
    OpenLcbGridConnect_to_can_msg(&gridconnect_buffer, &can_msg_back);
    EXPECT_EQ(can_msg_back.identifier, 0x12345678);
    EXPECT_EQ(can_msg_back.payload_count, 4);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(can_msg_back.payload[i], 0x00);
    }
}

/**
 * @brief Test conversion with all 0xFF in payload
 * 
 * Verifies:
 * - Maximum byte values format correctly
 * - Uppercase FF output
 */
TEST(OpenLcbGridConnect, all_ff_payload)
{
    can_msg_t can_msg;
    gridconnect_buffer_t gridconnect_buffer;
    
    can_msg.identifier = 0xABCDEF01;
    can_msg.payload_count = 3;
    can_msg.payload[0] = 0xFF;
    can_msg.payload[1] = 0xFF;
    can_msg.payload[2] = 0xFF;
    
    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, &can_msg);
    EXPECT_STREQ((char *)&gridconnect_buffer, ":XABCDEF01NFFFFFF;");
    
    // Convert back and verify
    can_msg_t can_msg_back;
    OpenLcbGridConnect_to_can_msg(&gridconnect_buffer, &can_msg_back);
    EXPECT_EQ(can_msg_back.identifier, 0xABCDEF01);
    EXPECT_EQ(can_msg_back.payload_count, 3);
    EXPECT_EQ(can_msg_back.payload[0], 0xFF);
    EXPECT_EQ(can_msg_back.payload[1], 0xFF);
    EXPECT_EQ(can_msg_back.payload[2], 0xFF);
}
