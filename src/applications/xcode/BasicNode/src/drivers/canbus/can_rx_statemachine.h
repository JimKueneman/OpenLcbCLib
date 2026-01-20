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
 * @file can_rx_statemachine.h
 * @brief State machine for receiving and decoding incoming CAN frames
 *
 * @details This module implements the CAN receive state machine that processes raw CAN
 * frames from the hardware driver and routes them to appropriate handlers. It decodes
 * the CAN frame format, extracts MTI and addressing information, and dispatches to
 * message handlers based on frame type.
 *
 * The state machine handles:
 * - CAN control frames (CID, RID, AMD, AME, AMR, error reports)
 * - OpenLCB message frames (global, addressed, datagram, stream)
 * - Multi-frame message assembly coordination
 * - Framing bit extraction and validation
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

    /**
     * @brief Interface structure for CAN receive state machine callback functions
     *
     * @details This structure defines the callback interface for the CAN receive state machine,
     * which serves as the primary entry point from the hardware CAN driver into the OpenLCB
     * library. The state machine decodes incoming CAN frames, determines their type and purpose,
     * and dispatches to appropriate message handlers for processing.
     *
     * The CAN Rx state machine performs frame classification and routing:
     *
     * CAN Frame Type Detection:
     * The state machine examines the 29-bit CAN identifier to determine frame type:
     * - Bit 27 = 0: CAN control frame (CID, RID, AMD, AME, AMR, error)
     * - Bit 27 = 1: OpenLCB message frame (requires further decoding)
     *
     * CAN Control Frame Routing (Bit 27 = 0):
     * - CID frames (0x07xx): Check ID during alias allocation → handle_cid_frame
     * - RID frames (0x0700): Reserve ID claim → handle_rid_frame
     * - AMD frames (0x0701): Alias Map Definition → handle_amd_frame
     * - AME frames (0x0702): Alias Map Enquiry → handle_ame_frame
     * - AMR frames (0x0703): Alias Map Reset → handle_amr_frame
     * - Error frames (0x071x): Error information → handle_error_info_report_frame
     *
     * OpenLCB Message Frame Routing (Bit 27 = 1):
     * The state machine extracts additional information from the CAN header:
     * - MTI (Message Type Indicator) from bits in header
     * - Addressing (global vs addressed) from bit 3
     * - Frame type (stream/datagram) from bit 12
     *
     * Multi-Frame Message Assembly:
     * For OpenLCB messages, the state machine examines framing bits in first payload byte:
     * - 00 (ONLY): Complete message in one frame → handle_single_frame
     * - 01 (FIRST): Start of multi-frame sequence → handle_first_frame
     * - 10 (LAST): Final frame of sequence → handle_last_frame
     * - 11 (MIDDLE): Continuation frame → handle_middle_frame
     *
     * Legacy Protocol Support:
     * - Legacy SNIP: Special handling for SNIP messages without framing bits
     *   Uses NULL byte counting instead of framing flags → handle_can_legacy_snip
     *
     * Stream Protocol Support:
     * - Stream frames: For continuous data transfer (future implementation)
     *   → handle_stream_frame
     *
     * Payload Offset Calculation:
     * The state machine determines where OpenLCB data begins in CAN payload:
     * - Addressed messages: offset = 2 (first 2 bytes contain destination alias)
     * - Global messages: offset = 0 (no destination, data starts immediately)
     *
     * Alias Mapping Integration:
     * The state machine validates addressed messages by checking if destination alias
     * belongs to one of our nodes before dispatching to handlers. Uses:
     * - alias_mapping_find_mapping_by_alias callback for validation
     *
     * All 13 required callbacks (12 message handlers + 1 alias lookup) must be provided
     * for proper frame processing. One optional callback (on_receive) allows application
     * notification for monitoring or statistics gathering.
     *
     * The state machine is invoked from interrupt/thread context via
     * CanRxStatemachine_incoming_can_driver_callback, which is called by the hardware
     * driver when a frame is received.
     *
     * @note 13 callbacks are REQUIRED, 1 callback is Optional
     * @note State machine called from interrupt/thread context
     * @note Dispatches synchronously - handlers must execute quickly
     *
     * @see CanRxStatemachine_initialize
     * @see CanRxStatemachine_incoming_can_driver_callback
     * @see can_rx_message_handler.h - Message handler implementations
     */
typedef struct
{

        /**
         * @brief Callback to handle legacy SNIP messages without framing bits
         *
         * @details This required callback processes SNIP (Simple Node Information Protocol)
         * messages from early OpenLCB implementations that predate the standard multi-frame
         * framing bit protocol. Instead of using framing bits, legacy SNIP uses NULL byte
         * counting to detect message completion (exactly 6 NULL terminators required).
         *
         * The callback should:
         * - Find or allocate in-progress SNIP message in buffer list
         * - Append payload data to message
         * - Count NULL bytes in accumulated payload
         * - When 6 NULLs found, mark message complete and push to OpenLCB FIFO
         *
         * Typical implementation: CanRxMessageHandler_can_legacy_snip
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Only used for MTI 0x0A08 (Simple Node Info Reply) from legacy nodes
         */
    void (*handle_can_legacy_snip)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

        /**
         * @brief Callback to handle single-frame OpenLCB messages
         *
         * @details This required callback processes complete OpenLCB messages that fit
         * entirely within one CAN frame. These are the most common message type for events,
         * short commands, and replies.
         *
         * The callback should:
         * - Allocate OpenLCB buffer of appropriate type
         * - Extract source alias, destination alias (if addressed), and MTI from CAN header
         * - Copy payload data from CAN frame to OpenLCB message
         * - Push complete message to OpenLCB FIFO for protocol processing
         *
         * Framing bits for single-frame messages:
         * - 00 (MULTIFRAME_ONLY) or no framing bits present
         *
         * Typical implementation: CanRxMessageHandler_single_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Most common message type (events, short commands)
         */
    void (*handle_single_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

        /**
         * @brief Callback to handle first frame of multi-frame messages
         *
         * @details This required callback processes the initial CAN frame of a multi-frame
         * OpenLCB message sequence. Initiates message assembly by allocating buffer and
         * storing first payload chunk.
         *
         * The callback should:
         * - Allocate OpenLCB buffer of specified type (DATAGRAM, SNIP, etc.)
         * - Extract source alias, destination alias (if addressed), and MTI from CAN header
         * - Initialize OpenLCB message structure
         * - Copy first payload chunk from CAN frame
         * - Add message to buffer list for continued assembly
         *
         * Framing bits for first-frame messages:
         * - 01 (MULTIFRAME_FIRST)
         *
         * Frame must be exactly 8 bytes total:
         * - Addressed: 2 bytes dest alias + 6 bytes data
         * - Global: 8 bytes data
         *
         * Typical implementation: CanRxMessageHandler_first_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Addressed frames have 2-byte destination overhead
         */
    void (*handle_first_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

        /**
         * @brief Callback to handle middle frames of multi-frame messages
         *
         * @details This required callback processes continuation frames in a multi-frame
         * message sequence. Appends payload data to in-progress message being assembled
         * in the buffer list.
         *
         * The callback should:
         * - Find in-progress message in buffer list (match source, dest, MTI)
         * - Append payload data from this frame to message
         * - Verify frame is in correct sequence
         * - Send rejection if no matching message found (out of sequence)
         *
         * Framing bits for middle-frame messages:
         * - 11 (MULTIFRAME_MIDDLE)
         *
         * Frame must be exactly 8 bytes total:
         * - Addressed: 2 bytes dest alias + 6 bytes data
         * - Global: 8 bytes data
         *
         * Typical implementation: CanRxMessageHandler_middle_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note First frame must have been received before middle frames
         */
    void (*handle_middle_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index);

        /**
         * @brief Callback to handle last frame of multi-frame messages
         *
         * @details This required callback processes the final CAN frame of a multi-frame
         * message sequence. Completes message assembly and forwards to OpenLCB layer.
         *
         * The callback should:
         * - Find in-progress message in buffer list (match source, dest, MTI)
         * - Append final payload data from this frame
         * - Mark message as complete
         * - Remove from buffer list
         * - Push to OpenLCB FIFO for protocol processing
         * - Send rejection if no matching message found (out of sequence)
         *
         * Framing bits for last-frame messages:
         * - 10 (MULTIFRAME_LAST)
         *
         * Frame may contain 0-8 bytes:
         * - Addressed: 2 bytes dest alias + 0-6 bytes data
         * - Global: 0-8 bytes data
         *
         * Typical implementation: CanRxMessageHandler_last_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Completes assembly and forwards message
         */
    void (*handle_last_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index);

        /**
         * @brief Callback to handle stream protocol CAN frames
         *
         * @details This required callback processes stream frames used for continuous
         * high-throughput data transfer. Stream protocol is used for applications like
         * firmware upgrades that require transferring large amounts of data efficiently.
         *
         * The callback should:
         * - Process stream initiation, data, and completion frames
         * - Handle stream flow control
         * - Manage stream buffers
         *
         * Note: Stream protocol is currently not fully implemented. This is a placeholder
         * for future functionality.
         *
         * Typical implementation: CanRxMessageHandler_stream_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Currently unimplemented - placeholder for future use
         */
    void (*handle_stream_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

        /**
         * @brief Callback to handle RID (Reserve ID) CAN control frames
         *
         * @details This required callback processes RID frames which indicate a node has
         * completed its CID sequence and is claiming its alias. If we already have this
         * alias registered to one of our nodes, sends RID response to signal conflict.
         *
         * The callback should:
         * - Extract alias from CAN identifier
         * - Check if alias is already registered to our nodes
         * - If conflict detected, send RID response frame
         *
         * RID frame format:
         * - CAN header: 0x0700 + alias
         * - Payload: empty (0 bytes)
         *
         * Typical implementation: CanRxMessageHandler_rid_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Part of CAN Frame Transfer Protocol alias allocation
         */
    void (*handle_rid_frame)(can_msg_t *can_msg);

        /**
         * @brief Callback to handle AMD (Alias Map Definition) CAN control frames
         *
         * @details This required callback processes AMD frames which announce alias/NodeID
         * mappings to the network. Checks for duplicate alias conditions and flags conflicts
         * for main state machine resolution.
         *
         * The callback should:
         * - Extract alias from CAN identifier
         * - Extract 48-bit NodeID from 6-byte payload
         * - Check for duplicate alias (same alias, different NodeID)
         * - Flag duplicate if conflict detected
         *
         * AMD frame format:
         * - CAN header: 0x0701 + alias
         * - Payload: 6 bytes of NodeID
         *
         * Typical implementation: CanRxMessageHandler_amd_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Final step in CAN login sequence
         */
    void (*handle_amd_frame)(can_msg_t *can_msg);

        /**
         * @brief Callback to handle AME (Alias Map Enquiry) CAN control frames
         *
         * @details This required callback processes AME frames which request alias
         * information from the network. Responds with AMD frames for our registered aliases.
         *
         * The callback should:
         * - Check if AME is global (empty payload) or targeted (contains NodeID)
         * - If global: send AMD for all our registered aliases
         * - If targeted: send AMD only if NodeID matches one of our nodes
         *
         * AME frame format:
         * - CAN header: 0x0702 + alias (or 0 for global)
         * - Payload: empty (global query) or 6 bytes NodeID (targeted query)
         *
         * Typical implementation: CanRxMessageHandler_ame_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Gateways use AME to synchronize alias tables
         */
    void (*handle_ame_frame)(can_msg_t *can_msg);

        /**
         * @brief Callback to handle AMR (Alias Map Reset) CAN control frames
         *
         * @details This required callback processes AMR frames which command a node to
         * release its alias. Checks for duplicate alias condition.
         *
         * The callback should:
         * - Extract 48-bit NodeID from 6-byte payload
         * - Check if NodeID matches any of our nodes
         * - If match found, flag duplicate alias condition
         *
         * AMR frame format:
         * - CAN header: 0x0703 + alias
         * - Payload: 6 bytes of NodeID
         *
         * Main state machine handles actual alias release and re-login.
         *
         * Typical implementation: CanRxMessageHandler_amr_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Indicates alias conflict detected by another node
         */
    void (*handle_amr_frame)(can_msg_t *can_msg);

        /**
         * @brief Callback to handle error information report CAN control frames
         *
         * @details This required callback processes error report frames from other nodes
         * indicating network problems or protocol violations. Checks for duplicate alias
         * condition.
         *
         * The callback should:
         * - Extract error code and source information
         * - Check for duplicate alias indicators
         * - Flag duplicate if detected
         *
         * Error frame format:
         * - CAN header: 0x0710-0x0713 + alias
         * - Payload: Error code and details
         *
         * Typical implementation: CanRxMessageHandler_error_info_report_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Error reports are informational - no response required
         */
    void (*handle_error_info_report_frame)(can_msg_t *can_msg);

        /**
         * @brief Callback to handle CID (Check ID) CAN control frames
         *
         * @details This required callback processes CID frames during another node's alias
         * allocation sequence. If we have the alias being checked, sends RID response to
         * indicate conflict.
         *
         * The callback should:
         * - Extract alias from CAN identifier
         * - Check if alias is registered to any of our nodes
         * - If conflict, send RID response to signal alias in use
         *
         * CID sequence consists of 4 frames with NodeID fragments:
         * - CID7 (0x07xx): NodeID bits 47-36 in CAN header
         * - CID6 (0x06xx): NodeID bits 35-24 in CAN header
         * - CID5 (0x05xx): NodeID bits 23-12 in CAN header
         * - CID4 (0x04xx): NodeID bits 11-0 in CAN header
         *
         * Typical implementation: CanRxMessageHandler_cid_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Part of CAN Frame Transfer Protocol alias allocation
         */
    void (*handle_cid_frame)(can_msg_t *can_msg);

        /**
         * @brief Callback to find alias mapping by alias
         *
         * @details This required callback searches the alias mapping table for an entry
         * matching the specified 12-bit CAN alias. Used by state machine to validate
         * that addressed messages are for one of our nodes before dispatching to handlers.
         *
         * The callback should:
         * - Search alias mapping table for matching alias
         * - Return pointer to mapping entry if found
         * - Return NULL if alias not found
         *
         * Typical implementation: AliasMappings_find_mapping_by_alias
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Called before dispatching addressed messages to handlers
         */
    alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

        /**
         * @brief Optional callback for application notification on frame reception
         *
         * @details This optional callback provides immediate notification when a CAN frame
         * is received, before any processing or routing occurs. Useful for monitoring,
         * logging, statistics gathering, or LED indicators.
         *
         * The callback is invoked in interrupt/thread context and must:
         * - Execute very quickly (microseconds, not milliseconds)
         * - Not block execution
         * - Not call blocking functions
         * - Not perform lengthy processing
         *
         * Common uses:
         * - Increment frame counters
         * - Toggle activity LEDs
         * - Set flags for main loop processing
         * - Timestamp frame arrival
         *
         * @note Optional - can be NULL if notification is not needed
         * @note Called in interrupt/thread context - keep processing minimal
         */
    void (*on_receive)(can_msg_t *can_msg);

} interface_can_rx_statemachine_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Receive State Machine module
     *
     * @details Registers the application's callback interface with the receive state machine.
     * The interface provides handlers for all CAN frame types and alias mapping lookups.
     * Must be called during application initialization before processing any CAN frames.
     *
     * Use cases:
     * - Application startup sequence
     * - System initialization
     *
     * @param interface_can_rx_statemachine Pointer to interface structure containing all
     * required callback function pointers
     *
     * @warning interface_can_rx_statemachine must remain valid for lifetime of application
     * @warning All 13 required callbacks must be valid (non-NULL)
     * @warning Must be called exactly once during initialization
     * @warning NOT thread-safe - call during single-threaded initialization only
     *
     * @attention Call after CanRxMessageHandler_initialize
     * @attention Call before CanRxStatemachine_incoming_can_driver_callback is invoked
     * @attention All required function pointers in interface must be initialized
     *
     * @see interface_can_rx_statemachine_t - Interface structure definition
     * @see CanRxMessageHandler_initialize - Initialize message handlers first
     */
    extern void CanRxStatemachine_initialize(const interface_can_rx_statemachine_t *interface_can_rx_statemachine);


    /**
     * @brief Entry point for incoming CAN frames from hardware driver
     *
     * @details Called by the application's CAN hardware driver when a frame is received.
     * Serves as the primary entry point from the hardware layer into the OpenLCB library.
     * Decodes the CAN frame format, determines frame type and routing, and dispatches to
     * appropriate handler for processing.
     *
     * Frame Processing Flow:
     * 1. Invoke on_receive callback if provided (application notification)
     * 2. Examine CAN identifier bit 27 to determine frame category:
     *    - Bit 27 = 0: CAN control frame → route to control handlers
     *    - Bit 27 = 1: OpenLCB message → continue to step 3
     * 3. Extract MTI, addressing, and frame type from CAN header
     * 4. For addressed messages, validate destination alias belongs to our nodes
     * 5. Examine framing bits in first payload byte (for OpenLCB messages):
     *    - 00 or no bits: Single frame → handle_single_frame
     *    - 01: First frame → handle_first_frame
     *    - 10: Last frame → handle_last_frame
     *    - 11: Middle frame → handle_middle_frame
     * 6. Calculate payload offset (0 for global, 2 for addressed)
     * 7. Dispatch to appropriate handler with frame, offset, and buffer type
     *
     * Thread Safety Considerations:
     * This function is typically called from interrupt/thread context and accesses
     * shared resources (FIFOs, buffer lists). The application must ensure this function
     * is not called while the main state machine has resources locked.
     *
     * Recommended approaches:
     * - Interrupt-based: Disable CAN Rx interrupt during lock_shared_resources
     * - Thread-based (RTOS): Suspend Rx thread or queue frames during lock
     * - Polled: Don't poll during lock
     *
     * Lock duration is minimal (microseconds), so:
     * - Interrupt method: Brief interrupt disable, no frames dropped
     * - Thread method: Use lightweight queuing, resume after unlock
     *
     * Use cases:
     * - CAN receive interrupt handler
     * - CAN receive thread/task
     * - Polled CAN reception
     *
     * @param can_msg Pointer to received CAN frame buffer from hardware driver
     *
     * @warning can_msg must NOT be NULL
     * @warning Must not be called when shared resources are locked
     * @warning Frame buffer must remain valid until processing completes
     * @warning NOT thread-safe with main state machine
     *
     * @attention Application must respect lock_shared_resources/unlock_shared_resources
     * @attention If using interrupts, disable CAN RX interrupt during resource lock
     * @attention If using threads, either suspend thread or queue frame for later processing
     * @attention Lock duration is minimal (microseconds) - queuing should not overflow
     *
     * @note When lock_shared_resources is active, either:
     * @note - Disable CAN interrupts until unlock_shared_resources
     * @note - Queue incoming frames and call this function after unlock
     * @note - Suspend receiving thread until unlock (RTOS only)
     *
     * @see CanMainStatemachine_run - Main state machine that may lock resources
     * @see interface_can_rx_statemachine_t::on_receive - Optional receive callback
     * @see interface_can_rx_statemachine_t - All handler callbacks
     */
    extern void CanRxStatemachine_incoming_can_driver_callback(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__ */
