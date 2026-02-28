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
 * @brief Bidirectional conversion between CAN messages and GridConnect ASCII
 *        format (:X<8-hex-ID>N<hex-data>;).  Includes a streaming byte-at-a-time
 *        parser with automatic error recovery.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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

/** @brief Max GridConnect string length: :X(8)N(16); + NUL = 29 bytes. */
#define MAX_GRID_CONNECT_LEN 29

/** @brief Type definition for GridConnect message buffer */
typedef uint8_t gridconnect_buffer_t[MAX_GRID_CONNECT_LEN];

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
         * @brief Feeds one byte into the streaming GridConnect parser.
         *
         * @details Uses static state — NOT thread-safe, single context only.
         * Malformed input resets the parser automatically.
         *
         * @param next_byte              Next byte from the incoming stream.
         * @param gridconnect_buffer     Destination for the completed message.
         *
         * @return true when a complete, valid GridConnect message is ready.
         */
    extern bool OpenLcbGridConnect_copy_out_gridconnect_when_done(uint8_t next_byte, gridconnect_buffer_t *gridconnect_buffer);

        /**
         * @brief Converts a validated GridConnect string to a @ref can_msg_t.
         *
         * @details Input must come from the parser; no format validation is done here.
         *
         * @param gridconnect_buffer     Completed GridConnect string.
         * @param can_msg                Destination CAN message.
         */
    extern void OpenLcbGridConnect_to_can_msg(gridconnect_buffer_t *gridconnect_buffer, can_msg_t *can_msg);

        /**
         * @brief Converts a @ref can_msg_t to a null-terminated GridConnect string.
         *
         * @details Output is uppercase hex with leading zeros on the 8-char ID.
         *
         * @param gridconnect_buffer     Destination buffer (>= MAX_GRID_CONNECT_LEN).
         * @param can_msg                Source CAN message.
         *
         * @warning Payload count must not exceed 8 or buffer overflow will occur.
         */
    extern void OpenLcbGridConnect_from_can_msg(gridconnect_buffer_t *gridconnect_buffer, can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_GRIDCONNECT__ */
