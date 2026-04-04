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
 * @file protocol_config_mem_stream_handler_Test.cxx
 * @brief Test suite for Memory Configuration stream transport handler.
 *
 * @details Tests the CDI Read Stream proof-of-concept:
 * - Section 1: Initialization
 * - Section 2: Datagram callback two-phase dispatch
 * - Section 3: Stream callback routing (context-based dispatch)
 * - Section 4: Pump state machine (reply datagram, data pumping, completion)
 * - Section 5: Error and edge cases
 *
 * @author Jim Kueneman
 * @date 04 Apr 2026
 */

#include "test/main_Test.hxx"

#include <cstring>

#include "protocol_config_mem_stream_handler.h"
#include "protocol_stream_handler.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

// ============================================================================
// TEST CONSTANTS
// ============================================================================

#define SOURCE_ALIAS  0x222
#define SOURCE_ID     0x010203040506ULL
#define DEST_ALIAS    0xBBB
#define DEST_ID       0x060504030201ULL

// Small CDI for predictable test behavior
static const uint8_t _test_cdi[] = {

    '<', 'c', 'd', 'i', '>', 'H', 'e', 'l', 'l', 'o', '<', '/', 'c', 'd', 'i', '>', 0x00

};

#define TEST_CDI_LEN (sizeof(_test_cdi))

// ============================================================================
// MOCK CALLBACK TRACKING
// ============================================================================

static int _datagram_ok_called = 0;
static int _datagram_rejected_called = 0;
static uint16_t _datagram_ok_timeout = 0;
static uint16_t _datagram_rejected_code = 0;

static int _send_msg_called = 0;
static openlcb_msg_t *_last_sent_msg = NULL;
static bool _send_msg_return = true;

static int _stream_initiate_called = 0;
static stream_state_t _mock_stream;
static stream_state_t *_initiate_return = NULL;

static int _stream_send_data_called = 0;
static const uint8_t *_last_data_ptr = NULL;
static uint16_t _last_data_len = 0;

static int _stream_send_complete_called = 0;

static int _stream_send_terminate_called = 0;
static uint16_t _last_terminate_error = 0;

// User callback forwarding tracking
static int _user_initiate_request_called = 0;
static int _user_initiate_reply_called = 0;
static int _user_data_received_called = 0;
static int _user_data_proceed_called = 0;
static int _user_complete_called = 0;

// ============================================================================
// MOCK CALLBACKS
// ============================================================================

static void _mock_datagram_ok(openlcb_statemachine_info_t *statemachine_info, uint16_t timeout) {

    _datagram_ok_called++;
    _datagram_ok_timeout = timeout;

}

static void _mock_datagram_rejected(openlcb_statemachine_info_t *statemachine_info, uint16_t code) {

    _datagram_rejected_called++;
    _datagram_rejected_code = code;

}

static bool _mock_send_msg(openlcb_msg_t *msg) {

    _send_msg_called++;
    _last_sent_msg = msg;
    return _send_msg_return;

}

static stream_state_t *_mock_initiate_outbound(
        openlcb_statemachine_info_t *statemachine_info,
        uint16_t dest_alias,
        node_id_t dest_id,
        uint16_t proposed_buffer_size,
        uint8_t suggested_dest_stream_id,
        const uint8_t *content_uid) {

    _stream_initiate_called++;

    if (_initiate_return) {

        _initiate_return->state = STREAM_STATE_INITIATED;
        _initiate_return->is_source = true;
        _initiate_return->remote_alias = dest_alias;
        _initiate_return->remote_node_id = dest_id;
        _initiate_return->max_buffer_size = proposed_buffer_size;
        _initiate_return->bytes_remaining = 0;
        _initiate_return->bytes_transferred = 0;
        _initiate_return->source_stream_id = 0x01;
        _initiate_return->dest_stream_id = 0;
        statemachine_info->outgoing_msg_info.valid = true;

    }

    return _initiate_return;

}

static bool _mock_send_data(
        openlcb_statemachine_info_t *statemachine_info,
        stream_state_t *stream,
        const uint8_t *data,
        uint16_t data_len) {

    _stream_send_data_called++;
    _last_data_ptr = data;
    _last_data_len = data_len;

    stream->bytes_transferred += data_len;
    stream->bytes_remaining -= data_len;
    statemachine_info->outgoing_msg_info.valid = true;

    return true;

}

static void _mock_send_complete(
        openlcb_statemachine_info_t *statemachine_info,
        stream_state_t *stream) {

    _stream_send_complete_called++;
    statemachine_info->outgoing_msg_info.valid = true;

}

static void _mock_send_terminate(
        openlcb_statemachine_info_t *statemachine_info,
        stream_state_t *stream,
        uint16_t error_code) {

    _stream_send_terminate_called++;
    _last_terminate_error = error_code;

}

static bool _mock_user_initiate_request(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _user_initiate_request_called++;
    return true;

}

static void _mock_user_initiate_reply(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _user_initiate_reply_called++;

}

static void _mock_user_data_received(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _user_data_received_called++;

}

static void _mock_user_data_proceed(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _user_data_proceed_called++;

}

static void _mock_user_complete(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    _user_complete_called++;

}

// ---- Mock read_request callbacks ----

static uint16_t _mock_read_request_cdi(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    if (!node->parameters->cdi) {

        return 0;

    }

    memcpy(buffer, &node->parameters->cdi[address], count);

    return count;

}

static int _mock_read_request_config_mem_called = 0;
static uint8_t _mock_config_mem_data[64];

static uint16_t _mock_read_request_config_memory(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    _mock_read_request_config_mem_called++;

    for (uint16_t i = 0; i < count; i++) {

        buffer[i] = _mock_config_mem_data[(address + i) % sizeof(_mock_config_mem_data)];

    }

    return count;

}

// ---- Mock write_request callbacks ----

static int _mock_write_request_config_mem_called = 0;
static uint8_t _mock_write_dest[64];

static uint16_t _mock_write_request_config_memory(
        openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer) {

    _mock_write_request_config_mem_called++;

    for (uint16_t i = 0; i < count; i++) {

        _mock_write_dest[(address + i) % sizeof(_mock_write_dest)] = buffer[i];

    }

    return count;

}

// ============================================================================
// INTERFACE CONFIGURATIONS
// ============================================================================

static const interface_protocol_config_mem_stream_handler_t _interface_full = {

    .send_openlcb_msg = &_mock_send_msg,

    .load_datagram_received_ok_message = &_mock_datagram_ok,
    .load_datagram_received_rejected_message = &_mock_datagram_rejected,

    .stream_initiate_outbound = &_mock_initiate_outbound,
    .stream_send_data = &_mock_send_data,
    .stream_send_complete = &_mock_send_complete,
    .stream_send_terminate = &_mock_send_terminate,

    .read_request_config_definition_info = &_mock_read_request_cdi,
    .read_request_all = NULL,
    .read_request_configuration_memory = &_mock_read_request_config_memory,
    .read_request_acdi_manufacturer = NULL,
    .read_request_acdi_user = NULL,

    .write_request_configuration_memory = &_mock_write_request_config_memory,
    .write_request_acdi_user = NULL,

    .user_on_initiate_request = &_mock_user_initiate_request,
    .user_on_initiate_reply = &_mock_user_initiate_reply,
    .user_on_data_received = &_mock_user_data_received,
    .user_on_data_proceed = &_mock_user_data_proceed,
    .user_on_complete = &_mock_user_complete,

};

static const interface_protocol_config_mem_stream_handler_t _interface_no_user_callbacks = {

    .send_openlcb_msg = &_mock_send_msg,

    .load_datagram_received_ok_message = &_mock_datagram_ok,
    .load_datagram_received_rejected_message = &_mock_datagram_rejected,

    .stream_initiate_outbound = &_mock_initiate_outbound,
    .stream_send_data = &_mock_send_data,
    .stream_send_complete = &_mock_send_complete,
    .stream_send_terminate = &_mock_send_terminate,

    .read_request_config_definition_info = &_mock_read_request_cdi,
    .read_request_all = NULL,
    .read_request_configuration_memory = NULL,
    .read_request_acdi_manufacturer = NULL,
    .read_request_acdi_user = NULL,

    .write_request_configuration_memory = NULL,
    .write_request_acdi_user = NULL,

    .user_on_initiate_request = NULL,
    .user_on_initiate_reply = NULL,
    .user_on_data_received = NULL,
    .user_on_data_proceed = NULL,
    .user_on_complete = NULL,

};

// ============================================================================
// NODE PARAMETERS
// ============================================================================

static const node_parameters_t _node_params = {

    .snip = {

        .name = "Test Mfg",
        .model = "Test Model",
        .hardware_version = "1.0",
        .software_version = "1.0",

    },

    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_STREAM | PSI_DATAGRAM,

    .address_space_configuration_definition = {

        .present = true,
        .read_only = true,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .highest_address = (uint32_t) (TEST_CDI_LEN - 1),
        .low_address = 0,
        .description = "CDI",

    },

    .cdi = _test_cdi,
    .fdi = NULL,

};

static const node_parameters_t _node_params_no_cdi = {

    .snip = {

        .name = "Test Mfg",
        .model = "Test Model",
        .hardware_version = "1.0",
        .software_version = "1.0",

    },

    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_STREAM | PSI_DATAGRAM,

    .address_space_configuration_definition = {

        .present = false,
        .read_only = true,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .highest_address = 0,
        .low_address = 0,
        .description = "CDI",

    },

    .cdi = NULL,
    .fdi = NULL,

};

static const node_parameters_t _node_params_config_mem = {

    .snip = {

        .name = "Test Mfg",
        .model = "Test Model",
        .hardware_version = "1.0",
        .software_version = "1.0",

    },

    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_STREAM | PSI_DATAGRAM,

    .address_space_configuration_definition = {

        .present = true,
        .read_only = true,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .highest_address = (uint32_t) (TEST_CDI_LEN - 1),
        .low_address = 0,
        .description = "CDI",

    },

    .address_space_config_memory = {

        .present = true,
        .read_only = false,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .highest_address = 63,
        .low_address = 0,
        .description = "Config",

    },

    .cdi = _test_cdi,
    .fdi = NULL,

};

// ============================================================================
// HELPERS
// ============================================================================

static void _reset_mocks(void) {

    _datagram_ok_called = 0;
    _datagram_rejected_called = 0;
    _datagram_ok_timeout = 0;
    _datagram_rejected_code = 0;

    _send_msg_called = 0;
    _last_sent_msg = NULL;
    _send_msg_return = true;

    _stream_initiate_called = 0;
    memset(&_mock_stream, 0, sizeof(_mock_stream));
    _initiate_return = &_mock_stream;

    _stream_send_data_called = 0;
    _last_data_ptr = NULL;
    _last_data_len = 0;

    _stream_send_complete_called = 0;
    _stream_send_terminate_called = 0;
    _last_terminate_error = 0;

    _user_initiate_request_called = 0;
    _user_initiate_reply_called = 0;
    _user_data_received_called = 0;
    _user_data_proceed_called = 0;
    _user_complete_called = 0;

    _mock_read_request_config_mem_called = 0;
    memset(_mock_config_mem_data, 0, sizeof(_mock_config_mem_data));

    _mock_write_request_config_mem_called = 0;
    memset(_mock_write_dest, 0, sizeof(_mock_write_dest));

}

    /** @brief Resets only call counters, preserving mock_stream state. */
static void _reset_counters(void) {

    _datagram_ok_called = 0;
    _datagram_rejected_called = 0;
    _datagram_ok_timeout = 0;
    _datagram_rejected_code = 0;

    _send_msg_called = 0;
    _last_sent_msg = NULL;
    _send_msg_return = true;

    _stream_initiate_called = 0;

    _stream_send_data_called = 0;
    _last_data_ptr = NULL;
    _last_data_len = 0;

    _stream_send_complete_called = 0;
    _stream_send_terminate_called = 0;
    _last_terminate_error = 0;

    _user_initiate_request_called = 0;
    _user_initiate_reply_called = 0;
    _user_data_received_called = 0;
    _user_data_proceed_called = 0;
    _user_complete_called = 0;

    _mock_read_request_config_mem_called = 0;

}

static void _global_init(const interface_protocol_config_mem_stream_handler_t *iface) {

    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbNode_initialize(NULL);
    ProtocolConfigMemStreamHandler_initialize(iface);
    _reset_mocks();

}

static openlcb_statemachine_info_t _build_sm_info(
        openlcb_node_t *node,
        openlcb_msg_t *incoming,
        openlcb_msg_t *outgoing) {

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));
    info.openlcb_node = node;
    info.incoming_msg_info.msg_ptr = incoming;
    info.incoming_msg_info.enumerate = false;
    info.outgoing_msg_info.msg_ptr = outgoing;
    info.outgoing_msg_info.valid = false;
    return info;

}

    /** @brief Loads a Read Stream CDI command datagram (byte 1 = 0x63). */
static void _load_read_stream_cdi_datagram(
        openlcb_msg_t *msg,
        uint32_t address,
        uint8_t dest_stream_id,
        uint32_t read_count) {

    OpenLcbUtilities_load_openlcb_message(msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, CONFIG_MEM_CONFIGURATION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, CONFIG_MEM_READ_STREAM_SPACE_FF, 1);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, address, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0xFF, 6);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, dest_stream_id, 7);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, read_count, 8);
    msg->payload_count = 12;

}

    /** @brief Loads a Read Stream Config Memory (0xFD) command datagram. */
static void _load_read_stream_config_memory_datagram(
        openlcb_msg_t *msg,
        uint32_t address,
        uint8_t dest_stream_id,
        uint32_t read_count) {

    OpenLcbUtilities_load_openlcb_message(msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, CONFIG_MEM_CONFIGURATION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, CONFIG_MEM_READ_STREAM_SPACE_FD, 1);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, address, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0xFD, 6);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, dest_stream_id, 7);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, read_count, 8);
    msg->payload_count = 12;

}

    /** @brief Runs the full two-phase datagram dispatch. */
static void _run_two_phase_dispatch(openlcb_statemachine_info_t *info) {

    // Phase 1: ACK
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(info);

    // Phase 2: Initiate stream
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(info);

}

    /** @brief Simulates the stream being accepted by the remote node. */
static void _simulate_stream_accepted(void) {

    _mock_stream.state = STREAM_STATE_OPEN;
    _mock_stream.dest_stream_id = 0x42;
    _mock_stream.bytes_remaining = _mock_stream.max_buffer_size;

}

// ============================================================================
// SECTION 1: INITIALIZATION
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, initialize_does_not_crash) {

    _global_init(&_interface_full);
    SUCCEED();

}

TEST(ProtocolConfigMemStreamHandler, initialize_with_null_user_callbacks) {

    _global_init(&_interface_no_user_callbacks);
    SUCCEED();

}

// ============================================================================
// SECTION 2: DATAGRAM CALLBACK TWO-PHASE DISPATCH
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, phase1_sends_datagram_ack) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    // Phase 1 only
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);

    EXPECT_EQ(_datagram_ok_called, 1);
    EXPECT_EQ(_datagram_ok_timeout, 0x00);
    EXPECT_TRUE(node->state.openlcb_datagram_ack_sent);
    EXPECT_TRUE(info.incoming_msg_info.enumerate);

    // Stream not yet initiated
    EXPECT_EQ(_stream_initiate_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, phase2_initiates_outbound_stream) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    _run_two_phase_dispatch(&info);

    EXPECT_EQ(_datagram_ok_called, 1);
    EXPECT_EQ(_stream_initiate_called, 1);

    // Flags cleared after phase 2
    EXPECT_FALSE(node->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(info.incoming_msg_info.enumerate);

    // Stream context is set by the handler to its internal _context.
    // We can't access _context directly, but we can verify it's non-NULL.
    EXPECT_NE(_mock_stream.context, nullptr);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, phase2_stream_table_full_resets) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    // Force initiate_outbound to return NULL (table full)
    _initiate_return = NULL;

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    _run_two_phase_dispatch(&info);

    EXPECT_EQ(_stream_initiate_called, 1);

    // Module should reset to idle -- next call should be accepted
    // (not rejected as "busy")
    _initiate_return = &_mock_stream;
    _reset_mocks();

    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);
    info = _build_sm_info(node, incoming, outgoing);

    // Phase 1 should succeed (not busy)
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, reject_when_busy) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // Fill all USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS context pool slots
    stream_state_t mock_streams[USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS];
    memset(mock_streams, 0, sizeof(mock_streams));

    for (int i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        _initiate_return = &mock_streams[i];
        node->state.openlcb_datagram_ack_sent = false;
        _load_read_stream_cdi_datagram(incoming, 0, (uint8_t) (0x10 + i), 0);
        openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
        _run_two_phase_dispatch(&info);

        mock_streams[i].state = STREAM_STATE_OPEN;
        mock_streams[i].dest_stream_id = (uint8_t) (0x42 + i);
        mock_streams[i].bytes_remaining = mock_streams[i].max_buffer_size;
        ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &mock_streams[i]);

    }

    // Now try one more request -- should be rejected (pool exhausted)
    _reset_mocks();
    _initiate_return = &_mock_stream;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(incoming, 0, 0x20, 0);
    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);

    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, reject_when_no_cdi) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_no_cdi);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);

    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_PERMANENT_INVALID_ARGUMENTS);
    EXPECT_EQ(_datagram_ok_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, parses_request_parameters) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    uint32_t test_address = 0x00000005;
    uint8_t test_dest_sid = 0x42;
    uint32_t test_count = 10;

    _load_read_stream_cdi_datagram(incoming, test_address, test_dest_sid, test_count);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    _run_two_phase_dispatch(&info);

    // Verify the mock stream was set up with the parsed remote identity
    EXPECT_EQ(_mock_stream.remote_alias, SOURCE_ALIAS);
    EXPECT_EQ(_mock_stream.remote_node_id, SOURCE_ID);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

// ============================================================================
// SECTION 3: STREAM CALLBACK ROUTING
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, initiate_request_forwards_to_user) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));
    info.openlcb_node = node;

    stream_state_t other_stream;
    memset(&other_stream, 0, sizeof(other_stream));
    other_stream.context = NULL;  // Not ours

    bool result = ProtocolConfigMemStreamHandler_on_initiate_request(&info, &other_stream);

    EXPECT_EQ(_user_initiate_request_called, 1);
    EXPECT_TRUE(result);

}

TEST(ProtocolConfigMemStreamHandler, initiate_request_no_user_callback_returns_false) {

    _global_init(&_interface_no_user_callbacks);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));
    info.openlcb_node = node;

    stream_state_t other_stream;
    memset(&other_stream, 0, sizeof(other_stream));

    bool result = ProtocolConfigMemStreamHandler_on_initiate_request(&info, &other_stream);

    EXPECT_EQ(_user_initiate_request_called, 0);
    EXPECT_FALSE(result);

}

TEST(ProtocolConfigMemStreamHandler, initiate_reply_accepted_transitions_to_send_reply) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    // Simulate accepted
    _simulate_stream_accepted();

    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Should NOT forward to user (it's our stream)
    EXPECT_EQ(_user_initiate_reply_called, 0);

    // Pump should be in SEND_REPLY_DATAGRAM state
    // Verify by running the pump -- it should produce the Reply OK datagram
    _reset_counters();
    _send_msg_return = true;

    // Run 1: loads the reply datagram into the pump buffer (sets valid = true)
    ProtocolConfigMemStreamHandler_run();

    // Run 2: sees valid = true, sends the datagram
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, initiate_reply_rejected_resets_to_idle) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    // Simulate rejected (stream stays INITIATED, not OPEN)
    _mock_stream.state = STREAM_STATE_CLOSED;

    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Module should be idle -- can accept new request
    _reset_mocks();
    _initiate_return = &_mock_stream;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);
    info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);  // Accepted, not rejected as busy

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, initiate_reply_forwards_non_config_stream_to_user) {

    _global_init(&_interface_full);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));

    stream_state_t other_stream;
    memset(&other_stream, 0, sizeof(other_stream));
    other_stream.context = NULL;
    other_stream.state = STREAM_STATE_OPEN;

    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &other_stream);

    EXPECT_EQ(_user_initiate_reply_called, 1);

}

TEST(ProtocolConfigMemStreamHandler, data_received_always_forwards_to_user) {

    _global_init(&_interface_full);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));

    stream_state_t stream;
    memset(&stream, 0, sizeof(stream));

    ProtocolConfigMemStreamHandler_on_data_received(&info, &stream);

    EXPECT_EQ(_user_data_received_called, 1);

}

TEST(ProtocolConfigMemStreamHandler, data_proceed_for_our_stream_does_not_forward) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    // Our stream (context set by handler)
    ProtocolConfigMemStreamHandler_on_data_proceed(&info, &_mock_stream);

    EXPECT_EQ(_user_data_proceed_called, 0);  // Not forwarded

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, data_proceed_forwards_other_stream_to_user) {

    _global_init(&_interface_full);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));

    stream_state_t other_stream;
    memset(&other_stream, 0, sizeof(other_stream));
    other_stream.context = NULL;

    ProtocolConfigMemStreamHandler_on_data_proceed(&info, &other_stream);

    EXPECT_EQ(_user_data_proceed_called, 1);

}

TEST(ProtocolConfigMemStreamHandler, complete_for_our_stream_resets_to_idle) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    ProtocolConfigMemStreamHandler_on_complete(&info, &_mock_stream);

    EXPECT_EQ(_user_complete_called, 0);  // Not forwarded

    // Should be idle -- accepts new requests
    _reset_mocks();
    _initiate_return = &_mock_stream;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);
    info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, complete_forwards_other_stream_to_user) {

    _global_init(&_interface_full);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));

    stream_state_t other_stream;
    memset(&other_stream, 0, sizeof(other_stream));
    other_stream.context = NULL;

    ProtocolConfigMemStreamHandler_on_complete(&info, &other_stream);

    EXPECT_EQ(_user_complete_called, 1);

}

// ============================================================================
// SECTION 4: PUMP STATE MACHINE
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, pump_idle_does_nothing) {

    _global_init(&_interface_full);

    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 0);
    EXPECT_EQ(_stream_send_data_called, 0);
    EXPECT_EQ(_stream_send_complete_called, 0);

}

TEST(ProtocolConfigMemStreamHandler, pump_sends_reply_datagram_then_pumps_data) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Pump cycle 1: loads Reply OK datagram (sets valid = true)
    _reset_counters();
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();

    // Pump cycle 2: sees valid = true, sends the datagram
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);

    // Pump cycle 3: should start sending data
    _reset_counters();
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_stream_send_data_called, 1);
    EXPECT_NE(_last_data_ptr, nullptr);

    // Data should be CDI bytes starting from address 0
    EXPECT_EQ(_last_data_ptr[0], '<');  // First byte of CDI
    EXPECT_LE(_last_data_len, TEST_CDI_LEN);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, pump_sends_complete_after_all_data) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // CDI is 17 bytes. With a large enough window, should send in one chunk.
    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Run pump until complete
    int max_iterations = 20;
    int iteration = 0;
    bool complete_sent = false;

    while (iteration < max_iterations && !complete_sent) {

        _send_msg_return = true;
        ProtocolConfigMemStreamHandler_run();

        if (_stream_send_complete_called > 0) {

            complete_sent = true;

        }

        iteration++;

    }

    EXPECT_TRUE(complete_sent);
    EXPECT_GT(_stream_send_data_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, pump_respects_zero_bytes_remaining) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    _simulate_stream_accepted();
    // Set bytes_remaining to something very small
    _mock_stream.bytes_remaining = 3;
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Send reply datagram
    ProtocolConfigMemStreamHandler_run();
    ProtocolConfigMemStreamHandler_run();

    // Now pump data -- should only send 3 bytes
    _reset_counters();
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_stream_send_data_called, 1);
    EXPECT_EQ(_last_data_len, 3);

    // bytes_remaining is now 0 (mock decremented it), pump should NOT send more
    _reset_counters();
    ProtocolConfigMemStreamHandler_run();
    // It may try to send the pending message first, then check
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_stream_send_data_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, pump_with_specific_read_count) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // Request only 5 bytes of CDI
    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 5);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Run to completion
    uint32_t total_data_sent = 0;
    int max_iterations = 20;

    for (int i = 0; i < max_iterations; i++) {

        _send_msg_return = true;
        int before = _stream_send_data_called;
        ProtocolConfigMemStreamHandler_run();

        if (_stream_send_data_called > before) {

            total_data_sent += _last_data_len;

        }

        if (_stream_send_complete_called > 0) {

            break;

        }

    }

    EXPECT_EQ(total_data_sent, (uint32_t) 5);
    EXPECT_GT(_stream_send_complete_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, pump_with_start_address_offset) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // Start at offset 5 ("Hello" starts at offset 5 in _test_cdi)
    _load_read_stream_cdi_datagram(incoming, 5, 0x10, 5);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Run past the reply datagram
    ProtocolConfigMemStreamHandler_run();
    ProtocolConfigMemStreamHandler_run();

    // Now data pump runs
    _reset_counters();
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_stream_send_data_called, 1);
    EXPECT_EQ(_last_data_ptr[0], 'H');  // CDI[5] = 'H' from "Hello"
    EXPECT_EQ(_last_data_len, 5);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

// ============================================================================
// SECTION 5: ERROR AND EDGE CASES
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, pump_send_fails_retries_next_run) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Pump loads the reply datagram
    ProtocolConfigMemStreamHandler_run();

    // send_openlcb_msg fails
    _reset_counters();
    _send_msg_return = false;
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);

    // Try again -- send succeeds
    _send_msg_return = true;
    _reset_counters();
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);  // Retry succeeded

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, pump_address_past_end_sends_fail_reply) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // Start address past end of CDI
    _load_read_stream_cdi_datagram(incoming, (uint32_t) TEST_CDI_LEN + 10, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    // Phase 1: ACK
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    // Phase 2: Out-of-bounds -- no stream initiated, fail reply queued
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_stream_initiate_called, 0);

    // Pump sends the fail reply
    _reset_counters();
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);
    ASSERT_NE(_last_sent_msg, nullptr);
    EXPECT_EQ(*_last_sent_msg->payload[1], CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FF);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, remote_close_during_pump_resets) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    _run_two_phase_dispatch(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Remote end closes the stream
    ProtocolConfigMemStreamHandler_on_complete(&info, &_mock_stream);

    // Module should be idle now
    _reset_mocks();
    _initiate_return = &_mock_stream;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);
    info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);  // Accepted, not busy

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

// ============================================================================
// SECTION 6: GENERALIZED ADDRESS SPACE HANDLERS
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, config_memory_read_stream_pumps_data) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // Fill mock config memory with a known pattern
    for (int i = 0; i < 64; i++) {

        _mock_config_mem_data[i] = (uint8_t) (0xA0 + i);

    }

    _load_read_stream_config_memory_datagram(incoming, 0, 0x10, 10);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    // Phase 1
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    // Phase 2
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);
    EXPECT_EQ(_stream_initiate_called, 1);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Run pump to completion
    uint32_t total_data_sent = 0;

    for (int i = 0; i < 20; i++) {

        _send_msg_return = true;
        int before = _stream_send_data_called;
        ProtocolConfigMemStreamHandler_run();

        if (_stream_send_data_called > before) {

            total_data_sent += _last_data_len;

        }

        if (_stream_send_complete_called > 0) {

            break;

        }

    }

    EXPECT_EQ(total_data_sent, (uint32_t) 10);
    EXPECT_GT(_stream_send_complete_called, 0);
    EXPECT_GT(_mock_read_request_config_mem_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, reject_space_with_null_read_callback) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // All (0xFE) has read_request_all = NULL in _interface_full
    OpenLcbUtilities_load_openlcb_message(incoming, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, CONFIG_MEM_CONFIGURATION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, CONFIG_MEM_READ_STREAM_SPACE_FE, 1);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming, 0, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xFE, 6);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0x10, 7);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming, 0, 8);
    incoming->payload_count = 12;

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_all(&info);

    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_PERMANENT_INVALID_ARGUMENTS);
    EXPECT_EQ(_datagram_ok_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, reply_datagram_uses_correct_cmd_byte_for_config_memory) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_config_memory_datagram(incoming, 0, 0x10, 10);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    // Phase 1
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);

    // Phase 2
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Pump cycle 1: loads Reply OK datagram
    ProtocolConfigMemStreamHandler_run();

    // Pump cycle 2: sends it
    _reset_counters();
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);

    // The reply datagram should have cmd byte 0x71 (CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FD)
    ASSERT_NE(_last_sent_msg, nullptr);
    EXPECT_EQ(*_last_sent_msg->payload[1], CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FD);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, reject_space_not_present) {

    _global_init(&_interface_full);

    // Use _node_params which does NOT have config_memory space configured
    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_config_memory_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);

    // Config memory space not present on _node_params -- should reject
    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_PERMANENT_INVALID_ARGUMENTS);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

// ============================================================================
// SECTION 7: OUT-OF-BOUNDS ADDRESS FAIL REPLY
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, out_of_bounds_cdi_sends_fail_reply) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // Address well past CDI end (highest_address = TEST_CDI_LEN - 1 = 16)
    _load_read_stream_cdi_datagram(incoming, 0x1000, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    // Phase 1: ACK
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    // Phase 2: Should detect out-of-bounds, NOT initiate a stream
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_stream_initiate_called, 0);

    // Flags should be cleared
    EXPECT_FALSE(node->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(info.incoming_msg_info.enumerate);

    // Pump should have a pending fail reply datagram
    _reset_counters();
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);
    ASSERT_NE(_last_sent_msg, nullptr);

    // Check fail reply format
    EXPECT_EQ(*_last_sent_msg->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*_last_sent_msg->payload[1], CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FF);

    // Bytes 2-5: address (0x1000 big-endian)
    EXPECT_EQ(*_last_sent_msg->payload[2], 0x00);
    EXPECT_EQ(*_last_sent_msg->payload[3], 0x00);
    EXPECT_EQ(*_last_sent_msg->payload[4], 0x10);
    EXPECT_EQ(*_last_sent_msg->payload[5], 0x00);

    // Bytes 6-7: error code (0x1082 big-endian)
    EXPECT_EQ(*_last_sent_msg->payload[6], 0x10);
    EXPECT_EQ(*_last_sent_msg->payload[7], 0x82);

    EXPECT_EQ(_last_sent_msg->payload_count, 8);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, out_of_bounds_config_memory_sends_fail_reply) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // Config memory highest_address = 63; address 100 is out of bounds
    _load_read_stream_config_memory_datagram(incoming, 100, 0x10, 10);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    // Phase 1
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    // Phase 2
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);
    EXPECT_EQ(_stream_initiate_called, 0);

    // Pump sends the fail reply
    _reset_counters();
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);
    ASSERT_NE(_last_sent_msg, nullptr);
    EXPECT_EQ(*_last_sent_msg->payload[1], CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FD);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, out_of_bounds_module_accepts_new_request) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // First request: out of bounds
    _load_read_stream_cdi_datagram(incoming, 0x1000, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    // Two-phase dispatch for the out-of-bounds request
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);

    // Let the pump send the fail reply
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();

    // Now try a valid request -- should NOT be rejected as busy
    _reset_mocks();
    _initiate_return = &_mock_stream;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);
    info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);
    EXPECT_EQ(_datagram_rejected_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

// ============================================================================
// SECTION 8: TIMEOUT HANDLING
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, timeout_in_wait_initiate_reply_resets) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 10;

    _run_two_phase_dispatch(&info);

    // Now in WAIT_INITIATE_REPLY with tick_snapshot = 10
    // No reply arrives -- check timeout at tick 39 (elapsed = 29, not yet)
    _reset_counters();
    ProtocolConfigMemStreamHandler_check_timeouts(39);
    EXPECT_EQ(_stream_send_terminate_called, 0);

    // Check timeout at tick 40 (elapsed = 30, times out)
    ProtocolConfigMemStreamHandler_check_timeouts(40);
    EXPECT_EQ(_stream_send_terminate_called, 1);
    EXPECT_EQ(_last_terminate_error, ERROR_TEMPORARY_TIME_OUT);

    // Module should be idle -- accepts new requests
    _reset_mocks();
    _initiate_return = &_mock_stream;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);
    info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);
    EXPECT_EQ(_datagram_rejected_called, 0);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, timeout_in_pumping_stall_terminates) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 5;

    _run_two_phase_dispatch(&info);

    // Simulate stream accepted at tick 5
    _simulate_stream_accepted();
    info.current_tick = 5;
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Run pump to send reply datagram and start pumping
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();  // loads reply datagram
    ProtocolConfigMemStreamHandler_run();  // sends reply datagram
    ProtocolConfigMemStreamHandler_run();  // pumps first data chunk

    // Now bytes_remaining = 0 (exhausted window), remote never sends Data Proceed
    // tick_snapshot was refreshed to 5 at on_initiate_reply
    // Check at tick 34 (elapsed = 29, not yet)
    _reset_counters();
    ProtocolConfigMemStreamHandler_check_timeouts(34);
    EXPECT_EQ(_stream_send_terminate_called, 0);

    // Check at tick 35 (elapsed = 30, times out)
    ProtocolConfigMemStreamHandler_check_timeouts(35);
    EXPECT_EQ(_stream_send_terminate_called, 1);
    EXPECT_EQ(_last_terminate_error, ERROR_TEMPORARY_TIME_OUT);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, data_proceed_resets_timeout) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 0;

    _run_two_phase_dispatch(&info);

    // Stream accepted at tick 0
    _simulate_stream_accepted();
    info.current_tick = 0;
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Run pump to get into PUMPING and exhaust initial window
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();  // loads reply datagram
    ProtocolConfigMemStreamHandler_run();  // sends reply datagram
    ProtocolConfigMemStreamHandler_run();  // pumps data, exhausts window

    // At tick 25, Data Proceed arrives -- refreshes tick_snapshot to 25
    info.current_tick = 25;
    _mock_stream.bytes_remaining = _mock_stream.max_buffer_size;
    ProtocolConfigMemStreamHandler_on_data_proceed(&info, &_mock_stream);

    // At tick 54 (elapsed since last proceed = 29), should NOT time out
    _reset_counters();
    ProtocolConfigMemStreamHandler_check_timeouts(54);
    EXPECT_EQ(_stream_send_terminate_called, 0);

    // At tick 55 (elapsed since last proceed = 30), SHOULD time out
    ProtocolConfigMemStreamHandler_check_timeouts(55);
    EXPECT_EQ(_stream_send_terminate_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, timeout_wraps_correctly) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_read_stream_cdi_datagram(incoming, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 240;  // Near wrap point

    _run_two_phase_dispatch(&info);

    // tick_snapshot = 240, check at tick 13 (240 + 30 = 270, wraps to 14)
    // elapsed = (13 - 240) as uint8_t = 29, not yet
    _reset_counters();
    ProtocolConfigMemStreamHandler_check_timeouts(13);
    EXPECT_EQ(_stream_send_terminate_called, 0);

    // tick 14: elapsed = (14 - 240) as uint8_t = 30, times out
    ProtocolConfigMemStreamHandler_check_timeouts(14);
    EXPECT_EQ(_stream_send_terminate_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

// ============================================================================
// SECTION 9: WRITE STREAM
// ============================================================================

    /** @brief Loads a Write Stream Config Memory (0xFD) command datagram.
     *
     * Write Stream Command layout (byte-1 encoding for 0xFD):
     * - Byte 0: 0x20
     * - Byte 1: 0x21 (CONFIG_MEM_WRITE_STREAM_SPACE_FD)
     * - Bytes 2-5: Starting address (big-endian)
     * - Byte 6: Source Stream ID
     */
static void _load_write_stream_config_memory_datagram(
        openlcb_msg_t *msg,
        uint32_t address,
        uint8_t source_stream_id) {

    OpenLcbUtilities_load_openlcb_message(msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, CONFIG_MEM_CONFIGURATION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, CONFIG_MEM_WRITE_STREAM_SPACE_FD, 1);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, address, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, source_stream_id, 6);
    msg->payload_count = 7;

}

    /** @brief Runs the two-phase dispatch for a Write Stream Config Memory command. */
static void _run_write_two_phase_dispatch(openlcb_statemachine_info_t *info) {

    // Phase 1: ACK
    ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(info);

    // Phase 2: Validate and transition to WRITE_WAIT_STREAM_INITIATE
    ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(info);

}

    /** @brief Simulates the remote node initiating an inbound stream to us. */
static void _simulate_write_stream_initiate(openlcb_statemachine_info_t *info) {

    _mock_stream.state = STREAM_STATE_OPEN;
    _mock_stream.remote_alias = SOURCE_ALIAS;
    _mock_stream.remote_node_id = SOURCE_ID;
    _mock_stream.dest_stream_id = 0x55;
    _mock_stream.source_stream_id = 0x01;
    _mock_stream.is_source = false;
    _mock_stream.bytes_remaining = 0;
    _mock_stream.bytes_transferred = 0;
    _mock_stream.max_buffer_size = 256;

    ProtocolConfigMemStreamHandler_on_initiate_request(info, &_mock_stream);

}

    /** @brief Simulates receiving a data frame on the inbound stream.
     *
     * CAN stream data frames: byte 0 = DID, bytes 1..N = data.
     */
static void _simulate_data_received(
        openlcb_statemachine_info_t *info,
        openlcb_msg_t *msg,
        const uint8_t *data,
        uint16_t data_len) {

    OpenLcbUtilities_load_openlcb_message(msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);

    // Byte 0 = DID
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, _mock_stream.dest_stream_id, 0);

    // Bytes 1..N = actual data
    for (uint16_t i = 0; i < data_len; i++) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, data[i], i + 1);

    }

    msg->payload_count = data_len + 1;

    ProtocolConfigMemStreamHandler_on_data_received(info, &_mock_stream);

}

TEST(ProtocolConfigMemStreamHandler, write_stream_basic_flow) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_write_stream_config_memory_datagram(incoming, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 0;

    // Two-phase dispatch: ACK + transition to WRITE_WAIT_STREAM_INITIATE
    _run_write_two_phase_dispatch(&info);

    EXPECT_EQ(_datagram_ok_called, 1);

    // Remote initiates inbound stream
    _simulate_write_stream_initiate(&info);

    // Send a small data chunk: 4 bytes
    uint8_t write_data[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    _simulate_data_received(&info, incoming, write_data, 4);

    EXPECT_EQ(_mock_write_request_config_mem_called, 1);
    EXPECT_EQ(_mock_write_dest[0], 0xDE);
    EXPECT_EQ(_mock_write_dest[1], 0xAD);
    EXPECT_EQ(_mock_write_dest[2], 0xBE);
    EXPECT_EQ(_mock_write_dest[3], 0xEF);

    // Remote sends stream complete
    ProtocolConfigMemStreamHandler_on_complete(&info, &_mock_stream);

    // Pump: first run() loads the reply datagram, second run() sends it
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();  // loads write reply ok datagram
    ProtocolConfigMemStreamHandler_run();  // sends it

    EXPECT_EQ(_send_msg_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, write_stream_reject_read_only_space) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // CDI is read-only, so Write Stream CDI should be rejected
    OpenLcbUtilities_load_openlcb_message(incoming, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM);
    OpenLcbUtilities_clear_openlcb_message_payload(incoming);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, CONFIG_MEM_CONFIGURATION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, CONFIG_MEM_WRITE_STREAM_SPACE_FF, 1);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming, 0, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(incoming, 0xAA, 6);
    incoming->payload_count = 7;

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_write_stream_space_config_description_info(&info);

    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_PERMANENT_INVALID_ARGUMENTS);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, write_stream_reject_space_not_present) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // _node_params has no config memory space (not present)
    _load_write_stream_config_memory_datagram(incoming, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(&info);

    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_PERMANENT_INVALID_ARGUMENTS);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, write_stream_out_of_bounds_sends_fail_reply) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    // Address 100 is past highest_address (63)
    _load_write_stream_config_memory_datagram(incoming, 100, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);

    _run_write_two_phase_dispatch(&info);

    // Phase 2 should have loaded a fail reply datagram
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_send_msg_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, write_stream_non_matching_initiate_forwards_to_user) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_write_stream_config_memory_datagram(incoming, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 0;

    _run_write_two_phase_dispatch(&info);

    // Initiate from a DIFFERENT alias -- should forward to user callback
    stream_state_t other_stream;
    memset(&other_stream, 0, sizeof(other_stream));
    other_stream.remote_alias = 0x999;  // Not SOURCE_ALIAS
    other_stream.state = STREAM_STATE_OPEN;

    bool accepted = ProtocolConfigMemStreamHandler_on_initiate_request(&info, &other_stream);

    EXPECT_TRUE(accepted);  // user callback returns true
    EXPECT_EQ(_user_initiate_request_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, write_stream_timeout_waiting_for_stream) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_write_stream_config_memory_datagram(incoming, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 5;

    _run_write_two_phase_dispatch(&info);

    // At tick 34 (elapsed = 29), should NOT time out
    _reset_counters();
    ProtocolConfigMemStreamHandler_check_timeouts(34);
    EXPECT_EQ(_stream_send_terminate_called, 0);

    // At tick 35 (elapsed = 30), SHOULD time out
    // No stream is open yet, so terminate should not be called
    // but context should reset to IDLE
    ProtocolConfigMemStreamHandler_check_timeouts(35);

    // After timeout, a new request should be accepted (proves we are back to IDLE)
    _reset_mocks();
    _load_write_stream_config_memory_datagram(incoming, 0, 0xBB);
    info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, write_stream_accepts_new_request_after_complete) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_write_stream_config_memory_datagram(incoming, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 0;

    _run_write_two_phase_dispatch(&info);
    _simulate_write_stream_initiate(&info);

    uint8_t write_data[] = { 0x11, 0x22 };
    _simulate_data_received(&info, incoming, write_data, 2);

    ProtocolConfigMemStreamHandler_on_complete(&info, &_mock_stream);

    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();

    // Now start a second write request -- should succeed
    _reset_mocks();
    _load_write_stream_config_memory_datagram(incoming, 10, 0xBB);
    info = _build_sm_info(node, incoming, outgoing);

    ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

TEST(ProtocolConfigMemStreamHandler, write_stream_multiple_data_chunks) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(incoming, nullptr);
    ASSERT_NE(outgoing, nullptr);

    _load_write_stream_config_memory_datagram(incoming, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, incoming, outgoing);
    info.current_tick = 0;

    _run_write_two_phase_dispatch(&info);
    _simulate_write_stream_initiate(&info);

    // First chunk: 3 bytes at address 0
    uint8_t chunk1[] = { 0xAA, 0xBB, 0xCC };
    _simulate_data_received(&info, incoming, chunk1, 3);

    EXPECT_EQ(_mock_write_request_config_mem_called, 1);
    EXPECT_EQ(_mock_write_dest[0], 0xAA);
    EXPECT_EQ(_mock_write_dest[1], 0xBB);
    EXPECT_EQ(_mock_write_dest[2], 0xCC);

    // Second chunk: 2 bytes at address 3
    uint8_t chunk2[] = { 0xDD, 0xEE };
    _simulate_data_received(&info, incoming, chunk2, 2);

    EXPECT_EQ(_mock_write_request_config_mem_called, 2);
    EXPECT_EQ(_mock_write_dest[3], 0xDD);
    EXPECT_EQ(_mock_write_dest[4], 0xEE);

    ProtocolConfigMemStreamHandler_on_complete(&info, &_mock_stream);

    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();  // loads write reply ok datagram
    ProtocolConfigMemStreamHandler_run();  // sends it

    EXPECT_EQ(_send_msg_called, 1);

    OpenLcbBufferStore_free_buffer(incoming);
    OpenLcbBufferStore_free_buffer(outgoing);

}

// ============================================================================
// SECTION 10: REMAINING READ/WRITE SPACE HANDLERS AND EDGE CASES
// ============================================================================

static void _load_read_stream_space_in_byte6_datagram(
        openlcb_msg_t *msg, uint8_t cmd_byte, uint8_t space_byte,
        uint32_t address, uint8_t dest_stream_id, uint32_t read_count) {

    OpenLcbUtilities_load_openlcb_message(msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, CONFIG_MEM_CONFIGURATION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, cmd_byte, 1);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, address, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, space_byte, 6);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, dest_stream_id, 7);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, read_count, 8);
    msg->payload_count = 12;

}

static void _load_write_stream_space_in_byte6_datagram(
        openlcb_msg_t *msg, uint8_t cmd_byte, uint8_t space_byte,
        uint32_t address, uint8_t source_stream_id) {

    OpenLcbUtilities_load_openlcb_message(msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM);
    OpenLcbUtilities_clear_openlcb_message_payload(msg);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, CONFIG_MEM_CONFIGURATION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, cmd_byte, 1);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, address, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, space_byte, 6);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, source_stream_id, 7);
    msg->payload_count = 8;

}

static const node_parameters_t _node_params_all_spaces = {

    .snip = { .name = "Mfg", .model = "Model", .hardware_version = "1.0", .software_version = "1.0" },
    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_STREAM | PSI_DATAGRAM,
    .address_space_configuration_definition = { .present = true, .read_only = true, .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO, .highest_address = (uint32_t)(TEST_CDI_LEN - 1) },
    .address_space_all = { .present = true, .read_only = true, .address_space = CONFIG_MEM_SPACE_ALL, .highest_address = 255 },
    .address_space_config_memory = { .present = true, .read_only = false, .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY, .highest_address = 63 },
    .address_space_acdi_manufacturer = { .present = true, .read_only = true, .address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS, .highest_address = 0x7C },
    .address_space_acdi_user = { .present = true, .read_only = false, .address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS, .highest_address = 0x7F },
    .address_space_train_function_definition_info = { .present = true, .read_only = true, .address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO, .highest_address = 255 },
    .address_space_train_function_config_memory = { .present = true, .read_only = false, .address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY, .highest_address = 255 },
    .address_space_firmware = { .present = true, .read_only = false, .address_space = 0xEF, .highest_address = 255 },
    .cdi = _test_cdi, .fdi = NULL,

};

static const node_parameters_t _node_params_large_config_mem = {

    .snip = { .name = "Mfg", .model = "Model", .hardware_version = "1.0", .software_version = "1.0" },
    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_STREAM | PSI_DATAGRAM,
    .address_space_configuration_definition = { .present = true, .read_only = true, .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO, .highest_address = (uint32_t)(TEST_CDI_LEN - 1) },
    .address_space_config_memory = { .present = true, .read_only = false, .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY, .highest_address = 511 },
    .cdi = _test_cdi, .fdi = NULL,

};

static uint16_t _mock_read_returns_zero(openlcb_node_t *n, uint32_t a, uint16_t c, uint8_t *b) { return 0; }

static bool _mock_send_data_fails(openlcb_statemachine_info_t *si, stream_state_t *s, const uint8_t *d, uint16_t l) {

    _stream_send_data_called++;
    return false;

}

static const interface_protocol_config_mem_stream_handler_t _interface_all_spaces = {

    .send_openlcb_msg = &_mock_send_msg,
    .load_datagram_received_ok_message = &_mock_datagram_ok,
    .load_datagram_received_rejected_message = &_mock_datagram_rejected,
    .stream_initiate_outbound = &_mock_initiate_outbound,
    .stream_send_data = &_mock_send_data,
    .stream_send_complete = &_mock_send_complete,
    .stream_send_terminate = &_mock_send_terminate,
    .read_request_config_definition_info = &_mock_read_request_cdi,
    .read_request_all = &_mock_read_request_cdi,
    .read_request_configuration_memory = &_mock_read_request_config_memory,
    .read_request_acdi_manufacturer = &_mock_read_request_cdi,
    .read_request_acdi_user = &_mock_read_request_cdi,
    .read_request_train_function_definition_info = &_mock_read_request_cdi,
    .read_request_train_function_config_memory = &_mock_read_request_cdi,
    .read_request_firmware = &_mock_read_request_cdi,
    .write_request_configuration_memory = &_mock_write_request_config_memory,
    .write_request_acdi_user = &_mock_write_request_config_memory,
    .write_request_train_function_config_memory = &_mock_write_request_config_memory,
    .write_request_firmware = &_mock_write_request_config_memory,
    .user_on_initiate_request = &_mock_user_initiate_request,
    .user_on_initiate_reply = &_mock_user_initiate_reply,
    .user_on_data_received = &_mock_user_data_received,
    .user_on_data_proceed = &_mock_user_data_proceed,
    .user_on_complete = &_mock_user_complete,

};

#define SPACE_HANDLER_TEST(test_name, handler_func, cmd, space) \
TEST(ProtocolConfigMemStreamHandler, test_name) { \
    _global_init(&_interface_all_spaces); \
    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces); \
    node->alias = DEST_ALIAS; \
    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM); \
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM); \
    _load_read_stream_space_in_byte6_datagram(in, cmd, space, 0, 0x10, 10); \
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out); \
    handler_func(&info); \
    EXPECT_EQ(_datagram_ok_called, 1); \
    OpenLcbBufferStore_free_buffer(in); \
    OpenLcbBufferStore_free_buffer(out); \
}

SPACE_HANDLER_TEST(read_stream_acdi_manufacturer_accepts,
        ProtocolConfigMemStreamHandler_handle_read_stream_space_acdi_manufacturer,
        CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6, 0xFC)

SPACE_HANDLER_TEST(read_stream_acdi_user_accepts,
        ProtocolConfigMemStreamHandler_handle_read_stream_space_acdi_user,
        CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6, 0xFB)

SPACE_HANDLER_TEST(read_stream_train_fdi_accepts,
        ProtocolConfigMemStreamHandler_handle_read_stream_space_train_function_definition_info,
        CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6, 0xFA)

SPACE_HANDLER_TEST(read_stream_train_fn_config_accepts,
        ProtocolConfigMemStreamHandler_handle_read_stream_space_train_function_config_memory,
        CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6, 0xF9)

SPACE_HANDLER_TEST(read_stream_firmware_accepts,
        ProtocolConfigMemStreamHandler_handle_read_stream_space_firmware,
        CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6, 0xEF)

#define WRITE_REJECT_TEST(test_name, handler_func, space) \
TEST(ProtocolConfigMemStreamHandler, test_name) { \
    _global_init(&_interface_all_spaces); \
    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces); \
    node->alias = DEST_ALIAS; \
    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM); \
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM); \
    _load_write_stream_space_in_byte6_datagram(in, CONFIG_MEM_WRITE_STREAM_SPACE_IN_BYTE_6, space, 0, 0xAA); \
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out); \
    handler_func(&info); \
    EXPECT_EQ(_datagram_rejected_called, 1); \
    OpenLcbBufferStore_free_buffer(in); \
    OpenLcbBufferStore_free_buffer(out); \
}

WRITE_REJECT_TEST(write_stream_all_rejected, ProtocolConfigMemStreamHandler_handle_write_stream_space_all, 0xFE)
WRITE_REJECT_TEST(write_stream_acdi_mfg_rejected, ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_manufacturer, 0xFC)
WRITE_REJECT_TEST(write_stream_train_fdi_rejected, ProtocolConfigMemStreamHandler_handle_write_stream_space_train_function_definition_info, 0xFA)

#define WRITE_ACCEPT_TEST(test_name, handler_func, space) \
TEST(ProtocolConfigMemStreamHandler, test_name) { \
    _global_init(&_interface_all_spaces); \
    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces); \
    node->alias = DEST_ALIAS; \
    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM); \
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM); \
    _load_write_stream_space_in_byte6_datagram(in, CONFIG_MEM_WRITE_STREAM_SPACE_IN_BYTE_6, space, 0, 0xAA); \
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out); \
    handler_func(&info); \
    EXPECT_EQ(_datagram_ok_called, 1); \
    OpenLcbBufferStore_free_buffer(in); \
    OpenLcbBufferStore_free_buffer(out); \
}

WRITE_ACCEPT_TEST(write_stream_acdi_user_accepts, ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_user, 0xFB)
WRITE_ACCEPT_TEST(write_stream_train_fn_config_accepts, ProtocolConfigMemStreamHandler_handle_write_stream_space_train_function_config_memory, 0xF9)
WRITE_ACCEPT_TEST(write_stream_firmware_accepts, ProtocolConfigMemStreamHandler_handle_write_stream_space_firmware, 0xEF)

TEST(ProtocolConfigMemStreamHandler, write_stream_busy_rejection) {

    _global_init(&_interface_all_spaces);
    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces);
    node->alias = DEST_ALIAS;
    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    // Fill all pool slots with write stream operations
    for (int i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        node->state.openlcb_datagram_ack_sent = false;
        _load_write_stream_config_memory_datagram(in, (uint32_t) (i * 10), (uint8_t) (0xAA + i));
        openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
        info.current_tick = 0;
        _run_write_two_phase_dispatch(&info);

    }

    // One more -- should be rejected (pool exhausted)
    _reset_mocks();
    node->state.openlcb_datagram_ack_sent = false;
    _load_write_stream_config_memory_datagram(in, 100, 0xBB);
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);

    ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(&info);
    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, pump_read_returns_zero) {

    interface_protocol_config_mem_stream_handler_t iface = _interface_all_spaces;
    iface.read_request_config_definition_info = &_mock_read_returns_zero;
    _global_init(&iface);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;
    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    _load_read_stream_cdi_datagram(in, 0, 0x10, 0);
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    _run_two_phase_dispatch(&info);
    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();
    ProtocolConfigMemStreamHandler_run();

    _reset_counters();
    ProtocolConfigMemStreamHandler_run();
    EXPECT_EQ(_stream_send_data_called, 0);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, pump_send_data_fails) {

    interface_protocol_config_mem_stream_handler_t iface = _interface_all_spaces;
    iface.stream_send_data = &_mock_send_data_fails;
    _global_init(&iface);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;
    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    _load_read_stream_cdi_datagram(in, 0, 0x10, 0);
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    _run_two_phase_dispatch(&info);
    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();
    ProtocolConfigMemStreamHandler_run();

    _reset_counters();
    ProtocolConfigMemStreamHandler_run();
    EXPECT_EQ(_stream_send_data_called, 1);
    EXPECT_EQ(_stream_send_complete_called, 0);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, read_stream_short_payload) {

    _global_init(&_interface_full);
    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;
    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    OpenLcbUtilities_load_openlcb_message(in, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_DATAGRAM);
    OpenLcbUtilities_clear_openlcb_message_payload(in);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(in, CONFIG_MEM_CONFIGURATION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(in, CONFIG_MEM_READ_STREAM_SPACE_FF, 1);
    OpenLcbUtilities_copy_dword_to_openlcb_payload(in, 0, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(in, 0xFF, 6);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(in, 0x10, 7);
    in->payload_count = 8;

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, run_write_wait_and_receiving_noop) {

    _global_init(&_interface_all_spaces);
    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces);
    node->alias = DEST_ALIAS;
    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    _load_write_stream_config_memory_datagram(in, 0, 0xAA);
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    info.current_tick = 0;
    _run_write_two_phase_dispatch(&info);

    _reset_counters();
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();
    EXPECT_EQ(_send_msg_called, 0);

    _simulate_write_stream_initiate(&info);

    _reset_counters();
    ProtocolConfigMemStreamHandler_run();
    EXPECT_EQ(_send_msg_called, 0);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

// ============================================================================
// SECTION 7: CONCURRENT STREAM TESTS
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, two_concurrent_read_streams) {

    _global_init(&_interface_all_spaces);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    // Start stream A -- CDI read
    stream_state_t mock_stream_a;
    memset(&mock_stream_a, 0, sizeof(mock_stream_a));
    _initiate_return = &mock_stream_a;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(in, 0, 0x10, 0);
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    _run_two_phase_dispatch(&info);
    mock_stream_a.state = STREAM_STATE_OPEN;
    mock_stream_a.dest_stream_id = 0x42;
    mock_stream_a.bytes_remaining = mock_stream_a.max_buffer_size;
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &mock_stream_a);

    // Start stream B -- Config Memory read
    stream_state_t mock_stream_b;
    memset(&mock_stream_b, 0, sizeof(mock_stream_b));
    _initiate_return = &mock_stream_b;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(in, 0, 0x20, 10);
    info = _build_sm_info(node, in, out);

    // Use Config Memory handler for stream B
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);
    mock_stream_b.state = STREAM_STATE_OPEN;
    mock_stream_b.dest_stream_id = 0x43;
    mock_stream_b.bytes_remaining = mock_stream_b.max_buffer_size;
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &mock_stream_b);

    // Both streams are now in SEND_REPLY_DATAGRAM phase
    // Pump should service them round-robin
    _reset_counters();
    _send_msg_return = true;

    // First run: loads reply datagram for context 0 (sets valid=true)
    ProtocolConfigMemStreamHandler_run();
    // Second run: drains the outgoing message (calls send_openlcb_msg)
    ProtocolConfigMemStreamHandler_run();
    EXPECT_EQ(_send_msg_called, 1);

    // Third run: loads reply datagram for context 1
    ProtocolConfigMemStreamHandler_run();
    // Fourth run: drains it
    ProtocolConfigMemStreamHandler_run();
    EXPECT_EQ(_send_msg_called, 2);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, concurrent_read_and_write_stream) {

    _global_init(&_interface_all_spaces);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    // Start a read stream (CDI)
    stream_state_t mock_read_stream;
    memset(&mock_read_stream, 0, sizeof(mock_read_stream));
    _initiate_return = &mock_read_stream;
    node->state.openlcb_datagram_ack_sent = false;
    _load_read_stream_cdi_datagram(in, 0, 0x10, 0);
    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    _run_two_phase_dispatch(&info);
    mock_read_stream.state = STREAM_STATE_OPEN;
    mock_read_stream.dest_stream_id = 0x42;
    mock_read_stream.bytes_remaining = mock_read_stream.max_buffer_size;
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &mock_read_stream);

    // Start a write stream (Config Memory)
    node->state.openlcb_datagram_ack_sent = false;
    _load_write_stream_config_memory_datagram(in, 0, 0xAA);
    info = _build_sm_info(node, in, out);
    info.current_tick = 0;
    _run_write_two_phase_dispatch(&info);

    // Read stream is in SEND_REPLY_DATAGRAM, write stream is in WRITE_WAIT
    // Pump should service the read stream (write is waiting for initiate)
    _reset_counters();
    _send_msg_return = true;

    // First run: loads reply datagram for the read stream
    ProtocolConfigMemStreamHandler_run();
    // Second run: drains it (calls send_openlcb_msg)
    ProtocolConfigMemStreamHandler_run();
    EXPECT_EQ(_send_msg_called, 1);

    // Write stream in WRITE_WAIT -- pump should not try to send anything more
    _reset_counters();
    ProtocolConfigMemStreamHandler_run();
    EXPECT_EQ(_send_msg_called, 0);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, round_robin_fairness) {

    _global_init(&_interface_all_spaces);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    // Start two read streams, both pumping
    stream_state_t streams[2];
    memset(streams, 0, sizeof(streams));

    for (int i = 0; i < 2; i++) {

        _initiate_return = &streams[i];
        node->state.openlcb_datagram_ack_sent = false;
        _load_read_stream_cdi_datagram(in, 0, (uint8_t) (0x10 + i), 0);
        openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
        _run_two_phase_dispatch(&info);
        streams[i].state = STREAM_STATE_OPEN;
        streams[i].dest_stream_id = (uint8_t) (0x42 + i);
        streams[i].bytes_remaining = streams[i].max_buffer_size;
        ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &streams[i]);

    }

    // Both in SEND_REPLY_DATAGRAM. Pump through reply datagrams.
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();  // Services context 0 (sends reply datagram)
    ProtocolConfigMemStreamHandler_run();  // Drains outgoing for context 0
    ProtocolConfigMemStreamHandler_run();  // Services context 1 (sends reply datagram)
    ProtocolConfigMemStreamHandler_run();  // Drains outgoing for context 1

    // Both now in PUMPING. Give them flow control window.
    streams[0].bytes_remaining = 128;
    streams[1].bytes_remaining = 128;

    _reset_counters();

    // Each run() should alternate which stream gets data pumped
    ProtocolConfigMemStreamHandler_run();
    int after_first = _stream_send_data_called;
    EXPECT_EQ(after_first, 1);

    ProtocolConfigMemStreamHandler_run();  // Drain outgoing
    ProtocolConfigMemStreamHandler_run();  // Pump other stream
    int after_second = _stream_send_data_called;
    EXPECT_EQ(after_second, 2);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

// ============================================================================
// SECTION 12: BRANCH COVERAGE GAP TESTS
// ============================================================================

TEST(ProtocolConfigMemStreamHandler, initiate_reply_null_stream_null_user_callback) {

    _global_init(&_interface_no_user_callbacks);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));

    // NULL stream triggers _find_context_by_stream's !stream guard
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, NULL);

    // No user callback to forward to either
    EXPECT_EQ(_user_initiate_reply_called, 0);

}

TEST(ProtocolConfigMemStreamHandler, data_proceed_null_user_callback) {

    _global_init(&_interface_no_user_callbacks);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));

    stream_state_t other;
    memset(&other, 0, sizeof(other));

    ProtocolConfigMemStreamHandler_on_data_proceed(&info, &other);

    EXPECT_EQ(_user_data_proceed_called, 0);

}

TEST(ProtocolConfigMemStreamHandler, complete_null_user_callback) {

    _global_init(&_interface_no_user_callbacks);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));

    stream_state_t other;
    memset(&other, 0, sizeof(other));

    ProtocolConfigMemStreamHandler_on_complete(&info, &other);

    EXPECT_EQ(_user_complete_called, 0);

}

TEST(ProtocolConfigMemStreamHandler, data_received_null_user_callback) {

    _global_init(&_interface_no_user_callbacks);

    openlcb_statemachine_info_t info;
    memset(&info, 0, sizeof(info));

    stream_state_t other;
    memset(&other, 0, sizeof(other));

    ProtocolConfigMemStreamHandler_on_data_received(&info, &other);

    EXPECT_EQ(_user_data_received_called, 0);

}

TEST(ProtocolConfigMemStreamHandler, data_received_read_stream_context_forwards_to_user) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    _load_read_stream_cdi_datagram(in, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    _run_two_phase_dispatch(&info);

    // Context is WAIT_INITIATE_REPLY; _find_context_by_stream finds ctx
    // but ctx->phase != WRITE_RECEIVING, so outer if is false
    _reset_counters();
    ProtocolConfigMemStreamHandler_on_data_received(&info, &_mock_stream);

    EXPECT_EQ(_user_data_received_called, 1);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, phase2_find_allocated_iterates_past_other_node) {

    _global_init(&_interface_full);

    openlcb_node_t *node_a = OpenLcbNode_allocate(0x010101010101ULL, &_node_params);
    node_a->alias = 0x111;

    openlcb_node_t *node_b = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node_b->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    // Phase 1 on node A first -- takes context pool slot 0
    _load_read_stream_cdi_datagram(in, 0, 0x10, 0);
    openlcb_statemachine_info_t info_a = _build_sm_info(node_a, in, out);
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info_a);

    // Phase 1 on node B -- takes context pool slot 1
    stream_state_t mock_stream_b;
    memset(&mock_stream_b, 0, sizeof(mock_stream_b));
    _initiate_return = &mock_stream_b;
    _load_read_stream_cdi_datagram(in, 0, 0x20, 5);
    openlcb_statemachine_info_t info_b = _build_sm_info(node_b, in, out);
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info_b);

    // Phase 2 on node B -- _find_allocated_context iterates:
    //   slot 0: phase==ALLOCATED, node==node_a (!=node_b) -> skip
    //   slot 1: phase==ALLOCATED, node==node_b -> match
    _reset_counters();
    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info_b);
    EXPECT_EQ(_stream_initiate_called, 1);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, phase2_without_phase1_returns_safely) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    _load_read_stream_cdi_datagram(in, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);

    // Force Phase 2 path without Phase 1 allocation
    node->state.openlcb_datagram_ack_sent = true;
    info.incoming_msg_info.enumerate = true;

    ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(&info);

    // _find_allocated_context returns NULL; handler clears flags and returns
    EXPECT_FALSE(node->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(info.incoming_msg_info.enumerate);
    EXPECT_EQ(_stream_initiate_called, 0);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, write_phase2_without_phase1_returns_safely) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    _load_write_stream_config_memory_datagram(in, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);

    // Force Phase 2 path without Phase 1
    node->state.openlcb_datagram_ack_sent = true;
    info.incoming_msg_info.enumerate = true;

    ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(&info);

    EXPECT_FALSE(node->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(info.incoming_msg_info.enumerate);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, read_count_exceeds_space_clamps_to_available) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    // CDI is 17 bytes. Request 100 -- should clamp to 17.
    _load_read_stream_cdi_datagram(in, 0, 0x10, 100);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    _run_two_phase_dispatch(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    uint32_t total = 0;

    for (int i = 0; i < 20; i++) {

        _send_msg_return = true;
        int before = _stream_send_data_called;
        ProtocolConfigMemStreamHandler_run();

        if (_stream_send_data_called > before) {

            total += _last_data_len;

        }

        if (_stream_send_complete_called > 0) {

            break;

        }

    }

    EXPECT_EQ(total, (uint32_t) TEST_CDI_LEN);
    EXPECT_GT(_stream_send_complete_called, 0);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, timeout_in_write_receiving_terminates) {

    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    _load_write_stream_config_memory_datagram(in, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    info.current_tick = 10;

    _run_write_two_phase_dispatch(&info);

    // Simulate inbound stream -- transitions to WRITE_RECEIVING
    _simulate_write_stream_initiate(&info);

    // At tick 39 (elapsed = 29), should NOT time out
    _reset_counters();
    ProtocolConfigMemStreamHandler_check_timeouts(39);
    EXPECT_EQ(_stream_send_terminate_called, 0);

    // At tick 40 (elapsed = 30), SHOULD time out
    ProtocolConfigMemStreamHandler_check_timeouts(40);
    EXPECT_EQ(_stream_send_terminate_called, 1);
    EXPECT_EQ(_last_terminate_error, ERROR_TEMPORARY_TIME_OUT);

    // Module should be idle -- accepts new requests
    _reset_mocks();
    _load_write_stream_config_memory_datagram(in, 0, 0xBB);
    info = _build_sm_info(node, in, out);

    ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(&info);
    EXPECT_EQ(_datagram_ok_called, 1);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, reject_present_read_space_with_null_callback) {

    // _interface_full has read_request_all = NULL
    // _node_params_all_spaces has address_space_all.present = true
    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    _load_read_stream_space_in_byte6_datagram(
            in, CONFIG_MEM_READ_STREAM_SPACE_FE, 0xFE, 0, 0x10, 10);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    ProtocolConfigMemStreamHandler_handle_read_stream_space_all(&info);

    // present=true, read_func=NULL -> !present false, !read_func true
    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_PERMANENT_INVALID_ARGUMENTS);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, reject_writable_space_with_null_write_callback) {

    // _interface_full has write_request_acdi_user = NULL
    // _node_params_all_spaces has address_space_acdi_user = { present=true, read_only=false }
    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_all_spaces);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    _load_write_stream_space_in_byte6_datagram(
            in, CONFIG_MEM_WRITE_STREAM_SPACE_IN_BYTE_6, 0xFB, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_user(&info);

    // present=true, read_only=false, write_func=NULL -> rejected
    EXPECT_EQ(_datagram_rejected_called, 1);
    EXPECT_EQ(_datagram_rejected_code, ERROR_PERMANENT_INVALID_ARGUMENTS);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, pump_chunk_clamps_to_max_payload) {

    // Space has 512 bytes. remaining (512) > max_payload (LEN_MESSAGE_BYTES_STREAM - 1).
    // This covers the remaining >= max_payload ternary branch in _pump_next_chunk.
    _global_init(&_interface_all_spaces);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_large_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    // Fill mock config memory
    for (int i = 0; i < 64; i++) {

        _mock_config_mem_data[i] = (uint8_t) (0xA0 + i);

    }

    // Read from address 0, count 0 (read all = 512 bytes)
    _load_read_stream_config_memory_datagram(in, 0, 0x10, 0);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);

    // Phase 1
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);

    // Phase 2
    ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(&info);

    _simulate_stream_accepted();
    ProtocolConfigMemStreamHandler_on_initiate_reply(&info, &_mock_stream);

    // Send reply datagram
    _send_msg_return = true;
    ProtocolConfigMemStreamHandler_run();
    ProtocolConfigMemStreamHandler_run();

    // Now pump data -- first chunk should be clamped to max_payload
    _reset_counters();
    ProtocolConfigMemStreamHandler_run();

    EXPECT_EQ(_stream_send_data_called, 1);
    EXPECT_EQ(_last_data_len, LEN_MESSAGE_BYTES_STREAM - 1);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}

TEST(ProtocolConfigMemStreamHandler, write_data_received_empty_payload_no_write) {

    // A stream data frame with only the DID byte (payload_count = 1, no actual data)
    // covers the payload_count <= 1 branch in on_data_received.
    _global_init(&_interface_full);

    openlcb_node_t *node = OpenLcbNode_allocate(DEST_ID, &_node_params_config_mem);
    node->alias = DEST_ALIAS;

    openlcb_msg_t *in = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *out = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);

    _load_write_stream_config_memory_datagram(in, 0, 0xAA);

    openlcb_statemachine_info_t info = _build_sm_info(node, in, out);
    info.current_tick = 0;

    _run_write_two_phase_dispatch(&info);
    _simulate_write_stream_initiate(&info);

    // Send a data frame with only the DID byte and no data
    OpenLcbUtilities_load_openlcb_message(
            in, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_STREAM_SEND);
    OpenLcbUtilities_clear_openlcb_message_payload(in);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(in, _mock_stream.dest_stream_id, 0);
    in->payload_count = 1;

    _reset_counters();
    ProtocolConfigMemStreamHandler_on_data_received(&info, &_mock_stream);

    // payload_count == 1, so write callback should NOT be called
    EXPECT_EQ(_mock_write_request_config_mem_called, 0);

    OpenLcbBufferStore_free_buffer(in);
    OpenLcbBufferStore_free_buffer(out);

}
