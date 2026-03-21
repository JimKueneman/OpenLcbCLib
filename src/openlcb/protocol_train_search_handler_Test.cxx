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
* - Section 9: Handler — Prefix / Exact Match Tests
* - Section 10: Handler — Name Match Tests
* - Section 11: Handler — Short/Long Address Disambiguation Tests
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

static int _no_match_count;
static uint16_t _no_match_address;
static uint8_t _no_match_flags;
static openlcb_node_t *_no_match_return_node;

static openlcb_node_t* _test_on_search_no_match(uint16_t search_address, uint8_t flags) {

    _no_match_count++;
    _no_match_address = search_address;
    _no_match_flags = flags;

    return _no_match_return_node;

}

static void _reset_tracking(void) {

    _search_matched_count = 0;
    _search_matched_node = NULL;
    _search_matched_address = 0;
    _search_matched_flags = 0;

    _no_match_count = 0;
    _no_match_address = 0;
    _no_match_flags = 0;
    _no_match_return_node = NULL;

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

static interface_protocol_train_search_handler_t _interface_with_no_match = {

    .on_search_matched = &_test_on_search_matched,
    .on_search_no_match = &_test_on_search_no_match,

};

static interface_protocol_train_handler_t _interface_train = {};

static interface_openlcb_application_train_t _interface_app_train = {

    .send_openlcb_msg = NULL,

};

static interface_openlcb_node_t _interface_openlcb_node = {};

static node_parameters_t _test_node_parameters = {

    .snip = {
        .mfg_version = 4,
        .name = "Test Train Node",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

    .configuration_options = {
        .high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY
    },

};

static node_parameters_t _test_node_parameters_named = {

    .snip = {
        .mfg_version = 4,
        .name = "Loco 1234 Express",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

    .configuration_options = {
        .high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY
    },

};

static node_parameters_t _test_node_parameters_empty_name = {

    .snip = {
        .mfg_version = 4,
        .name = "",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

};

static node_parameters_t _test_node_parameters_ab12cd34 = {

    .snip = {
        .mfg_version = 4,
        .name = "AB12CD34",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

};

static node_parameters_t _test_node_parameters_dashes = {

    .snip = {
        .mfg_version = 4,
        .name = "Unit 1-2-3",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

};

static node_parameters_t _test_node_parameters_1357 = {

    .snip = {
        .mfg_version = 4,
        .name = "Train 1357",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

};

static node_parameters_t _test_node_parameters_a5b = {

    .snip = {
        .mfg_version = 4,
        .name = "A5B",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

};

static node_parameters_t _test_node_parameters_test42 = {

    .snip = {
        .mfg_version = 4,
        .name = "Test 42",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

};

static node_parameters_t _test_node_parameters_digit_start = {

    .snip = {
        .mfg_version = 4,
        .name = "5Train",
        .model = "Test Model",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_EVENT_EXCHANGE |
                         PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

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

static void _global_initialize_with_no_match(void) {

    ProtocolTrainSearch_initialize(&_interface_with_no_match);
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

static openlcb_node_t* _create_named_train_node(void) {

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters_named);
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

    EXPECT_TRUE(ProtocolTrainSearch_is_search_event(event_id));

}

TEST(TrainSearch, is_train_search_event_with_flags)
{

    // Address 1234 with DCC flag
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFF12348ULL;

    EXPECT_TRUE(ProtocolTrainSearch_is_search_event(event_id));

}

TEST(TrainSearch, is_train_search_event_broadcast_time_false)
{

    event_id_t event_id = BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0000ULL;

    EXPECT_FALSE(ProtocolTrainSearch_is_search_event(event_id));

}

TEST(TrainSearch, is_train_search_event_random_false)
{

    event_id_t event_id = 0x0505050505050000ULL;

    EXPECT_FALSE(ProtocolTrainSearch_is_search_event(event_id));

}

TEST(TrainSearch, is_train_search_event_zero_false)
{

    event_id_t event_id = 0x0000000000000000ULL;

    EXPECT_FALSE(ProtocolTrainSearch_is_search_event(event_id));

}


// ============================================================================
// Section 2: Utility — Digit Extraction Tests
// ============================================================================

TEST(TrainSearch, extract_digits_address_3)
{

    // Address 3: nibbles should be F,F,F,F,F,3 + flags=0x00
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF300ULL;
    uint8_t digits[6];

    ProtocolTrainSearch_extract_digits(event_id, digits);

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

    ProtocolTrainSearch_extract_digits(event_id, digits);

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

    ProtocolTrainSearch_extract_digits(event_id, digits);

    for (int i = 0; i < 6; i++) {

        EXPECT_EQ(digits[i], 0x0F);

    }

}

TEST(TrainSearch, extract_digits_address_9999)
{

    // Address 9999: nibbles F,F,9,9,9,9
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFF999900ULL;
    uint8_t digits[6];

    ProtocolTrainSearch_extract_digits(event_id, digits);

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

    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 3);

}

TEST(TrainSearch, digits_to_address_1234)
{

    uint8_t digits[] = {0x0F, 0x0F, 0x01, 0x02, 0x03, 0x04};

    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 1234);

}

TEST(TrainSearch, digits_to_address_all_empty)
{

    uint8_t digits[] = {0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F};

    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 0);

}

TEST(TrainSearch, digits_to_address_leading_zeros)
{

    // 003 = {F,F,F,0,0,3}
    uint8_t digits[] = {0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x03};

    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 3);

}

TEST(TrainSearch, digits_to_address_9999)
{

    uint8_t digits[] = {0x0F, 0x0F, 0x09, 0x09, 0x09, 0x09};

    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 9999);

}


// ============================================================================
// Section 4: Utility — Flags Extraction Tests
// ============================================================================

TEST(TrainSearch, extract_flags_allocate)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF380ULL;
    uint8_t flags = ProtocolTrainSearch_extract_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_ALLOCATE, TRAIN_SEARCH_FLAG_ALLOCATE);

}

TEST(TrainSearch, extract_flags_exact)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF340ULL;
    uint8_t flags = ProtocolTrainSearch_extract_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_EXACT, TRAIN_SEARCH_FLAG_EXACT);

}

TEST(TrainSearch, extract_flags_address_only)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF320ULL;
    uint8_t flags = ProtocolTrainSearch_extract_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_ADDRESS_ONLY, TRAIN_SEARCH_FLAG_ADDRESS_ONLY);

}

TEST(TrainSearch, extract_flags_dcc)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF308ULL;
    uint8_t flags = ProtocolTrainSearch_extract_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_DCC, TRAIN_SEARCH_FLAG_DCC);

}

TEST(TrainSearch, extract_flags_dcc_long_addr)
{

    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF30CULL;
    uint8_t flags = ProtocolTrainSearch_extract_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_DCC, TRAIN_SEARCH_FLAG_DCC);
    EXPECT_EQ(flags & TRAIN_SEARCH_FLAG_LONG_ADDR, TRAIN_SEARCH_FLAG_LONG_ADDR);

}

TEST(TrainSearch, extract_flags_speed_steps_128)
{

    // Speed steps 128 = 0x03 in bits 1-0
    event_id_t event_id = EVENT_TRAIN_SEARCH_SPACE | 0xFFFFF30BULL;
    uint8_t flags = ProtocolTrainSearch_extract_flags(event_id);

    EXPECT_EQ(flags & TRAIN_SEARCH_SPEED_STEP_MASK, 0x03);

}


// ============================================================================
// Section 5: Utility — Event ID Creation Tests
// ============================================================================

TEST(TrainSearch, create_event_id_address_3)
{

    event_id_t event_id = ProtocolTrainSearch_create_event_id(3, 0x00);

    EXPECT_TRUE(ProtocolTrainSearch_is_search_event(event_id));

    uint8_t digits[6];
    ProtocolTrainSearch_extract_digits(event_id, digits);
    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 3);
    EXPECT_EQ(ProtocolTrainSearch_extract_flags(event_id), 0x00);

}

TEST(TrainSearch, create_event_id_address_1234_dcc_long)
{

    uint8_t flags = TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR;
    event_id_t event_id = ProtocolTrainSearch_create_event_id(1234, flags);

    EXPECT_TRUE(ProtocolTrainSearch_is_search_event(event_id));

    uint8_t digits[6];
    ProtocolTrainSearch_extract_digits(event_id, digits);
    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 1234);
    EXPECT_EQ(ProtocolTrainSearch_extract_flags(event_id), flags);

}

TEST(TrainSearch, create_event_id_roundtrip)
{

    // Create, extract, compare
    uint16_t address = 567;
    uint8_t flags = TRAIN_SEARCH_FLAG_DCC | 0x03;
    event_id_t event_id = ProtocolTrainSearch_create_event_id(address, flags);

    uint8_t digits[6];
    ProtocolTrainSearch_extract_digits(event_id, digits);

    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 567);
    EXPECT_EQ(ProtocolTrainSearch_extract_flags(event_id), flags);

}

TEST(TrainSearch, create_event_id_address_zero)
{

    event_id_t event_id = ProtocolTrainSearch_create_event_id(0, 0x00);

    EXPECT_TRUE(ProtocolTrainSearch_is_search_event(event_id));

    uint8_t digits[6];
    ProtocolTrainSearch_extract_digits(event_id, digits);
    EXPECT_EQ(ProtocolTrainSearch_digits_to_address(digits), 0);

}


// ============================================================================
// Section 6: Handler — Address Matching Tests
// ============================================================================

TEST(TrainSearch, handler_exact_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 1234, DCC long (address >= 128 implies long)
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
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
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 5678 — different from node's 1234
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
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
    event_id_t search_event = ProtocolTrainSearch_create_event_id(42, 0x00);

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
    node->train_state = NULL;  // Explicitly NULL — no train setup

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = ProtocolTrainSearch_create_event_id(3, 0x00);

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
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, true);
    OpenLcbApplicationTrain_set_speed_steps(node, 3); // 128 speed steps

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            1234, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    // Extract event ID from the reply payload
    event_id_t reply_event = OpenLcbUtilities_extract_event_id_from_openlcb_payload(
            sm.outgoing_msg_info.msg_ptr);

    // Reply should be a train search event
    EXPECT_TRUE(ProtocolTrainSearch_is_search_event(reply_event));

    // Reply should echo the search event ID
    EXPECT_EQ(reply_event, search_event);

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

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            5000, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    // Reply should echo the search event ID
    event_id_t reply_event = OpenLcbUtilities_extract_event_id_from_openlcb_payload(
            sm.outgoing_msg_info.msg_ptr);
    EXPECT_EQ(reply_event, search_event);

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

    event_id_t search_event = ProtocolTrainSearch_create_event_id(3, 0x00);

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
    event_id_t search_event = ProtocolTrainSearch_create_event_id(42, flags);

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

    event_id_t search_event = ProtocolTrainSearch_create_event_id(99, 0x00);

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

    event_id_t search_event = ProtocolTrainSearch_create_event_id(42, 0x00);

    // Should not crash with NULL callbacks
    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Reply should still be generated
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    // Callback count stays at 0 since we used null interface
    EXPECT_EQ(_search_matched_count, 0);

}


// ============================================================================
// Section 9: Handler — Prefix / Exact Match Tests (Item 25)
// ============================================================================

TEST(TrainSearch, handler_prefix_match_no_exact_flag)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "12" without EXACT — should prefix-match train 1234
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            12, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_prefix_no_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "13" without EXACT — does NOT prefix-match train 1234
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            13, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_exact_flag_rejects_prefix)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "12" WITH EXACT — must NOT match train 1234
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            12, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_EXACT);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_exact_flag_full_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 1234, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "1234" WITH EXACT — should match train 1234 (long address)
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            1234, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_EXACT);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}


// ============================================================================
// Section 10: Handler — Name Match Tests (Item 26)
// ============================================================================

TEST(TrainSearch, handler_name_match_when_address_only_clear)
{

    _global_initialize();

    // Named train "Loco 1234 Express" with DCC address 5000
    openlcb_node_t *node = _create_named_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 5000, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "1234" without ADDRESS_ONLY — should match name "Loco 1234 Express"
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            1234, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_name_match_blocked_by_address_only)
{

    _global_initialize();

    // Named train "Loco 1234 Express" with DCC address 5000
    openlcb_node_t *node = _create_named_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 5000, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "1234" WITH ADDRESS_ONLY — name match blocked, address 5000 != 1234
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            1234, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR | TRAIN_SEARCH_FLAG_ADDRESS_ONLY);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_name_prefix_match)
{

    _global_initialize();

    // Named train "Loco 1234 Express" with DCC address 5000
    openlcb_node_t *node = _create_named_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 5000, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "12" without EXACT or ADDRESS_ONLY — prefix matches "1234" in name
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            12, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_name_exact_rejects_prefix)
{

    _global_initialize();

    // Named train "Loco 1234 Express" with DCC address 5000
    openlcb_node_t *node = _create_named_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 5000, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "12" WITH EXACT — "12" is not exact match of "1234" in name
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            12, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR | TRAIN_SEARCH_FLAG_EXACT);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_name_exact_full_match)
{

    _global_initialize();

    // Named train "Loco 1234 Express" with DCC address 5000
    openlcb_node_t *node = _create_named_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 5000, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "1234" WITH EXACT — exact match of "1234" in name
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            1234, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR | TRAIN_SEARCH_FLAG_EXACT);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}


// ============================================================================
// Section 11: Handler — Short/Long Address Disambiguation Tests (Item 18)
// ============================================================================

TEST(TrainSearch, handler_disambig_short_search_matches_short_train)
{

    _global_initialize();

    // Short address search (< 128) should match a short-address train
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42, DCC flag set, no LONG_ADDR flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Short search + short train — should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_short_search_rejects_long_train)
{

    _global_initialize();

    // Short address search (< 128) should reject a long-address train
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42, DCC flag set, no LONG_ADDR flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Short search + long train — should reject
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_high_search_matches_long_train)
{

    _global_initialize();

    // High address (>= 128) should match a long-address train
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 200, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 200, DCC flag set, no LONG_ADDR flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // High search + long train — should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_high_search_rejects_short_train)
{

    _global_initialize();

    // High address (>= 128) should reject a short-address train — THE BUG FIX
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 200, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 200, DCC flag set, no LONG_ADDR flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // High search + short train — should reject
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_boundary_128_rejects_short_train)
{

    _global_initialize();

    // Boundary: address exactly 128, short train — rejected
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 128, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 128, DCC flag set, no LONG_ADDR flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            128, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Address 128 is >= TRAIN_MAX_DCC_SHORT_ADDRESS, short train — should reject
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_boundary_127_allows_short_train)
{

    _global_initialize();

    // Boundary: address exactly 127, short train — allowed
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 127, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 127, DCC flag set, no LONG_ADDR flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            127, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Address 127 is < TRAIN_MAX_DCC_SHORT_ADDRESS, short train — should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_boundary_128_allows_long_train)
{

    _global_initialize();

    // Boundary: address exactly 128, long train — allowed
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 128, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 128, DCC flag set, no LONG_ADDR flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            128, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Address 128 is >= TRAIN_MAX_DCC_SHORT_ADDRESS, long train — should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_allocate_bypasses_high_search_short_train)
{

    _global_initialize();

    // ALLOCATE flag should bypass disambiguation for high address + short train
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 200, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 200, DCC + ALLOCATE flags
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // ALLOCATE bypasses disambiguation — should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_allocate_bypasses_short_search_long_train)
{

    _global_initialize();

    // ALLOCATE flag should bypass disambiguation for low address + long train
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42, DCC + ALLOCATE flags
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // ALLOCATE bypasses disambiguation — should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_explicit_long_flag_matches_long_train)
{

    _global_initialize();

    // Explicit LONG_ADDR flag should match a long-address train
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 5000, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 5000, DCC + LONG_ADDR flags
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            5000, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Explicit LONG_ADDR + long train — should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_explicit_long_flag_rejects_short_train)
{

    _global_initialize();

    // Explicit LONG_ADDR flag should reject a short-address train
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42, DCC + LONG_ADDR flags
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Explicit LONG_ADDR + short train — should reject
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

TEST(TrainSearch, handler_disambig_non_dcc_search_ignores_address_type)
{

    _global_initialize();

    // Non-DCC search (flags = 0x00) should ignore disambiguation entirely
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 200, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 200, no DCC flag — protocol=any
    event_id_t search_event = ProtocolTrainSearch_create_event_id(200, 0x00);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Non-DCC ignores address type — should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}


// ============================================================================
// SECTION 12: HANDLER -- No-Match Callback Tests
// @details Tests for ProtocolTrainSearch_handle_search_no_match()
// ============================================================================

// ============================================================================
// TEST: No-match handler invokes callback when ALLOCATE flag set
// @coverage ProtocolTrainSearch_handle_search_no_match()
// ============================================================================

TEST(TrainSearch, no_match_allocate_flag_set)
{

    _global_initialize_with_no_match();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search with ALLOCATE flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    _no_match_return_node = NULL;  // application returns NULL (no allocation)

    ProtocolTrainSearch_handle_search_no_match(&sm, search_event);

    // Callback must be invoked
    EXPECT_EQ(_no_match_count, 1);
    EXPECT_EQ(_no_match_address, (uint16_t) 200);
    EXPECT_EQ(_no_match_flags & TRAIN_SEARCH_FLAG_ALLOCATE, TRAIN_SEARCH_FLAG_ALLOCATE);

    // No reply since callback returned NULL
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: No-match handler does NOT invoke callback without ALLOCATE flag
// @coverage ProtocolTrainSearch_handle_search_no_match()
// ============================================================================

TEST(TrainSearch, no_match_allocate_flag_clear)
{

    _global_initialize_with_no_match();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search WITHOUT ALLOCATE flag
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_no_match(&sm, search_event);

    // Callback must NOT be invoked
    EXPECT_EQ(_no_match_count, 0);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: No-match handler graceful with NULL interface
// @coverage ProtocolTrainSearch_handle_search_no_match()
// ============================================================================

TEST(TrainSearch, no_match_null_interface)
{

    _global_initialize_with_nulls();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->alias = TEST_DEST_ALIAS;
    node->train_state = NULL;
    OpenLcbApplicationTrain_setup(node);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search with ALLOCATE flag but NULL interface callback
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    // Must not crash
    ProtocolTrainSearch_handle_search_no_match(&sm, search_event);

    EXPECT_EQ(_no_match_count, 0);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: No-match handler loads reply when callback returns valid node
// @coverage ProtocolTrainSearch_handle_search_no_match()
// ============================================================================

TEST(TrainSearch, no_match_returns_node_with_train_state)
{

    _global_initialize_with_no_match();

    // Create a node that the no-match callback will "allocate"
    openlcb_node_t *new_node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(new_node, 200, true);

    // Use a separate node as the statemachine context (last enumerated node)
    openlcb_node_t *current_node = OpenLcbNode_allocate(0x050101010A00ULL, &_test_node_parameters);
    current_node->alias = 0x0CCC;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, current_node, incoming, outgoing);

    // Set up callback to return the new node
    _no_match_return_node = new_node;

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR | TRAIN_SEARCH_FLAG_ALLOCATE);

    ProtocolTrainSearch_handle_search_no_match(&sm, search_event);

    // Callback invoked
    EXPECT_EQ(_no_match_count, 1);

    // Reply loaded from the NEW node
    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing->source_alias, new_node->alias);
    EXPECT_EQ(outgoing->source_id, (uint64_t) new_node->id);
    EXPECT_EQ(outgoing->mti, MTI_PRODUCER_IDENTIFIED_SET);

}

// ============================================================================
// TEST: No-match handler does not load reply when callback returns NULL
// @coverage ProtocolTrainSearch_handle_search_no_match()
// ============================================================================

TEST(TrainSearch, no_match_returns_null)
{

    _global_initialize_with_no_match();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    _no_match_return_node = NULL;

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    ProtocolTrainSearch_handle_search_no_match(&sm, search_event);

    // Callback invoked but returned NULL
    EXPECT_EQ(_no_match_count, 1);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}


// ============================================================================
// SECTION 13: ADDITIONAL COVERAGE -- Missing Branch Tests
// @details Tests targeting uncovered branches for 100% branch coverage
// ============================================================================

// ============================================================================
// TEST: All-0xF digit search returns false (line 115)
// @coverage _does_address_match — search_digit_count == 0 TRUE branch
// ============================================================================

TEST(TrainSearch, address_match_all_f_digits_returns_no_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Craft event with all-0xF nibbles + ADDRESS_ONLY to skip name match
    // Lower 32 bits = 0xFFFFFF00 | flags; ADDRESS_ONLY = 0x20
    event_id_t search_event = EVENT_TRAIN_SEARCH_SPACE
            | 0xFFFFFF00ULL
            | (uint64_t) TRAIN_SEARCH_FLAG_ADDRESS_ONLY;

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // All-0xF digits means zero valid search digits — no address match
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Prefix mode with more search digits than train address (line 145)
// @coverage _does_address_match — prefix: search_digit_count > train_digit_count
// ============================================================================

TEST(TrainSearch, prefix_more_search_digits_than_train_no_match)
{

    _global_initialize();

    // Train address 3 has 1 digit
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 3, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "12345" (5 digits) in prefix mode on 1-digit train address
    // Use no DCC flag to bypass DCC disambiguation; ADDRESS_ONLY to skip name matching
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            12345, TRAIN_SEARCH_FLAG_ADDRESS_ONLY);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // More search digits than train digits — no match
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Name match with empty string name (line 170)
// @coverage _does_name_match — name[0] == '\0' TRUE branch
// ============================================================================

TEST(TrainSearch, name_match_empty_name_returns_no_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    node->parameters = &_test_node_parameters_empty_name;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42 — won't match address 9999, will try name match
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Empty name — name match returns false, no overall match
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Name match with consecutive digit run — search starts mid-run (line 213)
// @coverage _does_name_match — p > 0 && name[p-1] is digit, skip branch
// ============================================================================

TEST(TrainSearch, name_match_digit_mid_run_skipped)
{

    _global_initialize();

    // Name "AB12CD34" — digit runs: "12" starting at index 2, "34" at index 6
    // Search for "2" should NOT match because '2' is mid-run of "12"
    // but it IS the start of no run by itself. Actually searching for "2":
    // At p=2, name[2]='1', is digit, p>0, name[1]='B' not digit => start of run.
    // Match '1' vs '2' => mismatch, move on.
    // At p=3, name[3]='2', is digit, p>0, name[2]='1' IS digit => skip (line 213).
    // At p=6, name[6]='3', is digit, p>0, name[5]='D' not digit => start of run.
    // Match '3' vs '2' => mismatch.
    // At p=7, name[7]='4', is digit, p>0, name[6]='3' IS digit => skip (line 213).
    // No match found => false
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    node->parameters = &_test_node_parameters_ab12cd34;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "2" — should not match (2 is mid-run in "12", not start)
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            2, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // "2" doesn't start any digit run — no match
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Name match with non-digit separator in name (line 226)
// @coverage _does_name_match — name[np] < '0' || name[np] > '9' skip non-digit
// ============================================================================

TEST(TrainSearch, name_match_non_digit_separator_in_name)
{

    _global_initialize();

    // Name "Unit 1-2-3" — digits interspersed with dashes
    // Search for "123" should match: at run start '1', match '1',
    // skip '-', match '2', skip '-', match '3'
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    node->parameters = &_test_node_parameters_dashes;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "123" in prefix mode — should match "1-2-3" in name
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            123, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Name match succeeds — digits skip over separators
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Name match digit mismatch during matching (line 233)
// @coverage _does_name_match — (name[np] - '0') != seq[si] break
// ============================================================================

TEST(TrainSearch, name_match_digit_mismatch_during_run)
{

    _global_initialize();

    // Name "Train 1357" — search for "13" prefix should match,
    // but search for "139" should fail at the third digit (5 != 9)
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    node->parameters = &_test_node_parameters_1357;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "139" — matches "1", "3" but then "5" != "9" => mismatch
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            139, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Mismatch during name matching — no match
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Name match partial sequence not complete (line 244 FALSE branch)
// @coverage _does_name_match — si < seq_len after loop exits (partial match)
// ============================================================================

TEST(TrainSearch, name_match_partial_sequence_not_complete)
{

    _global_initialize();

    // Name "A5B" — only one digit "5"
    // Search for "56" — matches "5" then runs out of name digits; si=1 < seq_len=2
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    node->parameters = &_test_node_parameters_a5b;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "56" — partial match "5" only, sequence incomplete
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            56, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Partial sequence — no match
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Exact name match — trailing digit fails exact check (line 256 FALSE)
// @coverage _does_name_match — exact match where next char is digit (run continues)
// ============================================================================

TEST(TrainSearch, name_match_exact_trailing_digit_fails)
{

    _global_initialize();

    // Name "Loco 1234 Express" — search for "123" with EXACT flag
    // "123" matches at start of "1234" run, but "4" follows => not exact
    openlcb_node_t *node = _create_named_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "123" WITH EXACT — run "1234" has trailing digit '4'
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            123, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR | TRAIN_SEARCH_FLAG_EXACT);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // "123" is prefix of "1234", but exact mode rejects it
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Exact name match — run ended by non-digit then end of name (line 256 TRUE paths)
// @coverage _does_name_match — np >= name_len sub-condition
// ============================================================================

TEST(TrainSearch, name_match_exact_run_ends_at_name_end)
{

    _global_initialize();

    // Name "Test 42" — digit run "42" is at end of string
    // Search for "42" with EXACT — after matching "42", np moves past end => exact match
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    node->parameters = &_test_node_parameters_test42;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "42" WITH EXACT — matches run "42" at end of name
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR | TRAIN_SEARCH_FLAG_EXACT);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // "42" exactly matches run at end of name
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Name match with NULL parameters (line 334)
// @coverage _does_train_match — params == NULL
// ============================================================================

TEST(TrainSearch, train_match_null_parameters_no_name_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    // Set parameters to NULL so _does_train_match sees params == NULL
    node->parameters = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42 — won't match 9999, name match skipped (params NULL)
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // NULL parameters — no name match possible
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: handle_search_event with NULL statemachine_info (line 362)
// @coverage ProtocolTrainSearch_handle_search_event — statemachine_info == NULL
// ============================================================================

TEST(TrainSearch, handle_search_event_null_statemachine_info)
{

    _global_initialize();

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC);

    // Must not crash
    ProtocolTrainSearch_handle_search_event(NULL, search_event);

    // No crash = success; no way to check output since statemachine is NULL
    EXPECT_EQ(_search_matched_count, 0);

}

// ============================================================================
// TEST: handle_search_event with NULL openlcb_node (line 362)
// @coverage ProtocolTrainSearch_handle_search_event — statemachine_info->openlcb_node == NULL
// ============================================================================

TEST(TrainSearch, handle_search_event_null_openlcb_node)
{

    _global_initialize();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    sm.openlcb_node = NULL;
    sm.incoming_msg_info.msg_ptr = incoming;
    sm.incoming_msg_info.enumerate = false;
    sm.outgoing_msg_info.msg_ptr = outgoing;
    sm.outgoing_msg_info.enumerate = false;
    sm.outgoing_msg_info.valid = false;

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC);

    // Must not crash
    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: handle_search_event with NULL interface pointer (line 416)
// @coverage ProtocolTrainSearch_handle_search_event — _interface == NULL
// ============================================================================

TEST(TrainSearch, handle_search_event_null_interface_pointer)
{

    // Initialize with NULL interface pointer (not null callbacks — null pointer)
    ProtocolTrainSearch_initialize(NULL);
    ProtocolTrainHandler_initialize(&_interface_train);
    OpenLcbApplicationTrain_initialize(&_interface_app_train);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    _reset_tracking();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = ProtocolTrainSearch_create_event_id(42, 0x00);

    // Should match and generate reply but skip callback since _interface is NULL
    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    EXPECT_TRUE(sm.outgoing_msg_info.valid);
    EXPECT_EQ(_search_matched_count, 0);

}

// ============================================================================
// TEST: handle_search_no_match with NULL statemachine_info (line 442)
// @coverage ProtocolTrainSearch_handle_search_no_match — statemachine_info == NULL
// ============================================================================

TEST(TrainSearch, handle_search_no_match_null_statemachine_info)
{

    _global_initialize_with_no_match();

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    // Must not crash
    ProtocolTrainSearch_handle_search_no_match(NULL, search_event);

    EXPECT_EQ(_no_match_count, 0);

}

// ============================================================================
// TEST: handle_search_no_match with NULL interface pointer (line 456)
// @coverage ProtocolTrainSearch_handle_search_no_match — _interface == NULL
// ============================================================================

TEST(TrainSearch, handle_search_no_match_null_interface_pointer)
{

    // Initialize with NULL interface pointer
    ProtocolTrainSearch_initialize(NULL);
    ProtocolTrainHandler_initialize(&_interface_train);
    OpenLcbApplicationTrain_initialize(&_interface_app_train);
    OpenLcbNode_initialize(&_interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    _reset_tracking();

    openlcb_node_t *node = _create_train_node();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    // Must not crash — _interface is NULL
    ProtocolTrainSearch_handle_search_no_match(&sm, search_event);

    EXPECT_EQ(_no_match_count, 0);
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: No-match returns node with train_state == NULL (line 468)
// @coverage ProtocolTrainSearch_handle_search_no_match — new_node->train_state == NULL
// ============================================================================

TEST(TrainSearch, no_match_returns_node_with_null_train_state)
{

    _global_initialize_with_no_match();

    // Create a node WITHOUT train state
    openlcb_node_t *new_node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    new_node->alias = TEST_DEST_ALIAS;
    new_node->train_state = NULL;  // no train state

    // Use a separate node as the statemachine context
    openlcb_node_t *current_node = OpenLcbNode_allocate(0x050101010A00ULL, &_test_node_parameters);
    current_node->alias = 0x0CCC;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, current_node, incoming, outgoing);

    // Callback will return a node with NULL train_state
    _no_match_return_node = new_node;

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    ProtocolTrainSearch_handle_search_no_match(&sm, search_event);

    // Callback invoked
    EXPECT_EQ(_no_match_count, 1);

    // No reply since train_state is NULL
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: No-match returns node with short address (line 472 FALSE branch)
// @coverage ProtocolTrainSearch_handle_search_no_match — is_long_address == false
// ============================================================================

TEST(TrainSearch, no_match_returns_node_with_short_address)
{

    _global_initialize_with_no_match();

    // Create a train node with SHORT address
    openlcb_node_t *new_node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(new_node, 42, false);

    // Use a separate node as the statemachine context
    openlcb_node_t *current_node = OpenLcbNode_allocate(0x050101010A00ULL, &_test_node_parameters);
    current_node->alias = 0x0CCC;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, current_node, incoming, outgoing);

    // Callback returns node with short address
    _no_match_return_node = new_node;

    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    ProtocolTrainSearch_handle_search_no_match(&sm, search_event);

    // Callback invoked
    EXPECT_EQ(_no_match_count, 1);

    // Reply loaded — should echo the search event ID
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

    event_id_t reply_event = OpenLcbUtilities_extract_event_id_from_openlcb_payload(
            sm.outgoing_msg_info.msg_ptr);
    EXPECT_EQ(reply_event, search_event);

}

// ============================================================================
// TEST: Name match with name starting with digit (line 213 p==0 sub-branch)
// @coverage _does_name_match — p > 0 FALSE (p==0 is a digit, start of run)
// ============================================================================

TEST(TrainSearch, name_match_digit_at_position_zero)
{

    _global_initialize();

    // Name "5Train" — digit '5' at p=0
    // Search for "5" — at p=0, name[0]='5' is digit, p > 0 is FALSE → condition FALSE
    // Start matching: '5' == seq[0]='5' → match. Prefix mode → seq_matched = true.
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    node->parameters = &_test_node_parameters_digit_start;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "5" — should match name "5Train" at p=0
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            5, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // "5" matches digit at start of name
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Address match with train DCC address 0 (line 76 TRUE branch)
// @coverage _does_address_match — temp == 0
// ============================================================================

TEST(TrainSearch, address_match_train_address_zero)
{

    _global_initialize();

    // Train has DCC address 0 — exercises temp == 0 branch in _does_address_match
    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 0, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 1 — forces _does_address_match to be called with train_address=0
    // This exercises the temp == 0 TRUE branch at line 76
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            1, TRAIN_SEARCH_FLAG_ADDRESS_ONLY);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Address 0 != 1 → no match (but line 76 was exercised)
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}

// ============================================================================
// TEST: Name match skipped when owner_node is NULL (line 330)
// @coverage _does_train_match — train_state->owner_node == NULL
// ============================================================================

TEST(TrainSearch, train_match_null_owner_node_skips_name_match)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 9999, true);
    // Explicitly NULL out owner_node so name matching is skipped
    node->train_state->owner_node = NULL;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for "42" — address 9999 won't match, name match blocked by NULL owner_node
    event_id_t search_event = ProtocolTrainSearch_create_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // No match — address mismatch and name match skipped
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}
