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
* @file openlcb_main_statemachine_Test.cxx
* @brief Comprehensive test suite for OpenLCB main protocol state machine
*
* @details This test suite validates:
* - Main state machine initialization
* - Node message processing logic (does_node_process_msg)
* - MTI-based message dispatch to protocol handlers
* - NULL handler fallback (Interaction Rejected)
* - Outgoing message transmission
* - Re-enumeration for multi-message responses
* - Incoming message pop from FIFO
* - Node enumeration (first/next)
* - Main run loop priority and dispatch
*
* Test Strategy:
* - Tests with all optional handlers populated (full protocol)
* - Tests with NULL optional handlers (minimal protocol + auto-reject)
* - Edge case coverage for addressing, initialization states
* - Priority order verification in run loop
*
* Coverage Analysis:
* - OpenLcbMainStatemachine_initialize: 100%
* - OpenLcbMainStatemachine_does_node_process_msg: 100%
* - OpenLcbMainStatemachine_load_interaction_rejected: 100%
* - OpenLcbMainStatemachine_process_main_statemachine: 100%
* - OpenLcbMainStatemachine_handle_outgoing_openlcb_message: 100%
* - OpenLcbMainStatemachine_handle_try_reenumerate: 100%
* - OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message: 100%
* - OpenLcbMainStatemachine_handle_try_enumerate_first_node: 100%
* - OpenLcbMainStatemachine_handle_try_enumerate_next_node: 100%
* - OpenLcbMainStatemachine_run: 100%
*
* @author Jim Kueneman
* @date 19 Jan 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_main_statemachine.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "protocol_broadcast_time_handler.h"
#include "protocol_train_search_handler.h"
#include "protocol_train_handler.h"

// ============================================================================
// Test Control Variables
// ============================================================================

void *called_function_ptr = nullptr;
bool track_all_calls = true;  // Control whether to track all calls or just specific ones
bool load_interaction_rejected_called = false;
bool broadcast_time_handler_called = false;
bool pc_event_report_handler_called = false;
bool producer_identified_set_handler_called = false;
bool reply_to_protocol_support_inquiry = false;
bool force_process_statemachine_to_fail = false;
bool send_openlcb_msg_called = false;
bool process_statemachine_called = false;
bool node_get_first_called = false;
bool node_get_next_called = false;
bool does_node_process_msg = false;
openlcb_node_t *node_get_first = nullptr;
openlcb_node_t *node_get_next = nullptr;
uint16_t mock_node_count = 2;
bool allow_successful_transmit = true;
bool force_true_does_node_process_msg = false;
bool force_false_does_node_process_msg = false;

// Mock handle function control flags (following login statemachine pattern)
bool fail_handle_outgoing_openlcb_message = false;
bool fail_handle_try_reenumerate = false;
bool fail_handle_try_pop_next_incoming_openlcb_message = false;
bool fail_handle_try_enumerate_first_node = false;
bool fail_handle_try_enumerate_next_node = false;
bool fifo_has_message = false;
bool fifo_pop_called = false;
openlcb_node_t *mock_first_node = nullptr;
openlcb_node_t *mock_next_node = nullptr;

// Mock tick counter for get_current_tick interface
static uint8_t _test_global_100ms_tick = 0;

// Handle function call tracking
bool handle_outgoing_called = false;
bool handle_reenumerate_called = false;
bool handle_pop_called = false;
bool handle_enumerate_first_called = false;
bool handle_enumerate_next_called = false;

// Train search / emergency / datagram tracking
bool train_search_event_handler_called = false;
bool train_search_no_match_handler_called = false;
bool train_emergency_event_handler_called = false;
bool load_datagram_rejected_called = false;
bool force_is_last_node = false;
bool train_search_handler_set_valid = false;

// Static dummy train state for tests
static train_state_t _dummy_train_state;

// ============================================================================
// Test Node Parameters
// ============================================================================

node_parameters_t _node_parameters_main_node = {

    .snip = {
        .mfg_version = 4,
        .name = "Test",
        .model = "Test Model J",
        .hardware_version = "0.001",
        .software_version = "0.002",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .configuration_options = {
        .write_under_mask_supported = 1,
        .unaligned_reads_supported = 1,
        .unaligned_writes_supported = 1,
        .read_from_manufacturer_space_0xfc_supported = 1,
        .read_from_user_space_0xfb_supported = 1,
        .write_to_user_space_0xfb_supported = 1,
        .stream_read_write_supported = 0,
        .high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .description = "These are options that defined the memory space capabilities"
    },

    // Space 0xFF - Configuration Definition Info
    .address_space_configuration_definition = {
        .present = 0,
        .read_only = 1,
        .low_address_valid = 0,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .highest_address = 0x200,
        .low_address = 0,
        .description = "Configuration definition info"
    },

    // Space 0xFE - All Memory
    .address_space_all = {
        .present = 0,
        .read_only = 1,
        .low_address_valid = 0,
        .address_space = CONFIG_MEM_SPACE_ALL,
        .highest_address = 0,
        .low_address = 0,
        .description = "All memory Info"
    },

    // Space 0xFD - Configuration Memory
    .address_space_config_memory = {
        .present = 0,
        .read_only = 0,
        .low_address_valid = 0,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .highest_address = 0x200,
        .low_address = 0,
        .description = "Configuration memory storage"
    },

    .cdi = NULL,
    .fdi = NULL,
};

// ============================================================================
// Mock Helper Functions
// ============================================================================

/**
 * @brief Accumulates called function addresses for tracking
 * @param function_ptr Address of function that was called
 */
void _update_called_function_ptr(void *function_ptr)
{
    called_function_ptr = (void *)((long long)function_ptr + (long long)called_function_ptr);
}

// ============================================================================
// Mock Clock Access
// ============================================================================

uint8_t _mock_get_current_tick(void)
{

    return _test_global_100ms_tick;

}

// ============================================================================
// Mock Protocol Handlers - SNIP
// ============================================================================

void _ProtocolSnip_handle_simple_node_info_request(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolSnip_handle_simple_node_info_request);
}

void _ProtocolSnip_handle_simple_node_info_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolSnip_handle_simple_node_info_reply);
}

// ============================================================================
// Mock Protocol Handlers - Message Network
// ============================================================================

void _ProtocolMessageNetwork_initialization_complete(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_initialization_complete);
}

void _ProtocolMessageNetwork_initialization_complete_simple(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_initialization_complete_simple);
}

void _ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_handle_verify_node_id_addressed);
}

void _ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_handle_verify_node_id_global);
}

void _ProtocolMessageNetwork_handle_verified_node_id(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_handle_verified_node_id);
}

void _ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_handle_optional_interaction_rejected);
}

void _ProtocolMessageNetwork_handle_terminate_due_to_error(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_handle_terminate_due_to_error);
}

void _ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_handle_protocol_support_inquiry);

    if (reply_to_protocol_support_inquiry)
    {
        statemachine_info->outgoing_msg_info.valid = true;
    }
}

void _ProtocolMessageNetwork_handle_protocol_support_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolMessageNetwork_handle_protocol_support_reply);
}

// ============================================================================
// Mock Protocol Handlers - Event Transport
// ============================================================================

void _ProtocolEventTransport_handle_consumer_identify(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_consumer_identify);
}

void _ProtocolEventTransport_handle_consumer_range_identified(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_consumer_range_identified);
}

void _ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_consumer_identified_unknown);
}

void _ProtocolEventTransport_handle_consumer_identified_set(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_consumer_identified_set);
}

void _ProtocolEventTransport_handle_consumer_identified_clear(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_consumer_identified_clear);
}

void _ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_consumer_identified_reserved);
}

void _ProtocolEventTransport_handle_producer_identify(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_producer_identify);
}

void _ProtocolEventTransport_handle_producer_range_identified(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_producer_range_identified);
}

void _ProtocolEventTransport_handle_producer_identified_unknown(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_producer_identified_unknown);
}

void _ProtocolEventTransport_handle_producer_identified_set(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_producer_identified_set);
    producer_identified_set_handler_called = true;
}

void _ProtocolEventTransport_handle_producer_identified_clear(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_producer_identified_clear);
}

void _ProtocolEventTransport_handle_producer_identified_reserved(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_producer_identified_reserved);
}

void _ProtocolEventTransport_handle_identify_dest(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_identify_dest);
}

void _ProtocolEventTransport_handle_identify(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_identify);
}

void _ProtocolEventTransport_handle_event_learn(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_event_learn);
}

void _ProtocolEventTransport_handle_pc_event_report(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_pc_event_report);
    pc_event_report_handler_called = true;
}

void _ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolEventTransport_handle_pc_event_report_with_payload);
}

// ============================================================================
// Mock Protocol Handlers - Broadcast Time
// ============================================================================

void _ProtocolBroadcastTime_handle_time_event(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id)
{
    _update_called_function_ptr((void *)&_ProtocolBroadcastTime_handle_time_event);
    broadcast_time_handler_called = true;
}

// ============================================================================
// Mock Protocol Handlers - Train Search & Emergency
// ============================================================================

void _mock_train_search_event_handler(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id)
{
    _update_called_function_ptr((void *)&_mock_train_search_event_handler);
    train_search_event_handler_called = true;
    if (train_search_handler_set_valid) {
        statemachine_info->outgoing_msg_info.valid = true;
    }
}

void _mock_train_search_no_match_handler(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id)
{
    _update_called_function_ptr((void *)&_mock_train_search_no_match_handler);
    train_search_no_match_handler_called = true;
}

void _mock_train_emergency_event_handler(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id)
{
    _update_called_function_ptr((void *)&_mock_train_emergency_event_handler);
    train_emergency_event_handler_called = true;
}

// ============================================================================
// Mock Protocol Handlers - Datagram Rejected
// ============================================================================

void _mock_load_datagram_rejected(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code)
{
    _update_called_function_ptr((void *)&_mock_load_datagram_rejected);
    load_datagram_rejected_called = true;
}

// ============================================================================
// Mock Protocol Handlers - Train
// ============================================================================

void _ProtocolTrainControl_command(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolTrainControl_command);
}

void _ProtocolTrainControl_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolTrainControl_reply);
}

// ============================================================================
// Mock Protocol Handlers - Simple Train Node
// ============================================================================

void _ProtocolSimpleTrainNodeIdentInfo_request(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolSimpleTrainNodeIdentInfo_request);
}

void _ProtocolSimpleTrainNodeIdentInfo_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolSimpleTrainNodeIdentInfo_reply);
}

// ============================================================================
// Mock Protocol Handlers - Datagram
// ============================================================================

void _ProtocolDatagram_handle_datagram(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolDatagram_handle_datagram);
}

void _ProtocolDatagram_handle_datagram_ok_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolDatagram_handle_datagram_ok_reply);
}

void _ProtocolDatagram_handle_datagram_rejected_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolDatagram_handle_datagram_rejected_reply);
}

// ============================================================================
// Mock Protocol Handlers - Stream
// ============================================================================

void _ProtocolStream_initiate_request(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolStream_initiate_request);
}

void _ProtocolStream_initiate_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolStream_initiate_reply);
}

void _ProtocolStream_send_data(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolStream_send_data);
}

void _ProtocolStream_data_proceed(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolStream_data_proceed);
}

void _ProtocolStream_data_complete(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_ProtocolStream_data_complete);
}

// ============================================================================
// Mock Required Functions - Node Management
// ============================================================================

openlcb_node_t *_OpenLcbNode_get_first(uint8_t key)
{
    _update_called_function_ptr((void *)&_OpenLcbNode_get_first);
    node_get_first_called = true;

    return node_get_first;
}

openlcb_node_t *_OpenLcbNode_get_next(uint8_t key)
{
    _update_called_function_ptr((void *)&_OpenLcbNode_get_next);
    node_get_next_called = true;

    return node_get_next;
}

bool _OpenLcbNode_is_last(uint8_t key)
{
    _update_called_function_ptr((void *)&_OpenLcbNode_is_last);

    return force_is_last_node;
}

uint16_t _OpenLcbNode_get_count(void)
{

    return mock_node_count;

}

// ============================================================================
// Mock Required Functions - Message Transmission
// ============================================================================

bool _CanTxStatemachine_send_openlcb_message(openlcb_msg_t *outgoing_msg)
{
    _update_called_function_ptr((void *)&_CanTxStatemachine_send_openlcb_message);
    send_openlcb_msg_called = true;

    return allow_successful_transmit;
}

// ============================================================================
// Mock Required Functions - Resource Locking
// ============================================================================

void _ExampleDrivers_lock_shared_resources(void)
{
    _update_called_function_ptr((void *)&_ExampleDrivers_lock_shared_resources);
}

void _ExampleDrivers_unlock_shared_resources(void)
{
    _update_called_function_ptr((void *)&_ExampleDrivers_unlock_shared_resources);
}

// ============================================================================
// Mock Required Functions - Utilities
// ============================================================================

void _OpenLcbUtilities_load_interaction_rejected(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_OpenLcbUtilities_load_interaction_rejected);
    load_interaction_rejected_called = true;
}

// ============================================================================
// Mock Handler Functions - State Machine Control
// ============================================================================

bool _OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void)
{
    _update_called_function_ptr((void *)&_OpenLcbMainStatemachine_handle_outgoing_openlcb_message);
    handle_outgoing_called = true;

    if (fail_handle_outgoing_openlcb_message)
    {
        return false;
    }

    // Simulate real behavior without calling real function to avoid recursive call tracking
    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    if (state->outgoing_msg_info.valid)
    {
        // Simulate sending
        if (allow_successful_transmit)
        {
            state->outgoing_msg_info.valid = false;
        }
        send_openlcb_msg_called = true;
        return true;  // Message pending, stop cascade
    }
    
    return false;  // No outgoing message, continue cascade
}

bool _OpenLcbMainStatemachine_handle_try_reenumerate(void)
{
    _update_called_function_ptr((void *)&_OpenLcbMainStatemachine_handle_try_reenumerate);
    handle_reenumerate_called = true;

    if (fail_handle_try_reenumerate)
    {
        return false;
    }

    // Simulate real behavior
    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    if (state->incoming_msg_info.enumerate)
    {
        process_statemachine_called = true;
        return true;  // Enumerate flag set, stop cascade
    }
    
    return false;  // No enumerate flag, continue cascade
}

bool _OpenLcbMainStatemachine_handle_try_enumerate_first_node(void)
{
    _update_called_function_ptr((void *)&_OpenLcbMainStatemachine_handle_try_enumerate_first_node);
    handle_enumerate_first_called = true;

    if (fail_handle_try_enumerate_first_node)
    {
        return false;
    }

    // Simulate real behavior
    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    if (!state->openlcb_node)
    {
        if (node_get_first)
        {
            state->openlcb_node = node_get_first;
            node_get_first_called = true;
        }
        return true;  // Attempted, stop cascade
    }
    
    return false;  // Already have node, continue cascade
}

bool _OpenLcbMainStatemachine_handle_try_enumerate_next_node(void)
{
    _update_called_function_ptr((void *)&_OpenLcbMainStatemachine_handle_try_enumerate_next_node);
    handle_enumerate_next_called = true;

    if (fail_handle_try_enumerate_next_node)
    {
        return false;
    }

    // Simulate real behavior
    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    if (state->openlcb_node)
    {
        if (node_get_next)
        {
            state->openlcb_node = node_get_next;
            node_get_next_called = true;
        }
        return true;  // Attempted, stop cascade
    }
    
    return false;  // No current node, not applicable
}

bool _OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message(void)
{
    _update_called_function_ptr((void *)&_OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message);
    handle_pop_called = true;

    if (fail_handle_try_pop_next_incoming_openlcb_message)
    {
        return false;
    }

    // If fifo has a message, simulate successful pop
    if (fifo_has_message)
    {
        fifo_pop_called = true;
        openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
        if (!state->incoming_msg_info.msg_ptr)
        {
            state->incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
        }
        // Real function returns true if pop FAILED, false if pop SUCCEEDED
        // So if we successfully popped, return false to stop cascade
        return false;
    }

    return OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message();
}

// ============================================================================
// Mock Internal Functions - Process Dispatch
// ============================================================================

void _OpenLcbMainStatemachine_process_main_statemachine(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_OpenLcbMainStatemachine_process_main_statemachine);
    process_statemachine_called = true;

    if (!force_process_statemachine_to_fail)
    {
        OpenLcbMainStatemachine_process_main_statemachine(statemachine_info);
    }
}

bool _OpenLcbMainStatemachine_does_node_process_msg(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_OpenLcbMainStatemachine_does_node_process_msg);

    if (force_true_does_node_process_msg)
    {
        return true;
    }

    if (force_false_does_node_process_msg)
    {
        return false;
    }

    return OpenLcbMainStatemachine_does_node_process_msg(statemachine_info);
}

// ============================================================================
// Interface Structures
// ============================================================================

/**
 * @brief Full interface with all optional handlers populated
 * @details Used to test complete protocol stack functionality
 */
const interface_openlcb_main_statemachine_t interface_openlcb_main_statemachine = {

    // Resource Management (REQUIRED)
    .lock_shared_resources = &_ExampleDrivers_lock_shared_resources,
    .unlock_shared_resources = &_ExampleDrivers_unlock_shared_resources,
    .send_openlcb_msg = &_CanTxStatemachine_send_openlcb_message,
    .get_current_tick = &_mock_get_current_tick,

    // Node Enumeration (REQUIRED)
    .openlcb_node_get_first = &_OpenLcbNode_get_first,
    .openlcb_node_get_next = &_OpenLcbNode_get_next,
    .openlcb_node_is_last = &_OpenLcbNode_is_last,
    .openlcb_node_get_count = &_OpenLcbNode_get_count,

    // Core Handlers (REQUIRED)
    .load_interaction_rejected = &_OpenLcbUtilities_load_interaction_rejected,

    // Message Network Protocol Handlers
    .message_network_initialization_complete = &_ProtocolMessageNetwork_initialization_complete,
    .message_network_initialization_complete_simple = &_ProtocolMessageNetwork_initialization_complete_simple,
    .message_network_verify_node_id_addressed = &_ProtocolMessageNetwork_handle_verify_node_id_addressed,
    .message_network_verify_node_id_global = &_ProtocolMessageNetwork_handle_verify_node_id_global,
    .message_network_verified_node_id = &_ProtocolMessageNetwork_handle_verified_node_id,
    .message_network_optional_interaction_rejected = &_ProtocolMessageNetwork_handle_optional_interaction_rejected,
    .message_network_terminate_due_to_error = &_ProtocolMessageNetwork_handle_terminate_due_to_error,
    .message_network_protocol_support_inquiry = &_ProtocolMessageNetwork_handle_protocol_support_inquiry,
    .message_network_protocol_support_reply = &_ProtocolMessageNetwork_handle_protocol_support_reply,

    // Internal functions (exposed for unit testing)
    .process_main_statemachine = _OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = _OpenLcbMainStatemachine_does_node_process_msg,
    .handle_outgoing_openlcb_message = &_OpenLcbMainStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &_OpenLcbMainStatemachine_handle_try_reenumerate,
    .handle_try_pop_next_incoming_openlcb_message = &_OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message,
    .handle_try_enumerate_first_node = &_OpenLcbMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &_OpenLcbMainStatemachine_handle_try_enumerate_next_node,

    // Optional - SNIP
    .snip_simple_node_info_request = &_ProtocolSnip_handle_simple_node_info_request,
    .snip_simple_node_info_reply = &_ProtocolSnip_handle_simple_node_info_reply,

    // Optional - Event Transport
    .event_transport_consumer_identify = &_ProtocolEventTransport_handle_consumer_identify,
    .event_transport_consumer_range_identified = &_ProtocolEventTransport_handle_consumer_range_identified,
    .event_transport_consumer_identified_unknown = &_ProtocolEventTransport_handle_consumer_identified_unknown,
    .event_transport_consumer_identified_set = &_ProtocolEventTransport_handle_consumer_identified_set,
    .event_transport_consumer_identified_clear = &_ProtocolEventTransport_handle_consumer_identified_clear,
    .event_transport_consumer_identified_reserved = &_ProtocolEventTransport_handle_consumer_identified_reserved,
    .event_transport_producer_identify = &_ProtocolEventTransport_handle_producer_identify,
    .event_transport_producer_range_identified = &_ProtocolEventTransport_handle_producer_range_identified,
    .event_transport_producer_identified_unknown = &_ProtocolEventTransport_handle_producer_identified_unknown,
    .event_transport_producer_identified_set = &_ProtocolEventTransport_handle_producer_identified_set,
    .event_transport_producer_identified_clear = &_ProtocolEventTransport_handle_producer_identified_clear,
    .event_transport_producer_identified_reserved = &_ProtocolEventTransport_handle_producer_identified_reserved,
    .event_transport_identify_dest = &_ProtocolEventTransport_handle_identify_dest,
    .event_transport_identify = &_ProtocolEventTransport_handle_identify,
    .event_transport_learn = &_ProtocolEventTransport_handle_event_learn,
    .event_transport_pc_report = &_ProtocolEventTransport_handle_pc_event_report,
    .event_transport_pc_report_with_payload = &_ProtocolEventTransport_handle_pc_event_report_with_payload,

    // Optional - Train
    .train_control_command = &_ProtocolTrainControl_command,
    .train_control_reply = &_ProtocolTrainControl_reply,

    // Optional - Simple Train Node
    .simple_train_node_ident_info_request = &_ProtocolSimpleTrainNodeIdentInfo_request,
    .simple_train_node_ident_info_reply = &_ProtocolSimpleTrainNodeIdentInfo_reply,

    // Optional - Datagram
    .datagram = &_ProtocolDatagram_handle_datagram,
    .datagram_ok_reply = &_ProtocolDatagram_handle_datagram_ok_reply,
    .load_datagram_rejected = &_mock_load_datagram_rejected,
    .datagram_rejected_reply = &_ProtocolDatagram_handle_datagram_rejected_reply,

    // Optional - Stream
    .stream_initiate_request = &_ProtocolStream_initiate_request,
    .stream_initiate_reply = &_ProtocolStream_initiate_reply,
    .stream_send_data = &_ProtocolStream_send_data,
    .stream_data_proceed = &_ProtocolStream_data_proceed,
    .stream_data_complete = &_ProtocolStream_data_complete,

    // Optional - Broadcast Time
    .broadcast_time_event_handler = &_ProtocolBroadcastTime_handle_time_event,

    // Optional - Train Search & Emergency
    .train_search_event_handler = &_mock_train_search_event_handler,
    .train_search_no_match_handler = &_mock_train_search_no_match_handler,
    .train_emergency_event_handler = &_mock_train_emergency_event_handler,

    // Event Classification Filters
    .is_broadcast_time_event = &ProtocolBroadcastTime_is_time_event,
    .is_train_search_event = &ProtocolTrainSearch_is_search_event,
    .is_emergency_event = &ProtocolTrainHandler_is_emergency_event
};

/**
 * @brief Minimal interface with NULL optional handlers
 * @details Used to test automatic Interaction Rejected for unsupported protocols
 */
const interface_openlcb_main_statemachine_t interface_openlcb_main_statemachine_null_handlers = {

    // Resource Management (REQUIRED)
    .lock_shared_resources = &_ExampleDrivers_lock_shared_resources,
    .unlock_shared_resources = &_ExampleDrivers_unlock_shared_resources,
    .send_openlcb_msg = &_CanTxStatemachine_send_openlcb_message,
    .get_current_tick = &_mock_get_current_tick,

    // Node Enumeration (REQUIRED)
    .openlcb_node_get_first = &_OpenLcbNode_get_first,
    .openlcb_node_get_next = &_OpenLcbNode_get_next,
    .openlcb_node_is_last = &_OpenLcbNode_is_last,
    .openlcb_node_get_count = &_OpenLcbNode_get_count,

    // Core Handlers (REQUIRED)
    .load_interaction_rejected = &_OpenLcbUtilities_load_interaction_rejected,

    // Message Network - all NULL
    .message_network_initialization_complete = nullptr,
    .message_network_initialization_complete_simple = nullptr,
    .message_network_verify_node_id_addressed = nullptr,
    .message_network_verify_node_id_global = nullptr,
    .message_network_verified_node_id = nullptr,
    .message_network_optional_interaction_rejected = nullptr,
    .message_network_terminate_due_to_error = nullptr,
    .message_network_protocol_support_inquiry = nullptr,
    .message_network_protocol_support_reply = nullptr,

    // Internal functions
    .process_main_statemachine = _OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = _OpenLcbMainStatemachine_does_node_process_msg,
    .handle_outgoing_openlcb_message = &_OpenLcbMainStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &_OpenLcbMainStatemachine_handle_try_reenumerate,
    .handle_try_pop_next_incoming_openlcb_message = &_OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message,
    .handle_try_enumerate_first_node = &_OpenLcbMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &_OpenLcbMainStatemachine_handle_try_enumerate_next_node,

    // SNIP - NULL
    .snip_simple_node_info_request = nullptr,
    .snip_simple_node_info_reply = nullptr,

    // Event Transport - all NULL
    .event_transport_consumer_identify = nullptr,
    .event_transport_consumer_range_identified = nullptr,
    .event_transport_consumer_identified_unknown = nullptr,
    .event_transport_consumer_identified_set = nullptr,
    .event_transport_consumer_identified_clear = nullptr,
    .event_transport_consumer_identified_reserved = nullptr,
    .event_transport_producer_identify = nullptr,
    .event_transport_producer_range_identified = nullptr,
    .event_transport_producer_identified_unknown = nullptr,
    .event_transport_producer_identified_set = nullptr,
    .event_transport_producer_identified_clear = nullptr,
    .event_transport_producer_identified_reserved = nullptr,
    .event_transport_identify_dest = nullptr,
    .event_transport_identify = nullptr,
    .event_transport_learn = nullptr,
    .event_transport_pc_report = nullptr,
    .event_transport_pc_report_with_payload = nullptr,

    // Train - NULL
    .train_control_command = nullptr,
    .train_control_reply = nullptr,

    // Simple Train Node - NULL
    .simple_train_node_ident_info_request = nullptr,
    .simple_train_node_ident_info_reply = nullptr,

    // Datagram - NULL
    .datagram = nullptr,
    .datagram_ok_reply = nullptr,
    .datagram_rejected_reply = nullptr,

    // Stream - NULL
    .stream_initiate_request = nullptr,
    .stream_initiate_reply = nullptr,
    .stream_send_data = nullptr,
    .stream_data_proceed = nullptr,
    .stream_data_complete = nullptr
};

interface_openlcb_node_t interface_openlcb_node = {};

// ============================================================================
// Test Helpers
// ============================================================================

/**
 * @brief Reset all test control variables to default state
 */
void _reset_variables(void)
{
    called_function_ptr = nullptr;
    load_interaction_rejected_called = false;
    broadcast_time_handler_called = false;
    pc_event_report_handler_called = false;
    producer_identified_set_handler_called = false;
    reply_to_protocol_support_inquiry = false;
    force_process_statemachine_to_fail = false;
    send_openlcb_msg_called = false;
    process_statemachine_called = false;
    node_get_first_called = false;
    node_get_next_called = false;
    does_node_process_msg = false;
    node_get_first = nullptr;
    node_get_next = nullptr;
    mock_node_count = 2;
    allow_successful_transmit = true;
    force_true_does_node_process_msg = false;
    force_false_does_node_process_msg = false;
    
    // Reset fail flags for handle functions
    fail_handle_outgoing_openlcb_message = false;
    fail_handle_try_reenumerate = false;
    fail_handle_try_pop_next_incoming_openlcb_message = false;
    fail_handle_try_enumerate_first_node = false;
    fail_handle_try_enumerate_next_node = false;
    fifo_has_message = false;
    fifo_pop_called = false;
    mock_first_node = nullptr;
    mock_next_node = nullptr;
    
    // Reset handle function call tracking
    handle_outgoing_called = false;
    handle_reenumerate_called = false;
    handle_pop_called = false;
    handle_enumerate_first_called = false;
    handle_enumerate_next_called = false;

    // Reset train search / emergency / datagram tracking
    train_search_event_handler_called = false;
    train_search_no_match_handler_called = false;
    train_emergency_event_handler_called = false;
    load_datagram_rejected_called = false;
    force_is_last_node = false;
    train_search_handler_set_valid = false;
    memset(&_dummy_train_state, 0, sizeof(_dummy_train_state));
}

/**
 * @brief Initialize all required modules with full handlers
 */
void _global_initialize(void)
{
    _reset_variables();
    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
}

/**
 * @brief Initialize all required modules with NULL handlers
 */
void _global_initialize_null_handlers(void)
{
    _reset_variables();
    OpenLcbMainStatemachine_initialize(&interface_openlcb_main_statemachine_null_handlers);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
}

// ============================================================================
// TEST: Module Initialization
// ============================================================================

TEST(OpenLcbMainStatemachine, initialize)
{
    _global_initialize();

    // Test passes if no crashes occur
    // Internal state checked in subsequent tests
}

// ============================================================================
// TEST: does_node_process_msg - NULL node
// ============================================================================

TEST(OpenLcbMainStatemachine, does_node_process_msg_null_node)
{
    _global_initialize();

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = nullptr;

    bool result = OpenLcbMainStatemachine_does_node_process_msg(&statemachine_info);

    EXPECT_FALSE(result);
}

// ============================================================================
// TEST: does_node_process_msg - Node not initialized
// ============================================================================

TEST(OpenLcbMainStatemachine, does_node_process_msg_not_initialized)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = false;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;

    bool result = OpenLcbMainStatemachine_does_node_process_msg(&statemachine_info);

    EXPECT_FALSE(result);
}

// ============================================================================
// TEST: does_node_process_msg - Global message (no destination)
// ============================================================================

TEST(OpenLcbMainStatemachine, does_node_process_msg_global_message)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_UNKNOWN;  // Global message (no address bit)
    msg->dest_alias = 0;
    msg->dest_id = 0;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;

    bool result = OpenLcbMainStatemachine_does_node_process_msg(&statemachine_info);

    // Should process global messages
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: does_node_process_msg - Addressed to this node (alias match)
// ============================================================================

TEST(OpenLcbMainStatemachine, does_node_process_msg_addressed_alias_match)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFY_NODE_ID_ADDRESSED;  // Addressed message
    msg->dest_alias = 0xBBB;  // Matches node alias
    msg->dest_id = 0;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;

    bool result = OpenLcbMainStatemachine_does_node_process_msg(&statemachine_info);

    // Should process addressed message to us
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: does_node_process_msg - Addressed to this node (ID match)
// ============================================================================

TEST(OpenLcbMainStatemachine, does_node_process_msg_addressed_id_match)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFY_NODE_ID_ADDRESSED;
    msg->dest_alias = 0;
    msg->dest_id = 0x060504030201;  // Matches node ID

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;

    bool result = OpenLcbMainStatemachine_does_node_process_msg(&statemachine_info);

    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: does_node_process_msg - Addressed to different node
// ============================================================================

TEST(OpenLcbMainStatemachine, does_node_process_msg_addressed_different_node)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFY_NODE_ID_ADDRESSED;
    msg->dest_alias = 0xCCC;  // Different alias
    msg->dest_id = 0x999;     // Different ID

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;

    bool result = OpenLcbMainStatemachine_does_node_process_msg(&statemachine_info);

    // Should NOT process - addressed to different node
    EXPECT_FALSE(result);
}

// ============================================================================
// TEST: does_node_process_msg - Verify Node ID Global (special case)
// ============================================================================

TEST(OpenLcbMainStatemachine, does_node_process_msg_verify_node_id_global)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFY_NODE_ID_GLOBAL;  // Special case - always processed
    msg->dest_alias = 0;
    msg->dest_id = 0;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;

    bool result = OpenLcbMainStatemachine_does_node_process_msg(&statemachine_info);

    // Special case - should always process
    EXPECT_TRUE(result);
}

// ============================================================================
// TEST: process_main_statemachine - SNIP Request with handler
// ============================================================================

TEST(OpenLcbMainStatemachine, process_snip_request_with_handler)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_NODE_INFO_REQUEST;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    // Reset before calling
    called_function_ptr = nullptr;
    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Verify SNIP handler was called - check that called_function_ptr is non-null
    // (accumulated from wrapper functions + handler)
    EXPECT_NE(called_function_ptr, nullptr);
    
    // Verify Interaction Rejected was NOT called (handler exists)
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: process_main_statemachine - SNIP Request with NULL handler
// ============================================================================

TEST(OpenLcbMainStatemachine, process_snip_request_null_handler)
{
    _global_initialize_null_handlers();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_NODE_INFO_REQUEST;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    // Reset flags
    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Verify Interaction Rejected was called (NULL handler)
    EXPECT_TRUE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: load_interaction_rejected - NULL statemachine_info
// ============================================================================

TEST(OpenLcbMainStatemachine, load_interaction_rejected_null_statemachine_info)
{
    _global_initialize();

    // Call with NULL - should return early without crash
    OpenLcbMainStatemachine_load_interaction_rejected(nullptr);

    // Test passes if no crash
}

// ============================================================================
// TEST: load_interaction_rejected - NULL openlcb_node
// ============================================================================

TEST(OpenLcbMainStatemachine, load_interaction_rejected_null_node)
{
    _global_initialize();

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = nullptr;
    statemachine_info.incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    // Should return early without crash
    OpenLcbMainStatemachine_load_interaction_rejected(&statemachine_info);

    // Test passes if no crash
}

// ============================================================================
// TEST: load_interaction_rejected - NULL outgoing message
// ============================================================================

TEST(OpenLcbMainStatemachine, load_interaction_rejected_null_outgoing_msg)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
    statemachine_info.outgoing_msg_info.msg_ptr = nullptr;

    // Should return early without crash
    OpenLcbMainStatemachine_load_interaction_rejected(&statemachine_info);

    // Test passes if no crash
}

// ============================================================================
// TEST: load_interaction_rejected - NULL incoming message
// ============================================================================

TEST(OpenLcbMainStatemachine, load_interaction_rejected_null_incoming_msg)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = nullptr;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    // Should return early without crash
    OpenLcbMainStatemachine_load_interaction_rejected(&statemachine_info);

    // Test passes if no crash
}

// ============================================================================
// TEST: load_interaction_rejected - Valid call
// ============================================================================

TEST(OpenLcbMainStatemachine, load_interaction_rejected_valid)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->alias = 0xBBB;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming_msg->mti = MTI_SIMPLE_NODE_INFO_REQUEST;
    incoming_msg->source_alias = 0x222;
    incoming_msg->source_id = 0x010203040506;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbMainStatemachine_load_interaction_rejected(&statemachine_info);

    // Verify outgoing message was created
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_OPTIONAL_INTERACTION_REJECTED);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->dest_alias, 0x222);
}

// ============================================================================
// TEST: process_main_statemachine - NULL statemachine_info
// ============================================================================

TEST(OpenLcbMainStatemachine, process_null_statemachine_info)
{
    _global_initialize();

    // Should return early without crash
    OpenLcbMainStatemachine_process_main_statemachine(nullptr);

    // Test passes if no crash
}

// DUE TO FILE SIZE CONSTRAINTS, I'LL PROVIDE THE PATTERN FOR ADDITIONAL TESTS
// THESE SHOULD BE ADDED ONE AT A TIME AS COMMENTED-OUT SECTIONS

// ============================================================================
// TEST: process_main_statemachine - Protocol Support Inquiry
// ============================================================================

TEST(OpenLcbMainStatemachine, process_protocol_support_inquiry)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PROTOCOL_SUPPORT_INQUIRY;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    // Reset before calling
    called_function_ptr = nullptr;
    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Verify handler was called
    EXPECT_NE(called_function_ptr, nullptr);
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: handle_outgoing_message - Message pending, send succeeds
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_outgoing_message_send_succeeds)
{
    _global_initialize();

    // Single node — no sibling dispatch, valid clears immediately
    mock_node_count = 1;

    // Get access to internal state
    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    // Set up a valid outgoing message
    state->outgoing_msg_info.valid = true;
    allow_successful_transmit = true;
    send_openlcb_msg_called = false;

    bool result = OpenLcbMainStatemachine_handle_outgoing_openlcb_message();

    EXPECT_TRUE(result);
    EXPECT_TRUE(send_openlcb_msg_called);
    EXPECT_FALSE(state->outgoing_msg_info.valid);  // Should be cleared after successful send
}

// ============================================================================
// TEST: handle_outgoing_message - Message pending, send fails
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_outgoing_message_send_fails)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    // Set up a valid outgoing message but transmission will fail
    state->outgoing_msg_info.valid = true;
    allow_successful_transmit = false;
    send_openlcb_msg_called = false;

    bool result = OpenLcbMainStatemachine_handle_outgoing_openlcb_message();

    EXPECT_TRUE(result);  // Still returns true (message pending)
    EXPECT_TRUE(send_openlcb_msg_called);
    EXPECT_TRUE(state->outgoing_msg_info.valid);  // Should still be valid (retry needed)
}

// ============================================================================
// TEST: handle_outgoing_message - No message pending
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_outgoing_message_no_message)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    state->outgoing_msg_info.valid = false;
    send_openlcb_msg_called = false;

    bool result = OpenLcbMainStatemachine_handle_outgoing_openlcb_message();

    EXPECT_FALSE(result);
    EXPECT_FALSE(send_openlcb_msg_called);
}

// ============================================================================
// TEST: handle_try_reenumerate - Enumerate flag set
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_reenumerate_flag_set)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    state->incoming_msg_info.enumerate = true;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_reenumerate();

    EXPECT_TRUE(result);
    EXPECT_TRUE(process_statemachine_called);
}

// ============================================================================
// TEST: handle_try_reenumerate - Enumerate flag clear
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_reenumerate_flag_clear)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    state->incoming_msg_info.enumerate = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_reenumerate();

    EXPECT_FALSE(result);
    EXPECT_FALSE(process_statemachine_called);
}

// ============================================================================
// TEST: handle_try_pop - Message already present
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_pop_message_already_present)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    // Set incoming message pointer (already have a message)
    state->incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);

    bool result = OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message();

    EXPECT_FALSE(result);  // Already have message, returns false
}

// ============================================================================
// TEST: handle_try_pop - No current message, FIFO has message (successful pop)
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_pop_fifo_success)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    // No current message
    state->incoming_msg_info.msg_ptr = nullptr;
    
    // Put an actual message in the FIFO
    openlcb_msg_t *fifo_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(fifo_msg, nullptr);
    fifo_msg->mti = MTI_VERIFIED_NODE_ID;
    OpenLcbBufferFifo_push(fifo_msg);

    bool result = OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message();

    // Should pop message successfully and return false (got message, continue processing)
    EXPECT_FALSE(result);
    EXPECT_NE(state->incoming_msg_info.msg_ptr, nullptr);  // Should have message now
    EXPECT_EQ(state->incoming_msg_info.msg_ptr, fifo_msg);  // Should be the same message
}

// ============================================================================
// TEST: handle_try_pop - No current message, FIFO empty (unsuccessful pop)
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_pop_fifo_empty)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    // No current message
    state->incoming_msg_info.msg_ptr = nullptr;
    
    // FIFO is empty (nothing pushed)
    // OpenLcbBufferFifo was initialized in _global_initialize(), so it's empty

    bool result = OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message();

    // Should fail to pop and return true (no message, stop processing)
    EXPECT_TRUE(result);
    EXPECT_EQ(state->incoming_msg_info.msg_ptr, nullptr);  // Still no message
}

// ============================================================================
// TEST: handle_try_pop - Invalid message discarded immediately
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_pop_invalid_message_discarded)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    // No current message
    state->incoming_msg_info.msg_ptr = nullptr;

    // Push an invalid message into the FIFO
    openlcb_msg_t *invalid_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(invalid_msg, nullptr);
    invalid_msg->mti = MTI_VERIFIED_NODE_ID;
    invalid_msg->state.invalid = true;
    OpenLcbBufferFifo_push(invalid_msg);

    bool result = OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message();

    // Should discard invalid message and return true (idle, try again next iteration)
    EXPECT_TRUE(result);
    EXPECT_EQ(state->incoming_msg_info.msg_ptr, nullptr);

    // Buffer should have been freed
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: handle_try_pop - Non-invalid message processed normally
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_pop_valid_message_not_discarded)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();

    // No current message
    state->incoming_msg_info.msg_ptr = nullptr;

    // Push a valid (non-invalid) message into the FIFO
    openlcb_msg_t *valid_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(valid_msg, nullptr);
    valid_msg->mti = MTI_VERIFIED_NODE_ID;
    valid_msg->state.invalid = false;
    OpenLcbBufferFifo_push(valid_msg);

    bool result = OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message();

    // Should pop message normally (returns false = got a message)
    EXPECT_FALSE(result);
    EXPECT_NE(state->incoming_msg_info.msg_ptr, nullptr);
    EXPECT_EQ(state->incoming_msg_info.msg_ptr, valid_msg);
    EXPECT_EQ(state->incoming_msg_info.msg_ptr->state.invalid, false);
}

// ============================================================================
// TEST: handle_try_enumerate_first_node - Node available in RUN state
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_enumerate_first_node_run_state)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // Clear node pointer
    state->openlcb_node = nullptr;
    
    // Set up a node to be returned
    openlcb_node_t *test_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    test_node->state.run_state = RUNSTATE_RUN;
    node_get_first = test_node;
    node_get_first_called = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_enumerate_first_node();

    EXPECT_TRUE(result);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_TRUE(process_statemachine_called);
    EXPECT_EQ(state->openlcb_node, test_node);
}

// ============================================================================
// TEST: handle_try_enumerate_first_node - Node NOT in RUN state (skip processing)
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_enumerate_first_node_not_run_state)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // Clear node pointer
    state->openlcb_node = nullptr;
    
    // Set up a node NOT in RUN state
    openlcb_node_t *test_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    test_node->state.run_state = RUNSTATE_INIT;  // Not in RUN state
    node_get_first = test_node;
    node_get_first_called = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_enumerate_first_node();

    // Should still return true and set node, but NOT process message
    EXPECT_TRUE(result);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_FALSE(process_statemachine_called);  // Should NOT process
    EXPECT_EQ(state->openlcb_node, test_node);
}

// ============================================================================
// TEST: handle_try_enumerate_first_node - No nodes allocated (frees message)
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_enumerate_first_node_no_nodes)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // Clear node pointer
    state->openlcb_node = nullptr;
    
    // Set up an incoming message that needs to be freed
    state->incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(state->incoming_msg_info.msg_ptr, nullptr);
    state->incoming_msg_info.msg_ptr->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    
    // No nodes exist - get_first will return NULL
    node_get_first = nullptr;
    node_get_first_called = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_enumerate_first_node();

    // Should return true (done), free the message
    EXPECT_TRUE(result);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_FALSE(process_statemachine_called);
    EXPECT_EQ(state->openlcb_node, nullptr);  // Still no node
    EXPECT_EQ(state->incoming_msg_info.msg_ptr, nullptr);  // Message should be freed
}

// ============================================================================
// TEST: handle_try_enumerate_first_node - Already have node (safety exit)
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_enumerate_first_node_already_have_node)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // ALREADY have a node set (safety condition)
    openlcb_node_t *existing_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    state->openlcb_node = existing_node;
    
    node_get_first_called = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_enumerate_first_node();

    // Should return false immediately (not applicable - already have node)
    EXPECT_FALSE(result);
    EXPECT_FALSE(node_get_first_called);  // Should NOT try to get first
    EXPECT_FALSE(process_statemachine_called);
    EXPECT_EQ(state->openlcb_node, existing_node);  // Node unchanged
}

// ============================================================================
// TEST: handle_try_enumerate_next_node - Node available in RUN state
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_enumerate_next_node_run_state)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // Set up current node
    openlcb_node_t *current_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    state->openlcb_node = current_node;
    
    // Set up next node to be returned
    openlcb_node_t *next_node = OpenLcbNode_allocate(0x070605040302, &_node_parameters_main_node);
    next_node->state.run_state = RUNSTATE_RUN;
    node_get_next = next_node;
    node_get_next_called = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_enumerate_next_node();

    EXPECT_TRUE(result);
    EXPECT_TRUE(node_get_next_called);
    EXPECT_TRUE(process_statemachine_called);
    EXPECT_EQ(state->openlcb_node, next_node);
}

// ============================================================================
// TEST: handle_try_enumerate_next_node - Node NOT in RUN state (skip processing)
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_enumerate_next_node_not_run_state)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // Set up current node
    openlcb_node_t *current_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    state->openlcb_node = current_node;
    
    // Set up next node NOT in RUN state
    openlcb_node_t *next_node = OpenLcbNode_allocate(0x070605040302, &_node_parameters_main_node);
    next_node->state.run_state = RUNSTATE_INIT;  // Not in RUN state
    node_get_next = next_node;
    node_get_next_called = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_enumerate_next_node();

    // Should still return true and set node, but NOT process message
    EXPECT_TRUE(result);
    EXPECT_TRUE(node_get_next_called);
    EXPECT_FALSE(process_statemachine_called);  // Should NOT process
    EXPECT_EQ(state->openlcb_node, next_node);
}

// ============================================================================
// TEST: handle_try_enumerate_next_node - End of list (frees message)
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_enumerate_next_node_end_of_list)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // Set up current node
    openlcb_node_t *current_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    state->openlcb_node = current_node;
    
    // Set up an incoming message that needs to be freed
    state->incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(state->incoming_msg_info.msg_ptr, nullptr);
    state->incoming_msg_info.msg_ptr->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    
    // No more nodes - get_next returns NULL (end of list)
    node_get_next = nullptr;
    node_get_next_called = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_enumerate_next_node();

    // Should return true (done), free the message
    EXPECT_TRUE(result);
    EXPECT_TRUE(node_get_next_called);
    EXPECT_FALSE(process_statemachine_called);
    EXPECT_EQ(state->openlcb_node, nullptr);  // Node cleared (end of list)
    EXPECT_EQ(state->incoming_msg_info.msg_ptr, nullptr);  // Message should be freed
}

// ============================================================================
// TEST: handle_try_enumerate_next_node - No current node (safety exit)
// ============================================================================

TEST(OpenLcbMainStatemachine, handle_enumerate_next_node_no_current_node)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // No current node - safety condition
    state->openlcb_node = nullptr;
    
    node_get_next_called = false;
    process_statemachine_called = false;

    bool result = OpenLcbMainStatemachine_handle_try_enumerate_next_node();

    // Should return false immediately (not applicable - no current node)
    EXPECT_FALSE(result);
    EXPECT_FALSE(node_get_next_called);  // Should not try to get next
    EXPECT_FALSE(process_statemachine_called);
}

// ============================================================================
// TEST: _free_incoming_message - Called with NULL msg_ptr (safety check)
// ============================================================================

TEST(OpenLcbMainStatemachine, free_message_with_null_ptr)
{
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    
    // Clear node pointer
    state->openlcb_node = nullptr;
    
    // No incoming message (msg_ptr is NULL)
    state->incoming_msg_info.msg_ptr = nullptr;
    
    // No nodes exist - will call _free_incoming_message with NULL msg_ptr
    node_get_first = nullptr;
    node_get_first_called = false;

    // This should NOT crash - _free_incoming_message checks for NULL
    bool result = OpenLcbMainStatemachine_handle_try_enumerate_first_node();

    // Should return true, not crash
    EXPECT_TRUE(result);
    EXPECT_TRUE(node_get_first_called);
    EXPECT_EQ(state->incoming_msg_info.msg_ptr, nullptr);  // Still NULL
}

// ============================================================================
// TEST: Main Run Loop - Comprehensive Priority and Flow Testing
// ============================================================================

TEST(OpenLcbMainStatemachine, run_comprehensive)
{
    // ************************************************************************
    //  Test 1: Outgoing message has highest priority
    // ************************************************************************

    _reset_variables();
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    state->outgoing_msg_info.valid = true;
    state->outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
    allow_successful_transmit = false;  // Keep message pending so handle returns true

    OpenLcbMainStatemachine_run();

    // Only outgoing handler should be called (returns true, stops cascade)
    EXPECT_TRUE(handle_outgoing_called);
    EXPECT_FALSE(handle_reenumerate_called);
    EXPECT_FALSE(handle_pop_called);
    EXPECT_FALSE(handle_enumerate_first_called);
    EXPECT_FALSE(handle_enumerate_next_called);

    // ************************************************************************
    //  Test 2: Re-enumerate has second priority
    // ************************************************************************

    _reset_variables();
    _global_initialize();

    state = OpenLcbMainStatemachine_get_statemachine_info();
    state->outgoing_msg_info.valid = false;  // No outgoing message
    state->incoming_msg_info.enumerate = true;  // Need to re-enumerate
    state->incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
    state->openlcb_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    state->openlcb_node->state.run_state = RUNSTATE_RUN;

    OpenLcbMainStatemachine_run();

    // Outgoing and re-enumerate should be called (re-enumerate returns true, stops cascade)
    EXPECT_TRUE(handle_outgoing_called);
    EXPECT_TRUE(handle_reenumerate_called);
    EXPECT_FALSE(handle_pop_called);
    EXPECT_FALSE(handle_enumerate_first_called);
    EXPECT_FALSE(handle_enumerate_next_called);

    // ************************************************************************
    //  Test 3: Pop message has third priority
    // ************************************************************************

    _reset_variables();
    _global_initialize();

    state = OpenLcbMainStatemachine_get_statemachine_info();
    state->outgoing_msg_info.valid = false;
    state->incoming_msg_info.enumerate = false;
    state->incoming_msg_info.msg_ptr = nullptr;  // No current message
    state->openlcb_node = nullptr;
    fifo_has_message = true;  // Simulate FIFO has a message

    OpenLcbMainStatemachine_run();

    // Pop returns false (got message), so cascade continues to enumerate_first
    EXPECT_TRUE(handle_outgoing_called);
    EXPECT_TRUE(handle_reenumerate_called);
    EXPECT_TRUE(handle_pop_called);
    EXPECT_TRUE(handle_enumerate_first_called);  // Continues to enumerate
    EXPECT_FALSE(handle_enumerate_next_called);  // No current node, so enumerate_first returns true
    EXPECT_TRUE(fifo_pop_called);

    // ************************************************************************
    //  Test 4: Enumerate first node has fourth priority
    // ************************************************************************

    _reset_variables();
    _global_initialize();

    openlcb_node_t *test_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    test_node->state.run_state = RUNSTATE_RUN;
    node_get_first = test_node;

    state = OpenLcbMainStatemachine_get_statemachine_info();
    state->outgoing_msg_info.valid = false;
    state->incoming_msg_info.enumerate = false;
    state->incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
    state->openlcb_node = nullptr;  // No current node
    fifo_has_message = false;

    OpenLcbMainStatemachine_run();

    // All up to enumerate first should be called (enumerate_first returns true, stops)
    EXPECT_TRUE(handle_outgoing_called);
    EXPECT_TRUE(handle_reenumerate_called);
    EXPECT_TRUE(handle_pop_called);  // Returns true (no FIFO message)
    EXPECT_TRUE(handle_enumerate_first_called);
    EXPECT_FALSE(handle_enumerate_next_called);

    // ************************************************************************
    //  Test 5: Enumerate next node has fifth priority
    // ************************************************************************

    _reset_variables();
    _global_initialize();

    openlcb_node_t *next_test_node = OpenLcbNode_allocate(0x070605040302, &_node_parameters_main_node);
    next_test_node->state.run_state = RUNSTATE_RUN;
    node_get_next = next_test_node;

    state = OpenLcbMainStatemachine_get_statemachine_info();
    state->outgoing_msg_info.valid = false;
    state->incoming_msg_info.enumerate = false;
    state->incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);
    state->openlcb_node = test_node;  // Already have a node
    fifo_has_message = false;

    OpenLcbMainStatemachine_run();

    // All handlers should be called (enumerate_next returns true, stops)
    EXPECT_TRUE(handle_outgoing_called);
    EXPECT_TRUE(handle_reenumerate_called);
    EXPECT_TRUE(handle_pop_called);  // Returns true (no FIFO message)
    EXPECT_TRUE(handle_enumerate_first_called);  // Returns false (already have node)
    EXPECT_TRUE(handle_enumerate_next_called);
}

// ============================================================================
// TEST: Main Run Loop - Pop returns true (FIFO empty, stops cascade)
// ============================================================================

TEST(OpenLcbMainStatemachine, run_pop_returns_true)
{
    _reset_variables();
    _global_initialize();

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    state->outgoing_msg_info.valid = false;  // No outgoing message
    state->incoming_msg_info.enumerate = false;  // No re-enumerate
    state->incoming_msg_info.msg_ptr = nullptr;  // No current message
    state->openlcb_node = nullptr;  // No current node
    fifo_has_message = false;  // FIFO is empty

    // Set up fail flags - we want pop to call the REAL function which returns true (FIFO empty)
    fail_handle_outgoing_openlcb_message = false;
    fail_handle_try_reenumerate = false;
    fail_handle_try_pop_next_incoming_openlcb_message = false;  // Call real function
    fail_handle_try_enumerate_first_node = false;
    fail_handle_try_enumerate_next_node = false;

    OpenLcbMainStatemachine_run();

    // Pop should return true (FIFO empty), stopping cascade before enumerate
    EXPECT_TRUE(handle_outgoing_called);
    EXPECT_TRUE(handle_reenumerate_called);
    EXPECT_TRUE(handle_pop_called);
    EXPECT_FALSE(handle_enumerate_first_called);  // Should NOT be called (cascade stopped)
    EXPECT_FALSE(handle_enumerate_next_called);
}

// ============================================================================
// TEST: Main Run Loop - Enumerate next returns true (stops cascade)
// ============================================================================

TEST(OpenLcbMainStatemachine, run_enumerate_next_returns_true)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *current_node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    current_node->state.run_state = RUNSTATE_RUN;
    
    openlcb_node_t *next_node = OpenLcbNode_allocate(0x070605040302, &_node_parameters_main_node);
    next_node->state.run_state = RUNSTATE_RUN;
    node_get_next = next_node;

    openlcb_statemachine_info_t *state = OpenLcbMainStatemachine_get_statemachine_info();
    state->outgoing_msg_info.valid = false;  // No outgoing message
    state->incoming_msg_info.enumerate = false;  // No re-enumerate
    state->incoming_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(BASIC);  // Have message
    state->openlcb_node = current_node;  // Already have a node
    fifo_has_message = false;

    OpenLcbMainStatemachine_run();

    // Enumerate_next should return true (has node, processes next), stopping cascade
    EXPECT_TRUE(handle_outgoing_called);
    EXPECT_TRUE(handle_reenumerate_called);
    EXPECT_TRUE(handle_pop_called);  // Returns false (already have message)
    EXPECT_TRUE(handle_enumerate_first_called);  // Returns false (already have node)
    EXPECT_TRUE(handle_enumerate_next_called);  // Returns true (processes node), STOPS
}

// ============================================================================
// ADDITIONAL MTI COVERAGE TESTS - NOW ACTIVE
// ============================================================================

// ============================================================================
// TEST: Message Network - Initialization Complete
// ============================================================================

TEST(OpenLcbMainStatemachine, process_initialization_complete)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_INITIALIZATION_COMPLETE;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;
    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: Message Network - Verify Node ID Addressed
// ============================================================================

TEST(OpenLcbMainStatemachine, process_verify_node_id_addressed)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFY_NODE_ID_ADDRESSED;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Event Transport - Consumer Identify
// ============================================================================

TEST(OpenLcbMainStatemachine, process_consumer_identify)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_CONSUMER_IDENTIFY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Event Transport - Producer Identify
// ============================================================================

TEST(OpenLcbMainStatemachine, process_producer_identify)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Train - Command with handler
// ============================================================================

TEST(OpenLcbMainStatemachine, process_train_command_with_handler)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_TRAIN_PROTOCOL;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;
    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: Train - Command with NULL handler (should reject)
// ============================================================================

TEST(OpenLcbMainStatemachine, process_train_command_null_handler)
{
    _global_initialize_null_handlers();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_TRAIN_PROTOCOL;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_TRUE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: Datagram Protocol
// ============================================================================

TEST(OpenLcbMainStatemachine, process_datagram)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_DATAGRAM;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Stream Protocol - Init Request
// ============================================================================

TEST(OpenLcbMainStatemachine, process_stream_init_request)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_INIT_REQUEST;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Default case - Unknown MTI addressed to node
// ============================================================================

TEST(OpenLcbMainStatemachine, process_unknown_mti_addressed)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = 0xFFFF;  // Unknown MTI
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Should call interaction rejected for unknown addressed MTI
    EXPECT_TRUE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: Default case - Unknown MTI global (no action)
// ============================================================================

TEST(OpenLcbMainStatemachine, process_unknown_mti_global)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;  // Node alias is 0xBBB

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = 0xFFF7;  // Unknown MTI without address bit (bit 3 = 0) = global
    msg->dest_alias = 0xCCC;  // Different from node alias
    msg->dest_id = 0x070605040302;  // Different from node ID

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Should NOT call interaction rejected for unknown global MTI
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: Reply MTI with NULL handler (no rejection)
// ============================================================================

TEST(OpenLcbMainStatemachine, process_reply_mti_null_handler_no_reject)
{
    _global_initialize_null_handlers();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_NODE_INFO_REPLY;  // Reply MTI
    msg->dest_alias = 0;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Reply MTIs should NOT trigger interaction rejected even if handler is NULL
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: All Event Transport MTIs
// ============================================================================

TEST(OpenLcbMainStatemachine, process_all_event_transport_mtis)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    uint16_t event_mtis[] = {
        MTI_CONSUMER_IDENTIFY,
        MTI_CONSUMER_RANGE_IDENTIFIED,
        MTI_CONSUMER_IDENTIFIED_UNKNOWN,
        MTI_CONSUMER_IDENTIFIED_SET,
        MTI_CONSUMER_IDENTIFIED_CLEAR,
        MTI_CONSUMER_IDENTIFIED_RESERVED,
        MTI_PRODUCER_IDENTIFY,
        MTI_PRODUCER_RANGE_IDENTIFIED,
        MTI_PRODUCER_IDENTIFIED_UNKNOWN,
        MTI_PRODUCER_IDENTIFIED_SET,
        MTI_PRODUCER_IDENTIFIED_CLEAR,
        MTI_PRODUCER_IDENTIFIED_RESERVED,
        MTI_EVENTS_IDENTIFY_DEST,
        MTI_EVENTS_IDENTIFY,
        MTI_EVENT_LEARN,
        MTI_PC_EVENT_REPORT,
        MTI_PC_EVENT_REPORT_WITH_PAYLOAD
    };

    for (unsigned int i = 0; i < sizeof(event_mtis) / sizeof(event_mtis[0]); i++)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
        msg->mti = event_mtis[i];

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node;
        statemachine_info.incoming_msg_info.msg_ptr = msg;
        statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

        called_function_ptr = nullptr;

        OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

        // Each should call its handler
        EXPECT_NE(called_function_ptr, nullptr) << "Failed for MTI: 0x" << std::hex << event_mtis[i];
    }
}

// ============================================================================
// ADDITIONAL MTI COVERAGE - Message Network Protocol
// ============================================================================

// ============================================================================
// TEST: Message Network - Initialization Complete Simple
// ============================================================================

TEST(OpenLcbMainStatemachine, process_initialization_complete_simple)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Message Network - Protocol Support Reply
// ============================================================================

TEST(OpenLcbMainStatemachine, process_protocol_support_reply)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PROTOCOL_SUPPORT_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Message Network - Verify Node ID Global
// ============================================================================

TEST(OpenLcbMainStatemachine, process_verify_node_id_global)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFY_NODE_ID_GLOBAL;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Message Network - Verified Node ID
// ============================================================================

TEST(OpenLcbMainStatemachine, process_verified_node_id)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFIED_NODE_ID;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

TEST(OpenLcbMainStatemachine, process_verified_node_id_simple)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFIED_NODE_ID_SIMPLE;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Message Network - Optional Interaction Rejected
// ============================================================================

TEST(OpenLcbMainStatemachine, process_optional_interaction_rejected)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_OPTIONAL_INTERACTION_REJECTED;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Message Network - Terminate Due to Error
// ============================================================================

TEST(OpenLcbMainStatemachine, process_terminate_due_to_error)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_TERMINATE_DUE_TO_ERROR;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// ADDITIONAL MTI COVERAGE - Train Protocol
// ============================================================================

// ============================================================================
// TEST: Train - Reply
// ============================================================================

TEST(OpenLcbMainStatemachine, process_train_reply)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_TRAIN_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Train - Simple Train Info Request
// ============================================================================

TEST(OpenLcbMainStatemachine, process_simple_train_info_request)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_TRAIN_INFO_REQUEST;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;
    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: Train - Simple Train Info Request with NULL Handler (should reject)
// ============================================================================

TEST(OpenLcbMainStatemachine, process_simple_train_info_request_null_handler)
{
    _global_initialize_null_handlers();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_TRAIN_INFO_REQUEST;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Should call interaction rejected when handler is NULL for request MTI
    EXPECT_TRUE(load_interaction_rejected_called);
}

// ============================================================================
// TEST: Train - Simple Train Info Reply
// ============================================================================

TEST(OpenLcbMainStatemachine, process_simple_train_info_reply)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_TRAIN_INFO_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// ADDITIONAL MTI COVERAGE - Datagram Protocol
// ============================================================================

// ============================================================================
// TEST: Datagram - OK Reply
// ============================================================================

TEST(OpenLcbMainStatemachine, process_datagram_ok_reply)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_DATAGRAM_OK_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Datagram - Rejected Reply
// ============================================================================

TEST(OpenLcbMainStatemachine, process_datagram_rejected_reply)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_DATAGRAM_REJECTED_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// ADDITIONAL MTI COVERAGE - Stream Protocol
// ============================================================================

// ============================================================================
// TEST: Stream - Init Reply
// ============================================================================

TEST(OpenLcbMainStatemachine, process_stream_init_reply)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_INIT_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Stream - Send
// ============================================================================

TEST(OpenLcbMainStatemachine, process_stream_send)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_SEND;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Stream - Proceed
// ============================================================================

TEST(OpenLcbMainStatemachine, process_stream_proceed)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_PROCEED;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// TEST: Stream - Complete
// ============================================================================

TEST(OpenLcbMainStatemachine, process_stream_complete)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_COMPLETE;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
}

// ============================================================================
// ADDITIONAL MTI COVERAGE - SNIP Protocol
// ============================================================================

// ============================================================================
// TEST: SNIP - Simple Node Info Reply with Handler
// ============================================================================

TEST(OpenLcbMainStatemachine, process_simple_node_info_reply_with_handler)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_NODE_INFO_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;
    load_interaction_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_NE(called_function_ptr, nullptr);
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// NULL HANDLER TESTS - ALL MTI CASES
// Tests that all MTI cases handle NULL dependency injection gracefully
// ============================================================================

// ----------------------------------------------------------------------------
// Message Network - NULL Handler Tests
// ----------------------------------------------------------------------------

TEST(OpenLcbMainStatemachine, null_handler_initialization_complete)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_INITIALIZATION_COMPLETE;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    // Informational message - no rejection
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_initialization_complete_simple)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_protocol_support_inquiry)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PROTOCOL_SUPPORT_INQUIRY;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_protocol_support_reply)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PROTOCOL_SUPPORT_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_verify_node_id_addressed)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFY_NODE_ID_ADDRESSED;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_verify_node_id_global)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFY_NODE_ID_GLOBAL;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_verified_node_id)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFIED_NODE_ID;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_verified_node_id_simple)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFIED_NODE_ID_SIMPLE;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_optional_interaction_rejected)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_OPTIONAL_INTERACTION_REJECTED;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_terminate_due_to_error)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_TERMINATE_DUE_TO_ERROR;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ----------------------------------------------------------------------------
// Event Transport - NULL Handler Tests  
// ----------------------------------------------------------------------------

TEST(OpenLcbMainStatemachine, null_handler_consumer_identify)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_CONSUMER_IDENTIFY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_consumer_range_identified)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_CONSUMER_RANGE_IDENTIFIED;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_consumer_identified_unknown)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_CONSUMER_IDENTIFIED_UNKNOWN;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_consumer_identified_set)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_CONSUMER_IDENTIFIED_SET;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_consumer_identified_clear)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_CONSUMER_IDENTIFIED_CLEAR;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_consumer_identified_reserved)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_CONSUMER_IDENTIFIED_RESERVED;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_producer_identify)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_producer_range_identified)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_RANGE_IDENTIFIED;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_producer_identified_unknown)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_UNKNOWN;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_producer_identified_set)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_SET;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_producer_identified_clear)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_CLEAR;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_producer_identified_reserved)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_RESERVED;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_events_identify_dest)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_EVENTS_IDENTIFY_DEST;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_events_identify)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_EVENTS_IDENTIFY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_event_learn)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_EVENT_LEARN;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_pc_event_report)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PC_EVENT_REPORT;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_pc_event_report_with_payload)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PC_EVENT_REPORT_WITH_PAYLOAD;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ----------------------------------------------------------------------------
// SNIP - NULL Handler Tests  
// ----------------------------------------------------------------------------

TEST(OpenLcbMainStatemachine, null_handler_simple_node_info_reply)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_NODE_INFO_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    // Reply message - no rejection
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ----------------------------------------------------------------------------
// Train - NULL Handler Tests
// ----------------------------------------------------------------------------

TEST(OpenLcbMainStatemachine, null_handler_train_reply)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_TRAIN_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_simple_train_info_reply)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_SIMPLE_TRAIN_INFO_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ----------------------------------------------------------------------------
// Datagram - NULL Handler Tests
// ----------------------------------------------------------------------------

TEST(OpenLcbMainStatemachine, null_handler_datagram)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_DATAGRAM;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_datagram_ok_reply)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_DATAGRAM_OK_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_datagram_rejected_reply)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_DATAGRAM_REJECTED_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ----------------------------------------------------------------------------
// Stream - NULL Handler Tests
// ----------------------------------------------------------------------------

TEST(OpenLcbMainStatemachine, null_handler_stream_init_request)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_INIT_REQUEST;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_TRUE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_stream_init_reply)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_INIT_REPLY;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_stream_send)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_SEND;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_TRUE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_stream_proceed)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_PROCEED;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);
    
    EXPECT_FALSE(load_interaction_rejected_called);
}

TEST(OpenLcbMainStatemachine, null_handler_stream_complete)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_STREAM_COMPLETE;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_TRUE(load_interaction_rejected_called);
}

// ----------------------------------------------------------------------------
// Broadcast Time - Routing Tests
// ----------------------------------------------------------------------------

TEST(OpenLcbMainStatemachine, broadcast_time_event_calls_handler)
{
    _global_initialize();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PC_EVENT_REPORT;

    // Load a broadcast time event ID into the payload (default fast clock, 14:30)
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg,
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Should have called broadcast_time_event_handler, NOT event_transport_pc_report
    EXPECT_TRUE(broadcast_time_handler_called);
    EXPECT_FALSE(pc_event_report_handler_called);
}

TEST(OpenLcbMainStatemachine, non_broadcast_time_event_falls_through)
{
    _global_initialize();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PC_EVENT_REPORT;

    // Load a non-broadcast-time event ID into the payload
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, 0x0505050505050000ULL);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Should have called event_transport_pc_report, NOT broadcast_time_event_handler
    EXPECT_FALSE(broadcast_time_handler_called);
    EXPECT_TRUE(pc_event_report_handler_called);
}

TEST(OpenLcbMainStatemachine, broadcast_time_null_handler_falls_through)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PC_EVENT_REPORT;

    // Load a broadcast time event ID — but null handlers mean no handler set
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg,
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // With null handlers, no rejection should happen (events don't generate rejections)
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ----------------------------------------------------------------------------
// Broadcast Time via MTI_PRODUCER_IDENTIFIED_SET - Routing Tests
// ----------------------------------------------------------------------------

TEST(OpenLcbMainStatemachine, broadcast_time_pid_set_calls_handler)
{
    _global_initialize();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_SET;

    // Load a broadcast time event ID into the payload (default fast clock, 14:30)
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg,
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Should have called broadcast_time_event_handler, NOT event_transport_producer_identified_set
    EXPECT_TRUE(broadcast_time_handler_called);
    EXPECT_FALSE(producer_identified_set_handler_called);
}

TEST(OpenLcbMainStatemachine, non_broadcast_time_pid_set_falls_through)
{
    _global_initialize();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_SET;

    // Load a non-broadcast-time event ID into the payload
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, 0x0505050505050000ULL);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    called_function_ptr = nullptr;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Should have called event_transport_producer_identified_set, NOT broadcast_time_event_handler
    EXPECT_FALSE(broadcast_time_handler_called);
    EXPECT_TRUE(producer_identified_set_handler_called);
}

TEST(OpenLcbMainStatemachine, broadcast_time_pid_set_null_handler_falls_through)
{
    _global_initialize_null_handlers();
    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_SET;

    // Load a broadcast time event ID — but null handlers mean no handler set
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg,
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_interaction_rejected_called = false;
    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // With null handlers, no rejection should happen (events don't generate rejections)
    EXPECT_FALSE(load_interaction_rejected_called);
}

// ============================================================================
// BRANCH COVERAGE TESTS — Train Search, Emergency, Broadcast Time Index,
//                          Datagram Rejected, Run Loop Fallthrough
// ============================================================================

// ============================================================================
// TEST: Train search event — match found (sets valid)
// Covers: lines 455 (handler non-NULL), 458 (is_train_search true),
//         461 (train_state non-NULL), 465 (outgoing valid true)
// ============================================================================

TEST(OpenLcbMainStatemachine, train_search_event_match_found)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->train_state = &_dummy_train_state;  // Non-NULL = train node

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFY;

    // Load a train search event ID into the payload
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, EVENT_TRAIN_SEARCH_SPACE | 0x00000100);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);
    statemachine_info.outgoing_msg_info.valid = false;

    // Tell mock handler to set outgoing valid (simulates match)
    train_search_handler_set_valid = true;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_TRUE(train_search_event_handler_called);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
}

// ============================================================================
// TEST: Train search event — no match, last node triggers no-match handler
// Covers: lines 474 (is_last true), 475 (no_match_handler non-NULL)
// ============================================================================

TEST(OpenLcbMainStatemachine, train_search_event_no_match_last_node)
{
    _global_initialize();

    // Reset _train_search_match_found by triggering the enumerate-first path.
    // After initialize(), openlcb_node is NULL, so handle_try_enumerate_first_node
    // enters the NULL-node branch which clears the static match flag.
    node_get_first = nullptr;
    OpenLcbMainStatemachine_handle_try_enumerate_first_node();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->train_state = &_dummy_train_state;  // Train node

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFY;

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, EVENT_TRAIN_SEARCH_SPACE | 0x00000100);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);
    statemachine_info.outgoing_msg_info.valid = false;

    // Handler does NOT set valid (no match), and this is the last node
    train_search_handler_set_valid = false;
    force_is_last_node = true;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_TRUE(train_search_event_handler_called);
    EXPECT_TRUE(train_search_no_match_handler_called);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
}

// ============================================================================
// TEST: Train search event — match found but is_last is true
// Covers: line 475 false branch (!_train_search_match_found is false)
// ============================================================================

TEST(OpenLcbMainStatemachine, train_search_event_match_found_last_node)
{
    _global_initialize();

    // Reset _train_search_match_found
    node_get_first = nullptr;
    OpenLcbMainStatemachine_handle_try_enumerate_first_node();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->train_state = &_dummy_train_state;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFY;

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, EVENT_TRAIN_SEARCH_SPACE | 0x00000100);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);
    statemachine_info.outgoing_msg_info.valid = false;

    // Handler SETS valid (match found), AND this is the last node
    train_search_handler_set_valid = true;
    force_is_last_node = true;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Match was found, so no-match handler should NOT be called
    EXPECT_TRUE(train_search_event_handler_called);
    EXPECT_FALSE(train_search_no_match_handler_called);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
}

// ============================================================================
// TEST: Train search event — no match, last node, no_match_handler NULL
// Covers: line 474 remaining branch (is_last=true, match=false, handler=NULL)
// ============================================================================

TEST(OpenLcbMainStatemachine, train_search_event_no_match_handler_null)
{
    // Custom interface: train_search_event_handler set, but no_match_handler NULL
    interface_openlcb_main_statemachine_t custom_interface = interface_openlcb_main_statemachine;
    custom_interface.train_search_no_match_handler = nullptr;

    _reset_variables();
    OpenLcbMainStatemachine_initialize(&custom_interface);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    // Reset _train_search_match_found
    node_get_first = nullptr;
    OpenLcbMainStatemachine_handle_try_enumerate_first_node();
    _reset_variables();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->train_state = &_dummy_train_state;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFY;

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, EVENT_TRAIN_SEARCH_SPACE | 0x00000100);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);
    statemachine_info.outgoing_msg_info.valid = false;

    // No match, last node, but no_match_handler is NULL
    train_search_handler_set_valid = false;
    force_is_last_node = true;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    EXPECT_TRUE(train_search_event_handler_called);
    // no_match_handler is NULL, so it should NOT be called
    EXPECT_FALSE(train_search_no_match_handler_called);
}

// ============================================================================
// TEST: Train search event — non-train node (train_state NULL)
// Covers: line 461 false branch (train_state is NULL)
// ============================================================================

TEST(OpenLcbMainStatemachine, train_search_event_non_train_node)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->train_state = nullptr;  // NOT a train node

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFY;

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, EVENT_TRAIN_SEARCH_SPACE | 0x00000100);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);
    statemachine_info.outgoing_msg_info.valid = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Handler should NOT have been called (non-train node)
    EXPECT_FALSE(train_search_event_handler_called);
}

// ============================================================================
// TEST: PRODUCER_IDENTIFIED_SET — non-zero index skips broadcast time
// Covers: line 518 false branch (index != 0)
// ============================================================================

TEST(OpenLcbMainStatemachine, broadcast_time_pid_set_nonzero_index_skips)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->index = 1;  // Non-zero index — broadcast time only handled on index 0

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PRODUCER_IDENTIFIED_SET;

    // Load a broadcast time event ID
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg,
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    broadcast_time_handler_called = false;
    producer_identified_set_handler_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Broadcast time handler should NOT be called (index != 0)
    EXPECT_FALSE(broadcast_time_handler_called);
    // Falls through to the regular producer_identified_set handler
    EXPECT_TRUE(producer_identified_set_handler_called);
}

// ============================================================================
// TEST: PC_EVENT_REPORT — non-zero index skips broadcast time
// Covers: line 592 false branch (index != 0)
// ============================================================================

TEST(OpenLcbMainStatemachine, broadcast_time_pc_report_nonzero_index_skips)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->index = 1;  // Non-zero index

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PC_EVENT_REPORT;

    // Load a broadcast time event ID
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg,
        BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK | 0x0E1E);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    broadcast_time_handler_called = false;
    pc_event_report_handler_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Broadcast time handler should NOT be called (index != 0)
    EXPECT_FALSE(broadcast_time_handler_called);
    // Falls through to the regular pc_event_report handler
    EXPECT_TRUE(pc_event_report_handler_called);
}

// ============================================================================
// TEST: PC_EVENT_REPORT — train emergency event
// Covers: lines 605 (handler && train_state true), 607 (is_emergency true)
// ============================================================================

TEST(OpenLcbMainStatemachine, train_emergency_event_on_pc_report)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->index = 0;  // Index 0 so broadcast time check passes first
    node->train_state = &_dummy_train_state;  // Train node

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PC_EVENT_REPORT;

    // Load an emergency event ID (NOT a broadcast time event)
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, EVENT_ID_EMERGENCY_STOP);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    broadcast_time_handler_called = false;
    train_emergency_event_handler_called = false;
    pc_event_report_handler_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Emergency event is NOT a broadcast time event, so broadcast time check
    // enters but is_broadcast_time_event returns false → falls through
    EXPECT_FALSE(broadcast_time_handler_called);
    // Emergency handler should be called
    EXPECT_TRUE(train_emergency_event_handler_called);
    // Should NOT fall through to regular pc_event_report (break after emergency)
    EXPECT_FALSE(pc_event_report_handler_called);
}

// ============================================================================
// TEST: PC_EVENT_REPORT — non-emergency event on train node (emergency FALSE)
// Covers: line 607 false branch (is_emergency_event returns false)
// ============================================================================

TEST(OpenLcbMainStatemachine, train_node_non_emergency_pc_report)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->index = 1;  // Non-zero to skip broadcast time check
    node->train_state = &_dummy_train_state;  // Train node

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_PC_EVENT_REPORT;

    // Load a non-emergency, non-broadcast-time event ID
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(msg, 0x0505050505050505ULL);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    train_emergency_event_handler_called = false;
    pc_event_report_handler_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // Not an emergency event — emergency handler should NOT be called
    EXPECT_FALSE(train_emergency_event_handler_called);
    // Falls through to regular pc_event_report handler
    EXPECT_TRUE(pc_event_report_handler_called);
}

// ============================================================================
// TEST: Datagram NULL handler — load_datagram_rejected called
// Covers: line 693 true branch (load_datagram_rejected non-NULL)
// Uses a local copy of the null handler interface with load_datagram_rejected
// wired, so the base null handler struct keeps load_datagram_rejected=NULL
// for the FALSE branch coverage.
// ============================================================================

TEST(OpenLcbMainStatemachine, datagram_null_handler_sends_rejected)
{
    // Create a custom interface: datagram=NULL, load_datagram_rejected=non-NULL
    interface_openlcb_main_statemachine_t custom_interface = interface_openlcb_main_statemachine_null_handlers;
    custom_interface.load_datagram_rejected = &_mock_load_datagram_rejected;

    _reset_variables();
    OpenLcbMainStatemachine_initialize(&custom_interface);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xBBB;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_DATAGRAM;
    msg->dest_alias = 0xBBB;

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;
    statemachine_info.outgoing_msg_info.msg_ptr = OpenLcbBufferStore_allocate_buffer(STREAM);

    load_datagram_rejected_called = false;

    OpenLcbMainStatemachine_process_main_statemachine(&statemachine_info);

    // datagram handler is NULL, so load_datagram_rejected should be called
    EXPECT_TRUE(load_datagram_rejected_called);
}

// ============================================================================
// TEST: Run loop — all handlers return false (full fallthrough)
// Covers: line 1020 false branch (handle_try_enumerate_next_node returns false)
// ============================================================================

TEST(OpenLcbMainStatemachine, run_all_handlers_return_false)
{
    _reset_variables();
    _global_initialize();

    // Force ALL handle functions to return false
    fail_handle_outgoing_openlcb_message = true;
    fail_handle_try_reenumerate = true;
    fail_handle_try_pop_next_incoming_openlcb_message = true;
    fail_handle_try_enumerate_first_node = true;
    fail_handle_try_enumerate_next_node = true;

    OpenLcbMainStatemachine_run();

    // All five handlers should have been called (all returned false)
    EXPECT_TRUE(handle_outgoing_called);
    EXPECT_TRUE(handle_reenumerate_called);
    EXPECT_TRUE(handle_pop_called);
    EXPECT_TRUE(handle_enumerate_first_called);
    EXPECT_TRUE(handle_enumerate_next_called);
}

// ============================================================================
// TEST: Sibling dispatch — loopback with DATAGRAM-sized payload
// Covers: line 161 FALSE branch, line 165 TRUE branch, line 194 TRUE branch
// ============================================================================

// Old loopback payload-size tests removed — _loopback_to_siblings() deleted.
// Sibling dispatch uses zero-copy (points at outgoing slot), no buffer allocation.

// ============================================================================
// TEST: does_node_process_msg — loopback self-skip
// Covers: line 239 compound TRUE branch (loopback && source_id matches node id)
// ============================================================================

TEST(OpenLcbMainStatemachine, does_node_process_msg_loopback_self_skip)
{
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(0x060504030201, &_node_parameters_main_node);
    node->state.initialized = true;
    node->alias = 0xAAA;

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    msg->mti = MTI_VERIFIED_NODE_ID;
    msg->state.loopback = true;
    msg->source_id = 0x060504030201;  // Same as node id → self-skip

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = msg;

    bool result = OpenLcbMainStatemachine_does_node_process_msg(&statemachine_info);

    EXPECT_FALSE(result);  // Should skip its own loopback message
}

// Old cascade prevention tests removed — loopback is now handled by
// sequential sibling dispatch, not FIFO-based copy with boolean guard.

// ============================================================================
// Sibling Dispatch Integration Tests
//
// These tests use REAL node enumeration (OpenLcbNode_get_first/get_next) and
// REAL internal handler functions to exercise the full sibling dispatch
// mechanism end-to-end.  Mock protocol handlers produce controlled responses
// to force specific dispatch paths.
// ============================================================================

// ---- Tracking infrastructure ----

#define SIBLING_TEST_LOG_DEPTH 1024

static struct {

    node_id_t node_id;
    uint16_t mti;

} _st_dispatch_log[SIBLING_TEST_LOG_DEPTH];

static int _st_dispatch_count;

static struct {

    uint16_t mti;
    node_id_t source_id;
    node_id_t dest_id;

} _st_wire_log[SIBLING_TEST_LOG_DEPTH];

static int _st_wire_count;

// ---- Enumerate response control ----

static int _st_enumerate_remaining;
static uint16_t _st_enumerate_response_mti;
static int _st_enumerate_per_node;
static node_id_t _st_last_enumerate_node;

// ---- Conditional response control ----

static node_id_t _st_respond_if_node;
static uint16_t _st_response_mti;
static bool _st_has_responded;

// ---- Transport busy control ----

static bool _st_wire_busy;

// ---- Stream addressed reply control ----

static bool _st_stream_reply_active;
static node_id_t _st_stream_reply_from_node;

// ---- Reset all sibling test state ----

static void _st_reset(void) {

    _st_dispatch_count = 0;
    _st_wire_count = 0;
    _st_enumerate_remaining = 0;
    _st_enumerate_response_mti = MTI_PRODUCER_IDENTIFIED_SET;
    _st_enumerate_per_node = 0;
    _st_last_enumerate_node = 0;
    _st_respond_if_node = 0;
    _st_response_mti = 0;
    _st_has_responded = false;
    _st_wire_busy = false;
    _st_stream_reply_active = false;
    _st_stream_reply_from_node = 0;

}

// ---- Wire-capturing send function ----

static bool _st_wire_send(openlcb_msg_t *msg) {

    if (_st_wire_busy) {

        return false;

    }

    if (_st_wire_count < SIBLING_TEST_LOG_DEPTH) {

        _st_wire_log[_st_wire_count].mti = msg->mti;
        _st_wire_log[_st_wire_count].source_id = msg->source_id;
        _st_wire_log[_st_wire_count].dest_id = msg->dest_id;
        _st_wire_count++;

    }

    return true;

}

// ---- Logging handler: tracks which node saw which MTI ----

static void _st_log_handler(openlcb_statemachine_info_t *si) {

    if (_st_dispatch_count < SIBLING_TEST_LOG_DEPTH) {

        _st_dispatch_log[_st_dispatch_count].node_id = si->openlcb_node->id;
        _st_dispatch_log[_st_dispatch_count].mti =
                si->incoming_msg_info.msg_ptr->mti;
        _st_dispatch_count++;

    }

}

// ---- Handler that produces a Verified Node ID response ----

static void _st_verified_reply_handler(openlcb_statemachine_info_t *si) {

    _st_log_handler(si);

    OpenLcbUtilities_load_openlcb_message(
            si->outgoing_msg_info.msg_ptr,
            si->openlcb_node->alias,
            si->openlcb_node->id,
            0, 0,
            MTI_VERIFIED_NODE_ID);
    si->outgoing_msg_info.valid = true;

}

// ---- Enumerate handler: produces N outgoing messages per node ----

static void _st_enumerate_handler(openlcb_statemachine_info_t *si) {

    // Reset counter when dispatched to a new node
    if (si->openlcb_node->id != _st_last_enumerate_node) {

        _st_enumerate_remaining = _st_enumerate_per_node;
        _st_last_enumerate_node = si->openlcb_node->id;

    }

    _st_log_handler(si);

    if (_st_enumerate_remaining > 0) {

        OpenLcbUtilities_load_openlcb_message(
                si->outgoing_msg_info.msg_ptr,
                si->openlcb_node->alias,
                si->openlcb_node->id,
                0, 0,
                _st_enumerate_response_mti);
        si->outgoing_msg_info.valid = true;
        _st_enumerate_remaining--;

        if (_st_enumerate_remaining > 0) {

            si->incoming_msg_info.enumerate = true;

        } else {

            si->incoming_msg_info.enumerate = false;

        }

    }

}

// ---- Conditional response handler: responds only for a specific node ----

static void _st_conditional_respond_handler(openlcb_statemachine_info_t *si) {

    _st_log_handler(si);

    if (si->openlcb_node->id == _st_respond_if_node && !_st_has_responded) {

        OpenLcbUtilities_load_openlcb_message(
                si->outgoing_msg_info.msg_ptr,
                si->openlcb_node->alias,
                si->openlcb_node->id,
                0, 0,
                _st_response_mti);
        si->outgoing_msg_info.valid = true;
        _st_has_responded = true;

    }

}

// ---- Handler: produces MTI_STREAM_INIT_REPLY addressed back to the request source ----
//
// Used only by sibling_stream_reply_routes_back_to_originating_sibling.
// Reads source_alias/source_id from the incoming message and builds a reply
// addressed to that node, then clears _st_stream_reply_active so it fires once.

static void _st_stream_initiate_request_with_reply(openlcb_statemachine_info_t *si) {

    _st_log_handler(si);

    if (!_st_stream_reply_active) { return; }

    if (si->openlcb_node->id != _st_stream_reply_from_node) { return; }

    OpenLcbUtilities_load_openlcb_message(
            si->outgoing_msg_info.msg_ptr,
            si->openlcb_node->alias,
            si->openlcb_node->id,
            si->incoming_msg_info.msg_ptr->source_alias,
            si->incoming_msg_info.msg_ptr->source_id,
            MTI_STREAM_INIT_REPLY);
    si->outgoing_msg_info.valid = true;
    _st_stream_reply_active = false;

}

// ---- Integration test interface: real node functions, real handlers ----

static const interface_openlcb_main_statemachine_t _st_interface = {

    .lock_shared_resources = &_ExampleDrivers_lock_shared_resources,
    .unlock_shared_resources = &_ExampleDrivers_unlock_shared_resources,
    .send_openlcb_msg = &_st_wire_send,
    .get_current_tick = &_mock_get_current_tick,

    // Real node enumeration
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .openlcb_node_is_last = &OpenLcbNode_is_last,
    .openlcb_node_get_count = &OpenLcbNode_get_count,

    .load_interaction_rejected = &OpenLcbMainStatemachine_load_interaction_rejected,

    // Controllable protocol handlers
    .message_network_initialization_complete = &_st_log_handler,
    .message_network_initialization_complete_simple = &_st_log_handler,
    .message_network_verify_node_id_addressed = &_st_log_handler,
    .message_network_verify_node_id_global = &_st_verified_reply_handler,
    .message_network_verified_node_id = &_st_conditional_respond_handler,
    .message_network_optional_interaction_rejected = &_st_log_handler,
    .message_network_terminate_due_to_error = &_st_log_handler,
    .message_network_protocol_support_inquiry = &_st_log_handler,
    .message_network_protocol_support_reply = &_st_log_handler,

    .snip_simple_node_info_request = &_st_log_handler,
    .snip_simple_node_info_reply = &_st_log_handler,

    .event_transport_consumer_identify = &_st_log_handler,
    .event_transport_consumer_range_identified = &_st_log_handler,
    .event_transport_consumer_identified_unknown = &_st_log_handler,
    .event_transport_consumer_identified_set = &_st_log_handler,
    .event_transport_consumer_identified_clear = &_st_log_handler,
    .event_transport_consumer_identified_reserved = &_st_log_handler,
    .event_transport_producer_identify = &_st_log_handler,
    .event_transport_producer_range_identified = &_st_log_handler,
    .event_transport_producer_identified_unknown = &_st_log_handler,
    .event_transport_producer_identified_set = &_st_log_handler,
    .event_transport_producer_identified_clear = &_st_log_handler,
    .event_transport_producer_identified_reserved = &_st_log_handler,
    .event_transport_identify_dest = &_st_log_handler,
    .event_transport_identify = &_st_enumerate_handler,
    .event_transport_learn = &_st_log_handler,
    .event_transport_pc_report = &_st_log_handler,
    .event_transport_pc_report_with_payload = &_st_log_handler,

    .train_control_command = &_st_log_handler,
    .train_control_reply = &_st_log_handler,
    .simple_train_node_ident_info_request = &_st_log_handler,
    .simple_train_node_ident_info_reply = &_st_log_handler,

    .datagram = &_st_log_handler,
    .datagram_ok_reply = &_st_log_handler,
    .datagram_rejected_reply = &_st_log_handler,

    .stream_initiate_request = &_st_log_handler,
    .stream_initiate_reply = &_st_log_handler,
    .stream_send_data = &_st_log_handler,
    .stream_data_proceed = &_st_log_handler,
    .stream_data_complete = &_st_log_handler,

    // Real internal handlers
    .process_main_statemachine = &OpenLcbMainStatemachine_process_main_statemachine,
    .does_node_process_msg = &OpenLcbMainStatemachine_does_node_process_msg,
    .handle_outgoing_openlcb_message = &OpenLcbMainStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &OpenLcbMainStatemachine_handle_try_reenumerate,
    .handle_try_pop_next_incoming_openlcb_message = &OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message,
    .handle_try_enumerate_first_node = &OpenLcbMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &OpenLcbMainStatemachine_handle_try_enumerate_next_node,

};

    /** @brief Initialize for sibling dispatch integration tests. */
static void _st_init(void) {

    _st_reset();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbMainStatemachine_initialize(&_st_interface);

}

    /** @brief Count dispatch log entries for a given node_id. */
static int _st_count_dispatches_for_node(node_id_t node_id) {

    int count = 0;

    for (int i = 0; i < _st_dispatch_count; i++) {

        if (_st_dispatch_log[i].node_id == node_id) {

            count++;

        }

    }

    return count;

}

    /** @brief Count dispatch log entries for a given node_id and mti. */
static int _st_count_dispatches_for_node_mti(node_id_t node_id, uint16_t mti) {

    int count = 0;

    for (int i = 0; i < _st_dispatch_count; i++) {

        if (_st_dispatch_log[i].node_id == node_id &&
                _st_dispatch_log[i].mti == mti) {

            count++;

        }

    }

    return count;

}

    /** @brief Count wire log entries for a given mti. */
static int _st_count_wire_mti(uint16_t mti) {

    int count = 0;

    for (int i = 0; i < _st_wire_count; i++) {

        if (_st_wire_log[i].mti == mti) {

            count++;

        }

    }

    return count;

}

// ============================================================================
// TEST: Single node — no sibling dispatch, valid clears immediately
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_single_node_no_dispatch)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    // Push incoming Verify Node ID Global (triggers Verified Node ID response)
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    // Run until idle
    for (int i = 0; i < 20; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Node A should have been dispatched the message
    EXPECT_EQ(_st_count_dispatches_for_node(0x010203040501), 1);

    // Verified Node ID should have gone to wire
    EXPECT_EQ(_st_count_wire_mti(MTI_VERIFIED_NODE_ID), 1);

    // No sibling dispatch overhead
    EXPECT_EQ(OpenLcbMainStatemachine_get_sibling_response_queue_high_water(), 0);
}

// ============================================================================
// TEST: 3 nodes — outgoing message dispatched to all siblings
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_3_nodes_global_visibility)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Push incoming Verify Node ID Global from external node
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 100; i++) {

        OpenLcbMainStatemachine_run();

    }

    // All 3 nodes process the incoming Verify Node ID Global
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_VERIFY_NODE_ID_GLOBAL), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_VERIFY_NODE_ID_GLOBAL), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_VERIFY_NODE_ID_GLOBAL), 1);

    // Each node produces a Verified Node ID → 3 on wire
    EXPECT_EQ(_st_count_wire_mti(MTI_VERIFIED_NODE_ID), 3);

    // Each Verified Node ID goes to 2 siblings via sibling dispatch
    // Node A's Verified → B and C see it (MTI_VERIFIED_NODE_ID)
    // Node B's Verified → A and C see it
    // Node C's Verified → A and B see it
    // Total: each node sees 2 sibling Verified messages + 1 original dispatch = 3 Verified dispatches
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_VERIFIED_NODE_ID), 2);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_VERIFIED_NODE_ID), 2);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_VERIFIED_NODE_ID), 2);

    // Zero extra buffer allocations (all static slots)
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: Self-skip — originating node does NOT process its own loopback
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_self_skip_during_dispatch)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    // Push incoming that triggers Node A to respond
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 50; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Node A produces Verified Node ID with source_id = A
    // During sibling dispatch, A must NOT process its own Verified (self-skip)
    // Only B should see A's Verified as a sibling dispatch
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_VERIFIED_NODE_ID), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_VERIFIED_NODE_ID), 1);
}

// ============================================================================
// TEST: Sibling response chain — depth 2
// Node A sends Verified Node ID.  During sibling dispatch, Node B responds
// with its own message.  B's response goes to wire AND reaches Node A via
// the sibling response queue.
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_response_chain_depth_2)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Configure: when Node B sees Verified Node ID, it responds with Init Complete
    _st_respond_if_node = 0x010203040502;
    _st_response_mti = MTI_INITIALIZATION_COMPLETE;

    // Push incoming Verify Global → Node A responds with Verified
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 200; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Node A's Verified went to wire
    // Node B's Init Complete response went to wire (depth 2)
    EXPECT_GE(_st_count_wire_mti(MTI_VERIFIED_NODE_ID), 3);
    EXPECT_GE(_st_count_wire_mti(MTI_INITIALIZATION_COMPLETE), 1);

    // Node B's Init Complete should reach siblings via response queue
    // Node A and C should see it
    EXPECT_GE(_st_count_dispatches_for_node_mti(0x010203040501, MTI_INITIALIZATION_COMPLETE), 1);
    EXPECT_GE(_st_count_dispatches_for_node_mti(0x010203040503, MTI_INITIALIZATION_COMPLETE), 1);

    // High-water mark should be at least 1
    EXPECT_GE(OpenLcbMainStatemachine_get_sibling_response_queue_high_water(), 1);

    // All buffers freed
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: Enumerate + sibling dispatch interleaving — 4 nodes × 10 events
// Each node produces 10 P.Id Set responses via enumerate.  Every response
// is dispatched to all 3 siblings before the next is produced.
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_enumerate_interleaving_4_nodes_10_events)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeD = OpenLcbNode_allocate(0x010203040504, &_node_parameters_main_node);
    nodeD->state.initialized = true;
    nodeD->alias = 0xDDD;
    nodeD->state.run_state = RUNSTATE_RUN;

    _st_enumerate_per_node = 10;
    _st_enumerate_response_mti = MTI_PRODUCER_IDENTIFIED_SET;

    OpenLcbBufferStore_clear_max_allocated();

    // Push Identify Events Global
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_EVENTS_IDENTIFY;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    // Run enough iterations: 4 nodes × 10 events × (1 send + 3 siblings) + overhead
    for (int i = 0; i < 2000; i++) {

        OpenLcbMainStatemachine_run();

    }

    // 4 nodes × 10 events = 40 P.Id Set on wire
    EXPECT_EQ(_st_count_wire_mti(MTI_PRODUCER_IDENTIFIED_SET), 40);

    // Each P.Id Set dispatched to 3 siblings = 40 × 3 = 120 sibling dispatches
    // (The 40 direct enumerate calls log as MTI_EVENTS_IDENTIFY, not P.Id Set)
    int total_pid = _st_count_dispatches_for_node_mti(0x010203040501, MTI_PRODUCER_IDENTIFIED_SET)
                  + _st_count_dispatches_for_node_mti(0x010203040502, MTI_PRODUCER_IDENTIFIED_SET)
                  + _st_count_dispatches_for_node_mti(0x010203040503, MTI_PRODUCER_IDENTIFIED_SET)
                  + _st_count_dispatches_for_node_mti(0x010203040504, MTI_PRODUCER_IDENTIFIED_SET);

    // Each node sees 30 sibling P.Id messages (10 from each of 3 other nodes)
    EXPECT_EQ(total_pid, 120);

    // Maximum 1 buffer allocated at a time (the incoming message)
    // After processing, all buffers freed
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

    // Peak allocation: only the 1 incoming message from FIFO
    EXPECT_LE(OpenLcbBufferStore_basic_messages_max_allocated(), 1);
}

// ============================================================================
// TEST: Stress — 4 nodes × 100 events via enumerate
// Verifies bounded buffer usage and correct dispatch count at scale.
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_stress_4_nodes_100_events)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeD = OpenLcbNode_allocate(0x010203040504, &_node_parameters_main_node);
    nodeD->state.initialized = true;
    nodeD->alias = 0xDDD;
    nodeD->state.run_state = RUNSTATE_RUN;

    _st_enumerate_per_node = 100;
    _st_enumerate_response_mti = MTI_PRODUCER_IDENTIFIED_SET;

    OpenLcbBufferStore_clear_max_allocated();

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_EVENTS_IDENTIFY;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    // 4 nodes × 100 events × ~5 iterations each + overhead
    for (int i = 0; i < 10000; i++) {

        OpenLcbMainStatemachine_run();

    }

    // 4 × 100 = 400 messages on wire
    EXPECT_EQ(_st_count_wire_mti(MTI_PRODUCER_IDENTIFIED_SET), 400);

    // All buffers freed — ZERO leak
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

    // Peak allocation: only the 1 incoming message (no loopback copies!)
    EXPECT_LE(OpenLcbBufferStore_basic_messages_max_allocated(), 1);
}

// ============================================================================
// TEST: Path B wrapper — application send gets sibling dispatch
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_path_b_wrapper)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Simulate an application-layer send (Path B)
    openlcb_msg_t app_msg;
    payload_basic_t app_payload;
    app_msg.payload = (openlcb_payload_t *) &app_payload;
    app_msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &app_msg,
            nodeA->alias,
            nodeA->id,
            0, 0,
            MTI_PC_EVENT_REPORT);
    app_msg.payload_count = 8;

    bool sent = OpenLcbMainStatemachine_send_with_sibling_dispatch(&app_msg);
    EXPECT_TRUE(sent);

    // Message went to wire immediately
    EXPECT_EQ(_st_count_wire_mti(MTI_PC_EVENT_REPORT), 1);

    // Run loop picks up the pending slot and dispatches to siblings
    for (int i = 0; i < 50; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Siblings B and C should have seen the PCER
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_PC_EVENT_REPORT), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_PC_EVENT_REPORT), 1);

    // Node A should NOT see its own message (self-skip)
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_PC_EVENT_REPORT), 0);
}

// ============================================================================
// TEST: Transport busy during sibling outgoing — retries correctly
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_transport_busy_retry)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    // Configure Node B to respond during sibling dispatch
    _st_respond_if_node = 0x010203040502;
    _st_response_mti = MTI_INITIALIZATION_COMPLETE;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    // Run a few iterations to pop and start enumeration
    for (int i = 0; i < 5; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Now make wire busy — sibling outgoing should stall
    _st_wire_busy = true;

    for (int i = 0; i < 10; i++) {

        OpenLcbMainStatemachine_run();

    }

    int wire_count_while_busy = _st_wire_count;

    // Unblock wire
    _st_wire_busy = false;

    for (int i = 0; i < 100; i++) {

        OpenLcbMainStatemachine_run();

    }

    // After unblocking, more messages should have been sent
    EXPECT_GT(_st_wire_count, wire_count_while_busy);

    // All buffers freed
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: 4 nodes — addressed message only reaches destination during dispatch
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_addressed_message_filtering)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeD = OpenLcbNode_allocate(0x010203040504, &_node_parameters_main_node);
    nodeD->state.initialized = true;
    nodeD->alias = 0xDDD;
    nodeD->state.run_state = RUNSTATE_RUN;

    // Push addressed SNIP request: external → Node B only
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_SIMPLE_NODE_INFO_REQUEST;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    incoming->dest_alias = 0xBBB;
    incoming->dest_id = 0x010203040502;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 50; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Only Node B should process the SNIP request
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_SIMPLE_NODE_INFO_REQUEST), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_SIMPLE_NODE_INFO_REQUEST), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_SIMPLE_NODE_INFO_REQUEST), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040504, MTI_SIMPLE_NODE_INFO_REQUEST), 0);
}

// ============================================================================
// TEST: High-water mark tracking
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_response_queue_high_water_tracking)
{
    _st_init();

    // Verify initial state
    EXPECT_EQ(OpenLcbMainStatemachine_get_sibling_response_queue_high_water(), 0);

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    // Node B responds during sibling dispatch → pushes to response queue
    _st_respond_if_node = 0x010203040502;
    _st_response_mti = MTI_INITIALIZATION_COMPLETE;

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 100; i++) {

        OpenLcbMainStatemachine_run();

    }

    // High-water should reflect the response queue usage
    EXPECT_GE(OpenLcbMainStatemachine_get_sibling_response_queue_high_water(), 1);

    // All buffers freed
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: Path B with single node — wrapper sends to wire, no pending
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_path_b_single_node_no_pending)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_msg_t app_msg;
    payload_basic_t app_payload;
    app_msg.payload = (openlcb_payload_t *) &app_payload;
    app_msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &app_msg,
            nodeA->alias,
            nodeA->id,
            0, 0,
            MTI_PC_EVENT_REPORT);
    app_msg.payload_count = 8;

    bool sent = OpenLcbMainStatemachine_send_with_sibling_dispatch(&app_msg);
    EXPECT_TRUE(sent);

    // Message on wire
    EXPECT_EQ(_st_count_wire_mti(MTI_PC_EVENT_REPORT), 1);

    // Run loop — nothing pending for sibling dispatch (single node)
    for (int i = 0; i < 20; i++) {

        OpenLcbMainStatemachine_run();

    }

    // No sibling dispatches
    EXPECT_EQ(_st_dispatch_count, 0);
}

// ============================================================================
// Stream Sibling Dispatch Tests
//
// All five stream MTIs carry MASK_DEST_ADDRESS_PRESENT (0x0008), so they are
// addressed messages.  The sibling dispatch path must route each one only to
// the matching virtual node; bystander siblings must receive nothing.
// ============================================================================

// ============================================================================
// TEST: STREAM_INIT_REQUEST routes only to addressed sibling
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_stream_init_request_routes_to_destination_only)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    // External node initiates a stream to Node B only
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_STREAM_INIT_REQUEST;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    incoming->dest_alias = 0xBBB;
    incoming->dest_id = 0x010203040502;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 50; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Only Node B is addressed — it receives the request
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_STREAM_INIT_REQUEST), 1);

    // Node A and C are bystanders — they must not receive it
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_STREAM_INIT_REQUEST), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_INIT_REQUEST), 0);

    // No buffer leak
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: STREAM_INIT_REPLY routes only to addressed sibling
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_stream_init_reply_routes_to_destination_only)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    // External node replies to Node A's earlier request
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_STREAM_INIT_REPLY;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    incoming->dest_alias = 0xAAA;
    incoming->dest_id = 0x010203040501;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 50; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Only Node A is addressed
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_STREAM_INIT_REPLY), 1);

    // B and C are bystanders
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_STREAM_INIT_REPLY), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_INIT_REPLY), 0);

    // No buffer leak
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: STREAM_SEND and STREAM_PROCEED route only to their addressed siblings
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_stream_send_and_proceed_route_to_destination_only)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Data payload sent to Node B
    openlcb_msg_t *send_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    send_msg->mti = MTI_STREAM_SEND;
    send_msg->source_alias = 0xFFF;
    send_msg->source_id = 0x0A0B0C0D0E0F;
    send_msg->dest_alias = 0xBBB;
    send_msg->dest_id = 0x010203040502;
    OpenLcbBufferFifo_push(send_msg);

    for (int i = 0; i < 50; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Flow-control ack sent to Node A
    openlcb_msg_t *proceed_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    proceed_msg->mti = MTI_STREAM_PROCEED;
    proceed_msg->source_alias = 0xFFF;
    proceed_msg->source_id = 0x0A0B0C0D0E0F;
    proceed_msg->dest_alias = 0xAAA;
    proceed_msg->dest_id = 0x010203040501;
    OpenLcbBufferFifo_push(proceed_msg);

    for (int i = 0; i < 50; i++) {

        OpenLcbMainStatemachine_run();

    }

    // STREAM_SEND: only B receives it
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_STREAM_SEND), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_STREAM_SEND), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_SEND), 0);

    // STREAM_PROCEED: only A receives it
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_STREAM_PROCEED), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_STREAM_PROCEED), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_PROCEED), 0);

    // No buffer leak
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: STREAM_INIT_REPLY from sibling B routes back to originating sibling A
//       via the sibling response queue; bystander C receives nothing
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_stream_reply_routes_back_to_originating_sibling)
{
    // Use a local interface copy that rewires stream_initiate_request to the
    // reply-generating handler.  All other slots remain identical to _st_interface.
    _st_reset();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);

    interface_openlcb_main_statemachine_t local_iface = _st_interface;
    local_iface.stream_initiate_request = &_st_stream_initiate_request_with_reply;
    OpenLcbMainStatemachine_initialize(&local_iface);

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Node B will produce a STREAM_INIT_REPLY addressed back to the request source
    _st_stream_reply_active = true;
    _st_stream_reply_from_node = 0x010203040502;

    // Node A sends STREAM_INIT_REQUEST to Node B via application-layer path
    openlcb_msg_t app_msg;
    payload_basic_t app_payload;
    app_msg.payload = (openlcb_payload_t *) &app_payload;
    app_msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &app_msg,
            nodeA->alias,
            nodeA->id,
            nodeB->alias,
            nodeB->id,
            MTI_STREAM_INIT_REQUEST);

    bool sent = OpenLcbMainStatemachine_send_with_sibling_dispatch(&app_msg);
    EXPECT_TRUE(sent);

    // Request on wire immediately
    EXPECT_EQ(_st_count_wire_mti(MTI_STREAM_INIT_REQUEST), 1);

    for (int i = 0; i < 100; i++) {

        OpenLcbMainStatemachine_run();

    }

    // STREAM_INIT_REQUEST routing: only B is addressed, A self-skips, C is bystander
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_STREAM_INIT_REQUEST), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_STREAM_INIT_REQUEST), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_INIT_REQUEST), 0);

    // B's STREAM_INIT_REPLY went to wire addressed to A
    EXPECT_EQ(_st_count_wire_mti(MTI_STREAM_INIT_REPLY), 1);

    // STREAM_INIT_REPLY routing via sibling response queue:
    //   A receives it (addressed to A), B self-skips, C is bystander
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_STREAM_INIT_REPLY), 1);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_STREAM_INIT_REPLY), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_INIT_REPLY), 0);

    // No buffer leak
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: STREAM_COMPLETE routes only to addressed sibling
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_stream_complete_routes_to_destination_only)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Stream initiator closes the stream to Node B
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti = MTI_STREAM_COMPLETE;
    incoming->source_alias = 0xFFF;
    incoming->source_id = 0x0A0B0C0D0E0F;
    incoming->dest_alias = 0xBBB;
    incoming->dest_id = 0x010203040502;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 50; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Only Node B is addressed
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_STREAM_COMPLETE), 1);

    // A and C are bystanders
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_STREAM_COMPLETE), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_COMPLETE), 0);

    // No buffer leak
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

// ============================================================================
// TEST: Under load, third and fourth siblings never receive stream messages
//       and no buffers are exhausted
// ============================================================================

TEST(OpenLcbMainStatemachine, sibling_stream_third_sibling_isolation_under_load)
{
    _st_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0xAAA;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0xBBB;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0xCCC;
    nodeC->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeD = OpenLcbNode_allocate(0x010203040504, &_node_parameters_main_node);
    nodeD->state.initialized = true;
    nodeD->alias = 0xDDD;
    nodeD->state.run_state = RUNSTATE_RUN;

    OpenLcbBufferStore_clear_max_allocated();

    // 10 STREAM_SEND frames addressed to Node B (bulk data transfer)
    for (int i = 0; i < 10; i++) {

        openlcb_msg_t *send_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
        send_msg->mti = MTI_STREAM_SEND;
        send_msg->source_alias = 0xFFF;
        send_msg->source_id = 0x0A0B0C0D0E0F;
        send_msg->dest_alias = 0xBBB;
        send_msg->dest_id = 0x010203040502;
        OpenLcbBufferFifo_push(send_msg);

    }

    // 10 STREAM_PROCEED flow-control acks addressed to Node A
    for (int i = 0; i < 10; i++) {

        openlcb_msg_t *proceed_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
        proceed_msg->mti = MTI_STREAM_PROCEED;
        proceed_msg->source_alias = 0xFFF;
        proceed_msg->source_id = 0x0A0B0C0D0E0F;
        proceed_msg->dest_alias = 0xAAA;
        proceed_msg->dest_id = 0x010203040501;
        OpenLcbBufferFifo_push(proceed_msg);

    }

    for (int i = 0; i < 500; i++) {

        OpenLcbMainStatemachine_run();

    }

    // B received every STREAM_SEND
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040502, MTI_STREAM_SEND), 10);

    // A received every STREAM_PROCEED
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040501, MTI_STREAM_PROCEED), 10);

    // C and D received no stream messages of any kind
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_SEND), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040503, MTI_STREAM_PROCEED), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040504, MTI_STREAM_SEND), 0);
    EXPECT_EQ(_st_count_dispatches_for_node_mti(0x010203040504, MTI_STREAM_PROCEED), 0);

    // No buffer leak
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}
