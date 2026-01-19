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
 * @brief Implementation of message handlers for CAN receive operations
 * @author Jim Kueneman
 * @date 17 Jan 2026
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

    /**
     * @brief Initializes the CAN Receive Message Handler module
     *
     * @details Algorithm:
     * -# Store pointer to dependency injection interface in static variable
     * -# Interface remains valid for lifetime of application
     *
     * Use cases:
     * - Application startup sequence
     * - System initialization before CAN reception begins
     *
     * @verbatim
     * @param interface_can_frame_message_handler Pointer to interface structure containing required function pointers
     * @endverbatim
     *
     * @warning Must be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call before any CAN frames are processed
     *
     * @see interface_can_rx_message_handler_t - Interface structure definition
     */
void CanRxMessageHandler_initialize(const interface_can_rx_message_handler_t *interface_can_frame_message_handler) {

    _interface = (interface_can_rx_message_handler_t*) interface_can_frame_message_handler;

}

    /**
     * @brief Builds a datagram rejected reply message for out-of-order frames
     *
     * @details Algorithm:
     * -# Allocate CAN message buffer from store
     * -# Return early if allocation fails (silently drop)
     * -# Build OpenLCB Datagram Rejected message:
     *    - Set source alias from dest_alias parameter
     *    - Set destination alias from source_alias parameter
     *    - Set MTI to MTI_DATAGRAM_REJECTED
     * -# Pack error code into payload bytes [2-3]
     * -# Push message to CAN TX FIFO
     * -# Free buffer after transmission
     *
     * Use cases:
     * - Multi-frame message received out of sequence
     * - Middle or last frame received with no matching first frame
     * - Protocol violation detected during message assembly
     *
     * @verbatim
     * @param source_alias Alias of node that sent the problematic frame
     * @param dest_alias Alias of our node receiving the frame
     * @param mti Original MTI of the message being rejected
     * @param error_code OpenLCB error code indicating rejection reason
     * @endverbatim
     *
     * @warning Silently drops message if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention Error codes should be from OpenLCB standard (0x2040 for out-of-order)
     *
     * @see CanBufferStore_allocate_buffer - Buffer allocation
     * @see CanUtilities_load_openlcb_datagram_rejected_can_message - Message builder
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
     * @brief Checks if received frame indicates a duplicate alias condition
     *
     * @details Algorithm:
     * -# Extract source alias from CAN message identifier
     * -# Look up alias in our alias mapping table
     * -# If alias not found: return false (no duplicate)
     * -# If alias found: we have a duplicate condition
     * -# Set duplicate alias flag in mapping info
     * -# Allocate CAN buffer for RID response
     * -# If allocation succeeds:
     *    - Build RID frame with our alias
     *    - Send to indicate we already have this alias
     *    - Free buffer
     * -# Return true (duplicate detected)
     *
     * Use cases:
     * - CID frame received during another node's login
     * - RID frame received claiming our alias
     * - AMD frame received with our alias
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame being checked
     * @endverbatim
     *
     * @return true if duplicate alias detected, false if no conflict
     *
     * @warning Silently drops RID response if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention Main state machine must check duplicate flag and handle conflict
     * @attention RID response helps other node detect conflict quickly
     *
     * @see AliasMappings_find_mapping_by_alias - Alias lookup
     * @see AliasMappings_set_has_duplicate_alias_flag - Sets global flag
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

    /**
     * @brief Handles a complete single-frame OpenLCB message
     *
     * @details Algorithm:
     * -# Extract source and destination aliases
     * -# Convert CAN MTI to OpenLCB MTI
     * -# Allocate OpenLCB message buffer
     * -# If allocation fails: silently drop
     * -# Initialize message structure
     * -# Copy all payload data
     * -# Push directly to OpenLCB FIFO
     *
     * Use cases:
     * - Short addressed messages
     * - Event reports
     * - Protocol queries
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame (complete message)
     * @param offset Byte offset where OpenLCB data begins
     * @param data_type Type of buffer to allocate (typically BASIC)
     * @endverbatim
     *
     * @warning Silently drops if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention Most common message type on OpenLCB networks
     *
     * @see OpenLcbBufferFifo_push - Queues for protocol processing
     */
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
     * @brief Handles legacy SNIP messages without standard framing bits
     *
     * @details Algorithm:
     * -# Search for in-progress SNIP message
     * -# If not found: allocate and initialize
     * -# Append payload data
     * -# Count NULL bytes
     * -# If 6 NULLs found: message complete, push to FIFO
     *
     * Use cases:
     * - Supporting early SNIP implementations
     * - Backward compatibility
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame containing SNIP data
     * @param offset Byte offset where SNIP data begins
     * @param data_type Buffer type (must be SNIP)
     * @endverbatim
     *
     * @warning Only for SNIP messages with 6 NULL terminators
     * @warning NOT thread-safe
     *
     * @attention Legacy protocol - modern nodes use standard framing
     *
     * @see ProtocolSnip_validate_snip_reply - Validates format
     */
void CanRxMessageHandler_can_legacy_snip(can_msg_t* can_msg, uint8_t offset, payload_type_enum data_type) {

    // Early implementations did not have the multi-frame bits to use... special case

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* openlcb_msg_inprocess = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!openlcb_msg_inprocess) { // Do we have one in process?

    /**
     * @brief Handles the first frame of a multi-frame OpenLCB message
     *
     * @details Algorithm:
     * -# Extract source and destination aliases from CAN message
     * -# Convert CAN MTI to OpenLCB MTI
     * -# Check if message already in progress (duplicate first frame)
     * -# If found: send rejection for out-of-order sequence, return
     * -# Allocate OpenLCB message buffer of specified data_type
     * -# If allocation fails: send rejection, return
     * -# Initialize message structure with source/dest/MTI
     * -# Copy payload data from CAN frame starting at offset
     * -# Add message to buffer list for continued assembly
     *
     * Use cases:
     * - Starting datagram reception
     * - Starting SNIP message reception
     * - Starting any multi-frame addressed message
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame (first frame)
     * @param offset Byte offset where OpenLCB data begins (2 if addressed, 0 if global)
     * @param data_type Type of buffer to allocate (BASIC, DATAGRAM, SNIP, STREAM)
     * @endverbatim
     *
     * @warning Sends rejection if buffer allocation fails
     * @warning Sends rejection if duplicate first frame detected
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_FIRST
     *
     * @see CanRxMessageHandler_middle_frame - Processes continuation frames
     * @see CanRxMessageHandler_last_frame - Completes message assembly
     */
        CanRxMessageHandler_first_frame(can_msg, offset, data_type);

    } else { // Yes we have one in process

        if (CanUtilities_count_nulls_in_payloads(openlcb_msg_inprocess, can_msg) < 6) {

    /**
     * @brief Handles a middle frame of a multi-frame OpenLCB message
     *
     * @details Algorithm:
     * -# Extract source and destination aliases from CAN message
     * -# Convert CAN MTI to OpenLCB MTI
     * -# Search buffer list for in-progress message
     * -# If not found: send rejection, return
     * -# Append payload data to message buffer
     * -# Update payload count
     *
     * Use cases:
     * - Continuing datagram reception
     * - Continuing SNIP message reception
     * - Processing frames between first and last
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame (middle frame)
     * @param offset Byte offset where OpenLCB data begins
     * @endverbatim
     *
     * @warning Sends rejection if no matching message in progress
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_MIDDLE
     *
     * @see CanRxMessageHandler_first_frame - Started message assembly
     */
            CanRxMessageHandler_middle_frame(can_msg, offset);

        } else {

    /**
     * @brief Handles the last frame of a multi-frame OpenLCB message
     *
     * @details Algorithm:
     * -# Extract source and destination aliases
     * -# Search buffer list for in-progress message
     * -# If not found: send rejection, return
     * -# Append final payload data
     * -# Mark message complete
     * -# Remove from buffer list
     * -# Push to OpenLCB FIFO
     *
     * Use cases:
     * - Completing datagram reception
     * - Completing SNIP message reception
     * - Finalizing any multi-frame message
     *
     * @verbatim
     * @param can_msg Pointer to received CAN frame (last frame)
     * @param offset Byte offset where OpenLCB data begins
     * @endverbatim
     *
     * @warning Sends rejection if no matching message in progress
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_FINAL
     *
     * @see OpenLcbBufferFifo_push - Queues for protocol processing
     */
            CanRxMessageHandler_last_frame(can_msg, offset);

        }

    };

}

    /**
     * @brief Placeholder for stream protocol frame handling
     *
     * @details Algorithm:
     * -# Currently no implementation
     * -# Reserved for future stream protocol
     *
     * Use cases:
     * - Firmware upgrade (future)
     * - Data streaming (future)
     *
     * @verbatim
     * @param can_msg Pointer to received stream frame
     * @param offset Byte offset where stream data begins
     * @param data_type Buffer type (must be STREAM)
     * @endverbatim
     *
     * @warning Currently unimplemented
     * @warning NOT thread-safe
     *
     * @attention Reserved for future use
     */
void CanRxMessageHandler_stream_frame(can_msg_t* can_msg, uint8_t offset, payload_type_enum data_type) {


}

    /**
     * @brief Handles CID (Check ID) frames during alias allocation
     *
     * @details Algorithm:
     * -# Extract alias being checked
     * -# Look up in our mapping table
     * -# If we have it: send RID response
     *
     * Use cases:
     * - Responding to other nodes' login
     * - Preventing duplicate aliases
     *
     * @verbatim
     * @param can_msg Pointer to received CID frame
     * @endverbatim
     *
     * @warning Silently drops RID if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention Part of alias allocation protocol
     *
     * @see CanRxMessageHandler_rid_frame - Handles RID reception
     */
void CanRxMessageHandler_cid_frame(can_msg_t* can_msg) {

    if (!can_msg) { return; }

        // Check for duplicate Alias
        uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
        alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(source_alias);

        if (alias_mapping)
        {

            can_msg_t *reply_msg = _interface->can_buffer_store_allocate_buffer();

            if (reply_msg)
            {
                reply_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | source_alias;
                reply_msg->payload_count = 0;

                CanBufferFifo_push(reply_msg);
            }
        }

}

    /**
     * @brief Handles RID (Reserve ID) frames
     *
     * @details Algorithm:
     * -# Check for duplicate alias
     * -# Sets flag if conflict detected
     *
     * Use cases:
     * - Alias conflict detection
     * - Network monitoring
     *
     * @verbatim
     * @param can_msg Pointer to received RID frame
     * @endverbatim
     *
     * @warning Sets duplicate flag if conflict
     * @warning NOT thread-safe
     *
     * @see _check_for_duplicate_alias - Conflict detection
     */
void CanRxMessageHandler_rid_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

    /**
     * @brief Handles AMD (Alias Map Definition) frames
     *
     * @details Algorithm:
     * -# Check for duplicate alias
     * -# AMD contains full NodeID
     *
     * Use cases:
     * - Learning alias/NodeID mappings
     * - Duplicate detection
     *
     * @verbatim
     * @param can_msg Pointer to received AMD frame with NodeID
     * @endverbatim
     *
     * @warning Sets duplicate flag if conflict
     * @warning NOT thread-safe
     *
     * @see AliasMappings_register - Stores mapping
     */
void CanRxMessageHandler_amd_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

    /**
     * @brief Handles AME (Alias Map Enquiry) frames
     *
     * @details Algorithm:
     * -# Check for duplicate (return if found)
     * -# If global: respond with all aliases
     * -# If specific: respond if match
     *
     * Use cases:
     * - Gateway alias table building
     * - Network discovery
     *
     * @verbatim
     * @param can_msg Pointer to received AME frame
     * @endverbatim
     *
     * @warning Silently drops if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @see AliasMappings_get_alias_mapping_info - Access mappings
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

    /**
     * @brief Handles AMR (Alias Map Reset) frames
     *
     * @details Algorithm:
     * -# Check for duplicate alias
     *
     * Use cases:
     * - Alias conflict resolution
     * - Forced deallocation
     *
     * @verbatim
     * @param can_msg Pointer to received AMR frame
     * @endverbatim
     *
     * @warning Sets duplicate flag if our alias
     * @warning NOT thread-safe
     *
     * @see CanMainStatemachine_handle_duplicate_aliases - Resolution
     */
void CanRxMessageHandler_amr_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

    /**
     * @brief Handles Error Information Report frames
     *
     * @details Algorithm:
     * -# Check for duplicate alias
     *
     * Use cases:
     * - Error notifications
     * - Network diagnostics
     *
     * @verbatim
     * @param can_msg Pointer to received error report frame
     * @endverbatim
     *
     * @warning Sets duplicate flag if relevant
     * @warning NOT thread-safe
     *
     * @see _check_for_duplicate_alias - Checks relevance
     */
void CanRxMessageHandler_error_info_report_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

