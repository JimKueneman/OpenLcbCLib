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
* @file can_multinode_e2e_Test.cxx
* @brief Multi-node end-to-end integration tests for the CAN transport layer.
*
* @details Covers seams between CAN login, alias management, and listener alias
* repopulation that are only exercised when multiple virtual nodes share a
* single physical CAN interface:
*
*   1. Two nodes complete CAN login concurrently and receive distinct aliases.
*      Exercises the Phase-3A sibling collision prevention in
*      CanLoginMessageHandler_state_generate_alias().
*
*   2. Incoming global AME repopulates listener aliases for local virtual nodes
*      immediately, without waiting for AMD frames off the wire.
*      Exercises the Phase-3B fix in CanRxMessageHandler_ame_frame().
*
*   3. Self-originated global AME (CanMainStatemachine_send_global_alias_enquiry)
*      performs the same flush-then-repopulate sequence and queues the correct
*      wire frames.
*
* All real CAN layer implementations are wired (no mocks for login, alias table,
* or listener table).  listener_flush_aliases and listener_set_alias are wired
* unconditionally regardless of OPENLCB_COMPILE_TRAIN.
*
* @author Jim Kueneman
* @date 17 Mar 2026
*/

#include "test/main_Test.hxx"

#include "can_main_statemachine.h"
#include "can_login_message_handler.h"
#include "can_login_statemachine.h"
#include "can_rx_message_handler.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "alias_mappings.h"
#include "alias_mapping_listener.h"
#include "can_utilities.h"

#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"

#include <cstring>

// ============================================================================
// Node parameters
// ============================================================================

node_parameters_t _node_parameters_main_node = {

    .snip = {
        .mfg_version = 4,
        .name = "E2E CAN Test",
        .model = "E2E CAN Model",
        .hardware_version = "1.0",
        .software_version = "1.0",
        .user_version = 2
    },

    .protocol_support = PSI_SIMPLE_NODE_INFORMATION,

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

};

interface_openlcb_node_t interface_openlcb_node = {};

// ============================================================================
// Wire log — captures every CAN frame sent via the test send hook
// ============================================================================

#define CAN_E2E_WIRE_DEPTH 512

static struct {

    uint32_t identifier;
    uint8_t  payload_count;
    uint8_t  payload[8];

} _can_wire_log[CAN_E2E_WIRE_DEPTH];

static int _can_wire_count;

static bool _can_wire_send(can_msg_t *msg) {

    if (_can_wire_count < CAN_E2E_WIRE_DEPTH) {

        _can_wire_log[_can_wire_count].identifier   = msg->identifier;
        _can_wire_log[_can_wire_count].payload_count = msg->payload_count;

        for (int i = 0; i < msg->payload_count && i < 8; i++) {

            _can_wire_log[_can_wire_count].payload[i] = msg->payload[i];

        }

        _can_wire_count++;

    }

    return true;

}

// Returns true if an AMD frame for the given alias appears in the wire log.
static bool _wire_has_amd_for_alias(uint16_t alias) {

    uint32_t expected = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias;

    for (int i = 0; i < _can_wire_count; i++) {

        if (_can_wire_log[i].identifier == expected) { return true; }

    }

    return false;

}

//============================================================================
// Tick counter — increments on every call so the 200 ms login wait expires
// naturally after ~3 iterations without a wall-clock dependency.
// ============================================================================

static uint8_t _tick_counter;

static uint8_t _can_e2e_get_tick(void) { return _tick_counter++; }

// ============================================================================
// Lock / unlock stubs
// ============================================================================

static void _can_e2e_lock(void)   { }
static void _can_e2e_unlock(void) { }

// ============================================================================
// Login message handler interface — real implementations
// ============================================================================

static const interface_can_login_message_handler_t _can_login_msg_interface = {

    .alias_mapping_register           = &AliasMappings_register,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    .on_alias_change                  = NULL,

};

// ============================================================================
// Login statemachine interface — all 10 real state handlers
// ============================================================================

static const interface_can_login_state_machine_t _can_login_sm_interface = {

    .state_init           = &CanLoginMessageHandler_state_init,
    .state_generate_seed  = &CanLoginMessageHandler_state_generate_seed,
    .state_generate_alias = &CanLoginMessageHandler_state_generate_alias,
    .state_load_cid07     = &CanLoginMessageHandler_state_load_cid07,
    .state_load_cid06     = &CanLoginMessageHandler_state_load_cid06,
    .state_load_cid05     = &CanLoginMessageHandler_state_load_cid05,
    .state_load_cid04     = &CanLoginMessageHandler_state_load_cid04,
    .state_wait_200ms     = &CanLoginMessageHandler_state_wait_200ms,
    .state_load_rid       = &CanLoginMessageHandler_state_load_rid,
    .state_load_amd       = &CanLoginMessageHandler_state_load_amd,

};

// ============================================================================
// RX message handler interface — real implementations
// ============================================================================

static const interface_can_rx_message_handler_t _can_rx_msg_interface = {

    .can_buffer_store_allocate_buffer        = &CanBufferStore_allocate_buffer,
    .openlcb_buffer_store_allocate_buffer    = &OpenLcbBufferStore_allocate_buffer,
    .alias_mapping_find_mapping_by_alias     = &AliasMappings_find_mapping_by_alias,
    .alias_mapping_find_mapping_by_node_id   = &AliasMappings_find_mapping_by_node_id,
    .alias_mapping_get_alias_mapping_info    = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_set_has_duplicate_alias_flag = &AliasMappings_set_has_duplicate_alias_flag,
    .get_current_tick                        = &_can_e2e_get_tick,

    // Listener alias management — wired unconditionally (no OPENLCB_COMPILE_TRAIN guard)
    .listener_register          = &ListenerAliasTable_register,
    .listener_set_alias         = &ListenerAliasTable_set_alias,
    .listener_clear_alias_by_alias = &ListenerAliasTable_clear_alias_by_alias,
    .listener_flush_aliases     = &ListenerAliasTable_flush_aliases,

};

// ============================================================================
// Main statemachine interface — real implementations; listener hooks wired
// unconditionally (no OPENLCB_COMPILE_TRAIN guard)
// ============================================================================

static const interface_can_main_statemachine_t _can_main_interface = {

    .lock_shared_resources   = &_can_e2e_lock,
    .unlock_shared_resources = &_can_e2e_unlock,
    .send_can_message        = &_can_wire_send,

    .openlcb_node_get_first     = &OpenLcbNode_get_first,
    .openlcb_node_get_next      = &OpenLcbNode_get_next,
    .openlcb_node_find_by_alias = &OpenLcbNode_find_by_alias,

    .login_statemachine_run              = &CanLoginStateMachine_run,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_unregister            = &AliasMappings_unregister,

    .get_current_tick = &_can_e2e_get_tick,

    .handle_duplicate_aliases          = &CanMainStatemachine_handle_duplicate_aliases,
    .handle_outgoing_can_message       = &CanMainStatemachine_handle_outgoing_can_message,
    .handle_login_outgoing_can_message = &CanMainStatemachine_handle_login_outgoing_can_message,
    .handle_try_enumerate_first_node   = &CanMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node    = &CanMainStatemachine_handle_try_enumerate_next_node,
    .handle_listener_verification      = &CanMainStatemachine_handle_listener_verification,

    // Listener alias management for self-originated global AME
    .listener_flush_aliases = &ListenerAliasTable_flush_aliases,
    .listener_set_alias     = &ListenerAliasTable_set_alias,

};

// ============================================================================
// Reset and init helpers
// ============================================================================

static void _can_e2e_reset(void) {

    _can_wire_count = 0;
    _tick_counter   = 0;
    memset(_can_wire_log, 0, sizeof(_can_wire_log));

}

static void _can_e2e_init(void) {

    _can_e2e_reset();

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    AliasMappings_initialize();
    ListenerAliasTable_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);

    CanLoginMessageHandler_initialize(&_can_login_msg_interface);
    CanLoginStateMachine_initialize(&_can_login_sm_interface);
    CanRxMessageHandler_initialize(&_can_rx_msg_interface);
    CanMainStatemachine_initialize(&_can_main_interface);

}

// Returns true once every allocated node has a permitted entry in the alias
// mapping table (i.e. CAN login is complete for all nodes).
static bool _all_nodes_can_login_complete(void) {

    openlcb_node_t *node = OpenLcbNode_get_first(0);

    while (node) {

        alias_mapping_t *m = AliasMappings_find_mapping_by_node_id(node->id);

        if (!m || !m->is_permitted) { return false; }

        node = OpenLcbNode_get_next(0);

    }

    return true;

}

// ============================================================================
// TEST 1: Two nodes complete CAN login concurrently with distinct aliases
//
// NodeA and NodeB both start in RUNSTATE_INIT.  The CAN main statemachine
// enumerates them round-robin and drives each through the 10-state login
// sequence.  Phase-3A collision prevention in state_generate_alias() ensures
// that if both nodes would derive the same 12-bit alias (same LFSR path),
// the second node re-seeds until it finds an unoccupied alias.
//
// Observable: both nodes are permitted, aliases are non-zero and distinct,
// and AMD frames for both appear on the wire.
// ============================================================================

TEST(CanMultinodeE2E, two_nodes_login_with_distinct_aliases)
{

    _can_e2e_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.run_state = RUNSTATE_INIT;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.run_state = RUNSTATE_INIT;

    for (int i = 0; i < 2000; i++) {

        CanMainStateMachine_run();

        if (_all_nodes_can_login_complete()) { break; }

    }

    // Drain any pending login frames: state_load_amd sets is_permitted and queues the
    // AMD in login_outgoing_can_msg, but handle_login_outgoing_can_message needs one
    // more run() cycle to actually send it to the wire.
    for (int i = 0; i < 50; i++) {

        CanMainStateMachine_run();

    }

    ASSERT_TRUE(_all_nodes_can_login_complete());

    // Both aliases must be non-zero
    EXPECT_NE(nodeA->alias, 0);
    EXPECT_NE(nodeB->alias, 0);

    // Aliases must be distinct — collision prevention worked
    EXPECT_NE(nodeA->alias, nodeB->alias);

    // Both must appear in the alias mapping table as permitted
    alias_mapping_t *mappingA = AliasMappings_find_mapping_by_node_id(nodeA->id);
    alias_mapping_t *mappingB = AliasMappings_find_mapping_by_node_id(nodeB->id);

    ASSERT_NE(mappingA, nullptr);
    ASSERT_NE(mappingB, nullptr);
    EXPECT_TRUE(mappingA->is_permitted);
    EXPECT_TRUE(mappingB->is_permitted);

    // AMD frames for both must have gone to the wire
    EXPECT_TRUE(_wire_has_amd_for_alias(nodeA->alias));
    EXPECT_TRUE(_wire_has_amd_for_alias(nodeB->alias));

}

// ============================================================================
// TEST 2: Incoming global AME repopulates local listener aliases immediately
//
// After login, nodeA and nodeB are registered in the listener table with
// their resolved aliases.  An incoming global AME (payload_count == 0) is
// delivered directly to CanRxMessageHandler_ame_frame().  The Phase-3B fix
// inside that handler must:
//   a) flush all listener aliases
//   b) immediately repopulate entries for local virtual nodes from the alias
//      mapping table (without waiting for AMD frames off the wire)
//   c) send AMD frames for all permitted local nodes
//
// After the call, both listener aliases must be non-zero (repopulated) and
// AMD frames for both nodes must appear in the wire log.
// ============================================================================

TEST(CanMultinodeE2E, incoming_global_ame_repopulates_local_listener_aliases)
{

    _can_e2e_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.run_state = RUNSTATE_INIT;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.run_state = RUNSTATE_INIT;

    for (int i = 0; i < 2000; i++) {

        CanMainStateMachine_run();

        if (_all_nodes_can_login_complete()) { break; }

    }

    ASSERT_TRUE(_all_nodes_can_login_complete());
    ASSERT_NE(nodeA->alias, 0);
    ASSERT_NE(nodeB->alias, 0);

    // Register both nodes as listeners and populate their aliases
    ListenerAliasTable_register(nodeA->id);
    ListenerAliasTable_set_alias(nodeA->id, nodeA->alias);
    ListenerAliasTable_register(nodeB->id);
    ListenerAliasTable_set_alias(nodeB->id, nodeB->alias);

    // Confirm pre-condition: both aliases are populated
    listener_alias_entry_t *entryA = ListenerAliasTable_find_by_node_id(nodeA->id);
    listener_alias_entry_t *entryB = ListenerAliasTable_find_by_node_id(nodeB->id);
    ASSERT_NE(entryA, nullptr);
    ASSERT_NE(entryB, nullptr);
    ASSERT_NE(entryA->alias, 0);
    ASSERT_NE(entryB->alias, 0);

    // Reset wire log so we can count only what the AME generates
    _can_wire_count = 0;

    // Deliver an incoming global AME (empty payload = global scope)
    can_msg_t *global_ame = CanBufferStore_allocate_buffer();
    ASSERT_NE(global_ame, nullptr);
    global_ame->identifier   = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AME;
    global_ame->payload_count = 0;

    CanRxMessageHandler_ame_frame(global_ame);

    CanBufferStore_free_buffer(global_ame);

    // Both listener aliases must be repopulated immediately — no AMD round-trip needed
    EXPECT_NE(entryA->alias, 0);
    EXPECT_NE(entryB->alias, 0);
    EXPECT_EQ(entryA->alias, nodeA->alias);
    EXPECT_EQ(entryB->alias, nodeB->alias);

    // AMD frames for both nodes must have been queued (still in FIFO, not yet sent)
    // Drain the CAN outgoing FIFO so they appear in the wire log
    for (int i = 0; i < 100; i++) {

        CanMainStateMachine_run();

    }

    EXPECT_TRUE(_wire_has_amd_for_alias(nodeA->alias));
    EXPECT_TRUE(_wire_has_amd_for_alias(nodeB->alias));

}

// ============================================================================
// TEST 3: Self-originated global AME repopulates listener aliases and queues
//         the correct wire frames
//
// Same listener setup as TEST 2.  CanMainStatemachine_send_global_alias_enquiry()
// is called directly (the application calls this when it needs to re-announce
// all local nodes).  The function must:
//   a) flush listener aliases
//   b) immediately repopulate local virtual node entries
//   c) queue AMD frames for each permitted local node
//   d) queue a global AME frame (triggers AMD responses from external nodes)
//
// After draining the CAN FIFO, the wire log must contain AMD frames for both
// nodes and exactly one global AME.
// ============================================================================

TEST(CanMultinodeE2E, self_originated_global_ame_repopulates_listener_and_queues_wire_frames)
{

    _can_e2e_init();

    openlcb_node_t *nodeA = OpenLcbNode_allocate(0x010203040501, &_node_parameters_main_node);
    nodeA->state.run_state = RUNSTATE_INIT;

    openlcb_node_t *nodeB = OpenLcbNode_allocate(0x010203040502, &_node_parameters_main_node);
    nodeB->state.run_state = RUNSTATE_INIT;

    for (int i = 0; i < 2000; i++) {

        CanMainStateMachine_run();

        if (_all_nodes_can_login_complete()) { break; }

    }

    ASSERT_TRUE(_all_nodes_can_login_complete());
    ASSERT_NE(nodeA->alias, 0);
    ASSERT_NE(nodeB->alias, 0);

    // Register both nodes as listeners and populate their aliases
    ListenerAliasTable_register(nodeA->id);
    ListenerAliasTable_set_alias(nodeA->id, nodeA->alias);
    ListenerAliasTable_register(nodeB->id);
    ListenerAliasTable_set_alias(nodeB->id, nodeB->alias);

    listener_alias_entry_t *entryA = ListenerAliasTable_find_by_node_id(nodeA->id);
    listener_alias_entry_t *entryB = ListenerAliasTable_find_by_node_id(nodeB->id);
    ASSERT_NE(entryA, nullptr);
    ASSERT_NE(entryB, nullptr);

    // Reset wire log
    _can_wire_count = 0;

    // Self-originated global AME: flush + repopulate + queue AMD + queue AME
    CanMainStatemachine_send_global_alias_enquiry();

    // Listener aliases must be repopulated synchronously (before run())
    EXPECT_NE(entryA->alias, 0);
    EXPECT_NE(entryB->alias, 0);
    EXPECT_EQ(entryA->alias, nodeA->alias);
    EXPECT_EQ(entryB->alias, nodeB->alias);

    // Drain the CAN outgoing FIFO
    for (int i = 0; i < 200; i++) {

        CanMainStateMachine_run();

    }

    // AMD frames for both nodes must be on the wire
    EXPECT_TRUE(_wire_has_amd_for_alias(nodeA->alias));
    EXPECT_TRUE(_wire_has_amd_for_alias(nodeB->alias));

    // Exactly one global AME frame must be on the wire
    // Global AME has no alias bits set: identifier == RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AME
    int global_ame_count = 0;

    for (int i = 0; i < _can_wire_count; i++) {

        if (_can_wire_log[i].identifier == (RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AME)) {

            global_ame_count++;

        }

    }

    EXPECT_EQ(global_ame_count, 1);

}
