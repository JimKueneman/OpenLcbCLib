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
 * @file tcp_utilities.c
 * @brief Implementation of TCP/IP OpenLCB preamble encoding/decoding utilities.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_utilities.h"

// =========================================================================
// Preamble Encoding
// =========================================================================

uint16_t TcpUtilities_encode_preamble(
        uint8_t *buf,
        uint16_t flags,
        uint32_t body_len,
        node_id_t originating_node_id,
        uint64_t capture_time_ms) {

    // The length field encodes: originating NodeID (6) + capture time (6) + body
    uint32_t length = 12 + body_len;

    // Flags (2 bytes, big-endian)
    buf[0] = (uint8_t) (flags >> 8);
    buf[1] = (uint8_t) (flags & 0xFF);

    // Length (3 bytes, big-endian)
    buf[2] = (uint8_t) ((length >> 16) & 0xFF);
    buf[3] = (uint8_t) ((length >> 8) & 0xFF);
    buf[4] = (uint8_t) (length & 0xFF);

    // Originating Node ID (6 bytes, big-endian)
    TcpUtilities_encode_node_id(&buf[TCP_PREAMBLE_OFFSET_ORIGINATING_NODE_ID],
                                originating_node_id);

    // Capture Time (6 bytes, big-endian)
    TcpUtilities_encode_uint48(&buf[TCP_PREAMBLE_OFFSET_CAPTURE_TIME],
                               capture_time_ms);

    return TCP_PREAMBLE_LEN;
}

// =========================================================================
// Preamble Decoding
// =========================================================================

uint16_t TcpUtilities_decode_flags(const uint8_t *buf) {

    return ((uint16_t) buf[0] << 8) | (uint16_t) buf[1];
}

uint32_t TcpUtilities_decode_length(const uint8_t *buf) {

    return ((uint32_t) buf[2] << 16) |
           ((uint32_t) buf[3] << 8) |
           (uint32_t) buf[4];
}

node_id_t TcpUtilities_decode_originating_node_id(const uint8_t *buf) {

    return TcpUtilities_decode_node_id(&buf[TCP_PREAMBLE_OFFSET_ORIGINATING_NODE_ID]);
}

uint64_t TcpUtilities_decode_capture_time(const uint8_t *buf) {

    const uint8_t *p = &buf[TCP_PREAMBLE_OFFSET_CAPTURE_TIME];

    return ((uint64_t) p[0] << 40) |
           ((uint64_t) p[1] << 32) |
           ((uint64_t) p[2] << 24) |
           ((uint64_t) p[3] << 16) |
           ((uint64_t) p[4] << 8) |
           (uint64_t) p[5];
}

// =========================================================================
// Message Body Helpers
// =========================================================================

uint16_t TcpUtilities_decode_mti(const uint8_t *body) {

    return ((uint16_t) body[0] << 8) | (uint16_t) body[1];
}

node_id_t TcpUtilities_decode_node_id(const uint8_t *buf) {

    return ((uint64_t) buf[0] << 40) |
           ((uint64_t) buf[1] << 32) |
           ((uint64_t) buf[2] << 24) |
           ((uint64_t) buf[3] << 16) |
           ((uint64_t) buf[4] << 8) |
           (uint64_t) buf[5];
}

uint8_t TcpUtilities_encode_node_id(uint8_t *buf, node_id_t node_id) {

    buf[0] = (uint8_t) ((node_id >> 40) & 0xFF);
    buf[1] = (uint8_t) ((node_id >> 32) & 0xFF);
    buf[2] = (uint8_t) ((node_id >> 24) & 0xFF);
    buf[3] = (uint8_t) ((node_id >> 16) & 0xFF);
    buf[4] = (uint8_t) ((node_id >> 8) & 0xFF);
    buf[5] = (uint8_t) (node_id & 0xFF);

    return 6;
}

uint8_t TcpUtilities_encode_uint16(uint8_t *buf, uint16_t value) {

    buf[0] = (uint8_t) (value >> 8);
    buf[1] = (uint8_t) (value & 0xFF);

    return 2;
}

uint8_t TcpUtilities_encode_uint48(uint8_t *buf, uint64_t value) {

    buf[0] = (uint8_t) ((value >> 40) & 0xFF);
    buf[1] = (uint8_t) ((value >> 32) & 0xFF);
    buf[2] = (uint8_t) ((value >> 24) & 0xFF);
    buf[3] = (uint8_t) ((value >> 16) & 0xFF);
    buf[4] = (uint8_t) ((value >> 8) & 0xFF);
    buf[5] = (uint8_t) (value & 0xFF);

    return 6;
}

// =========================================================================
// Flag Helpers
// =========================================================================

bool TcpUtilities_is_openlcb_message(uint16_t flags) {

    return (flags & TCP_FLAGS_MESSAGE) != 0;
}

bool TcpUtilities_is_chained(uint16_t flags) {

    return (flags & TCP_FLAGS_CHAINING) != 0;
}

uint16_t TcpUtilities_multipart_type(uint16_t flags) {

    return flags & TCP_FLAGS_MULTIPART_MASK;
}
