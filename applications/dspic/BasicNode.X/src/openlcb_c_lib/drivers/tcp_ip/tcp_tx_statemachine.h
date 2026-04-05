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
 * @file tcp_tx_statemachine.h
 * @brief Transmit state machine for the TCP/IP transport layer.
 *
 * @details Converts @ref openlcb_msg_t messages into TCP/IP wire format
 * (17-byte preamble + message body) and sends them via the user's transmit
 * callback.  No fragmentation is needed since TCP is a byte stream.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_TX_STATEMACHINE__
#define __DRIVERS_TCP_IP_TCP_TX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

/**
 * @brief Dependency-injection interface for the TCP transmit state machine.
 *
 * @details All REQUIRED pointers must be non-NULL.
 *
 * @see TcpTxStatemachine_initialize
 */
typedef struct {

    /** @brief REQUIRED. Transmit raw bytes over the TCP connection.
     *  Returns true on success, false if the socket is unavailable. */
    bool (*transmit_raw_tcp_data)(uint8_t *data, uint16_t len);

    /** @brief REQUIRED. Check if the TCP TX buffer can accept more data. */
    bool (*is_tx_buffer_clear)(void);

    /** @brief REQUIRED. Get the local Node ID for the originating-node field. */
    node_id_t (*get_local_node_id)(void);

    /** @brief REQUIRED. Get the monotonic capture time in milliseconds. */
    uint64_t (*get_capture_time_ms)(void);

    /** @brief OPTIONAL. Called after a message is transmitted. May be NULL. */
    void (*on_tx)(uint8_t *data, uint16_t len);

} interface_tcp_tx_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface Pointer to a populated @ref interface_tcp_tx_statemachine_t.
     *        Must remain valid for the lifetime of the application.
     *
     * @warning NOT thread-safe — call during single-threaded initialization only.
     */
    extern void TcpTxStatemachine_initialize(const interface_tcp_tx_statemachine_t *interface);

    /**
     * @brief Converts an OpenLCB message to TCP wire format and transmits it.
     *
     * @details Builds the 17-byte preamble followed by MTI, source Node ID,
     * optional destination Node ID, and payload bytes.  Calls the user's
     * transmit_raw_tcp_data() callback with the complete byte sequence.
     *
     * This function is wired as the transport's send_openlcb_msg() in the
     * OpenLCB main state machine interface when OPENLCB_COMPILE_TCP is defined.
     *
     * @param msg  Pointer to the message to send.  The caller retains ownership
     *             of the buffer (this function does not free it).
     *
     * @return true on success, false if TX buffer is busy or transmit fails.
     */
    extern bool TcpTxStatemachine_send_openlcb_message(openlcb_msg_t *msg);

    /**
     * @brief Sends raw bytes with a TCP/IP OpenLCB preamble (link control).
     *
     * @details Used by TcpLinkControl to send link-level messages (status,
     * drop link).  The flags field has bit 15 clear (not an OpenLCB message).
     *
     * @param flags     16-bit flags (bit 15 = 0 for link control).
     * @param body      Pointer to the link control body bytes.
     * @param body_len  Number of body bytes.
     *
     * @return true on success, false if transmit fails.
     */
    extern bool TcpTxStatemachine_send_link_control(uint16_t flags, const uint8_t *body, uint16_t body_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_TX_STATEMACHINE__ */
