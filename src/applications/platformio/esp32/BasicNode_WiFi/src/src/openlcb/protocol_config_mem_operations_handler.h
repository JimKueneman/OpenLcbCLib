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
     * @file protocol_config_mem_operations_handler.h
     * @brief Configuration memory operations dispatcher — routes memory-config
     *        datagram sub-commands (options, address-space info, lock, freeze,
     *        reset, factory-reset) to registered callbacks.
     *
     * @author Jim Kueneman
     * @date 28 Feb 2026
     *
     * @see MemoryConfigurationS.pdf
     */

#ifndef __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__
#define    __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Callback interface for memory-config operations.
     *
     * @details Required callbacks: load_datagram_received_ok_message,
     *          load_datagram_received_rejected_message.
     *          All others are optional (NULL disables the sub-command).
     */
typedef struct {

        /** @brief REQUIRED — Send Datagram Received OK with reply-pending time. */
    void (*load_datagram_received_ok_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);

        /** @brief REQUIRED — Send Datagram Received Rejected with error code. */
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code);

        /** @brief Optional — Handle Get Configuration Options command. */
    void (*operations_request_options_cmd)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Get Configuration Options reply. */
    void (*operations_request_options_cmd_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Get Address Space Info command. */
    void (*operations_request_get_address_space_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Address Space Info Present reply. */
    void (*operations_request_get_address_space_info_reply_present)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Address Space Info Not Present reply. */
    void (*operations_request_get_address_space_info_reply_not_present)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Lock/Reserve command. */
    void (*operations_request_reserve_lock)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Lock/Reserve reply. */
    void (*operations_request_reserve_lock_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Get Unique ID command. */
    void (*operations_request_get_unique_id)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Get Unique ID reply. */
    void (*operations_request_get_unique_id_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Freeze command. */
    void (*operations_request_freeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Unfreeze command. */
    void (*operations_request_unfreeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Update Complete command. */
    void (*operations_request_update_complete)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Reset/Reboot command. */
    void (*operations_request_reset_reboot)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /** @brief Optional — Handle Factory Reset command (destructive). */
    void (*operations_request_factory_reset)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);


} interface_protocol_config_mem_operations_handler_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup.
         *
         * @param interface_protocol_config_mem_operations_handler  Pointer to @ref interface_protocol_config_mem_operations_handler_t (must remain valid for application lifetime).
         */
    extern void ProtocolConfigMemOperationsHandler_initialize(const interface_protocol_config_mem_operations_handler_t *interface_protocol_config_mem_operations_handler);

    // ---- Incoming command/reply handlers (dispatched from datagram handler) ----

        /**
         * @brief Handle incoming Get Configuration Options command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_options_cmd(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Get Configuration Options reply.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_options_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Get Address Space Info command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_get_address_space_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Address Space Info Not Present reply.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Address Space Info Present reply.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Lock/Reserve command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_reserve_lock(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Lock/Reserve reply.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_reserve_lock_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Get Unique ID command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_get_unique_id(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Get Unique ID reply.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_get_unique_id_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Unfreeze command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_unfreeze(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Freeze command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_freeze(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Update Complete command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_update_complete(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Reset/Reboot command.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_reset_reboot(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handle incoming Factory Reset command (destructive).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolConfigMemOperationsHandler_factory_reset(openlcb_statemachine_info_t *statemachine_info);

    // ---- Outgoing request builders (used when acting as a config tool) ----

        /**
         * @brief Build outgoing Get Configuration Options command datagram.
         *
         * @param statemachine_info                   Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_operations_request_info   Pointer to @ref config_mem_operations_request_info_t request.
         */
    extern void ProtocolConfigMemOperationsHandler_request_options_cmd(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Build outgoing Get Address Space Info command datagram.
         *
         * @param statemachine_info                   Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_operations_request_info   Pointer to @ref config_mem_operations_request_info_t request.
         */
    extern void ProtocolConfigMemOperationsHandler_request_get_address_space_info(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Build outgoing Lock/Reserve command datagram.
         *
         * @param statemachine_info                   Pointer to @ref openlcb_statemachine_info_t context.
         * @param config_mem_operations_request_info   Pointer to @ref config_mem_operations_request_info_t request.
         */
    extern void ProtocolConfigMemOperationsHandler_request_reserve_lock(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__ */

