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
     * @file protocol_config_mem_write_handler.h
     * @brief Configuration memory write handler — two-phase dispatch for write
     *        commands across all standard address spaces.
     *
     * @author Jim Kueneman
     * @date 28 Feb 2026
     *
     * @see MemoryConfigurationS.pdf
     */

#ifndef __OPENLCB_PROTOCOL_CONFIG_MEM_WRITE_HANDLER__
#define    __OPENLCB_PROTOCOL_CONFIG_MEM_WRITE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Callback interface for memory-config write operations.
     *
     * @details Required: load_datagram_received_ok_message,
     *          load_datagram_received_rejected_message, config_memory_write.
     *          All per-space handlers and delayed_reply_time are optional (NULL).
     */
typedef struct {

    // ---- Required ----

        /** @brief REQUIRED — Send Datagram Received OK with reply-pending time. */
    void (*load_datagram_received_ok_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);

        /** @brief REQUIRED — Send Datagram Received Rejected with error code. */
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

        /** @brief REQUIRED — Write bytes to config memory; returns bytes written. */
    uint16_t(*config_memory_write) (openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

    // ---- Optional per-space write handlers ----

        /** @brief Optional — CDI (0xFF) write handler (normally read-only). */
    void (*write_request_config_definition_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);
        /** @brief Optional — All (0xFE) write handler. */
    void (*write_request_all)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);
        /** @brief Optional — Config (0xFD) write handler. */
    void (*write_request_config_mem)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);
        /** @brief Optional — ACDI-Mfg (0xFC) write handler (normally read-only). */
    void (*write_request_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);
        /** @brief Optional — ACDI-User (0xFB) write handler. */
    void (*write_request_acdi_user)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);
        /** @brief Optional — Train FDI (0xFA) write handler (normally read-only). */
    void (*write_request_train_function_config_definition_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);
        /** @brief Optional — Train Fn Config (0xF9) write handler. */
    void (*write_request_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);
        /** @brief Optional — Firmware (0xEF) write handler. */
    void (*write_request_firmware)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

    // ---- Optional extras ----

        /** @brief Optional — Override reply delay (return N → 2^N seconds).  Default 0. */
    uint16_t (*delayed_reply_time)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /** @brief Optional — Notifier fired when a train function changes via 0xF9 write. */
    void (*on_function_changed)(openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value);

} interface_protocol_config_mem_write_handler_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup.
         *
         * @param interface_protocol_config_mem_write_handler  Pointer to @ref interface_protocol_config_mem_write_handler_t (must remain valid for application lifetime).
         */
    extern void ProtocolConfigMemWriteHandler_initialize(const interface_protocol_config_mem_write_handler_t *interface_protocol_config_mem_write_handler);

    // ---- Incoming write commands (server side — this node is being written) ----

        /**
         * @brief Write to CDI space (0xFF).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_config_description_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Write to All space (0xFE).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_all(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Write to Config space (0xFD).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_config_memory(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Write to ACDI-Mfg space (0xFC).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Write to ACDI-User space (0xFB).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_acdi_user(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Write to Train FDI space (0xFA).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Write to Train Fn Config space (0xF9).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Write to Firmware space (0xEF).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_firmware(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Write-under-mask command.  Placeholder / stub.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param space              Address space number.
         * @param return_msg_ok      MTI for success reply.
         * @param return_msg_fail    MTI for failure reply.
         */
    extern void ProtocolConfigMemWriteHandler_write_space_under_mask_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

    // ---- Outgoing write requests (client side — writing to another node) ----

        /**
         * @brief Send write request targeting Config (0xFD) on another node.
         *
         * @param statemachine_info            Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_write_request_info Pointer to @ref config_mem_write_request_info_t request.
         */
    extern void ProtocolConfigMemWriteHandler_write_request_config_mem(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

        /**
         * @brief Send write request targeting ACDI-User (0xFB) on another node.
         *
         * @param statemachine_info            Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_write_request_info Pointer to @ref config_mem_write_request_info_t request.
         */
    extern void ProtocolConfigMemWriteHandler_write_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

        /**
         * @brief Write to Train Fn Config (0xF9): updates in-RAM functions[].
         *
         * @param statemachine_info            Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_write_request_info Pointer to @ref config_mem_write_request_info_t request.
         */
    extern void ProtocolConfigMemWriteHandler_write_request_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

    // ---- Generic message handlers (stubs for future use) ----

        /**
         * @brief Generic write message handler.  Placeholder.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param space              Address space number.
         * @param return_msg_ok      MTI for success reply.
         * @param return_msg_fail    MTI for failure reply.
         */
    extern void ProtocolConfigMemWriteHandler_write_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

        /**
         * @brief Generic write reply OK handler.  Placeholder.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param space              Address space number.
         */
    extern void ProtocolConfigMemWriteHandler_write_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);

        /**
         * @brief Generic write reply fail handler.  Placeholder.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param space              Address space number.
         */
    extern void ProtocolConfigMemWriteHandler_write_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_CONFIG_MEM_WRITE_HANDLER__ */