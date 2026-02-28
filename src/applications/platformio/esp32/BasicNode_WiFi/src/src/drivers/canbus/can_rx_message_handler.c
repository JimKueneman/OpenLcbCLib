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
 * @file can_rx_message_handler.c
 * @brief Implementation of message handlers for CAN receive operations.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "can_rx_message_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_utilities.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"

#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_utilities.h"


static interface_can_rx_message_handler_t *_interface;

    /** @brief Stores the dependency-injection interface pointer. */
void CanRxMessageHandler_initialize(const interface_can_rx_message_handler_t *interface_can_frame_message_handler) {

    _interface = (interface_can_rx_message_handler_t*) interface_can_frame_message_handler;

}

    /**
     * @brief Builds and queues a Datagram Rejected or Optional Interaction Rejected reply.
     *
     * @details Algorithm:
     * -# Allocate an OpenLCB BASIC buffer; silently drop if allocation fails.
     * -# Map MTI: datagram → MTI_DATAGRAM_REJECTED_REPLY, others → MTI_OPTIONAL_INTERACTION_REJECTED.
     * -# Load the reply message with swapped source/dest aliases.
     * -# Write dest_alias and error_code into payload bytes 0-1 and 2-3.
     * -# Push to the OpenLCB Tx FIFO.
     *
     * @verbatim
     * @param source_alias Alias of the node that sent the problematic frame.
     * @param dest_alias   Alias of our node that received the frame.
     * @param mti          Original MTI of the message being rejected.
     * @param error_code   OpenLCB error code (e.g. ERROR_TEMPORARY_OUT_OF_ORDER_*).
     * @endverbatim
     *
     * @warning Silently drops if buffer allocation fails.
     */
static void _load_reject_message(uint16_t source_alias, uint16_t dest_alias, uint16_t mti, uint16_t error_code) {

    openlcb_msg_t * target_openlcb_msg = _interface->openlcb_buffer_store_allocate_buffer(BASIC);

    if (target_openlcb_msg) {

        if (mti == MTI_DATAGRAM) {

            mti = MTI_DATAGRAM_REJECTED_REPLY;


        } else {

            mti = MTI_OPTIONAL_INTERACTION_REJECTED;

        }

        // TODO: Probably Stream is a special case too

        OpenLcbUtilities_load_openlcb_message(
                target_openlcb_msg,
                source_alias,
                0,
                dest_alias,
                0,
                mti);

        OpenLcbUtilities_copy_word_to_openlcb_payload(
                target_openlcb_msg,
                dest_alias,
                0);

        OpenLcbUtilities_copy_word_to_openlcb_payload(
                target_openlcb_msg,
                error_code,
                2);

        OpenLcbBufferFifo_push(target_openlcb_msg);

    }

}

    /**
     * @brief Checks whether the source alias of a received frame duplicates one of our registered aliases.
     *
     * @details Algorithm:
     * -# Extract source alias from can_msg identifier.
     * -# Look up alias in our mapping table; return false if not found.
     * -# Mark the mapping entry as duplicate and set the global duplicate flag.
     * -# If our alias is already permitted, send an AMR frame to signal the conflict.
     * -# Return true.
     *
     * @verbatim
     * @param can_msg Received CAN frame to check.
     * @endverbatim
     *
     * @return true if a duplicate alias was detected, false otherwise.
     *
     * @warning Silently drops the AMR response if buffer allocation fails.
     */
static bool _check_for_duplicate_alias(can_msg_t* can_msg) {

    // Check for duplicate Alias
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(source_alias);

    if (!alias_mapping) {

        return false; // Done nothing to do
    }

    alias_mapping->is_duplicate = true; // flag for the main loop to handle
    _interface->alias_mapping_set_has_duplicate_alias_flag();

    if (alias_mapping->is_permitted) {

        can_msg_t *outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

        if (outgoing_can_msg) {

            outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | source_alias;
            CanUtilities_copy_node_id_to_payload(outgoing_can_msg, alias_mapping->node_id, 0);
            CanBufferFifo_push(outgoing_can_msg);

        }

    }

    return true;

}

void CanRxMessageHandler_first_frame(can_msg_t* can_msg, uint8_t offset, payload_type_enum data_type) {

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    // See if there is a message already started for this.
    openlcb_msg_t* target_openlcb_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (target_openlcb_msg) {

        // If we find a message for this source/dest/mti then it is an error as it is out of order
        _load_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END);

        return;

    }

    // Try to allocate an openlcb message buffer to start accumulating the frames into an openlcb message
    target_openlcb_msg = _interface->openlcb_buffer_store_allocate_buffer(data_type);

    if (!target_openlcb_msg) {

        _load_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

        return;

    }

    OpenLcbUtilities_load_openlcb_message(
            target_openlcb_msg,
            source_alias,
            0,
            dest_alias,
            0,
            mti);

    target_openlcb_msg->state.inprocess = true;

    CanUtilities_append_can_payload_to_openlcb_payload(target_openlcb_msg, can_msg, offset);

    OpenLcbBufferList_add(target_openlcb_msg); // Can not fail List is as large as the number of buffers

}

void CanRxMessageHandler_middle_frame(can_msg_t* can_msg, uint8_t offset) {

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* target_openlcb_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!target_openlcb_msg) {

        _load_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

        return;

    }

    CanUtilities_append_can_payload_to_openlcb_payload(target_openlcb_msg, can_msg, offset);

}

void CanRxMessageHandler_last_frame(can_msg_t* can_msg, uint8_t offset) {

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t * target_openlcb_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!target_openlcb_msg) {

        _load_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

        return;

    }

    CanUtilities_append_can_payload_to_openlcb_payload(target_openlcb_msg, can_msg, offset);

    target_openlcb_msg->state.inprocess = false;

    OpenLcbBufferList_release(target_openlcb_msg);
    OpenLcbBufferFifo_push(target_openlcb_msg);

}

void CanRxMessageHandler_single_frame(can_msg_t* can_msg, uint8_t offset, payload_type_enum data_type) {

    openlcb_msg_t* target_openlcb_msg = _interface->openlcb_buffer_store_allocate_buffer(data_type);

    if (!target_openlcb_msg) {

        return;

    }

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);
    OpenLcbUtilities_load_openlcb_message(
            target_openlcb_msg,
            source_alias,
            0,
            dest_alias,
            0,
            mti);

    CanUtilities_append_can_payload_to_openlcb_payload(
            target_openlcb_msg,
            can_msg,
            offset);

    OpenLcbBufferFifo_push(target_openlcb_msg); // Can not fail List is as large as the number of buffers

}

    /**
     * @brief Handles legacy SNIP messages that lack standard multi-frame framing bits.
     *
     * @details Algorithm:
     * -# Search for an in-progress SNIP message matching source/dest/MTI.
     * -# If none found: delegate to CanRxMessageHandler_first_frame to allocate and start one.
     * -# If found and NULL count < 6: delegate to CanRxMessageHandler_middle_frame to append data.
     * -# If found and NULL count >= 6: delegate to CanRxMessageHandler_last_frame to complete it.
     *
     * @verbatim
     * @param can_msg    Received CAN frame containing SNIP data.
     * @param offset     Byte offset where SNIP data begins.
     * @param data_type  Must be SNIP.
     * @endverbatim
     *
     * @warning Only correct for MTI_SIMPLE_NODE_INFO_REPLY without framing bits.
     */
void CanRxMessageHandler_can_legacy_snip(can_msg_t* can_msg, uint8_t offset, payload_type_enum data_type) {

    // Early implementations did not have the multi-frame bits to use... special case

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* openlcb_msg_inprocess = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!openlcb_msg_inprocess) { // Do we have one in process?

        CanRxMessageHandler_first_frame(can_msg, offset, data_type);

    } else { // Yes we have one in process

        if (CanUtilities_count_nulls_in_payloads(openlcb_msg_inprocess, can_msg) < 6) {

            CanRxMessageHandler_middle_frame(can_msg, offset);

        } else {

            CanRxMessageHandler_last_frame(can_msg, offset);

        }

    };

}

    /** @brief Stream frame handler — reserved for future use, not yet implemented. */
void CanRxMessageHandler_stream_frame(can_msg_t* can_msg, uint8_t offset, payload_type_enum data_type) {


}

    /**
     * @brief Handles CID frames: sends an RID reply if the claimed alias is already ours.
     *
     * @verbatim
     * @param can_msg Received CID frame.
     * @endverbatim
     *
     * @warning Silently drops the RID reply if buffer allocation fails.
     */
void CanRxMessageHandler_cid_frame(can_msg_t* can_msg) {

    if (!can_msg) { return; }

    // Check for duplicate Alias
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(source_alias);

    if (alias_mapping) {

        can_msg_t *reply_msg = _interface->can_buffer_store_allocate_buffer();

        if (reply_msg) {

            reply_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | source_alias;
            reply_msg->payload_count = 0;

            CanBufferFifo_push(reply_msg);

        }

    }

}

    /** @brief Handles RID frames: checks for a duplicate alias and flags it if found. */
void CanRxMessageHandler_rid_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

    /** @brief Handles AMD frames: checks for a duplicate alias and flags it if found. */
void CanRxMessageHandler_amd_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

    /**
     * @brief Handles AME frames: responds with AMD frames for matching aliases.
     *
     * @details Algorithm:
     * -# Check for duplicate alias; return early if detected.
     * -# If payload is non-empty: look up by Node ID, respond with one AMD if found.
     * -# If payload is empty (global query): respond with AMD for every registered alias.
     *
     * @verbatim
     * @param can_msg Received AME frame.
     * @endverbatim
     *
     * @warning Silently drops responses if buffer allocation fails.
     */
void CanRxMessageHandler_ame_frame(can_msg_t* can_msg) {

    if (_check_for_duplicate_alias(can_msg)) {

        return;

    }

    can_msg_t *outgoing_can_msg = NULL;

    if (can_msg->payload_count > 0) {

        alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_node_id(CanUtilities_extract_can_payload_as_node_id(can_msg));

        if (alias_mapping) {

            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

            if (outgoing_can_msg) {

                outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping->alias;
                CanUtilities_copy_node_id_to_payload(outgoing_can_msg, alias_mapping->node_id, 0);
                CanBufferFifo_push(outgoing_can_msg);

            }

            return;

        }

        return;

    }

    alias_mapping_info_t *alias_mapping_info = _interface->alias_mapping_get_alias_mapping_info();

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping_info->list[i].alias != 0x00) {

            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

            if (outgoing_can_msg) {

                outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping_info->list[i].alias;
                CanUtilities_copy_node_id_to_payload(outgoing_can_msg, alias_mapping_info->list[i].node_id, 0);
                CanBufferFifo_push(outgoing_can_msg);

            }

        }

    }

}

    /** @brief Handles AMR frames: checks for a duplicate alias and flags it if found. */
void CanRxMessageHandler_amr_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

    /** @brief Handles Error Information Report frames: checks for a duplicate alias and flags it if found. */
void CanRxMessageHandler_error_info_report_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}
