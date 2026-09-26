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
 * @file tcp_login_statemachine.c
 * @brief Implementation of the TCP/IP login state machine.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_login_statemachine.h"

#ifdef OPENLCB_COMPILE_TCP

// =========================================================================
// Module state
// =========================================================================

static tcp_login_state_enum _login_state;

// =========================================================================
// Public API
// =========================================================================

    /** @brief Resets the login state to TCP_LOGIN_IDLE. */
void TcpLoginStatemachine_initialize(void) {

    _login_state = TCP_LOGIN_IDLE;

}

    /**
     * @brief Marks the link as up.
     *
     * @details Goes straight to TCP_LOGIN_COMPLETE.  TCP has no transport-level
     * login step: the protocol layer's OpenLCB login statemachine sends
     * Initialization Complete and the identified events for each local node,
     * and nodes enter it immediately (no alias negotiation delay like CAN).
     *
     * Nothing is sent from here.  A node shall not emit any message before its
     * Initialization Complete (Message Network 3.4.1), and the transport has no
     * Node ID of its own to send from.
     */
void TcpLoginStatemachine_link_up(void) {

    _login_state = TCP_LOGIN_COMPLETE;

}

    /** @brief No state has pending work; always returns false. */
bool TcpLoginStatemachine_run(void) {

    return false;

}

    /** @brief Returns the current login state. */
tcp_login_state_enum TcpLoginStatemachine_get_state(void) {

    return _login_state;

}

#endif /* OPENLCB_COMPILE_TCP */
