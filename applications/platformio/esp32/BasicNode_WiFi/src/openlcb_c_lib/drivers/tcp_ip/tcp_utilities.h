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
 * @file tcp_utilities.h
 * @brief Utility functions for TCP/IP OpenLCB preamble encoding/decoding and
 * byte-order helpers.
 *
 * @details All functions are stateless — no internal buffers or side effects
 * beyond the pointers passed in.  Analogous to can_utilities.h for the CAN
 * transport layer.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_UTILITIES__
#define __DRIVERS_TCP_IP_TCP_UTILITIES__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // =========================================================================
    // Preamble Encoding (TX path)
    // =========================================================================

    /**
     * @brief Builds a 17-byte TCP/IP OpenLCB preamble into the output buffer.
     *
     * @details Writes the flags, 24-bit length, originating Node ID, and
     * 48-bit capture time into buf[0..16].  The caller must ensure buf has
     * at least TCP_PREAMBLE_LEN (17) bytes available.
     *
     * @param buf                Output buffer (must be >= 17 bytes).
     * @param flags              16-bit flags field (e.g. TCP_FLAGS_MESSAGE).
     * @param body_len           Number of bytes in the message body that follow
     *                           the preamble.  This is the value encoded in the
     *                           3-byte length field (does NOT include the 5 bytes
     *                           of flags+length themselves, but DOES include the
     *                           12 bytes of originating NodeID + capture time).
     * @param originating_node_id 48-bit Node ID of the originating node/gateway.
     * @param capture_time_ms    48-bit monotonic capture time in milliseconds.
     *
     * @return TCP_PREAMBLE_LEN (17) on success.
     */
    extern uint16_t TcpUtilities_encode_preamble(
            uint8_t *buf,
            uint16_t flags,
            uint32_t body_len,
            node_id_t originating_node_id,
            uint64_t capture_time_ms);

    // =========================================================================
    // Preamble Decoding (RX path)
    // =========================================================================

    /**
     * @brief Extracts the 16-bit flags field from a preamble buffer.
     *
     * @param buf  Pointer to at least 2 bytes (start of preamble).
     *
     * @return 16-bit flags value.
     */
    extern uint16_t TcpUtilities_decode_flags(const uint8_t *buf);

    /**
     * @brief Extracts the 24-bit length field from a preamble buffer.
     *
     * @param buf  Pointer to at least 5 bytes (start of preamble).
     *
     * @return 24-bit length value (bytes remaining after flags+length).
     */
    extern uint32_t TcpUtilities_decode_length(const uint8_t *buf);

    /**
     * @brief Extracts the 48-bit originating Node ID from a preamble buffer.
     *
     * @param buf  Pointer to at least 11 bytes (start of preamble).
     *
     * @return 48-bit node_id_t.
     */
    extern node_id_t TcpUtilities_decode_originating_node_id(const uint8_t *buf);

    /**
     * @brief Extracts the 48-bit capture time from a preamble buffer.
     *
     * @param buf  Pointer to at least 17 bytes (start of preamble).
     *
     * @return 48-bit capture time in milliseconds.
     */
    extern uint64_t TcpUtilities_decode_capture_time(const uint8_t *buf);

    // =========================================================================
    // Message Body Helpers
    // =========================================================================

    /**
     * @brief Extracts the 16-bit MTI from a message body buffer.
     *
     * @param body  Pointer to the message body (after preamble).
     *
     * @return 16-bit MTI value.
     */
    extern uint16_t TcpUtilities_decode_mti(const uint8_t *body);

    /**
     * @brief Extracts a 48-bit Node ID from 6 consecutive bytes (big-endian).
     *
     * @param buf  Pointer to 6 bytes.
     *
     * @return 48-bit node_id_t.
     */
    extern node_id_t TcpUtilities_decode_node_id(const uint8_t *buf);

    /**
     * @brief Writes a 48-bit Node ID into 6 consecutive bytes (big-endian).
     *
     * @param buf      Output buffer (must be >= 6 bytes).
     * @param node_id  48-bit node_id_t to encode.
     *
     * @return 6 (bytes written).
     */
    extern uint8_t TcpUtilities_encode_node_id(uint8_t *buf, node_id_t node_id);

    /**
     * @brief Writes a 16-bit value into 2 consecutive bytes (big-endian).
     *
     * @param buf    Output buffer (must be >= 2 bytes).
     * @param value  16-bit value to encode.
     *
     * @return 2 (bytes written).
     */
    extern uint8_t TcpUtilities_encode_uint16(uint8_t *buf, uint16_t value);

    /**
     * @brief Writes a 48-bit value into 6 consecutive bytes (big-endian).
     *
     * @param buf    Output buffer (must be >= 6 bytes).
     * @param value  48-bit value to encode (upper 16 bits ignored).
     *
     * @return 6 (bytes written).
     */
    extern uint8_t TcpUtilities_encode_uint48(uint8_t *buf, uint64_t value);

    // =========================================================================
    // Flag Helpers
    // =========================================================================

    /**
     * @brief Returns true if the flags indicate an OpenLCB message (bit 15 set).
     *
     * @param flags  16-bit flags from preamble.
     *
     * @return true for OpenLCB messages, false for link control messages.
     */
    extern bool TcpUtilities_is_openlcb_message(uint16_t flags);

    /**
     * @brief Returns true if the chaining bit is set (another preamble follows).
     *
     * @param flags  16-bit flags from preamble.
     *
     * @return true if chaining bit is set.
     */
    extern bool TcpUtilities_is_chained(uint16_t flags);

    /**
     * @brief Returns the multi-part type from the flags field.
     *
     * @param flags  16-bit flags from preamble.
     *
     * @return One of TCP_FLAGS_MULTIPART_SINGLE, _FIRST, _MIDDLE, or _LAST.
     */
    extern uint16_t TcpUtilities_multipart_type(uint16_t flags);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_UTILITIES__ */
