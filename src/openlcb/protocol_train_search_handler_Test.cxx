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
* @file protocol_train_search_handler_Test.cxx
* @brief Unit tests for Train Search Protocol utilities and handler
*
* Test Organization:
* - Section 1: Utility — Event ID Detection Tests
* - Section 2: Utility — Digit Extraction Tests
* - Section 3: Utility — Digits to Address Conversion Tests
* - Section 4: Utility — Flags Extraction Tests
* - Section 5: Utility — Event ID Creation Tests
* - Section 6: Handler — Address Matching Tests
* - Section 7: Handler — Reply Generation Tests
* - Section 8: Handler — Callback Tests
*
* @author Jim Kueneman
* @date 17 Feb 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_utilities.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_list.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_node.h"
#include "protocol_train_search_handler.h"
#include "protocol_train_handler.h"
#include "openlcb_application_train.h"


#define TEST_SOURCE_ID    0x050101010800ULL
#define TEST_SOURCE_ALIAS 0x0AAA
#define TEST_DEST_ID      0x050101010900ULL
#define TEST_DEST_ALIAS   0x0BBB


// ============================================================================
// Test Tracking
// ============================================================================

static int _search_matched_count;
static openlcb_node_t *_search_matched_node;
static uint16_t _search_matched_address;
static uint8_t _search_matched_flags;

static void _test_on_search_matched(openlcb_node_t *openlcb_node,
        uint16_t search_address, uint8_t flags) {

    _search_matched_count++;
    _search_matched_node = openlcb_node;
    _search_matched_address = search_address;
    _search_matched_flags = flags;

}

static void _reset_tracking(void) {

    _search_matched_count = 0;
    _search_matched_node = NULL;
    _search_matched_address = 0;
    _search_matched_flags = 0;

}


// ============================================================================
// Interfaces
// ============================================================================

static interface_protocol_train_search_handler_t _interface_all = {

    .on_search_matched = &_test_on_search_matched,
    .on_search_no_match = NULL,

};

static interface_protocol_train_search_handler_t _interface_nulls = {

    .on_search_matched = NULL,
    .on_search_no_match = NULL,

};

static interface_protocol_train_handler_t _interface_train = {};

static interface_openlcb_application_train_t _interface_app_train = {

    .send_openlcb_msg = NULL,

};

static interface_openlcb_node_t _interface_openlcb_node = {};

static node_parameters_t _test_node_parameters = {

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

    .snip.mfg_version = 4,
    .snip.name = "Test Train Node",
    .snip.model = "Test Model",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2,

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,

};


// ============================================================================
// Test Helpers
// ============================================================================

static void _global_initialize(void) {

    ProtocolTrainSearch_initialize(&_interface_all);
    ProtocolTrainHandler_initialize(&_interface_train);
    OpenLcbApplicationTrain_initialize(&_interface_app_train);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    _reset_tracking();

}

static void _global_initialize_with_nulls(void) {

    ProtocolTrainSearch_initialize(&_interface_nulls);
    ProtocolTrainHandler_initialize(&_interface_train);
    OpenLcbApplicationTrain_initialize(&_interface_app_train);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    _reset_tracking();

}

static openlcb_node_t* _create_train_node(void) {

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    OpenLcbApplicationTrain_setup(node);

    return node;

}

static void _setup_statemachine(openlcb_statemachine_info_t *sm,
        openlcb_node_t *node, openlcb_msg_t *incoming,
        openlcb_msg_t *outgoing) {

    sm->openlcb_node = node;
    sm->incoming_msg_info.msg_ptr = incoming;
    sm->incoming_msg_info.enumerate = false;
    sm->outgoing_msg_info.msg_ptr = outgoing;
    sm->outgoing_msg_info.enumerate = false;
    sm->outgoing_msg_info.valid = false;

    incoming->source_id = TEST_SOURCE_ID;
    incoming->source_alias = TEST_SOURCE_ALIAS;
    incoming->dest_id = TEST_DEST_ID;
    incoming->dest_alias = TEST_DEST_ALIAS;

}


// ============================================================================
// Section 1: Utility — Event ID Detection Tests
// ============================================================================

TEST(TrainSearch, is_train_search_event_valid)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0x00000300ULL;

    EXPECT_TRUE(OpenLcbUtilities_is_train_search_event(event_id));

}

TEST(TrainSearch, is_train_search_event_with_flags)
{

    // Address 1234 with DCC flag
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFF12348ULL;

    EXPECT_TRUE(OpenLcbUtilities_is_train_search_event(event_id));

}

TEST(TrainSearch, is_train_search_event_broadcast_time_false)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0000ULL;

    EXPECT_FALSE(OpenLcbUtilities_is_train_search_event(event_id));

}

TEST(TrainSearch, is_train_search_event_random_false)
{

    event_id_t event_id = 0x0505050505050000ULL;

    EXPECT_FALSE(OpenLcbUtilities_is_train_search_event(event_id));

}

TEST(TrainSearch, is_train_search_event_zero_false)
{

    event_id_t event_id = 0x0000000000000000ULL;

    EXPECT_FALSE(OpenLcbUtilities_is_train_search_event(event_id));

}


// ============================================================================
// Section 2: Utility — Digit Extraction Tests
// ============================================================================

TEST(TrainSearch, extract_digits_address_3)
{

    // Address 3: nibbles should be F,F,F,F,F,3 + flags=0x00
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF300ULL;
    uint8_t digits[6];

    OpenLcbUtilities_extract_train_search_digits(event_id, digits);

    EXPECT_EQ(digits[0], 0x0F);
    EXPECT_EQ(digits[1], 0x0F);
    EXPECT_EQ(digits[2], 0x0F);
    EXPECT_EQ(digits[3], 0x0F);
    EXPECT_EQ(digits[4], 0x0F);
    EXPECT_EQ(digits[5], 0x03);

}

TEST(TrainSearch, extract_digits_address_1234)
{

    // Address 1234: nibbles F,F,1,2,3,4 + flags=0x00
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFF123400ULL;
    uint8_t digits[6];

    OpenLcbUtilities_extract_train_search_digits(event_id, digits);

    EXPECT_EQ(digits[0], 0x0F);
    EXPECT_EQ(digits[1], 0x0F);
    EXPECT_EQ(digits[2], 0x01);
    EXPECT_EQ(digits[3], 0x02);
    EXPECT_EQ(digits[4], 0x03);
    EXPECT_EQ(digits[5], 0x04);

}

TEST(TrainSearch, extract_digits_all_empty)
{

    // All 0xF: FFFFFF + flags=0x00
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFFF00ULL;
    uint8_t digits[6];

    OpenLcbUtilities_extract_train_search_digits(event_id, digits);

    for (int i = 0; i < 6; i++) {

        EXPECT_EQ(digits[i], 0x0F);

    }

}

TEST(TrainSearch, extract_digits_address_9999)
{

    // Address 9999: nibbles F,F,9,9,9,9
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFF999900ULL;
    uint8_t digits[6];

    OpenLcbUtilities_extract_train_search_digits(event_id, digits);

    EXPECT_EQ(digits[0], 0x0F);
    EXPECT_EQ(digits[1], 0x0F);
    EXPECT_EQ(digits[2], 0x09);
    EXPECT_EQ(digits[3], 0x09);
    EXPECT_EQ(digits[4], 0x09);
    EXPECT_EQ(digits[5], 0x09);

}


// ============================================================================
// Section 3: Utility — Digits to Address Conversion Tests
// ============================================================================

TEST(TrainSearch, digits_to_address_3)
{

    uint8_t digits[] = {0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x03};

    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 3);

}

TEST(TrainSearch, digits_to_address_1234)
{

    uint8_t digits[] = {0x0F, 0x0F, 0x01, 0x02, 0x03, 0x04};

    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 1234);

}

TEST(TrainSearch, digits_to_address_all_empty)
{

    uint8_t digits[] = {0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F};

    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 0);

}

TEST(TrainSearch, digits_to_address_leading_zeros)
{

    // 003 = {F,F,F,0,0,3}
    uint8_t digits[] = {0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x03};

    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 3);

}

TEST(TrainSearch, digits_to_address_9999)
{

    uint8_t digits[] = {0x0F, 0x0F, 0x09, 0x09, 0x09, 0x09};

    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 9999);

}


// ============================================================================
// Section 4: Utility — Flags Extraction Tests
// ============================================================================

TEST(TrainSearch, extract_flags_allocate)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF380ULL;
    uint8_t flags = OpenLcbUtilities_extract_train_search_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_ALLOCATE, TRAIN_SEARCH_FLAG_ALLOCATE);

}

TEST(TrainSearch, extract_flags_exact)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF340ULL;
    uint8_t flags = OpenLcbUtilities_extract_train_search_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_EXACT, TRAIN_SEARCH_FLAG_EXACT);

}

TEST(TrainSearch, extract_flags_address_only)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF320ULL;
    uint8_t flags = OpenLcbUtilities_extract_train_search_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_ADDRESS_ONLY, TRAIN_SEARCH_FLAG_ADDRESS_ONLY);

}

TEST(TrainSearch, extract_flags_dcc)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF308ULL;
    uint8_t flags = OpenLcbUtilities_extract_train_search_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_DCC, TRAIN_SEARCH_FLAG_DCC);

}

TEST(TrainSearch, extract_flags_dcc_long_addr)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF30CULL;
    uint8_t flags = OpenLcbUtilities_extract_train_search_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_DCC, TRAIN_SEARCH_FLAG_DCC);
    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_LONG_ADDR, TRAIN_SEARCH_FLAG_LONG_ADDR);

}

TEST(TrainSearch, extract_flags_speed_steps_128)
{

    // Speed steps 128 = 0x03 in bits 1-0
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF30BULL;
    uint8_t flags = OpenLcbUtilities_extract_train_search_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_SPEED_STEP_MASK, 0x03);

}


// ============================================================================
// Section 5: Utility — Event ID Creation Tests
// ============================================================================

TEST(TrainSearch, create_event_id_address_3)
{

    event_id_t event_id = OpenLcbUtilities_create_train_search_event_id(3, 0x00);

    EXPECT_TRUE(OpenLcbUtilities_is_train_search_event(event_id));

    uint8_t digits[6];
    OpenLcbUtilities_extract_train_search_digits(event_id, digits);
    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 3);
    EXPECT_EQ(OpenLcbUtilities_extract_train_search_flags(event_id), 0x00);

}

TEST(TrainSearch, create_event_id_address_1234_dcc_long)
{

    uint8_t flags = TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR;
    event_id_t event_id = OpenLcbUtilities_create_train_search_event_id(1234, flags);

    EXPECT_TRUE(OpenLcbUtilities_is_train_search_event(event_id));

    uint8_t digits[6];
    OpenLcbUtilities_extract_train_search_digits(event_id, digits);
    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 1234);
    EXPECT_EQ(OpenLcbUtilities_extract_train_search_flags(event_id), flags);

}

TEST(TrainSearch, create_event_id_roundtrip)
{

    // Create, extract, compare
    uint16_t address = 567;
    uint8_t flags = TRAIN_SEARCH_FLAG_DCC | 0x03;
    event_id_t event_id = OpenLcbUtilities_create_train_search_event_id(address, flags);

    uint8_t digits[6];
    OpenLcbUtilities_extract_train_search_digits(event_id, digits);

    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 567);
    EXPECT_EQ(OpenLcbUtilities_extract_train_search_flags(event_id), flags);

}

TEST(TrainSearch, create_event_id_address_zero)
{

    event_id_t event_id = OpenLcbUtilities_create_train_search_event_id(0, 0x00);

    EXPECT_TRUE(OpenLcbUtilities_is_train_search_event(event_id));

    uint8_t digits[6];
    OpenLcbUtilities_extract_train_search_digits(event_id, digits);
    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(digits), 0);

}


// ============================================================================
// Section 6: Handler — Address Matching Tests
// ============================================================================

TEST(TrainSearch, handler_exact_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 1234, DCC short
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            1234, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Should match — outgoing valid
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->mti, MTI_PRODUCER_IDENTIFIED_SET);

}

TEST(TrainSearch, handler_no_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 5678 — different from node's 1234
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            5678, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Should NOT match — outgoing not valid
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_dcc_protocol_any_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42 with protocol=any (flags=0x00)
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(42, 0x00);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Should match — protocol=any matches everything
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_non_train_node_skipped)
{

    _global_initialize();

    // Create a regular (non-train) node
    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    // Do NOT call OpenLcbApplicationTrain_setup — train_state stays NULL

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(3, 0x00);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Non-train node — should be skipped, no reply
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}


// ============================================================================
// Section 7: Handler — Reply Generation Tests
// ============================================================================

TEST(TrainSearch, handler_reply_contains_train_address)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, false);
    OpenLcbApplicationTrain_set_speed_steps(node, 3); // 128 speed steps

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            1234, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    // Extract event ID from the reply payload
    event_id_t reply_event = OpenLcbUtilities_extract_event_id_from_openlcb_payload(
            sm.outgoing_msg_info.msg_ptr);

    // Reply should be a train search event
    EXPECT_TRUE(OpenLcbUtilities_is_train_search_event(reply_event));

    // Reply should contain the train's actual address
    uint8_t reply_digits[6];
    OpenLcbUtilities_extract_train_search_digits(reply_event, reply_digits);
    EXPECT_EQ(OpenLcbUtilities_train_search_digits_to_address(reply_digits), 1234);

    // Reply flags should include DCC + speed steps 128 (0x03)
    uint8_t reply_flags = OpenLcbUtilities_extract_train_search_flags(reply_event);
    EXPECT_EQ(reply_flags & TRAIN_SEARCH_FLAG_DCC, TRAIN_SEARCH_FLAG_DCC);
    EXPECT_EQ(reply_flags & TRAIN_SEARCH_SPEED_STEP_MASK, 0x03);

}

TEST(TrainSearch, handler_reply_long_address_flag)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 5000, true); // long address

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            5000, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    // Reply should have long address flag set
    event_id_t reply_event = OpenLcbUtilities_extract_event_id_from_openlcb_payload(
            sm.outgoing_msg_info.msg_ptr);
    uint8_t reply_flags = OpenLcbUtilities_extract_train_search_flags(reply_event);
    EXPECT_EQ(reply_flags & TRAIN_SEARCH_FLAG_LONG_ADDR, TRAIN_SEARCH_FLAG_LONG_ADDR);

}

TEST(TrainSearch, handler_reply_source_is_train_node)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 3, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(3, 0x00);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    // Reply source should be the train node
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->source_alias, TEST_DEST_ALIAS);
    EXPECT_EQ(sm.outgoing_msg_info.msg_ptr->source_id, TEST_DEST_ID);

}


// ============================================================================
// Section 8: Handler — Callback Tests
// ============================================================================

TEST(TrainSearch, handler_callback_fires_on_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    uint8_t flags = TRAIN_SEARCH_FLAG_DCC;
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(42, flags);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_EQ(_search_matched_count, 1);
    EXPECT_EQ(_search_matched_node, node);
    EXPECT_EQ(_search_matched_address, 42);
    EXPECT_EQ(_search_matched_flags, flags);

}

TEST(TrainSearch, handler_callback_not_fired_on_no_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(99, 0x00);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_EQ(_search_matched_count, 0);

}

TEST(TrainSearch, handler_null_callbacks_no_crash)
{

    _global_initialize_with_nulls();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(42, 0x00);

    // Should not crash with NULL callbacks
    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Reply should still be generated
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    // Callback count stays at 0 since we used null interface
    EXPECT_EQ(_search_matched_count, 0);

}
