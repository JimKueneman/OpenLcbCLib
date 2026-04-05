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
 * @file tcp_config.c
 * @brief Internal wiring for the TCP/IP transport layer.
 *
 * @details Builds all internal DI interface structs from the user-facing
 * tcp_config_t and initializes all TCP sub-modules in the correct order.
 * Analogous to can_config.c for the CAN transport.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_config.h"
#include "tcp_rx_statemachine.h"
#include "tcp_tx_statemachine.h"
#include "tcp_link_control.h"
#include "tcp_login_statemachine.h"
#include "tcp_main_statemachine.h"

#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_config.h"

// =========================================================================
// Module state
// =========================================================================

static const tcp_config_t *_user_config;

// Static interface structs (must persist for application lifetime)
static interface_tcp_rx_statemachine_t _rx_interface;
static interface_tcp_tx_statemachine_t _tx_interface;
static interface_tcp_link_control_t _link_control_interface;
static interface_tcp_login_statemachine_t _login_interface;
static interface_tcp_main_statemachine_t _main_interface;

// =========================================================================
// Internal wiring helpers
// =========================================================================

/**
 * @brief Wrapper to adapt TcpTxStatemachine_send_openlcb_message to the
 * bool (*)(openlcb_msg_t *) signature expected by the protocol layer.
 */
static bool _send_openlcb_msg_wrapper(openlcb_msg_t *msg) {

    return TcpTxStatemachine_send_openlcb_message(msg);
}

static void _build_rx_interface(void) {

    _rx_interface.allocate_buffer = &OpenLcbBufferStore_allocate_buffer;
    _rx_interface.free_buffer = &OpenLcbBufferStore_free_buffer;
    _rx_interface.push_to_fifo = &OpenLcbBufferFifo_push;
    _rx_interface.handle_link_control = &TcpLinkControl_handle;
    _rx_interface.lock_shared_resources = _user_config->lock_shared_resources;
    _rx_interface.unlock_shared_resources = _user_config->unlock_shared_resources;
    _rx_interface.on_rx = _user_config->on_rx;
}

static void _build_tx_interface(void) {

    _tx_interface.transmit_raw_tcp_data = _user_config->transmit_raw_tcp_data;
    _tx_interface.is_tx_buffer_clear = _user_config->is_tx_buffer_clear;
    _tx_interface.get_local_node_id = _user_config->get_local_node_id;
    _tx_interface.get_capture_time_ms = _user_config->get_capture_time_ms;
    _tx_interface.on_tx = _user_config->on_tx;
}

static void _build_link_control_interface(void) {

    _link_control_interface.send_link_control = &TcpTxStatemachine_send_link_control;
    _link_control_interface.get_statemachine_info = &TcpMainStatemachine_get_statemachine_info;
    _link_control_interface.on_link_drop_requested = _user_config->on_link_drop_requested;
    _link_control_interface.on_link_status_changed = _user_config->on_link_status_changed;
}

static void _build_login_interface(void) {

    _login_interface.send_openlcb_msg = &_send_openlcb_msg_wrapper;
    _login_interface.allocate_buffer = &OpenLcbBufferStore_allocate_buffer;
    _login_interface.free_buffer = &OpenLcbBufferStore_free_buffer;
    _login_interface.lock_shared_resources = _user_config->lock_shared_resources;
    _login_interface.unlock_shared_resources = _user_config->unlock_shared_resources;
}

static void _build_main_interface(void) {

    _main_interface.login_link_up = &TcpLoginStatemachine_link_up;
    _main_interface.login_run = &TcpLoginStatemachine_run;
    _main_interface.login_get_state = &TcpLoginStatemachine_get_state;
    _main_interface.link_control_run = &TcpLinkControl_run;
    _main_interface.get_current_tick = &OpenLcbConfig_get_global_100ms_tick;
    _main_interface.on_link_status_changed = _user_config->on_link_status_changed;
}

// =========================================================================
// Public API
// =========================================================================

void TcpConfig_initialize(const tcp_config_t *config) {

    _user_config = config;

    // Build all interface structs
    _build_tx_interface();
    _build_link_control_interface();
    _build_rx_interface();
    _build_login_interface();
    _build_main_interface();

    // Initialize modules in dependency order
    TcpTxStatemachine_initialize(&_tx_interface);
    TcpLinkControl_initialize(&_link_control_interface);
    TcpRxStatemachine_initialize(&_rx_interface);
    TcpLoginStatemachine_initialize(&_login_interface);
    TcpMainStatemachine_initialize(&_main_interface);
}

void TcpConfig_link_up(void) {

    TcpMainStatemachine_link_up();
}

void TcpConfig_link_down(void) {

    TcpRxStatemachine_reset();
    TcpMainStatemachine_link_down();
}

bool TcpConfig_run(void) {

    return TcpMainStatemachine_run();
}

void TcpConfig_incoming_data(uint8_t *data, uint16_t len) {

    TcpRxStatemachine_incoming_data(data, len);
}

bool (*TcpConfig_get_send_openlcb_msg(void))(openlcb_msg_t *msg) {

    return &_send_openlcb_msg_wrapper;
}
