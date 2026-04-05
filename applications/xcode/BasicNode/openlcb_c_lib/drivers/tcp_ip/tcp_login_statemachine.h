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
 * multi-step alias negotiation), TCP login simply transitions nodes through the
 * standard OpenLCB login sequence via the protocol layer's login state machine.
 *
 * The TCP login state machine is responsible for:
 * 1. Notifying the protocol layer that the link is up
 * 2. Triggering node login through the existing OpenLCB login statemachine
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_LOGIN_STATEMACHINE__
#define __DRIVERS_TCP_IP_TCP_LOGIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

/**
 * @brief Dependency-injection interface for the TCP login state machine.
 *
 * @see TcpLoginStatemachine_initialize
 */
typedef struct {

    /** @brief REQUIRED. Send a Verify Node ID Global to discover remote nodes.
     *  Typical: wired to send through TcpTxStatemachine_send_openlcb_message. */
    bool (*send_openlcb_msg)(openlcb_msg_t *msg);

    /** @brief REQUIRED. Allocate a message buffer.
     *  Typical: OpenLcbBufferStore_allocate_buffer. */
    openlcb_msg_t *(*allocate_buffer)(payload_type_enum payload_type);

    /** @brief REQUIRED. Free a message buffer.
     *  Typical: OpenLcbBufferStore_free_buffer. */
    void (*free_buffer)(openlcb_msg_t *msg);

    /** @brief REQUIRED. Disable interrupts / acquire mutex. */
    void (*lock_shared_resources)(void);

    /** @brief REQUIRED. Re-enable interrupts / release mutex. */
    void (*unlock_shared_resources)(void);

} interface_tcp_login_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface Pointer to a populated @ref interface_tcp_login_statemachine_t.
     *        Must remain valid for the lifetime of the application.
     *
     * @warning NOT thread-safe — call during single-threaded initialization only.
     */
    extern void TcpLoginStatemachine_initialize(const interface_tcp_login_statemachine_t *interface);

    /**
     * @brief Signals that the TCP connection has been established.
     *
     * @details Transitions to TCP_LOGIN_SEND_VERIFY_GLOBAL state.
     * The actual send happens in TcpLoginStatemachine_run().
     */
    extern void TcpLoginStatemachine_link_up(void);

    /**
     * @brief Drives the login state machine.
     *
     * @details If in TCP_LOGIN_SEND_VERIFY_GLOBAL state, attempts to
     * allocate a buffer and send a Verify Node ID Global message.  Retries
     * each cycle until the transport accepts the message.
     *
     * @return true if work is pending (caller should keep calling), false if idle or done.
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
