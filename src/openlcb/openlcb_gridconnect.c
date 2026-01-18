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
 * @file openlcb_gridconnect.c
 * @brief Implementation of GridConnect protocol conversion
 *
 * @details This module implements the GridConnect protocol, a human-readable ASCII
 * encoding of CAN bus messages. The protocol is widely used in OpenLCB systems for
 * serial and TCP/IP communication, debugging, and bridging applications.
 *
 * Implementation features:
 * - Stateful streaming parser for byte-by-byte reception
 * - Automatic error detection and recovery
 * - Bidirectional conversion between CAN and GridConnect formats
 * - No dynamic memory allocation
 * - Single-threaded design (not thread-safe)
 *
 * The parser state machine handles:
 * - Message synchronization (finding ':X' start sequence)
 * - Hexadecimal validation for identifier and data
 * - Length validation (8-char identifier, even data byte count)
 * - Automatic error recovery by resetting to start state
 *
 * GridConnect Protocol Format:
 * ```
 * :X<8-hex-ID>N<hex-data>;
 *
 * Example: :X19170640N0501010107015555;
 *   :       - Start delimiter
 *   X       - Extended frame indicator
 *   19170640 - 29-bit CAN identifier (8 hex chars)
 *   N       - Normal priority flag
 *   050101... - Data bytes (2 hex chars per byte)
 *   ;       - End delimiter
 * ```
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 *
 * @see openlcb_gridconnect.h - Public interface
 * @see can_types.h - CAN message structures
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openlcb_gridconnect.h"
#include "openlcb_types.h"
#include "../drivers/canbus/can_types.h"

/**
 * @brief Current state of the GridConnect parser state machine
 *
 * @details Tracks which phase of message parsing is active. Valid states are:
 * - GRIDCONNECT_STATE_SYNC_START: Waiting for message start (':X')
 * - GRIDCONNECT_STATE_SYNC_FIND_HEADER: Collecting 8-char identifier
 * - GRIDCONNECT_STATE_SYNC_FIND_DATA: Collecting data bytes until ';'
 *
 * The state is preserved between function calls to maintain parsing context
 * across byte-by-byte reception. Static storage duration ensures persistence.
 */
static uint8_t _current_state = GRIDCONNECT_STATE_SYNC_START;

/**
 * @brief Current write position in the receive buffer
 *
 * @details Tracks where the next received byte should be stored in _receive_buffer.
 * Incremented as characters are accumulated. Reset to 0 when starting a new message
 * or recovering from errors. Valid range is 0 to MAX_GRID_CONNECT_LEN - 1.
 */
static uint8_t _receive_buffer_index = 0;

/**
 * @brief Internal buffer for assembling incoming GridConnect messages
 *
 * @details Accumulates characters as they arrive until a complete message is received.
 * The buffer stores the entire GridConnect message including delimiters and null
 * terminator. Contents are only valid after a complete message is parsed.
 */
static gridconnect_buffer_t _receive_buffer;

/**
 * @brief Validates if a character is a valid hexadecimal digit
 *
 * @details Algorithm:
 * Checks if the character falls within the valid hexadecimal ranges:
 * -# '0' through '9' (ASCII 48-57)
 * -# 'A' through 'F' (ASCII 65-70)
 * -# 'a' through 'f' (ASCII 97-102)
 *
 * This validation is critical for the GridConnect parser to ensure data integrity
 * and detect malformed messages early.
 *
 * Use cases:
 * - Validating CAN identifier characters during header parsing
 * - Validating data byte characters during payload parsing
 * - Detecting transmission errors or protocol violations
 *
 * @param next_byte Character to validate
 *
 * @return true if next_byte is a valid hexadecimal character (0-9, A-F, a-f)
 * @return false if next_byte is not a hexadecimal character
 *
 * @note This function is case-insensitive and accepts both uppercase and lowercase
 *
 * @see OpenLcbGridConnect_copy_out_gridconnect_when_done - Uses this for validation
 */
static bool _is_valid_hex_char(uint8_t next_byte)
{

    return (((next_byte >= '0') && (next_byte <= '9')) ||
            ((next_byte >= 'A') && (next_byte <= 'F')) ||
            ((next_byte >= 'a') && (next_byte <= 'f')));
}

/**
 * @brief Processes incoming GridConnect byte stream and extracts complete message
 *
 * @details Algorithm:
 * Implements a three-state parser that processes GridConnect protocol data one byte
 * at a time. The state machine ensures proper message framing and data validation:
 *
 * -# SYNC_START state:
 *    - Wait for 'X' or 'x' character
 *    - When found, store ':X' prefix and advance to SYNC_FIND_HEADER
 *    - All other characters are ignored (auto-sync)
 *
 * -# SYNC_FIND_HEADER state:
 *    - Collect exactly 8 hexadecimal characters for CAN identifier
 *    - Validate each character is valid hex (0-9, A-F, a-f)
 *    - Expect 'N' or 'n' at position 10
 *    - On 'N' at correct position, advance to SYNC_FIND_DATA
 *    - On any error, reset to SYNC_START
 *
 * -# SYNC_FIND_DATA state:
 *    - Collect hexadecimal data characters until ';' terminator
 *    - Validate all characters are valid hex
 *    - Ensure even character count (2 hex chars = 1 byte)
 *    - On ';', validate and copy to output buffer, return true
 *    - On buffer overflow or invalid char, reset to SYNC_START
 *
 * Error handling:
 * - Invalid hex characters → Reset to SYNC_START
 * - Wrong identifier length → Reset to SYNC_START
 * - Odd data character count → Reset to SYNC_START
 * - Buffer overflow → Reset to SYNC_START
 * - All errors are silent; parser automatically recovers
 *
 * The parser maintains internal state between calls using static variables,
 * enabling byte-by-byte processing suitable for interrupt-driven serial reception
 * or network socket data streams.
 *
 * Use cases:
 * - Serial port receive interrupt handlers calling once per received byte
 * - TCP/IP socket data processing with partial packet reception
 * - File parsing of GridConnect log files
 * - Real-time monitoring applications
 *
 * @param next_byte Next byte from the incoming GridConnect stream
 * @param gridconnect_buffer Pointer to buffer where complete message will be stored
 *
 * @return true when a complete and valid GridConnect message has been extracted and
 *         copied to gridconnect_buffer
 * @return false while still collecting data or after recovering from errors
 *
 * @warning gridconnect_buffer must point to a valid gridconnect_buffer_t array.
 *          NULL pointer will cause undefined behavior on message completion.
 *
 * @warning This function uses static variables (_current_state, _receive_buffer_index,
 *          _receive_buffer) to maintain parser state. It is NOT thread-safe and must
 *          be called from a single context only.
 *
 * @attention After receiving true, caller must process the message before the next
 *            call, as the internal state will be reset for the next message and
 *            _receive_buffer may be overwritten.
 *
 * @attention The function modifies gridconnect_buffer only when returning true.
 *            The buffer contents are undefined when the function returns false.
 *
 * @note The parser is designed for continuous streaming operation. Feed bytes
 *       sequentially and check for true return on each call.
 *
 * @note Invalid messages are silently discarded. The function does not report
 *       error types or counts; it simply resynchronizes to the next valid message.
 *
 * @see OpenLcbGridConnect_to_can_msg - Convert extracted GridConnect to CAN message
 * @see _is_valid_hex_char - Hexadecimal character validation helper
 */
bool OpenLcbGridConnect_copy_out_gridconnect_when_done(uint8_t next_byte, gridconnect_buffer_t *gridconnect_buffer)
{

    switch (_current_state)
    {

    case GRIDCONNECT_STATE_SYNC_START:

        if ((next_byte == 'X') || (next_byte == 'x'))
        {

            _receive_buffer_index = 0;
            _receive_buffer[_receive_buffer_index] = ':';
            _receive_buffer_index++;
            _receive_buffer[_receive_buffer_index] = next_byte;
            _receive_buffer_index++;
            _current_state = GRIDCONNECT_STATE_SYNC_FIND_HEADER;
        }

        break;

    case GRIDCONNECT_STATE_SYNC_FIND_HEADER:

        if (_receive_buffer_index > GRIDCONNECT_NORMAL_FLAG_POS)
        {
            
            _current_state = GRIDCONNECT_STATE_SYNC_START;

            break;
        }

        if ((next_byte == 'N') || (next_byte == 'n'))
        {

            if (_receive_buffer_index == GRIDCONNECT_NORMAL_FLAG_POS)
            {
                
                _receive_buffer[_receive_buffer_index] = next_byte;
                
                _receive_buffer_index++;
                _current_state = GRIDCONNECT_STATE_SYNC_FIND_DATA;
            }
            else
            {

                _current_state = GRIDCONNECT_STATE_SYNC_START;

                break;
            }
        }
        else
        {

            if (!_is_valid_hex_char(next_byte))
            {

                _current_state = GRIDCONNECT_STATE_SYNC_START;

                break;
            }

            _receive_buffer[_receive_buffer_index] = next_byte;
            _receive_buffer_index++;
        }

        break;

    case GRIDCONNECT_STATE_SYNC_FIND_DATA:

        if (next_byte == ';')
        {

            if ((_receive_buffer_index + 1) % 2 != 0)
            {
                
                _current_state = GRIDCONNECT_STATE_SYNC_START;

                break;
            }

            _receive_buffer[_receive_buffer_index] = ';';
            
            _receive_buffer[_receive_buffer_index + 1] = 0;
            _current_state = GRIDCONNECT_STATE_SYNC_START;

            for (int i = 0; i < MAX_GRID_CONNECT_LEN; i++)
            {

                (*gridconnect_buffer)[i] = _receive_buffer[i];
            }

            return true;
        }
        else
        {

            if (!_is_valid_hex_char(next_byte))
            {

                _current_state = GRIDCONNECT_STATE_SYNC_START;

                break;
            }

            _receive_buffer[_receive_buffer_index] = next_byte;
            _receive_buffer_index++;
        }

        if (_receive_buffer_index > MAX_GRID_CONNECT_LEN - 1)
        {

            _current_state = GRIDCONNECT_STATE_SYNC_START;
        }

        break;

    default:

        _current_state = GRIDCONNECT_STATE_SYNC_START;

        break;
    }

    return false;
}

/**
 * @brief Converts a GridConnect message to a CAN message structure
 *
 * @details Algorithm:
 * Parses a complete GridConnect format message and populates a CAN message structure
 * with the extracted identifier and payload data:
 *
 * -# Validate message length is at least GRIDCONNECT_HEADER_LEN characters
 * -# Extract identifier (8 hex characters starting at position GRIDCONNECT_IDENTIFIER_START_POS):
 *    - Copy characters to temporary string buffer
 *    - Null-terminate the string
 *    - Convert hex string to uint32_t using strtoul()
 *    - Store in can_msg->identifier
 *
 * -# Calculate payload length:
 *    - Get total message length with strlen()
 *    - Subtract GRIDCONNECT_HEADER_LEN (accounts for ":X", 8-char ID, "N", ";")
 *    - Divide by 2 (each byte is 2 hex characters)
 *    - Store in can_msg->payload_count
 *
 * -# Extract data bytes:
 *    - Start at position GRIDCONNECT_DATA_START_POS
 *    - For each pair of hex characters:
 *      * Copy pair to temporary buffer with "0x" prefix
 *      * Convert to byte using strtoul()
 *      * Store in can_msg->payload[]
 *      * Advance by 2 characters
 *
 * Example parsing:
 * ```
 * Input:  ":X19170640N0501;"
 *          ^^19170640^0501^
 *          ||   ID   | Data|
 * Output: can_msg->identifier = 0x19170640
 *         can_msg->payload_count = 2
 *         can_msg->payload[0] = 0x05
 *         can_msg->payload[1] = 0x01
 * ```
 *
 * Use cases:
 * - Processing received GridConnect messages from serial/TCP
 * - Converting logged GridConnect data to CAN for replay
 * - Bridging GridConnect protocol to native CAN bus
 * - Testing and simulation environments
 *
 * @param gridconnect_buffer Pointer to GridConnect message buffer (null-terminated string)
 * @param can_msg Pointer to CAN message structure to populate with converted data
 *
 * @warning gridconnect_buffer must contain a valid, complete GridConnect message as
 *          produced by OpenLcbGridConnect_copy_out_gridconnect_when_done(). Passing
 *          malformed data may produce incorrect CAN messages or undefined behavior.
 *
 * @warning gridconnect_buffer must be at least GRIDCONNECT_HEADER_LEN characters long.
 *          Shorter buffers will cause strlen() underflow when calculating data length.
 *
 * @warning can_msg must not be NULL. No NULL check is performed, dereferencing
 *          NULL will cause immediate crash.
 *
 * @attention This function does not validate the GridConnect format. It assumes
 *            the input has been validated by the parser. Always use messages from
 *            OpenLcbGridConnect_copy_out_gridconnect_when_done() or equivalent.
 *
 * @attention The function will attempt to process any string passed to it. For
 *            safety, a length check is performed before parsing begins.
 *
 * @note The function uses strtoul() for hex string conversion with automatic base
 *       detection. While not the most efficient method, it provides robust parsing.
 *
 * @note Temporary stack buffers are used for string conversion operations to avoid
 *       dynamic memory allocation.
 *
 * @see OpenLcbGridConnect_copy_out_gridconnect_when_done - Extract valid GridConnect messages
 * @see OpenLcbGridConnect_from_can_msg - Reverse conversion (CAN to GridConnect)
 * @see can_msg_t - CAN message structure definition
 */
void OpenLcbGridConnect_to_can_msg(gridconnect_buffer_t *gridconnect_buffer, can_msg_t *can_msg)
{

    size_t message_length;
    unsigned long data_char_count;
    
    message_length = strlen((char *)gridconnect_buffer);
    
    if (message_length < GRIDCONNECT_HEADER_LEN)
    {

        can_msg->identifier = 0;
        can_msg->payload_count = 0;
        return;
    }

    
    char identifier_str[GRIDCONNECT_IDENTIFIER_LEN + 1];

    for (int i = GRIDCONNECT_IDENTIFIER_START_POS; i < GRIDCONNECT_IDENTIFIER_START_POS + GRIDCONNECT_IDENTIFIER_LEN; i++)
    {

        identifier_str[i - GRIDCONNECT_IDENTIFIER_START_POS] = (*gridconnect_buffer)[i];
    }
    identifier_str[GRIDCONNECT_IDENTIFIER_LEN] = 0;

    char hex_it[64] = "0x";
    strcat(hex_it, identifier_str);
    can_msg->identifier = (uint32_t)strtoul(hex_it, NULL, 0);

    
    data_char_count = message_length - GRIDCONNECT_HEADER_LEN;
    can_msg->payload_count = (uint8_t)(data_char_count / 2);

    
    char byte_str[3];
    uint8_t byte;
    int payload_index = 0;
    int char_index = GRIDCONNECT_DATA_START_POS;
    
    while (char_index < (int)(data_char_count + GRIDCONNECT_DATA_START_POS))
    {

        byte_str[0] = (*gridconnect_buffer)[char_index];
        byte_str[1] = (*gridconnect_buffer)[char_index + 1];
        byte_str[2] = 0;

        byte = (uint8_t)strtoul((char *)byte_str, NULL, 16);
        can_msg->payload[payload_index] = byte;
        payload_index++;
        char_index += 2;
    }
}

/**
 * @brief Converts a CAN message structure to GridConnect format
 *
 * @details Algorithm:
 * Generates a complete GridConnect protocol message from a CAN message structure:
 *
 * -# Initialize output buffer:
 *    - Clear first byte to ensure clean start
 *    - Append ":X" start sequence
 *
 * -# Format CAN identifier:
 *    - Convert 32-bit identifier to 8-character uppercase hexadecimal
 *    - Use sprintf with "%08lX" format (8 digits, leading zeros, uppercase)
 *    - Append to output buffer
 *
 * -# Append normal priority flag:
 *    - Add "N" character
 *
 * -# Format payload data:
 *    - For each byte in payload (0 to payload_count - 1):
 *      * Convert byte to 2-character uppercase hexadecimal
 *      * Use sprintf with "%02X" format (2 digits, leading zero if needed)
 *      * Append to output buffer
 *
 * -# Append terminator:
 *    - Add ";" character
 *    - Result is null-terminated string ready for transmission
 *
 * Example conversions:
 * ```
 * CAN message 1:
 *   ID = 0x19170640
 *   Payload = {0x05, 0x01, 0x01, 0x01, 0x07, 0x01, 0x55, 0x55}
 *   Count = 8
 * GridConnect: ":X19170640N0501010107015555;"
 *
 * CAN message 2:
 *   ID = 0x00000001
 *   Payload = {} (empty)
 *   Count = 0
 * GridConnect: ":X00000001N;"
 * ```
 *
 * Use cases:
 * - Transmitting CAN messages over serial/TCP connections
 * - Logging CAN traffic in human-readable format
 * - Debugging and monitoring OpenLCB bus activity
 * - Gateway applications forwarding CAN to GridConnect clients
 * - Recording CAN data for later analysis
 *
 * @param gridconnect_buffer Pointer to buffer where GridConnect message will be stored
 * @param can_msg Pointer to source CAN message structure to convert
 *
 * @warning gridconnect_buffer must point to a valid gridconnect_buffer_t array
 *          (minimum MAX_GRID_CONNECT_LEN = 29 bytes). Buffer overflow will occur
 *          if a smaller buffer is provided.
 *
 * @warning can_msg must not be NULL. No NULL check is performed, dereferencing
 *          NULL will cause immediate crash.
 *
 * @warning can_msg->payload_count must be accurate and not exceed 8. Values > 8
 *          will cause buffer overflow when writing data bytes. The function does
 *          not validate this constraint.
 *
 * @attention The output buffer is null-terminated and safe to use as a C string
 *            for transmission, logging, or display purposes.
 *
 * @attention This function uses sprintf() internally, which requires a properly
 *            sized temporary buffer. Stack overflow is possible on very small
 *            embedded systems with limited stack space.
 *
 * @note All hexadecimal output is uppercase (A-F, not a-f) for consistency with
 *       standard GridConnect implementations.
 *
 * @note The identifier is always formatted with leading zeros to maintain the
 *       8-character width required by GridConnect protocol, even for small values.
 *
 * @note The function uses strcat() for string building, which requires string
 *       operations. For performance-critical applications, consider using direct
 *       buffer manipulation instead.
 *
 * @see OpenLcbGridConnect_to_can_msg - Reverse conversion (GridConnect to CAN)
 * @see OpenLcbGridConnect_copy_out_gridconnect_when_done - Parse received GridConnect
 * @see can_msg_t - CAN message structure definition
 */
void OpenLcbGridConnect_from_can_msg(gridconnect_buffer_t *gridconnect_buffer, can_msg_t *can_msg)
{

    char temp_str[9];

    (*gridconnect_buffer)[0] = 0;
    strcat((char *)gridconnect_buffer, ":");
    strcat((char *)gridconnect_buffer, "X");

    sprintf(temp_str, "%08lX", (unsigned long)can_msg->identifier);
    strcat((char *)gridconnect_buffer, temp_str);
    strcat((char *)gridconnect_buffer, "N");
    
    for (int i = 0; i < can_msg->payload_count; i++)
    {

        sprintf(temp_str, "%02X", can_msg->payload[i]);
        strcat((char *)gridconnect_buffer, temp_str);
    }

    strcat((char *)gridconnect_buffer, ";");
}
