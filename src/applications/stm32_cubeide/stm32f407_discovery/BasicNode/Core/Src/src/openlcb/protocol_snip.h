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
 * @file protocol_snip.h
 * @brief Simple Node Information Protocol (SNIP) — returns manufacturer info
 *        (from node params) and user name/description (from config memory) in
 *        a single reply message.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __OPENLCB_PROTOCOL_SNIP__
#define    __OPENLCB_PROTOCOL_SNIP__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /** @brief Callback interface for SNIP.  config_memory_read is REQUIRED. */
typedef struct {

        /** @brief Read from config memory (ACDI User space) for user name/description.  REQUIRED. */
   uint16_t(*config_memory_read)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

} interface_openlcb_protocol_snip_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup.
         *
         * @param interface_openlcb_protocol_snip  Pointer to @ref interface_openlcb_protocol_snip_t (must remain valid for application lifetime).
         */
    extern void ProtocolSnip_initialize(const interface_openlcb_protocol_snip_t *interface_openlcb_protocol_snip);

        /**
         * @brief Builds a SNIP reply (MTI 0x0A08) from node params + config memory.
         *
         * @details Assembles eight fields: mfg version, name, model, HW ver, SW ver,
         *          user version, user name, user description.  Max 253 bytes.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolSnip_handle_simple_node_info_request(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Handles an incoming SNIP reply (MTI 0x0A08).  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolSnip_handle_simple_node_info_reply(openlcb_statemachine_info_t *statemachine_info);

    // ---- Individual SNIP field loaders (used internally to build the reply) ----
    //
    // All share the same signature:
    //   (node, outgoing_msg, payload_offset, max_bytes) → bytes_written

        /**
         * @brief Copies manufacturer version ID byte (1 byte).
         *
         * @param openlcb_node     Pointer to @ref openlcb_node_t source node.
         * @param outgoing_msg     Pointer to @ref openlcb_msg_t reply being built.
         * @param offset           Payload byte offset to write at.
         * @param requested_bytes  Maximum bytes to copy.
         * @return Bytes actually written.
         */
    extern uint16_t ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);

        /**
         * @brief Copies manufacturer name string (null-terminated).
         *
         * @param openlcb_node     Pointer to @ref openlcb_node_t source node.
         * @param outgoing_msg     Pointer to @ref openlcb_msg_t reply being built.
         * @param offset           Payload byte offset to write at.
         * @param requested_bytes  Maximum bytes to copy.
         * @return Bytes actually written.
         */
    extern uint16_t ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);

        /**
         * @brief Copies model string (null-terminated).
         *
         * @param openlcb_node     Pointer to @ref openlcb_node_t source node.
         * @param outgoing_msg     Pointer to @ref openlcb_msg_t reply being built.
         * @param offset           Payload byte offset to write at.
         * @param requested_bytes  Maximum bytes to copy.
         * @return Bytes actually written.
         */
    extern uint16_t ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);

        /**
         * @brief Copies hardware version string (null-terminated).
         *
         * @param openlcb_node     Pointer to @ref openlcb_node_t source node.
         * @param outgoing_msg     Pointer to @ref openlcb_msg_t reply being built.
         * @param offset           Payload byte offset to write at.
         * @param requested_bytes  Maximum bytes to copy.
         * @return Bytes actually written.
         */
    extern uint16_t ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);

        /**
         * @brief Copies software version string (null-terminated).
         *
         * @param openlcb_node     Pointer to @ref openlcb_node_t source node.
         * @param outgoing_msg     Pointer to @ref openlcb_msg_t reply being built.
         * @param offset           Payload byte offset to write at.
         * @param requested_bytes  Maximum bytes to copy.
         * @return Bytes actually written.
         */
    extern uint16_t ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);

        /**
         * @brief Copies user version ID byte (1 byte).
         *
         * @param openlcb_node     Pointer to @ref openlcb_node_t source node.
         * @param outgoing_msg     Pointer to @ref openlcb_msg_t reply being built.
         * @param offset           Payload byte offset to write at.
         * @param requested_bytes  Maximum bytes to copy.
         * @return Bytes actually written.
         */
    extern uint16_t ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);

        /**
         * @brief Reads user name from config memory (null-terminated, max 63 bytes).
         *
         * @param openlcb_node     Pointer to @ref openlcb_node_t source node.
         * @param outgoing_msg     Pointer to @ref openlcb_msg_t reply being built.
         * @param offset           Payload byte offset to write at.
         * @param requested_bytes  Maximum bytes to copy.
         * @return Bytes actually written.
         */
    extern uint16_t ProtocolSnip_load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);

        /**
         * @brief Reads user description from config memory (null-terminated, max 64 bytes).
         *
         * @param openlcb_node     Pointer to @ref openlcb_node_t source node.
         * @param outgoing_msg     Pointer to @ref openlcb_msg_t reply being built.
         * @param offset           Payload byte offset to write at.
         * @param requested_bytes  Maximum bytes to copy.
         * @return Bytes actually written.
         */
    extern uint16_t ProtocolSnip_load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);

        /**
         * @brief Validates a SNIP reply: correct MTI, valid length, exactly 6 null terminators.
         *
         * @param snip_reply_msg  Pointer to @ref openlcb_msg_t message to validate.
         * @return true if the message is a well-formed SNIP reply.
         */
    extern bool ProtocolSnip_validate_snip_reply(openlcb_msg_t* snip_reply_msg);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_SNIP__ */
