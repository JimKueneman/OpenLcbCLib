/** \copyright
* Copyright (c) 2024, Jim Kueneman
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
* @file protocol_broadcast_time_handler_Test.cxx
* @brief Unit tests for Broadcast Time Protocol utilities and handler
*
* Test Organization:
* - Section 1: Event ID Detection Tests
* - Section 2: Event Type Classification Tests
* - Section 3: Time Extraction Tests
* - Section 4: Date Extraction Tests
* - Section 5: Year Extraction Tests
* - Section 6: Rate Extraction Tests
* - Section 7: Event ID Creation Tests
* - Section 8: Round-trip Tests (create then extract)
* - Section 9: Protocol Handler Tests
*
* @author Jim Kueneman
* @date 09 Feb 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_utilities.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_buffer_store.h"
#include "openlcb_node.h"
#include "protocol_broadcast_time_handler.h"
#include "openlcb_application_broadcast_time.h"


// ============================================================================
// Section 1: Event ID Detection Tests
// ============================================================================

TEST(BroadcastTime, is_broadcast_time_event_default_fast_clock)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0000;

    EXPECT_TRUE(OpenLcbUtilities_is_broadcast_time_event(event_id));

}

TEST(BroadcastTime, is_broadcast_time_event_realtime_clock)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK | 0x0000;

    EXPECT_TRUE(OpenLcbUtilities_is_broadcast_time_event(event_id));

}

TEST(BroadcastTime, is_broadcast_time_event_alternate_clock_1)
{

    event_id_t event_id = BROADCAST_TIME_ID_ALTERNATE_CLOCK_1 | 0x0000;

    EXPECT_TRUE(OpenLcbUtilities_is_broadcast_time_event(event_id));

}

TEST(BroadcastTime, is_broadcast_time_event_alternate_clock_2)
{

    event_id_t event_id = BROADCAST_TIME_ID_ALTERNATE_CLOCK_2 | 0x0000;

    EXPECT_TRUE(OpenLcbUtilities_is_broadcast_time_event(event_id));

}

TEST(BroadcastTime, is_broadcast_time_event_not_a_clock)
{

    event_id_t event_id = 0x0505050505050000ULL;

    EXPECT_FALSE(OpenLcbUtilities_is_broadcast_time_event(event_id));

}

TEST(BroadcastTime, is_broadcast_time_event_with_time_data)
{

    // Default fast clock reporting 14:30
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | ((uint64_t) 14 << 8) | 30;

    EXPECT_TRUE(OpenLcbUtilities_is_broadcast_time_event(event_id));

}


// ============================================================================
// Section 2: Clock ID Extraction Tests
// ============================================================================

TEST(BroadcastTime, extract_clock_id_default_fast)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x1234;

    uint64_t clock_id = OpenLcbUtilities_extract_clock_id_from_time_event(event_id);

    EXPECT_EQ(clock_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

}

TEST(BroadcastTime, extract_clock_id_alternate_1)
{

    event_id_t event_id = BROADCAST_TIME_ID_ALTERNATE_CLOCK_1 | 0xABCD;

    uint64_t clock_id = OpenLcbUtilities_extract_clock_id_from_time_event(event_id);

    EXPECT_EQ(clock_id, BROADCAST_TIME_ID_ALTERNATE_CLOCK_1);

}


// ============================================================================
// Section 3: Event Type Classification Tests
// ============================================================================

TEST(BroadcastTime, get_event_type_report_time)
{

    // Report Time: hour=10, minute=30 -> 0x0A1E
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0A1E;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_REPORT_TIME);

}

TEST(BroadcastTime, get_event_type_report_time_midnight)
{

    // Report Time: hour=0, minute=0 -> 0x0000
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0000;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_REPORT_TIME);

}

TEST(BroadcastTime, get_event_type_report_date)
{

    // Report Date: month=6, day=15 -> 0x260F
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x260F;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_REPORT_DATE);

}

TEST(BroadcastTime, get_event_type_report_year)
{

    // Report Year: year=2026 -> 0x3000 + 2026 = 0x37EA
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x37EA;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_REPORT_YEAR);

}

TEST(BroadcastTime, get_event_type_report_rate)
{

    // Report Rate: 0x4004 (rate = 1.00)
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x4004;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_REPORT_RATE);

}

TEST(BroadcastTime, get_event_type_set_time)
{

    // Set Time: 0x8000 + hour/min
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x8A1E;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_SET_TIME);

}

TEST(BroadcastTime, get_event_type_set_date)
{

    // Set Date: 0xA100 + date encoding
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xA60F;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_SET_DATE);

}

TEST(BroadcastTime, get_event_type_set_year)
{

    // Set Year: 0xB000 + year
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xB7EA;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_SET_YEAR);

}

TEST(BroadcastTime, get_event_type_set_rate)
{

    // Set Rate: 0xC000 + rate
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xC004;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_SET_RATE);

}

TEST(BroadcastTime, get_event_type_query)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | BROADCAST_TIME_QUERY;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_QUERY);

}

TEST(BroadcastTime, get_event_type_stop)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | BROADCAST_TIME_STOP;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_STOP);

}

TEST(BroadcastTime, get_event_type_start)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | BROADCAST_TIME_START;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_START);

}

TEST(BroadcastTime, get_event_type_date_rollover)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | BROADCAST_TIME_DATE_ROLLOVER;

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_DATE_ROLLOVER);

}


// ============================================================================
// Section 4: Time Extraction Tests
// ============================================================================

TEST(BroadcastTime, extract_time_midnight)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0000;

    uint8_t hour, minute;

    EXPECT_TRUE(OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute));
    EXPECT_EQ(hour, 0);
    EXPECT_EQ(minute, 0);

}

TEST(BroadcastTime, extract_time_14_30)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E;

    uint8_t hour, minute;

    EXPECT_TRUE(OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute));
    EXPECT_EQ(hour, 14);
    EXPECT_EQ(minute, 30);

}

TEST(BroadcastTime, extract_time_23_59)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x173B;

    uint8_t hour, minute;

    EXPECT_TRUE(OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute));
    EXPECT_EQ(hour, 23);
    EXPECT_EQ(minute, 59);

}

TEST(BroadcastTime, extract_time_from_set_command)
{

    // Set Time for 14:30 = 0x8000 + 0x0E1E = 0x8E1E
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x8E1E;

    uint8_t hour, minute;

    EXPECT_TRUE(OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute));
    EXPECT_EQ(hour, 14);
    EXPECT_EQ(minute, 30);

}

TEST(BroadcastTime, extract_time_null_pointers)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E;

    uint8_t hour, minute;

    EXPECT_FALSE(OpenLcbUtilities_extract_time_from_event_id(event_id, NULL, &minute));
    EXPECT_FALSE(OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, NULL));

}


// ============================================================================
// Section 5: Date Extraction Tests
// ============================================================================

TEST(BroadcastTime, extract_date_jan_1)
{

    // Month 1 = 0x21, Day 1 -> 0x2101
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x2101;

    uint8_t month, day;

    EXPECT_TRUE(OpenLcbUtilities_extract_date_from_event_id(event_id, &month, &day));
    EXPECT_EQ(month, 1);
    EXPECT_EQ(day, 1);

}

TEST(BroadcastTime, extract_date_dec_31)
{

    // Month 12 = 0x2C, Day 31 -> 0x2C1F
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x2C1F;

    uint8_t month, day;

    EXPECT_TRUE(OpenLcbUtilities_extract_date_from_event_id(event_id, &month, &day));
    EXPECT_EQ(month, 12);
    EXPECT_EQ(day, 31);

}

TEST(BroadcastTime, extract_date_jun_15)
{

    // Month 6 = 0x26, Day 15 -> 0x260F
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x260F;

    uint8_t month, day;

    EXPECT_TRUE(OpenLcbUtilities_extract_date_from_event_id(event_id, &month, &day));
    EXPECT_EQ(month, 6);
    EXPECT_EQ(day, 15);

}

TEST(BroadcastTime, extract_date_from_set_command)
{

    // Set Date for Jun 15 = 0x8000 + 0x260F = 0xA60F
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xA60F;

    uint8_t month, day;

    EXPECT_TRUE(OpenLcbUtilities_extract_date_from_event_id(event_id, &month, &day));
    EXPECT_EQ(month, 6);
    EXPECT_EQ(day, 15);

}

TEST(BroadcastTime, extract_date_null_pointers)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x2101;

    uint8_t month, day;

    EXPECT_FALSE(OpenLcbUtilities_extract_date_from_event_id(event_id, NULL, &day));
    EXPECT_FALSE(OpenLcbUtilities_extract_date_from_event_id(event_id, &month, NULL));

}


// ============================================================================
// Section 6: Year Extraction Tests
// ============================================================================

TEST(BroadcastTime, extract_year_zero)
{

    // Year 0 -> 0x3000
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x3000;

    uint16_t year;

    EXPECT_TRUE(OpenLcbUtilities_extract_year_from_event_id(event_id, &year));
    EXPECT_EQ(year, 0);

}

TEST(BroadcastTime, extract_year_2026)
{

    // Year 2026 -> 0x3000 + 2026 = 0x37EA
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x37EA;

    uint16_t year;

    EXPECT_TRUE(OpenLcbUtilities_extract_year_from_event_id(event_id, &year));
    EXPECT_EQ(year, 2026);

}

TEST(BroadcastTime, extract_year_4095)
{

    // Year 4095 -> 0x3000 + 4095 = 0x3FFF
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x3FFF;

    uint16_t year;

    EXPECT_TRUE(OpenLcbUtilities_extract_year_from_event_id(event_id, &year));
    EXPECT_EQ(year, 4095);

}

TEST(BroadcastTime, extract_year_from_set_command)
{

    // Set Year 2026 = 0x8000 + 0x37EA = 0xB7EA
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xB7EA;

    uint16_t year;

    EXPECT_TRUE(OpenLcbUtilities_extract_year_from_event_id(event_id, &year));
    EXPECT_EQ(year, 2026);

}

TEST(BroadcastTime, extract_year_null_pointer)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x37EA;

    EXPECT_FALSE(OpenLcbUtilities_extract_year_from_event_id(event_id, NULL));

}


// ============================================================================
// Section 7: Rate Extraction Tests
// ============================================================================

TEST(BroadcastTime, extract_rate_realtime)
{

    // Rate 1.00 = 0x0004, event = 0x4004
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x4004;

    int16_t rate;

    EXPECT_TRUE(OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate));
    EXPECT_EQ(rate, 0x0004);

}

TEST(BroadcastTime, extract_rate_4x)
{

    // Rate 4.00 = 0x0010, event = 0x4010
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x4010;

    int16_t rate;

    EXPECT_TRUE(OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate));
    EXPECT_EQ(rate, 0x0010);

}

TEST(BroadcastTime, extract_rate_negative)
{

    // Rate -1.00 = 0xFFFC (12-bit), event = 0x4FFC
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x4FFC;

    int16_t rate;

    EXPECT_TRUE(OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate));
    EXPECT_EQ(rate, (int16_t) 0xFFFC);

}

TEST(BroadcastTime, extract_rate_zero)
{

    // Rate 0 = 0x0000, event = 0x4000
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x4000;

    int16_t rate;

    EXPECT_TRUE(OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate));
    EXPECT_EQ(rate, 0);

}

TEST(BroadcastTime, extract_rate_from_set_command)
{

    // Set Rate 4.00 = 0xC010
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xC010;

    int16_t rate;

    EXPECT_TRUE(OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate));
    EXPECT_EQ(rate, 0x0010);

}

TEST(BroadcastTime, extract_rate_null_pointer)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x4004;

    EXPECT_FALSE(OpenLcbUtilities_extract_rate_from_event_id(event_id, NULL));

}


// ============================================================================
// Section 8: Event ID Creation Tests
// ============================================================================

TEST(BroadcastTime, create_time_event_report)
{

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30, false);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E);

}

TEST(BroadcastTime, create_time_event_set)
{

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30, true);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x8E1E);

}

TEST(BroadcastTime, create_date_event_report)
{

    event_id_t event_id = OpenLcbUtilities_create_date_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15, false);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x260F);

}

TEST(BroadcastTime, create_date_event_set)
{

    event_id_t event_id = OpenLcbUtilities_create_date_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15, true);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xA60F);

}

TEST(BroadcastTime, create_year_event_report)
{

    event_id_t event_id = OpenLcbUtilities_create_year_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026, false);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x37EA);

}

TEST(BroadcastTime, create_year_event_set)
{

    event_id_t event_id = OpenLcbUtilities_create_year_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026, true);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xB7EA);

}

TEST(BroadcastTime, create_rate_event_report)
{

    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010, false);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x4010);

}

TEST(BroadcastTime, create_rate_event_set)
{

    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010, true);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0xC010);

}

TEST(BroadcastTime, create_rate_event_negative)
{

    // -1.00 = 0xFFFC as int16_t, lower 12 bits = 0xFFC
    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, (int16_t) 0xFFFC, false);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x4FFC);

}

TEST(BroadcastTime, create_command_query)
{

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_QUERY);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | BROADCAST_TIME_QUERY);

}

TEST(BroadcastTime, create_command_stop)
{

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_STOP);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | BROADCAST_TIME_STOP);

}

TEST(BroadcastTime, create_command_start)
{

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_START);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | BROADCAST_TIME_START);

}

TEST(BroadcastTime, create_command_date_rollover)
{

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_DATE_ROLLOVER);

    EXPECT_EQ(event_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | BROADCAST_TIME_DATE_ROLLOVER);

}


// ============================================================================
// Section 9: Round-trip Tests (create then extract)
// ============================================================================

TEST(BroadcastTime, roundtrip_time)
{

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 23, 59, false);

    uint8_t hour, minute;

    EXPECT_TRUE(OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute));
    EXPECT_EQ(hour, 23);
    EXPECT_EQ(minute, 59);

}

TEST(BroadcastTime, roundtrip_time_set)
{

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_ALTERNATE_CLOCK_2, 8, 15, true);

    uint8_t hour, minute;

    EXPECT_TRUE(OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute));
    EXPECT_EQ(hour, 8);
    EXPECT_EQ(minute, 15);

    EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_SET_TIME);
    EXPECT_EQ(OpenLcbUtilities_extract_clock_id_from_time_event(event_id), BROADCAST_TIME_ID_ALTERNATE_CLOCK_2);

}

TEST(BroadcastTime, roundtrip_date)
{

    event_id_t event_id = OpenLcbUtilities_create_date_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 12, 25, false);

    uint8_t month, day;

    EXPECT_TRUE(OpenLcbUtilities_extract_date_from_event_id(event_id, &month, &day));
    EXPECT_EQ(month, 12);
    EXPECT_EQ(day, 25);

}

TEST(BroadcastTime, roundtrip_year)
{

    event_id_t event_id = OpenLcbUtilities_create_year_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 1955, false);

    uint16_t year;

    EXPECT_TRUE(OpenLcbUtilities_extract_year_from_event_id(event_id, &year));
    EXPECT_EQ(year, 1955);

}

TEST(BroadcastTime, roundtrip_rate_positive)
{

    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0028, false);

    int16_t rate;

    EXPECT_TRUE(OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate));
    EXPECT_EQ(rate, 0x0028);

}

TEST(BroadcastTime, roundtrip_rate_negative)
{

    // -4.00 = 0xFFF0 as int16_t
    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, (int16_t) 0xFFF0, false);

    int16_t rate;

    EXPECT_TRUE(OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate));
    EXPECT_EQ(rate, (int16_t) 0xFFF0);

}

TEST(BroadcastTime, roundtrip_all_clocks)
{

    uint64_t clocks[] = {
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK,
        BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK,
        BROADCAST_TIME_ID_ALTERNATE_CLOCK_1,
        BROADCAST_TIME_ID_ALTERNATE_CLOCK_2
    };

    for (int i = 0; i < 4; i++) {

        event_id_t event_id = OpenLcbUtilities_create_time_event_id(clocks[i], 12, 0, false);

        EXPECT_TRUE(OpenLcbUtilities_is_broadcast_time_event(event_id));
        EXPECT_EQ(OpenLcbUtilities_extract_clock_id_from_time_event(event_id), clocks[i]);
        EXPECT_EQ(OpenLcbUtilities_get_broadcast_time_event_type(event_id), BROADCAST_TIME_EVENT_REPORT_TIME);

        uint8_t hour, minute;

        EXPECT_TRUE(OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute));
        EXPECT_EQ(hour, 12);
        EXPECT_EQ(minute, 0);

    }

}


// ============================================================================
// Section 10: Protocol Handler Tests
// ============================================================================

// Callback tracking variables
static bool g_time_callback_called = false;
static bool g_date_callback_called = false;
static bool g_year_callback_called = false;
static bool g_rate_callback_called = false;
static bool g_started_callback_called = false;
static bool g_stopped_callback_called = false;
static bool g_rollover_callback_called = false;
static broadcast_clock_state_t g_last_clock_state;

static void _reset_callback_flags() {

    g_time_callback_called = false;
    g_date_callback_called = false;
    g_year_callback_called = false;
    g_rate_callback_called = false;
    g_started_callback_called = false;
    g_stopped_callback_called = false;
    g_rollover_callback_called = false;
    memset(&g_last_clock_state, 0, sizeof(broadcast_clock_state_t));

}

static void _on_time_received(openlcb_node_t *node, broadcast_clock_state_t *state) {

    g_time_callback_called = true;
    g_last_clock_state = *state;

}

static void _on_date_received(openlcb_node_t *node, broadcast_clock_state_t *state) {

    g_date_callback_called = true;
    g_last_clock_state = *state;

}

static void _on_year_received(openlcb_node_t *node, broadcast_clock_state_t *state) {

    g_year_callback_called = true;
    g_last_clock_state = *state;

}

static void _on_rate_received(openlcb_node_t *node, broadcast_clock_state_t *state) {

    g_rate_callback_called = true;
    g_last_clock_state = *state;

}

static void _on_clock_started(openlcb_node_t *node, broadcast_clock_state_t *state) {

    g_started_callback_called = true;
    g_last_clock_state = *state;

}

static void _on_clock_stopped(openlcb_node_t *node, broadcast_clock_state_t *state) {

    g_stopped_callback_called = true;
    g_last_clock_state = *state;

}

static void _on_date_rollover(openlcb_node_t *node, broadcast_clock_state_t *state) {

    g_rollover_callback_called = true;
    g_last_clock_state = *state;

}

static const interface_openlcb_protocol_broadcast_time_handler_t _test_broadcast_time_interface = {

    .on_time_received = _on_time_received,
    .on_date_received = _on_date_received,
    .on_year_received = _on_year_received,
    .on_rate_received = _on_rate_received,
    .on_clock_started = _on_clock_started,
    .on_clock_stopped = _on_clock_stopped,
    .on_date_rollover = _on_date_rollover,

};

static const interface_openlcb_application_broadcast_time_t _test_app_broadcast_time_interface = {
    .on_time_changed = NULL,
};

TEST(BroadcastTimeHandler, handle_report_time)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30, false);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_TRUE(g_time_callback_called);
    EXPECT_EQ(cs->time.hour, 14);
    EXPECT_EQ(cs->time.minute, 30);
    EXPECT_EQ(cs->time.valid, 1);

}

TEST(BroadcastTimeHandler, handle_report_date)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_date_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 12, 25, false);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_TRUE(g_date_callback_called);
    EXPECT_EQ(cs->date.month, 12);
    EXPECT_EQ(cs->date.day, 25);
    EXPECT_EQ(cs->date.valid, 1);

}

TEST(BroadcastTimeHandler, handle_report_year)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_year_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 1955, false);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_TRUE(g_year_callback_called);
    EXPECT_EQ(cs->year.year, 1955);
    EXPECT_EQ(cs->year.valid, 1);

}

TEST(BroadcastTimeHandler, handle_report_rate)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010, false);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_TRUE(g_rate_callback_called);
    EXPECT_EQ(cs->rate.rate, 0x0010);
    EXPECT_EQ(cs->rate.valid, 1);

}

TEST(BroadcastTimeHandler, handle_start)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    cs->is_running = 0;

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_START);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_TRUE(g_started_callback_called);
    EXPECT_EQ(cs->is_running, 1);

}

TEST(BroadcastTimeHandler, handle_stop)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    cs->is_running = 1;

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_STOP);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_TRUE(g_stopped_callback_called);
    EXPECT_EQ(cs->is_running, 0);

}

TEST(BroadcastTimeHandler, handle_date_rollover)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_DATE_ROLLOVER);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_TRUE(g_rollover_callback_called);

}

TEST(BroadcastTimeHandler, not_clock_consumer_ignores)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    // Do NOT call setup_consumer — clock slot will not exist

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30, false);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_FALSE(g_time_callback_called);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), nullptr);

}

TEST(BroadcastTimeHandler, null_statemachine_info)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30, false);

    ProtocolBroadcastTime_handle_time_event(NULL, event_id);

    EXPECT_FALSE(g_time_callback_called);

}

TEST(BroadcastTimeHandler, set_time_updates_state)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    // Set Time should also update clock state (consumers treat Set same as Report)
    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 8, 45, true);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_TRUE(g_time_callback_called);
    EXPECT_EQ(cs->time.hour, 8);
    EXPECT_EQ(cs->time.minute, 45);

}

TEST(BroadcastTimeHandler, null_node_pointer)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = NULL;

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30, false);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_FALSE(g_time_callback_called);

}

TEST(BroadcastTimeHandler, set_date_updates_state)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_date_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 7, 4, true);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_TRUE(g_date_callback_called);
    EXPECT_EQ(cs->date.month, 7);
    EXPECT_EQ(cs->date.day, 4);
    EXPECT_EQ(cs->date.valid, 1);

}

TEST(BroadcastTimeHandler, set_year_updates_state)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_year_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026, true);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_TRUE(g_year_callback_called);
    EXPECT_EQ(cs->year.year, 2026);
    EXPECT_EQ(cs->year.valid, 1);

}

TEST(BroadcastTimeHandler, set_rate_updates_state)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0028, true);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_TRUE(g_rate_callback_called);
    EXPECT_EQ(cs->rate.rate, 0x0028);
    EXPECT_EQ(cs->rate.valid, 1);

}

TEST(BroadcastTimeHandler, handle_query_no_action)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_QUERY);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_FALSE(g_time_callback_called);
    EXPECT_FALSE(g_date_callback_called);
    EXPECT_FALSE(g_year_callback_called);
    EXPECT_FALSE(g_rate_callback_called);
    EXPECT_FALSE(g_started_callback_called);
    EXPECT_FALSE(g_stopped_callback_called);
    EXPECT_FALSE(g_rollover_callback_called);

}

TEST(BroadcastTimeHandler, handle_unknown_event_type)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    // Use a value in the gap between ranges to trigger UNKNOWN
    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x1800;

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_FALSE(g_time_callback_called);
    EXPECT_FALSE(g_date_callback_called);
    EXPECT_FALSE(g_year_callback_called);
    EXPECT_FALSE(g_rate_callback_called);
    EXPECT_FALSE(g_started_callback_called);
    EXPECT_FALSE(g_stopped_callback_called);
    EXPECT_FALSE(g_rollover_callback_called);

}

TEST(BroadcastTimeHandler, null_interface_no_crash)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(NULL);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    // Report Time — should update state but not crash on null callback
    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 10, 15, false);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_EQ(cs->time.hour, 10);
    EXPECT_EQ(cs->time.minute, 15);
    EXPECT_EQ(cs->time.valid, 1);

    // Report Date
    event_id = OpenLcbUtilities_create_date_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 3, 14, false);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->date.month, 3);
    EXPECT_EQ(cs->date.day, 14);
    EXPECT_EQ(cs->date.valid, 1);

    // Report Year
    event_id = OpenLcbUtilities_create_year_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2000, false);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->year.year, 2000);
    EXPECT_EQ(cs->year.valid, 1);

    // Report Rate
    event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0004, false);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->rate.rate, 0x0004);
    EXPECT_EQ(cs->rate.valid, 1);

    // Start
    event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_START);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->is_running, 1);

    // Stop
    event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_STOP);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->is_running, 0);

    // Date Rollover — just verify no crash
    event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_DATE_ROLLOVER);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

}

static const interface_openlcb_protocol_broadcast_time_handler_t _test_null_callbacks_interface = {

    .on_time_received = NULL,
    .on_date_received = NULL,
    .on_year_received = NULL,
    .on_rate_received = NULL,
    .on_clock_started = NULL,
    .on_clock_stopped = NULL,
    .on_date_rollover = NULL,

};

TEST(BroadcastTimeHandler, null_callbacks_no_crash)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_null_callbacks_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    // Report Time with null callback
    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 10, 15, false);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(cs, nullptr);
    EXPECT_EQ(cs->time.hour, 10);
    EXPECT_EQ(cs->time.minute, 15);

    // Report Date with null callback
    event_id = OpenLcbUtilities_create_date_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 3, 14, false);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->date.month, 3);
    EXPECT_EQ(cs->date.day, 14);

    // Report Year with null callback
    event_id = OpenLcbUtilities_create_year_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2000, false);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->year.year, 2000);

    // Report Rate with null callback
    event_id = OpenLcbUtilities_create_rate_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0004, false);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->rate.rate, 0x0004);

    // Start with null callback
    event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_START);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->is_running, 1);

    // Stop with null callback
    event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_STOP);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    EXPECT_EQ(cs->is_running, 0);

    // Date Rollover with null callback — just verify no crash
    event_id = OpenLcbUtilities_create_command_event_id(
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_DATE_ROLLOVER);
    ProtocolBroadcastTime_handle_time_event(&info, event_id);

}

TEST(BroadcastTimeHandler, clock_id_updated_in_state)
{

    _reset_callback_flags();

    OpenLcbBufferStore_initialize();

    ProtocolBroadcastTime_initialize(&_test_broadcast_time_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);

    openlcb_node_t node;
    memset(&node, 0, sizeof(openlcb_node_t));

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(openlcb_statemachine_info_t));
    info.openlcb_node = &node;

    // Setup consumer for ALTERNATE_CLOCK_1 and send a time event
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_ALTERNATE_CLOCK_1);

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_ALTERNATE_CLOCK_1, 12, 0, false);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs1 = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_ALTERNATE_CLOCK_1);
    ASSERT_NE(cs1, nullptr);
    EXPECT_TRUE(g_time_callback_called);
    EXPECT_EQ(cs1->time.hour, 12);
    EXPECT_EQ(cs1->time.minute, 0);

    // Setup consumer for DEFAULT_REALTIME_CLOCK and send a time event
    _reset_callback_flags();
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK);

    event_id = OpenLcbUtilities_create_time_event_id(
        BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK, 6, 30, false);

    ProtocolBroadcastTime_handle_time_event(&info, event_id);

    broadcast_clock_state_t *cs2 = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK);
    ASSERT_NE(cs2, nullptr);
    EXPECT_TRUE(g_time_callback_called);
    EXPECT_EQ(cs2->time.hour, 6);
    EXPECT_EQ(cs2->time.minute, 30);

}
