/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * @file openlcb_gridconnect.h
 * @brief GridConnect protocol implementation for CAN message conversion
 *
 * @details This module provides bidirectional conversion between CAN messages and
 * GridConnect protocol format, commonly used for serial and TCP/IP communication
 * in OpenLCB systems. GridConnect is a human-readable ASCII protocol that encodes
 * CAN frames as colon-delimited strings with hexadecimal identifiers and data.
 *
 * GridConnect Format:
 * ```
 * :X12345678N0102030405;
 *  │└──────┘││└───────┘│
 *  │   ID   ││  Data   │
 *  Start  Normal  Terminator
 * ```
 *
 * The protocol provides:
 * - Streaming parser for incoming byte-by-byte reception
 * - Automatic error recovery and synchronization
 * - Bidirectional CAN ↔ GridConnect conversion
 * - Support for variable-length data payloads (0-8 bytes)
 *
 * Use cases:
 * - Serial port communication with OpenLCB tools
 * - TCP/IP bridges for network-based OpenLCB systems
 * - Debugging and monitoring OpenLCB traffic
 * - Gateway applications between CAN and Ethernet
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 *
 * @see can_types.h - CAN message structures
 */

#ifndef __OPENLCB_OPENLCB_GRIDCONNECT__
#define __OPENLCB_OPENLCB_GRIDCONNECT__

#include <stdbool.h>
#include <stdint.h>

#include "../drivers/canbus/can_types.h"

/** @brief Parser state: Looking for start of GridConnect message (':X' or ':x') */
#define GRIDCONNECT_STATE_SYNC_START 0

/** @brief Parser state: Collecting 8-character hexadecimal CAN identifier */
#define GRIDCONNECT_STATE_SYNC_FIND_HEADER 2

/** @brief Parser state: Collecting data bytes until terminator (';') */
#define GRIDCONNECT_STATE_SYNC_FIND_DATA 4

/** @brief Position of first character after ':X' prefix (start of identifier) */
#define GRIDCONNECT_IDENTIFIER_START_POS 2

/** @brief Length of CAN identifier in GridConnect format (8 hex characters) */
#define GRIDCONNECT_IDENTIFIER_LEN 8

/** @brief Position where 'N' appears (after 8-char identifier) */
#define GRIDCONNECT_NORMAL_FLAG_POS 10

/** @brief Position where data bytes start (after ':X', 8-char ID, and 'N') */
#define GRIDCONNECT_DATA_START_POS 11

/** @brief Number of characters before data section (used for length calculation) */
#define GRIDCONNECT_HEADER_LEN 12

/**
 * @brief Maximum length of a GridConnect message
 *
 * @details Calculation:
 * - ':' (1) + 'X' (1) = 2 chars
 * - Identifier (8 hex chars) = 8 chars
 * - 'N' flag (1) = 1 char
 * - Data (max 8 bytes × 2 hex chars) = 16 chars
 * - ';' terminator (1) = 1 char
 * - Null terminator (1) = 1 char
 * Total: 2 + 8 + 1 + 16 + 1 + 1 = 29 bytes
 */
#define MAX_GRID_CONNECT_LEN 29

/** @brief Type definition for GridConnect message buffer */
typedef uint8_t gridconnect_buffer_t[MAX_GRID_CONNECT_LEN];

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Processes incoming GridConnect byte stream and extracts complete message
     *
     * @details This function implements a state machine parser that processes GridConnect
     * protocol data one byte at a time. It automatically handles synchronization and
     * error recovery, validating message framing, hexadecimal characters, identifier
     * length, and data byte counts.
     *
     * The parser maintains internal state between calls, making it suitable for
     * byte-by-byte processing from serial ports or network streams. It will return
     * true when a complete valid message has been extracted, and false while still
     * collecting data or recovering from errors.
     *
     * Use cases:
     * - Serial port receive interrupt handlers
     * - TCP/IP socket data processing
     * - File parsing of GridConnect logs
     * - Real-time stream processing
     *
     * @param next_byte Next byte from the incoming GridConnect stream
     * @param gridconnect_buffer Pointer to buffer where complete message will be stored
     *
     * @return true when a complete and valid GridConnect message has been extracted
     * @return false while still collecting data or after recovering from errors
     *
     * @warning gridconnect_buffer must point to a valid gridconnect_buffer_t array.
     *          NULL pointer will cause undefined behavior.
     *
     * @warning This function uses static variables to maintain parser state. It is
     *          NOT thread-safe and must be called from a single context only.
     *
     * @attention After receiving true, caller must process the message before the next
     *            call, as the internal state will be reset for the next message.
     *
     * @note The parser automatically recovers from malformed messages by resetting
     *       to SYNC_START state. No explicit reset function is needed.
     *
     * @note Invalid hex characters, wrong lengths, or buffer overflows are silently
     *       handled by resetting the parser. The function does not report error types.
     *
     * @see OpenLcbGridConnect_to_can_msg - Convert extracted GridConnect to CAN message
     * @see GRIDCONNECT_STATE_SYNC_START - Initial parser state
     */
    extern bool OpenLcbGridConnect_copy_out_gridconnect_when_done(uint8_t next_byte, gridconnect_buffer_t *gridconnect_buffer);

    /**
     * @brief Converts a GridConnect message to a CAN message structure
     *
     * @details Parses a complete GridConnect format message and populates a CAN message
     * structure with the extracted identifier and payload data. The function extracts
     * the hexadecimal identifier string, converts it to a 32-bit integer, calculates
     * the payload length from the data section, and converts each pair of hex characters
     * to data bytes.
     *
     * GridConnect format example:
     * ```
     * :X19170640N0501010107015555;
     *   ^^^^^^^^ ^^^^^^^^^^^^^^^
     *   ID (8)   Data (variable, even count)
     * ```
     *
     * Use cases:
     * - Processing received GridConnect messages from serial/TCP
     * - Converting logged GridConnect data to CAN for replay
     * - Bridging GridConnect protocol to native CAN bus
     *
     * @param gridconnect_buffer Pointer to GridConnect message buffer (null-terminated string)
     * @param can_msg Pointer to CAN message structure to populate with converted data
     *
     * @warning gridconnect_buffer must contain a valid, complete GridConnect message as
     *          produced by OpenLcbGridConnect_copy_out_gridconnect_when_done(). Passing
     *          malformed data may produce incorrect CAN messages or undefined behavior.
     *
     * @warning gridconnect_buffer must be at least 12 characters long (header minimum).
     *          Shorter buffers will cause buffer underflow when calculating data length.
     *
     * @warning can_msg must not be NULL. No NULL check is performed, dereferencing
     *          NULL will cause immediate crash.
     *
     * @attention This function does not validate the GridConnect format. It assumes
     *            the input has been validated by the parser. Always use messages from
     *            OpenLcbGridConnect_copy_out_gridconnect_when_done() or equivalent.
     *
     * @note The function uses strtoul() for hex string conversion, which is not the
     *       most efficient method but provides robust parsing with automatic base detection.
     *
     * @see OpenLcbGridConnect_copy_out_gridconnect_when_done - Extract valid GridConnect messages
     * @see OpenLcbGridConnect_from_can_msg - Reverse conversion (CAN to GridConnect)
     * @see can_msg_t - CAN message structure definition
     */
    extern void OpenLcbGridConnect_to_can_msg(gridconnect_buffer_t *gridconnect_buffer, can_msg_t *can_msg);

    /**
     * @brief Converts a CAN message structure to GridConnect format
     *
     * @details Generates a complete GridConnect protocol message from a CAN message
     * structure. The function formats the message start sequence, converts the 32-bit
     * identifier to uppercase hexadecimal with leading zeros, adds the normal flag,
     * converts each payload byte to uppercase hexadecimal, and adds the terminator.
     *
     * Output format: `:X<8-hex-ID>N<2-hex-byte>...<2-hex-byte>;`
     *
     * Example conversions:
     * - ID=0x19170640, Data={0x05,0x01} → ":X19170640N0501;"
     * - ID=0x00000001, Data={} → ":X00000001N;"
     *
     * Use cases:
     * - Transmitting CAN messages over serial/TCP connections
     * - Logging CAN traffic in human-readable format
     * - Debugging and monitoring OpenLCB bus activity
     * - Gateway applications forwarding CAN to GridConnect clients
     *
     * @param gridconnect_buffer Pointer to buffer where GridConnect message will be stored
     * @param can_msg Pointer to source CAN message structure to convert
     *
     * @warning gridconnect_buffer must point to a valid gridconnect_buffer_t array
     *          (minimum 29 bytes). Buffer overflow will occur if smaller.
     *
     * @warning can_msg must not be NULL. No NULL check is performed, dereferencing
     *          NULL will cause immediate crash.
     *
     * @warning can_msg->payload_count must be accurate and not exceed 8. Values > 8
     *          will cause buffer overflow when writing data bytes.
     *
     * @attention The output buffer is null-terminated and safe to use as a C string
     *            for transmission or logging.
     *
     * @note All hexadecimal output is uppercase (A-F, not a-f) for consistency.
     *
     * @note The identifier is always formatted with leading zeros to maintain the
     *       8-character width required by GridConnect protocol.
     *
     * @see OpenLcbGridConnect_to_can_msg - Reverse conversion (GridConnect to CAN)
     * @see OpenLcbGridConnect_copy_out_gridconnect_when_done - Parse received GridConnect
     * @see can_msg_t - CAN message structure definition
     */
    extern void OpenLcbGridConnect_from_can_msg(gridconnect_buffer_t *gridconnect_buffer, can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_GRIDCONNECT__ */
