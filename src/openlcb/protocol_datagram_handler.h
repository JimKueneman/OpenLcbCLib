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
 * @file protocol_datagram_handler.h
 * @brief Datagram protocol handler — reliable 0–72 byte addressed transfers.
 *        Routes incoming datagrams to per-address-space callbacks for read,
 *        write, write-under-mask, and stream variants.  NULL optional callbacks
 *        cause automatic rejection with SUBCOMMAND_UNKNOWN.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__
#define __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /** @brief Callback interface for the datagram handler.  lock/unlock are REQUIRED;
     *         all memory-operation pointers are optional (NULL = rejected). */
typedef struct {

    // =========================================================================
    // Resource Locking (REQUIRED)
    // =========================================================================

        /** @brief Disable interrupts / acquire mutex.  REQUIRED. */
    void (*lock_shared_resources)(void);

        /** @brief Re-enable interrupts / release mutex.  REQUIRED. */
    void (*unlock_shared_resources)(void);

    // =========================================================================
    // Datagram-transport READ handlers  (server side — incoming requests)
    //   Address spaces: CDI 0xFF, All 0xFD, Config 0xFE, ACDI-Mfg 0xFC,
    //   ACDI-User 0xFB, Train FDI 0xFA, Train Fn Config 0xF9
    // =========================================================================

        /** @brief Read CDI (0xFF) via datagram.  Optional. */
    void (*memory_read_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read All (0xFD) via datagram.  Optional. */
    void (*memory_read_space_all)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read Config (0xFE) via datagram.  Optional. */
    void (*memory_read_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read ACDI-Mfg (0xFC) via datagram.  Optional. */
    void (*memory_read_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read ACDI-User (0xFB) via datagram.  Optional. */
    void (*memory_read_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read Train FDI (0xFA) via datagram.  Optional. */
    void (*memory_read_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read Train Fn Config (0xF9) via datagram.  Optional. */
    void (*memory_read_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Datagram-transport READ reply handlers  (client side — OK / fail)
    // =========================================================================

        /** @brief Read reply OK — CDI via datagram.  Optional. */
    void (*memory_read_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — All via datagram.  Optional. */
    void (*memory_read_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — Config via datagram.  Optional. */
    void (*memory_read_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — ACDI-Mfg via datagram.  Optional. */
    void (*memory_read_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — ACDI-User via datagram.  Optional. */
    void (*memory_read_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — Train FDI via datagram.  Optional. */
    void (*memory_read_space_train_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — Train Fn Config via datagram.  Optional. */
    void (*memory_read_space_train_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief Read reply FAIL — CDI via datagram.  Optional. */
    void (*memory_read_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — All via datagram.  Optional. */
    void (*memory_read_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — Config via datagram.  Optional. */
    void (*memory_read_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — ACDI-Mfg via datagram.  Optional. */
    void (*memory_read_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — ACDI-User via datagram.  Optional. */
    void (*memory_read_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — Train FDI via datagram.  Optional. */
    void (*memory_read_space_train_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — Train Fn Config via datagram.  Optional. */
    void (*memory_read_space_train_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Stream-transport READ handlers  (server side — incoming requests)
    // =========================================================================

        /** @brief Read CDI (0xFF) via stream.  Optional. */
    void (*memory_read_stream_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read All (0xFD) via stream.  Optional. */
    void (*memory_read_stream_space_all)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read Config (0xFE) via stream.  Optional. */
    void (*memory_read_stream_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read ACDI-Mfg (0xFC) via stream.  Optional. */
    void (*memory_read_stream_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read ACDI-User (0xFB) via stream.  Optional. */
    void (*memory_read_stream_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read Train FDI (0xFA) via stream.  Optional. */
    void (*memory_read_stream_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read Train Fn Config (0xF9) via stream.  Optional. */
    void (*memory_read_stream_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Stream-transport READ reply handlers  (client side — OK / fail)
    // =========================================================================

        /** @brief Read reply OK — CDI via stream.  Optional. */
    void (*memory_read_stream_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — All via stream.  Optional. */
    void (*memory_read_stream_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — Config via stream.  Optional. */
    void (*memory_read_stream_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — ACDI-Mfg via stream.  Optional. */
    void (*memory_read_stream_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — ACDI-User via stream.  Optional. */
    void (*memory_read_stream_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — Train FDI via stream.  Optional. */
    void (*memory_read_stream_space_train_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply OK — Train Fn Config via stream.  Optional. */
    void (*memory_read_stream_space_train_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief Read reply FAIL — CDI via stream.  Optional. */
    void (*memory_read_stream_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — All via stream.  Optional. */
    void (*memory_read_stream_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — Config via stream.  Optional. */
    void (*memory_read_stream_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — ACDI-Mfg via stream.  Optional. */
    void (*memory_read_stream_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — ACDI-User via stream.  Optional. */
    void (*memory_read_stream_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — Train FDI via stream.  Optional. */
    void (*memory_read_stream_space_train_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Read reply FAIL — Train Fn Config via stream.  Optional. */
    void (*memory_read_stream_space_train_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Datagram-transport WRITE handlers  (server side — incoming requests)
    // =========================================================================

        /** @brief Write CDI (0xFF) via datagram.  Optional (usually NULL — read-only). */
    void (*memory_write_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write All (0xFD) via datagram.  Optional (usually NULL — read-only). */
    void (*memory_write_space_all)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write Config (0xFE) via datagram.  Optional. */
    void (*memory_write_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write ACDI-Mfg (0xFC) via datagram.  Optional (usually NULL — read-only). */
    void (*memory_write_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write ACDI-User (0xFB) via datagram.  Optional. */
    void (*memory_write_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write Train FDI (0xFA) via datagram.  Optional (usually NULL — read-only). */
    void (*memory_write_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write Train Fn Config (0xF9) via datagram.  Optional. */
    void (*memory_write_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write Firmware Upgrade space via datagram.  Optional. */
    void (*memory_write_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Datagram-transport WRITE reply handlers  (client side — OK / fail)
    // =========================================================================

        /** @brief Write reply OK — CDI via datagram.  Optional. */
    void (*memory_write_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — All via datagram.  Optional. */
    void (*memory_write_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — Config via datagram.  Optional. */
    void (*memory_write_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — ACDI-Mfg via datagram.  Optional. */
    void (*memory_write_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — ACDI-User via datagram.  Optional. */
    void (*memory_write_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — Train FDI via datagram.  Optional. */
    void (*memory_write_space_train_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — Train Fn Config via datagram.  Optional. */
    void (*memory_write_space_train_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief Write reply FAIL — CDI via datagram.  Optional. */
    void (*memory_write_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — All via datagram.  Optional. */
    void (*memory_write_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — Config via datagram.  Optional. */
    void (*memory_write_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — ACDI-Mfg via datagram.  Optional. */
    void (*memory_write_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — ACDI-User via datagram.  Optional. */
    void (*memory_write_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — Train FDI via datagram.  Optional. */
    void (*memory_write_space_train_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — Train Fn Config via datagram.  Optional. */
    void (*memory_write_space_train_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Datagram-transport WRITE-UNDER-MASK handlers  (server side)
    // =========================================================================

        /** @brief Write-under-mask CDI (0xFF).  Optional (usually NULL — read-only). */
    void (*memory_write_under_mask_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write-under-mask All (0xFD).  Optional (usually NULL — read-only). */
    void (*memory_write_under_mask_space_all)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write-under-mask Config (0xFE).  Optional. */
    void (*memory_write_under_mask_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write-under-mask ACDI-Mfg (0xFC).  Optional (usually NULL — read-only). */
    void (*memory_write_under_mask_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write-under-mask ACDI-User (0xFB).  Optional. */
    void (*memory_write_under_mask_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write-under-mask Train FDI (0xFA).  Optional (usually NULL — read-only). */
    void (*memory_write_under_mask_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write-under-mask Train Fn Config (0xF9).  Optional. */
    void (*memory_write_under_mask_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write-under-mask Firmware Upgrade space.  Optional. */
    void (*memory_write_under_mask_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Stream-transport WRITE handlers  (server side — incoming requests)
    // =========================================================================

        /** @brief Write CDI (0xFF) via stream.  Optional (usually NULL — read-only). */
    void (*memory_write_stream_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write All (0xFD) via stream.  Optional (usually NULL — read-only). */
    void (*memory_write_stream_space_all)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write Config (0xFE) via stream.  Optional. */
    void (*memory_write_stream_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write ACDI-Mfg (0xFC) via stream.  Optional (usually NULL — read-only). */
    void (*memory_write_stream_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write ACDI-User (0xFB) via stream.  Optional. */
    void (*memory_write_stream_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write Train FDI (0xFA) via stream.  Optional (usually NULL — read-only). */
    void (*memory_write_stream_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write Train Fn Config (0xF9) via stream.  Optional. */
    void (*memory_write_stream_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write Firmware Upgrade space via stream.  Optional. */
    void (*memory_write_stream_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Stream-transport WRITE reply handlers  (client side — OK / fail)
    // =========================================================================

        /** @brief Write reply OK — CDI via stream.  Optional. */
    void (*memory_write_stream_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — All via stream.  Optional. */
    void (*memory_write_stream_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — Config via stream.  Optional. */
    void (*memory_write_stream_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — ACDI-Mfg via stream.  Optional. */
    void (*memory_write_stream_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — ACDI-User via stream.  Optional. */
    void (*memory_write_stream_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — Train FDI via stream.  Optional. */
    void (*memory_write_stream_space_train_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply OK — Train Fn Config via stream.  Optional. */
    void (*memory_write_stream_space_train_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief Write reply FAIL — CDI via stream.  Optional. */
    void (*memory_write_stream_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — All via stream.  Optional. */
    void (*memory_write_stream_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — Config via stream.  Optional. */
    void (*memory_write_stream_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — ACDI-Mfg via stream.  Optional. */
    void (*memory_write_stream_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — ACDI-User via stream.  Optional. */
    void (*memory_write_stream_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — Train FDI via stream.  Optional. */
    void (*memory_write_stream_space_train_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Write reply FAIL — Train Fn Config via stream.  Optional. */
    void (*memory_write_stream_space_train_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Configuration Memory Commands  (all optional)
    // =========================================================================

        /** @brief Get Configuration Options command.  Optional. */
    void (*memory_options_cmd)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Configuration Options reply (received).  Optional. */
    void (*memory_options_reply)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Get Address Space Information command.  Optional. */
    void (*memory_get_address_space_info)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Address Space Not Present reply (received).  Optional. */
    void (*memory_get_address_space_info_reply_not_present)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Address Space Present reply (received).  Optional. */
    void (*memory_get_address_space_info_reply_present)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Lock/Reserve command.  Optional. */
    void (*memory_reserve_lock)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Lock/Reserve reply (received).  Optional. */
    void (*memory_reserve_lock_reply)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Get Unique ID command.  Optional. */
    void (*memory_get_unique_id)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Get Unique ID reply (received).  Optional. */
    void (*memory_get_unique_id_reply)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Unfreeze command.  Optional. */
    void (*memory_unfreeze)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Freeze command.  Optional. */
    void (*memory_freeze)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Update Complete notification.  Optional. */
    void (*memory_update_complete)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Reset/Reboot command.  Optional. */
    void (*memory_reset_reboot)(openlcb_statemachine_info_t *statemachine_info);
        /** @brief Factory Reset command.  Optional. */
    void (*memory_factory_reset)(openlcb_statemachine_info_t *statemachine_info);

} interface_protocol_datagram_handler_t;


    /** @brief Function pointer type shared by all per-address-space memory handlers. */
typedef void (*memory_handler_t)(openlcb_statemachine_info_t *statemachine_info);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup before any
         *        datagram processing.
         *
         * @param interface_protocol_datagram_handler  Pointer to @ref interface_protocol_datagram_handler_t (must remain valid for application lifetime).
         */
    extern void ProtocolDatagramHandler_initialize(const interface_protocol_datagram_handler_t *interface_protocol_datagram_handler);

        /**
         * @brief Builds a Datagram Received OK message (MTI 0x0A28).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param reply_pending_time_in_seconds  0 for simple OK, or Reply Pending + timeout flags.
         */
    extern void ProtocolDatagramHandler_load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);

        /**
         * @brief Builds a Datagram Rejected message (MTI 0x0A48).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param return_code        OpenLCB error code (0x1xxx permanent, 0x2xxx temporary).
         */
    extern void ProtocolDatagramHandler_load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

        /**
         * @brief Processes an incoming datagram.  Dispatches to the appropriate handler
         *        based on the command byte (payload[0]).
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context with the received datagram.
         */
    extern void ProtocolDatagramHandler_datagram(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles an incoming Datagram Received OK reply.  Clears the resend
         *        flag and frees the stored datagram awaiting acknowledgement.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context with the received reply.
         */
    extern void ProtocolDatagramHandler_datagram_received_ok(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles an incoming Datagram Rejected reply.  Sets the resend flag
         *        for temporary errors; clears retry state for permanent errors.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context with the received rejection.
         */
    extern void ProtocolDatagramHandler_datagram_rejected(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Frees any stored datagram and clears the resend flag for the node.
         *
         * @param openlcb_node  Pointer to @ref openlcb_node_t target node.
         */
    extern void ProtocolDatagramHandler_clear_resend_datagram_message(openlcb_node_t *openlcb_node);

        /** @brief 100 ms timer tick for datagram timeout management (placeholder). */
    extern void ProtocolDatagramHandler_100ms_timer_tick(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__ */
