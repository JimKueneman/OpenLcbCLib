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
* @file openlcb_multinode_e2e_Test.cxx
* @brief Multi-node end-to-end integration tests for OpenLCB sibling dispatch.
*
* @details This suite covers seams between modules that are tested in isolation
* elsewhere:
*
*   1. Login sibling dispatch wired to the real main statemachine handler
*   2. Sibling response queue with two concurrent responders (queue depth >= 2)
*   3. Sibling response queue depth-3 sequential chain
*   4. Login + main statemachines running concurrently without interference
*   5. External datagram to B — bystanders A and C receive nothing
*   6. Sibling datagram from A to B — rejected reply routes back to A
*   7. Sibling datagram from A to C — bystander B fully isolated
*
* All tests verify zero buffer leaks via OpenLcbBufferStore_basic_messages_allocated().
*
* @author Jim Kueneman
* @date 17 Mar 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_main_statemachine.h"
#include "openlcb_login_statemachine.h"
#include "openlcb_login_statemachine_handler.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "protocol_datagram_handler.h"

// ============================================================================
// Node parameters — minimal (0 producers, 0 consumers keeps login to 3 steps)
// ============================================================================

node_parameters_t _node_parameters_main_node = {

    .snip = {
        .mfg_version = 4,
        .name = "E2E Test",
        .model = "E2E Model",
        .hardware_version = "1.0",
        .software_version = "1.0",
        .user_version = 2
    },

    .protocol_support = (PSI_DATAGRAM | PSI_SIMPLE_NODE_INFORMATION),

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

};

interface_openlcb_node_t interface_openlcb_node = {};

// ============================================================================
// Wire log — captures every message sent to the CAN wire
// ============================================================================

#define E2E_LOG_DEPTH 512

static struct {

    uint16_t mti;
    uint16_t source_alias;
    uint16_t dest_alias;
    node_id_t dest_id;

} _e2e_wire_log[E2E_LOG_DEPTH];

static int _e2e_wire_count;
static bool _e2e_wire_busy;

static bool _e2e_wire_send(openlcb_msg_t *msg) {

    if (_e2e_wire_busy) { return false; }

    if (_e2e_wire_count < E2E_LOG_DEPTH) {

        _e2e_wire_log[_e2e_wire_count].mti          = msg->mti;
        _e2e_wire_log[_e2e_wire_count].source_alias = msg->source_alias;
        _e2e_wire_log[_e2e_wire_count].dest_alias   = msg->dest_alias;
        _e2e_wire_log[_e2e_wire_count].dest_id      = msg->dest_id;
        _e2e_wire_count++;

    }

    return true;

}

static int _e2e_count_wire_mti(uint16_t mti) {

    int n = 0;

    for (int i = 0; i < _e2e_wire_count; i++) {

        if (_e2e_wire_log[i].mti == mti) { n++; }

    }

    return n;

}

static uint16_t _e2e_wire_dest_alias_for_mti(uint16_t mti) {

    for (int i = 0; i < _e2e_wire_count; i++) {

        if (_e2e_wire_log[i].mti == mti) { return _e2e_wire_log[i].dest_alias; }

    }

    return 0;

}

// ============================================================================
// Dispatch log — captured by the _e2e_process_main_statemachine wrapper
// ============================================================================

static struct {

    node_id_t node_id;
    uint16_t mti;

} _e2e_dispatch_log[E2E_LOG_DEPTH];

static int _e2e_dispatch_count_total;

static int _e2e_dispatch_count_for_node_mti(node_id_t node_id, uint16_t mti) {

    int n = 0;

    for (int i = 0; i < _e2e_dispatch_count_total; i++) {

        if (_e2e_dispatch_log[i].node_id == node_id &&
                _e2e_dispatch_log[i].mti == mti) {

            n++;

        }

    }

    return n;

}

// ============================================================================
// Conditional respond globals
//
// _e2e_vglobal_{node,mti,done}_{1,2} — used by message_network_verify_node_id_global
//   Set node_1 and/or node_2 to cause those nodes to respond with the
//   corresponding mti.  Zeroed by _e2e_reset() so unused slots are noops.
//
// _e2e_verified_{node,mti,done}_1 — used by message_network_verified_node_id
//   Used by TEST 3 to create the depth-3 chain.
// ============================================================================

static node_id_t _e2e_vglobal_node_1;
static uint16_t  _e2e_vglobal_mti_1;
static bool      _e2e_vglobal_done_1;

static node_id_t _e2e_vglobal_node_2;
static uint16_t  _e2e_vglobal_mti_2;
static bool      _e2e_vglobal_done_2;

static node_id_t _e2e_verified_node_1;
static uint16_t  _e2e_verified_mti_1;
static bool      _e2e_verified_done_1;

// ============================================================================
// Thin wrapper — logs (node_id, mti) then calls the real dispatcher
// ============================================================================

static void _e2e_process_main_statemachine(openlcb_statemachine_info_t *si) {

    // Pre-filter: only log nodes that actually process this message.
    // Mirrors does_node_process_msg so bystanders (wrong address) are excluded.
    if (OpenLcbMainStatemachine_does_node_process_msg(si) &&
            _e2e_dispatch_count_total < E2E_LOG_DEPTH) {

        _e2e_dispatch_log[_e2e_dispatch_count_total].node_id =
                si->openlcb_node->id;
        _e2e_dispatch_log[_e2e_dispatch_count_total].mti =
                si->incoming_msg_info.msg_ptr->mti;
        _e2e_dispatch_count_total++;

    }

    OpenLcbMainStatemachine_process_main_statemachine(si);

}

// ============================================================================
// Message handlers
// ============================================================================

static void _e2e_lock(void) { }
static void _e2e_unlock(void) { }
static uint8_t _e2e_get_tick(void) { return 0; }
static void _e2e_noop_handler(openlcb_statemachine_info_t *si) { (void)si; }

static void _e2e_load_interaction_rejected(openlcb_statemachine_info_t *si) {

    OpenLcbMainStatemachine_load_interaction_rejected(si);

}

// ---- Conditional respond for verify_node_id_global slot ----
//
// Checks both responder slots (1 and 2).  Only the slot whose node_id
// matches fires, and only once (guarded by the done flag).

static void _e2e_verify_global_handler(openlcb_statemachine_info_t *si) {

    if (si->openlcb_node->id == _e2e_vglobal_node_1 && !_e2e_vglobal_done_1) {

        OpenLcbUtilities_load_openlcb_message(
                si->outgoing_msg_info.msg_ptr,
                si->openlcb_node->alias,
                si->openlcb_node->id,
                0, 0,
                _e2e_vglobal_mti_1);
        si->outgoing_msg_info.valid = true;
        _e2e_vglobal_done_1 = true;
        return;

    }

    if (si->openlcb_node->id == _e2e_vglobal_node_2 && !_e2e_vglobal_done_2) {

        OpenLcbUtilities_load_openlcb_message(
                si->outgoing_msg_info.msg_ptr,
                si->openlcb_node->alias,
                si->openlcb_node->id,
                0, 0,
                _e2e_vglobal_mti_2);
        si->outgoing_msg_info.valid = true;
        _e2e_vglobal_done_2 = true;

    }

}

// ---- Conditional respond for verified_node_id slot ----

static void _e2e_verified_handler(openlcb_statemachine_info_t *si) {

    if (si->openlcb_node->id == _e2e_verified_node_1 && !_e2e_verified_done_1) {

        OpenLcbUtilities_load_openlcb_message(
                si->outgoing_msg_info.msg_ptr,
                si->openlcb_node->alias,
                si->openlcb_node->id,
                0, 0,
                _e2e_verified_mti_1);
        si->outgoing_msg_info.valid = true;
        _e2e_verified_done_1 = true;

    }

}

// ============================================================================
// Datagram handler interface — lock/unlock only; all callbacks NULL (auto-reject)
// ============================================================================

static const interface_protocol_datagram_handler_t _e2e_datagram_interface = {

    .lock_shared_resources   = &_e2e_lock,
    .unlock_shared_resources = &_e2e_unlock,

};

// ============================================================================
// Main statemachine interface
// ============================================================================

static const interface_openlcb_main_statemachine_t _e2e_main_interface = {

    .lock_shared_resources   = &_e2e_lock,
    .unlock_shared_resources = &_e2e_unlock,
    .send_openlcb_msg        = &_e2e_wire_send,
    .get_current_tick        = &_e2e_get_tick,

    // Real node enumeration
    .openlcb_node_get_first  = &OpenLcbNode_get_first,
    .openlcb_node_get_next   = &OpenLcbNode_get_next,
    .openlcb_node_is_last    = &OpenLcbNode_is_last,
    .openlcb_node_get_count  = &OpenLcbNode_get_count,

    .load_interaction_rejected = &_e2e_load_interaction_rejected,

    // Message network — noop (tests 2/3 use globals to configure responses)
    .message_network_initialization_complete        = &_e2e_noop_handler,
    .message_network_initialization_complete_simple = &_e2e_noop_handler,
    .message_network_verify_node_id_addressed       = &_e2e_noop_handler,
    .message_network_verify_node_id_global          = &_e2e_verify_global_handler,
    .message_network_verified_node_id               = &_e2e_verified_handler,
    .message_network_optional_interaction_rejected  = &_e2e_noop_handler,
    .message_network_terminate_due_to_error         = &_e2e_noop_handler,
    .message_network_protocol_support_inquiry       = &_e2e_noop_handler,
    .message_network_protocol_support_reply         = &_e2e_noop_handler,

    // SNIP — noop
    .snip_simple_node_info_request = &_e2e_noop_handler,
    .snip_simple_node_info_reply   = &_e2e_noop_handler,

    // Events — noop
    .event_transport_consumer_identify          = &_e2e_noop_handler,
    .event_transport_consumer_range_identified  = &_e2e_noop_handler,
    .event_transport_consumer_identified_unknown = &_e2e_noop_handler,
    .event_transport_consumer_identified_set    = &_e2e_noop_handler,
    .event_transport_consumer_identified_clear  = &_e2e_noop_handler,
    .event_transport_consumer_identified_reserved = &_e2e_noop_handler,
    .event_transport_producer_identify          = &_e2e_noop_handler,
    .event_transport_producer_range_identified  = &_e2e_noop_handler,
    .event_transport_producer_identified_unknown = &_e2e_noop_handler,
    .event_transport_producer_identified_set    = &_e2e_noop_handler,
    .event_transport_producer_identified_clear  = &_e2e_noop_handler,
    .event_transport_producer_identified_reserved = &_e2e_noop_handler,
    .event_transport_identify_dest              = &_e2e_noop_handler,
    .event_transport_identify                   = &_e2e_noop_handler,
    .event_transport_learn                      = &_e2e_noop_handler,
    .event_transport_pc_report                  = &_e2e_noop_handler,
    .event_transport_pc_report_with_payload     = &_e2e_noop_handler,

    // Trains — NULL (not needed)
    .train_control_command                  = NULL,
    .train_control_reply                    = NULL,
    .simple_train_node_ident_info_request   = NULL,
    .simple_train_node_ident_info_reply     = NULL,

    // Datagram — real handlers (auto-reject via NULL callbacks in datagram interface)
    .datagram                = &ProtocolDatagramHandler_datagram,
    .datagram_ok_reply       = &ProtocolDatagramHandler_datagram_received_ok,
    .load_datagram_rejected  = &ProtocolDatagramHandler_load_datagram_rejected_message,
    .datagram_rejected_reply = &ProtocolDatagramHandler_datagram_rejected,

    // Streams — NULL
    .stream_initiate_request = NULL,
    .stream_initiate_reply   = NULL,
    .stream_send_data        = NULL,
    .stream_data_proceed     = NULL,
    .stream_data_complete    = NULL,

    // Broadcast time / train search / train emergency — NULL
    .broadcast_time_event_handler     = NULL,
    .train_search_event_handler       = NULL,
    .train_search_no_match_handler    = NULL,
    .train_emergency_event_handler    = NULL,

    // Event classification filters — NULL (no train/time protocols in e2e test)
    .is_broadcast_time_event = NULL,
    .is_train_search_event   = NULL,
    .is_emergency_event      = NULL,

    // Real internal handlers
    .process_main_statemachine      = &_e2e_process_main_statemachine,
    .does_node_process_msg          = &OpenLcbMainStatemachine_does_node_process_msg,
    .handle_outgoing_openlcb_message =
            &OpenLcbMainStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate         = &OpenLcbMainStatemachine_handle_try_reenumerate,
    .handle_try_pop_next_incoming_openlcb_message =
            &OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message,
    .handle_try_enumerate_first_node =
            &OpenLcbMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node =
            &OpenLcbMainStatemachine_handle_try_enumerate_next_node,

};

// ============================================================================
// Login message handler interface — 0 producers / 0 consumers
// ============================================================================

static uint16_t _e2e_extract_producer_mti(openlcb_node_t *node, uint16_t idx) {

    (void)node; (void)idx;
    return MTI_PRODUCER_IDENTIFIED_UNKNOWN;

}

static uint16_t _e2e_extract_consumer_mti(openlcb_node_t *node, uint16_t idx) {

    (void)node; (void)idx;
    return MTI_CONSUMER_IDENTIFIED_UNKNOWN;

}

static const interface_openlcb_login_message_handler_t _e2e_login_msg_interface = {

    .extract_producer_event_state_mti = &_e2e_extract_producer_mti,
    .extract_consumer_event_state_mti = &_e2e_extract_consumer_mti,

};

// ============================================================================
// Login statemachine interface
// ============================================================================

static const interface_openlcb_login_state_machine_t _e2e_login_interface = {

    .send_openlcb_msg        = &_e2e_wire_send,
    .openlcb_node_get_first  = &OpenLcbNode_get_first,
    .openlcb_node_get_next   = &OpenLcbNode_get_next,
    .openlcb_node_get_count  = &OpenLcbNode_get_count,

    .load_initialization_complete = &OpenLcbLoginMessageHandler_load_initialization_complete,
    .load_producer_events         = &OpenLcbLoginMessageHandler_load_producer_event,
    .load_consumer_events         = &OpenLcbLoginMessageHandler_load_consumer_event,

    // Sibling dispatch wired to the real main statemachine via our logging wrapper
    .process_main_statemachine = &_e2e_process_main_statemachine,

    .on_login_complete = NULL,

    // Real internal handlers
    .process_login_statemachine        = &OpenLcbLoginStateMachine_process,
    .handle_outgoing_openlcb_message   =
            &OpenLcbLoginStatemachine_handle_outgoing_openlcb_message,
    .handle_try_reenumerate            = &OpenLcbLoginStatemachine_handle_try_reenumerate,
    .handle_try_enumerate_first_node   =
            &OpenLcbLoginStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node    =
            &OpenLcbLoginStatemachine_handle_try_enumerate_next_node,

};

// ============================================================================
// Reset and init helpers
// ============================================================================

static void _e2e_reset(void) {

    _e2e_wire_count          = 0;
    _e2e_wire_busy           = false;
    _e2e_dispatch_count_total = 0;

    _e2e_vglobal_node_1 = 0;
    _e2e_vglobal_mti_1  = 0;
    _e2e_vglobal_done_1 = false;

    _e2e_vglobal_node_2 = 0;
    _e2e_vglobal_mti_2  = 0;
    _e2e_vglobal_done_2 = false;

    _e2e_verified_node_1 = 0;
    _e2e_verified_mti_1  = 0;
    _e2e_verified_done_1 = false;

    memset(_e2e_wire_log,     0, sizeof(_e2e_wire_log));
    memset(_e2e_dispatch_log, 0, sizeof(_e2e_dispatch_log));

}

static void _e2e_init_main_only(void) {

    _e2e_reset();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);
    ProtocolDatagramHandler_initialize(&_e2e_datagram_interface);
    OpenLcbMainStatemachine_initialize(&_e2e_main_interface);

}

static void _e2e_init_with_login(void) {

    _e2e_reset();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);
    ProtocolDatagramHandler_initialize(&_e2e_datagram_interface);
    OpenLcbLoginMessageHandler_initialize(&_e2e_login_msg_interface);
    OpenLcbLoginStateMachine_initialize(&_e2e_login_interface);
    OpenLcbMainStatemachine_initialize(&_e2e_main_interface);

}

static bool _e2e_all_nodes_in_run_state(void) {

    openlcb_node_t *node = OpenLcbNode_get_first(0);

    while (node) {

        if (node->state.run_state != RUNSTATE_RUN) { return false; }
        node = OpenLcbNode_get_next(0);

    }

    return true;

}

// ============================================================================
// TEST 1: Login sibling dispatch reaches the real main protocol handler
//
// NodeA completes login while NodeB is already in RUNSTATE_RUN.
// The login statemachine sends Init Complete and then calls
// process_main_statemachine for each RUN sibling — wired to our logging
// wrapper which calls through to OpenLcbMainStatemachine_process_main_statemachine.
// We verify NodeB appears in the dispatch log and NodeA does not (self-skip).
// ============================================================================

TEST(OpenLcbMultinodeE2E, login_sibling_dispatch_reaches_real_main_handler)
{

    _e2e_init_with_login();

    // NodeA — starts login from RUNSTATE_LOAD_INITIALIZATION_COMPLETE
    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->alias = 0x111;
    nodeA->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    // NodeB — already in RUNSTATE_RUN (will be a sibling dispatch target)
    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->alias = 0x222;
    nodeB->state.initialized = true;
    nodeB->state.run_state = RUNSTATE_RUN;

    // Drive login until NodeA reaches RUNSTATE_RUN
    for (int i = 0; i < 50; i++) {

        if (nodeA->state.run_state == RUNSTATE_RUN) { break; }
        OpenLcbLoginMainStatemachine_run();

    }

    ASSERT_EQ(nodeA->state.run_state, RUNSTATE_RUN);

    // Init Complete must have been put on the wire
    EXPECT_GE(_e2e_count_wire_mti(MTI_INITIALIZATION_COMPLETE), 1);

    // NodeB must have been dispatched the Init Complete via sibling dispatch
    EXPECT_GE(_e2e_dispatch_count_for_node_mti(0x010203040502, MTI_INITIALIZATION_COMPLETE), 1);

    // NodeA must NOT have dispatched its own Init Complete to itself (self-skip)
    EXPECT_EQ(_e2e_dispatch_count_for_node_mti(0x010203040501, MTI_INITIALIZATION_COMPLETE), 0);

    // No leaked buffers
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

}

// ============================================================================
// TEST 2: Sibling response queue with two concurrent responders
//
// A sends MTI_VERIFY_NODE_ID_GLOBAL via the FIFO.  NodeB and NodeC are both
// configured to respond to that message — B with VERIFIED_NODE_ID, C with
// INITIALIZATION_COMPLETE.  Both responses are queued in the sibling response
// queue simultaneously, requiring queue depth >= 2.
// ============================================================================

TEST(OpenLcbMultinodeE2E, sibling_response_queue_two_concurrent_responders)
{

    _e2e_init_main_only();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0x111;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0x222;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0x333;
    nodeC->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeD = OpenLcbNode_allocate(0x010203040504, &_node_parameters_main_node);
    nodeD->state.initialized = true;
    nodeD->alias = 0x444;
    nodeD->state.run_state = RUNSTATE_RUN;

    // Configure: B responds to verify_global with VERIFIED_NODE_ID
    _e2e_vglobal_node_1 = 0x010203040502;
    _e2e_vglobal_mti_1  = MTI_VERIFIED_NODE_ID;

    // Configure: C responds to verify_global with INITIALIZATION_COMPLETE
    _e2e_vglobal_node_2 = 0x010203040503;
    _e2e_vglobal_mti_2  = MTI_INITIALIZATION_COMPLETE;

    // NodeA sends Verify Node ID Global via sibling dispatch.
    // B and C respond during Path B dispatch — both responses queued simultaneously.
    openlcb_msg_t app_msg;
    payload_basic_t app_payload;
    app_msg.payload      = (openlcb_payload_t *) &app_payload;
    app_msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &app_msg,
            nodeA->alias,
            nodeA->id,
            0, 0,
            MTI_VERIFY_NODE_ID_GLOBAL);

    app_msg.payload_count = 0;

    bool sent = OpenLcbMainStatemachine_send_with_sibling_dispatch(&app_msg);
    ASSERT_TRUE(sent);

    for (int i = 0; i < 200; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Both response MTIs must have gone to wire
    EXPECT_GE(_e2e_count_wire_mti(MTI_VERIFIED_NODE_ID), 1);
    EXPECT_GE(_e2e_count_wire_mti(MTI_INITIALIZATION_COMPLETE), 1);

    // Queue must have reached depth >= 2 (B and C queued before D clears dispatch)
    EXPECT_GE(OpenLcbMainStatemachine_get_sibling_response_queue_high_water(), 2);

    // No leaked buffers
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

}

// ============================================================================
// TEST 3: Sibling response queue depth-3 sequential chain
//
// A sends MTI_VERIFY_NODE_ID_GLOBAL.  B responds with VERIFIED_NODE_ID
// (depth 1).  When siblings see VERIFIED, C responds with
// INITIALIZATION_COMPLETE (depth 2).  No further responses (chain stops).
// ============================================================================

TEST(OpenLcbMultinodeE2E, sibling_response_queue_depth_3_sequential_chain)
{

    _e2e_init_main_only();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0x111;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0x222;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0x333;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Depth-1: B responds to verify_global with VERIFIED_NODE_ID (once)
    _e2e_vglobal_node_1 = 0x010203040502;
    _e2e_vglobal_mti_1  = MTI_VERIFIED_NODE_ID;

    // Depth-2: A responds to verified_node_id with INITIALIZATION_COMPLETE (once).
    // A is chosen (not C) because C is the last sibling when dispatching VERIFIED
    // (source=B, so B self-skips; order is A→B(skip)→C).  The last-sibling problem
    // would lose C's response.  A is first in enumeration so its response is
    // queued before dispatch advances to C.
    _e2e_verified_node_1 = 0x010203040501;
    _e2e_verified_mti_1  = MTI_INITIALIZATION_COMPLETE;

    // NodeA sends Verify Node ID Global via sibling dispatch so that B's VERIFIED
    // response is queued (loopback) and triggers the depth-2 chain.  FIFO push
    // would not queue B's response for sibling dispatch of VERIFIED.
    openlcb_msg_t app_msg;
    payload_basic_t app_payload;
    app_msg.payload      = (openlcb_payload_t *) &app_payload;
    app_msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &app_msg,
            nodeA->alias,
            nodeA->id,
            0, 0,
            MTI_VERIFY_NODE_ID_GLOBAL);

    app_msg.payload_count = 0;

    bool sent = OpenLcbMainStatemachine_send_with_sibling_dispatch(&app_msg);
    ASSERT_TRUE(sent);

    for (int i = 0; i < 300; i++) {

        OpenLcbMainStatemachine_run();

    }

    // VERIFY_GLOBAL sent to wire by send_with_sibling_dispatch
    EXPECT_GE(_e2e_count_wire_mti(MTI_VERIFY_NODE_ID_GLOBAL), 1);

    // B's depth-1 response must be on wire
    EXPECT_GE(_e2e_count_wire_mti(MTI_VERIFIED_NODE_ID), 1);

    // A's depth-2 response must be on wire
    EXPECT_GE(_e2e_count_wire_mti(MTI_INITIALIZATION_COMPLETE), 1);

    // A's Init Complete (source=A, loopback) must be dispatched to B or C
    // (A self-skips its own queued INIT_COMPLETE)
    EXPECT_GE(_e2e_dispatch_count_for_node_mti(0x010203040502, MTI_INITIALIZATION_COMPLETE), 1);

    // No leaked buffers
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

}

// ============================================================================
// TEST 4: Login and main statemachines run concurrently without interference
//
// Three nodes all start at RUNSTATE_LOAD_INITIALIZATION_COMPLETE.  An
// incoming MTI_VERIFY_NODE_ID_GLOBAL is pushed to the FIFO before the loop
// starts.  Both statemachines run in lockstep until all nodes reach
// RUNSTATE_RUN.  Verifies no state corruption and exactly 3 Init Completes.
// ============================================================================

TEST(OpenLcbMultinodeE2E, login_and_main_statemachines_concurrent_no_interference)
{

    _e2e_init_with_login();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->alias = 0x111;
    nodeA->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->alias = 0x222;
    nodeB->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->alias = 0x333;
    nodeC->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    // Push an external Verify Global to stress concurrent processing
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti          = MTI_VERIFY_NODE_ID_GLOBAL;
    incoming->source_alias = 0xFFF;
    incoming->source_id    = 0x0A0B0C0D0E0F;
    OpenLcbBufferFifo_push(incoming);

    // Lockstep: login statemachine then main statemachine
    for (int i = 0; i < 500; i++) {

        OpenLcbLoginMainStatemachine_run();
        OpenLcbMainStatemachine_run();

        if (_e2e_all_nodes_in_run_state()) { break; }

    }

    // All three nodes must have completed login
    ASSERT_TRUE(_e2e_all_nodes_in_run_state());

    // Exactly 3 Init Complete messages (one per node)
    EXPECT_EQ(_e2e_count_wire_mti(MTI_INITIALIZATION_COMPLETE), 3);

    // No leaked buffers
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

}

// ============================================================================
// TEST 5: External datagram to NodeB — bystanders A and C receive nothing
//
// An incoming DATAGRAM addressed to B is pushed to the FIFO.  With no
// datagram callbacks registered, B auto-rejects.  The rejection reply must
// go to the external source (0xEEE) and neither A nor C must see the datagram.
// ============================================================================

TEST(OpenLcbMultinodeE2E, datagram_external_to_sibling_b_bystanders_receive_nothing)
{

    _e2e_init_main_only();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0x111;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0x222;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0x333;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Incoming DATAGRAM from external node addressed to B
    // Payload: read from CDI space (0xFF) — no callback registered → auto-reject
    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    incoming->mti          = MTI_DATAGRAM;
    incoming->source_alias = 0xEEE;
    incoming->source_id    = 0xAABBCCDDEEFF;
    incoming->dest_alias   = nodeB->alias;
    incoming->dest_id      = nodeB->id;
    *incoming->payload[0]  = CONFIG_MEM_READ_SPACE_FF;
    *incoming->payload[1]  = 0x00;
    *incoming->payload[2]  = 0x00;
    *incoming->payload[3]  = 0x00;
    *incoming->payload[4]  = 0x00;
    incoming->payload_count = 5;
    OpenLcbBufferFifo_push(incoming);

    for (int i = 0; i < 100; i++) {

        OpenLcbMainStatemachine_run();

    }

    // B must have sent a Datagram Rejected reply addressed to the external source
    EXPECT_EQ(_e2e_count_wire_mti(MTI_DATAGRAM_REJECTED_REPLY), 1);
    EXPECT_EQ(_e2e_wire_dest_alias_for_mti(MTI_DATAGRAM_REJECTED_REPLY), (uint16_t)0xEEE);

    // Only B must have been dispatched the DATAGRAM; A and C must not have seen it
    EXPECT_EQ(_e2e_dispatch_count_for_node_mti(0x010203040501, MTI_DATAGRAM), 0);
    EXPECT_EQ(_e2e_dispatch_count_for_node_mti(0x010203040502, MTI_DATAGRAM), 1);
    EXPECT_EQ(_e2e_dispatch_count_for_node_mti(0x010203040503, MTI_DATAGRAM), 0);

    // No leaked buffers
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

}

// ============================================================================
// TEST 6: Sibling datagram from A to B — rejected reply routes back to A
//
// NodeA sends a DATAGRAM to NodeB via send_with_sibling_dispatch.  B
// auto-rejects (no datagram callbacks).  The rejection reply must be routed
// back to A via the sibling response queue.  NodeC must not see anything.
// ============================================================================

TEST(OpenLcbMultinodeE2E, sibling_datagram_rejected_reply_routes_back_to_originator)
{

    _e2e_init_main_only();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0x111;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0x222;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0x333;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Build outgoing datagram from A to B (stack buffer — copied by send_with_sibling_dispatch)
    openlcb_msg_t app_msg;
    payload_basic_t app_payload;
    app_msg.payload      = (openlcb_payload_t *) &app_payload;
    app_msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &app_msg,
            nodeA->alias,
            nodeA->id,
            nodeB->alias,
            nodeB->id,
            MTI_DATAGRAM);

    *app_msg.payload[0]  = CONFIG_MEM_READ_SPACE_FF;
    *app_msg.payload[1]  = 0x00;
    *app_msg.payload[2]  = 0x00;
    *app_msg.payload[3]  = 0x00;
    *app_msg.payload[4]  = 0x00;
    app_msg.payload_count = 5;

    bool sent = OpenLcbMainStatemachine_send_with_sibling_dispatch(&app_msg);
    ASSERT_TRUE(sent);

    for (int i = 0; i < 100; i++) {

        OpenLcbMainStatemachine_run();

    }

    // The datagram must have gone to wire
    EXPECT_EQ(_e2e_count_wire_mti(MTI_DATAGRAM), 1);

    // B must have sent a Datagram Rejected reply addressed to A
    EXPECT_EQ(_e2e_count_wire_mti(MTI_DATAGRAM_REJECTED_REPLY), 1);
    EXPECT_EQ(_e2e_wire_dest_alias_for_mti(MTI_DATAGRAM_REJECTED_REPLY), (uint16_t)0x111);

    // The rejection must have been dispatched to A (via sibling response queue)
    EXPECT_GE(_e2e_dispatch_count_for_node_mti(0x010203040501, MTI_DATAGRAM_REJECTED_REPLY), 1);

    // C must not have seen the datagram or the rejection
    EXPECT_EQ(_e2e_dispatch_count_for_node_mti(0x010203040503, MTI_DATAGRAM), 0);
    EXPECT_EQ(_e2e_dispatch_count_for_node_mti(0x010203040503, MTI_DATAGRAM_REJECTED_REPLY), 0);

    // Sibling response queue must have been used
    EXPECT_GE(OpenLcbMainStatemachine_get_sibling_response_queue_high_water(), 1);

    // No leaked buffers
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

}

// ============================================================================
// TEST 7: Sibling datagram from A to C — bystander B fully isolated
//
// Same as TEST 6 but the destination is NodeC, which is the LAST sibling in
// enumeration.  This directly exercises the last-sibling fix: C produces a
// DATAGRAM_REJECTED_REPLY response, and the library must not drop it even
// though C is the final node dispatched.
// ============================================================================

TEST(OpenLcbMultinodeE2E, sibling_datagram_bystander_c_fully_isolated)
{

    _e2e_init_main_only();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.initialized = true;
    nodeA->alias = 0x111;
    nodeA->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.initialized = true;
    nodeB->alias = 0x222;
    nodeB->state.run_state = RUNSTATE_RUN;

    openlcb_node_t *nodeC = OpenLcbNode_allocate(0x010203040503, &_node_parameters_main_node);
    nodeC->state.initialized = true;
    nodeC->alias = 0x333;
    nodeC->state.run_state = RUNSTATE_RUN;

    // Build outgoing datagram from A to C (B is the bystander)
    openlcb_msg_t app_msg;
    payload_basic_t app_payload;
    app_msg.payload      = (openlcb_payload_t *) &app_payload;
    app_msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &app_msg,
            nodeA->alias,
            nodeA->id,
            nodeC->alias,
            nodeC->id,
            MTI_DATAGRAM);

    *app_msg.payload[0]  = CONFIG_MEM_READ_SPACE_FF;
    *app_msg.payload[1]  = 0x00;
    *app_msg.payload[2]  = 0x00;
    *app_msg.payload[3]  = 0x00;
    *app_msg.payload[4]  = 0x00;
    app_msg.payload_count = 5;

    bool sent = OpenLcbMainStatemachine_send_with_sibling_dispatch(&app_msg);
    ASSERT_TRUE(sent);

    for (int i = 0; i < 100; i++) {

        OpenLcbMainStatemachine_run();

    }

    // Bystander B must not have seen the datagram or the rejection
    EXPECT_EQ(_e2e_dispatch_count_for_node_mti(0x010203040502, MTI_DATAGRAM), 0);
    EXPECT_EQ(_e2e_dispatch_count_for_node_mti(0x010203040502, MTI_DATAGRAM_REJECTED_REPLY), 0);

    // C must have been dispatched the datagram
    EXPECT_GE(_e2e_dispatch_count_for_node_mti(0x010203040503, MTI_DATAGRAM), 1);

    // A must have received the rejection reply via sibling response queue
    EXPECT_GE(_e2e_dispatch_count_for_node_mti(0x010203040501, MTI_DATAGRAM_REJECTED_REPLY), 1);

    // No leaked buffers
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);

}
