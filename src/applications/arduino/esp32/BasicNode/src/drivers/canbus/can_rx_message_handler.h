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
     * @brief Interface structure for CAN RX message handler callback functions
     *
     * @details This structure defines the callback interface for CAN receive message handlers,
     * which process incoming CAN frames and convert them to OpenLCB messages. The handlers
     * manage multi-frame message assembly, alias mapping lookups, and buffer allocation from
     * both CAN and OpenLCB buffer pools.
     *
     * The Rx message handlers perform the following key operations:
     *
     * Multi-Frame Message Assembly:
     * - Single-frame messages: Complete OpenLCB message in one CAN frame
     * - First-frame: Allocates buffer, initializes message, adds to buffer list
     * - Middle-frame: Finds in-progress message, appends payload data
     * - Last-frame: Appends final data, completes message, pushes to OpenLCB FIFO
     *
     * CAN Control Frame Processing:
     * - CID (Check ID): Checks for alias conflicts during other nodes' login
     * - RID (Reserve ID): Detects alias conflicts and responds if needed
     * - AMD (Alias Map Definition): Learns alias/NodeID mappings, detects duplicates
     * - AME (Alias Map Enquiry): Responds with our alias mappings
     * - AMR (Alias Map Reset): Detects duplicate alias conflicts
     * - Error Reports: Monitors network errors, detects duplicates
     *
     * Legacy Protocol Support:
     * - Legacy SNIP: Handles SNIP messages without standard framing bits
     *   Uses NULL byte counting to detect message completion
     *
     * Buffer Management:
     * The handlers require access to both buffer allocation systems:
     * - CAN buffers: For building outgoing control messages (AMD, RID responses)
     * - OpenLCB buffers: For assembling received multi-frame messages
     *
     * Alias Management:
     * Handlers interact with alias mapping system to:
     * - Look up NodeID by alias (validate addressed messages)
     * - Look up alias by NodeID (respond to AME queries)
     * - Access full mapping table (global AME responses)
     * - Flag duplicate alias conditions (conflict detection)
     *
     * All required callbacks must be provided for proper frame processing. The handlers
     * are invoked by the CAN Rx state machine which decodes frame types and routes to
     * appropriate handlers.
     *
     * @note All callbacks are REQUIRED - none can be NULL
     * @note Handlers are called from Rx state machine context
     * @note Multi-frame assembly uses buffer list for in-progress messages
     *
     * @see CanRxMessageHandler_initialize
     * @see can_rx_statemachine.h - Invokes these handlers
     */
    typedef struct {

        /**
         * @brief Callback to allocate CAN message buffers
         *
         * @details This required callback allocates buffers from the CAN buffer pool.
         * Used when building outgoing CAN control messages in response to received frames,
         * such as:
         * - AMD frames in response to AME (Alias Map Enquiry)
         * - RID frames in response to CID conflicts
         *
         * Returns pointer to allocated CAN message buffer, or NULL if pool exhausted.
         *
         * Typical implementation: CanBufferStore_allocate_buffer
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
        can_msg_t *(*can_buffer_store_allocate_buffer)(void);

        /**
         * @brief Callback to allocate OpenLCB message buffers
         *
         * @details This required callback allocates buffers from the OpenLCB buffer pool.
         * Used when assembling multi-frame CAN messages into complete OpenLCB messages.
         *
         * The payload_type parameter specifies buffer size needed:
         * - BASIC: Short messages (8 bytes)
         * - DATAGRAM: Datagram messages (up to 72 bytes)
         * - SNIP: SNIP replies (up to 256 bytes)
         * - STREAM: Stream data (large transfers)
         *
         * Returns pointer to allocated OpenLCB message buffer, or NULL if pool exhausted.
         *
         * Typical implementation: OpenLcbBufferStore_allocate_buffer
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
        openlcb_msg_t *(*openlcb_buffer_store_allocate_buffer)(payload_type_enum payload_type);

        /**
         * @brief Callback to find alias mapping by alias
         *
         * @details This required callback searches the alias mapping table for an entry
         * matching the specified 12-bit CAN alias. Used for:
         * - Validating addressed messages (check if alias is ours)
         * - Alias conflict detection during RID/CID processing
         * - Determining if response needed to CID frames
         *
         * Returns pointer to alias_mapping_t if found, NULL otherwise.
         *
         * Typical implementation: AliasMappings_find_mapping_by_alias
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
        alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

        /**
         * @brief Callback to find alias mapping by Node ID
         *
         * @details This required callback searches the alias mapping table for an entry
         * matching the specified 48-bit Node ID. Used when responding to:
         * - AME frames with specific NodeID (targeted alias query)
         * - Validating AMD frames contain correct NodeID
         *
         * Returns pointer to alias_mapping_t if found, NULL otherwise.
         *
         * Typical implementation: AliasMappings_find_mapping_by_node_id
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
        alias_mapping_t *(*alias_mapping_find_mapping_by_node_id)(node_id_t node_id);

        /**
         * @brief Callback to access complete alias mapping table
         *
         * @details This required callback returns pointer to the alias mapping structure
         * containing all registered alias/NodeID pairs. Used when responding to:
         * - Global AME frames (request for all mappings)
         * - Network topology queries
         *
         * Returns pointer to alias_mapping_info_t structure containing:
         * - Array of all alias mappings
         * - Duplicate alias flag
         *
         * Typical implementation: AliasMappings_get_alias_mapping_info
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
        alias_mapping_info_t *(*alias_mapping_get_alias_mapping_info)(void);

        /**
         * @brief Callback to flag duplicate alias detection
         *
         * @details This required callback sets the global duplicate alias flag indicating
         * at least one alias conflict has been detected. The main state machine monitors
         * this flag and invokes conflict resolution.
         *
         * Called when handlers detect duplicate conditions in:
         * - AMD frames (two nodes claiming same alias)
         * - AMR frames (alias reset command received)
         * - Error report frames (duplicate alias errors)
         *
         * Typical implementation: AliasMappings_set_has_duplicate_alias_flag
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
        void (*alias_mapping_set_has_duplicate_alias_flag)(void);

    } interface_can_rx_message_handler_t;


    /**
     * @brief Initializes the CAN Receive Message Handler module
     *
     * @details Registers the application's callback interface with the receive message
     * handlers. The interface provides access to buffer allocation, alias mapping lookups,
     * and duplicate detection. Must be called during application initialization before
     * processing any CAN frames.
     *
     * Use cases:
     * - Application startup sequence
     * - System initialization
     *
     * @param interface_can_frame_message_handler Pointer to interface structure containing
     * all required callback function pointers
     *
     * @warning interface_can_frame_message_handler must remain valid for lifetime of application
     * @warning All required callbacks must be valid (non-NULL)
     * @warning Must be called exactly once during initialization
     * @warning NOT thread-safe - call during single-threaded initialization only
     *
     * @attention Call after CanBufferStore_initialize and OpenLcbBufferStore_initialize
     * @attention Call before CanRxStatemachine_incoming_can_driver_callback is invoked
     *
     * @see interface_can_rx_message_handler_t - Interface structure definition
     * @see CanRxStatemachine_initialize - Initialize Rx state machine
     */
    extern void CanRxMessageHandler_initialize(const interface_can_rx_message_handler_t *interface_can_frame_message_handler);


    /**
     * @brief Handles the first frame of a multi-frame OpenLCB message
     *
     * @details Processes the initial CAN frame of a multi-frame message sequence. Allocates
     * an OpenLCB message buffer of appropriate type, initializes it with source/dest/MTI
     * information extracted from CAN header, copies payload data starting at the specified
     * offset, and adds the message to the buffer list for continued assembly.
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
     * @warning Sends rejection message if message already in progress (out of sequence)
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_FIRST
     * @attention Addressed frames have 2-byte destination alias overhead
     *
     * @note First frame must contain exactly 8 bytes (full CAN frame)
     *
     * @see CanRxMessageHandler_middle_frame - Processes subsequent frames
     * @see CanRxMessageHandler_last_frame - Completes message assembly
     */
    extern void CanRxMessageHandler_first_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles a middle frame of a multi-frame OpenLCB message
     *
     * @details Processes continuation frames in a multi-frame message sequence. Finds the
     * in-progress message in the buffer list by matching source/dest aliases and MTI, then
     * appends payload data from this frame.
     *
     * Use cases:
     * - Receiving middle frames of long datagrams
     * - Receiving middle frames of SNIP replies
     * - Processing frames between first and last
     *
     * @param can_msg Pointer to received CAN frame buffer
     * @param offset Byte offset in payload where OpenLCB data begins (2 if addressed, 0 if global)
     *
     * @warning Sends rejection message if no matching message found (out of sequence)
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_MIDDLE
     * @attention First frame must have been received and processed
     * @attention Middle frames must contain exactly 8 bytes
     *
     * @note Searches buffer list for in-progress message
     *
     * @see CanRxMessageHandler_first_frame - Starts message assembly
     * @see CanRxMessageHandler_last_frame - Completes message assembly
     */
    extern void CanRxMessageHandler_middle_frame(can_msg_t *can_msg, uint8_t offset);


    /**
     * @brief Handles the last frame of a multi-frame OpenLCB message
     *
     * @details Processes the final CAN frame of a multi-frame message sequence. Finds the
     * in-progress message in buffer list, appends final payload data, marks message as
     * complete, removes from buffer list, and pushes to OpenLCB FIFO for protocol processing.
     *
     * Use cases:
     * - Completing datagram reception
     * - Completing SNIP message assembly
     * - Finishing any multi-frame message
     *
     * @param can_msg Pointer to received CAN frame buffer
     * @param offset Byte offset in payload where OpenLCB data begins (2 if addressed, 0 if global)
     *
     * @warning Sends rejection message if no matching message found (out of sequence)
     * @warning NOT thread-safe
     *
     * @attention Frame must have framing bits set to MULTIFRAME_FINAL
     * @attention First frame must have been received and processed
     * @attention Last frame may contain 0-8 bytes of data
     *
     * @note Completes assembly and forwards to OpenLCB layer
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
     * @attention Single-frame messages bypass buffer list (no assembly needed)
     *
     * @note Most common message type (events, short commands)
     *
     * @see OpenLcbBufferFifo_push - Where complete messages are queued
     */
    extern void CanRxMessageHandler_single_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles legacy SNIP messages without standard framing bits
     *
     * @details Processes SNIP (Simple Node Information Protocol) messages from early OpenLCB
     * implementations that predated the multi-frame framing bit protocol. Determines message
     * completion by counting NULL terminators - complete SNIP messages contain exactly 6 NULLs
     * marking the end of 6 null-terminated strings.
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
     * @warning Only works correctly with SNIP messages (MTI 0x0A08)
     * @warning NOT thread-safe
     *
     * @attention Requires exactly 6 NULL terminators in complete message
     * @attention Do not use for messages with standard framing bits
     * @attention Legacy protocol - modern implementations should use framing bits
     *
     * @note SNIP format: 4 manufacturer strings + 2 user strings
     *
     * @see CanRxMessageHandler_first_frame - Modern multi-frame handling
     * @see ProtocolSnip_validate_snip_reply - Validates 6 NULL requirement
     */
    extern void CanRxMessageHandler_can_legacy_snip(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles stream protocol CAN frames
     *
     * @details Placeholder for future stream protocol implementation. Stream protocol
     * allows continuous data transfer between nodes for applications like firmware
     * upgrades and sensor data streaming. Currently unimplemented.
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
     * @warning Do not rely on this function until implemented
     * @warning NOT thread-safe
     *
     * @attention Reserved for future use
     * @attention Requires full implementation before production use
     *
     * @see CAN_FRAME_TYPE_STREAM - Stream frame identifier
     */
    extern void CanRxMessageHandler_stream_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles RID (Reserve ID) CAN control frames
     *
     * @details Processes RID frames which indicate a node has completed its CID sequence
     * and is claiming its alias. If we already have this alias mapped to one of our nodes,
     * sends an RID response to signal potential conflict.
     *
     * Use cases:
     * - Alias conflict detection during other nodes' login
     * - Monitoring network for duplicate aliases
     * - Responding to RID claims
     *
     * @param can_msg Pointer to received RID CAN frame
     *
     * @warning Silently drops response if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention Part of CAN Frame Transfer Protocol alias allocation sequence
     * @attention RID follows 4 CID frames and 200ms wait
     *
     * @note RID response indicates alias conflict
     *
     * @see CanRxMessageHandler_cid_frame - Check ID frame handler
     * @see CanRxMessageHandler_amd_frame - Alias Map Definition handler
     */
    extern void CanRxMessageHandler_rid_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AMD (Alias Map Definition) CAN control frames
     *
     * @details Processes AMD frames which announce alias/NodeID mappings to the network.
     * Extracts the 48-bit NodeID from payload and checks for duplicate alias conditions
     * by comparing with our registered aliases. Flags conflicts for main state machine
     * to resolve.
     *
     * Use cases:
     * - Learning alias/NodeID mappings from other nodes
     * - Duplicate alias detection
     * - Network topology discovery
     *
     * @param can_msg Pointer to received AMD CAN frame containing 6-byte NodeID in payload
     *
     * @warning Sets duplicate flag if alias conflict detected
     * @warning NOT thread-safe
     *
     * @attention AMD frames contain full 48-bit NodeID in 6-byte payload
     * @attention Part of CAN Frame Transfer Protocol
     * @attention AMD is final step in login sequence
     *
     * @note AMD announces successful alias allocation
     *
     * @see CanRxMessageHandler_rid_frame - Reserve ID handler
     * @see AliasMappings_register - Where mappings are stored
     */
    extern void CanRxMessageHandler_amd_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AME (Alias Map Enquiry) CAN control frames
     *
     * @details Processes AME frames which request alias information from the network.
     * Responds with AMD frames for:
     * - All our registered aliases (if AME payload empty - global query)
     * - Specific NodeID mapping (if AME contains 6-byte NodeID - targeted query)
     *
     * Use cases:
     * - Responding to network topology queries
     * - Gateway alias table synchronization
     * - Network diagnostics and monitoring
     *
     * @param can_msg Pointer to received AME CAN frame (may contain optional 6-byte NodeID)
     *
     * @warning Silently drops responses if buffer allocation fails
     * @warning Returns early if duplicate alias detected
     * @warning NOT thread-safe
     *
     * @attention Empty payload (0 bytes) requests all mappings
     * @attention 6-byte payload requests specific NodeID mapping
     * @attention May generate multiple AMD responses for global query
     *
     * @note Gateways use AME to synchronize alias tables
     *
     * @see CanRxMessageHandler_amd_frame - Processes our AMD replies
     */
    extern void CanRxMessageHandler_ame_frame(can_msg_t *can_msg);


    /**
     * @brief Handles AMR (Alias Map Reset) CAN control frames
     *
     * @details Processes AMR frames which command a node to release its alias. Extracts
     * NodeID from payload and checks if it matches any of our nodes' IDs. If match found,
     * flags duplicate alias condition for main state machine to handle.
     *
     * Use cases:
     * - Receiving alias conflict resolution commands
     * - Detecting duplicate aliases
     * - Network alias management
     *
     * @param can_msg Pointer to received AMR CAN frame containing 6-byte NodeID
     *
     * @warning Sets duplicate flag if alias conflict detected
     * @warning NOT thread-safe
     *
     * @attention Node must go to Inhibited state if AMR is for our alias
     * @attention Main state machine handles actual alias release
     * @attention AMR payload contains full 48-bit NodeID
     *
     * @note AMR typically sent when duplicate detected
     *
     * @see CanMainStatemachine_handle_duplicate_aliases - Conflict resolution
     */
    extern void CanRxMessageHandler_amr_frame(can_msg_t *can_msg);


    /**
     * @brief Handles Error Information Report CAN control frames
     *
     * @details Processes error report frames from other nodes indicating network problems
     * or protocol violations. Extracts NodeID and checks if it matches any of our nodes,
     * flagging duplicate alias if match found.
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
     * @attention May indicate serious network problems
     *
     * @note Error codes defined in OpenLCB standards
     *
     * @see CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0 - Error frame identifier base
     */
    extern void CanRxMessageHandler_error_info_report_frame(can_msg_t *can_msg);


    /**
     * @brief Handles CID (Check ID) CAN control frames
     *
     * @details Processes CID frames which check for alias conflicts during another node's
     * login sequence. If we already have this alias mapped to one of our nodes, sends an
     * RID response to indicate conflict, forcing the other node to generate a new alias.
     *
     * The CID sequence consists of 4 frames announcing Node ID fragments:
     * - CID7: Node ID bits 47-36
     * - CID6: Node ID bits 35-24
     * - CID5: Node ID bits 23-12
     * - CID4: Node ID bits 11-0
     *
     * Use cases:
     * - Alias conflict detection during other nodes' login
     * - Responding to CID sequences
     * - Network alias validation
     *
     * @param can_msg Pointer to received CID CAN frame containing NodeID fragment in header
     *
     * @warning Silently drops RID response if buffer allocation fails
     * @warning NOT thread-safe
     *
     * @attention CID sequence uses frames CID7, CID6, CID5, CID4 with NodeID fragments
     * @attention Part of CAN Frame Transfer Protocol alias allocation
     * @attention Node ID fragments are in CAN header, not payload
     *
     * @note RID response indicates alias conflict
     *
     * @see CanRxMessageHandler_rid_frame - Reserve ID handler
     * @see CanLoginStateMachine_run - Our login sequence
     */
    extern void CanRxMessageHandler_cid_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__ */
