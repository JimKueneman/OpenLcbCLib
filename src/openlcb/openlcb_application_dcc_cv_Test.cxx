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
 * \file openlcb_application_dcc_cv_Test.cxx
 *
 * Tests for the DCC CV memory space (0xF8) module: immediate and deferred
 * replies, duplicate absorption, the pending table, timeouts, the bus-busy
 * retry path, stale handles, and the reply-time defaults.
 *
 * @author Jim Kueneman
 * @date 25 Sep 2026
 */

#include "test/main_Test.hxx"

#include <string.h>

#include "openlcb_application_dcc_cv.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_utilities.h"

// ============================================================================
// Fixtures
// ============================================================================

static const node_id_t NODE_ID = 0x050101010700ULL;
static const uint16_t NODE_ALIAS = 0x222;
static const node_id_t REQUESTER_ID = 0x050101010701ULL;
static const uint16_t REQUESTER_ALIAS = 0x333;

static openlcb_node_t _node;

static openlcb_msg_t _incoming;
static payload_datagram_t _incoming_payload;
static openlcb_msg_t _outgoing;
static payload_datagram_t _outgoing_payload;
static openlcb_statemachine_info_t _sm;

// ---- send mock -------------------------------------------------------------

static bool _send_ok = true;
static int _send_count = 0;
static openlcb_msg_t _sent;
static uint8_t _sent_payload[LEN_MESSAGE_BYTES_DATAGRAM];

static bool _mock_send(openlcb_msg_t *msg)
{

    if (!_send_ok) {

        return false;

    }

    _send_count++;
    _sent = *msg;
    memset(_sent_payload, 0, sizeof(_sent_payload));

    for (uint16_t i = 0; i < msg->payload_count && i < LEN_MESSAGE_BYTES_DATAGRAM; i++) {

        _sent_payload[i] = *msg->payload[i];

    }

    return true;

}

// ---- application hooks -----------------------------------------------------

static uint16_t _read_result = S_OK;
static uint8_t _read_value = 0x5A;
static int _read_calls = 0;
static uint16_t _write_result = S_OK;
static int _write_calls = 0;
static uint16_t _last_handle = 0;
static uint32_t _last_cv_address = 0;
static uint8_t _last_written = 0;
static openlcb_node_t *_last_hook_node = nullptr;
static bool _complete_inside_hook = false;

static uint16_t _mock_read(openlcb_node_t *node, uint32_t cv_address, uint8_t *value, uint16_t handle)
{

    _read_calls++;
    _last_hook_node = node;
    _last_cv_address = cv_address;
    _last_handle = handle;

    if (_complete_inside_hook) {

        OpenLcbApplicationDccCv_complete(handle, S_OK, 0x99);

        return OPENLCB_DCC_CV_RESULT_PENDING;

    }

    if (_read_result == S_OK) {

        *value = _read_value;

    }

    return _read_result;

}

static uint16_t _mock_write(openlcb_node_t *node, uint32_t cv_address, uint8_t value, uint16_t handle)
{

    _write_calls++;
    _last_hook_node = node;
    _last_cv_address = cv_address;
    _last_written = value;
    _last_handle = handle;

    return _write_result;

}

static uint16_t _app_read_reply_time(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    (void) statemachine_info;
    (void) config_mem_read_request_info;

    return 7;

}

static uint16_t _app_write_reply_time(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    (void) statemachine_info;
    (void) config_mem_write_request_info;

    return 9;

}

// ---- interfaces (fields in declaration order) -----------------------------

static const interface_openlcb_application_dcc_cv_t _iface = {
    .send_openlcb_msg = &_mock_send,
    .dcc_cv_read = &_mock_read,
    .dcc_cv_write = &_mock_write,
    .config_mem_read_delayed_reply_time = &_app_read_reply_time,
    .config_mem_write_delayed_reply_time = &_app_write_reply_time,
};

static const interface_openlcb_application_dcc_cv_t _iface_no_app_reply_time = {
    .send_openlcb_msg = &_mock_send,
    .dcc_cv_read = &_mock_read,
    .dcc_cv_write = &_mock_write,
    .config_mem_read_delayed_reply_time = nullptr,
    .config_mem_write_delayed_reply_time = nullptr,
};

// ---- address spaces --------------------------------------------------------

static const user_address_space_info_t _dcc_cv_space = {
    .present = true,
    .read_only = false,
    .low_address_valid = false,
    .address_space = CONFIG_MEM_SPACE_DCC_CV,
    .highest_address = 1023,
    .low_address = 0,
    .description = "DCC CV",
};

static const user_address_space_info_t _other_space = {
    .present = true,
    .read_only = false,
    .low_address_valid = false,
    .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .highest_address = 0x100,
    .low_address = 0,
    .description = "Config",
};

// ---- helpers ---------------------------------------------------------------

static void _reset(uint8_t tick)
{

    _send_ok = true;
    _send_count = 0;
    memset(&_sent, 0, sizeof(_sent));
    memset(_sent_payload, 0, sizeof(_sent_payload));

    _read_result = S_OK;
    _read_value = 0x5A;
    _read_calls = 0;
    _write_result = S_OK;
    _write_calls = 0;
    _last_handle = 0;
    _last_cv_address = 0;
    _last_written = 0;
    _last_hook_node = nullptr;
    _complete_inside_hook = false;

    memset(&_node, 0, sizeof(_node));
    _node.alias = NODE_ALIAS;
    _node.id = NODE_ID;

    memset(&_incoming, 0, sizeof(_incoming));
    memset(&_incoming_payload, 0, sizeof(_incoming_payload));
    _incoming.payload = (openlcb_payload_t *) &_incoming_payload;
    _incoming.payload_type = DATAGRAM;
    _incoming.mti = MTI_DATAGRAM;
    _incoming.source_alias = REQUESTER_ALIAS;
    _incoming.source_id = REQUESTER_ID;
    _incoming.dest_alias = NODE_ALIAS;
    _incoming.dest_id = NODE_ID;
    *_incoming.payload[0] = CONFIG_MEM_CONFIGURATION;

    memset(&_outgoing, 0, sizeof(_outgoing));
    memset(&_outgoing_payload, 0, sizeof(_outgoing_payload));
    _outgoing.payload = (openlcb_payload_t *) &_outgoing_payload;
    _outgoing.payload_type = DATAGRAM;

    memset(&_sm, 0, sizeof(_sm));
    _sm.openlcb_node = &_node;
    _sm.incoming_msg_info.msg_ptr = &_incoming;
    _sm.outgoing_msg_info.msg_ptr = &_outgoing;
    _sm.outgoing_msg_info.valid = false;
    _sm.current_tick = tick;

    OpenLcbApplicationDccCv_initialize(&_iface);

}

static void _build_read(config_mem_read_request_info_t *info, uint32_t address, uint16_t bytes)
{

    *_incoming.payload[1] = CONFIG_MEM_READ_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(&_incoming, address, 2);
    *_incoming.payload[6] = CONFIG_MEM_SPACE_DCC_CV;
    *_incoming.payload[7] = (uint8_t) bytes;
    _incoming.payload_count = 8;

    info->encoding = ADDRESS_SPACE_IN_BYTE_6;
    info->address = address;
    info->bytes = bytes;
    info->data_start = 7;
    info->space_info = &_dcc_cv_space;
    info->read_space_func = &OpenLcbApplicationDccCv_handle_read_request;

    _sm.outgoing_msg_info.valid = false;

}

static void _build_write(config_mem_write_request_info_t *info, uint32_t address, uint8_t value, uint16_t bytes)
{

    *_incoming.payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(&_incoming, address, 2);
    *_incoming.payload[6] = CONFIG_MEM_SPACE_DCC_CV;
    *_incoming.payload[7] = value;
    _incoming.payload_count = 7 + bytes;

    info->encoding = ADDRESS_SPACE_IN_BYTE_6;
    info->address = address;
    info->bytes = bytes;
    info->data_start = 7;
    info->space_info = &_dcc_cv_space;
    info->write_buffer = (configuration_memory_buffer_t *) &_incoming.payload[7];
    info->write_space_func = &OpenLcbApplicationDccCv_handle_write_request;

    _sm.outgoing_msg_info.valid = false;

}

static uint32_t _dword_at(const uint8_t *p)
{

    return ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3];

}

static uint16_t _word_at(const uint8_t *p)
{

    return (uint16_t) (((uint16_t) p[0] << 8) | p[1]);

}

// Checks the common header of a reply that went out on the bus via the send mock.
static void _expect_sent_header(uint8_t command, uint32_t address)
{

    EXPECT_EQ(_sent.mti, MTI_DATAGRAM);
    EXPECT_EQ(_sent.source_alias, NODE_ALIAS);
    EXPECT_EQ(_sent.source_id, NODE_ID);
    EXPECT_EQ(_sent.dest_alias, REQUESTER_ALIAS);
    EXPECT_EQ(_sent.dest_id, REQUESTER_ID);
    EXPECT_EQ(_sent_payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(_sent_payload[1], command);
    EXPECT_EQ(_dword_at(&_sent_payload[2]), address);
    EXPECT_EQ(_sent_payload[6], CONFIG_MEM_SPACE_DCC_CV);

}

// Checks the common header of a reply loaded into the state machine's outgoing message.
static void _expect_outgoing_header(uint8_t command, uint32_t address)
{

    EXPECT_TRUE(_sm.outgoing_msg_info.valid);
    EXPECT_EQ(_outgoing.mti, MTI_DATAGRAM);
    EXPECT_EQ(_outgoing.source_alias, NODE_ALIAS);
    EXPECT_EQ(_outgoing.dest_alias, REQUESTER_ALIAS);
    EXPECT_EQ(*_outgoing.payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*_outgoing.payload[1], command);
    EXPECT_EQ(OpenLcbUtilities_extract_dword_from_openlcb_payload(&_outgoing, 2), address);
    EXPECT_EQ(*_outgoing.payload[6], CONFIG_MEM_SPACE_DCC_CV);

}

// ============================================================================
// Immediate replies (hook answers inside the call)
// ============================================================================

TEST(DccCv, read_immediate_ok)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 28, 1);   // CV 29

    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    EXPECT_EQ(_read_calls, 1);
    EXPECT_EQ(_last_hook_node, &_node);
    EXPECT_EQ(_last_cv_address, 28u);
    _expect_outgoing_header(CONFIG_MEM_READ_REPLY_OK_SPACE_IN_BYTE_6, 28);
    EXPECT_EQ(*_outgoing.payload[7], 0x5A);
    EXPECT_EQ(_outgoing.payload_count, 8);

    // Nothing left in the table: the same request again runs the hook again
    _build_read(&info, 28, 1);
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    EXPECT_EQ(_read_calls, 2);

    // and run() has nothing to send
    OpenLcbApplicationDccCv_run(1);
    EXPECT_EQ(_send_count, 0);

}

TEST(DccCv, read_immediate_error)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 0, 1);
    _read_result = OPENLCB_DCC_CV_ERROR_NO_LOCO;

    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    EXPECT_EQ(_read_calls, 1);
    _expect_outgoing_header(CONFIG_MEM_READ_REPLY_FAIL_SPACE_IN_BYTE_6, 0);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(&_outgoing, 7), OPENLCB_DCC_CV_ERROR_NO_LOCO);
    EXPECT_EQ(_outgoing.payload_count, 9);

}

TEST(DccCv, read_wrong_length_rejected_without_calling_hook)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 0, 2);

    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    EXPECT_EQ(_read_calls, 0);
    _expect_outgoing_header(CONFIG_MEM_READ_REPLY_FAIL_SPACE_IN_BYTE_6, 0);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(&_outgoing, 7), ERROR_PERMANENT_INVALID_ARGUMENTS);

}

TEST(DccCv, write_immediate_ok)
{

    _reset(0);
    config_mem_write_request_info_t info;
    _build_write(&info, 2, 0x7F, 1);   // CV 3 = 127

    OpenLcbApplicationDccCv_handle_write_request(&_sm, &info);

    EXPECT_EQ(_write_calls, 1);
    EXPECT_EQ(_last_cv_address, 2u);
    EXPECT_EQ(_last_written, 0x7F);
    _expect_outgoing_header(CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6, 2);
    EXPECT_EQ(_outgoing.payload_count, 7);

}

TEST(DccCv, write_immediate_error)
{

    _reset(0);
    config_mem_write_request_info_t info;
    _build_write(&info, 2, 0x7F, 1);
    _write_result = OPENLCB_DCC_CV_ERROR_PROGRAMMING_SHORT;

    OpenLcbApplicationDccCv_handle_write_request(&_sm, &info);

    EXPECT_EQ(_write_calls, 1);
    _expect_outgoing_header(CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6, 2);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(&_outgoing, 7), OPENLCB_DCC_CV_ERROR_PROGRAMMING_SHORT);
    EXPECT_EQ(_outgoing.payload_count, 9);

}

TEST(DccCv, write_wrong_length_rejected_without_calling_hook)
{

    _reset(0);
    config_mem_write_request_info_t info;
    _build_write(&info, 2, 0x7F, 2);

    OpenLcbApplicationDccCv_handle_write_request(&_sm, &info);

    EXPECT_EQ(_write_calls, 0);
    _expect_outgoing_header(CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6, 2);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(&_outgoing, 7), ERROR_PERMANENT_INVALID_ARGUMENTS);

}

// ============================================================================
// Deferred replies
// ============================================================================

TEST(DccCv, read_pending_then_complete_sends_reply_from_run)
{

    _reset(10);
    config_mem_read_request_info_t info;
    _build_read(&info, 28, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;

    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    // Nothing goes out inside the call
    EXPECT_EQ(_read_calls, 1);
    EXPECT_FALSE(_sm.outgoing_msg_info.valid);
    EXPECT_NE(_last_handle, 0);

    // Nothing goes out before the application completes it either
    OpenLcbApplicationDccCv_run(11);
    EXPECT_EQ(_send_count, 0);

    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(_last_handle, S_OK, 0x77));
    EXPECT_EQ(_send_count, 0);   // complete() itself never sends

    OpenLcbApplicationDccCv_run(12);
    EXPECT_EQ(_send_count, 1);
    _expect_sent_header(CONFIG_MEM_READ_REPLY_OK_SPACE_IN_BYTE_6, 28);
    EXPECT_EQ(_sent_payload[7], 0x77);
    EXPECT_EQ(_sent.payload_count, 8);

    // Sent once only, and the handle is now stale
    OpenLcbApplicationDccCv_run(13);
    EXPECT_EQ(_send_count, 1);
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(_last_handle, S_OK, 0x77));

}

TEST(DccCv, read_pending_then_complete_with_error)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 5, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;

    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(_last_handle, OPENLCB_DCC_CV_ERROR_NO_RAILCOM, 0));
    OpenLcbApplicationDccCv_run(1);

    EXPECT_EQ(_send_count, 1);
    _expect_sent_header(CONFIG_MEM_READ_REPLY_FAIL_SPACE_IN_BYTE_6, 5);
    EXPECT_EQ(_word_at(&_sent_payload[7]), OPENLCB_DCC_CV_ERROR_NO_RAILCOM);
    EXPECT_EQ(_sent.payload_count, 9);

}

TEST(DccCv, write_pending_then_complete_ok_and_error)
{

    _reset(0);
    config_mem_write_request_info_t info;

    // OK
    _build_write(&info, 2, 0x7F, 1);
    _write_result = OPENLCB_DCC_CV_RESULT_PENDING;
    OpenLcbApplicationDccCv_handle_write_request(&_sm, &info);
    EXPECT_FALSE(_sm.outgoing_msg_info.valid);
    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(_last_handle, S_OK, 0));
    OpenLcbApplicationDccCv_run(1);

    EXPECT_EQ(_send_count, 1);
    _expect_sent_header(CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6, 2);
    EXPECT_EQ(_sent.payload_count, 7);

    // Error
    _build_write(&info, 3, 0x01, 1);
    OpenLcbApplicationDccCv_handle_write_request(&_sm, &info);
    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(_last_handle, OPENLCB_DCC_CV_ERROR_VERIFY_FAILED, 0));
    OpenLcbApplicationDccCv_run(2);

    EXPECT_EQ(_send_count, 2);
    _expect_sent_header(CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6, 3);
    EXPECT_EQ(_word_at(&_sent_payload[7]), OPENLCB_DCC_CV_ERROR_VERIFY_FAILED);
    EXPECT_EQ(_sent.payload_count, 9);

}

TEST(DccCv, complete_called_inside_hook_still_works)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 1, 1);
    _complete_inside_hook = true;

    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    EXPECT_FALSE(_sm.outgoing_msg_info.valid);
    OpenLcbApplicationDccCv_run(1);
    EXPECT_EQ(_send_count, 1);
    _expect_sent_header(CONFIG_MEM_READ_REPLY_OK_SPACE_IN_BYTE_6, 1);
    EXPECT_EQ(_sent_payload[7], 0x99);

}

// ============================================================================
// Duplicates and the pending table
// ============================================================================

TEST(DccCv, duplicate_request_absorbed_and_answered_once)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 28, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;

    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    uint16_t handle = _last_handle;

    // The requester re-sends the identical request while the first is outstanding
    _build_read(&info, 28, 1);
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    EXPECT_EQ(_read_calls, 1);                 // no second operation on the track
    EXPECT_FALSE(_sm.outgoing_msg_info.valid); // and no reply of its own

    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(handle, S_OK, 0x11));
    OpenLcbApplicationDccCv_run(1);
    OpenLcbApplicationDccCv_run(2);

    EXPECT_EQ(_send_count, 1);                 // one reply answers both

}

TEST(DccCv, different_address_from_same_requester_is_not_a_duplicate)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;

    _build_read(&info, 28, 1);
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    uint16_t first = _last_handle;

    _build_read(&info, 29, 1);
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    uint16_t second = _last_handle;

    EXPECT_EQ(_read_calls, 2);
    EXPECT_NE(first, second);

    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(first, S_OK, 1));
    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(second, S_OK, 2));
    OpenLcbApplicationDccCv_run(1);
    EXPECT_EQ(_send_count, 2);

}

TEST(DccCv, write_duplicate_needs_same_value)
{

    _reset(0);
    config_mem_write_request_info_t info;
    _write_result = OPENLCB_DCC_CV_RESULT_PENDING;

    _build_write(&info, 2, 0x10, 1);
    OpenLcbApplicationDccCv_handle_write_request(&_sm, &info);

    // Same CV, same value: absorbed
    _build_write(&info, 2, 0x10, 1);
    OpenLcbApplicationDccCv_handle_write_request(&_sm, &info);
    EXPECT_EQ(_write_calls, 1);

    // Same CV, different value: a new operation
    _build_write(&info, 2, 0x20, 1);
    OpenLcbApplicationDccCv_handle_write_request(&_sm, &info);
    EXPECT_EQ(_write_calls, 2);

}

TEST(DccCv, table_full_answers_temporary_buffer_unavailable)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;

    for (uint32_t i = 0; i < USER_DEFINED_DCC_CV_PENDING_COUNT; i++) {

        _build_read(&info, 100 + i, 1);
        OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
        EXPECT_FALSE(_sm.outgoing_msg_info.valid);

    }

    EXPECT_EQ(_read_calls, USER_DEFINED_DCC_CV_PENDING_COUNT);

    // One more does not reach the hook and is refused with a temporary error
    _build_read(&info, 200, 1);
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    EXPECT_EQ(_read_calls, USER_DEFINED_DCC_CV_PENDING_COUNT);
    _expect_outgoing_header(CONFIG_MEM_READ_REPLY_FAIL_SPACE_IN_BYTE_6, 200);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(&_outgoing, 7), ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

}

TEST(DccCv, handle_encodes_slot_and_generation)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;

    _build_read(&info, 1, 1);
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    EXPECT_LT(_last_handle & 0xFF, USER_DEFINED_DCC_CV_PENDING_COUNT);
    EXPECT_NE(_last_handle >> 8, 0);

    // Complete it, then reuse the slot: the generation changes, so the old handle is stale
    uint16_t old_handle = _last_handle;
    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(old_handle, S_OK, 0));
    OpenLcbApplicationDccCv_run(1);

    _build_read(&info, 1, 1);
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    EXPECT_EQ(_last_handle & 0xFF, old_handle & 0xFF);
    EXPECT_NE(_last_handle, old_handle);
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(old_handle, S_OK, 0));

}

// ============================================================================
// Stale handles and bad arguments to complete()
// ============================================================================

TEST(DccCv, complete_rejects_unknown_stale_and_pending_result)
{

    _reset(0);

    // Nothing outstanding at all
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(0, S_OK, 0));
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(0x0100, S_OK, 0));
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(0x01FF, S_OK, 0));   // slot index out of range

    config_mem_read_request_info_t info;
    _build_read(&info, 1, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    // Wrong generation, and the PENDING value itself as a result
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete((uint16_t) (_last_handle ^ 0x0100), S_OK, 0));
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(_last_handle, OPENLCB_DCC_CV_RESULT_PENDING, 0));

    // The right handle still works once
    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(_last_handle, S_OK, 0));
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(_last_handle, S_OK, 0));

}

TEST(DccCv, initialize_forgets_outstanding_requests)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 1, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    uint16_t handle = _last_handle;

    OpenLcbApplicationDccCv_initialize(&_iface);

    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(handle, S_OK, 0));
    OpenLcbApplicationDccCv_run(1);
    EXPECT_EQ(_send_count, 0);

}

// ============================================================================
// Timeouts and the bus
// ============================================================================

TEST(DccCv, unanswered_request_times_out_with_temporary_error)
{

    _reset(50);
    config_mem_read_request_info_t info;
    _build_read(&info, 28, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    uint16_t handle = _last_handle;

    // One tick short: still waiting
    OpenLcbApplicationDccCv_run((uint8_t) (50 + USER_DEFINED_DCC_CV_TIMEOUT_TICKS - 1));
    EXPECT_EQ(_send_count, 0);

    // At the timeout: a temporary time-out reply goes out
    OpenLcbApplicationDccCv_run((uint8_t) (50 + USER_DEFINED_DCC_CV_TIMEOUT_TICKS));
    EXPECT_EQ(_send_count, 1);
    _expect_sent_header(CONFIG_MEM_READ_REPLY_FAIL_SPACE_IN_BYTE_6, 28);
    EXPECT_EQ(_word_at(&_sent_payload[7]), ERROR_TEMPORARY_TIME_OUT);

    // A late completion is refused
    EXPECT_FALSE(OpenLcbApplicationDccCv_complete(handle, S_OK, 0));

}

TEST(DccCv, timeout_survives_tick_wraparound)
{

    _reset(250);
    config_mem_read_request_info_t info;
    _build_read(&info, 1, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    OpenLcbApplicationDccCv_run((uint8_t) (250 + USER_DEFINED_DCC_CV_TIMEOUT_TICKS - 1));
    EXPECT_EQ(_send_count, 0);
    OpenLcbApplicationDccCv_run((uint8_t) (250 + USER_DEFINED_DCC_CV_TIMEOUT_TICKS));
    EXPECT_EQ(_send_count, 1);
    EXPECT_EQ(_word_at(&_sent_payload[7]), ERROR_TEMPORARY_TIME_OUT);

}

TEST(DccCv, busy_bus_retries_until_sent)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 28, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(_last_handle, S_OK, 0x42));

    _send_ok = false;
    OpenLcbApplicationDccCv_run(1);
    OpenLcbApplicationDccCv_run(2);
    EXPECT_EQ(_send_count, 0);

    _send_ok = true;
    OpenLcbApplicationDccCv_run(3);
    EXPECT_EQ(_send_count, 1);
    EXPECT_EQ(_sent_payload[7], 0x42);

    OpenLcbApplicationDccCv_run(4);
    EXPECT_EQ(_send_count, 1);

}

TEST(DccCv, reply_stuck_on_dead_bus_is_dropped_after_timeout)
{

    _reset(0);
    config_mem_read_request_info_t info;
    _build_read(&info, 28, 1);
    _read_result = OPENLCB_DCC_CV_RESULT_PENDING;
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);

    OpenLcbApplicationDccCv_run(0);   // run() stamps the tick complete() will use
    EXPECT_TRUE(OpenLcbApplicationDccCv_complete(_last_handle, S_OK, 0x42));

    _send_ok = false;
    OpenLcbApplicationDccCv_run((uint8_t) (USER_DEFINED_DCC_CV_TIMEOUT_TICKS - 1));
    OpenLcbApplicationDccCv_run((uint8_t) USER_DEFINED_DCC_CV_TIMEOUT_TICKS);   // gives up here

    _send_ok = true;
    OpenLcbApplicationDccCv_run((uint8_t) (USER_DEFINED_DCC_CV_TIMEOUT_TICKS + 1));
    EXPECT_EQ(_send_count, 0);

    // and the slot is free again
    _build_read(&info, 28, 1);
    OpenLcbApplicationDccCv_handle_read_request(&_sm, &info);
    EXPECT_EQ(_read_calls, 2);

}

// ============================================================================
// Reply time advertised in Datagram Received OK
// ============================================================================

TEST(DccCv, reply_time_default_for_cv_space_and_forwarding_for_others)
{

    _reset(0);
    config_mem_read_request_info_t read_info;
    config_mem_write_request_info_t write_info;
    _build_read(&read_info, 1, 1);
    _build_write(&write_info, 1, 0, 1);

    EXPECT_EQ(OpenLcbApplicationDccCv_read_delayed_reply_time(&_sm, &read_info), USER_DEFINED_DCC_CV_REPLY_TIME_SECONDS);
    EXPECT_EQ(OpenLcbApplicationDccCv_write_delayed_reply_time(&_sm, &write_info), USER_DEFINED_DCC_CV_REPLY_TIME_SECONDS);

    read_info.space_info = &_other_space;
    write_info.space_info = &_other_space;

    EXPECT_EQ(OpenLcbApplicationDccCv_read_delayed_reply_time(&_sm, &read_info), 7);
    EXPECT_EQ(OpenLcbApplicationDccCv_write_delayed_reply_time(&_sm, &write_info), 9);

}

TEST(DccCv, reply_time_is_zero_for_other_spaces_without_application_callback)
{

    _reset(0);
    OpenLcbApplicationDccCv_initialize(&_iface_no_app_reply_time);

    config_mem_read_request_info_t read_info;
    config_mem_write_request_info_t write_info;
    _build_read(&read_info, 1, 1);
    _build_write(&write_info, 1, 0, 1);

    EXPECT_EQ(OpenLcbApplicationDccCv_read_delayed_reply_time(&_sm, &read_info), USER_DEFINED_DCC_CV_REPLY_TIME_SECONDS);
    EXPECT_EQ(OpenLcbApplicationDccCv_write_delayed_reply_time(&_sm, &write_info), USER_DEFINED_DCC_CV_REPLY_TIME_SECONDS);

    read_info.space_info = &_other_space;
    write_info.space_info = &_other_space;

    EXPECT_EQ(OpenLcbApplicationDccCv_read_delayed_reply_time(&_sm, &read_info), 0);
    EXPECT_EQ(OpenLcbApplicationDccCv_write_delayed_reply_time(&_sm, &write_info), 0);

}

TEST(DccCv, default_reply_time_fits_inside_completion_timeout)
{

    EXPECT_LE(USER_DEFINED_DCC_CV_REPLY_TIME_SECONDS * 10, USER_DEFINED_DCC_CV_TIMEOUT_TICKS);

}
