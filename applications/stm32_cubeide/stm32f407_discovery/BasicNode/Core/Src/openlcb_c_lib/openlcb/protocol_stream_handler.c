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
 * @file protocol_stream_handler.c
 * @brief Stream Transport protocol implementation.
 *
 * @details Implements the OpenLCB Stream Transport Standard (Feb 2026).
 * Manages a fixed-size table of stream state entries, handles initiation
 * negotiation, flow-controlled data transfer, and stream completion.
 * All application interaction is through optional DI callbacks.
 *
 * @author Jim Kueneman
 * @date 03 Apr 2026
 */

#include "protocol_stream_handler.h"

#include "openlcb_config.h"

#ifdef OPENLCB_COMPILE_STREAM

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_utilities.h"

#ifndef USER_DEFINED_MAX_STREAM_COUNT
#define USER_DEFINED_MAX_STREAM_COUNT 1
#endif

// =============================================================================
// Static state
// =============================================================================

    /** @brief Stored callback interface pointer. */
static const interface_protocol_stream_handler_t *_interface;

    /** @brief Fixed-size stream state table. */
static stream_state_t _stream_table[USER_DEFINED_MAX_STREAM_COUNT];

    /** @brief Next Destination Stream ID to assign (monotonic, wraps at 0xFE). */
static uint8_t _next_dest_stream_id = 0;

    /** @brief Next Source Stream ID to assign (monotonic, wraps at 0xFE). */
static uint8_t _next_source_stream_id = 0;

// =============================================================================
// Internal helpers
// =============================================================================

    /**
     * @brief Finds a stream state entry by remote node ID and stream ID.
     *
     * @details Algorithm:
     * -# Iterate through the stream table
     * -# Match on remote_node_id and the appropriate stream ID
     * -# Return pointer to matching entry or NULL if not found
     *
     * @verbatim
     * @param remote_node_id  Node ID of the remote end.
     * @param stream_id       Stream ID to match (SID or DID depending on role).
     * @param match_source_id If true, match source_stream_id; if false, match dest_stream_id.
     * @endverbatim
     *
     * @return Pointer to matching entry, or NULL if not found.
     */
static stream_state_t *_find_stream(node_id_t remote_node_id, uint8_t stream_id, bool match_source_id) {

    for (int i = 0; i < USER_DEFINED_MAX_STREAM_COUNT; i++) {

        if (_stream_table[i].state == STREAM_STATE_CLOSED) {

            continue;

        }

        if (_stream_table[i].remote_node_id != remote_node_id) {

            continue;

        }

        if (match_source_id) {

            if (_stream_table[i].source_stream_id == stream_id) { return &_stream_table[i]; }

        } else {

            if (_stream_table[i].dest_stream_id == stream_id) { return &_stream_table[i]; }

        }

    }

    return NULL;

}

    /**
     * @brief Allocates a new stream state entry from the table.
     *
     * @details Algorithm:
     * -# Iterate through the stream table looking for a CLOSED entry
     * -# Zero the entry and return pointer
     * -# Return NULL if table is full
     *
     * @return Pointer to a fresh CLOSED entry, or NULL if table is full.
     */
static stream_state_t *_allocate_stream(void) {

    for (int i = 0; i < USER_DEFINED_MAX_STREAM_COUNT; i++) {

        if (_stream_table[i].state == STREAM_STATE_CLOSED) {

            memset(&_stream_table[i], 0, sizeof(stream_state_t));
            return &_stream_table[i];

        }

    }

    return NULL;

}

    /**
     * @brief Releases a stream state entry back to the pool.
     *
     * @details Algorithm:
     * -# Set state to CLOSED
     *
     * @verbatim
     * @param stream  Pointer to stream_state_t entry to free.
     * @endverbatim
     */
static void _free_stream(stream_state_t *stream) {

    stream->state = STREAM_STATE_CLOSED;

}

    /**
     * @brief Assigns the next available Destination Stream ID.
     *
     * @details Algorithm:
     * -# Return current _next_dest_stream_id
     * -# Increment and wrap at 0xFE (skip 0xFF which is reserved)
     *
     * @return A DID value in the range [0..254].
     */
static uint8_t _assign_dest_stream_id(void) {

    uint8_t id = _next_dest_stream_id;

    _next_dest_stream_id++;

    if (_next_dest_stream_id >= STREAM_ID_RESERVED) {

        _next_dest_stream_id = 0;

    }

    return id;

}

    /**
     * @brief Loads a Stream Initiate Reply into the outgoing message.
     *
     * @details Algorithm:
     * -# Load message header with MTI_STREAM_INIT_REPLY
     * -# Clear payload
     * -# Set bytes 0-1 to max_buffer_size (big-endian)
     * -# Set bytes 2-3 to flags_or_error
     * -# Set byte 4 to source_stream_id
     * -# Set byte 5 to dest_stream_id
     * -# Set payload_count to 6
     * -# Mark outgoing as valid
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param max_buffer_size    Negotiated buffer size (0 if reject).
     * @param flags_or_error     Accept (0x8000) or error code (0x1xxx, 0x2xxx).
     * @param source_stream_id   SID from the request.
     * @param dest_stream_id     DID assigned by this node.
     * @endverbatim
     */
static void _load_initiate_reply(
            openlcb_statemachine_info_t *statemachine_info,
            uint16_t max_buffer_size,
            uint16_t flags_or_error,
            uint8_t source_stream_id,
            uint8_t dest_stream_id) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_STREAM_INIT_REPLY);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, max_buffer_size, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, flags_or_error, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, source_stream_id, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, dest_stream_id, 5);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 6;
    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Loads a Stream Data Proceed (MTI 0x0888) into the outgoing slot.
     *
     * @details Sent by the destination after receiving max_buffer_size bytes,
     * telling the source it may send the next window.  Payload is 2 bytes:
     * SID (byte 0) and DID (byte 1).
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to the stream state entry.
     * @endverbatim
     */
static void _load_data_proceed(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_STREAM_PROCEED);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->source_stream_id, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->dest_stream_id, 1);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 2;
    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Assigns the next available Source Stream ID.
     *
     * @details Algorithm:
     * -# Return current _next_source_stream_id
     * -# Increment and wrap at 0xFE (skip 0xFF which is reserved)
     *
     * @return A SID value in the range [0..254].
     */
static uint8_t _assign_source_stream_id(void) {

    uint8_t id = _next_source_stream_id;

    _next_source_stream_id++;

    if (_next_source_stream_id >= STREAM_ID_RESERVED) {

        _next_source_stream_id = 0;

    }

    return id;

}

    /**
     * @brief Loads a Stream Initiate Request into the outgoing message.
     *
     * @details Algorithm:
     * -# Load message header with MTI_STREAM_INIT_REQUEST
     * -# Clear payload
     * -# Set bytes 0-1 to proposed_buffer_size (big-endian)
     * -# Set bytes 2-3 to flags (0x0000)
     * -# Set byte 4 to source_stream_id
     * -# Set byte 5 to suggested_dest_stream_id
     * -# Set payload_count to 6
     * -# Mark outgoing as valid
     *
     * @verbatim
     * @param statemachine_info         Pointer to openlcb_statemachine_info_t context.
     * @param proposed_buffer_size      Proposed max buffer size.
     * @param source_stream_id          SID assigned by this (source) node.
     * @param suggested_dest_stream_id  Suggested DID (0xFF if no preference).
     * @param dest_alias                CAN alias of the destination node.
     * @param dest_id                   Node ID of the destination node.
     * @param content_uid               Optional 6-byte Content UID (NULL to omit).
     * @endverbatim
     */
static void _load_initiate_request(
            openlcb_statemachine_info_t *statemachine_info,
            uint16_t proposed_buffer_size,
            uint8_t source_stream_id,
            uint8_t suggested_dest_stream_id,
            uint16_t dest_alias,
            node_id_t dest_id,
            const uint8_t *content_uid) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            dest_alias,
            dest_id,
            MTI_STREAM_INIT_REQUEST);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, proposed_buffer_size, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x0000, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, source_stream_id, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, suggested_dest_stream_id, 5);

    if (content_uid) {

        for (int i = 0; i < 6; i++) {

            OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, content_uid[i], 6 + i);

        }

        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 12;

    } else {

        statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 6;

    }

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Loads a Stream Data Send message into the outgoing slot.
     *
     * @details Payload: byte 0 = Destination Stream ID, bytes 1+ = data.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to the stream state entry.
     * @param data               Pointer to data bytes to send.
     * @param data_len           Number of data bytes.
     * @endverbatim
     */
static void _load_data_send(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream,
            const uint8_t *data,
            uint16_t data_len) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            stream->remote_alias,
            stream->remote_node_id,
            MTI_STREAM_SEND);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->dest_stream_id, 0);

    for (uint16_t i = 0; i < data_len; i++) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, data[i], 1 + i);

    }

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 1 + data_len;
    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Loads a Stream Data Complete message into the outgoing slot.
     *
     * @details Payload: byte 0 = SID, byte 1 = DID, bytes 2-3 = flags (0),
     * bytes 4-7 = total bytes transferred mod 2^32 (per spec Section 5.5).
     * Either side can send.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to the stream state entry.
     * @endverbatim
     */
static void _load_data_complete(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            stream->remote_alias,
            stream->remote_node_id,
            MTI_STREAM_COMPLETE);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->source_stream_id, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->dest_stream_id, 1);

    // Optional flags (bytes 2-3)
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x0000, 2);

    // Total bytes transferred mod 2^32 (bytes 4-7)
    uint32_t total = stream->bytes_transferred;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((total >> 24) & 0xFF), 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((total >> 16) & 0xFF), 5);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((total >> 8) & 0xFF), 6);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) (total & 0xFF), 7);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;
    statemachine_info->outgoing_msg_info.valid = true;

}

// =============================================================================
// Public API
// =============================================================================

    /**
     * @brief Initializes the Stream Transport protocol layer.
     *
     * @details Algorithm:
     * -# Store pointer to callback interface
     * -# Clear the stream state table
     * -# Reset the DID counter
     *
     * @verbatim
     * @param interface_protocol_stream_handler  Pointer to callback interface structure.
     * @endverbatim
     *
     * @warning Pointer must remain valid for lifetime of application.
     * @warning NOT thread-safe -- call during single-threaded initialization only.
     */
void ProtocolStreamHandler_initialize(const interface_protocol_stream_handler_t *interface_protocol_stream_handler) {

    _interface = interface_protocol_stream_handler;

    memset(_stream_table, 0, sizeof(_stream_table));

    _next_dest_stream_id = 0;
    _next_source_stream_id = 0;

}

    /**
     * @brief Handles incoming Stream Initiate Request (MTI 0x0CC8).
     *
     * @details Algorithm:
     * -# Extract max_buffer_size (bytes 0-1), flags (bytes 2-3), SID (byte 4)
     * -# If payload >= 6, extract proposed DID (byte 5)
     * -# If payload >= 12, extract 6-byte Stream Content UID (bytes 6-11)
     * -# If no on_initiate_request callback, reject with streams not supported
     * -# Allocate a stream state entry; if table full, reject with buffer unavailable
     * -# Assign a DID, populate stream state
     * -# Call on_initiate_request callback
     * -# Send accept reply with negotiated buffer size
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @endverbatim
     */
void ProtocolStreamHandler_initiate_request(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *incoming = statemachine_info->incoming_msg_info.msg_ptr;

    uint16_t proposed_buffer_size = OpenLcbUtilities_extract_word_from_openlcb_payload(incoming, 0);
    uint8_t source_stream_id = OpenLcbUtilities_extract_byte_from_openlcb_payload(incoming, 4);

    // No application callback registered -- reject with "streams not supported"
    if (!_interface->on_initiate_request) {

        _load_initiate_reply(statemachine_info, 0, ERROR_PERMANENT_STREAMS_NOT_SUPPORTED, source_stream_id, 0);
        return;

    }

    // Allocate stream state entry
    stream_state_t *stream = _allocate_stream();

    if (!stream) {

        _load_initiate_reply(statemachine_info, 0, ERROR_TEMPORARY_BUFFER_UNAVAILABLE, source_stream_id, 0);
        return;

    }

    // Populate stream state
    stream->state = STREAM_STATE_INITIATED;
    stream->source_stream_id = source_stream_id;
    stream->dest_stream_id = _assign_dest_stream_id();
    stream->remote_node_id = incoming->source_id;
    stream->remote_alias = incoming->source_alias;
    stream->is_source = false;
    stream->bytes_transferred = 0;

    // Negotiate buffer size: use proposed or LEN_MESSAGE_BYTES_STREAM, whichever is smaller
    if (proposed_buffer_size <= LEN_MESSAGE_BYTES_STREAM) {

        stream->max_buffer_size = proposed_buffer_size;

    } else {

        stream->max_buffer_size = LEN_MESSAGE_BYTES_STREAM;

    }

    stream->bytes_remaining = stream->max_buffer_size;

    // Extract optional proposed DID (byte 5) -- informational only, we assign our own
    // Extract optional Stream Content UID (bytes 6-11)
    if (incoming->payload_count >= 12) {

        for (int i = 0; i < 6; i++) {

            stream->content_uid[i] = OpenLcbUtilities_extract_byte_from_openlcb_payload(incoming, 6 + i);

        }

    }

    // Notify application -- callback returns true to accept, false to reject
    bool accepted = _interface->on_initiate_request(statemachine_info, stream);

    if (accepted) {

        stream->state = STREAM_STATE_OPEN;

        _load_initiate_reply(
                statemachine_info,
                stream->max_buffer_size,
                STREAM_REPLY_ACCEPT,
                stream->source_stream_id,
                stream->dest_stream_id);

    } else {

        uint8_t sid = stream->source_stream_id;

        _free_stream(stream);

        _load_initiate_reply(statemachine_info, 0, ERROR_PERMANENT_STREAMS_NOT_SUPPORTED, sid, 0);

    }

}

    /**
     * @brief Handles incoming Stream Initiate Reply (MTI 0x0868).
     *
     * @details Algorithm:
     * -# Extract max_buffer_size (bytes 0-1), flags (bytes 2-3), SID (byte 4), DID (byte 5)
     * -# Find stream state entry by SID and remote node ID
     * -# If not found, ignore (stale or unknown reply)
     * -# If flags indicate accept (0x8000 or 0x0000 with non-zero buffer), transition to OPEN
     * -# If reject, free the stream state entry
     * -# Notify application callback
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @endverbatim
     */
void ProtocolStreamHandler_initiate_reply(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *incoming = statemachine_info->incoming_msg_info.msg_ptr;

    uint16_t negotiated_buffer_size = OpenLcbUtilities_extract_word_from_openlcb_payload(incoming, 0);
    uint16_t flags = OpenLcbUtilities_extract_word_from_openlcb_payload(incoming, 2);
    uint8_t source_stream_id = OpenLcbUtilities_extract_byte_from_openlcb_payload(incoming, 4);
    uint8_t dest_stream_id = OpenLcbUtilities_extract_byte_from_openlcb_payload(incoming, 5);

    // Find our stream by SID (we are the source)
    stream_state_t *stream = _find_stream(incoming->source_id, source_stream_id, true);

    if (!stream) { return; }

    // Check for accept: non-zero buffer AND (0x8000 bit set or 0x0000)
    bool accepted = (negotiated_buffer_size > 0) && (((flags & STREAM_REPLY_ACCEPT) != 0) || (flags == 0x0000));

    if (accepted) {

        stream->state = STREAM_STATE_OPEN;
        stream->dest_stream_id = dest_stream_id;
        stream->max_buffer_size = negotiated_buffer_size;
        stream->bytes_remaining = negotiated_buffer_size;

    } else {

        _free_stream(stream);

    }

    if (_interface->on_initiate_reply) {

        _interface->on_initiate_reply(statemachine_info, stream);

    }

}

    /**
     * @brief Handles incoming Stream Data Send (MTI 0x1F88).
     *
     * @details Algorithm:
     * -# Extract DID from byte 0 of payload
     * -# Find stream state entry by DID and remote node ID
     * -# If not found, ignore
     * -# Track bytes received (payload_count - 1 for the DID byte)
     * -# Decrement bytes_remaining by data length
     * -# Notify application callback
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @endverbatim
     */
void ProtocolStreamHandler_data_send(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *incoming = statemachine_info->incoming_msg_info.msg_ptr;

    uint8_t dest_stream_id = OpenLcbUtilities_extract_byte_from_openlcb_payload(incoming, 0);

    stream_state_t *stream = _find_stream(incoming->source_id, dest_stream_id, false);

    if (!stream) { return; }

    if (stream->state != STREAM_STATE_OPEN) { return; }

    // Data bytes are payload[1..payload_count-1]
    uint16_t data_len = 0;

    if (incoming->payload_count > 1) {

        data_len = incoming->payload_count - 1;

    }

    stream->bytes_transferred += data_len;

    if (stream->bytes_remaining >= data_len) {

        stream->bytes_remaining -= data_len;

    } else {

        stream->bytes_remaining = 0;

    }

    if (_interface->on_data_received) {

        _interface->on_data_received(statemachine_info, stream);

    }

    // Send Proceed when the full buffer window has been received
    if (stream->bytes_remaining == 0) {

        stream->bytes_remaining = stream->max_buffer_size;

        _load_data_proceed(statemachine_info, stream);

    }

}

    /**
     * @brief Handles incoming Stream Data Proceed (MTI 0x0888).
     *
     * @details Algorithm:
     * -# Extract SID (byte 0) and DID (byte 1) from payload
     * -# Find stream state entry by SID and remote node ID (we are source)
     * -# If not found, ignore
     * -# Extend send window by max_buffer_size
     * -# Notify application callback
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @endverbatim
     */
void ProtocolStreamHandler_data_proceed(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *incoming = statemachine_info->incoming_msg_info.msg_ptr;

    uint8_t source_stream_id = OpenLcbUtilities_extract_byte_from_openlcb_payload(incoming, 0);

    stream_state_t *stream = _find_stream(incoming->source_id, source_stream_id, true);

    if (!stream) { return; }

    if (stream->state != STREAM_STATE_OPEN) { return; }

    // Extend send window
    stream->bytes_remaining += stream->max_buffer_size;

    if (_interface->on_data_proceed) {

        _interface->on_data_proceed(statemachine_info, stream);

    }

}

    /**
     * @brief Handles incoming Stream Data Complete (MTI 0x08A8).
     *
     * @details Algorithm:
     * -# Extract SID (byte 0) and DID (byte 1) from payload
     * -# Find stream by SID+remote (if we are destination) or DID+remote (if we are source)
     * -# If not found, ignore
     * -# Notify application callback
     * -# Free the stream state entry
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @endverbatim
     */
void ProtocolStreamHandler_data_complete(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *incoming = statemachine_info->incoming_msg_info.msg_ptr;

    uint8_t source_stream_id = OpenLcbUtilities_extract_byte_from_openlcb_payload(incoming, 0);
    uint8_t dest_stream_id = OpenLcbUtilities_extract_byte_from_openlcb_payload(incoming, 1);

    // Try to find by SID (we are destination, remote is source)
    stream_state_t *stream = _find_stream(incoming->source_id, source_stream_id, true);

    // If not found, try by DID (we are source, remote is destination)
    if (!stream) {

        stream = _find_stream(incoming->source_id, dest_stream_id, false);

    }

    if (!stream) { return; }

    if (_interface->on_complete) {

        _interface->on_complete(statemachine_info, stream);

    }

    _free_stream(stream);

}

    /**
     * @brief Handles Terminate Due To Error for stream-related MTIs.
     *
     * @details Per StreamTransportS Section 7.4, a stream is closed if:
     * -# The destination sends TDE with rejected_mti = MTI_STREAM_SEND
     * -# The source sends TDE with rejected_mti = MTI_STREAM_PROCEED
     *
     * Algorithm:
     * -# If payload < 4 bytes, return (no rejected_mti field)
     * -# Extract rejected_mti from bytes 2-3
     * -# If not a stream MTI, return
     * -# Scan stream table for entries matching remote node ID
     * -# For each match, notify on_complete callback and free the entry
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @endverbatim
     */
void ProtocolStreamHandler_handle_terminate_due_to_error(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *incoming = statemachine_info->incoming_msg_info.msg_ptr;

    // Need at least 4 bytes: error_code (2) + rejected_mti (2)
    if (incoming->payload_count < 4) { return; }

    uint16_t rejected_mti = OpenLcbUtilities_extract_word_from_openlcb_payload(incoming, 2);

    if ((rejected_mti != MTI_STREAM_SEND) && (rejected_mti != MTI_STREAM_PROCEED)) { return; }

    // Close all streams with this remote node
    for (int i = 0; i < USER_DEFINED_MAX_STREAM_COUNT; i++) {

        if (_stream_table[i].state == STREAM_STATE_CLOSED) { continue; }

        if (_stream_table[i].remote_node_id != incoming->source_id) { continue; }

        if (_interface->on_complete) {

            _interface->on_complete(statemachine_info, &_stream_table[i]);

        }

        _free_stream(&_stream_table[i]);

    }

}

    /**
     * @brief Opens an outbound stream where this node is the source.
     *
     * @details Algorithm:
     * -# Allocate a stream state entry; return NULL if table full
     * -# Assign a SID, populate state with is_source=true, INITIATED
     * -# Send Stream Initiate Request to the destination
     * -# Return pointer to stream state entry
     *
     * The caller waits for on_initiate_reply to learn accept/reject.
     *
     * @verbatim
     * @param statemachine_info         Pointer to openlcb_statemachine_info_t context.
     * @param dest_alias                CAN alias of the destination node.
     * @param dest_id                   Node ID of the destination node.
     * @param proposed_buffer_size      Proposed max buffer size.
     * @param suggested_dest_stream_id  Suggested DID (0xFF if no preference).
     * @endverbatim
     *
     * @return Pointer to allocated stream_state_t, or NULL if table full.
     */
stream_state_t *ProtocolStreamHandler_initiate_outbound(
            openlcb_statemachine_info_t *statemachine_info,
            uint16_t dest_alias,
            node_id_t dest_id,
            uint16_t proposed_buffer_size,
            uint8_t suggested_dest_stream_id,
            const uint8_t *content_uid) {

    stream_state_t *stream = _allocate_stream();

    if (!stream) { return NULL; }

    stream->state = STREAM_STATE_INITIATED;
    stream->source_stream_id = _assign_source_stream_id();
    stream->dest_stream_id = 0;
    stream->remote_node_id = dest_id;
    stream->remote_alias = dest_alias;
    stream->is_source = true;
    stream->max_buffer_size = proposed_buffer_size;
    stream->bytes_transferred = 0;
    stream->bytes_remaining = 0;
    stream->context = NULL;

    if (content_uid) {

        for (int i = 0; i < 6; i++) {

            stream->content_uid[i] = content_uid[i];

        }

    }

    _load_initiate_request(
            statemachine_info,
            proposed_buffer_size,
            stream->source_stream_id,
            suggested_dest_stream_id,
            dest_alias,
            dest_id,
            content_uid);

    return stream;

}

    /**
     * @brief Sends data on an open stream where this node is the source.
     *
     * @details Algorithm:
     * -# Validate stream is non-NULL, OPEN, and is_source
     * -# Validate data_len fits within bytes_remaining
     * -# Build and send Stream Data Send message
     * -# Update bytes_transferred and bytes_remaining
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to the stream state entry.
     * @param data               Pointer to data bytes to send.
     * @param data_len           Number of data bytes.
     * @endverbatim
     *
     * @return true if data was sent, false if precondition failed.
     */
bool ProtocolStreamHandler_send_data(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream,
            const uint8_t *data,
            uint16_t data_len) {

    if (!stream) { return false; }

    if (stream->state != STREAM_STATE_OPEN) { return false; }

    if (!stream->is_source) { return false; }

    if (data_len > stream->bytes_remaining) { return false; }

    _load_data_send(statemachine_info, stream, data, data_len);

    stream->bytes_transferred += data_len;
    stream->bytes_remaining -= data_len;

    return true;

}

    /**
     * @brief Sends Stream Data Complete and frees the stream slot.
     *
     * @details Either the source or destination may call this to close
     * the stream normally.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to the stream state entry.
     * @endverbatim
     */
void ProtocolStreamHandler_send_complete(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream) {

    if (!stream) { return; }

    _load_data_complete(statemachine_info, stream);

    _free_stream(stream);

}

    /**
     * @brief Sends Terminate Due To Error to abort a stream and frees the slot.
     *
     * @details The rejected MTI is chosen automatically based on the role:
     * - Source sends TDE with rejected_mti = MTI_STREAM_PROCEED
     * - Destination sends TDE with rejected_mti = MTI_STREAM_SEND
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to the stream state entry.
     * @param error_code         Error code for bytes 0-1 of the TDE payload.
     * @endverbatim
     */
void ProtocolStreamHandler_send_terminate(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream,
            uint16_t error_code) {

    if (!stream) { return; }

    uint16_t rejected_mti = stream->is_source ? MTI_STREAM_PROCEED : MTI_STREAM_SEND;

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            stream->remote_alias,
            stream->remote_node_id,
            MTI_TERMINATE_DUE_TO_ERROR);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, error_code, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, rejected_mti, 2);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 4;
    statemachine_info->outgoing_msg_info.valid = true;

    _free_stream(stream);

}

    /**
     * @brief Sends an early Stream Data Proceed to allow double-buffering.
     *
     * @details Per StreamTransportS Section 7.3, the destination may send one
     * advance Proceed before the full buffer window is consumed.  This extends
     * the source's send window by max_buffer_size.  Only valid when this node
     * is the destination and the stream is OPEN.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to the stream state entry.
     * @endverbatim
     *
     * @return true if Proceed was sent, false if precondition failed.
     */
bool ProtocolStreamHandler_send_early_proceed(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream) {

    if (!stream) { return false; }

    if (stream->state != STREAM_STATE_OPEN) { return false; }

    if (stream->is_source) { return false; }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            stream->remote_alias,
            stream->remote_node_id,
            MTI_STREAM_PROCEED);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->source_stream_id, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->dest_stream_id, 1);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 2;
    statemachine_info->outgoing_msg_info.valid = true;

    return true;

}

#endif /* OPENLCB_COMPILE_STREAM */
