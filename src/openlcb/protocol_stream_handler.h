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
 * @file protocol_stream_handler.h
 * @brief Stream Transport protocol handler.
 *
 * @details Handles stream initiation, data transfer with flow control, and
 * stream completion per the OpenLCB Stream Transport Standard (Feb 2026).
 * All application callbacks are optional (NULL = reject or ignore).
 *
 * @author Jim Kueneman
 * @date 03 Apr 2026
 */

#ifndef __OPENLCB_PROTOCOL_STREAM_HANDLER__
#define __OPENLCB_PROTOCOL_STREAM_HANDLER__

#include "openlcb_types.h"

#ifdef OPENLCB_COMPILE_STREAM

#include <stdbool.h>
#include <stdint.h>

// =============================================================================
// Stream state enumeration
// =============================================================================

    /** @brief Stream lifecycle states per the OpenLCB Stream Transport Standard. */
typedef enum {

    STREAM_STATE_CLOSED,
    STREAM_STATE_ANNOUNCED,
    STREAM_STATE_INITIATED,
    STREAM_STATE_OPEN

} stream_state_enum;

// =============================================================================
// Stream state entry
// =============================================================================

    /**
     * @brief Tracks the state of a single open stream.
     *
     * @details One entry per concurrent stream.  The table is a fixed-size
     * array dimensioned by USER_DEFINED_MAX_STREAM_COUNT.
     */
typedef struct {

        /** @brief Current lifecycle state. */
    stream_state_enum state;

        /** @brief Source Stream ID assigned by the source node. */
    uint8_t source_stream_id;

        /** @brief Destination Stream ID assigned by the destination node. */
    uint8_t dest_stream_id;

        /** @brief Node ID of the remote end of the stream. */
    node_id_t remote_node_id;

        /** @brief Negotiated max buffer size (bytes per window). */
    uint16_t max_buffer_size;

        /** @brief Running total of payload bytes transferred. */
    uint32_t bytes_transferred;

        /** @brief Bytes remaining in current send window before pause. */
    uint16_t bytes_remaining;

        /** @brief True if this node is the source (sender) of the stream. */
    bool is_source;

        /** @brief Optional 6-byte Stream Content UID (zero if not provided). */
    uint8_t content_uid[6];

} stream_state_t;

// =============================================================================
// DI interface struct
// =============================================================================

    /**
     * @brief Application callbacks for stream transport notifications.
     *
     * @details All callbacks are optional (NULL = reject initiate requests,
     * ignore other messages).  The stream_state pointer is valid for the
     * duration of the callback.
     */
typedef struct {

        /** @brief Stream Initiate Request received (this node is destination).  Optional. */
    void (*on_initiate_request)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

        /** @brief Stream Initiate Reply received (this node is source).  Optional. */
    void (*on_initiate_reply)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

        /** @brief Stream data received.  Optional. */
    void (*on_data_received)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

        /** @brief Stream Data Proceed received (flow control ack).  Optional. */
    void (*on_data_proceed)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

        /** @brief Stream completed or closed.  Optional. */
    void (*on_complete)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

} interface_protocol_stream_handler_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup.
         *
         * @param interface_protocol_stream_handler  Pointer to @ref interface_protocol_stream_handler_t (must remain valid for application lifetime).
         */
    extern void ProtocolStreamHandler_initialize(const interface_protocol_stream_handler_t *interface_protocol_stream_handler);

        /**
         * @brief Handles incoming Stream Initiate Request (MTI 0x0CC8).
         *
         * @details Parses the request, allocates a stream state entry, negotiates
         * buffer size, and sends a Stream Initiate Reply.  If no callback is
         * registered or the stream table is full, sends a reject reply.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolStreamHandler_initiate_request(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Stream Initiate Reply (MTI 0x0868).
         *
         * @details Parses accept/reject, updates stream state to OPEN or CLOSED,
         * and notifies the application callback.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolStreamHandler_initiate_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Stream Data Send (MTI 0x1F88).
         *
         * @details Tracks bytes received against the flow control window and
         * notifies the application callback with the received data.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolStreamHandler_data_send(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Stream Data Proceed (MTI 0x0888).
         *
         * @details Extends the send window by max_buffer_size and notifies the
         * application callback.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolStreamHandler_data_proceed(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Stream Data Complete (MTI 0x08A8).
         *
         * @details Finalizes the stream, frees the state entry, and notifies the
         * application callback.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolStreamHandler_data_complete(openlcb_statemachine_info_t *statemachine_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENLCB_COMPILE_STREAM */

#endif /* __OPENLCB_PROTOCOL_STREAM_HANDLER__ */
