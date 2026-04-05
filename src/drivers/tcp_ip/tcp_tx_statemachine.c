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
 * @file tcp_tx_statemachine.c
 * @brief Implementation of the TCP/IP transmit state machine.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_tx_statemachine.h"
#include "tcp_utilities.h"
#include "../../openlcb/openlcb_defines.h"

#include <string.h>

// =========================================================================
// Module state
// =========================================================================

static const interface_tcp_tx_statemachine_t *_interface;

static tcp_tx_buffer_t _tx_buffer;

// =========================================================================
// Public API
// =========================================================================

void TcpTxStatemachine_initialize(const interface_tcp_tx_statemachine_t *interface) {

    _interface = interface;
}

bool TcpTxStatemachine_send_openlcb_message(openlcb_msg_t *msg) {

    if (!_interface->is_tx_buffer_clear())
        return false;

    // Calculate message body length:
    //   2 (MTI) + 6 (source NodeID) + [6 (dest NodeID)] + payload
    uint16_t body_len = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN;

    if ((msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT)
        body_len += TCP_BODY_NODE_ID_LEN;

    body_len += msg->payload_count;

    // Check total fits in TX buffer
    uint16_t total_len = TCP_PREAMBLE_LEN + body_len;

    if (total_len > USER_DEFINED_TCP_TX_BUFFER_LEN)
        return false;

    // Build preamble
    uint16_t offset = TcpUtilities_encode_preamble(
            _tx_buffer,
            TCP_FLAGS_MESSAGE,
            body_len,
            _interface->get_local_node_id(),
            _interface->get_capture_time_ms());

    // MTI (2 bytes, big-endian)
    offset += TcpUtilities_encode_uint16(&_tx_buffer[offset], msg->mti);

    // Source Node ID (6 bytes)
    offset += TcpUtilities_encode_node_id(&_tx_buffer[offset], msg->source_id);

    // Destination Node ID (6 bytes, if addressed)
    if ((msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT)
        offset += TcpUtilities_encode_node_id(&_tx_buffer[offset], msg->dest_id);

    // Payload
    if (msg->payload_count > 0 && msg->payload) {

        memcpy(&_tx_buffer[offset], msg->payload, msg->payload_count);
        offset += msg->payload_count;
    }

    // Transmit
    bool result = _interface->transmit_raw_tcp_data(_tx_buffer, offset);

    if (result && _interface->on_tx)
        _interface->on_tx(_tx_buffer, offset);

    return result;
}

bool TcpTxStatemachine_send_link_control(uint16_t flags, const uint8_t *body, uint16_t body_len) {

    uint16_t total_len = TCP_PREAMBLE_LEN + body_len;

    if (total_len > USER_DEFINED_TCP_TX_BUFFER_LEN)
        return false;

    if (!_interface->is_tx_buffer_clear())
        return false;

    // Build preamble with bit 15 clear (link control)
    uint16_t offset = TcpUtilities_encode_preamble(
            _tx_buffer,
            flags & (uint16_t) ~TCP_FLAGS_MESSAGE,
            body_len,
            _interface->get_local_node_id(),
            _interface->get_capture_time_ms());

    // Copy body
    if (body_len > 0 && body) {

        memcpy(&_tx_buffer[offset], body, body_len);
        offset += body_len;
    }

    bool result = _interface->transmit_raw_tcp_data(_tx_buffer, offset);

    if (result && _interface->on_tx)
        _interface->on_tx(_tx_buffer, offset);

    return result;
}
