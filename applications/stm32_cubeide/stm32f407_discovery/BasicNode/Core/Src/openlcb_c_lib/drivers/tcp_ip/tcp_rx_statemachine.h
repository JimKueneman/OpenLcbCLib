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
 * @file tcp_rx_statemachine.h
 * @brief Receive state machine for the TCP/IP transport layer.
 *
 * @details Accumulates incoming TCP bytes, parses the 17-byte preamble,
 * reassembles multi-part messages, and pushes complete @ref openlcb_msg_t
 * messages to the OpenLCB buffer FIFO.  Link control messages are routed
 * to the link control handler.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_RX_STATEMACHINE__
#define __DRIVERS_TCP_IP_TCP_RX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

/**
 * @brief Dependency-injection interface for the TCP receive state machine.
 *
 * @details All REQUIRED pointers must be non-NULL.
 *
 * @see TcpRxStatemachine_initialize
 */
typedef struct {

    /** @brief REQUIRED. Allocate a message buffer from the OpenLCB store.
     *  Typical: OpenLcbBufferStore_allocate_buffer. */
    openlcb_msg_t *(*allocate_buffer)(payload_type_enum payload_type);

    /** @brief REQUIRED. Free a message buffer back to the OpenLCB store.
     *  Typical: OpenLcbBufferStore_free_buffer. */
    void (*free_buffer)(openlcb_msg_t *msg);

    /** @brief REQUIRED. Push a completed message into the OpenLCB FIFO.
     *  Typical: OpenLcbBufferFifo_push. */
    openlcb_msg_t *(*push_to_fifo)(openlcb_msg_t *msg);

    /** @brief REQUIRED. Handle an incoming link control message.
     *  Typical: TcpLinkControl_handle. */
    void (*handle_link_control)(uint16_t flags, const uint8_t *data, uint16_t len);

    /** @brief REQUIRED. Disable interrupts / acquire mutex. */
    void (*lock_shared_resources)(void);

    /** @brief REQUIRED. Re-enable interrupts / release mutex. */
    void (*unlock_shared_resources)(void);

    /** @brief OPTIONAL. Called when raw bytes arrive, before processing. May be NULL. */
    void (*on_rx)(uint8_t *data, uint16_t len);

} interface_tcp_rx_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface Pointer to a populated @ref interface_tcp_rx_statemachine_t.
     *        Must remain valid for the lifetime of the application.
     *
     * @warning NOT thread-safe — call during single-threaded initialization only.
     */
    extern void TcpRxStatemachine_initialize(const interface_tcp_rx_statemachine_t *interface);

    /**
     * @brief Feeds incoming TCP data into the receive state machine.
     *
     * @details Called from the user's socket receive callback or thread.
     * Accumulates bytes, parses preambles, and pushes complete OpenLCB
     * messages to the FIFO.  May be called with any number of bytes
     * (partial messages are buffered internally).
     *
     * @param data  Pointer to received bytes.
     * @param len   Number of bytes received.
     *
     * @warning May be called from an interrupt context or a dedicated receive
     *          thread.  Uses lock_shared_resources/unlock_shared_resources
     *          around buffer allocation and FIFO push.
     */
    extern void TcpRxStatemachine_incoming_data(uint8_t *data, uint16_t len);

    /**
     * @brief Resets the receive state machine, clearing the accumulation
     * buffer and freeing any in-progress multi-part assemblies.
     *
     * @warning NOT thread-safe — call only when no data is arriving.
     */
    extern void TcpRxStatemachine_reset(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_RX_STATEMACHINE__ */
