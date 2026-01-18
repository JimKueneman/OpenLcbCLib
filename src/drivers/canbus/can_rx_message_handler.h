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
 * @brief Message handlers for processing received CAN frames
 *
 * @details This module provides handlers for processing incoming CAN frames and converting
 * them into OpenLCB messages. It handles multi-frame message assembly, legacy SNIP protocol,
 * and CAN control frames (AMD, AME, AMR, RID, CID, etc.).
 *
 * The module implements the CAN Frame Transfer protocol handlers that reassemble fragmented
 * OpenLCB messages from multiple CAN frames. It also handles special cases like legacy SNIP
 * messages that don't use standard framing bits.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
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
     * @brief Dependency injection interface for CAN RX message handler module
     *
     * @details OpenLcbCLib uses dependency injection to enable comprehensive testing
     * and reduce code size. Functions can be mocked in tests to validate all code paths.
     * Setting handlers to NULL strips out unused protocol code, useful for bootloaders
     * and resource-constrained applications.
     *
     * Use cases:
     * - Unit testing with mock implementations
     * - Reducing code size by excluding unused protocols
     * - Platform-specific customization
     *
     * @warning All function pointers marked as Required must be initialized
     *
     * @see CanRxMessageHandler_initialize - Stores this interface
     */
    typedef struct {
        /*@{*/

        // REQUIRED FUNCTIONS

        /**
         * @brief Pointer to function for allocating CAN message buffers
         *
         * @details Provides access to the CAN buffer allocation system. Used when
         * building outgoing CAN control messages (AMD, AMR, RID responses).
         *
         * @warning Required assignment - Defaults to CanBufferStore_allocate_buffer()
         *
         * @see CanBufferStore_allocate_buffer - Standard implementation
         */
        can_msg_t *(*can_buffer_store_allocate_buffer)(void);

        /**
         * @brief Pointer to function for allocating OpenLCB message buffers
         *
         * @details Provides access to the OpenLCB buffer allocation system. Used when
         * assembling multi-frame CAN messages into complete OpenLCB messages.
         *
         * @param payload_type Type of buffer needed (BASIC, DATAGRAM, SNIP, STREAM)
         *
         * @warning Required assignment - Defaults to OpenLcbBufferStore_allocate_buffer()
         *
         * @see OpenLcbBufferStore_allocate_buffer - Standard implementation
         */
        openlcb_msg_t *(*openlcb_buffer_store_allocate_buffer)(payload_type_enum payload_type);

        /**
         * @brief Pointer to function for finding alias mappings by alias
         *
         * @details Looks up the NodeID associated with a CAN alias. Used for alias
         * conflict detection and validation.
         *
         * @param alias 12-bit CAN alias to search for
         *
         * @warning Required assignment - Defaults to AliasMappings_find_mapping_by_alias()
         *
         * @see AliasMappings_find_mapping_by_alias - Standard implementation
         */
        alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

        /**
         * @brief Pointer to function for finding alias mappings by Node ID
         *
         * @details Looks up the alias associated with a 48-bit NodeID. Used when
         * responding to AME (Alias Map Enquiry) frames.
         *
         * @param node_id 48-bit OpenLCB Node ID to search for
         *
         * @warning Required assignment - Defaults to AliasMappings_find_mapping_by_node_id()
         *
         * @see AliasMappings_find_mapping_by_node_id - Standard implementation
         */
        alias_mapping_t *(*alias_mapping_find_mapping_by_node_id)(node_id_t node_id);

        /**
         * @brief Pointer to function for accessing complete alias mapping table
         *
         * @details Returns pointer to the alias mapping information structure containing
         * all registered alias/NodeID pairs. Used when responding to global AME frames.
         *
         * @warning Required assignment - Defaults to AliasMappings_get_alias_mapping_info()
         *
         * @see AliasMappings_get_alias_mapping_info - Standard implementation
         */
        alias_mapping_info_t *(*alias_mapping_get_alias_mapping_info)(void);

        /**
         * @brief Pointer to function for flagging duplicate alias detection
         *
         * @details Sets global flag indicating at least one duplicate alias was detected.
         * Main state machine checks this flag and handles conflicts.
         *
         * @warning Required assignment - Defaults to AliasMappings_set_has_duplicate_alias_flag()
         *
         * @see AliasMappings_set_has_duplicate_alias_flag - Standard implementation
         */
        void (*alias_mapping_set_has_duplicate_alias_flag)(void);

        // OPTIONAL FUNCTION

        // CALLBACK FUNCTIONS

        /*@}*/

    } interface_can_rx_message_handler_t;


    /**
     * @brief Initializes the CAN Receive Message Handler module
     *
     * @details Stores the dependency injection interface for use by message handlers.
     * Must be called during application initialization before processing any CAN frames.
     *
     * Use cases:
     * - Application startup sequence
     * - System initialization
     *
     * @param interface_can_frame_message_handler Pointer to interface structure containing required function pointers
     *
     * @warning Must be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call before CanRxStatemachine_incoming_can_driver_callback is invoked
     *
     * @see interface_can_rx_message_handler_t - Interface structure definition
     */
    extern void CanRxMessageHandler_initialize(const interface_can_rx_message_handler_t *interface_can_frame_message_handler);


    /**
     * @brief Handles the first frame of a multi-frame OpenLCB message
     *
     * @details Processes the initial CAN frame of a multi-frame message sequence. Allocates
     * an OpenLCB message buffer, initializes it with source/dest/MTI information, copies
     * payload data starting at the specified offset, and adds the message to the buffer list
     * for continued assembly.
     *
     * Use cases:
     * - Receiving first frame of datagram
     * - Receiving first frame of addressed message
     * - Starting multi-frame SNIP assembly
     *
     * @param can_msg Pointer to received CAN frame buffer
     * @param offset Byte offset in payload where OpenLCB data begins (2 if addressed, 0 if global)
     * @param data_type Type of OpenLCB buffer needed (BASIC, DATAGRAM, SNIP, STREAM)
     *
     * @warning Sends rejection message if buffer allocation fails
     * @warning Sends rejection message if message already in progress (out of order)
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_FIRST
     *
     * @see CanRxMessageHandler_middle_frame - Processes subsequent frames
     * @see CanRxMessageHandler_last_frame - Completes message assembly
     */
    extern void CanRxMessageHandler_first_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles a middle frame of a multi-frame OpenLCB message
     *
     * @details Processes continuation frames in a multi-frame message sequence. Finds the
     * in-progress message in the buffer list and appends payload data from this frame.
     *
     * Use cases:
     * - Receiving middle frames of long datagrams
     * - Receiving middle frames of SNIP replies
     * - Processing frames between first and last
     *
     * @param can_msg Pointer to received CAN frame buffer
     * @param offset Byte offset in payload where OpenLCB data begins (2 if addressed, 0 if global)
     *
     * @warning Sends rejection message if no matching message found (out of order)
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_MIDDLE
     * @attention First frame must have been received and processed
     *
     * @see CanRxMessageHandler_first_frame - Starts message assembly
     * @see CanRxMessageHandler_last_frame - Completes message assembly
     */
    extern void CanRxMessageHandler_middle_frame(can_msg_t *can_msg, uint8_t offset);


    /**
     * @brief Handles the last frame of a multi-frame OpenLCB message
     *
     * @details Processes the final CAN frame of a multi-frame message sequence. Finds the
     * in-progress message, appends final payload data, marks message as complete, removes
     * from buffer list, and pushes to OpenLCB FIFO for protocol processing.
     *
     * Use cases:
     * - Completing datagram reception
     * - Completing SNIP message assembly
     * - Finishing any multi-frame message
     *
     * @param can_msg Pointer to received CAN frame buffer
     * @param offset Byte offset in payload where OpenLCB data begins (2 if addressed, 0 if global)
     *
     * @warning Sends rejection message if no matching message found (out of order)
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_FINAL
     * @attention First frame must have been received and processed
     *
     * @see CanRxMessageHandler_first_frame - Starts message assembly
     * @see CanRxMessageHandler_middle_frame - Processes intermediate frames
     */
    extern void CanRxMessageHandler_last_frame(can_msg_t *can_msg, uint8_t offset);


    /**
     * @brief Handles a complete single-frame OpenLCB message
     *
     * @details Processes CAN frames that contain a complete OpenLCB message. Allocates an
     * OpenLCB buffer, initializes it with source/dest/MTI information, copies all payload
     * data, and pushes directly to OpenLCB FIFO for protocol processing.
     *
     * Use cases:
     * - Receiving short addressed messages
     * - Receiving event reports
     * - Processing messages that fit in one CAN frame
     *
     * @param can_msg Pointer to received CAN frame buffer
     * @param offset Byte offset in payload where OpenLCB data begins (2 if addressed, 0 if global)
     * @param data_type Type of OpenLCB buffer needed (typically BASIC)
     *
     * @warning Silently drops message if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_ONLY or no framing bits
     *
     * @see OpenLcbBufferFifo_push - Where complete messages are queued
     */
    extern void CanRxMessageHandler_single_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles legacy SNIP messages without standard framing bits
     *
     * @details Processes SNIP (Simple Node Information Protocol) messages from early OpenLCB
     * implementations that predated the multi-frame framing bit protocol. Determines frame
     * type by counting NULL bytes in the payload - when 6 NULLs are found, message is complete.
     *
     * Use cases:
     * - Supporting legacy SNIP implementations
     * - Backward compatibility with older nodes
     * - Processing MTI_SIMPLE_NODE_INFO_REPLY without framing bits
     *
     * @param can_msg Pointer to received CAN frame buffer
     * @param offset Byte offset in payload where SNIP data begins
     * @param data_type Type of buffer needed (must be SNIP)
     *
     * @warning Only works correctly with SNIP messages
     * @warning NOT thread-safe
     *
     * @attention Requires exactly 6 NULL terminators in complete message
     * @attention Do not use for messages with standard framing bits
     *
     * @see CanRxMessageHandler_first_frame - Modern multi-frame handling
     * @see ProtocolSnip_validate_snip_reply - Validates 6 NULL requirement
     */
    extern void CanRxMessageHandler_can_legacy_snip(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles stream protocol CAN frames
     *
     * @details Placeholder for future stream protocol implementation. Stream protocol
     * allows continuous data transfer between nodes. Currently unimplemented.
     *
     * Use cases:
     * - Firmware upgrade data transfer (future)
     * - Continuous sensor data streaming (future)
     * - Large file transfers (future)
     *
     * @param can_msg Pointer to received CAN frame buffer
     * @param offset Byte offset in payload where stream data begins
     * @param data_type Type of buffer needed (must be STREAM)
     *
     * @warning Currently unimplemented - function body is empty
     * @warning NOT thread-safe
     *
     * @attention Reserved for future use
     *
     * @see CAN_FRAME_TYPE_STREAM - Stream frame identifier
     */
    extern void CanRxMessageHandler_stream_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles RID (Reserve ID) CAN control frames
     *
     * @details Processes RID frames which indicate a node has completed its CID sequence
     * and is claiming its alias. If we already have this alias mapped, sends an RID response
     * to signal potential conflict.
     *
     * Use cases:
     * - Alias conflict detection during login
     * - Monitoring network for duplicate aliases
     *
     * @param can_msg Pointer to received RID CAN frame
     *
     * @warning Silently drops response if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention Part of CAN Frame Transfer Protocol alias allocation sequence
     *
     * @see CanRxMessageHandler_cid_frame - Check ID frame handler
     * @see CanRxMessageHandler_amd_frame - Alias Map Definition handler
     */
    extern void CanRxMessageHandler_rid_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AMD (Alias Map Definition) CAN control frames
     *
     * @details Processes AMD frames which announce alias/NodeID mappings. Checks for
     * duplicate aliases and flags conflicts for main state machine to resolve.
     *
     * Use cases:
     * - Learning alias/NodeID mappings from other nodes
     * - Duplicate alias detection
     * - Network topology discovery
     *
     * @param can_msg Pointer to received AMD CAN frame containing 6-byte NodeID
     *
     * @warning Sets duplicate flag if alias conflict detected
     * @warning NOT thread-safe
     *
     * @attention AMD frames contain full 48-bit NodeID in payload
     * @attention Part of CAN Frame Transfer Protocol
     *
     * @see CanRxMessageHandler_rid_frame - Reserve ID handler
     * @see AliasMappings_register - Where mappings are stored
     */
    extern void CanRxMessageHandler_amd_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AME (Alias Map Enquiry) CAN control frames
     *
     * @details Processes AME frames which request alias information. Responds with AMD
     * frames for all our registered aliases, or for a specific NodeID if provided.
     *
     * Use cases:
     * - Responding to network topology queries
     * - Gateway alias table synchronization
     * - Network diagnostics and monitoring
     *
     * @param can_msg Pointer to received AME CAN frame (may contain optional NodeID)
     *
     * @warning Silently drops responses if buffer allocation fails
     * @warning Returns early if duplicate alias detected
     * @warning NOT thread-safe
     *
     * @attention Empty payload requests all mappings
     * @attention 6-byte payload requests specific NodeID mapping
     *
     * @see CanRxMessageHandler_amd_frame - Processes our AMD replies
     */
    extern void CanRxMessageHandler_ame_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AMR (Alias Map Reset) CAN control frames
     *
     * @details Processes AMR frames which command a node to release its alias. Checks
     * for duplicate alias condition.
     *
     * Use cases:
     * - Receiving alias conflict resolution commands
     * - Detecting duplicate aliases
     * - Network alias management
     *
     * @param can_msg Pointer to received AMR CAN frame
     *
     * @warning Sets duplicate flag if alias conflict detected
     * @warning NOT thread-safe
     *
     * @attention Node must go to Inhibited state if AMR is for our alias
     * @attention Main state machine handles actual alias release
     *
     * @see CanMainStatemachine_handle_duplicate_aliases - Conflict resolution
     */
    extern void CanRxMessageHandler_amr_frame(can_msg_t *can_msg);


    /**
     * @brief Handles Error Information Report CAN control frames
     *
     * @details Processes error report frames from other nodes. Checks for duplicate
     * alias condition. Error reports indicate network problems or protocol violations.
     *
     * Use cases:
     * - Receiving error notifications from other nodes
     * - Network diagnostics
     * - Duplicate alias detection
     *
     * @param can_msg Pointer to received error report CAN frame
     *
     * @warning Sets duplicate flag if alias conflict detected
     * @warning NOT thread-safe
     *
     * @attention Error reports are informational - no response required
     *
     * @see CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0 - Error frame identifier base
     */
    extern void CanRxMessageHandler_error_info_report_frame(can_msg_t *can_msg);


    /**
     * @brief Handles CID (Check ID) CAN control frames
     *
     * @details Processes CID frames which check for alias conflicts during login sequence.
     * If we already have this alias mapped, sends an RID response to indicate conflict.
     *
     * Use cases:
     * - Alias conflict detection during other nodes' login
     * - Responding to CID sequences
     * - Network alias validation
     *
     * @param can_msg Pointer to received CID CAN frame containing NodeID fragment
     *
     * @warning Silently drops RID response if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention CID sequence uses frames CID7, CID6, CID5, CID4 with NodeID fragments
     * @attention Part of CAN Frame Transfer Protocol alias allocation
     *
     * @see CanRxMessageHandler_rid_frame - Reserve ID handler
     * @see CanLoginStateMachine_run - Our login sequence
     */
    extern void CanRxMessageHandler_cid_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__ */
