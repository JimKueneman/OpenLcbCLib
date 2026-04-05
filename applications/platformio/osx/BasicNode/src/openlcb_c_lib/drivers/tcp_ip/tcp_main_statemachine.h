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
 * @file tcp_main_statemachine.h
 * @brief Main TCP/IP transport state machine — orchestrates link setup and
 * message dispatch.
 *
 * @details Simpler than the CAN main state machine: no alias management,
 * no duplicate detection.  Coordinates link-up/down signaling and drives
 * the TCP login sequence.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_MAIN_STATEMACHINE__
#define __DRIVERS_TCP_IP_TCP_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

/**
 * @brief Dependency-injection interface for the TCP main state machine.
 *
 * @see TcpMainStatemachine_initialize
 */
typedef struct {

    /** @brief REQUIRED. Trigger TCP login (sends Verify Node ID Global).
     *  Typical: TcpLoginStatemachine_link_up. */
    void (*login_link_up)(void);

    /** @brief REQUIRED. Drive the login state machine.
     *  Typical: TcpLoginStatemachine_run. */
    bool (*login_run)(void);

    /** @brief REQUIRED. Get current login state.
     *  Typical: TcpLoginStatemachine_get_state. */
    tcp_login_state_enum (*login_get_state)(void);

    /** @brief REQUIRED. Drive the link control run-loop.
     *  Typical: TcpLinkControl_run. */
    bool (*link_control_run)(tcp_statemachine_info_t *info);

    /** @brief REQUIRED. Return the current 100ms tick counter.
     *  Typical: OpenLcbConfig_get_global_100ms_tick. */
    uint8_t (*get_current_tick)(void);

    /** @brief OPTIONAL. Called when the link state changes. May be NULL. */
    void (*on_link_status_changed)(bool is_up);

} interface_tcp_main_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface Pointer to a populated @ref interface_tcp_main_statemachine_t.
     *        Must remain valid for the lifetime of the application.
     *
     * @warning NOT thread-safe — call during single-threaded initialization only.
     */
    extern void TcpMainStatemachine_initialize(const interface_tcp_main_statemachine_t *interface);

    /**
     * @brief Signals that the TCP connection has been established.
     *
     * @details Transitions the link state to CONNECTING and starts the
     * login sequence.
     */
    extern void TcpMainStatemachine_link_up(void);

    /**
     * @brief Signals that the TCP connection has been lost.
     *
     * @details Transitions the link state to DISCONNECTED.
     */
    extern void TcpMainStatemachine_link_down(void);

    /**
     * @brief Returns the current link state.
     *
     * @return Current @ref tcp_link_state_enum value.
     */
    extern tcp_link_state_enum TcpMainStatemachine_get_link_state(void);

    /**
     * @brief Drives the TCP main state machine run-loop.
     *
     * @details Calls the login run-loop during LOGGING_IN state, and the
     * link control run-loop to flush pending replies.
     *
     * @return true if work is pending, false if idle.
     */
    extern bool TcpMainStatemachine_run(void);

    /**
     * @brief Returns a pointer to the internal statemachine context.
     *
     * @details Intended for unit testing and debugging only.
     *
     * @return Pointer to the internal @ref tcp_statemachine_info_t.
     */
    extern tcp_statemachine_info_t *TcpMainStatemachine_get_statemachine_info(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_MAIN_STATEMACHINE__ */
