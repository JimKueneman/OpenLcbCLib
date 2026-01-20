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
 * @file can_tx_statemachine.h
 * @brief State machine for transmitting CAN frames
 *
 * @details Orchestrates the transmission of OpenLCB messages and CAN frames to the physical
 * CAN bus. Manages hardware buffer availability checking and delegates to appropriate message
 * type handlers for frame conversion and multi-frame sequencing.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_TX_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_TX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Interface structure for CAN transmit state machine callback functions
     *
     * @details This structure defines the callback interface for the CAN transmit state machine,
     * which orchestrates the conversion of OpenLCB messages to CAN frames and manages transmission
     * to the physical CAN bus. The state machine coordinates hardware buffer availability checking,
     * message type dispatch, and multi-frame sequencing.
     *
     * The CAN Tx state machine performs the following key operations:
     *
     * Transmission Orchestration:
     * The state machine serves as the coordinator for all CAN transmission activities:
     * 1. Hardware buffer availability checking before each frame transmission
     * 2. Message type identification and handler dispatch
     * 3. Multi-frame sequence management for messages requiring fragmentation
     * 4. Retry coordination when hardware buffers temporarily unavailable
     *
     * Hardware Buffer Management:
     * Before transmitting any frame, the state machine checks hardware buffer availability:
     * - Calls is_tx_buffer_empty to query hardware transmit buffer status
     * - If buffer available: proceeds with frame construction and transmission
     * - If buffer full: returns false immediately, allowing caller to retry later
     *
     * This prevents:
     * - Buffer overflow conditions
     * - Lost frames
     * - Transmission errors
     * - Hardware controller error states
     *
     * Message Type Dispatch:
     * The state machine examines the OpenLCB message structure to determine message type
     * and dispatches to the appropriate handler for frame conversion:
     *
     * 1. Addressed Messages (dest_alias present):
     *    - Handler: handle_addressed_msg_frame
     *    - Includes 12-bit destination alias in each frame
     *    - 6 bytes payload per frame (2 bytes overhead)
     *    - Used for: Protocol Support Inquiry, Verify Node ID, targeted commands
     *
     * 2. Unaddressed Messages (no dest_alias):
     *    - Handler: handle_unaddressed_msg_frame
     *    - Broadcast to all nodes on network
     *    - 8 bytes payload per frame (no overhead)
     *    - Used for: Initialization Complete, Event Reports, Verified Node ID
     *
     * 3. Datagram Messages (MTI indicates datagram):
     *    - Handler: handle_datagram_frame
     *    - Up to 72 bytes maximum
     *    - Special datagram frame format with type indicators
     *    - Used for: Memory Configuration, Remote Button, Display protocols
     *
     * 4. Stream Messages (MTI indicates stream):
     *    - Handler: handle_stream_frame
     *    - High-throughput continuous transfer
     *    - Flow control coordination
     *    - Used for: Firmware upgrades, large file transfers
     *
     * 5. Raw CAN Frames (pre-constructed):
     *    - Handler: handle_can_frame
     *    - No OpenLCB processing
     *    - Direct hardware transmission
     *    - Used for: CID, RID, AMD control frames
     *
     * Multi-Frame Transmission:
     * For messages requiring multiple CAN frames, the state machine manages the sequence:
     *
     * Transmission Loop:
     * 1. Check hardware buffer availability
     * 2. Call appropriate handler to build next frame
     * 3. Handler builds frame and updates payload index
     * 4. Handler transmits frame
     * 5. Check if entire payload transmitted (index == payload_count)
     * 6. If more frames needed, repeat from step 1
     * 7. If transmission complete, return success
     *
     * The state machine ensures:
     * - Frames transmitted in correct sequence (FIRST→MIDDLE(s)→LAST)
     * - No interruption by same or lower priority messages
     * - Proper framing bit encoding in each frame
     * - Complete payload transmission before returning
     *
     * Working Buffer Strategy:
     * The state machine uses a working CAN buffer (can_msg_worker) for frame construction:
     * - Allocated on stack in send_openlcb_message
     * - Reused for each frame in multi-frame sequence
     * - Avoids dynamic allocation overhead
     * - Cleared and rebuilt for each frame
     *
     * Error Handling:
     * Transmission failures are handled at multiple levels:
     *
     * Hardware buffer full:
     * - is_tx_buffer_empty returns false
     * - State machine returns immediately without attempting transmission
     * - Caller can retry later (typically in main loop)
     *
     * Handler failure:
     * - Handler returns false if unable to transmit frame
     * - Payload index not updated on failure
     * - State machine can retry same frame on next call
     *
     * Partial transmission:
     * - Multi-frame sequence interrupted by handler failure
     * - Payload index indicates how much was transmitted
     * - Receiver may timeout waiting for remaining frames
     * - Generally indicates serious hardware problem
     *
     * All 6 callbacks are REQUIRED and must be provided. Five handlers convert different
     * OpenLCB message types to CAN frames, and one callback checks hardware buffer status.
     *
     * Integration with CAN Main State Machine:
     * The CAN main state machine calls CanTxStatemachine_send_can_message for login frames
     * and CanTxStatemachine_send_openlcb_message for protocol messages. This provides a
     * single entry point for all outgoing CAN traffic.
     *
     * @note All 6 callbacks are REQUIRED - none can be NULL
     * @note State machine handles complete multi-frame sequences atomically
     * @note Buffer availability checked before each frame transmission
     *
     * @see CanTxStatemachine_initialize
     * @see CanTxStatemachine_send_openlcb_message
     * @see CanTxStatemachine_send_can_message
     */
    typedef struct {

        /**
         * @brief Callback to check hardware CAN transmit buffer availability
         *
         * @details This required callback queries the hardware CAN controller to determine
         * if the transmit buffer can accept another frame. Called before every frame
         * transmission attempt to prevent buffer overflow and ensure reliable transmission.
         *
         * The callback should:
         * - Read hardware transmit buffer status register/flag
         * - Return true if transmit buffer is empty/available
         * - Return false if transmit buffer is full/busy
         *
         * Common hardware implementations:
         *
         * Microcontroller CAN peripheral:
         * - Check TXBnIF flag (transmit buffer n interrupt flag)
         * - Check TXREQn bit (transmit request pending)
         * - Return true if buffer ready for new frame
         *
         * External CAN controller (MCP2515 via SPI):
         * - Read transmit buffer control register
         * - Check TXREQ bit status
         * - Return true if buffer available
         *
         * CAN driver library:
         * - Call library function to query buffer status
         * - Return library's buffer availability status
         *
         * Why buffer checking is critical:
         * - CAN controllers typically have 1-3 transmit buffers
         * - Attempting to write to full buffer corrupts data
         * - Full buffer indicates frames being transmitted slower than queued
         * - Allows caller to defer transmission and retry later
         *
         * Typical buffer states:
         * - Empty: Previous frame transmitted and acknowledged
         * - Busy: Frame currently being transmitted on bus
         * - Full: Frame loaded and waiting for bus arbitration
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Called before every frame transmission
         * @note Critical for preventing buffer overflow and lost frames
         */
        bool (*is_tx_buffer_empty)(void);

        /**
         * @brief Callback to convert addressed OpenLCB messages to CAN frames
         *
         * @details This required callback converts OpenLCB addressed messages to CAN frame
         * format with destination alias encoding. Handles multi-frame fragmentation when
         * message payload exceeds 6 bytes (due to 2-byte destination alias overhead).
         *
         * The callback should:
         * - Build CAN frame with proper header (MTI, source alias)
         * - Encode destination alias with framing flags in bytes 0-1
         * - Copy up to 6 bytes of payload data starting at current index
         * - Transmit frame via transmit_can_frame
         * - Update payload index if transmission successful
         * - Return true on success, false on failure
         *
         * Typical implementation: CanTxMessageHandler_addressed_msg_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Handles multi-frame sequences automatically
         * @note Index updated only on successful transmission
         */
        bool (*handle_addressed_msg_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /**
         * @brief Callback to convert unaddressed OpenLCB messages to CAN frames
         *
         * @details This required callback converts broadcast OpenLCB messages to CAN frame
         * format without destination alias. All 8 payload bytes available for OpenLCB data.
         *
         * The callback should:
         * - Build CAN frame with proper header (MTI, source alias)
         * - Copy up to 8 bytes of payload data starting at current index
         * - Transmit frame via transmit_can_frame
         * - Update payload index if transmission successful
         * - Return true on success, false on failure
         *
         * Note: Multi-frame unaddressed messages not currently implemented
         *
         * Typical implementation: CanTxMessageHandler_unaddressed_msg_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Currently supports single-frame messages only (≤8 bytes)
         */
        bool (*handle_unaddressed_msg_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /**
         * @brief Callback to convert datagram OpenLCB messages to CAN frames
         *
         * @details This required callback converts datagram messages (up to 72 bytes) to
         * CAN datagram frame format with proper type indicators. Handles fragmentation
         * across multiple frames.
         *
         * The callback should:
         * - Build CAN datagram frame with proper header
         * - Set datagram frame type bits (only/first/middle/last)
         * - Copy up to 8 bytes of payload data per frame
         * - Transmit frame via transmit_can_frame
         * - Update payload index if transmission successful
         * - Return true on success, false on failure
         *
         * Typical implementation: CanTxMessageHandler_datagram_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Handles complete datagram fragmentation automatically
         * @note Maximum 72 bytes per datagram on CAN
         */
        bool (*handle_datagram_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /**
         * @brief Callback to convert stream OpenLCB messages to CAN frames
         *
         * @details This required callback converts stream messages to CAN stream frame
         * format for high-throughput data transfer. Coordinates with stream flow control.
         *
         * The callback should:
         * - Build CAN stream frame with proper header
         * - Handle stream initiation, data, and completion frames
         * - Coordinate with flow control mechanism
         * - Transmit frame via transmit_can_frame
         * - Update payload index if transmission successful
         * - Return true on success, false on failure
         *
         * Note: Stream protocol not fully implemented - placeholder
         *
         * Typical implementation: CanTxMessageHandler_stream_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Currently placeholder - requires full implementation
         */
        bool (*handle_stream_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /**
         * @brief Callback to transmit pre-constructed CAN frames
         *
         * @details This required callback transmits raw CAN frames without any OpenLCB
         * message processing. Used for CAN control frames and direct bus operations.
         *
         * The callback should:
         * - Transmit frame as-is via transmit_can_frame
         * - No frame construction or modification
         * - No payload processing
         * - Return true on success, false on failure
         *
         * Used for:
         * - CID frames during alias allocation
         * - RID (Reserve ID) frames
         * - AMD (Alias Map Definition) frames
         * - AME responses
         * - Direct CAN control operations
         *
         * Typical implementation: CanTxMessageHandler_can_frame
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note No OpenLCB processing - raw CAN transmission
         * @note Frame must be fully constructed before calling
         */
        bool (*handle_can_frame)(can_msg_t *can_msg);

    } interface_can_tx_statemachine_t;


    /**
     * @brief Initializes the CAN transmit state machine
     *
     * @details Registers the application's callback interface with the transmit state machine.
     * The interface provides handlers for all message types and hardware buffer status checking.
     * Must be called during application startup before any CAN transmission operations.
     *
     * Use cases:
     * - Called once during application initialization
     * - Required before any OpenLCB message or CAN frame transmission
     *
     * @param interface_can_tx_statemachine Pointer to interface structure containing all
     * required callback function pointers
     *
     * @warning interface_can_tx_statemachine must remain valid for lifetime of application
     * @warning All 6 required callbacks must be valid (non-NULL)
     * @warning MUST be called during application initialization before transmission begins
     * @warning NOT thread-safe - call only from main initialization context
     *
     * @attention Call after CanTxMessageHandler_initialize
     * @attention Call before any network traffic starts
     * @attention All required function pointers in interface must be initialized
     *
     * @see interface_can_tx_statemachine_t - Interface structure definition
     * @see CanTxMessageHandler_initialize - Initialize message handlers first
     */
    extern void CanTxStatemachine_initialize(const interface_can_tx_statemachine_t *interface_can_tx_statemachine);


    /**
     * @brief Transmits an OpenLCB message on the CAN physical layer
     *
     * @details Converts an OpenLCB message to one or more CAN frames and transmits them
     * sequentially as an atomic operation. Checks hardware buffer availability before each
     * frame transmission. Handles multi-frame messages by iterating through complete payload
     * until all data transmitted. Dispatches to appropriate handler based on message type.
     *
     * Transmission Flow:
     * 1. Check if hardware transmit buffer is empty via is_tx_buffer_empty
     * 2. If buffer full, return false immediately (caller can retry)
     * 3. Determine message type (addressed, unaddressed, datagram, stream)
     * 4. Initialize payload index to 0
     * 5. Loop while payload index < payload count:
     *    a. Build next CAN frame via appropriate handler
     *    b. Handler transmits frame and updates index
     *    c. If handler fails, return false (partial transmission)
     * 6. When entire payload transmitted, return true
     *
     * Message Type Detection:
     * - Addressed: dest_alias != 0 AND MTI has address bit set
     * - Datagram: MTI indicates datagram protocol
     * - Stream: MTI indicates stream protocol
     * - Unaddressed: All others (broadcast messages)
     *
     * Multi-Frame Transmission:
     * Messages requiring multiple frames are transmitted as atomic sequences:
     * - All frames transmitted consecutively
     * - No other messages can interrupt at same/lower priority
     * - Receiver assembles frames using framing bits
     * - Index tracks progress through payload
     *
     * Use cases:
     * - Sending any OpenLCB protocol message
     * - Transmitting events, datagrams, configuration data
     * - Broadcasting node status information
     *
     * @param openlcb_msg Pointer to OpenLCB message to transmit
     *
     * @return True if message fully transmitted, false if hardware buffer full or transmission failed
     *
     * @warning openlcb_msg must NOT be NULL
     * @warning Returns false immediately if transmit buffer not empty - caller must retry
     * @warning Blocks until entire multi-frame message transmitted or failure occurs
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention Multi-frame messages sent as atomic sequence - no interruption by same/lower priority
     * @attention Message must have valid MTI and addressing information
     * @attention Partial transmission possible if handler fails mid-sequence
     *
     * @note Caller should retry if returns false due to buffer full
     * @note Multi-frame transmission may block for several milliseconds
     *
     * @see CanTxStatemachine_send_can_message - For raw CAN frames
     * @see CanTxMessageHandler_addressed_msg_frame - Addressed message handler
     * @see interface_can_tx_statemachine_t::is_tx_buffer_empty - Buffer check
     */
    extern bool CanTxStatemachine_send_openlcb_message(openlcb_msg_t *openlcb_msg);


    /**
     * @brief Transmits a raw CAN frame on the physical layer
     *
     * @details Sends a pre-constructed CAN frame directly to the physical CAN bus without
     * any OpenLCB message processing or conversion. Used for CAN control frames during
     * alias allocation and other low-level CAN operations that don't involve OpenLCB
     * message structures.
     *
     * The function:
     * - Calls handle_can_frame with the pre-built frame
     * - Handler transmits frame via transmit_can_frame
     * - Returns transmission success status
     *
     * No buffer availability check performed - caller responsible for ensuring buffer
     * available if needed. Typically used in contexts where buffer known to be available
     * or caller handles retry logic.
     *
     * Use cases:
     * - Transmitting CID frames during alias allocation (CID7, CID6, CID5, CID4)
     * - Transmitting RID (Reserve ID) frame
     * - Transmitting AMD (Alias Map Definition) frame
     * - Sending AME (Alias Map Enquiry) responses
     * - Direct hardware-level CAN operations
     * - Low-level protocol testing
     *
     * @param can_msg Pointer to CAN message buffer containing frame to transmit
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning can_msg must NOT be NULL
     * @warning Frame must be fully constructed before calling
     * @warning No buffer availability check performed - caller responsible
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention No OpenLCB processing - raw CAN transmission only
     * @attention CAN identifier must be properly encoded
     * @attention Payload and payload count must be valid
     *
     * @note Primarily used for CAN control frames during login
     * @note Caller should check buffer availability if needed
     *
     * @see CanTxStatemachine_send_openlcb_message - For OpenLCB messages
     * @see CanLoginMessageHandler_state_load_cid07 - Builds CID frames
     * @see CanLoginMessageHandler_state_load_rid - Builds RID frame
     * @see CanLoginMessageHandler_state_load_amd - Builds AMD frame
     */
    extern bool CanTxStatemachine_send_can_message(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TX_STATEMACHINE__ */
