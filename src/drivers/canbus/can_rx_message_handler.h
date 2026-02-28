/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file can_rx_message_handler.h
 * @brief Message handlers for processing received CAN frames into OpenLCB messages.
 *
 * @details Handles multi-frame assembly, legacy SNIP, and all CAN control frames
 * (CID, RID, AMD, AME, AMR, error reports). Invoked by the CAN Rx state machine.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__
#define __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_node.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Dependency-injection interface for the CAN receive message handlers.
     *
     * @details Provides buffer allocation and alias-mapping callbacks needed to
     * assemble incoming CAN frames into OpenLCB messages and to respond to CAN
     * control frames (CID, AME, etc.).  All pointers are REQUIRED.
     *
     * @see CanRxMessageHandler_initialize
     */
    typedef struct {

        /** @brief REQUIRED. Allocate a CAN buffer (for outgoing control replies). Typical: CanBufferStore_allocate_buffer. */
        can_msg_t *(*can_buffer_store_allocate_buffer)(void);

        /** @brief REQUIRED. Allocate an OpenLCB buffer for message assembly. Typical: OpenLcbBufferStore_allocate_buffer. */
        openlcb_msg_t *(*openlcb_buffer_store_allocate_buffer)(payload_type_enum payload_type);

        /** @brief REQUIRED. Find an @ref alias_mapping_t by 12-bit alias. Typical: AliasMappings_find_mapping_by_alias. */
        alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

        /** @brief REQUIRED. Find an @ref alias_mapping_t by 48-bit @ref node_id_t. Typical: AliasMappings_find_mapping_by_node_id. */
        alias_mapping_t *(*alias_mapping_find_mapping_by_node_id)(node_id_t node_id);

        /** @brief REQUIRED. Return pointer to the full @ref alias_mapping_info_t table. Typical: AliasMappings_get_alias_mapping_info. */
        alias_mapping_info_t *(*alias_mapping_get_alias_mapping_info)(void);

        /** @brief REQUIRED. Set the global duplicate-alias flag. Typical: AliasMappings_set_has_duplicate_alias_flag. */
        void (*alias_mapping_set_has_duplicate_alias_flag)(void);

    } interface_can_rx_message_handler_t;


    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface_can_frame_message_handler Pointer to a populated
     *        @ref interface_can_rx_message_handler_t. Must remain valid for the
     *        lifetime of the application. All function pointers must be non-NULL.
     *
     * @warning NOT thread-safe - call during single-threaded initialization only.
     * @warning Must be called before any CAN frames are processed.
     *
     * @see CanRxStatemachine_initialize
     */
    extern void CanRxMessageHandler_initialize(const interface_can_rx_message_handler_t *interface_can_frame_message_handler);


    /**
     * @brief Handles the first frame of a multi-frame OpenLCB message.
     *
     * @details Allocates an OpenLCB buffer, initializes it with source/dest/MTI from
     * the CAN header, copies the first payload chunk, and adds it to the buffer list
     * for continued assembly.  Sends a rejection if a duplicate first frame is detected
     * or if buffer allocation fails.
     *
     * @param can_msg    Received CAN frame.
     * @param offset     Byte offset where OpenLCB data begins (2 if addressed, 0 if global).
     * @param data_type  Buffer type to allocate (BASIC, DATAGRAM, SNIP, STREAM).
     *
     * @warning NOT thread-safe.
     *
     * @see CanRxMessageHandler_middle_frame
     * @see CanRxMessageHandler_last_frame
     */
    extern void CanRxMessageHandler_first_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles a middle frame of a multi-frame OpenLCB message.
     *
     * @details Finds the in-progress message by source/dest/MTI and appends payload.
     * Sends a rejection if no matching message is found.
     *
     * @param can_msg  Received CAN frame.
     * @param offset   Byte offset where OpenLCB data begins.
     *
     * @warning NOT thread-safe.
     *
     * @see CanRxMessageHandler_first_frame
     * @see CanRxMessageHandler_last_frame
     */
    extern void CanRxMessageHandler_middle_frame(can_msg_t *can_msg, uint8_t offset);


    /**
     * @brief Handles the last frame of a multi-frame OpenLCB message.
     *
     * @details Appends final payload, marks message complete, removes from buffer list,
     * and pushes to the OpenLCB FIFO.  Sends a rejection if no matching in-progress
     * message is found.
     *
     * @param can_msg  Received CAN frame.
     * @param offset   Byte offset where OpenLCB data begins.
     *
     * @warning NOT thread-safe.
     *
     * @see CanRxMessageHandler_first_frame
     * @see CanRxMessageHandler_middle_frame
     */
    extern void CanRxMessageHandler_last_frame(can_msg_t *can_msg, uint8_t offset);


    /**
     * @brief Handles a complete single-frame OpenLCB message.
     *
     * @details Allocates a buffer, copies all payload data, and pushes directly to the
     * OpenLCB FIFO.  Silently drops if allocation fails.
     *
     * @param can_msg    Received CAN frame.
     * @param offset     Byte offset where OpenLCB data begins.
     * @param data_type  Buffer type to allocate (typically BASIC).
     *
     * @warning NOT thread-safe.
     *
     * @see OpenLcbBufferFifo_push
     */
    extern void CanRxMessageHandler_single_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles legacy SNIP messages that lack standard framing bits.
     *
     * @details Accumulates frames and counts NULL terminators to detect completion.
     * A complete SNIP message contains exactly 6 NULLs.
     *
     * @param can_msg    Received CAN frame.
     * @param offset     Byte offset where SNIP data begins.
     * @param data_type  Must be SNIP.
     *
     * @warning NOT thread-safe.
     * @warning Only correct for MTI_SIMPLE_NODE_INFO_REPLY without framing bits.
     *
     * @see CanRxMessageHandler_first_frame - modern multi-frame handling
     */
    extern void CanRxMessageHandler_can_legacy_snip(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles stream protocol CAN frames (placeholder, not yet implemented).
     *
     * @param can_msg    Received CAN frame.
     * @param offset     Byte offset where stream data begins.
     * @param data_type  Must be STREAM.
     *
     * @warning Empty body - reserved for future use.
     */
    extern void CanRxMessageHandler_stream_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles RID (Reserve ID) CAN control frames.
     *
     * @details Checks for a duplicate alias condition and flags it if found.
     *
     * @param can_msg  Received RID frame.
     *
     * @warning NOT thread-safe.
     */
    extern void CanRxMessageHandler_rid_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AMD (Alias Map Definition) CAN control frames.
     *
     * @details Checks for a duplicate alias condition and flags it if found.
     *
     * @param can_msg  Received AMD frame (6-byte NodeID in payload).
     *
     * @warning NOT thread-safe.
     */
    extern void CanRxMessageHandler_amd_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AME (Alias Map Enquiry) CAN control frames.
     *
     * @details Responds with AMD frames for all our aliases (global query) or for
     * a specific NodeID (targeted query if 6-byte payload present).
     *
     * @param can_msg  Received AME frame.
     *
     * @warning Silently drops responses if buffer allocation fails.
     * @warning NOT thread-safe.
     */
    extern void CanRxMessageHandler_ame_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AMR (Alias Map Reset) CAN control frames.
     *
     * @details Checks for a duplicate alias condition and flags it if found.
     *
     * @param can_msg  Received AMR frame.
     *
     * @warning NOT thread-safe.
     */
    extern void CanRxMessageHandler_amr_frame(can_msg_t *can_msg);


    /**
     * @brief Handles Error Information Report CAN control frames.
     *
     * @details Checks for a duplicate alias condition and flags it if found.
     *
     * @param can_msg  Received error report frame.
     *
     * @warning NOT thread-safe.
     */
    extern void CanRxMessageHandler_error_info_report_frame(can_msg_t *can_msg);


    /**
     * @brief Handles CID (Check ID) CAN control frames.
     *
     * @details If we already have the claimed alias registered, sends an RID response
     * to signal a conflict to the node performing alias allocation.
     *
     * @param can_msg  Received CID frame.
     *
     * @warning Silently drops RID response if buffer allocation fails.
     * @warning NOT thread-safe.
     *
     * @see CanRxMessageHandler_rid_frame
     */
    extern void CanRxMessageHandler_cid_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__ */
