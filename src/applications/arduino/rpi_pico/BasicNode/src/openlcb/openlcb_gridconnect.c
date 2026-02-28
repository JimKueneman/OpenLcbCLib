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
 * @date 28 Feb 2026
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

    /** @brief Current state of the GridConnect parser state machine. */
static uint8_t _current_state = GRIDCONNECT_STATE_SYNC_START;

    /** @brief Current write position in the receive buffer. */
static uint8_t _receive_buffer_index = 0;

    /** @brief Internal buffer for assembling incoming GridConnect messages. */
static gridconnect_buffer_t _receive_buffer;

    /** @brief Return true when the byte is a valid hexadecimal digit. */
static bool _is_valid_hex_char(uint8_t next_byte)
{

    return (((next_byte >= '0') && (next_byte <= '9')) ||
            ((next_byte >= 'A') && (next_byte <= 'F')) ||
            ((next_byte >= 'a') && (next_byte <= 'f')));
}

    /**
    * @brief Processes incoming GridConnect byte stream and extracts complete message.
    *
    * @details Algorithm:
    * Implements a three-state parser that processes GridConnect protocol data one byte
    * at a time:
    * -# SYNC_START: Wait for 'X'/'x', store ':X' prefix, advance to FIND_HEADER
    * -# SYNC_FIND_HEADER: Collect 8 hex chars for CAN ID, expect 'N' at position 10
    * -# SYNC_FIND_DATA: Collect hex data until ';', validate even count, copy to output
    *
    * Use cases:
    * - Serial port receive interrupt handlers calling once per received byte
    * - TCP/IP socket data processing with partial packet reception
    *
    * @verbatim
    * @param next_byte Next byte from the incoming GridConnect stream
    * @param gridconnect_buffer Pointer to buffer where complete message will be stored
    * @endverbatim
    *
    * @return true when a complete valid GridConnect message has been extracted
    * @return false while still collecting data or after recovering from errors
    *
    * @warning NOT thread-safe — uses static variables for parser state
    *
    * @see OpenLcbGridConnect_to_can_msg - Convert extracted GridConnect to CAN message
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
    * @brief Converts a GridConnect message string to a CAN message structure.
    *
    * @details Algorithm:
    * -# Validate message length is at least GRIDCONNECT_HEADER_LEN
    * -# Extract 8-char hex identifier via strtoul()
    * -# Calculate payload byte count from remaining hex characters
    * -# Extract data bytes in pairs via strtoul()
    *
    * Use cases:
    * - Processing received GridConnect messages from serial/TCP
    * - Converting logged GridConnect data to CAN for replay
    *
    * @verbatim
    * @param gridconnect_buffer Pointer to GridConnect message buffer (null-terminated)
    * @param can_msg Pointer to CAN message structure to populate
    * @endverbatim
    *
    * @warning Input must be a valid GridConnect message from the parser
    * @warning Pointers must NOT be NULL
    *
    * @see OpenLcbGridConnect_copy_out_gridconnect_when_done - Extract valid messages
    * @see OpenLcbGridConnect_from_can_msg - Reverse conversion
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
    * @brief Converts a CAN message structure to GridConnect format string.
    *
    * @details Algorithm:
    * -# Write ":X" start sequence
    * -# Format 32-bit CAN identifier as 8-char uppercase hex
    * -# Append "N" normal priority flag
    * -# Format each payload byte as 2-char uppercase hex
    * -# Append ";" terminator
    *
    * Use cases:
    * - Transmitting CAN messages over serial/TCP connections
    * - Logging CAN traffic in human-readable format
    *
    * @verbatim
    * @param gridconnect_buffer Pointer to buffer where GridConnect message will be stored
    * @param can_msg Pointer to source CAN message structure to convert
    * @endverbatim
    *
    * @warning Pointers must NOT be NULL
    * @warning Payload count must not exceed 8
    *
    * @see OpenLcbGridConnect_to_can_msg - Reverse conversion
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
