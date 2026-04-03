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
 * @file protocol_stream_handler_Test.cxx
 * @brief Test suite for Stream Transport protocol handler.
 *
 * @details Tests stream initiation (accept/reject), data transfer with
 * window tracking, proceed flow control, completion, and error cases.
 *
 * Test Organization:
 * - Section 1: Initialization and basic accept/reject
 * - Section 2: Data transfer and flow control
 * - Section 3: Stream completion
 * - Section 4: Error cases and edge conditions
 *
 * @author Jim Kueneman
 * @date 03 Apr 2026
 */

#include "test/main_Test.hxx"
#include "string.h"

#include "protocol_stream_handler.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

// ============================================================================
// TEST CONFIGURATION CONSTANTS
// ============================================================================

#define SOURCE_ALIAS 0x222
#define SOURCE_ID    0x010203040506
#define DEST_ALIAS   0xBBB
#define DEST_ID      0x060504030201

// ============================================================================
// MOCK CALLBACK TRACKING
// ============================================================================

static int _initiate_request_called = 0;
static int _initiate_reply_called = 0;
static int _data_received_called = 0;
static int _data_proceed_called = 0;
static int _complete_called = 0;

static stream_state_t *_last_stream = NULL;

// ============================================================================
// MOCK CALLBACKS
// ============================================================================

static bool _mock_accept_initiate = true;

static bool _mock_on_initiate_request(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _initiate_request_called++;
    _last_stream = stream;
    return _mock_accept_initiate;

}

static void _mock_on_initiate_reply(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _initiate_reply_called++;
    _last_stream = stream;

}

static void _mock_on_data_received(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _data_received_called++;
    _last_stream = stream;

}

static void _mock_on_data_proceed(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _data_proceed_called++;
    _last_stream = stream;

}

static void _mock_on_complete(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _complete_called++;
    _last_stream = stream;

}

// ============================================================================
// INTERFACE CONFIGURATIONS
// ============================================================================

static const interface_protocol_stream_handler_t _interface_full = {

    .on_initiate_request = &_mock_on_initiate_request,
    .on_initiate_reply   = &_mock_on_initiate_reply,
    .on_data_received    = &_mock_on_data_received,
    .on_data_proceed     = &_mock_on_data_proceed,
    .on_complete         = &_mock_on_complete,

};

static const interface_protocol_stream_handler_t _interface_no_initiate = {

    .on_initiate_request = NULL,
    .on_initiate_reply   = &_mock_on_initiate_reply,
    .on_data_received    = &_mock_on_data_received,
    .on_data_proceed     = &_mock_on_data_proceed,
    .on_complete         = &_mock_on_complete,

};

static const interface_protocol_stream_handler_t _interface_null = {

    .on_initiate_request = NULL,
    .on_initiate_reply   = NULL,
    .on_data_received    = NULL,
    .on_data_proceed     = NULL,
    .on_complete         = NULL,

};

// ============================================================================
// NODE PARAMETERS (minimal, just need a valid node)
// ============================================================================

static const node_parameters_t _node_params = {

    .snip = {
        .name = "Test Mfg",
        .model = "Test Model",
        .hardware_version = "1.0",
        .software_version = "1.0",
    },
    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_STREAM,

};

// ============================================================================
// HELPERS
// ============================================================================

static void _reset_mock_counters(void) {

    _initiate_request_called = 0;
    _initiate_reply_called = 0;
    _data_received_called = 0;
    _data_proceed_called = 0;
    _complete_called = 0;
    _last_stream = NULL;
    _mock_accept_initiate = true;

}

static void _global_initialize(const interface_protocol_stream_handler_t *iface) {

    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbNode_initialize(NULL);
    ProtocolStreamHandler_initialize(iface);
    _reset_mock_counters();

}

static openlcb_statemachine_info_t _build_statemachine_info(
            openlcb_node_t *node,
            openlcb_msg_t *incoming,
            openlcb_msg_t *outgoing) {

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));
    info.openlcb_node = node;
    info.incoming_msg_info.msg_ptr = incoming;
    info.incoming_msg_info.enumerate = false;
    info.outgoing_msg_info.msg_ptr = outgoing;
    info.outgoing_msg_info.enumerate = false;
    info.outgoing_msg_info.valid = false;
    return info;

}

static void _load_initiate_request(openlcb_msg_t *msg, uint16_t buffer_size, uint8_t sid) {

    OpenLcbUtilities_load_openlcb_message(msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REQUEST);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, buffer_size, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, 0, 2);  // flags (deprecated)
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, sid, 4);
    msg->payload_count = 5;

}

static void _load_initiate_request_with_content_uid(openlcb_msg_t *msg, uint16_t buffer_size, uint8_t sid) {

    _load_initiate_request(msg, buffer_size, sid);
    // Add proposed DID at byte 5
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0x10, 5);
    // Add 6-byte content UID at bytes 6-11
    for (int i = 0; i < 6; i++) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0xA0 + i, 6 + i);

    }
    msg->payload_count = 12;

}

// ============================================================================
// SECTION 1: INITIALIZATION AND INITIATE REQUEST/REPLY
// ============================================================================

TEST(ProtocolStreamHandler, initialize) {

    _global_initialize(&_interface_full);
    SUCCEED();

}

// ============================================================================
// TEST: Initiate request accepted with negotiated buffer size
// ============================================================================

TEST(ProtocolStreamHandler, initiate_request_accept) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_initiate_request(incoming, 128, 0x01);

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_request(&info);

    // Callback was called
    EXPECT_EQ(_initiate_request_called, 1);
    EXPECT_NE(_last_stream, nullptr);

    // Reply was sent
    EXPECT_TRUE(info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing->mti, MTI_STREAM_INIT_REPLY);

    // Source/dest swapped in reply
    EXPECT_EQ(outgoing->source_alias, DEST_ALIAS);
    EXPECT_EQ(outgoing->source_id, DEST_ID);
    EXPECT_EQ(outgoing->dest_alias, SOURCE_ALIAS);
    EXPECT_EQ(outgoing->dest_id, SOURCE_ID);

    // Payload: buffer size (bytes 0-1), accept flag (bytes 2-3), SID (byte 4), DID (byte 5)
    EXPECT_EQ(outgoing->payload_count, 6);

    uint16_t reply_buffer_size = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 0);
    uint16_t reply_flags = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 2);
    uint8_t reply_sid = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4);

    EXPECT_EQ(reply_buffer_size, 128);
    EXPECT_EQ(reply_flags, STREAM_REPLY_ACCEPT);
    EXPECT_EQ(reply_sid, 0x01);

    // Stream state is correct
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);
    EXPECT_EQ(_last_stream->source_stream_id, 0x01);
    EXPECT_EQ(_last_stream->max_buffer_size, 128);
    EXPECT_EQ(_last_stream->is_source, false);
    EXPECT_EQ(_last_stream->remote_node_id, SOURCE_ID);

}

// ============================================================================
// TEST: Initiate request negotiates buffer size down
// ============================================================================

TEST(ProtocolStreamHandler, initiate_request_buffer_negotiation) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    // Propose a buffer size larger than LEN_MESSAGE_BYTES_STREAM
    _load_initiate_request(incoming, 65535, 0x02);

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_request(&info);

    EXPECT_TRUE(info.outgoing_msg_info.valid);

    uint16_t reply_buffer_size = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 0);

    // Should be negotiated down to LEN_MESSAGE_BYTES_STREAM
    EXPECT_LE(reply_buffer_size, LEN_MESSAGE_BYTES_STREAM);
    EXPECT_GT(reply_buffer_size, 0);

}

// ============================================================================
// TEST: Initiate request rejected -- no callback (streams not supported)
// ============================================================================

TEST(ProtocolStreamHandler, initiate_request_reject_no_callback) {

    _global_initialize(&_interface_no_initiate);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    _load_initiate_request(incoming, 128, 0x03);

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_request(&info);

    // No callback was called
    EXPECT_EQ(_initiate_request_called, 0);

    // Reject reply was sent
    EXPECT_TRUE(info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing->mti, MTI_STREAM_INIT_REPLY);

    uint16_t reply_buffer_size = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 0);
    uint16_t reply_flags = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 2);

    EXPECT_EQ(reply_buffer_size, 0);
    EXPECT_EQ(reply_flags, ERROR_PERMANENT_STREAMS_NOT_SUPPORTED);

}

// ============================================================================
// TEST: Initiate request rejected -- stream table full
// ============================================================================

TEST(ProtocolStreamHandler, initiate_request_reject_table_full) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Fill the stream table (USER_DEFINED_MAX_STREAM_COUNT = 1 by default)
    openlcb_msg_t *incoming1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming1, 128, 0x10);
    openlcb_statemachine_info_t info1 = _build_statemachine_info(node, incoming1, outgoing1);
    ProtocolStreamHandler_initiate_request(&info1);
    EXPECT_EQ(_initiate_request_called, 1);
    EXPECT_TRUE(info1.outgoing_msg_info.valid);

    // Second request should be rejected -- table full
    _reset_mock_counters();
    openlcb_msg_t *incoming2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming2, 128, 0x11);
    openlcb_statemachine_info_t info2 = _build_statemachine_info(node, incoming2, outgoing2);
    ProtocolStreamHandler_initiate_request(&info2);

    EXPECT_EQ(_initiate_request_called, 0);
    EXPECT_TRUE(info2.outgoing_msg_info.valid);

    uint16_t reply_buffer_size = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing2, 0);
    uint16_t reply_flags = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing2, 2);

    EXPECT_EQ(reply_buffer_size, 0);
    EXPECT_EQ(reply_flags, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

}

// ============================================================================
// TEST: Initiate request with 12-byte message (includes content UID)
// ============================================================================

TEST(ProtocolStreamHandler, initiate_request_with_content_uid) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    _load_initiate_request_with_content_uid(incoming, 200, 0x05);

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_request(&info);

    EXPECT_EQ(_initiate_request_called, 1);
    EXPECT_NE(_last_stream, nullptr);

    // Verify content UID was extracted
    EXPECT_EQ(_last_stream->content_uid[0], 0xA0);
    EXPECT_EQ(_last_stream->content_uid[1], 0xA1);
    EXPECT_EQ(_last_stream->content_uid[5], 0xA5);

}

// ============================================================================
// TEST: Initiate reply -- accept
// ============================================================================

TEST(ProtocolStreamHandler, initiate_reply_accept) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // First, simulate that we (as source) sent an initiate request.
    // We need a stream in INITIATED state. Do this by sending an initiate
    // request to ourselves, which creates a stream entry as destination.
    // Instead, we'll test the reply handler by first creating a stream
    // through initiate_request, then testing reply on a fresh init.

    // Actually, the reply handler looks up by SID + remote node. We need
    // to manually create the scenario: we are source, we sent an initiate
    // request and are waiting for a reply.
    //
    // The simplest approach: send an initiate request (this node as dest),
    // then complete it to free the slot. Then the reply test needs a stream
    // in INITIATED state with is_source=true.
    //
    // Since we can't directly create that state through the public API alone
    // (initiate_request creates dest-role streams), let's test that the reply
    // handler correctly ignores unknown streams and processes known ones.

    // Send initiate request to create a stream (we are destination)
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x20);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_initiate_request_called, 1);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    // Now construct an initiate reply that references the SID 0x20 from SOURCE_ID.
    // This stream exists (we are destination, SID=0x20).
    // The reply handler searches by SID + remote node -- it should find it.
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_reply, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_reply);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 128, 0);      // buffer size
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, STREAM_REPLY_ACCEPT, 2);  // accept
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x20, 4);     // SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x30, 5);     // DID
    incoming_reply->payload_count = 6;

    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);

    ProtocolStreamHandler_initiate_reply(&info_reply);

    EXPECT_EQ(_initiate_reply_called, 1);

}

// ============================================================================
// TEST: Initiate reply -- unknown stream ignored
// ============================================================================

TEST(ProtocolStreamHandler, initiate_reply_unknown_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    // Send a reply for a stream that doesn't exist
    OpenLcbUtilities_load_openlcb_message(incoming, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, 128, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming, STREAM_REPLY_ACCEPT, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x99, 4);  // unknown SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x88, 5);
    incoming->payload_count = 6;

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_reply(&info);

    // Should be silently ignored -- no callback
    EXPECT_EQ(_initiate_reply_called, 0);

}

// ============================================================================
// SECTION 2: DATA TRANSFER AND FLOW CONTROL
// ============================================================================

// ============================================================================
// TEST: Data send -- received and tracked
// ============================================================================

TEST(ProtocolStreamHandler, data_send_basic) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream first
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x30);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);
    uint8_t did = _last_stream->dest_stream_id;

    // Now send stream data
    _reset_mock_counters();
    openlcb_msg_t *incoming_data = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_data = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_data, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_data);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, did, 0);  // DID
    // 7 bytes of payload data
    for (int i = 0; i < 7; i++) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, 0x40 + i, 1 + i);

    }
    incoming_data->payload_count = 8;  // 1 (DID) + 7 (data)

    openlcb_statemachine_info_t info_data = _build_statemachine_info(node, incoming_data, outgoing_data);

    ProtocolStreamHandler_data_send(&info_data);

    EXPECT_EQ(_data_received_called, 1);
    EXPECT_NE(_last_stream, nullptr);
    EXPECT_EQ(_last_stream->bytes_transferred, (uint32_t)7);
    EXPECT_EQ(_last_stream->bytes_remaining, (uint16_t)(128 - 7));

}

// ============================================================================
// TEST: Data send -- unknown stream ignored
// ============================================================================

TEST(ProtocolStreamHandler, data_send_unknown_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xEE, 0);  // unknown DID
    incoming->payload_count = 5;

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_data_send(&info);

    EXPECT_EQ(_data_received_called, 0);

}

// ============================================================================
// TEST: Data proceed -- extends send window
// ============================================================================

TEST(ProtocolStreamHandler, data_proceed_extends_window) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream (we are destination)
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x40);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);

    stream_state_t *stream = _last_stream;
    EXPECT_EQ(stream->state, STREAM_STATE_OPEN);
    uint16_t initial_remaining = stream->bytes_remaining;

    // Now simulate receiving a proceed (byte 0 = SID, byte 1 = DID)
    _reset_mock_counters();
    openlcb_msg_t *incoming_proceed = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_proceed = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_proceed, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_PROCEED);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_proceed);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_proceed, 0x40, 0);  // SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_proceed, stream->dest_stream_id, 1);  // DID
    incoming_proceed->payload_count = 2;

    openlcb_statemachine_info_t info_proceed = _build_statemachine_info(node, incoming_proceed, outgoing_proceed);

    ProtocolStreamHandler_data_proceed(&info_proceed);

    EXPECT_EQ(_data_proceed_called, 1);
    EXPECT_EQ(stream->bytes_remaining, initial_remaining + 128);

}

// ============================================================================
// SECTION 3: STREAM COMPLETION
// ============================================================================

// ============================================================================
// TEST: Stream complete -- frees stream entry
// ============================================================================

TEST(ProtocolStreamHandler, data_complete_frees_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x50);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);
    uint8_t did = _last_stream->dest_stream_id;

    // Send complete
    _reset_mock_counters();
    openlcb_msg_t *incoming_complete = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_complete = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_complete, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_COMPLETE);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_complete);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_complete, 0x50, 0);  // SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_complete, did, 1);   // DID
    incoming_complete->payload_count = 2;

    openlcb_statemachine_info_t info_complete = _build_statemachine_info(node, incoming_complete, outgoing_complete);

    ProtocolStreamHandler_data_complete(&info_complete);

    EXPECT_EQ(_complete_called, 1);

    // Stream slot should now be free -- can open a new one
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 64, 0x51);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);

    EXPECT_EQ(_initiate_request_called, 1);  // Should succeed, not table-full

}

// ============================================================================
// TEST: Stream complete -- unknown stream ignored
// ============================================================================

TEST(ProtocolStreamHandler, data_complete_unknown_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_COMPLETE);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xDD, 0);  // unknown SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xCC, 1);  // unknown DID
    incoming->payload_count = 2;

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_data_complete(&info);

    EXPECT_EQ(_complete_called, 0);

}

// ============================================================================
// SECTION 4: ERROR CASES AND EDGE CONDITIONS
// ============================================================================

// ============================================================================
// TEST: All NULL callbacks -- initiate request rejected gracefully
// ============================================================================

TEST(ProtocolStreamHandler, null_callbacks_reject_gracefully) {

    _global_initialize(&_interface_null);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    _load_initiate_request(incoming, 128, 0x60);

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_request(&info);

    // Should reject with streams not supported
    EXPECT_TRUE(info.outgoing_msg_info.valid);
    uint16_t reply_flags = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 2);
    EXPECT_EQ(reply_flags, ERROR_PERMANENT_STREAMS_NOT_SUPPORTED);

}

// ============================================================================
// TEST: Data send with zero payload (flush)
// ============================================================================

TEST(ProtocolStreamHandler, data_send_zero_payload_flush) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x70);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    uint8_t did = _last_stream->dest_stream_id;

    // Send data with just DID, no payload bytes (flush per spec section 7.3)
    _reset_mock_counters();
    openlcb_msg_t *incoming_data = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_data = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_data, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_data);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, did, 0);  // DID only
    incoming_data->payload_count = 1;

    openlcb_statemachine_info_t info_data = _build_statemachine_info(node, incoming_data, outgoing_data);

    ProtocolStreamHandler_data_send(&info_data);

    EXPECT_EQ(_data_received_called, 1);
    EXPECT_EQ(_last_stream->bytes_transferred, (uint32_t)0);
    EXPECT_EQ(_last_stream->bytes_remaining, (uint16_t)128);

}

// ============================================================================
// TEST: Data proceed -- unknown stream ignored
// ============================================================================

TEST(ProtocolStreamHandler, data_proceed_unknown_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_PROCEED);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xBB, 0);  // unknown SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xAA, 1);  // unknown DID
    incoming->payload_count = 2;

    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_data_proceed(&info);

    EXPECT_EQ(_data_proceed_called, 0);

}

// ============================================================================
// TEST: DID counter wraps at STREAM_ID_RESERVED (0xFF)
// ============================================================================

TEST(ProtocolStreamHandler, dest_stream_id_wraps_at_reserved) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open and close 255 streams to push DID counter to 0xFE.
    // After that the next assign should wrap to 0.
    for (int i = 0; i < 255; i++) {

        _reset_mock_counters();

        openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
        openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(incoming_req, nullptr);
        ASSERT_NE(outgoing_req, nullptr);

        _load_initiate_request(incoming_req, 128, (uint8_t)i);
        openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
        ProtocolStreamHandler_initiate_request(&info_req);

        uint8_t did = _last_stream->dest_stream_id;

        // Complete to free the slot
        openlcb_msg_t *incoming_cmp = OpenLcbBufferStore_allocate_buffer(BASIC);
        openlcb_msg_t *outgoing_cmp = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(incoming_cmp, nullptr);
        ASSERT_NE(outgoing_cmp, nullptr);

        OpenLcbUtilities_load_openlcb_message(incoming_cmp, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_COMPLETE);
        OpenLcbUtilities_clear_openlcb_message_payload(incoming_cmp);
        OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_cmp, (uint8_t)i, 0);
        OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_cmp, did, 1);
        incoming_cmp->payload_count = 2;
        openlcb_statemachine_info_t info_cmp = _build_statemachine_info(node, incoming_cmp, outgoing_cmp);
        ProtocolStreamHandler_data_complete(&info_cmp);

        OpenLcbBufferStore_free_buffer(incoming_req);
        OpenLcbBufferStore_free_buffer(outgoing_req);
        OpenLcbBufferStore_free_buffer(incoming_cmp);
        OpenLcbBufferStore_free_buffer(outgoing_cmp);

    }

    // The 256th allocation should wrap the DID back to 0
    _reset_mock_counters();
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(incoming_req, nullptr);
    ASSERT_NE(outgoing_req, nullptr);

    _load_initiate_request(incoming_req, 128, 0xF0);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info);

    EXPECT_EQ(_initiate_request_called, 1);
    EXPECT_EQ(_last_stream->dest_stream_id, 0);

}

// ============================================================================
// TEST: Initiate reply -- alternate accept (flags 0x0000 with non-zero buffer)
// ============================================================================

TEST(ProtocolStreamHandler, initiate_reply_alternate_accept) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Create a stream (we are destination, SID=0x20)
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x20);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    // Send reply with flags=0x0000 and non-zero buffer (alternate accept)
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_reply, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_reply);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 64, 0);      // non-zero buffer
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 0x0000, 2);  // flags = 0x0000
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x20, 4);    // SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x30, 5);    // DID
    incoming_reply->payload_count = 6;

    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);
    ProtocolStreamHandler_initiate_reply(&info_reply);

    EXPECT_EQ(_initiate_reply_called, 1);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);
    EXPECT_EQ(_last_stream->max_buffer_size, 64);

}

// ============================================================================
// TEST: Initiate reply -- reject (error flags, zero buffer)
// ============================================================================

TEST(ProtocolStreamHandler, initiate_reply_reject) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Create a stream (we are destination, SID=0x20)
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x20);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    // Send reply with reject (permanent error, zero buffer)
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_reply, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_reply);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 0, 0);           // zero buffer = reject
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 0x1010, 2);      // permanent error
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x20, 4);        // SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x00, 5);        // DID
    incoming_reply->payload_count = 6;

    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);
    ProtocolStreamHandler_initiate_reply(&info_reply);

    EXPECT_EQ(_initiate_reply_called, 1);

    // Stream slot should be freed -- can open a new stream
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 128, 0x21);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);
    EXPECT_EQ(_initiate_request_called, 1);

}

// ============================================================================
// TEST: Initiate reply with NULL callback -- still processes state
// ============================================================================

TEST(ProtocolStreamHandler, initiate_reply_null_callback) {

    _global_initialize(&_interface_null);

    // Re-init with an interface that has on_initiate_request set but
    // on_initiate_reply is NULL. We need a custom interface for this.
    static const interface_protocol_stream_handler_t iface_no_reply = {

        .on_initiate_request = &_mock_on_initiate_request,
        .on_initiate_reply   = NULL,
        .on_data_received    = &_mock_on_data_received,
        .on_data_proceed     = &_mock_on_data_proceed,
        .on_complete         = &_mock_on_complete,

    };

    _global_initialize(&iface_no_reply);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Create a stream
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x20);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    // Send accept reply -- on_initiate_reply is NULL, should not crash
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_reply, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_reply);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 128, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, STREAM_REPLY_ACCEPT, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x20, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x30, 5);
    incoming_reply->payload_count = 6;

    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);
    ProtocolStreamHandler_initiate_reply(&info_reply);

    // Should not crash, callback not called
    EXPECT_EQ(_initiate_reply_called, 0);

}

// ============================================================================
// TEST: Data send on non-OPEN stream -- ignored
// ============================================================================

TEST(ProtocolStreamHandler, data_send_non_open_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Create stream then manipulate state to INITIATED (non-OPEN)
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x30);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    uint8_t did = _last_stream->dest_stream_id;

    // Force state back to INITIATED to test the guard
    _last_stream->state = STREAM_STATE_INITIATED;

    _reset_mock_counters();
    openlcb_msg_t *incoming_data = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_data = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_data, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_data);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, did, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, 0x42, 1);
    incoming_data->payload_count = 2;

    openlcb_statemachine_info_t info_data = _build_statemachine_info(node, incoming_data, outgoing_data);
    ProtocolStreamHandler_data_send(&info_data);

    EXPECT_EQ(_data_received_called, 0);

}

// ============================================================================
// TEST: Data send -- bytes_remaining underflow protection
// ============================================================================

TEST(ProtocolStreamHandler, data_send_bytes_remaining_underflow) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream with small buffer
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 4, 0x30);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    uint8_t did = _last_stream->dest_stream_id;
    EXPECT_EQ(_last_stream->bytes_remaining, 4);

    // Send more data bytes than bytes_remaining
    _reset_mock_counters();
    openlcb_msg_t *incoming_data = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_data = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_data, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_data);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, did, 0);
    for (int i = 0; i < 7; i++) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, 0x10 + i, 1 + i);

    }
    incoming_data->payload_count = 8;  // 1 DID + 7 data (7 > 4 remaining)

    openlcb_statemachine_info_t info_data = _build_statemachine_info(node, incoming_data, outgoing_data);
    ProtocolStreamHandler_data_send(&info_data);

    EXPECT_EQ(_data_received_called, 1);
    // bytes_remaining clamps to 0 on underflow, then auto-proceed resets it to max_buffer_size
    EXPECT_EQ(_last_stream->bytes_remaining, 4);
    EXPECT_EQ(_last_stream->bytes_transferred, (uint32_t)7);

}

// ============================================================================
// TEST: Data send with NULL on_data_received callback
// ============================================================================

TEST(ProtocolStreamHandler, data_send_null_callback) {

    static const interface_protocol_stream_handler_t iface_no_data = {

        .on_initiate_request = &_mock_on_initiate_request,
        .on_initiate_reply   = &_mock_on_initiate_reply,
        .on_data_received    = NULL,
        .on_data_proceed     = &_mock_on_data_proceed,
        .on_complete         = &_mock_on_complete,

    };

    _global_initialize(&iface_no_data);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x30);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    uint8_t did = _last_stream->dest_stream_id;
    stream_state_t *stream = _last_stream;

    _reset_mock_counters();
    openlcb_msg_t *incoming_data = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_data = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_data, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_data);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, did, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, 0x42, 1);
    incoming_data->payload_count = 2;

    openlcb_statemachine_info_t info_data = _build_statemachine_info(node, incoming_data, outgoing_data);
    ProtocolStreamHandler_data_send(&info_data);

    // Should not crash, bytes still tracked
    EXPECT_EQ(_data_received_called, 0);
    EXPECT_EQ(stream->bytes_transferred, (uint32_t)1);

}

// ============================================================================
// TEST: Data proceed on non-OPEN stream -- ignored
// ============================================================================

TEST(ProtocolStreamHandler, data_proceed_non_open_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x40);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    stream_state_t *stream = _last_stream;
    uint16_t initial_remaining = stream->bytes_remaining;

    // Force state to INITIATED
    stream->state = STREAM_STATE_INITIATED;

    _reset_mock_counters();
    openlcb_msg_t *incoming_proceed = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_proceed = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_proceed, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_PROCEED);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_proceed);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_proceed, 0x40, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_proceed, stream->dest_stream_id, 1);
    incoming_proceed->payload_count = 2;

    openlcb_statemachine_info_t info_proceed = _build_statemachine_info(node, incoming_proceed, outgoing_proceed);
    ProtocolStreamHandler_data_proceed(&info_proceed);

    EXPECT_EQ(_data_proceed_called, 0);
    EXPECT_EQ(stream->bytes_remaining, initial_remaining);

}

// ============================================================================
// TEST: Data proceed with NULL callback
// ============================================================================

TEST(ProtocolStreamHandler, data_proceed_null_callback) {

    static const interface_protocol_stream_handler_t iface_no_proceed = {

        .on_initiate_request = &_mock_on_initiate_request,
        .on_initiate_reply   = &_mock_on_initiate_reply,
        .on_data_received    = &_mock_on_data_received,
        .on_data_proceed     = NULL,
        .on_complete         = &_mock_on_complete,

    };

    _global_initialize(&iface_no_proceed);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x40);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    stream_state_t *stream = _last_stream;
    uint16_t initial_remaining = stream->bytes_remaining;

    _reset_mock_counters();
    openlcb_msg_t *incoming_proceed = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_proceed = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_proceed, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_PROCEED);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_proceed);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_proceed, 0x40, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_proceed, stream->dest_stream_id, 1);
    incoming_proceed->payload_count = 2;

    openlcb_statemachine_info_t info_proceed = _build_statemachine_info(node, incoming_proceed, outgoing_proceed);
    ProtocolStreamHandler_data_proceed(&info_proceed);

    // Should not crash, window still extended
    EXPECT_EQ(_data_proceed_called, 0);
    EXPECT_EQ(stream->bytes_remaining, initial_remaining + 128);

}

// ============================================================================
// TEST: Data complete with NULL on_complete callback
// ============================================================================

TEST(ProtocolStreamHandler, data_complete_null_callback) {

    static const interface_protocol_stream_handler_t iface_no_complete = {

        .on_initiate_request = &_mock_on_initiate_request,
        .on_initiate_reply   = &_mock_on_initiate_reply,
        .on_data_received    = &_mock_on_data_received,
        .on_data_proceed     = &_mock_on_data_proceed,
        .on_complete         = NULL,

    };

    _global_initialize(&iface_no_complete);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x50);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    uint8_t did = _last_stream->dest_stream_id;

    _reset_mock_counters();
    openlcb_msg_t *incoming_cmp = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_cmp = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_cmp, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_COMPLETE);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_cmp);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_cmp, 0x50, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_cmp, did, 1);
    incoming_cmp->payload_count = 2;

    openlcb_statemachine_info_t info_cmp = _build_statemachine_info(node, incoming_cmp, outgoing_cmp);
    ProtocolStreamHandler_data_complete(&info_cmp);

    // Should not crash, stream still freed
    EXPECT_EQ(_complete_called, 0);

    // Slot should be free -- can open a new stream
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 128, 0x51);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);
    EXPECT_EQ(_initiate_request_called, 1);

}

// ============================================================================
// TEST: _find_stream -- matching remote_node_id but wrong stream_id
// ============================================================================

TEST(ProtocolStreamHandler, find_stream_wrong_stream_id) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream (SID=0x20, from SOURCE_ID)
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x20);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    stream_state_t *stream = _last_stream;
    EXPECT_EQ(stream->state, STREAM_STATE_OPEN);
    uint8_t saved_did = stream->dest_stream_id;

    // Send data with correct remote but wrong DID -- should be ignored
    // This exercises the branch where remote_node_id matches but dest_stream_id does not
    _reset_mock_counters();
    openlcb_msg_t *incoming_data = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_data = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_data, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_data);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, 0xEE, 0);  // wrong DID, correct remote
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, 0x42, 1);
    incoming_data->payload_count = 2;

    openlcb_statemachine_info_t info_data = _build_statemachine_info(node, incoming_data, outgoing_data);
    ProtocolStreamHandler_data_send(&info_data);

    EXPECT_EQ(_data_received_called, 0);

    // Exercise the remote_node_id mismatch branch: data from a different node
    openlcb_msg_t *incoming_other = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_other = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_other, 0x333, 0xAABBCCDDEEFF, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_other);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_other, saved_did, 0);  // correct DID but wrong remote
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_other, 0x42, 1);
    incoming_other->payload_count = 2;

    openlcb_statemachine_info_t info_other = _build_statemachine_info(node, incoming_other, outgoing_other);
    ProtocolStreamHandler_data_send(&info_other);

    EXPECT_EQ(_data_received_called, 0);

    // Also exercise the source_stream_id mismatch path via proceed
    // Proceed searches by SID. Send proceed with correct remote but wrong SID
    openlcb_msg_t *incoming_proceed = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_proceed = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_proceed, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_PROCEED);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_proceed);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_proceed, 0xEE, 0);  // wrong SID, correct remote
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_proceed, 0x00, 1);
    incoming_proceed->payload_count = 2;

    openlcb_statemachine_info_t info_proceed = _build_statemachine_info(node, incoming_proceed, outgoing_proceed);
    ProtocolStreamHandler_data_proceed(&info_proceed);

    EXPECT_EQ(_data_proceed_called, 0);

}

// ============================================================================
// TEST: Full lifecycle -- open, send data, complete
// ============================================================================

TEST(ProtocolStreamHandler, full_lifecycle) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // 1. Open stream
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 64, 0x80);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_initiate_request_called, 1);
    stream_state_t *stream = _last_stream;
    EXPECT_EQ(stream->state, STREAM_STATE_OPEN);
    uint8_t did = stream->dest_stream_id;

    // 2. Send 10 bytes of data
    _reset_mock_counters();
    openlcb_msg_t *incoming_data = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_data = OpenLcbBufferStore_allocate_buffer(BASIC);
    OpenLcbUtilities_load_openlcb_message(incoming_data, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_data);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, did, 0);
    for (int i = 0; i < 10; i++) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_data, (uint8_t)i, 1 + i);

    }
    incoming_data->payload_count = 11;
    openlcb_statemachine_info_t info_data = _build_statemachine_info(node, incoming_data, outgoing_data);
    ProtocolStreamHandler_data_send(&info_data);

    EXPECT_EQ(_data_received_called, 1);
    EXPECT_EQ(stream->bytes_transferred, (uint32_t)10);
    EXPECT_EQ(stream->bytes_remaining, (uint16_t)(64 - 10));

    // 3. Complete
    _reset_mock_counters();
    openlcb_msg_t *incoming_complete = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_complete = OpenLcbBufferStore_allocate_buffer(BASIC);
    OpenLcbUtilities_load_openlcb_message(incoming_complete, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_COMPLETE);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_complete);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_complete, 0x80, 0);  // SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_complete, did, 1);   // DID
    incoming_complete->payload_count = 2;
    openlcb_statemachine_info_t info_complete = _build_statemachine_info(node, incoming_complete, outgoing_complete);
    ProtocolStreamHandler_data_complete(&info_complete);

    EXPECT_EQ(_complete_called, 1);

    // 4. Verify slot is free -- can open another stream
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 64, 0x81);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);
    EXPECT_EQ(_initiate_request_called, 1);

}

// ============================================================================
// SECTION 5: FINDING #2 -- ACCEPT LOGIC EDGE CASES
// ============================================================================

// ============================================================================
// TEST: Initiate reply -- 0x8000 flag with zero buffer is rejected
// ============================================================================

TEST(ProtocolStreamHandler, initiate_reply_0x8000_zero_buffer_rejected) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Create a stream (we are destination, SID=0x20)
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x20);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    // Send reply with 0x8000 but ZERO buffer -- malformed accept per spec 7.1
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_reply, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_reply);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 0, 0);           // zero buffer
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, STREAM_REPLY_ACCEPT, 2);  // 0x8000
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x20, 4);        // SID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x30, 5);        // DID
    incoming_reply->payload_count = 6;

    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);
    ProtocolStreamHandler_initiate_reply(&info_reply);

    EXPECT_EQ(_initiate_reply_called, 1);

    // Stream slot should be freed -- zero buffer means reject regardless of flags
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 128, 0x21);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);
    EXPECT_EQ(_initiate_request_called, 1);

}

// ============================================================================
// TEST: Initiate reply -- 0x0000 flag with zero buffer is rejected
// ============================================================================

TEST(ProtocolStreamHandler, initiate_reply_0x0000_zero_buffer_rejected) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Create a stream
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x20);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);

    // Send reply with 0x0000 and zero buffer
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_reply, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_reply);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 0, 0);       // zero buffer
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 0x0000, 2);  // flags
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x20, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x30, 5);
    incoming_reply->payload_count = 6;

    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);
    ProtocolStreamHandler_initiate_reply(&info_reply);

    EXPECT_EQ(_initiate_reply_called, 1);

    // Stream slot freed
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 128, 0x21);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);
    EXPECT_EQ(_initiate_request_called, 1);

}

// ============================================================================
// TEST: Initiate reply -- 0x0000 flag with non-zero buffer is accepted (regression)
// ============================================================================

TEST(ProtocolStreamHandler, initiate_reply_0x0000_nonzero_buffer_accepted) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Create a stream
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0x20);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);

    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(incoming_reply, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_reply);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 64, 0);      // non-zero buffer
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_reply, 0x0000, 2);  // flags = 0x0000
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x20, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_reply, 0x30, 5);
    incoming_reply->payload_count = 6;

    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);
    ProtocolStreamHandler_initiate_reply(&info_reply);

    EXPECT_EQ(_initiate_reply_called, 1);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);
    EXPECT_EQ(_last_stream->max_buffer_size, 64);

}

// ============================================================================
// SECTION 6: FINDING #1 -- TERMINATE DUE TO ERROR STREAM CLOSURE
// ============================================================================

// Helper: build a TDE message with error_code and rejected_mti
static void _load_tde(openlcb_msg_t *msg, uint16_t source_alias, node_id_t source_id,
                      uint16_t dest_alias, node_id_t dest_id,
                      uint16_t error_code, uint16_t rejected_mti) {

    OpenLcbUtilities_load_openlcb_message(msg, source_alias, source_id, dest_alias, dest_id, MTI_TERMINATE_DUE_TO_ERROR);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, error_code, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, rejected_mti, 2);
    msg->payload_count = 4;

}

// ============================================================================
// TEST: TDE with MTI_STREAM_SEND closes stream
// ============================================================================

TEST(ProtocolStreamHandler, tde_stream_send_mti_closes_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xA0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    // Send TDE with rejected_mti = MTI_STREAM_SEND
    _reset_mock_counters();
    openlcb_msg_t *incoming_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_tde(incoming_tde, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, ERROR_TEMPORARY, MTI_STREAM_SEND);
    openlcb_statemachine_info_t info_tde = _build_statemachine_info(node, incoming_tde, outgoing_tde);

    ProtocolStreamHandler_handle_terminate_due_to_error(&info_tde);

    EXPECT_EQ(_complete_called, 1);

    // Stream slot freed -- can open a new one
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 128, 0xA1);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);
    EXPECT_EQ(_initiate_request_called, 1);

}

// ============================================================================
// TEST: TDE with MTI_STREAM_PROCEED closes stream
// ============================================================================

TEST(ProtocolStreamHandler, tde_stream_proceed_mti_closes_stream) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xA0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    _reset_mock_counters();
    openlcb_msg_t *incoming_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_tde(incoming_tde, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, ERROR_TEMPORARY, MTI_STREAM_PROCEED);
    openlcb_statemachine_info_t info_tde = _build_statemachine_info(node, incoming_tde, outgoing_tde);

    ProtocolStreamHandler_handle_terminate_due_to_error(&info_tde);

    EXPECT_EQ(_complete_called, 1);

    // Slot freed
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 128, 0xA1);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);
    EXPECT_EQ(_initiate_request_called, 1);

}

// ============================================================================
// TEST: TDE with unrelated MTI -- stream NOT closed
// ============================================================================

TEST(ProtocolStreamHandler, tde_unrelated_mti_ignored) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xA0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    _reset_mock_counters();
    openlcb_msg_t *incoming_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_tde(incoming_tde, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, ERROR_TEMPORARY, MTI_SIMPLE_NODE_INFO_REQUEST);
    openlcb_statemachine_info_t info_tde = _build_statemachine_info(node, incoming_tde, outgoing_tde);

    ProtocolStreamHandler_handle_terminate_due_to_error(&info_tde);

    EXPECT_EQ(_complete_called, 0);

    // Stream still open -- second request rejected (table full)
    _reset_mock_counters();
    openlcb_msg_t *incoming_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req2, 128, 0xA1);
    openlcb_statemachine_info_t info_req2 = _build_statemachine_info(node, incoming_req2, outgoing_req2);
    ProtocolStreamHandler_initiate_request(&info_req2);
    EXPECT_EQ(_initiate_request_called, 0);  // rejected, table full

}

// ============================================================================
// TEST: TDE with no open streams -- no crash, no callback
// ============================================================================

TEST(ProtocolStreamHandler, tde_no_stream_ignored) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_tde(incoming_tde, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, ERROR_TEMPORARY, MTI_STREAM_SEND);
    openlcb_statemachine_info_t info_tde = _build_statemachine_info(node, incoming_tde, outgoing_tde);

    ProtocolStreamHandler_handle_terminate_due_to_error(&info_tde);

    EXPECT_EQ(_complete_called, 0);

}

// ============================================================================
// TEST: TDE payload too short (< 4 bytes) -- ignored
// ============================================================================

TEST(ProtocolStreamHandler, tde_payload_too_short_ignored) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xA0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);

    // Send TDE with only 2 bytes (error code, no rejected_mti)
    _reset_mock_counters();
    openlcb_msg_t *incoming_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    OpenLcbUtilities_load_openlcb_message(incoming_tde, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DUE_TO_ERROR);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_tde);
    OpenLcbUtilities_copy_word_to_openlcb_payload(incoming_tde, ERROR_TEMPORARY, 0);
    incoming_tde->payload_count = 2;

    openlcb_statemachine_info_t info_tde = _build_statemachine_info(node, incoming_tde, outgoing_tde);

    ProtocolStreamHandler_handle_terminate_due_to_error(&info_tde);

    EXPECT_EQ(_complete_called, 0);

}

// ============================================================================
// TEST: TDE from wrong node -- stream NOT closed
// ============================================================================

TEST(ProtocolStreamHandler, tde_wrong_node_ignored) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open a stream with SOURCE_ID
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xA0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

    // Send TDE from a DIFFERENT node
    _reset_mock_counters();
    openlcb_msg_t *incoming_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_tde = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_tde(incoming_tde, 0x333, 0xAABBCCDDEEFF, DEST_ALIAS, DEST_ID, ERROR_TEMPORARY, MTI_STREAM_SEND);
    openlcb_statemachine_info_t info_tde = _build_statemachine_info(node, incoming_tde, outgoing_tde);

    ProtocolStreamHandler_handle_terminate_due_to_error(&info_tde);

    EXPECT_EQ(_complete_called, 0);

}

// ============================================================================
// SECTION 7: OUTBOUND STREAM INITIATION
// ============================================================================

// Helper: build an initiate reply from SOURCE back to DEST
static void _load_initiate_reply_msg(openlcb_msg_t *msg, uint16_t buffer_size,
                                     uint16_t flags, uint8_t sid, uint8_t did) {

    OpenLcbUtilities_load_openlcb_message(msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_INIT_REPLY);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, buffer_size, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, flags, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, sid, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, did, 5);
    msg->payload_count = 6;

}

// Helper: open an outbound stream and accept it, returns the stream pointer
static stream_state_t *_open_outbound_stream(openlcb_node_t *node,
                                             uint16_t buffer_size,
                                             openlcb_msg_t *outgoing) {

    openlcb_msg_t *dummy_incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_init = _build_statemachine_info(node, dummy_incoming, outgoing);

    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info_init, SOURCE_ALIAS, SOURCE_ID, buffer_size, 0xFF, NULL);

    if (!stream) { return NULL; }

    uint8_t sid = stream->source_stream_id;

    // Feed accept reply
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_reply_msg(incoming_reply, buffer_size, STREAM_REPLY_ACCEPT, sid, 0x50);
    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);
    ProtocolStreamHandler_initiate_reply(&info_reply);

    return stream;

}

// ============================================================================
// TEST: initiate_outbound allocates as source
// ============================================================================

TEST(ProtocolStreamHandler, initiate_outbound_allocates_as_source) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 256, 0xFF, NULL);

    EXPECT_NE(stream, nullptr);
    EXPECT_EQ(stream->is_source, true);
    EXPECT_EQ(stream->state, STREAM_STATE_INITIATED);
    EXPECT_EQ(stream->remote_node_id, SOURCE_ID);
    EXPECT_EQ(stream->remote_alias, SOURCE_ALIAS);
    EXPECT_EQ(stream->max_buffer_size, 256);
    EXPECT_EQ(stream->bytes_remaining, 0);
    EXPECT_EQ(stream->bytes_transferred, 0);
    EXPECT_EQ(stream->dest_stream_id, 0);

}

// ============================================================================
// TEST: initiate_outbound sends Stream Initiate Request
// ============================================================================

TEST(ProtocolStreamHandler, initiate_outbound_sends_initiate_request) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 256, 0xFF, NULL);

    EXPECT_EQ(info.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing->mti, MTI_STREAM_INIT_REQUEST);
    EXPECT_EQ(outgoing->dest_id, SOURCE_ID);
    EXPECT_EQ(outgoing->dest_alias, SOURCE_ALIAS);

    uint16_t proposed = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 0);
    EXPECT_EQ(proposed, 256);

    uint8_t sid = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4);
    EXPECT_EQ(sid, stream->source_stream_id);

    uint8_t suggested_did = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 5);
    EXPECT_EQ(suggested_did, 0xFF);

}

// ============================================================================
// TEST: initiate_outbound table full returns NULL
// ============================================================================

TEST(ProtocolStreamHandler, initiate_outbound_table_full_returns_null) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Fill the table (USER_DEFINED_MAX_STREAM_COUNT = 1)
    openlcb_msg_t *incoming1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info1 = _build_statemachine_info(node, incoming1, outgoing1);
    stream_state_t *s1 = ProtocolStreamHandler_initiate_outbound(
            &info1, SOURCE_ALIAS, SOURCE_ID, 128, 0xFF, NULL);
    EXPECT_NE(s1, nullptr);

    // Second allocation should fail
    openlcb_msg_t *incoming2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info2 = _build_statemachine_info(node, incoming2, outgoing2);
    stream_state_t *s2 = ProtocolStreamHandler_initiate_outbound(
            &info2, SOURCE_ALIAS, SOURCE_ID, 128, 0xFF, NULL);
    EXPECT_EQ(s2, nullptr);

}

// ============================================================================
// TEST: initiate_outbound then accept reply transitions to OPEN
// ============================================================================

TEST(ProtocolStreamHandler, initiate_outbound_then_reply_accept) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 256, 0xFF, NULL);
    uint8_t sid = stream->source_stream_id;

    // Feed accept reply with negotiated buffer=128, DID=0x50
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_reply_msg(incoming_reply, 128, STREAM_REPLY_ACCEPT, sid, 0x50);
    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);

    ProtocolStreamHandler_initiate_reply(&info_reply);

    EXPECT_EQ(_initiate_reply_called, 1);
    EXPECT_EQ(stream->state, STREAM_STATE_OPEN);
    EXPECT_EQ(stream->dest_stream_id, 0x50);
    EXPECT_EQ(stream->max_buffer_size, 128);
    EXPECT_EQ(stream->bytes_remaining, 128);

}

// ============================================================================
// TEST: initiate_outbound then reject reply frees slot
// ============================================================================

TEST(ProtocolStreamHandler, initiate_outbound_then_reply_reject) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 256, 0xFF, NULL);
    uint8_t sid = stream->source_stream_id;

    // Feed reject reply (buffer=0, error code in flags)
    _reset_mock_counters();
    openlcb_msg_t *incoming_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_reply = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_reply_msg(incoming_reply, 0, 0x1000, sid, 0x00);
    openlcb_statemachine_info_t info_reply = _build_statemachine_info(node, incoming_reply, outgoing_reply);

    ProtocolStreamHandler_initiate_reply(&info_reply);

    EXPECT_EQ(_initiate_reply_called, 1);
    EXPECT_EQ(stream->state, STREAM_STATE_CLOSED);

}

// ============================================================================
// SECTION 8: SEND DATA
// ============================================================================

// ============================================================================
// TEST: send_data basic
// ============================================================================

TEST(ProtocolStreamHandler, send_data_basic) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_init = OpenLcbBufferStore_allocate_buffer(BASIC);
    stream_state_t *stream = _open_outbound_stream(node, 128, outgoing_init);
    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->state, STREAM_STATE_OPEN);

    // Send 4 bytes of data
    openlcb_msg_t *incoming_send = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_send = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_send = _build_statemachine_info(node, incoming_send, outgoing_send);

    uint8_t data[] = { 0xAA, 0xBB, 0xCC, 0xDD };
    bool ok = ProtocolStreamHandler_send_data(&info_send, stream, data, 4);

    EXPECT_EQ(ok, true);
    EXPECT_EQ(info_send.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing_send->mti, MTI_STREAM_SEND);
    EXPECT_EQ(outgoing_send->payload_count, 5);

    // Byte 0 = DID
    uint8_t did = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_send, 0);
    EXPECT_EQ(did, stream->dest_stream_id);

    // Bytes 1-4 = data
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_send, 1), 0xAA);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_send, 2), 0xBB);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_send, 3), 0xCC);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_send, 4), 0xDD);

    EXPECT_EQ(stream->bytes_transferred, 4);
    EXPECT_EQ(stream->bytes_remaining, 124);

}

// ============================================================================
// TEST: send_data respects flow control window
// ============================================================================

TEST(ProtocolStreamHandler, send_data_respects_window) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_init = OpenLcbBufferStore_allocate_buffer(BASIC);
    stream_state_t *stream = _open_outbound_stream(node, 8, outgoing_init);
    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->bytes_remaining, 8);

    // Send exactly 8 bytes to exhaust window
    openlcb_msg_t *incoming_s = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_s = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_s = _build_statemachine_info(node, incoming_s, outgoing_s);

    uint8_t data[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    bool ok = ProtocolStreamHandler_send_data(&info_s, stream, data, 8);
    EXPECT_EQ(ok, true);
    EXPECT_EQ(stream->bytes_remaining, 0);

    // Next send should fail -- window exhausted
    openlcb_msg_t *incoming_s2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_s2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_s2 = _build_statemachine_info(node, incoming_s2, outgoing_s2);

    uint8_t more[] = { 9 };
    bool ok2 = ProtocolStreamHandler_send_data(&info_s2, stream, more, 1);
    EXPECT_EQ(ok2, false);

}

// ============================================================================
// TEST: send_data wrong role rejected
// ============================================================================

TEST(ProtocolStreamHandler, send_data_wrong_role_rejected) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open as destination (incoming initiate request)
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xA0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);
    EXPECT_EQ(_last_stream->is_source, false);

    // Try to send data -- should fail (we are destination)
    openlcb_msg_t *incoming_s = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_s = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_s = _build_statemachine_info(node, incoming_s, outgoing_s);

    uint8_t data[] = { 0x01 };
    bool ok = ProtocolStreamHandler_send_data(&info_s, _last_stream, data, 1);
    EXPECT_EQ(ok, false);

}

// ============================================================================
// TEST: send_data not open rejected
// ============================================================================

TEST(ProtocolStreamHandler, send_data_not_open_rejected) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    // Initiate outbound -- state is INITIATED, not OPEN
    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 128, 0xFF, NULL);
    EXPECT_EQ(stream->state, STREAM_STATE_INITIATED);

    openlcb_msg_t *incoming_s = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_s = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_s = _build_statemachine_info(node, incoming_s, outgoing_s);

    uint8_t data[] = { 0x01 };
    bool ok = ProtocolStreamHandler_send_data(&info_s, stream, data, 1);
    EXPECT_EQ(ok, false);

}

// ============================================================================
// SECTION 9: SEND COMPLETE
// ============================================================================

// ============================================================================
// TEST: send_complete as source
// ============================================================================

TEST(ProtocolStreamHandler, send_complete_source) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_init = OpenLcbBufferStore_allocate_buffer(BASIC);
    stream_state_t *stream = _open_outbound_stream(node, 128, outgoing_init);
    ASSERT_NE(stream, nullptr);

    uint8_t sid = stream->source_stream_id;
    uint8_t did = stream->dest_stream_id;

    openlcb_msg_t *incoming_c = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_c = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_c = _build_statemachine_info(node, incoming_c, outgoing_c);

    ProtocolStreamHandler_send_complete(&info_c, stream);

    EXPECT_EQ(info_c.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing_c->mti, MTI_STREAM_COMPLETE);
    EXPECT_EQ(outgoing_c->dest_id, SOURCE_ID);
    EXPECT_EQ(outgoing_c->dest_alias, SOURCE_ALIAS);
    EXPECT_EQ(outgoing_c->payload_count, 8);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_c, 0), sid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_c, 1), did);
    // Bytes 2-3: flags (0x0000)
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_c, 2), 0x0000);
    EXPECT_EQ(stream->state, STREAM_STATE_CLOSED);

}

// ============================================================================
// TEST: send_complete as destination
// ============================================================================

TEST(ProtocolStreamHandler, send_complete_destination) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open as destination
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xB0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);

    stream_state_t *stream = _last_stream;
    uint8_t sid = stream->source_stream_id;
    uint8_t did = stream->dest_stream_id;

    openlcb_msg_t *incoming_c = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_c = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_c = _build_statemachine_info(node, incoming_c, outgoing_c);

    ProtocolStreamHandler_send_complete(&info_c, stream);

    EXPECT_EQ(info_c.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing_c->mti, MTI_STREAM_COMPLETE);
    EXPECT_EQ(outgoing_c->payload_count, 8);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_c, 0), sid);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_c, 1), did);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_c, 2), 0x0000);
    EXPECT_EQ(stream->state, STREAM_STATE_CLOSED);

}

// ============================================================================
// SECTION 10: SEND TERMINATE
// ============================================================================

// ============================================================================
// TEST: send_terminate as destination
// ============================================================================

TEST(ProtocolStreamHandler, send_terminate_as_destination) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open as destination
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xC0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);

    stream_state_t *stream = _last_stream;

    openlcb_msg_t *incoming_t = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_t = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_t = _build_statemachine_info(node, incoming_t, outgoing_t);

    ProtocolStreamHandler_send_terminate(&info_t, stream, 0x1000);

    EXPECT_EQ(info_t.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing_t->mti, MTI_TERMINATE_DUE_TO_ERROR);
    EXPECT_EQ(outgoing_t->dest_id, SOURCE_ID);
    EXPECT_EQ(outgoing_t->payload_count, 4);

    uint16_t error_code = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_t, 0);
    EXPECT_EQ(error_code, 0x1000);

    uint16_t rejected_mti = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_t, 2);
    EXPECT_EQ(rejected_mti, MTI_STREAM_SEND);

    EXPECT_EQ(stream->state, STREAM_STATE_CLOSED);

}

// ============================================================================
// TEST: send_terminate as source
// ============================================================================

TEST(ProtocolStreamHandler, send_terminate_as_source) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_init = OpenLcbBufferStore_allocate_buffer(BASIC);
    stream_state_t *stream = _open_outbound_stream(node, 128, outgoing_init);
    ASSERT_NE(stream, nullptr);

    openlcb_msg_t *incoming_t = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_t = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_t = _build_statemachine_info(node, incoming_t, outgoing_t);

    ProtocolStreamHandler_send_terminate(&info_t, stream, 0x2000);

    EXPECT_EQ(info_t.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing_t->mti, MTI_TERMINATE_DUE_TO_ERROR);
    EXPECT_EQ(outgoing_t->payload_count, 4);

    uint16_t error_code = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_t, 0);
    EXPECT_EQ(error_code, 0x2000);

    uint16_t rejected_mti = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_t, 2);
    EXPECT_EQ(rejected_mti, MTI_STREAM_PROCEED);

    EXPECT_EQ(stream->state, STREAM_STATE_CLOSED);

}

// ============================================================================
// SECTION 11: FULL OUTBOUND LIFECYCLE
// ============================================================================

TEST(ProtocolStreamHandler, outbound_full_lifecycle) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open outbound stream with buffer=16
    openlcb_msg_t *outgoing_init = OpenLcbBufferStore_allocate_buffer(BASIC);
    stream_state_t *stream = _open_outbound_stream(node, 16, outgoing_init);
    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->state, STREAM_STATE_OPEN);
    EXPECT_EQ(stream->bytes_remaining, 16);

    // Send 16 bytes (full window)
    openlcb_msg_t *incoming_s1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_s1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_s1 = _build_statemachine_info(node, incoming_s1, outgoing_s1);

    uint8_t data1[16];
    for (int i = 0; i < 16; i++) { data1[i] = (uint8_t)i; }

    bool ok1 = ProtocolStreamHandler_send_data(&info_s1, stream, data1, 16);
    EXPECT_EQ(ok1, true);
    EXPECT_EQ(stream->bytes_remaining, 0);
    EXPECT_EQ(stream->bytes_transferred, 16);

    // Feed Data Proceed to extend window
    _reset_mock_counters();
    openlcb_msg_t *incoming_p = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_p = OpenLcbBufferStore_allocate_buffer(BASIC);
    OpenLcbUtilities_load_openlcb_message(incoming_p, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_PROCEED);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming_p);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_p, stream->source_stream_id, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming_p, stream->dest_stream_id, 1);
    incoming_p->payload_count = 2;
    openlcb_statemachine_info_t info_p = _build_statemachine_info(node, incoming_p, outgoing_p);

    ProtocolStreamHandler_data_proceed(&info_p);

    EXPECT_EQ(_data_proceed_called, 1);
    EXPECT_EQ(stream->bytes_remaining, 16);

    // Send 8 more bytes
    openlcb_msg_t *incoming_s2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_s2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_s2 = _build_statemachine_info(node, incoming_s2, outgoing_s2);

    uint8_t data2[8] = { 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7 };
    bool ok2 = ProtocolStreamHandler_send_data(&info_s2, stream, data2, 8);
    EXPECT_EQ(ok2, true);
    EXPECT_EQ(stream->bytes_transferred, 24);
    EXPECT_EQ(stream->bytes_remaining, 8);

    // Send complete
    openlcb_msg_t *incoming_c = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_c = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_c = _build_statemachine_info(node, incoming_c, outgoing_c);

    ProtocolStreamHandler_send_complete(&info_c, stream);

    EXPECT_EQ(outgoing_c->mti, MTI_STREAM_COMPLETE);
    EXPECT_EQ(stream->state, STREAM_STATE_CLOSED);

}

// ============================================================================
// SECTION 12: REMOTE ALIAS STORAGE
// ============================================================================

// ============================================================================
// TEST: remote_alias stored on inbound (destination role)
// ============================================================================

TEST(ProtocolStreamHandler, remote_alias_stored_on_inbound) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming, 128, 0xD0);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_request(&info);

    EXPECT_EQ(_last_stream->remote_alias, SOURCE_ALIAS);
    EXPECT_EQ(_last_stream->remote_node_id, SOURCE_ID);

}

// ============================================================================
// TEST: remote_alias stored on outbound (source role)
// ============================================================================

TEST(ProtocolStreamHandler, remote_alias_stored_on_outbound) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 128, 0xFF, NULL);

    EXPECT_EQ(stream->remote_alias, SOURCE_ALIAS);
    EXPECT_EQ(stream->remote_node_id, SOURCE_ID);

}

// ============================================================================
// SECTION 13: APPLICATION VETO ON INITIATE REQUEST
// ============================================================================

// ============================================================================
// TEST: callback returns false rejects the stream
// ============================================================================

TEST(ProtocolStreamHandler, initiate_request_veto_rejects) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    _mock_accept_initiate = false;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming, 256, 0xE0);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_request(&info);

    EXPECT_EQ(_initiate_request_called, 1);
    EXPECT_EQ(info.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing->mti, MTI_STREAM_INIT_REPLY);

    uint16_t buf_size = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 0);
    EXPECT_EQ(buf_size, 0);

    uint16_t reply_flags = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 2);
    EXPECT_EQ(reply_flags, ERROR_PERMANENT_STREAMS_NOT_SUPPORTED);

    uint8_t reply_sid = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 4);
    EXPECT_EQ(reply_sid, 0xE0);

}

// ============================================================================
// TEST: callback returns true still accepts the stream
// ============================================================================

TEST(ProtocolStreamHandler, initiate_request_accept_via_callback) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    _mock_accept_initiate = true;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming, 256, 0xE1);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    ProtocolStreamHandler_initiate_request(&info);

    EXPECT_EQ(_initiate_request_called, 1);

    uint16_t reply_flags = OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing, 2);
    EXPECT_EQ(reply_flags, STREAM_REPLY_ACCEPT);

    EXPECT_NE(_last_stream, nullptr);
    EXPECT_EQ(_last_stream->state, STREAM_STATE_OPEN);

}

// ============================================================================
// SECTION 14: CONTENT UID IN OUTBOUND INITIATE
// ============================================================================

// ============================================================================
// TEST: outbound initiate with content UID sends 12-byte form
// ============================================================================

TEST(ProtocolStreamHandler, initiate_outbound_with_content_uid) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    uint8_t uid[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 256, 0xFF, uid);

    EXPECT_NE(stream, nullptr);
    EXPECT_EQ(info.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing->mti, MTI_STREAM_INIT_REQUEST);
    EXPECT_EQ(outgoing->payload_count, 12);

    // Bytes 6-11: Content UID
    for (int i = 0; i < 6; i++) {

        EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing, 6 + i), uid[i]);

    }

    // Content UID stored in stream state
    for (int i = 0; i < 6; i++) {

        EXPECT_EQ(stream->content_uid[i], uid[i]);

    }

}

// ============================================================================
// TEST: outbound initiate without content UID sends 6-byte form
// ============================================================================

TEST(ProtocolStreamHandler, initiate_outbound_without_content_uid) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 256, 0xFF, NULL);

    EXPECT_NE(stream, nullptr);
    EXPECT_EQ(outgoing->payload_count, 6);

}

// ============================================================================
// SECTION 15: TOTAL BYTES IN STREAM DATA COMPLETE
// ============================================================================

// ============================================================================
// TEST: send_complete includes total bytes transferred
// ============================================================================

TEST(ProtocolStreamHandler, send_complete_includes_total_bytes) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_init = OpenLcbBufferStore_allocate_buffer(BASIC);
    stream_state_t *stream = _open_outbound_stream(node, 128, outgoing_init);
    ASSERT_NE(stream, nullptr);

    // Send 24 bytes of data
    openlcb_msg_t *incoming_s = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_s = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_s = _build_statemachine_info(node, incoming_s, outgoing_s);

    uint8_t data[24];
    for (int i = 0; i < 24; i++) { data[i] = (uint8_t) i; }

    ProtocolStreamHandler_send_data(&info_s, stream, data, 24);
    EXPECT_EQ(stream->bytes_transferred, 24);

    // Send complete
    openlcb_msg_t *incoming_c = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_c = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_c = _build_statemachine_info(node, incoming_c, outgoing_c);

    ProtocolStreamHandler_send_complete(&info_c, stream);

    EXPECT_EQ(outgoing_c->payload_count, 8);

    // Bytes 4-7: total bytes = 24 (big-endian)
    uint8_t b4 = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_c, 4);
    uint8_t b5 = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_c, 5);
    uint8_t b6 = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_c, 6);
    uint8_t b7 = OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_c, 7);

    uint32_t total = ((uint32_t) b4 << 24) | ((uint32_t) b5 << 16) | ((uint32_t) b6 << 8) | b7;
    EXPECT_EQ(total, 24);

}

// ============================================================================
// SECTION 16: EARLY PROCEED
// ============================================================================

// ============================================================================
// TEST: early proceed sends Proceed as destination
// ============================================================================

TEST(ProtocolStreamHandler, early_proceed_as_destination) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    // Open as destination
    openlcb_msg_t *incoming_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_req = OpenLcbBufferStore_allocate_buffer(BASIC);
    _load_initiate_request(incoming_req, 128, 0xF0);
    openlcb_statemachine_info_t info_req = _build_statemachine_info(node, incoming_req, outgoing_req);
    ProtocolStreamHandler_initiate_request(&info_req);

    stream_state_t *stream = _last_stream;
    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->state, STREAM_STATE_OPEN);

    openlcb_msg_t *incoming_p = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_p = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_p = _build_statemachine_info(node, incoming_p, outgoing_p);

    bool ok = ProtocolStreamHandler_send_early_proceed(&info_p, stream);

    EXPECT_EQ(ok, true);
    EXPECT_EQ(info_p.outgoing_msg_info.valid, true);
    EXPECT_EQ(outgoing_p->mti, MTI_STREAM_PROCEED);
    EXPECT_EQ(outgoing_p->payload_count, 2);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_p, 0), stream->source_stream_id);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_p, 1), stream->dest_stream_id);
    EXPECT_EQ(outgoing_p->dest_alias, SOURCE_ALIAS);
    EXPECT_EQ(outgoing_p->dest_id, SOURCE_ID);

}

// ============================================================================
// TEST: early proceed rejected as source
// ============================================================================

TEST(ProtocolStreamHandler, early_proceed_rejected_as_source) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_init = OpenLcbBufferStore_allocate_buffer(BASIC);
    stream_state_t *stream = _open_outbound_stream(node, 128, outgoing_init);
    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->is_source, true);

    openlcb_msg_t *incoming_p = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_p = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info_p = _build_statemachine_info(node, incoming_p, outgoing_p);

    bool ok = ProtocolStreamHandler_send_early_proceed(&info_p, stream);

    EXPECT_EQ(ok, false);
    EXPECT_EQ(info_p.outgoing_msg_info.valid, false);

}

// ============================================================================
// SECTION 17: CONTEXT POINTER
// ============================================================================

// ============================================================================
// TEST: context pointer defaults to NULL and can be set
// ============================================================================

TEST(ProtocolStreamHandler, context_pointer) {

    _global_initialize(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_statemachine_info_t info = _build_statemachine_info(node, incoming, outgoing);

    stream_state_t *stream = ProtocolStreamHandler_initiate_outbound(
            &info, SOURCE_ALIAS, SOURCE_ID, 128, 0xFF, NULL);

    EXPECT_NE(stream, nullptr);
    EXPECT_EQ(stream->context, nullptr);

    // Application can set context
    int dummy_context = 42;
    stream->context = &dummy_context;
    EXPECT_EQ(stream->context, &dummy_context);

}
