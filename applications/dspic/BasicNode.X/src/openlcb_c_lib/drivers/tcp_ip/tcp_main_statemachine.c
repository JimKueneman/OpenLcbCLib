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
 * @file tcp_main_statemachine.c
 * @brief Implementation of the TCP/IP main state machine.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_main_statemachine.h"

// =========================================================================
// Module state
// =========================================================================

static const interface_tcp_main_statemachine_t *_interface;

static tcp_statemachine_info_t _statemachine_info;

// =========================================================================
// Public API
// =========================================================================

void TcpMainStatemachine_initialize(const interface_tcp_main_statemachine_t *interface) {

    _interface = interface;

    _statemachine_info.openlcb_node = NULL;
    _statemachine_info.login_state = TCP_LOGIN_IDLE;
    _statemachine_info.link_state = TCP_LINK_STATE_DISCONNECTED;
    _statemachine_info.current_tick = 0;
}

void TcpMainStatemachine_link_up(void) {

    _statemachine_info.link_state = TCP_LINK_STATE_LOGGING_IN;
    _statemachine_info.pending_status_reply = 0;
    _statemachine_info.pending_drop_reply = 0;

    _interface->login_link_up();

    if (_interface->on_link_status_changed)
        _interface->on_link_status_changed(true);
}

void TcpMainStatemachine_link_down(void) {

    _statemachine_info.link_state = TCP_LINK_STATE_DISCONNECTED;
    _statemachine_info.login_state = TCP_LOGIN_IDLE;

    if (_interface->on_link_status_changed)
        _interface->on_link_status_changed(false);
}

bool TcpMainStatemachine_run(void) {

    bool busy = false;

    // Drive login sequence
    if (_statemachine_info.link_state == TCP_LINK_STATE_LOGGING_IN) {

        if (_interface->login_run()) {

            busy = true;

        } else if (_interface->login_get_state() == TCP_LOGIN_COMPLETE) {

            _statemachine_info.link_state = TCP_LINK_STATE_RUNNING;

        }

    }

    // Drive link control pending replies
    if (_interface->link_control_run(&_statemachine_info)) {

        busy = true;

    }

    return busy;

}

tcp_link_state_enum TcpMainStatemachine_get_link_state(void) {

    return _statemachine_info.link_state;
}

tcp_statemachine_info_t *TcpMainStatemachine_get_statemachine_info(void) {

    return &_statemachine_info;
}
