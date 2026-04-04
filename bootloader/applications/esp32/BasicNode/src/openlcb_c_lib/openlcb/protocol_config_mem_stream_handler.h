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
 * @file protocol_config_mem_stream_handler.h
 * @brief Memory Configuration stream transport handler.
 *
 * @details Layer 2 module that bridges the Memory Configuration datagram
 * commands (Read Stream, Write Stream) with the Layer 1 stream transport.
 * Implements Read Stream for all standard address spaces (0xFF CDI, 0xFE All,
 * 0xFD Config, 0xFC ACDI-Mfg, 0xFB ACDI-User, 0xFA Train FDI,
 * 0xF9 Train Fn Config, 0xEF Firmware).
 *
 * The module intercepts stream handler callbacks via the stream->context
 * pointer to distinguish config-mem streams from user application streams,
 * forwarding unrecognized streams to the user callbacks.
 *
 * @author Jim Kueneman
 * @date 04 Apr 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_CONFIG_MEM_STREAM_HANDLER__
#define __OPENLCB_PROTOCOL_CONFIG_MEM_STREAM_HANDLER__

#include "openlcb_types.h"

#ifdef OPENLCB_COMPILE_STREAM
#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#ifndef OPENLCB_COMPILE_BOOTLOADER

#include <stdbool.h>
#include <stdint.h>

#include "protocol_stream_handler.h"

// =============================================================================
// Config-mem stream phase enumeration
// =============================================================================

    /** @brief Internal state machine phases for a config-mem stream operation. */
typedef enum {

    CONFIG_MEM_STREAM_PHASE_IDLE,

    // ---- Transitional phase (Phase 1 done, awaiting Phase 2 re-entry) ----
    CONFIG_MEM_STREAM_PHASE_ALLOCATED,

    // ---- Read phases ----
    CONFIG_MEM_STREAM_PHASE_WAIT_INITIATE_REPLY,
    CONFIG_MEM_STREAM_PHASE_SEND_REPLY_DATAGRAM,
    CONFIG_MEM_STREAM_PHASE_PUMPING,
    CONFIG_MEM_STREAM_PHASE_SEND_COMPLETE,

    // ---- Write phases ----
    CONFIG_MEM_STREAM_PHASE_WRITE_WAIT_STREAM_INITIATE,
    CONFIG_MEM_STREAM_PHASE_WRITE_RECEIVING,
    CONFIG_MEM_STREAM_PHASE_WRITE_SEND_REPLY

} config_mem_stream_phase_enum;

// =============================================================================
// Read-request callback typedef
// =============================================================================

    /**
     * @brief Signature for a per-space stream read callback.
     *
     * @details Reads up to @p count bytes from the address space at @p address
     * into @p buffer.  Returns the number of bytes actually read.
     *
     * @param node     Node whose address space is being read.
     * @param address  Starting byte offset within the address space.
     * @param count    Maximum number of bytes to read.
     * @param buffer   Destination buffer (caller-owned, at least @p count bytes).
     *
     * @return Actual number of bytes placed in @p buffer.
     */
typedef uint16_t (*config_mem_stream_read_request_func_t)(
        openlcb_node_t *node,
        uint32_t address,
        uint16_t count,
        uint8_t *buffer);

// =============================================================================
// Write-request callback typedef
// =============================================================================

    /**
     * @brief Signature for a per-space stream write callback.
     *
     * @details Writes up to @p count bytes from @p buffer into the address
     * space at @p address.  Returns the number of bytes actually written.
     *
     * @param node     Node whose address space is being written.
     * @param address  Starting byte offset within the address space.
     * @param count    Number of bytes to write.
     * @param buffer   Source buffer (caller-owned, at least @p count bytes).
     *
     * @return Actual number of bytes written.
     */
typedef uint16_t (*config_mem_stream_write_request_func_t)(
        openlcb_node_t *node,
        uint32_t address,
        uint16_t count,
        const uint8_t *buffer);

// =============================================================================
// Context for an active config-mem stream operation
// =============================================================================

    /**
     * @brief Tracks the state of one config-mem stream operation (read or write).
     *
     * @details A pool of these contexts is allocated in the module, sized by
     * USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS.  Multiple config-mem stream operations
     * can be active concurrently.  The phase enum distinguishes read vs write
     * operations.  Each active context is linked to its Layer 1 stream via
     * stream->context.
     */
typedef struct {

        /** @brief Current phase of the operation. */
    config_mem_stream_phase_enum phase;

        /** @brief Pointer to the Layer 1 stream state entry (owned by stream handler). */
    stream_state_t *stream;

        /** @brief Node being accessed. */
    openlcb_node_t *node;

        /** @brief Pointer to the address space info for the active space. */
    const user_address_space_info_t *space_info;

        /** @brief Per-space read callback (read operations only). */
    config_mem_stream_read_request_func_t read_request_func;

        /** @brief Per-space write callback (write operations only). */
    config_mem_stream_write_request_func_t write_request_func;

        /** @brief Starting address from the request datagram. */
    uint32_t start_address;

        /** @brief Current position in the address space. */
    uint32_t current_address;

        /** @brief Total bytes requested (0 = read to end of space). */
    uint32_t total_bytes;

        /** @brief Bytes already transferred on the stream. */
    uint32_t bytes_sent;

        /** @brief Reply OK command byte (e.g. 0x73 for CDI Read, 0x33 for CDI Write). */
    uint8_t reply_cmd_byte;

        /** @brief Reply Fail command byte (e.g. 0x7B for CDI Read, 0x3B for CDI Write). */
    uint8_t reply_fail_cmd_byte;

        /** @brief Destination Stream ID from the request datagram (read) or negotiated (write). */
    uint8_t dest_stream_id;

        /** @brief Source Stream ID from Write Stream Command datagram. */
    uint8_t source_stream_id;

        /** @brief Tick snapshot for timeout detection (full 8-bit, 100ms per tick). */
    uint8_t tick_snapshot;

        /** @brief CAN alias of the requester. */
    uint16_t remote_alias;

        /** @brief Node ID of the requester. */
    node_id_t remote_node_id;

} config_mem_stream_context_t;

// =============================================================================
// DI interface struct
// =============================================================================

    /**
     * @brief Dependency injection interface for the config-mem stream handler.
     *
     * @details All cross-module function calls are made through this interface,
     * wired by openlcb_config.c at startup.
     */
typedef struct {

    // ---- Transport ----

        /** @brief Transmit a message on the wire.  Returns true if sent. */
    bool (*send_openlcb_msg)(openlcb_msg_t *openlcb_msg);

    // ---- Datagram handler helpers ----

        /** @brief Load Datagram Received OK with reply pending timeout. */
    void (*load_datagram_received_ok_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);

        /** @brief Load Datagram Rejected with error code. */
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

    // ---- Stream handler API (Layer 1) ----

        /** @brief Initiate an outbound stream.  Returns stream pointer or NULL. */
    stream_state_t *(*stream_initiate_outbound)(
            openlcb_statemachine_info_t *statemachine_info,
            uint16_t dest_alias,
            node_id_t dest_id,
            uint16_t proposed_buffer_size,
            uint8_t suggested_dest_stream_id,
            const uint8_t *content_uid);

        /** @brief Send data on an open stream.  Returns true on success. */
    bool (*stream_send_data)(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream,
            const uint8_t *data,
            uint16_t data_len);

        /** @brief Send Stream Data Complete and free the stream slot. */
    void (*stream_send_complete)(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream);

        /** @brief Send Terminate Due To Error and free the stream slot. */
    void (*stream_send_terminate)(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream,
            uint16_t error_code);

    // ---- Per-space read request callbacks ----

        /** @brief Read from CDI (0xFF).  NULL if space not supported. */
    config_mem_stream_read_request_func_t read_request_config_definition_info;

        /** @brief Read from All (0xFE).  NULL if space not supported. */
    config_mem_stream_read_request_func_t read_request_all;

        /** @brief Read from Config Memory (0xFD).  NULL if space not supported. */
    config_mem_stream_read_request_func_t read_request_configuration_memory;

        /** @brief Read from ACDI Manufacturer (0xFC).  NULL if space not supported. */
    config_mem_stream_read_request_func_t read_request_acdi_manufacturer;

        /** @brief Read from ACDI User (0xFB).  NULL if space not supported. */
    config_mem_stream_read_request_func_t read_request_acdi_user;

#ifdef OPENLCB_COMPILE_TRAIN

        /** @brief Read from Train FDI (0xFA).  NULL if space not supported. */
    config_mem_stream_read_request_func_t read_request_train_function_definition_info;

        /** @brief Read from Train Fn Config (0xF9).  NULL if space not supported. */
    config_mem_stream_read_request_func_t read_request_train_function_config_memory;

#endif /* OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_FIRMWARE

        /** @brief Read from Firmware (0xEF).  NULL if space not supported. */
    config_mem_stream_read_request_func_t read_request_firmware;

#endif /* OPENLCB_COMPILE_FIRMWARE */

    // ---- Per-space write request callbacks ----

        /** @brief Write to Config Memory (0xFD).  NULL if space not supported. */
    config_mem_stream_write_request_func_t write_request_configuration_memory;

        /** @brief Write to ACDI User (0xFB).  NULL if space not supported. */
    config_mem_stream_write_request_func_t write_request_acdi_user;

#ifdef OPENLCB_COMPILE_TRAIN

        /** @brief Write to Train Fn Config (0xF9).  NULL if space not supported. */
    config_mem_stream_write_request_func_t write_request_train_function_config_memory;

#endif /* OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_FIRMWARE

        /** @brief Write to Firmware (0xEF).  NULL if space not supported. */
    config_mem_stream_write_request_func_t write_request_firmware;

#endif /* OPENLCB_COMPILE_FIRMWARE */

    // ---- User stream callbacks (forwarded when not a config-mem stream) ----

        /** @brief User on_initiate_request.  Optional (NULL to reject). */
    bool (*user_on_initiate_request)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

        /** @brief User on_initiate_reply.  Optional. */
    void (*user_on_initiate_reply)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

        /** @brief User on_data_received.  Optional. */
    void (*user_on_data_received)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

        /** @brief User on_data_proceed.  Optional. */
    void (*user_on_data_proceed)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

        /** @brief User on_complete.  Optional. */
    void (*user_on_complete)(openlcb_statemachine_info_t *statemachine_info, stream_state_t *stream);

} interface_protocol_config_mem_stream_handler_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the DI interface and initializes internal state.
         *
         * @param interface  Pointer to @ref interface_protocol_config_mem_stream_handler_t (must remain valid for application lifetime).
         */
    extern void ProtocolConfigMemStreamHandler_initialize(
            const interface_protocol_config_mem_stream_handler_t *interface);

    // ---- Datagram callbacks (wired into datagram handler interface) ----

        /**
         * @brief Handles incoming Read Stream CDI (0xFF) datagram command.
         *
         * @details Two-phase dispatch: Phase 1 sends Datagram Received OK,
         * Phase 2 initiates the outbound stream.
         *
         * @param statemachine_info  Main state machine context (from datagram handler).
         */
    extern void ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Read Stream All (0xFE) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_read_stream_space_all(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Read Stream Config Memory (0xFD) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Read Stream ACDI Manufacturer (0xFC) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_read_stream_space_acdi_manufacturer(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Read Stream ACDI User (0xFB) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_read_stream_space_acdi_user(
            openlcb_statemachine_info_t *statemachine_info);

#ifdef OPENLCB_COMPILE_TRAIN

        /**
         * @brief Handles incoming Read Stream Train FDI (0xFA) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_read_stream_space_train_function_definition_info(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Read Stream Train Fn Config (0xF9) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_read_stream_space_train_function_config_memory(
            openlcb_statemachine_info_t *statemachine_info);

#endif /* OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_FIRMWARE

        /**
         * @brief Handles incoming Read Stream Firmware (0xEF) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_read_stream_space_firmware(
            openlcb_statemachine_info_t *statemachine_info);

#endif /* OPENLCB_COMPILE_FIRMWARE */

    // ---- Write Stream datagram callbacks ----

        /**
         * @brief Handles incoming Write Stream CDI (0xFF) datagram command.
         *
         * @details Always rejected -- CDI is read-only.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_write_stream_space_config_description_info(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Write Stream All (0xFE) datagram command.
         *
         * @details Always rejected -- All space is read-only.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_write_stream_space_all(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Write Stream Config Memory (0xFD) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Write Stream ACDI Manufacturer (0xFC) datagram command.
         *
         * @details Always rejected -- ACDI Manufacturer is read-only.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_manufacturer(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Write Stream ACDI User (0xFB) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_user(
            openlcb_statemachine_info_t *statemachine_info);

#ifdef OPENLCB_COMPILE_TRAIN

        /**
         * @brief Handles incoming Write Stream Train FDI (0xFA) datagram command.
         *
         * @details Always rejected -- Train FDI is read-only.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_write_stream_space_train_function_definition_info(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles incoming Write Stream Train Fn Config (0xF9) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_write_stream_space_train_function_config_memory(
            openlcb_statemachine_info_t *statemachine_info);

#endif /* OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_FIRMWARE

        /**
         * @brief Handles incoming Write Stream Firmware (0xEF) datagram command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemStreamHandler_handle_write_stream_space_firmware(
            openlcb_statemachine_info_t *statemachine_info);

#endif /* OPENLCB_COMPILE_FIRMWARE */

    // ---- Pump (called from OpenLcbConfig_run) ----

        /**
         * @brief Drives the config-mem stream data pump.
         *
         * @details Called once per main loop iteration.  If an active stream
         * read is in progress and the outgoing buffer is free, loads the next
         * chunk of data and transmits it.
         */
    extern void ProtocolConfigMemStreamHandler_run(void);

    // ---- Timeout (called from _run_periodic_services) ----

        /**
         * @brief Checks for timed-out config-mem stream operations.
         *
         * @details Called from the periodic services loop with the current
         * global 100ms tick.  If the active operation has stalled for longer
         * than the timeout threshold, terminates the stream and resets to idle.
         *
         * @param current_tick  Current value of the global 100ms tick counter.
         */
    extern void ProtocolConfigMemStreamHandler_check_timeouts(uint8_t current_tick);

    // ---- Stream handler callbacks (wired as stream interface callbacks) ----

        /**
         * @brief Routes Stream Initiate Request between config-mem and user.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param stream             Pointer to @ref stream_state_t entry.
         *
         * @return true to accept, false to reject.
         */
    extern bool ProtocolConfigMemStreamHandler_on_initiate_request(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream);

        /**
         * @brief Routes Stream Initiate Reply between config-mem and user.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param stream             Pointer to @ref stream_state_t entry.
         */
    extern void ProtocolConfigMemStreamHandler_on_initiate_reply(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream);

        /**
         * @brief Routes Stream Data Received between config-mem and user.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param stream             Pointer to @ref stream_state_t entry.
         */
    extern void ProtocolConfigMemStreamHandler_on_data_received(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream);

        /**
         * @brief Routes Stream Data Proceed between config-mem and user.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param stream             Pointer to @ref stream_state_t entry.
         */
    extern void ProtocolConfigMemStreamHandler_on_data_proceed(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream);

        /**
         * @brief Routes Stream Complete between config-mem and user.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param stream             Pointer to @ref stream_state_t entry.
         */
    extern void ProtocolConfigMemStreamHandler_on_complete(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENLCB_COMPILE_BOOTLOADER */
#endif /* OPENLCB_COMPILE_MEMORY_CONFIGURATION */
#endif /* OPENLCB_COMPILE_STREAM */

#endif /* __OPENLCB_PROTOCOL_CONFIG_MEM_STREAM_HANDLER__ */
