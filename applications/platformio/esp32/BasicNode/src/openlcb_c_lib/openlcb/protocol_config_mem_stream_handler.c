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
 * @file protocol_config_mem_stream_handler.c
 * @brief Memory Configuration stream transport handler implementation.
 *
 * @details Implements Read Stream for all standard address spaces.  The module
 * owns a dedicated outgoing message buffer and data pump, following the same
 * pattern as the CAN and login state machines.
 *
 * @author Jim Kueneman
 * @date 04 Apr 2026
 *
 * @see protocol_config_mem_stream_handler.h
 * @see MemoryConfigurationS.pdf Section 4.6-4.7
 * @see StreamTransportS.pdf
 */

#include "protocol_config_mem_stream_handler.h"

#include "openlcb_config.h"

#ifdef OPENLCB_COMPILE_STREAM
#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#ifndef OPENLCB_COMPILE_BOOTLOADER

#include <stddef.h>
#include <string.h>

    /** @brief Timeout threshold in 100ms ticks (3 seconds, matches datagram handler). */
#define CONFIG_MEM_STREAM_TIMEOUT_TICKS 30

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"

// =============================================================================
// Static state
// =============================================================================

    /** @brief Stored DI interface pointer; set by _initialize(). */
static interface_protocol_config_mem_stream_handler_t *_interface;

    /** @brief Pool of config-mem stream contexts, one per concurrent operation. */
static config_mem_stream_context_t _context_pool[USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS];

    /** @brief Round-robin index for the pump loop in _run(). */
static uint8_t _pump_index;

    /** @brief Shared outgoing message buffer (worker-sized for max payload). */
static openlcb_worker_message_t _pump_outgoing_msg;

    /** @brief Shared statemachine info with embedded outgoing buffer. */
static openlcb_statemachine_info_t _pump_sm_info;

    /** @brief Shared buffer for read_request callbacks to fill before stream_send_data. */
static uint8_t _read_buffer[LEN_MESSAGE_BYTES_STREAM];

// =============================================================================
// Internal helpers
// =============================================================================

    /**
     * @brief Resets a context entry to idle.
     *
     * @param ctx  Pointer to the context to reset.
     */
static void _reset_context(config_mem_stream_context_t *ctx) {

    memset(ctx, 0, sizeof(*ctx));
    ctx->phase = CONFIG_MEM_STREAM_PHASE_IDLE;

}

    /**
     * @brief Allocates the first idle context from the pool.
     *
     * @return Pointer to an idle context, or NULL if all are busy.
     */
static config_mem_stream_context_t *_allocate_context(void) {

    for (uint8_t i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        if (_context_pool[i].phase == CONFIG_MEM_STREAM_PHASE_IDLE) {

            return &_context_pool[i];

        }

    }

    return NULL;

}

    /**
     * @brief Finds the context that owns the given stream.
     *
     * @param stream  Pointer to stream state entry.
     *
     * @return Pointer to the owning context, or NULL if not found.
     */
static config_mem_stream_context_t *_find_context_by_stream(stream_state_t *stream) {

    if (!stream) {

        return NULL;

    }

    for (uint8_t i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        if (_context_pool[i].phase != CONFIG_MEM_STREAM_PHASE_IDLE &&
                _context_pool[i].stream == stream) {

            return &_context_pool[i];

        }

    }

    return NULL;

}

    /**
     * @brief Finds the context in ALLOCATED phase for the given node.
     *
     * @details Used by the two-phase dispatch to retrieve the context
     * stashed in Phase 1 when Phase 2 re-enters the handler.
     *
     * @param node  Pointer to the node that owns the pending context.
     *
     * @return Pointer to the matching context, or NULL if not found.
     */
static config_mem_stream_context_t *_find_allocated_context(openlcb_node_t *node) {

    for (uint8_t i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        if (_context_pool[i].phase == CONFIG_MEM_STREAM_PHASE_ALLOCATED &&
                _context_pool[i].node == node) {

            return &_context_pool[i];

        }

    }

    return NULL;

}

    /**
     * @brief Finds a context waiting for a write stream initiate from the given alias.
     *
     * @param remote_alias  CAN alias of the remote node that will initiate.
     *
     * @return Pointer to the matching context, or NULL if not found.
     */
static config_mem_stream_context_t *_find_context_for_write_initiate(uint16_t remote_alias) {

    for (uint8_t i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        if (_context_pool[i].phase == CONFIG_MEM_STREAM_PHASE_WRITE_WAIT_STREAM_INITIATE &&
                _context_pool[i].remote_alias == remote_alias) {

            return &_context_pool[i];

        }

    }

    return NULL;

}

    /**
     * @brief Initializes the pump's outgoing message buffer.
     *
     * @details Wires the embedded openlcb_msg_t to its payload buffer,
     * following the same pattern as OpenLcbMainStatemachine_initialize().
     */
static void _init_pump_outgoing(void) {

    memset(&_pump_outgoing_msg, 0, sizeof(_pump_outgoing_msg));
    memset(&_pump_sm_info, 0, sizeof(_pump_sm_info));

    _pump_sm_info.outgoing_msg_info.msg_ptr = &_pump_outgoing_msg.openlcb_msg;
    _pump_sm_info.outgoing_msg_info.msg_ptr->payload =
            (openlcb_payload_t *) _pump_outgoing_msg.openlcb_payload;
    _pump_sm_info.outgoing_msg_info.msg_ptr->payload_type = WORKER;
    OpenLcbUtilities_clear_openlcb_message(_pump_sm_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_pump_sm_info.outgoing_msg_info.msg_ptr);
    _pump_sm_info.outgoing_msg_info.msg_ptr->state.allocated = true;
    _pump_sm_info.outgoing_msg_info.valid = false;

    _pump_sm_info.incoming_msg_info.msg_ptr = NULL;
    _pump_sm_info.incoming_msg_info.enumerate = false;
    _pump_sm_info.openlcb_node = NULL;

}

    /**
     * @brief Computes the number of bytes remaining to send for a read context.
     *
     * @details If total_bytes == 0 (read to end), returns bytes from
     * current_address to highest_address + 1.  Otherwise returns the lesser
     * of (total_bytes - bytes_sent) and bytes to end of space.
     *
     * @param ctx  Pointer to the active config-mem stream context.
     *
     * @return Number of bytes still to send.
     */
static uint32_t _bytes_remaining_to_send(config_mem_stream_context_t *ctx) {

    uint32_t space_end = ctx->space_info->highest_address + 1;

    if (ctx->current_address >= space_end) {

        return 0;

    }

    uint32_t available = space_end - ctx->current_address;

    if (ctx->total_bytes == 0) {

        return available;

    }

    uint32_t requested_remaining = ctx->total_bytes - ctx->bytes_sent;

    return (requested_remaining < available) ? requested_remaining : available;

}

    /**
     * @brief Loads a Read Stream Reply Fail datagram into the shared outgoing buffer.
     *
     * @details Payload layout:
     * - Byte 0: 0x20 (config mem command)
     * - Byte 1: reply_fail_cmd_byte (e.g. 0x7B for CDI)
     * - Bytes 2-5: Starting address (big-endian)
     * - Bytes 6-7: Error code (big-endian)
     *
     * @param ctx         Pointer to the active config-mem stream context.
     * @param error_code  The error code to include (e.g. out-of-bounds).
     */
static void _load_reply_fail_datagram(config_mem_stream_context_t *ctx, uint16_t error_code) {

    _pump_sm_info.openlcb_node = ctx->node;

    OpenLcbUtilities_load_openlcb_message(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->node->alias, ctx->node->id, ctx->remote_alias, ctx->remote_node_id, MTI_DATAGRAM);

    OpenLcbUtilities_clear_openlcb_message_payload(_pump_sm_info.outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, CONFIG_MEM_CONFIGURATION, 0);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->reply_fail_cmd_byte, 1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->start_address, 2);

    OpenLcbUtilities_copy_word_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, error_code, 6);

    _pump_sm_info.outgoing_msg_info.msg_ptr->payload_count = 8;
    _pump_sm_info.outgoing_msg_info.valid = true;

}

    /**
     * @brief Loads the Read Stream Reply OK datagram into the shared outgoing buffer.
     *
     * @details Payload layout (space in byte 1 encoding):
     * - Byte 0: 0x20 (config mem command)
     * - Byte 1: reply_cmd_byte (e.g. 0x73 for CDI)
     * - Bytes 2-5: Starting address (big-endian)
     * - Byte 6: Source Stream ID
     * - Byte 7: Destination Stream ID
     * - Bytes 8-11: Read Count (big-endian, from request)
     *
     * @param ctx  Pointer to the active config-mem stream context.
     */
static void _load_reply_ok_datagram(config_mem_stream_context_t *ctx) {

    _pump_sm_info.openlcb_node = ctx->node;

    OpenLcbUtilities_load_openlcb_message(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->node->alias, ctx->node->id, ctx->remote_alias, ctx->remote_node_id, MTI_DATAGRAM);

    OpenLcbUtilities_clear_openlcb_message_payload(_pump_sm_info.outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, CONFIG_MEM_CONFIGURATION, 0);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->reply_cmd_byte, 1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->start_address, 2);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->stream->source_stream_id, 6);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->dest_stream_id, 7);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->total_bytes, 8);

    _pump_sm_info.outgoing_msg_info.msg_ptr->payload_count = 12;
    _pump_sm_info.outgoing_msg_info.valid = true;

}

    /**
     * @brief Loads the next chunk of data into the shared outgoing buffer.
     *
     * @details Calls the per-space read_request callback to fill _read_buffer,
     * then passes the data to stream_send_data.  Chunk size is the minimum of:
     * stream bytes_remaining, bytes left in space, and (LEN_MESSAGE_BYTES_STREAM - 1)
     * to account for the DID prefix byte.
     *
     * @param ctx  Pointer to the active config-mem stream context.
     *
     * @return true if data was loaded, false if nothing to send.
     */
static bool _pump_next_chunk(config_mem_stream_context_t *ctx) {

    uint32_t remaining = _bytes_remaining_to_send(ctx);

    _pump_sm_info.openlcb_node = ctx->node;

    uint16_t stream_window = ctx->stream->bytes_remaining;

    if (stream_window == 0) {

        return false;

    }

    uint16_t max_payload = LEN_MESSAGE_BYTES_STREAM - 1;
    uint16_t chunk = (uint16_t) ((remaining < max_payload) ? remaining : max_payload);

    if (chunk > stream_window) {

        chunk = stream_window;

    }

    uint16_t actual = ctx->read_request_func(ctx->node, ctx->current_address, chunk, _read_buffer);

    if (actual == 0) {

        return false;

    }

    if (!_interface->stream_send_data(&_pump_sm_info, ctx->stream, _read_buffer, actual)) {

        return false;

    }

    ctx->current_address += actual;
    ctx->bytes_sent += actual;

    return true;

}

// =============================================================================
// Public API
// =============================================================================

    /**
     * @brief Stores the DI interface and initializes internal state.
     *
     * @verbatim
     * @param interface  Populated DI table.
     * @endverbatim
     */
void ProtocolConfigMemStreamHandler_initialize(const interface_protocol_config_mem_stream_handler_t *interface) {

    _interface = (interface_protocol_config_mem_stream_handler_t *) interface;

    for (uint8_t i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        _reset_context(&_context_pool[i]);

    }

    _pump_index = 0;
    _init_pump_outgoing();

}

    /**
     * @brief Shared two-phase dispatch for all Read Stream address spaces.
     *
     * @details Phase 1: Validate, send Datagram Received OK, parse and stash
     * request parameters, set enumerate for re-invoke.
     * Phase 2: Initiate outbound stream, transition to WAIT_INITIATE_REPLY.
     *
     * Read Stream Command layout:
     * - Byte 0: 0x20
     * - Byte 1: command byte (0x60-0x63 depending on space encoding)
     * - Bytes 2-5: Starting address
     * - Byte 6: space byte (for byte-6 encoding) or reserved
     * - Byte 7: Destination Stream ID (suggested by requester)
     * - Bytes 8-11: Read Count (0 = read to end)
     *
     * @param statemachine_info  Main state machine context.
     * @param space_info         Address space descriptor for the target space.
     * @param read_request_func  Per-space read callback (NULL = reject).
     * @param reply_cmd_byte      Reply OK command byte (e.g. 0x73 for CDI).
     * @param reply_fail_cmd_byte Reply Fail command byte (e.g. 0x7B for CDI).
     */
static void _handle_read_stream(openlcb_statemachine_info_t *statemachine_info, const user_address_space_info_t *space_info, config_mem_stream_read_request_func_t read_request_func, uint8_t reply_cmd_byte, uint8_t reply_fail_cmd_byte) {

    // Phase 1 needs a fresh context; Phase 2 reuses the one stashed on the node
    config_mem_stream_context_t *ctx = NULL;

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

        // Reject if no idle context available
        ctx = _allocate_context();

        if (!ctx) {

            _interface->load_datagram_received_rejected_message(statemachine_info, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

            return;

        }

        // Reject if space not present or no read callback
        if (!space_info->present || !read_request_func) {

            _interface->load_datagram_received_rejected_message(statemachine_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

            return;

        }

        // ---- Phase 1: ACK ----
        _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

        statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
        statemachine_info->incoming_msg_info.enumerate = true;

        // Parse and stash the request while the incoming message is still available
        ctx->phase = CONFIG_MEM_STREAM_PHASE_IDLE;
        ctx->node = statemachine_info->openlcb_node;
        ctx->space_info = space_info;
        ctx->read_request_func = read_request_func;
        ctx->start_address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);
        ctx->current_address = ctx->start_address;
        ctx->dest_stream_id = *statemachine_info->incoming_msg_info.msg_ptr->payload[7];
        ctx->reply_cmd_byte = reply_cmd_byte;
        ctx->reply_fail_cmd_byte = reply_fail_cmd_byte;
        ctx->bytes_sent = 0;
        ctx->stream = NULL;

        // Extract read count (bytes 8-11), 0 = read to end
        if (statemachine_info->incoming_msg_info.msg_ptr->payload_count >= 12) {

            ctx->total_bytes = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 8);

        } else {

            ctx->total_bytes = 0;

        }

        // Capture requester identity from the incoming message
        ctx->remote_alias = statemachine_info->incoming_msg_info.msg_ptr->source_alias;
        ctx->remote_node_id = statemachine_info->incoming_msg_info.msg_ptr->source_id;

        // Mark as allocated so Phase 2 can find it by node
        ctx->phase = CONFIG_MEM_STREAM_PHASE_ALLOCATED;

        return;

    }

    // ---- Phase 2: Validate address and initiate stream ----

    ctx = _find_allocated_context(statemachine_info->openlcb_node);

    if (!ctx) {

        statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false;
        statemachine_info->incoming_msg_info.enumerate = false;

        return;

    }

    // Reject if start address is past end of space
    if (ctx->start_address > ctx->space_info->highest_address) {

        _load_reply_fail_datagram(ctx, ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);

        statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false;
        statemachine_info->incoming_msg_info.enumerate = false;

        _reset_context(ctx);

        return;

    }

    stream_state_t *stream = _interface->stream_initiate_outbound(statemachine_info, ctx->remote_alias, ctx->remote_node_id, LEN_MESSAGE_BYTES_STREAM, ctx->dest_stream_id, NULL);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false;
    statemachine_info->incoming_msg_info.enumerate = false;

    if (!stream) {

        // Stream table full -- no way to reply (ACK already sent).
        // Reset and let the requester time out.
        _reset_context(ctx);

        return;

    }

    ctx->stream = stream;
    stream->context = ctx;
    ctx->tick_snapshot = statemachine_info->current_tick;
    ctx->phase = CONFIG_MEM_STREAM_PHASE_WAIT_INITIATE_REPLY;

}

// =============================================================================
// Public API -- per-space handler wrappers
// =============================================================================

    /** @brief Handles Read Stream CDI (0xFF). */
void ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(openlcb_statemachine_info_t *statemachine_info) {

    _handle_read_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_configuration_definition,
            _interface->read_request_config_definition_info,
            CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FF,
            CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FF);

}

    /** @brief Handles Read Stream All (0xFE). */
void ProtocolConfigMemStreamHandler_handle_read_stream_space_all(openlcb_statemachine_info_t *statemachine_info) {

    _handle_read_stream(statemachine_info, &statemachine_info->openlcb_node->parameters->address_space_all, _interface->read_request_all, CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FE, CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FE);

}

    /** @brief Handles Read Stream Config Memory (0xFD). */
void ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info) {

    _handle_read_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_config_memory,
            _interface->read_request_configuration_memory,
            CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FD,
            CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FD);

}

    /** @brief Handles Read Stream ACDI Manufacturer (0xFC). */
void ProtocolConfigMemStreamHandler_handle_read_stream_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info) {

    _handle_read_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer,
            _interface->read_request_acdi_manufacturer,
            CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6);

}

    /** @brief Handles Read Stream ACDI User (0xFB). */
void ProtocolConfigMemStreamHandler_handle_read_stream_space_acdi_user(openlcb_statemachine_info_t *statemachine_info) {

    _handle_read_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_acdi_user,
            _interface->read_request_acdi_user,
            CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6);

}

#ifdef OPENLCB_COMPILE_TRAIN

    /** @brief Handles Read Stream Train FDI (0xFA). */
void ProtocolConfigMemStreamHandler_handle_read_stream_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info) {

    _handle_read_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_train_function_definition_info,
            _interface->read_request_train_function_definition_info,
            CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6);

}

    /** @brief Handles Read Stream Train Fn Config (0xF9). */
void ProtocolConfigMemStreamHandler_handle_read_stream_space_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    _handle_read_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_train_function_config_memory,
            _interface->read_request_train_function_config_memory,
            CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6);

}

#endif /* OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_FIRMWARE

    /** @brief Handles Read Stream Firmware (0xEF). */
void ProtocolConfigMemStreamHandler_handle_read_stream_space_firmware(openlcb_statemachine_info_t *statemachine_info) {

    _handle_read_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_firmware,
            _interface->read_request_firmware,
            CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6);

}

#endif /* OPENLCB_COMPILE_FIRMWARE */

// =============================================================================
// Write Stream -- shared dispatch and per-space wrappers
// =============================================================================

    /**
     * @brief Loads a Write Stream Reply OK datagram into the pump's outgoing buffer.
     *
     * @details Payload layout (space in byte 1 encoding):
     * - Byte 0: 0x20 (config mem command)
     * - Byte 1: reply_cmd_byte (e.g. 0x31 for Config Memory Write Stream Reply OK)
     * - Bytes 2-5: Starting address (big-endian)
     * - Byte 6: Source Stream ID
     * - Byte 7: Destination Stream ID
     */
static void _load_write_reply_ok_datagram(config_mem_stream_context_t *ctx) {

    _pump_sm_info.openlcb_node = ctx->node;

    OpenLcbUtilities_load_openlcb_message(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->node->alias, ctx->node->id, ctx->remote_alias, ctx->remote_node_id, MTI_DATAGRAM);

    OpenLcbUtilities_clear_openlcb_message_payload(_pump_sm_info.outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, CONFIG_MEM_CONFIGURATION, 0);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->reply_cmd_byte, 1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->start_address, 2);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->source_stream_id, 6);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(_pump_sm_info.outgoing_msg_info.msg_ptr, ctx->dest_stream_id, 7);

    _pump_sm_info.outgoing_msg_info.msg_ptr->payload_count = 8;
    _pump_sm_info.outgoing_msg_info.valid = true;

}

    /**
     * @brief Shared two-phase dispatch for all Write Stream address spaces.
     *
     * @details Phase 1: Validate, send Datagram Received OK, parse and stash
     * request parameters, set enumerate for re-invoke.
     * Phase 2: Validate bounds, transition to WRITE_WAIT_STREAM_INITIATE.
     *
     * Write Stream Command layout:
     * - Byte 0: 0x20
     * - Byte 1: command byte (0x20-0x23 depending on space encoding)
     * - Bytes 2-5: Starting address
     * - Byte 6: space byte (for byte-6 encoding) or Source Stream ID
     * - Byte 7: Source Stream ID (for byte-6 encoding only)
     *
     * @param statemachine_info     Main state machine context.
     * @param space_info            Address space descriptor for the target space.
     * @param write_request_func    Per-space write callback (NULL = reject).
     * @param reply_cmd_byte        Reply OK command byte (e.g. 0x31 for Config).
     * @param reply_fail_cmd_byte   Reply Fail command byte (e.g. 0x39 for Config).
     * @param source_stream_id_offset Byte offset of Source Stream ID in payload (6 or 7).
     */
static void _handle_write_stream(
        openlcb_statemachine_info_t *statemachine_info,
        const user_address_space_info_t *space_info,
        config_mem_stream_write_request_func_t write_request_func,
        uint8_t reply_cmd_byte,
        uint8_t reply_fail_cmd_byte,
        uint8_t source_stream_id_offset) {

    config_mem_stream_context_t *ctx = NULL;

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

        // Reject if no idle context available
        ctx = _allocate_context();

        if (!ctx) {

            _interface->load_datagram_received_rejected_message(statemachine_info, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

            return;

        }

        // Reject if space not present, read-only, or no write callback
        if (!space_info->present || space_info->read_only || !write_request_func) {

            _interface->load_datagram_received_rejected_message(statemachine_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

            return;

        }

        // ---- Phase 1: ACK ----
        _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

        statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
        statemachine_info->incoming_msg_info.enumerate = true;

        // Parse and stash while incoming message is still available
        ctx->node = statemachine_info->openlcb_node;
        ctx->space_info = space_info;
        ctx->write_request_func = write_request_func;
        ctx->read_request_func = NULL;
        ctx->start_address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);
        ctx->current_address = ctx->start_address;
        ctx->source_stream_id = *statemachine_info->incoming_msg_info.msg_ptr->payload[source_stream_id_offset];
        ctx->reply_cmd_byte = reply_cmd_byte;
        ctx->reply_fail_cmd_byte = reply_fail_cmd_byte;
        ctx->bytes_sent = 0;
        ctx->total_bytes = 0;
        ctx->stream = NULL;

        // Capture requester identity
        ctx->remote_alias = statemachine_info->incoming_msg_info.msg_ptr->source_alias;
        ctx->remote_node_id = statemachine_info->incoming_msg_info.msg_ptr->source_id;

        // Mark as allocated so Phase 2 can find it by node
        ctx->phase = CONFIG_MEM_STREAM_PHASE_ALLOCATED;

        return;

    }

    // ---- Phase 2: Validate address and wait for stream ----

    ctx = _find_allocated_context(statemachine_info->openlcb_node);

    if (!ctx) {

        statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false;
        statemachine_info->incoming_msg_info.enumerate = false;

        return;

    }

    // Reject if start address is past end of space
    if (ctx->start_address > ctx->space_info->highest_address) {

        _load_reply_fail_datagram(ctx, ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);

        statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false;
        statemachine_info->incoming_msg_info.enumerate = false;

        _reset_context(ctx);

        return;

    }

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false;
    statemachine_info->incoming_msg_info.enumerate = false;

    ctx->tick_snapshot = statemachine_info->current_tick;
    ctx->phase = CONFIG_MEM_STREAM_PHASE_WRITE_WAIT_STREAM_INITIATE;

}

// =============================================================================
// Public API -- per-space write stream handler wrappers
// =============================================================================

    /** @brief Handles Write Stream CDI (0xFF) -- rejected (read-only). */
void ProtocolConfigMemStreamHandler_handle_write_stream_space_config_description_info(openlcb_statemachine_info_t *statemachine_info) {

    _handle_write_stream(statemachine_info, &statemachine_info->openlcb_node->parameters->address_space_configuration_definition, NULL, CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FF, CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FF, 6);

}

    /** @brief Handles Write Stream All (0xFE) -- rejected (read-only). */
void ProtocolConfigMemStreamHandler_handle_write_stream_space_all(openlcb_statemachine_info_t *statemachine_info) {

    _handle_write_stream(statemachine_info, &statemachine_info->openlcb_node->parameters->address_space_all, NULL, CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FE, CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FE, 6);

}

    /** @brief Handles Write Stream Config Memory (0xFD). */
void ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info) {

    _handle_write_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_config_memory,
            _interface->write_request_configuration_memory,
            CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FD,
            CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FD,
            6);

}

    /** @brief Handles Write Stream ACDI Manufacturer (0xFC) -- rejected (read-only). */
void ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info) {

    _handle_write_stream(statemachine_info, &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer, NULL, CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6, CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6, 7);

}

    /** @brief Handles Write Stream ACDI User (0xFB). */
void ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_user(openlcb_statemachine_info_t *statemachine_info) {

    _handle_write_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_acdi_user,
            _interface->write_request_acdi_user,
            CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6,
            7);

}

#ifdef OPENLCB_COMPILE_TRAIN

    /** @brief Handles Write Stream Train FDI (0xFA) -- rejected (read-only). */
void ProtocolConfigMemStreamHandler_handle_write_stream_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info) {

    _handle_write_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_train_function_definition_info,
            NULL,
            CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6,
            7);

}

    /** @brief Handles Write Stream Train Fn Config (0xF9). */
void ProtocolConfigMemStreamHandler_handle_write_stream_space_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    _handle_write_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_train_function_config_memory,
            _interface->write_request_train_function_config_memory,
            CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6,
            7);

}

#endif /* OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_FIRMWARE

    /** @brief Handles Write Stream Firmware (0xEF). */
void ProtocolConfigMemStreamHandler_handle_write_stream_space_firmware(openlcb_statemachine_info_t *statemachine_info) {

    _handle_write_stream(
            statemachine_info,
            &statemachine_info->openlcb_node->parameters->address_space_firmware,
            _interface->write_request_firmware,
            CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6,
            CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6,
            7);

}

#endif /* OPENLCB_COMPILE_FIRMWARE */

    /**
     * @brief Drives the config-mem stream data pump.
     *
     * @details Called once per main loop iteration from OpenLcbConfig_run().
     * State machine:
     * - IDLE: nothing to do
     * - WAIT_INITIATE_REPLY: waiting for stream accept (handled by callback)
     * - SEND_REPLY_DATAGRAM: load and send Read Stream Reply OK
     * - PUMPING: load and send next data chunk
     * - SEND_COMPLETE: send Stream Data Complete
     * - WRITE_WAIT_STREAM_INITIATE: waiting for inbound stream (callback)
     * - WRITE_RECEIVING: receiving data (callback)
     * - WRITE_SEND_REPLY: load and send Write Stream Reply OK
     */
void ProtocolConfigMemStreamHandler_run(void) {

    // If there is a pending outgoing message, try to send it
    if (_pump_sm_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_pump_sm_info.outgoing_msg_info.msg_ptr)) {

            _pump_sm_info.outgoing_msg_info.valid = false;

        }

        return;

    }

    // Round-robin scan: find the next context that needs service
    for (uint8_t i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        uint8_t idx = (_pump_index + i) % USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS;
        config_mem_stream_context_t *ctx = &_context_pool[idx];

        switch (ctx->phase) {

            case CONFIG_MEM_STREAM_PHASE_SEND_REPLY_DATAGRAM:

                _load_reply_ok_datagram(ctx);
                ctx->phase = CONFIG_MEM_STREAM_PHASE_PUMPING;

                _pump_index = (idx + 1) % USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS;

                return;

            case CONFIG_MEM_STREAM_PHASE_PUMPING:

                if (_bytes_remaining_to_send(ctx) == 0) {

                    ctx->phase = CONFIG_MEM_STREAM_PHASE_SEND_COMPLETE;

                    _pump_index = (idx + 1) % USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS;

                    return;

                }

                _pump_next_chunk(ctx);

                _pump_index = (idx + 1) % USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS;

                return;

            case CONFIG_MEM_STREAM_PHASE_SEND_COMPLETE:

                _pump_sm_info.openlcb_node = ctx->node;
                _interface->stream_send_complete(&_pump_sm_info, ctx->stream);
                _reset_context(ctx);

                _pump_index = (idx + 1) % USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS;

                return;

            case CONFIG_MEM_STREAM_PHASE_WRITE_SEND_REPLY:

                _load_write_reply_ok_datagram(ctx);
                _reset_context(ctx);

                _pump_index = (idx + 1) % USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS;

                return;

            default:

                // IDLE, ALLOCATED, WAIT_INITIATE_REPLY,
                // WRITE_WAIT_STREAM_INITIATE, WRITE_RECEIVING
                // -- nothing to pump for these phases
                break;

        }

    }

}

    /**
     * @brief Checks for timed-out config-mem stream operations.
     *
     * @details If the active operation is in WAIT_INITIATE_REPLY, PUMPING,
     * WRITE_WAIT_STREAM_INITIATE, or WRITE_RECEIVING and the elapsed time
     * exceeds the timeout threshold, terminates the stream (if open) and
     * resets to idle.
     *
     * @verbatim
     * @param current_tick  Current value of the global 100ms tick counter.
     * @endverbatim
     */
void ProtocolConfigMemStreamHandler_check_timeouts(uint8_t current_tick) {

    for (uint8_t i = 0; i < USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS; i++) {

        config_mem_stream_context_t *ctx = &_context_pool[i];

        if (ctx->phase != CONFIG_MEM_STREAM_PHASE_WAIT_INITIATE_REPLY &&
                ctx->phase != CONFIG_MEM_STREAM_PHASE_PUMPING &&
                ctx->phase != CONFIG_MEM_STREAM_PHASE_WRITE_WAIT_STREAM_INITIATE &&
                ctx->phase != CONFIG_MEM_STREAM_PHASE_WRITE_RECEIVING) {

            continue;

        }

        uint8_t elapsed = (uint8_t) (current_tick - ctx->tick_snapshot);

        if (elapsed < CONFIG_MEM_STREAM_TIMEOUT_TICKS) {

            continue;

        }

        // Timed out -- terminate the stream if it exists and reset
        if (ctx->stream) {

            _pump_sm_info.openlcb_node = ctx->node;
            _interface->stream_send_terminate(&_pump_sm_info, ctx->stream, ERROR_TEMPORARY_TIME_OUT);

        }

        _reset_context(ctx);

    }

}

// =============================================================================
// Stream handler callback routers
// =============================================================================

    /**
     * @brief Routes Stream Initiate Request.
     *
     * @details For read streams, this module is always the source (outbound),
     * so inbound initiate requests are never ours.  For write streams, the
     * remote initiates an inbound stream to us after we ACK the Write Stream
     * Command datagram.  We match by phase and remote alias.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to stream_state_t entry.
     * @endverbatim
     *
     * @return true to accept, false to reject.
     */
bool ProtocolConfigMemStreamHandler_on_initiate_request(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    // Check if this is the inbound stream for a pending write
    config_mem_stream_context_t *ctx = _find_context_for_write_initiate(stream->remote_alias);

    if (ctx) {

        stream->context = ctx;
        ctx->stream = stream;
        ctx->dest_stream_id = stream->dest_stream_id;
        ctx->tick_snapshot = statemachine_info->current_tick;
        ctx->phase = CONFIG_MEM_STREAM_PHASE_WRITE_RECEIVING;

        return true;

    }

    if (_interface->user_on_initiate_request) {

        return _interface->user_on_initiate_request(statemachine_info, stream);

    }

    return false;

}

    /**
     * @brief Routes Stream Initiate Reply.
     *
     * @details If the stream belongs to us, check accept/reject and transition
     * to SEND_REPLY_DATAGRAM or back to IDLE.  Otherwise forward to user.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to stream_state_t entry.
     * @endverbatim
     */
void ProtocolConfigMemStreamHandler_on_initiate_reply(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    config_mem_stream_context_t *ctx = _find_context_by_stream(stream);

    if (ctx) {

        if (stream->state == STREAM_STATE_OPEN) {

            ctx->tick_snapshot = statemachine_info->current_tick;
            ctx->phase = CONFIG_MEM_STREAM_PHASE_SEND_REPLY_DATAGRAM;

        } else {

            // Stream was rejected
            _reset_context(ctx);

        }

        return;

    }

    if (_interface->user_on_initiate_reply) {

        _interface->user_on_initiate_reply(statemachine_info, stream);

    }

}

    /**
     * @brief Routes Stream Data Received.
     *
     * @details For write streams, incoming data is written to the address space
     * via the per-space write callback.  For read streams, we are source-side
     * only and never receive data.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to stream_state_t entry.
     * @endverbatim
     */
void ProtocolConfigMemStreamHandler_on_data_received(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    config_mem_stream_context_t *ctx = _find_context_by_stream(stream);

    if (ctx && ctx->phase == CONFIG_MEM_STREAM_PHASE_WRITE_RECEIVING) {

        // Data payload starts after the DID byte at offset 0
        uint16_t payload_count = statemachine_info->incoming_msg_info.msg_ptr->payload_count;

        // write_request_func is guaranteed non-NULL by Phase 1 validation
        if (payload_count > 1) {

            uint16_t data_len = payload_count - 1;
            const uint8_t *data = (const uint8_t *) statemachine_info->incoming_msg_info.msg_ptr->payload[1];

            uint16_t actual = ctx->write_request_func(ctx->node, ctx->current_address, data_len, data);

            ctx->current_address += actual;
            ctx->bytes_sent += actual;

        }

        ctx->tick_snapshot = statemachine_info->current_tick;

        return;

    }

    if (_interface->user_on_data_received) {

        _interface->user_on_data_received(statemachine_info, stream);

    }

}

    /**
     * @brief Routes Stream Data Proceed.
     *
     * @details If the stream belongs to us, the Layer 1 stream handler has
     * already refilled bytes_remaining.  The pump will pick up and continue
     * on its next run() call.  No action needed here beyond recognition.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to stream_state_t entry.
     * @endverbatim
     */
void ProtocolConfigMemStreamHandler_on_data_proceed(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    config_mem_stream_context_t *ctx = _find_context_by_stream(stream);

    if (ctx) {

        // bytes_remaining was already refilled by the stream handler.
        // The pump will continue sending on its next run() call.
        ctx->tick_snapshot = statemachine_info->current_tick;

        return;

    }

    if (_interface->user_on_data_proceed) {

        _interface->user_on_data_proceed(statemachine_info, stream);

    }

}

    /**
     * @brief Routes Stream Complete or error.
     *
     * @details For write streams, completion means all data has been received.
     * Transition to WRITE_SEND_REPLY so the pump sends the Write Reply OK.
     * For read streams, the remote end closed early -- reset to idle.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param stream             Pointer to stream_state_t entry.
     * @endverbatim
     */
void ProtocolConfigMemStreamHandler_on_complete(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream) {

    config_mem_stream_context_t *ctx = _find_context_by_stream(stream);

    if (ctx) {

        if (ctx->phase == CONFIG_MEM_STREAM_PHASE_WRITE_RECEIVING) {

            // Write stream finished -- send reply OK via the pump
            ctx->phase = CONFIG_MEM_STREAM_PHASE_WRITE_SEND_REPLY;

        } else {

            // Read stream: remote closed early
            _reset_context(ctx);

        }

        return;

    }

    if (_interface->user_on_complete) {

        _interface->user_on_complete(statemachine_info, stream);

    }

}

#endif /* OPENLCB_COMPILE_BOOTLOADER */
#endif /* OPENLCB_COMPILE_MEMORY_CONFIGURATION */
#endif /* OPENLCB_COMPILE_STREAM */
