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
     * @file protocol_config_mem_read_handler.h
     * @brief Configuration memory read handler — processes datagram read commands
     *        for CDI (0xFF), All (0xFD), Config (0xFE), ACDI-Mfg (0xFC),
     *        ACDI-User (0xFB), Train FDI (0xFA), and Train Fn Config (0xF9).
     *
     * @author Jim Kueneman
     * @date 28 Feb 2026
     */

#ifndef __OPENLCB_PROTOCOL_CONFIG_MEM_READ_HANDLER__
#define    __OPENLCB_PROTOCOL_CONFIG_MEM_READ_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /** @brief Callback interface for config memory reads.  Datagram ACK and
     *         config_memory_read are REQUIRED; SNIP loaders and per-space
     *         handlers are optional. */
typedef struct {

    // ---- Required ----

        /** @brief Build Datagram Received OK with reply-pending time.  REQUIRED. */
    void (*load_datagram_received_ok_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);

        /** @brief Build Datagram Rejected with error code.  REQUIRED. */
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

        /** @brief Read bytes from config memory into buffer.  Returns bytes read.  REQUIRED. */
    uint16_t(*config_memory_read)(openlcb_node_t* openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

    // ---- Optional SNIP field loaders (needed only for ACDI 0xFC / 0xFB spaces) ----

        /** @brief Load mfg version ID into payload.  Optional (for 0xFC). */
    uint16_t(*snip_load_manufacturer_version_id)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
        /** @brief Load mfg name into payload.  Optional (for 0xFC). */
    uint16_t(*snip_load_name)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
        /** @brief Load model into payload.  Optional (for 0xFC). */
    uint16_t(*snip_load_model)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
        /** @brief Load HW version into payload.  Optional (for 0xFC). */
    uint16_t(*snip_load_hardware_version)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
        /** @brief Load SW version into payload.  Optional (for 0xFC). */
    uint16_t(*snip_load_software_version)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
        /** @brief Load user version ID into payload.  Optional (for 0xFB). */
    uint16_t(*snip_load_user_version_id)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
        /** @brief Load user name into payload.  Optional (for 0xFB). */
    uint16_t(*snip_load_user_name)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
        /** @brief Load user description into payload.  Optional (for 0xFB). */
    uint16_t(*snip_load_user_description)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

    // ---- Optional per-space read request overrides ----

        /** @brief Custom CDI (0xFF) read handler.  Optional. */
    void (*read_request_config_definition_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
        /** @brief Custom All (0xFE) read handler.  Optional. */
    void (*read_request_all)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
        /** @brief Custom Config (0xFD) read handler.  Optional. */
    void (*read_request_config_mem)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
        /** @brief Custom ACDI-Mfg (0xFC) read handler.  Optional. */
    void (*read_request_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
        /** @brief Custom ACDI-User (0xFB) read handler.  Optional. */
    void (*read_request_acdi_user)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
        /** @brief Custom Train FDI (0xFA) read handler.  Optional. */
    void (*read_request_train_function_config_definition_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
        /** @brief Custom Train Fn Config (0xF9) read handler.  Optional. */
    void (*read_request_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

        /** @brief Override reply delay (return N means 2^N seconds).  Optional (default 0). */
    uint16_t (*delayed_reply_time)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

} interface_protocol_config_mem_read_handler_t;


#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup.
         *
         * @param interface_protocol_config_mem_read_handler  Pointer to @ref interface_protocol_config_mem_read_handler_t (must remain valid for application lifetime).
         */
    extern void ProtocolConfigMemReadHandler_initialize(const interface_protocol_config_mem_read_handler_t *interface_protocol_config_mem_read_handler);

    // ---- Incoming read commands (server side — this node is being read) ----

        /**
         * @brief Read from CDI space (0xFF).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemReadHandler_read_space_config_description_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Read from All space (0xFE).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemReadHandler_read_space_all(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Read from Config space (0xFD).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemReadHandler_read_space_config_memory(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Read from ACDI-Mfg space (0xFC).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemReadHandler_read_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Read from ACDI-User space (0xFB).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemReadHandler_read_space_acdi_user(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Read from Train FDI space (0xFA).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemReadHandler_read_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Read from Train Fn Config space (0xF9).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemReadHandler_read_space_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info);

    // ---- Outgoing read requests (client side — reading from another node) ----

        /**
         * @brief Send read request targeting CDI (0xFF) on another node.
         *
         * @param statemachine_info           Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_read_request_info Pointer to @ref config_mem_read_request_info_t request.
         */
    extern void ProtocolConfigMemReadHandler_read_request_config_definition_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Send read request targeting Config (0xFD) on another node.
         *
         * @param statemachine_info           Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_read_request_info Pointer to @ref config_mem_read_request_info_t request.
         */
    extern void ProtocolConfigMemReadHandler_read_request_config_mem(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Send read request targeting ACDI-Mfg (0xFC) on another node.
         *
         * @param statemachine_info           Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_read_request_info Pointer to @ref config_mem_read_request_info_t request.
         */
    extern void ProtocolConfigMemReadHandler_read_request_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Send read request targeting ACDI-User (0xFB) on another node.
         *
         * @param statemachine_info           Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_read_request_info Pointer to @ref config_mem_read_request_info_t request.
         */
    extern void ProtocolConfigMemReadHandler_read_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Send read request targeting Train FDI (0xFA) on another node.
         *
         * @param statemachine_info           Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_read_request_info Pointer to @ref config_mem_read_request_info_t request.
         */
    extern void ProtocolConfigMemReadHandler_read_request_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Send read request targeting Train Fn Config (0xF9) on another node.
         *
         * @param statemachine_info           Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_read_request_info Pointer to @ref config_mem_read_request_info_t request.
         */
    extern void ProtocolConfigMemReadHandler_read_request_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

    // ---- Generic message handlers (stubs for future use) ----

        /**
         * @brief Generic read message handler.  Placeholder.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param space              Address space number.
         * @param return_msg_ok      MTI for success reply.
         * @param return_msg_fail    MTI for failure reply.
         */
    extern void ProtocolConfigMemReadHandler_read_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

        /**
         * @brief Generic read reply OK handler.  Placeholder.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param space              Address space number.
         */
    extern void ProtocolConfigMemReadHandler_read_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);

        /**
         * @brief Generic read reply reject handler.  Placeholder.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param space              Address space number.
         */
    extern void ProtocolConfigMemReadHandler_read_reply_reject_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_CONFIG_MEM_READ_HANDLER__ */
