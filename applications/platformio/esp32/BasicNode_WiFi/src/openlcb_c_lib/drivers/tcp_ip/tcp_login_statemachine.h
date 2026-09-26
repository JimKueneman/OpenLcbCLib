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
 * @file tcp_login_statemachine.h
 * @brief TCP/IP-specific login state machine.
 *
 * @details Handles the TCP connection setup phase.  Unlike CAN (which requires
 * multi-step alias negotiation), TCP has no transport-level login: once the
 * link is up, each local node goes through the standard OpenLCB login
 * (Initialization Complete, then its identified events) via the protocol
 * layer's login state machine.  This module only tracks link-up so the TCP
 * main statemachine can move the link to RUNNING.
 *
 * It deliberately sends nothing itself.  Message Network 3.4.1 forbids any
 * message before a node's Initialization Complete, and a Verify Node ID Global
 * from the transport would also have no valid source Node ID.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_LOGIN_STATEMACHINE__
#define __DRIVERS_TCP_IP_TCP_LOGIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Resets the module to TCP_LOGIN_IDLE.
     *
     * @warning NOT thread-safe - call during single-threaded initialization only.
     */
    extern void TcpLoginStatemachine_initialize(void);

    /**
     * @brief Signals that the TCP connection has been established.
     *
     * @details Transitions directly to TCP_LOGIN_COMPLETE.  Nothing is sent:
     * node login is handled by the OpenLCB login statemachine.
     */
    extern void TcpLoginStatemachine_link_up(void);

    /**
     * @brief Drives the login state machine.
     *
     * @details There is no pending work in any state.  Kept so the TCP main
     * statemachine's login_run hook has a single shape whether or not a
     * transport ever needs a multi-step login.
     *
     * @return Always false (no work pending).
     */
    extern bool TcpLoginStatemachine_run(void);

    /**
     * @brief Returns the current login state.
     *
     * @return Current @ref tcp_login_state_enum value.
     */
    extern tcp_login_state_enum TcpLoginStatemachine_get_state(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_LOGIN_STATEMACHINE__ */
