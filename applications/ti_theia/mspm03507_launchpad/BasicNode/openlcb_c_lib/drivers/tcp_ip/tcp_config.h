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
 * @file tcp_config.h
 * @brief User-facing TCP/IP transport configuration.
 *
 * @details Users provide their hardware-specific TCP driver functions here.
 * All other TCP-internal wiring is handled automatically by tcp_config.c.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 *
 * @see openlcb_config.h — OpenLCB protocol layer configuration
 * @see can_config.h — CAN transport configuration (alternative transport)
 */

#ifndef __DRIVERS_TCP_IP_TCP_CONFIG__
#define __DRIVERS_TCP_IP_TCP_CONFIG__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief TCP/IP transport configuration.
     *
     * @details Users provide their hardware-specific TCP driver functions here.
     * All other TCP-internal wiring is handled automatically.
     *
     * Example:
     * @code
     * static const tcp_config_t tcp_config = {
     *     .transmit_raw_tcp_data   = &MyTcpDriver_send,
     *     .is_tx_buffer_clear      = &MyTcpDriver_is_tx_ready,
     *     .get_local_node_id       = &MyApp_get_node_id,
     *     .get_capture_time_ms     = &MyApp_get_uptime_ms,
     *     .lock_shared_resources   = &MyDriver_lock,
     *     .unlock_shared_resources = &MyDriver_unlock,
     *     .on_rx                   = &my_tcp_rx_handler,    // optional
     *     .on_tx                   = &my_tcp_tx_handler,    // optional
     *     .on_link_status_changed  = &my_link_handler,      // optional
     * };
     *
     * TcpConfig_initialize(&tcp_config);
     * @endcode
     */
    typedef struct {

        /** @brief Transmit raw bytes over the TCP connection. REQUIRED.
         *  Returns true on success, false if the socket is unavailable. */
        bool (*transmit_raw_tcp_data)(uint8_t *data, uint16_t len);

        /** @brief Check if TCP TX buffer can accept another message. REQUIRED. */
        bool (*is_tx_buffer_clear)(void);

        /** @brief Return the local Node ID for the originating-node preamble field. REQUIRED. */
        node_id_t (*get_local_node_id)(void);

        /** @brief Return a monotonic capture time in milliseconds. REQUIRED.
         *  Used in the preamble capture-time field.  Only monotonicity is required,
         *  not wall-clock accuracy. */
        uint64_t (*get_capture_time_ms)(void);

        /** @brief Disable interrupts / acquire mutex for shared resource access. REQUIRED.
         *  Same function as openlcb_config_t.lock_shared_resources. */
        void (*lock_shared_resources)(void);

        /** @brief Re-enable interrupts / release mutex. REQUIRED.
         *  Same function as openlcb_config_t.unlock_shared_resources. */
        void (*unlock_shared_resources)(void);

        /** @brief Called when raw TCP bytes are received. Optional. */
        void (*on_rx)(uint8_t *data, uint16_t len);

        /** @brief Called when raw TCP bytes are transmitted. Optional. */
        void (*on_tx)(uint8_t *data, uint16_t len);

        /** @brief Called when the TCP link state changes. Optional. */
        void (*on_link_status_changed)(bool is_up);

        /** @brief Called when the peer requests a link drop. Optional.
         *  The application should close the TCP socket. */
        void (*on_link_drop_requested)(void);

    } tcp_config_t;

    /**
     * @brief Initializes the TCP/IP transport layer.
     *
     * @details Must be called BEFORE OpenLcbConfig_initialize().
     *
     * @param config  Pointer to @ref tcp_config_t configuration.  Must remain
     *                valid for the lifetime of the application.
     *
     * @warning NOT thread-safe — call during single-threaded initialization only.
     *
     * @see openlcb_config.h
     */
    extern void TcpConfig_initialize(const tcp_config_t *config);

    /**
     * @brief Signals that the TCP connection has been established.
     *
     * @details Call after successfully connecting the TCP socket.
     * Starts the login sequence (Verify Node ID Global, etc.).
     */
    extern void TcpConfig_link_up(void);

    /**
     * @brief Signals that the TCP connection has been lost.
     *
     * @details Call when the socket disconnects or an error occurs.
     */
    extern void TcpConfig_link_down(void);

    /**
     * @brief Drives the TCP transport run-loop.
     *
     * @details Call repeatedly from the application's main loop.
     * Handles login retries and pending link control replies.
     *
     * @return true if work is pending, false if idle.
     */
    extern bool TcpConfig_run(void);

    /**
     * @brief Feeds incoming TCP data into the receive state machine.
     *
     * @details Call from the user's socket receive callback or thread.
     *
     * @param data  Pointer to received bytes.
     * @param len   Number of bytes received.
     */
    extern void TcpConfig_incoming_data(uint8_t *data, uint16_t len);

    /**
     * @brief Returns the transmit function for use by the protocol layer.
     *
     * @details Used by openlcb_config.c to wire the send_openlcb_msg pointer
     * when OPENLCB_COMPILE_TCP is defined.
     *
     * @return Function pointer matching the send_openlcb_msg signature.
     */
    extern bool (*TcpConfig_get_send_openlcb_msg(void))(openlcb_msg_t *msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_CONFIG__ */
