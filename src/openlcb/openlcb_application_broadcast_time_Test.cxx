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
* @file openlcb_application_broadcast_time_Test.cxx
* @brief Unit tests for the Broadcast Time Application module
*
* Test Organization:
* - Section 1: Initialize Tests
* - Section 2: Setup Consumer/Producer Tests
* - Section 3: Accessor Tests (get_clock, is_consumer, is_producer)
* - Section 4: Time Tick Tests (forward advancement)
* - Section 5: Time Tick Tests (backward advancement)
* - Section 6: Producer Send Function Tests
* - Section 7: Consumer Send Function Tests
* - Section 8: Controller Send Function Tests
* - Section 9: Query Reply (State Machine) Tests
* - Section 10: Edge Cases
*
* @author Jim Kueneman
* @date 14 Feb 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_application_broadcast_time.h"
#include "openlcb_application.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "protocol_broadcast_time_handler.h"


// ============================================================================
// Test Constants
// ============================================================================

#define TEST_SOURCE_ALIAS 0x222
#define TEST_SOURCE_ID 0x010203040506
#define TEST_DEST_ALIAS 0xBBB
#define TEST_DEST_ID 0x060504030201
#define TEST_CONFIG_MEM_START_ADDRESS 0x100
#define TEST_CONFIG_MEM_NODE_ADDRESS_ALLOCATION 0x200


// ============================================================================
// Test Data Structures
// ============================================================================

static bool fail_transmit = false;
static int fail_after_count = -1;  // -1 means disabled; otherwise fail after N successful sends
static uint16_t last_sent_mti = 0;
static event_id_t last_sent_event_id = 0;
static int send_count = 0;

static node_parameters_t _test_node_parameters = {

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

    .snip.mfg_version = 4,
    .snip.name = "Test Node",
    .snip.model = "Test Model",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2,

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .configuration_options.read_from_manufacturer_space_0xfc_supported = 1,
    .configuration_options.read_from_user_space_0xfb_supported = 1,
    .configuration_options.stream_read_write_supported = 0,
    .configuration_options.unaligned_reads_supported = 1,
    .configuration_options.unaligned_writes_supported = 1,
    .configuration_options.write_to_user_space_0xfb_supported = 1,
    .configuration_options.write_under_mask_supported = 1,
    .configuration_options.description = "Memory space capabilities",

    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 0,
    .address_space_configuration_definition.low_address_valid = 0,
    .address_space_configuration_definition.low_address = 0,
    .address_space_configuration_definition.highest_address = 0x200,
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0,
    .address_space_all.low_address = 0,
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,
    .address_space_config_memory.low_address = 0,
    .address_space_config_memory.highest_address = TEST_CONFIG_MEM_NODE_ADDRESS_ALLOCATION,
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory"

};


// ============================================================================
// Mock Functions
// ============================================================================

static bool _mock_transmit_openlcb_message(openlcb_msg_t *openlcb_msg) {

    if (fail_transmit) {

        return false;

    }

    if (fail_after_count >= 0 && send_count >= fail_after_count) {

        return false;

    }

    last_sent_mti = openlcb_msg->mti;
    last_sent_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);
    send_count++;

    return true;

}

static uint16_t _mock_configuration_memory_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

    return count;

}

static uint16_t _mock_configuration_memory_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

    return count;

}


// ============================================================================
// Interface Structures
// ============================================================================

static interface_openlcb_application_t _test_application_interface = {

    .send_openlcb_msg = &_mock_transmit_openlcb_message,
    .config_memory_read = &_mock_configuration_memory_read,
    .config_memory_write = &_mock_configuration_memory_write

};

static interface_openlcb_node_t _test_node_interface = {};


// ============================================================================
// Callback tracking for protocol handler interface
// ============================================================================

static bool callback_time_received = false;
static bool callback_date_received = false;
static bool callback_year_received = false;
static bool callback_rate_received = false;
static bool callback_clock_started = false;
static bool callback_clock_stopped = false;
static bool callback_date_rollover = false;

static void _test_on_time_received(openlcb_node_t *node, broadcast_clock_state_t *clock_state) {

    callback_time_received = true;

}

static void _test_on_date_received(openlcb_node_t *node, broadcast_clock_state_t *clock_state) {

    callback_date_received = true;

}

static void _test_on_year_received(openlcb_node_t *node, broadcast_clock_state_t *clock_state) {

    callback_year_received = true;

}

static void _test_on_rate_received(openlcb_node_t *node, broadcast_clock_state_t *clock_state) {

    callback_rate_received = true;

}

static void _test_on_clock_started(openlcb_node_t *node, broadcast_clock_state_t *clock_state) {

    callback_clock_started = true;

}

static void _test_on_clock_stopped(openlcb_node_t *node, broadcast_clock_state_t *clock_state) {

    callback_clock_stopped = true;

}

static void _test_on_date_rollover(openlcb_node_t *node, broadcast_clock_state_t *clock_state) {

    callback_date_rollover = true;

}

static const interface_openlcb_protocol_broadcast_time_handler_t _test_handler_interface = {

    .on_time_received = _test_on_time_received,
    .on_date_received = _test_on_date_received,
    .on_year_received = _test_on_year_received,
    .on_rate_received = _test_on_rate_received,
    .on_clock_started = _test_on_clock_started,
    .on_clock_stopped = _test_on_clock_stopped,
    .on_date_rollover = _test_on_date_rollover,

};

static bool callback_app_time_changed = false;

static void _test_on_app_time_changed(broadcast_clock_t *clock) {
    callback_app_time_changed = true;
}

static const interface_openlcb_application_broadcast_time_t _test_app_broadcast_time_interface = {
    .on_time_changed = _test_on_app_time_changed,
};

static const interface_openlcb_application_broadcast_time_t _test_app_broadcast_time_null_callback_interface = {
    .on_time_changed = NULL,
};


// ============================================================================
// Setup / Teardown
// ============================================================================

static void _reset_test_state(void) {

    fail_transmit = false;
    fail_after_count = -1;
    last_sent_mti = 0;
    last_sent_event_id = 0;
    send_count = 0;
    callback_time_received = false;
    callback_date_received = false;
    callback_year_received = false;
    callback_rate_received = false;
    callback_clock_started = false;
    callback_clock_stopped = false;
    callback_date_rollover = false;
    callback_app_time_changed = false;

}

static void _full_initialize(void) {

    OpenLcbApplication_initialize(&_test_application_interface);
    OpenLcbNode_initialize(&_test_node_interface);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolBroadcastTime_initialize(&_test_handler_interface);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);

}




// ============================================================================
// Section 1: Initialize Tests
// ============================================================================

TEST(BroadcastTimeApp, initialize_clears_all_clocks)
{

    _reset_test_state();
    _full_initialize();

    // After initialize, no clocks should be allocated
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), nullptr);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK), nullptr);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_ALTERNATE_CLOCK_1), nullptr);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_ALTERNATE_CLOCK_2), nullptr);

}

TEST(BroadcastTimeApp, initialize_resets_previously_setup_clocks)
{

    _reset_test_state();
    _full_initialize();

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), nullptr);

    // Re-initialize should clear all
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_interface);

    EXPECT_EQ(OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), nullptr);

}


// ============================================================================
// Section 2: Setup Consumer/Producer Tests
// ============================================================================

TEST(BroadcastTimeApp, setup_consumer_returns_clock_state)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    ASSERT_NE(clock_state, nullptr);
    EXPECT_EQ(clock_state->clock_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

}

TEST(BroadcastTimeApp, setup_producer_returns_clock_state)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    ASSERT_NE(clock_state, nullptr);
    EXPECT_EQ(clock_state->clock_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

}

TEST(BroadcastTimeApp, setup_consumer_with_node_registers_ranges)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    ASSERT_NE(clock_state, nullptr);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_consumer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 1);

}

TEST(BroadcastTimeApp, setup_producer_with_node_registers_ranges)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    ASSERT_NE(clock_state, nullptr);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_producer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 1);

}

TEST(BroadcastTimeApp, setup_consumer_same_clock_twice_returns_same_state)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *first = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    broadcast_clock_state_t *second = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_EQ(first, second);

}

TEST(BroadcastTimeApp, setup_multiple_clocks)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *fast_clock = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    broadcast_clock_state_t *realtime_clock = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK);
    broadcast_clock_state_t *alt_clock_1 = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_ALTERNATE_CLOCK_1);
    broadcast_clock_state_t *alt_clock_2 = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_ALTERNATE_CLOCK_2);

    ASSERT_NE(fast_clock, nullptr);
    ASSERT_NE(realtime_clock, nullptr);
    ASSERT_NE(alt_clock_1, nullptr);
    ASSERT_NE(alt_clock_2, nullptr);

    EXPECT_NE(fast_clock, realtime_clock);
    EXPECT_NE(fast_clock, alt_clock_1);
    EXPECT_NE(fast_clock, alt_clock_2);

}

TEST(BroadcastTimeApp, setup_consumer_and_producer_same_clock)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *consumer_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    broadcast_clock_state_t *producer_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    // Should return the same clock state
    EXPECT_EQ(consumer_state, producer_state);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_consumer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 1);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_producer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 1);

}

TEST(BroadcastTimeApp, setup_overflow_returns_null)
{

    _reset_test_state();
    _full_initialize();

    // Allocate all available slots (BROADCAST_TIME_TOTAL_CLOCK_COUNT = 8)
    for (int i = 0; i < BROADCAST_TIME_TOTAL_CLOCK_COUNT; i++) {

        event_id_t clock_id = 0x0101000002000000ULL + ((uint64_t)(i) << 16);
        broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(NULL, clock_id);
        ASSERT_NE(clock_state, nullptr);

    }

    // Next allocation should fail
    event_id_t overflow_clock_id = 0x0101000003000000ULL;
    broadcast_clock_state_t *overflow_state = OpenLcbApplicationBroadcastTime_setup_consumer(NULL, overflow_clock_id);
    EXPECT_EQ(overflow_state, nullptr);

}


// ============================================================================
// Section 3: Accessor Tests
// ============================================================================

TEST(BroadcastTimeApp, get_clock_returns_null_for_unregistered)
{

    _reset_test_state();
    _full_initialize();

    EXPECT_EQ(OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), nullptr);

}

TEST(BroadcastTimeApp, get_clock_returns_state_for_registered)
{

    _reset_test_state();
    _full_initialize();

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    ASSERT_NE(clock_state, nullptr);
    EXPECT_EQ(clock_state->clock_id, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

}

TEST(BroadcastTimeApp, is_consumer_returns_zero_for_unregistered)
{

    _reset_test_state();
    _full_initialize();

    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_consumer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 0);

}

TEST(BroadcastTimeApp, is_consumer_returns_one_for_consumer)
{

    _reset_test_state();
    _full_initialize();

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_consumer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 1);

}

TEST(BroadcastTimeApp, is_consumer_returns_zero_for_producer_only)
{

    _reset_test_state();
    _full_initialize();

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_consumer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 0);

}

TEST(BroadcastTimeApp, is_producer_returns_zero_for_unregistered)
{

    _reset_test_state();
    _full_initialize();

    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_producer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 0);

}

TEST(BroadcastTimeApp, is_producer_returns_one_for_producer)
{

    _reset_test_state();
    _full_initialize();

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_producer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 1);

}

TEST(BroadcastTimeApp, is_producer_returns_zero_for_consumer_only)
{

    _reset_test_state();
    _full_initialize();

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    EXPECT_EQ(OpenLcbApplicationBroadcastTime_is_producer(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK), 0);

}


// ============================================================================
// Section 4: Time Tick Forward Tests
// ============================================================================

TEST(BroadcastTimeApp, time_tick_skips_inactive_clocks)
{

    _reset_test_state();
    _full_initialize();

    // No clocks set up - tick should do nothing (no crash)
    OpenLcbApplicationBroadcastTime_100ms_time_tick();

}

TEST(BroadcastTimeApp, time_tick_skips_non_consumer_clocks)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;  // 1.0x real-time

    OpenLcbApplicationBroadcastTime_100ms_time_tick();

    // Producer-only clock should not advance
    EXPECT_EQ(clock_state->time.minute, 0);

}

TEST(BroadcastTimeApp, time_tick_skips_stopped_clocks)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 0;
    clock_state->rate.rate = 4;

    OpenLcbApplicationBroadcastTime_100ms_time_tick();

    EXPECT_EQ(clock_state->time.minute, 0);

}

TEST(BroadcastTimeApp, time_tick_skips_zero_rate)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 0;

    OpenLcbApplicationBroadcastTime_100ms_time_tick();

    EXPECT_EQ(clock_state->time.minute, 0);

}

TEST(BroadcastTimeApp, time_tick_forward_one_minute_at_realtime)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;  // 1.0x real-time
    clock_state->time.hour = 10;
    clock_state->time.minute = 30;

    // 600 ticks at 100ms = 60 seconds = 1 real minute
    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 10);
    EXPECT_EQ(clock_state->time.minute, 31);

}

TEST(BroadcastTimeApp, time_tick_forward_hour_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;  // 1.0x real-time
    clock_state->time.hour = 10;
    clock_state->time.minute = 59;

    // Advance one minute
    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 11);
    EXPECT_EQ(clock_state->time.minute, 0);

}

TEST(BroadcastTimeApp, time_tick_forward_day_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 15;
    clock_state->date.month = 6;
    clock_state->year.year = 2026;

    // Advance one minute -> day rollover
    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 0);
    EXPECT_EQ(clock_state->time.minute, 0);
    EXPECT_EQ(clock_state->date.day, 16);
    EXPECT_TRUE(callback_date_rollover);
    EXPECT_TRUE(callback_time_received);

}

TEST(BroadcastTimeApp, time_tick_forward_month_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 31;
    clock_state->date.month = 1;  // January has 31 days
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 0);
    EXPECT_EQ(clock_state->time.minute, 0);
    EXPECT_EQ(clock_state->date.day, 1);
    EXPECT_EQ(clock_state->date.month, 2);
    EXPECT_TRUE(callback_date_rollover);
    EXPECT_TRUE(callback_date_received);

}

TEST(BroadcastTimeApp, time_tick_forward_year_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 31;
    clock_state->date.month = 12;
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 0);
    EXPECT_EQ(clock_state->time.minute, 0);
    EXPECT_EQ(clock_state->date.day, 1);
    EXPECT_EQ(clock_state->date.month, 1);
    EXPECT_EQ(clock_state->year.year, 2027);
    EXPECT_TRUE(callback_date_rollover);
    EXPECT_TRUE(callback_date_received);
    EXPECT_TRUE(callback_year_received);

}

TEST(BroadcastTimeApp, time_tick_forward_leap_year_feb_28)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 28;
    clock_state->date.month = 2;
    clock_state->year.year = 2024;  // Leap year

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // Feb 28 -> Feb 29 in a leap year (not month rollover)
    EXPECT_EQ(clock_state->date.day, 29);
    EXPECT_EQ(clock_state->date.month, 2);

}

TEST(BroadcastTimeApp, time_tick_forward_non_leap_year_feb_28)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 28;
    clock_state->date.month = 2;
    clock_state->year.year = 2026;  // Not a leap year

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // Feb 28 -> Mar 1 in non-leap year
    EXPECT_EQ(clock_state->date.day, 1);
    EXPECT_EQ(clock_state->date.month, 3);

}

TEST(BroadcastTimeApp, time_tick_forward_high_rate_multiple_minutes)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4 * 60;  // 60x real-time: 1 fast-minute per second
    clock_state->time.hour = 10;
    clock_state->time.minute = 0;

    // At 60x, each 100ms tick adds 100 * 240 = 24000
    // Threshold is 240,000, so 10 ticks = 1 fast-minute
    // 100 ticks = 10 fast-minutes
    for (int tick = 0; tick < 100; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 10);
    EXPECT_EQ(clock_state->time.minute, 10);

}


// ============================================================================
// Section 5: Time Tick Backward Tests
// ============================================================================

TEST(BroadcastTimeApp, time_tick_backward_one_minute)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;  // -1.0x (backward)
    clock_state->time.hour = 10;
    clock_state->time.minute = 30;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 10);
    EXPECT_EQ(clock_state->time.minute, 29);

}

TEST(BroadcastTimeApp, time_tick_backward_hour_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;
    clock_state->time.hour = 10;
    clock_state->time.minute = 0;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 9);
    EXPECT_EQ(clock_state->time.minute, 59);

}

TEST(BroadcastTimeApp, time_tick_backward_day_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;
    clock_state->time.hour = 0;
    clock_state->time.minute = 0;
    clock_state->date.day = 15;
    clock_state->date.month = 6;
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 23);
    EXPECT_EQ(clock_state->time.minute, 59);
    EXPECT_EQ(clock_state->date.day, 14);
    EXPECT_TRUE(callback_date_rollover);

}

TEST(BroadcastTimeApp, time_tick_backward_month_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;
    clock_state->time.hour = 0;
    clock_state->time.minute = 0;
    clock_state->date.day = 1;
    clock_state->date.month = 3;
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 23);
    EXPECT_EQ(clock_state->time.minute, 59);
    EXPECT_EQ(clock_state->date.day, 28);  // Feb has 28 days in 2026
    EXPECT_EQ(clock_state->date.month, 2);
    EXPECT_TRUE(callback_date_rollover);
    EXPECT_TRUE(callback_date_received);

}

TEST(BroadcastTimeApp, time_tick_backward_year_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;
    clock_state->time.hour = 0;
    clock_state->time.minute = 0;
    clock_state->date.day = 1;
    clock_state->date.month = 1;
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 23);
    EXPECT_EQ(clock_state->time.minute, 59);
    EXPECT_EQ(clock_state->date.day, 31);
    EXPECT_EQ(clock_state->date.month, 12);
    EXPECT_EQ(clock_state->year.year, 2025);
    EXPECT_TRUE(callback_date_rollover);
    EXPECT_TRUE(callback_date_received);
    EXPECT_TRUE(callback_year_received);

}

TEST(BroadcastTimeApp, time_tick_backward_leap_year_mar_1)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;
    clock_state->time.hour = 0;
    clock_state->time.minute = 0;
    clock_state->date.day = 1;
    clock_state->date.month = 3;
    clock_state->year.year = 2024;  // Leap year

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // Mar 1 backward -> Feb 29 in a leap year
    EXPECT_EQ(clock_state->date.day, 29);
    EXPECT_EQ(clock_state->date.month, 2);

}


// ============================================================================
// Section 6: Producer Send Function Tests
// ============================================================================

TEST(BroadcastTimeApp, send_report_time)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_time(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_time_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30, false);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_report_time_no_producer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    // Only setup consumer, not producer
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_time(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30));

}

TEST(BroadcastTimeApp, send_report_time_no_clocks_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_time(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30));

}

TEST(BroadcastTimeApp, send_report_date)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_date(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_date_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15, false);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_report_date_no_producer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    // Setup consumer only (not producer) - clock found but is_producer is false
    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_date(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15));

}

TEST(BroadcastTimeApp, send_report_year)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_year(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_year_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026, false);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_report_year_no_producer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_year(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026));

}

TEST(BroadcastTimeApp, send_report_rate)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_rate(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_rate_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010, false);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_report_rate_no_producer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_report_rate(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010));

}

TEST(BroadcastTimeApp, send_start)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_start(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_command_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_START);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_start_no_producer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_start(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}

TEST(BroadcastTimeApp, send_stop)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_stop(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_command_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_STOP);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_stop_no_producer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_stop(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}

TEST(BroadcastTimeApp, send_date_rollover)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_date_rollover(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_command_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_DATE_ROLLOVER);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_date_rollover_no_producer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_date_rollover(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}


// ============================================================================
// Section 7: Consumer Send Function Tests
// ============================================================================

TEST(BroadcastTimeApp, send_query)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_command_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_QUERY);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_query_no_consumer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    // Only producer, not consumer
    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}

TEST(BroadcastTimeApp, send_query_no_clocks_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}


// ============================================================================
// Section 8: Controller Send Function Tests
// ============================================================================

TEST(BroadcastTimeApp, send_set_time)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_set_time(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_time_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30, true);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_set_date)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_set_date(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_date_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15, true);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_set_year)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_set_year(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_year_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026, true);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_set_rate)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_set_rate(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_rate_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010, true);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_command_start)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_command_start(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_command_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_START);
    EXPECT_EQ(last_sent_event_id, expected);

}

TEST(BroadcastTimeApp, send_command_stop)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_command_stop(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);

    event_id_t expected = OpenLcbUtilities_create_command_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_STOP);
    EXPECT_EQ(last_sent_event_id, expected);

}


// ============================================================================
// Section 9: Query Reply (State Machine) Tests
// ============================================================================

TEST(BroadcastTimeApp, send_query_reply_running_clock_full_sequence)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    clock_state->is_running = 1;
    clock_state->rate.rate = 0x0010;
    clock_state->year.year = 2026;
    clock_state->date.month = 6;
    clock_state->date.day = 15;
    clock_state->time.hour = 14;
    clock_state->time.minute = 30;

    // The state machine advances state when send_event_with_mti fails (!false=true).
    // When send succeeds (!true=false), it breaks and returns true (done).
    // To drive through all 6 states, we fail sends for states 0-4 (advancing state each time)
    // then succeed at state 5 (send_event_pc_report).

    // States 0-4: Send fails -> state advances -> returns false (not done)
    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // state 0->1
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // state 1->2
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // state 2->3
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // state 3->4
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // state 4->5
    EXPECT_EQ(send_count, 0);  // No messages actually sent yet

    // State 5: send_event_pc_report succeeds -> state resets to 0 -> returns true (done)
    fail_transmit = false;
    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_EQ(send_count, 1);
    EXPECT_EQ(last_sent_mti, MTI_PC_EVENT_REPORT);
    event_id_t expected_next = OpenLcbUtilities_create_time_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31, false);
    EXPECT_EQ(last_sent_event_id, expected_next);

}

TEST(BroadcastTimeApp, send_query_reply_success_at_state_0_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    clock_state->is_running = 1;
    clock_state->rate.rate = 0x0010;
    clock_state->year.year = 2026;
    clock_state->date.month = 6;
    clock_state->date.day = 15;
    clock_state->time.hour = 14;
    clock_state->time.minute = 30;

    // When send succeeds at state 0, function returns true immediately (done)
    // and sends a Producer Identified Set with start event
    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_EQ(send_count, 1);
    EXPECT_EQ(last_sent_mti, MTI_PRODUCER_IDENTIFIED_SET);
    event_id_t expected_start = OpenLcbUtilities_create_command_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_START);
    EXPECT_EQ(last_sent_event_id, expected_start);

}

TEST(BroadcastTimeApp, send_query_reply_stopped_clock_sends_stop_event)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    clock_state->is_running = 0;  // Stopped
    clock_state->rate.rate = 0x0004;
    clock_state->year.year = 2026;
    clock_state->date.month = 1;
    clock_state->date.day = 1;
    clock_state->time.hour = 0;
    clock_state->time.minute = 0;

    // State 0: Send succeeds with stop event -> returns true (done)
    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0, 1));
    EXPECT_EQ(send_count, 1);
    EXPECT_EQ(last_sent_mti, MTI_PRODUCER_IDENTIFIED_SET);
    event_id_t expected_stop = OpenLcbUtilities_create_command_event_id(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, BROADCAST_TIME_EVENT_STOP);
    EXPECT_EQ(last_sent_event_id, expected_stop);

}

TEST(BroadcastTimeApp, send_query_reply_no_producer_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    // No clock setup at all
    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_EQ(send_count, 0);

}

TEST(BroadcastTimeApp, send_query_reply_consumer_only_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_EQ(send_count, 0);

}

TEST(BroadcastTimeApp, send_query_reply_fail_advances_state_then_success)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    clock_state->is_running = 1;
    clock_state->rate.rate = 0x0004;
    clock_state->year.year = 2026;
    clock_state->date.month = 6;
    clock_state->date.day = 15;
    clock_state->time.hour = 14;
    clock_state->time.minute = 30;

    // Transmit fails: !false = true, state 0->1, returns false (not done)
    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_EQ(send_count, 0);

    // Now allow transmit to succeed at state 1 (Rate):
    // send succeeds: !true = false, break, return true (done)
    fail_transmit = false;
    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_EQ(send_count, 1);
    EXPECT_EQ(last_sent_mti, MTI_PRODUCER_IDENTIFIED_SET);

    // Note: state is still at 1 (success doesn't advance/reset state for states 0-4).
    // Drive state machine to completion to reset state to 0 for next test.
    fail_transmit = true;
    OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31);  // 1->2
    OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31);  // 2->3
    OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31);  // 3->4
    OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31);  // 4->5
    fail_transmit = false;
    OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31);  // 5->0 (done)

}


// ============================================================================
// Section 10: Edge Cases
// ============================================================================

TEST(BroadcastTimeApp, time_tick_advances_multiple_consumer_clocks)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *fast_clock = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    fast_clock->is_running = 1;
    fast_clock->rate.rate = 4;
    fast_clock->time.hour = 10;
    fast_clock->time.minute = 0;

    broadcast_clock_state_t *realtime_clock = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK);
    realtime_clock->is_running = 1;
    realtime_clock->rate.rate = 8;  // 2.0x
    realtime_clock->time.hour = 5;
    realtime_clock->time.minute = 0;

    // Advance 600 ticks (1 real minute)
    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // Fast clock at 1.0x should advance 1 minute
    EXPECT_EQ(fast_clock->time.hour, 10);
    EXPECT_EQ(fast_clock->time.minute, 1);

    // Realtime clock at 2.0x should advance 2 minutes
    EXPECT_EQ(realtime_clock->time.hour, 5);
    EXPECT_EQ(realtime_clock->time.minute, 2);

}

TEST(BroadcastTimeApp, time_tick_null_interface_no_crash)
{

    _reset_test_state();

    OpenLcbApplication_initialize(&_test_application_interface);
    OpenLcbNode_initialize(&_test_node_interface);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolBroadcastTime_initialize(NULL);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_null_callback_interface);

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 31;
    clock_state->date.month = 12;
    clock_state->year.year = 2026;

    // Should advance without crashing even though callbacks are NULL
    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 0);
    EXPECT_EQ(clock_state->time.minute, 0);
    EXPECT_EQ(clock_state->date.day, 1);
    EXPECT_EQ(clock_state->date.month, 1);
    EXPECT_EQ(clock_state->year.year, 2027);

}

TEST(BroadcastTimeApp, days_in_month_invalid_month)
{

    _reset_test_state();
    _full_initialize();

    // Set month to invalid value and advance past day boundary
    // This tests the _days_in_month fallback return of 30
    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 30;
    clock_state->date.month = 0;  // Invalid month
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // With invalid month, _days_in_month returns 30, so day 31 > 30 triggers rollover
    EXPECT_EQ(clock_state->date.day, 1);
    EXPECT_EQ(clock_state->date.month, 1);

}

TEST(BroadcastTimeApp, century_leap_year_divisible_by_400)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 28;
    clock_state->date.month = 2;
    clock_state->year.year = 2000;  // Divisible by 400 -> leap year

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->date.day, 29);
    EXPECT_EQ(clock_state->date.month, 2);

}

TEST(BroadcastTimeApp, century_not_leap_year_divisible_by_100)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 28;
    clock_state->date.month = 2;
    clock_state->year.year = 1900;  // Divisible by 100 but not 400 -> not leap year

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // Feb 28 -> Mar 1 in non-leap century year
    EXPECT_EQ(clock_state->date.day, 1);
    EXPECT_EQ(clock_state->date.month, 3);

}


// ============================================================================
// Section 11: Start/Stop Function Tests
// ============================================================================

TEST(BroadcastTimeApp, start_sets_clock_running)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_FALSE(clock_state->is_running);

    OpenLcbApplicationBroadcastTime_start(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_TRUE(clock_state->is_running);

}

TEST(BroadcastTimeApp, stop_clears_clock_running)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = true;

    OpenLcbApplicationBroadcastTime_stop(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    EXPECT_FALSE(clock_state->is_running);

}

TEST(BroadcastTimeApp, start_invalid_clock_id_does_not_crash)
{

    _reset_test_state();
    _full_initialize();

    // No clock set up - should just return without crash
    OpenLcbApplicationBroadcastTime_start(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

}

TEST(BroadcastTimeApp, stop_invalid_clock_id_does_not_crash)
{

    _reset_test_state();
    _full_initialize();

    // No clock set up - should just return without crash
    OpenLcbApplicationBroadcastTime_stop(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

}


// ============================================================================
// Section 12: Setup Producer Overflow Test
// ============================================================================

TEST(BroadcastTimeApp, setup_producer_overflow_returns_null)
{

    _reset_test_state();
    _full_initialize();

    // Allocate all available slots using producer setup
    for (int i = 0; i < BROADCAST_TIME_TOTAL_CLOCK_COUNT; i++) {

        event_id_t clock_id = 0x0101000002000000ULL + ((uint64_t)(i) << 16);
        broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(NULL, clock_id);
        ASSERT_NE(clock_state, nullptr);

    }

    // Next allocation should fail
    event_id_t overflow_clock_id = 0x0101000003000000ULL;
    broadcast_clock_state_t *overflow_state = OpenLcbApplicationBroadcastTime_setup_producer(NULL, overflow_clock_id);
    EXPECT_EQ(overflow_state, nullptr);

}


// ============================================================================
// Section 13: Additional Days-in-Month Coverage
// ============================================================================

TEST(BroadcastTimeApp, days_in_month_invalid_month_above_12)
{

    _reset_test_state();
    _full_initialize();

    // Set month to 13 (above valid range) and advance past day boundary
    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 30;
    clock_state->date.month = 13;  // Invalid month > 12
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // With invalid month > 12, _days_in_month returns 30, so day 31 > 30 triggers rollover
    // month++ makes it 14, which is > 12, so month resets to 1 and year increments
    EXPECT_EQ(clock_state->date.day, 1);
    EXPECT_EQ(clock_state->date.month, 1);
    EXPECT_EQ(clock_state->year.year, 2027);

}

TEST(BroadcastTimeApp, time_tick_forward_30_day_month_rollover)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 30;
    clock_state->date.month = 4;  // April has 30 days
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // April 30 -> May 1
    EXPECT_EQ(clock_state->date.day, 1);
    EXPECT_EQ(clock_state->date.month, 5);
    EXPECT_TRUE(callback_date_received);

}

TEST(BroadcastTimeApp, time_tick_forward_no_month_rollover_at_day_30_in_31_day_month)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;
    clock_state->time.hour = 23;
    clock_state->time.minute = 59;
    clock_state->date.day = 30;
    clock_state->date.month = 3;  // March has 31 days
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // March 30 -> March 31 (no month rollover)
    EXPECT_EQ(clock_state->date.day, 31);
    EXPECT_EQ(clock_state->date.month, 3);
    EXPECT_FALSE(callback_date_received);

}


// ============================================================================
// Section 14: Transmit Failure Tests for Individual Send Functions
// ============================================================================

TEST(BroadcastTimeApp, send_report_time_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_report_time(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30));

}

TEST(BroadcastTimeApp, send_report_date_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_report_date(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15));

}

TEST(BroadcastTimeApp, send_report_year_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_report_year(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026));

}

TEST(BroadcastTimeApp, send_report_rate_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_report_rate(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010));

}

TEST(BroadcastTimeApp, send_start_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_start(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}

TEST(BroadcastTimeApp, send_stop_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_stop(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}

TEST(BroadcastTimeApp, send_date_rollover_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_producer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_date_rollover(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}

TEST(BroadcastTimeApp, send_query_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    OpenLcbApplicationBroadcastTime_setup_consumer(NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}

TEST(BroadcastTimeApp, send_set_time_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_set_time(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 30));

}

TEST(BroadcastTimeApp, send_set_date_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_set_date(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 6, 15));

}

TEST(BroadcastTimeApp, send_set_year_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_set_year(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 2026));

}

TEST(BroadcastTimeApp, send_set_rate_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_set_rate(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 0x0010));

}

TEST(BroadcastTimeApp, send_command_start_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_command_start(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}

TEST(BroadcastTimeApp, send_command_stop_transmit_failure)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_command_stop(
        node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK));

}


// ============================================================================
// Section 15: Null Interface Backward Tick Tests
// ============================================================================

TEST(BroadcastTimeApp, time_tick_backward_null_interface_no_crash)
{

    _reset_test_state();

    OpenLcbApplication_initialize(&_test_application_interface);
    OpenLcbNode_initialize(&_test_node_interface);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    ProtocolBroadcastTime_initialize(NULL);
    OpenLcbApplicationBroadcastTime_initialize(&_test_app_broadcast_time_null_callback_interface);

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;  // Backward
    clock_state->time.hour = 0;
    clock_state->time.minute = 0;
    clock_state->date.day = 1;
    clock_state->date.month = 1;
    clock_state->year.year = 2026;

    // Should advance backward through year rollover without crash
    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 23);
    EXPECT_EQ(clock_state->time.minute, 59);
    EXPECT_EQ(clock_state->date.day, 31);
    EXPECT_EQ(clock_state->date.month, 12);
    EXPECT_EQ(clock_state->year.year, 2025);

}


// ============================================================================
// Section 16: Query Reply Transmit Failure at Different Stages
// ============================================================================

TEST(BroadcastTimeApp, send_query_reply_failure_at_state_5_returns_true)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    clock_state->is_running = 1;
    clock_state->rate.rate = 0x0004;
    clock_state->year.year = 2026;
    clock_state->date.month = 6;
    clock_state->date.day = 15;
    clock_state->time.hour = 14;
    clock_state->time.minute = 30;

    // Drive through states 0-4 with failed sends (state advances on failure)
    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // 0->1
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // 1->2
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // 2->3
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // 3->4
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));  // 4->5
    EXPECT_EQ(send_count, 0);

    // State 5: send_event_pc_report also fails -> falls through break -> returns true (done)
    // (state 5 does NOT advance state on failure, it just falls through)
    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_EQ(send_count, 0);

    // Reset state to 0: succeed at state 5 to complete the sequence
    fail_transmit = false;
    OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31);

}

TEST(BroadcastTimeApp, send_query_reply_can_run_twice_consecutively)
{

    _reset_test_state();
    _full_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_producer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    clock_state->is_running = 1;
    clock_state->rate.rate = 0x0004;
    clock_state->year.year = 2026;
    clock_state->date.month = 6;
    clock_state->date.day = 15;
    clock_state->time.hour = 14;
    clock_state->time.minute = 30;

    // First full sequence: fail states 0-4, succeed at state 5
    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    fail_transmit = false;
    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 31));
    EXPECT_EQ(send_count, 1);

    // Second full sequence should work (state resets to 0 after completion)
    send_count = 0;
    fail_transmit = true;
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 32));
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 32));
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 32));
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 32));
    EXPECT_FALSE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 32));
    fail_transmit = false;
    EXPECT_TRUE(OpenLcbApplicationBroadcastTime_send_query_reply(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK, 14, 32));
    EXPECT_EQ(send_count, 1);

}


// ============================================================================
// Section 17: Additional Backward Time Tick Edge Cases
// ============================================================================

TEST(BroadcastTimeApp, time_tick_backward_month_rollover_to_31_day_month)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;
    clock_state->time.hour = 0;
    clock_state->time.minute = 0;
    clock_state->date.day = 1;
    clock_state->date.month = 4;  // April -> back to March (31 days)
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 23);
    EXPECT_EQ(clock_state->time.minute, 59);
    EXPECT_EQ(clock_state->date.day, 31);
    EXPECT_EQ(clock_state->date.month, 3);
    EXPECT_TRUE(callback_date_rollover);
    EXPECT_TRUE(callback_date_received);

}

TEST(BroadcastTimeApp, time_tick_backward_day_decrement_no_month_change)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -4;
    clock_state->time.hour = 0;
    clock_state->time.minute = 0;
    clock_state->date.day = 15;
    clock_state->date.month = 6;
    clock_state->year.year = 2026;

    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    // Day 15 -> Day 14 (date.day > 1 branch, no month change)
    EXPECT_EQ(clock_state->time.hour, 23);
    EXPECT_EQ(clock_state->time.minute, 59);
    EXPECT_EQ(clock_state->date.day, 14);
    EXPECT_EQ(clock_state->date.month, 6);
    EXPECT_TRUE(callback_date_rollover);
    EXPECT_FALSE(callback_date_received);

}

TEST(BroadcastTimeApp, time_tick_quarter_speed_forward)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 1;  // 0.25x real-time
    clock_state->time.hour = 10;
    clock_state->time.minute = 0;

    // At 0.25x, each tick adds 100 * 1 = 100
    // Threshold is 240,000, so 2400 ticks = 1 fast-minute (4 real minutes)
    for (int tick = 0; tick < 2400; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 10);
    EXPECT_EQ(clock_state->time.minute, 1);

}

TEST(BroadcastTimeApp, time_tick_negative_rate_quarter_speed)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = -1;  // -0.25x real-time
    clock_state->time.hour = 10;
    clock_state->time.minute = 30;

    // At -0.25x, each tick adds 100 * 1 = 100
    // 2400 ticks = 1 fast-minute backward
    for (int tick = 0; tick < 2400; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 10);
    EXPECT_EQ(clock_state->time.minute, 29);

}

TEST(BroadcastTimeApp, time_tick_super_high_rate_multiple_minutes_per_tick)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4 * 100;  // 100x real-time
    clock_state->time.hour = 10;
    clock_state->time.minute = 0;

    // At 100x, each tick adds 100 * 400 = 40,000
    // Threshold is 240,000, so ~6 ticks per fast-minute
    // But each tick can accumulate > 240,000 at very high rates
    // 1 tick: 40000 (no minute), 6 ticks: 240,000 (1 minute)
    // Actually 100 * 400 = 40000, threshold 240000 -> needs exactly 6 ticks per minute
    // 60 ticks = 10 fast-minutes
    for (int tick = 0; tick < 60; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.hour, 10);
    EXPECT_EQ(clock_state->time.minute, 10);

}


// ============================================================================
// Section 18: Accumulator Residual Tests
// ============================================================================

TEST(BroadcastTimeApp, time_tick_accumulator_does_not_lose_fractional_time)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->is_running = 1;
    clock_state->rate.rate = 4;  // 1.0x
    clock_state->time.hour = 10;
    clock_state->time.minute = 0;

    // Advance 599 ticks (just short of 1 minute)
    for (int tick = 0; tick < 599; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.minute, 0);  // Not yet advanced

    // One more tick should trigger the minute
    OpenLcbApplicationBroadcastTime_100ms_time_tick();

    EXPECT_EQ(clock_state->time.minute, 1);

}

TEST(BroadcastTimeApp, start_then_stop_then_tick_does_not_advance)
{

    _reset_test_state();
    _full_initialize();

    broadcast_clock_state_t *clock_state = OpenLcbApplicationBroadcastTime_setup_consumer(
        NULL, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    clock_state->rate.rate = 4;
    clock_state->time.hour = 10;
    clock_state->time.minute = 0;

    OpenLcbApplicationBroadcastTime_start(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    EXPECT_TRUE(clock_state->is_running);

    OpenLcbApplicationBroadcastTime_stop(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    EXPECT_FALSE(clock_state->is_running);

    // Tick should not advance because clock is stopped
    for (int tick = 0; tick < 600; tick++) {

        OpenLcbApplicationBroadcastTime_100ms_time_tick();

    }

    EXPECT_EQ(clock_state->time.minute, 0);

}
