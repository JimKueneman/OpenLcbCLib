/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * @file can_config.c
 * @brief Library-internal wiring module for CAN bus transport
 *
 * @details Reads from can_config_t and builds all 7 internal CAN interface
 * structs, then calls all CAN Module_initialize() functions in the correct
 * order. This single file replaces the CAN portion of per-application
 * dependency_injection_canbus.c copies.
 *
 * @author Jim Kueneman
 * @date 17 Feb 2026
 *
 * @see can_config.h - User-facing CAN configuration struct
 * @see openlcb_config.c - OpenLCB protocol layer wiring module
 */

#include "can_config.h"

#include <string.h>

// CAN module headers
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "can_login_message_handler.h"
#include "can_login_statemachine.h"
#include "can_rx_message_handler.h"
#include "can_rx_statemachine.h"
#include "can_tx_message_handler.h"
#include "can_tx_statemachine.h"
#include "can_main_statemachine.h"
#include "alias_mappings.h"

// Cross-layer includes
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_node.h"

// ---- Internal storage for built interface structs ----

static interface_can_login_message_handler_t _login_msg;
static interface_can_login_state_machine_t _login_sm;
static interface_can_rx_message_handler_t _rx_msg;
static interface_can_rx_statemachine_t _rx_sm;
static interface_can_tx_message_handler_t _tx_msg;
static interface_can_tx_statemachine_t _tx_sm;
static interface_can_main_statemachine_t _main_sm;

static const can_config_t *_config;

// ---- Build functions ----

static void _build_login_message_handler(void) {
    memset(&_login_msg, 0, sizeof(_login_msg));

    // Library-internal wiring
    _login_msg.alias_mapping_register = &AliasMappings_register;
    _login_msg.alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias;

    // User callback (optional)
    _login_msg.on_alias_change = _config->on_alias_change;
}

static void _build_login_statemachine(void) {
    memset(&_login_sm, 0, sizeof(_login_sm));

    // Library-internal wiring -- all 10 state handlers
    _login_sm.state_init            = &CanLoginMessageHandler_state_init;
    _login_sm.state_generate_seed   = &CanLoginMessageHandler_state_generate_seed;
    _login_sm.state_generate_alias  = &CanLoginMessageHandler_state_generate_alias;
    _login_sm.state_load_cid07      = &CanLoginMessageHandler_state_load_cid07;
    _login_sm.state_load_cid06      = &CanLoginMessageHandler_state_load_cid06;
    _login_sm.state_load_cid05      = &CanLoginMessageHandler_state_load_cid05;
    _login_sm.state_load_cid04      = &CanLoginMessageHandler_state_load_cid04;
    _login_sm.state_wait_200ms      = &CanLoginMessageHandler_state_wait_200ms;
    _login_sm.state_load_rid        = &CanLoginMessageHandler_state_load_rid;
    _login_sm.state_load_amd        = &CanLoginMessageHandler_state_load_amd;
}

static void _build_rx_message_handler(void) {
    memset(&_rx_msg, 0, sizeof(_rx_msg));

    // Library-internal wiring
    _rx_msg.can_buffer_store_allocate_buffer = &CanBufferStore_allocate_buffer;
    _rx_msg.openlcb_buffer_store_allocate_buffer = &OpenLcbBufferStore_allocate_buffer;
    _rx_msg.alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias;
    _rx_msg.alias_mapping_find_mapping_by_node_id = &AliasMappings_find_mapping_by_node_id;
    _rx_msg.alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info;
    _rx_msg.alias_mapping_set_has_duplicate_alias_flag = &AliasMappings_set_has_duplicate_alias_flag;
}

static void _build_rx_statemachine(void) {
    memset(&_rx_sm, 0, sizeof(_rx_sm));

    // Library-internal wiring -- 12 message handlers
    _rx_sm.handle_can_legacy_snip = &CanRxMessageHandler_can_legacy_snip;
    _rx_sm.handle_single_frame    = &CanRxMessageHandler_single_frame;
    _rx_sm.handle_first_frame     = &CanRxMessageHandler_first_frame;
    _rx_sm.handle_middle_frame    = &CanRxMessageHandler_middle_frame;
    _rx_sm.handle_last_frame      = &CanRxMessageHandler_last_frame;
    _rx_sm.handle_stream_frame    = &CanRxMessageHandler_stream_frame;
    _rx_sm.handle_rid_frame       = CanRxMessageHandler_rid_frame;
    _rx_sm.handle_amd_frame       = CanRxMessageHandler_amd_frame;
    _rx_sm.handle_ame_frame       = CanRxMessageHandler_ame_frame;
    _rx_sm.handle_amr_frame       = CanRxMessageHandler_amr_frame;
    _rx_sm.handle_error_info_report_frame = CanRxMessageHandler_error_info_report_frame;
    _rx_sm.handle_cid_frame       = CanRxMessageHandler_cid_frame;

    // Library-internal wiring -- alias lookup
    _rx_sm.alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias;

    // User callback (optional)
    _rx_sm.on_receive = _config->on_rx;
}

static void _build_tx_message_handler(void) {
    memset(&_tx_msg, 0, sizeof(_tx_msg));

    // User hardware driver (required)
    _tx_msg.transmit_can_frame = _config->transmit_raw_can_frame;

    // User callback (optional)
    _tx_msg.on_transmit = _config->on_tx;
}

static void _build_tx_statemachine(void) {
    memset(&_tx_sm, 0, sizeof(_tx_sm));

    // User hardware driver (required)
    _tx_sm.is_tx_buffer_empty = _config->is_tx_buffer_clear;

    // Library-internal wiring -- 5 message type handlers
    _tx_sm.handle_addressed_msg_frame   = &CanTxMessageHandler_addressed_msg_frame;
    _tx_sm.handle_unaddressed_msg_frame = &CanTxMessageHandler_unaddressed_msg_frame;
    _tx_sm.handle_datagram_frame        = &CanTxMessageHandler_datagram_frame;
    _tx_sm.handle_stream_frame          = &CanTxMessageHandler_stream_frame;
    _tx_sm.handle_can_frame             = &CanTxMessageHandler_can_frame;
}

static void _build_main_statemachine(void) {
    memset(&_main_sm, 0, sizeof(_main_sm));

    // User hardware drivers (required -- duplicated from openlcb_config_t)
    _main_sm.lock_shared_resources   = _config->lock_shared_resources;
    _main_sm.unlock_shared_resources = _config->unlock_shared_resources;

    // Library-internal wiring
    _main_sm.send_can_message = &CanTxStatemachine_send_can_message;
    _main_sm.openlcb_node_get_first = &OpenLcbNode_get_first;
    _main_sm.openlcb_node_get_next  = &OpenLcbNode_get_next;
    _main_sm.openlcb_node_find_by_alias = &OpenLcbNode_find_by_alias;
    _main_sm.login_statemachine_run = &CanLoginStateMachine_run;
    _main_sm.alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info;
    _main_sm.alias_mapping_unregister = &AliasMappings_unregister;

    // Internal handlers (exposed for testability)
    _main_sm.handle_duplicate_aliases = &CanMainStatemachine_handle_duplicate_aliases;
    _main_sm.handle_outgoing_can_message = &CanMainStatemachine_handle_outgoing_can_message;
    _main_sm.handle_login_outgoing_can_message = &CanMainStatemachine_handle_login_outgoing_can_message;
    _main_sm.handle_try_enumerate_first_node = &CanMainStatemachine_handle_try_enumerate_first_node;
    _main_sm.handle_try_enumerate_next_node = &CanMainStatemachine_handle_try_enumerate_next_node;
}

// ---- Public API ----

void CanConfig_initialize(const can_config_t *config) {
    _config = config;

    // 1. Buffer infrastructure
    CanBufferStore_initialize();
    CanBufferFifo_initialize();

    // 2. Build all interface structs from user config
    _build_login_message_handler();
    _build_login_statemachine();
    _build_rx_message_handler();
    _build_rx_statemachine();
    _build_tx_message_handler();
    _build_tx_statemachine();
    _build_main_statemachine();

    // 3. Initialize modules in dependency order
    CanRxMessageHandler_initialize(&_rx_msg);
    CanRxStatemachine_initialize(&_rx_sm);

    CanTxMessageHandler_initialize(&_tx_msg);
    CanTxStatemachine_initialize(&_tx_sm);

    CanLoginMessageHandler_initialize(&_login_msg);
    CanLoginStateMachine_initialize(&_login_sm);
    CanMainStatemachine_initialize(&_main_sm);

    AliasMappings_initialize();
}
