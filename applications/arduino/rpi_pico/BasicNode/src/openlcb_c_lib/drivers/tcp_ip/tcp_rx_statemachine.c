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
 * @file tcp_rx_statemachine.c
 * @brief Implementation of the TCP/IP receive state machine.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_rx_statemachine.h"
#include "tcp_utilities.h"
#include "../../openlcb/openlcb_defines.h"

#include <string.h>

// =========================================================================
// Module state
// =========================================================================

static const interface_tcp_rx_statemachine_t *_interface;

static tcp_rx_accumulation_buffer_t _rx_buffer;
static uint16_t _rx_count;

static tcp_multipart_table_t _multipart_table;

// =========================================================================
// Internal helpers
// =========================================================================

/**
 * @brief Selects the smallest payload type that can hold the given byte count.
 */
static payload_type_enum _select_payload_type(uint16_t byte_count) {

    if (byte_count <= LEN_MESSAGE_BYTES_BASIC)
        return BASIC;
    if (byte_count <= LEN_MESSAGE_BYTES_DATAGRAM)
        return DATAGRAM;
    if (byte_count <= LEN_MESSAGE_BYTES_SNIP)
        return SNIP;
    return STREAM;
}

/**
 * @brief Finds or allocates a multi-part slot for the given originating Node ID.
 *
 * @param node_id  Originating Node ID from the preamble.
 * @param is_first true if this is the first part (allocates a new slot).
 *
 * @return Pointer to the slot, or NULL if table is full / not found.
 */
static tcp_multipart_entry_t *_find_multipart_slot(node_id_t node_id, bool is_first) {

    if (is_first) {

        for (uint8_t i = 0; i < USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES; i++) {

            if (_multipart_table[i].originating_node_id == 0) {
                _multipart_table[i].originating_node_id = node_id;
                return &_multipart_table[i];
            }
        }
        return NULL;
    }

    for (uint8_t i = 0; i < USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES; i++) {

        if (_multipart_table[i].originating_node_id == node_id)
            return &_multipart_table[i];
    }
    return NULL;
}

/**
 * @brief Frees a multi-part slot.
 */
static void _free_multipart_slot(tcp_multipart_entry_t *slot) {

    slot->originating_node_id = 0;
    slot->openlcb_msg = NULL;
}

/**
 * @brief Parses a complete message body into an openlcb_msg_t and pushes to FIFO.
 *
 * @param body      Pointer to the message body (after preamble).
 * @param body_len  Length of the body in bytes.
 */
static void _forward_complete_message(const uint8_t *body, uint16_t body_len) {

    if (body_len < TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN)
        return;

    uint16_t mti = TcpUtilities_decode_mti(body);
    node_id_t source_id = TcpUtilities_decode_node_id(&body[TCP_BODY_OFFSET_SOURCE_NODE_ID]);

    node_id_t dest_id = 0;
    uint16_t data_offset = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN;

    if ((mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT) {

        if (body_len < TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN + TCP_BODY_NODE_ID_LEN)
            return;

        dest_id = TcpUtilities_decode_node_id(&body[TCP_BODY_OFFSET_DEST_NODE_ID]);
        data_offset = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN + TCP_BODY_NODE_ID_LEN;
    }

    uint16_t payload_len = (body_len > data_offset) ? (body_len - data_offset) : 0;

    payload_type_enum ptype = _select_payload_type(payload_len);

    _interface->lock_shared_resources();
    openlcb_msg_t *msg = _interface->allocate_buffer(ptype);
    _interface->unlock_shared_resources();

    if (!msg)
        return;

    msg->mti = mti;
    msg->source_id = source_id;
    msg->dest_id = dest_id;
    msg->source_alias = 0;
    msg->dest_alias = 0;
    msg->payload_count = 0;

    if (payload_len > 0 && msg->payload) {

        uint16_t max_payload;

        switch (ptype) {
            case BASIC:    max_payload = LEN_MESSAGE_BYTES_BASIC;    break;
            case DATAGRAM: max_payload = LEN_MESSAGE_BYTES_DATAGRAM; break;
            case SNIP:     max_payload = LEN_MESSAGE_BYTES_SNIP;     break;
            default:       max_payload = LEN_MESSAGE_BYTES_STREAM;   break;
        }

        if (payload_len > max_payload)
            payload_len = max_payload;

        memcpy(msg->payload, &body[data_offset], payload_len);
        msg->payload_count = payload_len;
    }

    _interface->lock_shared_resources();
    _interface->push_to_fifo(msg);
    _interface->unlock_shared_resources();
}

/**
 * @brief Processes one complete TCP message (preamble + body) from the
 * accumulation buffer.
 *
 * @param preamble_start  Pointer to the start of the preamble in _rx_buffer.
 * @param total_len       Total bytes consumed (5 + length field value).
 */
static void _process_message(const uint8_t *preamble_start, uint32_t total_len) {

    uint16_t flags = TcpUtilities_decode_flags(preamble_start);
    uint32_t length = TcpUtilities_decode_length(preamble_start);

    if (!TcpUtilities_is_openlcb_message(flags)) {

        uint16_t body_len = (length > 12) ? (uint16_t) (length - 12) : 0;

        _interface->handle_link_control(
                flags,
                &preamble_start[TCP_PREAMBLE_LEN],
                body_len);
        return;
    }

    const uint8_t *body = &preamble_start[TCP_PREAMBLE_LEN];
    uint16_t body_len = (length > 12) ? (uint16_t) (length - 12) : 0;

    uint16_t multipart = TcpUtilities_multipart_type(flags);

    if (multipart == TCP_FLAGS_MULTIPART_SINGLE) {

        _forward_complete_message(body, body_len);

    } else {

        node_id_t orig_id = TcpUtilities_decode_originating_node_id(preamble_start);

        if (multipart == TCP_FLAGS_MULTIPART_FIRST) {

            tcp_multipart_entry_t *slot = _find_multipart_slot(orig_id, true);

            if (!slot)
                return;

            if (body_len < TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN)
                return;

            uint16_t mti = TcpUtilities_decode_mti(body);
            node_id_t source_id = TcpUtilities_decode_node_id(&body[TCP_BODY_OFFSET_SOURCE_NODE_ID]);

            node_id_t dest_id = 0;
            uint16_t data_offset = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN;

            if ((mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT) {
                if (body_len >= TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN + TCP_BODY_NODE_ID_LEN) {
                    dest_id = TcpUtilities_decode_node_id(&body[TCP_BODY_OFFSET_DEST_NODE_ID]);
                    data_offset = TCP_BODY_MTI_LEN + TCP_BODY_NODE_ID_LEN + TCP_BODY_NODE_ID_LEN;
                }
            }

            uint16_t payload_len = (body_len > data_offset) ? (body_len - data_offset) : 0;

            _interface->lock_shared_resources();
            slot->openlcb_msg = _interface->allocate_buffer(SNIP);
            _interface->unlock_shared_resources();

            if (!slot->openlcb_msg) {
                _free_multipart_slot(slot);
                return;
            }

            slot->openlcb_msg->mti = mti;
            slot->openlcb_msg->source_id = source_id;
            slot->openlcb_msg->dest_id = dest_id;
            slot->openlcb_msg->source_alias = 0;
            slot->openlcb_msg->dest_alias = 0;
            slot->openlcb_msg->payload_count = 0;

            if (payload_len > 0 && slot->openlcb_msg->payload) {
                uint16_t max_p = LEN_MESSAGE_BYTES_SNIP;
                if (payload_len > max_p) payload_len = max_p;
                memcpy(slot->openlcb_msg->payload, &body[data_offset], payload_len);
                slot->openlcb_msg->payload_count = payload_len;
            }

        } else if (multipart == TCP_FLAGS_MULTIPART_MIDDLE ||
                   multipart == TCP_FLAGS_MULTIPART_LAST) {

            tcp_multipart_entry_t *slot = _find_multipart_slot(orig_id, false);

            if (!slot || !slot->openlcb_msg)
                return;

            if (body_len > 0 && slot->openlcb_msg->payload) {

                uint16_t max_p = LEN_MESSAGE_BYTES_SNIP;
                uint16_t remaining = (max_p > slot->openlcb_msg->payload_count) ?
                    (max_p - slot->openlcb_msg->payload_count) : 0;
                uint16_t copy_len = (body_len < remaining) ? body_len : remaining;

                if (copy_len > 0) {
                    memcpy(&((uint8_t *) slot->openlcb_msg->payload)[slot->openlcb_msg->payload_count],
                           body, copy_len);
                    slot->openlcb_msg->payload_count += copy_len;
                }
            }

            if (multipart == TCP_FLAGS_MULTIPART_LAST) {

                _interface->lock_shared_resources();
                _interface->push_to_fifo(slot->openlcb_msg);
                _interface->unlock_shared_resources();

                _free_multipart_slot(slot);
            }
        }
    }
}

// =========================================================================
// Public API
// =========================================================================

void TcpRxStatemachine_initialize(const interface_tcp_rx_statemachine_t *interface) {

    _interface = interface;
    _rx_count = 0;
    memset(_multipart_table, 0, sizeof(_multipart_table));
}

void TcpRxStatemachine_incoming_data(uint8_t *data, uint16_t len) {

    if (_interface->on_rx)
        _interface->on_rx(data, len);

    for (uint16_t i = 0; i < len; i++) {

        if (_rx_count >= USER_DEFINED_TCP_RX_ACCUMULATION_BUFFER_LEN) {
            _rx_count = 0;
            break;
        }

        _rx_buffer[_rx_count++] = data[i];
    }

    while (_rx_count >= TCP_PREAMBLE_LEN) {

        uint32_t length = TcpUtilities_decode_length(_rx_buffer);
        uint32_t total_msg_len = 5 + length;

        if (_rx_count < total_msg_len)
            return;

        _process_message(_rx_buffer, total_msg_len);

        uint32_t remaining = _rx_count - total_msg_len;

        if (remaining > 0)
            memmove(_rx_buffer, &_rx_buffer[total_msg_len], remaining);

        _rx_count = (uint16_t) remaining;
    }
}

void TcpRxStatemachine_reset(void) {

    _rx_count = 0;

    for (uint8_t i = 0; i < USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES; i++) {

        if (_multipart_table[i].openlcb_msg) {

            _interface->lock_shared_resources();
            _interface->free_buffer(_multipart_table[i].openlcb_msg);
            _interface->unlock_shared_resources();
        }

        _free_multipart_slot(&_multipart_table[i]);
    }
}
