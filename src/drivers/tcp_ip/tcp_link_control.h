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
 * @file tcp_link_control.h
 * @brief TCP/IP link control message handler.
 *
 * @details Handles link control messages (flags bit 15 = 0): status
 * request/reply and drop link request/reply per the TcpTransferS specification.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_LINK_CONTROL__
#define __DRIVERS_TCP_IP_TCP_LINK_CONTROL__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

/**
 * @brief Dependency-injection interface for TCP link control.
 *
 * @see TcpLinkControl_initialize
 */
typedef struct {

    /** @brief REQUIRED. Send a link control message via the TX path.
     *  Typical: TcpTxStatemachine_send_link_control. */
    bool (*send_link_control)(uint16_t flags, const uint8_t *body, uint16_t body_len);

    /** @brief REQUIRED. Return a pointer to the TCP statemachine info.
     *  Typical: TcpMainStatemachine_get_statemachine_info. */
    tcp_statemachine_info_t *(*get_statemachine_info)(void);

    /** @brief OPTIONAL. Called when a Drop Link Request is received.
     *  The application should close the TCP connection.  May be NULL. */
    void (*on_link_drop_requested)(void);

    /** @brief OPTIONAL. Called when link status changes.  May be NULL. */
    void (*on_link_status_changed)(bool is_up);

} interface_tcp_link_control_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface Pointer to a populated @ref interface_tcp_link_control_t.
     *        Must remain valid for the lifetime of the application.
     *
     * @warning NOT thread-safe — call during single-threaded initialization only.
     */
    extern void TcpLinkControl_initialize(const interface_tcp_link_control_t *interface);

    /**
     * @brief Handles an incoming link control message.
     *
     * @details Called by the RX state machine when a message with flags
     * bit 15 = 0 is received.  Routes to the appropriate handler based
     * on the link control type in the body.
     *
     * @param flags    16-bit flags from the preamble.
     * @param data     Pointer to the link control body bytes (after preamble).
     * @param len      Number of body bytes.
     */
    extern void TcpLinkControl_handle(uint16_t flags, const uint8_t *data, uint16_t len);

    /**
     * @brief Drives the link control run-loop.
     *
     * @details Retries any pending Status Reply or Drop Link Reply.
     * For Drop Link Reply, fires on_link_drop_requested only after
     * the reply has been successfully sent.
     *
     * @param info  Pointer to the TCP statemachine info (carries pending bits).
     *
     * @return true if work is still pending, false if idle.
     */
    extern bool TcpLinkControl_run(tcp_statemachine_info_t *info);

    /**
     * @brief Sends a Status Request message to the peer.
     *
     * @return true on success, false if transmit fails.
     */
    extern bool TcpLinkControl_send_status_request(void);

    /**
     * @brief Sends a Drop Link Request to the peer.
     *
     * @return true on success, false if transmit fails.
     */
    extern bool TcpLinkControl_send_drop_link_request(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_LINK_CONTROL__ */
