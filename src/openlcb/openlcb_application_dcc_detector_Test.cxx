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
* @file openlcb_application_dcc_detector_Test.cxx
* @brief Unit tests for DCC Detection Protocol Event ID encode/decode helpers
*
* Test Organization:
* - Section 1: Address builder helpers
* - Section 2: Encode event ID
* - Section 3: Extract direction
* - Section 4: Extract address type
* - Section 5: Extract raw address
* - Section 6: Extract DCC address
* - Section 7: Extract detector ID
* - Section 8: Track empty helper
* - Section 9: Spec examples from the Working Note
*
* @author Jim Kueneman
* @date 04 Apr 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_application_dcc_detector.h"


// ============================================================================
// Test constants
// ============================================================================

// Example detector node ID from the spec: 06.80.99.30.03.50
static const node_id_t TEST_DETECTOR_ID = 0x068099300350ULL;


// ============================================================================
// Section 1: Address builder helpers
// ============================================================================

TEST(DccDetector, make_short_address_3)
{

    uint16_t result = OpenLcbApplicationDccDetector_make_short_address(3);

    EXPECT_EQ(result, 0x3803);

}

TEST(DccDetector, make_short_address_0)
{

    uint16_t result = OpenLcbApplicationDccDetector_make_short_address(0);

    EXPECT_EQ(result, DCC_DETECTOR_TRACK_EMPTY);

}

TEST(DccDetector, make_short_address_127)
{

    uint16_t result = OpenLcbApplicationDccDetector_make_short_address(127);

    EXPECT_EQ(result, 0x387F);

}

TEST(DccDetector, make_consist_address_3)
{

    uint16_t result = OpenLcbApplicationDccDetector_make_consist_address(3);

    EXPECT_EQ(result, 0x3903);

}

TEST(DccDetector, make_consist_address_0)
{

    uint16_t result = OpenLcbApplicationDccDetector_make_consist_address(0);

    EXPECT_EQ(result, 0x3900);

}


// ============================================================================
// Section 2: Encode event ID
// ============================================================================

TEST(DccDetector, encode_short_address_3_exit)
{

    // Spec example: 06.80.99.30.03.50.38.03 - 3S leaves
    event_id_t result = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_unoccupied,
            OpenLcbApplicationDccDetector_make_short_address(3));

    EXPECT_EQ(result, 0x0680993003503803ULL);

}

TEST(DccDetector, encode_short_address_3_entry_forward)
{

    // Spec example: direction=forward(0b01), address 3S
    // Tail = 0b01 << 14 | 0x3803 = 0x4000 | 0x3803 = 0x7803
    // But wait -- the spec example shows 0xF803 for "3S arrives"
    // Let's check: 0xF8 = 0b11111000, 0x03 = 0b00000011
    // Upper 2 bits of tail: 0b11 = occupied direction unknown
    // Lower 14 bits: 0b111000 00000011 = 0x3803 = short address 3
    // So the spec example uses direction unknown (0b11), not forward (0b01)

    event_id_t result = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_forward,
            OpenLcbApplicationDccDetector_make_short_address(3));

    // 0b01 << 14 | 0x3803 = 0x4000 | 0x3803 = 0x7803
    EXPECT_EQ(result, 0x0680993003507803ULL);

}

TEST(DccDetector, encode_track_empty_exit)
{

    // Spec example: 06.80.99.30.03.50.38.00 - "track is empty" leaves
    event_id_t result = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_unoccupied,
            DCC_DETECTOR_TRACK_EMPTY);

    EXPECT_EQ(result, 0x0680993003503800ULL);

}

TEST(DccDetector, encode_track_empty_arrives)
{

    // Spec example: 06.80.99.30.03.50.F8.00 - "track is empty" arrives
    // 0xF800 = 0b11111000 00000000
    // Upper 2 bits = 0b11 = occupied direction unknown
    // Lower 14 bits = 0x3800 = track empty
    event_id_t result = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_unknown,
            DCC_DETECTOR_TRACK_EMPTY);

    EXPECT_EQ(result, 0x068099300350F800ULL);

}

TEST(DccDetector, encode_long_address_0)
{

    // Long address 0 (0x0000) -- valid though uncommon
    event_id_t result = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_forward,
            0x0000);

    // Tail: 0b01 << 14 | 0x0000 = 0x4000
    EXPECT_EQ(result, 0x0680993003504000ULL);

}

TEST(DccDetector, encode_long_address_1234)
{

    event_id_t result = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_reverse,
            1234);

    // Tail: 0b10 << 14 | 1234 = 0x8000 | 0x04D2 = 0x84D2
    EXPECT_EQ(result, 0x06809930035084D2ULL);

}

TEST(DccDetector, encode_preserves_only_48_bits_of_node_id)
{

    // Pass a node_id with garbage in the upper 16 bits
    node_id_t big_id = 0xFFFF068099300350ULL;

    event_id_t result = OpenLcbApplicationDccDetector_encode_event_id(
            big_id,
            dcc_detector_unoccupied,
            DCC_DETECTOR_TRACK_EMPTY);

    // Should use only the lower 48 bits
    EXPECT_EQ(result, 0x0680993003503800ULL);

}


// ============================================================================
// Section 3: Extract direction
// ============================================================================

TEST(DccDetector, extract_direction_unoccupied)
{

    event_id_t event = 0x0680993003503803ULL;  // 3S leaves (0b00)

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), dcc_detector_unoccupied);

}

TEST(DccDetector, extract_direction_occupied_forward)
{

    // Tail 0x7803: 0b01 << 14 | 0x3803
    event_id_t event = 0x0680993003507803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), dcc_detector_occupied_forward);

}

TEST(DccDetector, extract_direction_occupied_reverse)
{

    // Tail 0xB803: 0b10 << 14 | 0x3803
    event_id_t event = 0x068099300350B803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), dcc_detector_occupied_reverse);

}

TEST(DccDetector, extract_direction_occupied_unknown)
{

    // Spec example: 0xF803 = 0b11 << 14 | 0x3803
    event_id_t event = 0x068099300350F803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), dcc_detector_occupied_unknown);

}


// ============================================================================
// Section 4: Extract address type
// ============================================================================

TEST(DccDetector, extract_address_type_short)
{

    event_id_t event = 0x0680993003503803ULL;  // short address 3

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_short);

}

TEST(DccDetector, extract_address_type_consist)
{

    // Consist address: 0x3903
    event_id_t event = 0x0680993003503903ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_consist);

}

TEST(DccDetector, extract_address_type_track_empty)
{

    event_id_t event = 0x0680993003503800ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_track_empty);

}

TEST(DccDetector, extract_address_type_long)
{

    // Long address 1234 (0x04D2) -- high byte 0x04, not 0x38 or 0x39
    event_id_t event = 0x06809930035004D2ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_long);

}

TEST(DccDetector, extract_address_type_long_zero)
{

    // Long address 0 (0x0000) -- valid long address, NOT track empty
    event_id_t event = 0x0680993003500000ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_long);

}

TEST(DccDetector, extract_address_type_short_with_direction_bits)
{

    // Short address 3 with direction=occupied_unknown (0b11 in upper 2 bits)
    // Tail = 0xF803 -- direction bits should not affect address type
    event_id_t event = 0x068099300350F803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_short);

}


// ============================================================================
// Section 5: Extract raw address
// ============================================================================

TEST(DccDetector, extract_raw_address_short_3)
{

    event_id_t event = 0x0680993003503803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_raw_address(event), 0x3803);

}

TEST(DccDetector, extract_raw_address_strips_direction_bits)
{

    // Tail = 0xF803 (direction 0b11 + address 0x3803)
    event_id_t event = 0x068099300350F803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_raw_address(event), 0x3803);

}

TEST(DccDetector, extract_raw_address_long_1234)
{

    // Tail = 0x84D2 (direction 0b10 + long address 1234)
    event_id_t event = 0x06809930035084D2ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_raw_address(event), 0x04D2);

}


// ============================================================================
// Section 6: Extract DCC address
// ============================================================================

TEST(DccDetector, extract_dcc_address_short_3)
{

    event_id_t event = 0x0680993003503803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 3);

}

TEST(DccDetector, extract_dcc_address_short_127)
{

    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_unoccupied,
            OpenLcbApplicationDccDetector_make_short_address(127));

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 127);

}

TEST(DccDetector, extract_dcc_address_consist_3)
{

    event_id_t event = 0x0680993003503903ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 3);

}

TEST(DccDetector, extract_dcc_address_long_1234)
{

    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_forward,
            1234);

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 1234);

}

TEST(DccDetector, extract_dcc_address_long_zero)
{

    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_forward,
            0);

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 0);

}

TEST(DccDetector, extract_dcc_address_track_empty)
{

    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_unoccupied,
            DCC_DETECTOR_TRACK_EMPTY);

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 0);

}


// ============================================================================
// Section 7: Extract detector ID
// ============================================================================

TEST(DccDetector, extract_detector_id)
{

    event_id_t event = 0x0680993003503803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_detector_id(event), TEST_DETECTOR_ID);

}

TEST(DccDetector, extract_detector_id_with_direction_bits)
{

    // Direction bits should not affect the detector ID extraction
    event_id_t event = 0x068099300350F803ULL;

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_detector_id(event), TEST_DETECTOR_ID);

}


// ============================================================================
// Section 8: Track empty helper
// ============================================================================

TEST(DccDetector, is_track_empty_true)
{

    event_id_t event = 0x0680993003503800ULL;

    EXPECT_TRUE(OpenLcbApplicationDccDetector_is_track_empty(event));

}

TEST(DccDetector, is_track_empty_true_with_direction_bits)
{

    // Track empty with direction=occupied_unknown (0xF800)
    event_id_t event = 0x068099300350F800ULL;

    EXPECT_TRUE(OpenLcbApplicationDccDetector_is_track_empty(event));

}

TEST(DccDetector, is_track_empty_false_short_address)
{

    event_id_t event = 0x0680993003503803ULL;

    EXPECT_FALSE(OpenLcbApplicationDccDetector_is_track_empty(event));

}

TEST(DccDetector, is_track_empty_false_long_address_zero)
{

    // Long address 0 (0x0000) is NOT track empty
    event_id_t event = 0x0680993003500000ULL;

    EXPECT_FALSE(OpenLcbApplicationDccDetector_is_track_empty(event));

}


// ============================================================================
// Section 9: Spec examples from the Working Note
// ============================================================================

TEST(DccDetector, spec_example_track_empty_leaves)
{

    // 06.80.99.30.03.50.38.00 - "track is empty" leaves
    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_unoccupied,
            DCC_DETECTOR_TRACK_EMPTY);

    EXPECT_EQ(event, 0x0680993003503800ULL);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), dcc_detector_unoccupied);
    EXPECT_TRUE(OpenLcbApplicationDccDetector_is_track_empty(event));

}

TEST(DccDetector, spec_example_3s_arrives)
{

    // 06.80.99.30.03.50.F8.03 - 3S arrives
    // 0xF803 = 0b11 << 14 | 0x3803 = direction unknown + short address 3
    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_unknown,
            OpenLcbApplicationDccDetector_make_short_address(3));

    EXPECT_EQ(event, 0x068099300350F803ULL);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), dcc_detector_occupied_unknown);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_short);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 3);

}

TEST(DccDetector, spec_example_3s_leaves)
{

    // 06.80.99.30.03.50.38.03 - 3S leaves
    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_unoccupied,
            OpenLcbApplicationDccDetector_make_short_address(3));

    EXPECT_EQ(event, 0x0680993003503803ULL);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), dcc_detector_unoccupied);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_short);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 3);

}

TEST(DccDetector, spec_example_track_empty_arrives)
{

    // 06.80.99.30.03.50.F8.00 - "track is empty" arrives
    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_unknown,
            DCC_DETECTOR_TRACK_EMPTY);

    EXPECT_EQ(event, 0x068099300350F800ULL);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), dcc_detector_occupied_unknown);
    EXPECT_TRUE(OpenLcbApplicationDccDetector_is_track_empty(event));

}

TEST(DccDetector, roundtrip_all_directions_short_address)
{

    dcc_detector_direction_enum directions[] = {
        dcc_detector_unoccupied,
        dcc_detector_occupied_forward,
        dcc_detector_occupied_reverse,
        dcc_detector_occupied_unknown
    };

    for (int i = 0; i < 4; i++) {

        uint16_t raw_addr = OpenLcbApplicationDccDetector_make_short_address(42);

        event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
                TEST_DETECTOR_ID, directions[i], raw_addr);

        EXPECT_EQ(OpenLcbApplicationDccDetector_extract_direction(event), directions[i]);
        EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_short);
        EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), 42);
        EXPECT_EQ(OpenLcbApplicationDccDetector_extract_detector_id(event), TEST_DETECTOR_ID);

    }

}

TEST(DccDetector, roundtrip_long_address_max)
{

    // Maximum 14-bit long address: 0x37FF (just below the short prefix range)
    uint16_t max_long = 0x37FF;

    event_id_t event = OpenLcbApplicationDccDetector_encode_event_id(
            TEST_DETECTOR_ID,
            dcc_detector_occupied_forward,
            max_long);

    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_address_type(event), dcc_detector_address_long);
    EXPECT_EQ(OpenLcbApplicationDccDetector_extract_dcc_address(event), max_long);

}
