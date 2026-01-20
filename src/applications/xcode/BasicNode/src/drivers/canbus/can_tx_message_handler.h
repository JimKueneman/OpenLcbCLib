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
 * @file can_tx_message_handler.h
 * @brief Message handlers for CAN transmit operations
 *
 * @details Provides handlers for converting OpenLCB messages to CAN frames and
 * transmitting them on the physical CAN bus. Handles multi-frame message
 * fragmentation for addressed messages, unaddressed messages, datagrams, and streams.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_TX_MESSAGE_HANDLER__
#define __DRIVERS_CANBUS_CAN_TX_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

    /**
     * @brief Interface structure for CAN transmit message handler callback functions
     *
     * @details This structure defines the callback interface for CAN transmit message handlers,
     * which convert OpenLCB messages to CAN frames and coordinate transmission to the physical
     * CAN bus. The handlers manage multi-frame message fragmentation and ensure proper framing
     * bit encoding for message reassembly at the receiving end.
     *
     * The CAN Tx message handlers perform the following key operations:
     *
     * Message Type-Specific Fragmentation:
     * Different OpenLCB message types require different fragmentation strategies:
     *
     * 1. Addressed Messages:
     *    - Include 12-bit destination alias in first 2 bytes of each frame
     *    - Leaves 6 bytes per frame for OpenLCB payload data
     *    - Used for: Protocol Support Inquiry, Verify Node ID, targeted commands
     *    - Handler: CanTxMessageHandler_addressed_msg_frame
     *
     * 2. Unaddressed Messages:
     *    - No destination alias (broadcast to all nodes)
     *    - Full 8 bytes per frame available for payload
     *    - Used for: Initialization Complete, Event Reports, Verified Node ID
     *    - Handler: CanTxMessageHandler_unaddressed_msg_frame
     *    - Note: Multi-frame unaddressed currently not implemented
     *
     * 3. Datagram Messages:
     *    - Up to 72 bytes maximum payload
     *    - Uses datagram frame format with specific frame type encoding
     *    - Used for: Memory Configuration, Remote Button, Display protocols
     *    - Handler: CanTxMessageHandler_datagram_frame
     *
     * 4. Stream Messages:
     *    - High-throughput continuous data transfer
     *    - Used for: Firmware upgrades, large file transfers
     *    - Handler: CanTxMessageHandler_stream_frame
     *    - Note: Currently placeholder - not fully implemented
     *
     * 5. Direct CAN Frames:
     *    - Pre-built CAN frames (no OpenLCB processing)
     *    - Used for: CID, RID, AMD control frames
     *    - Handler: CanTxMessageHandler_can_frame
     *
     * Framing Bit Encoding:
     * Multi-frame messages use framing flags in the first payload byte to indicate
     * frame position in sequence:
     *
     * For Addressed Messages (destination alias in bytes 0-1):
     * - Byte 0 bits 7-6: Framing flags
     * - Byte 0 bits 5-4: Reserved (typically 11)
     * - Byte 0 bits 3-0 + Byte 1: 12-bit destination alias
     *
     * Framing Flag Values:
     * - 00 (MULTIFRAME_ONLY): Complete message in one frame
     * - 01 (MULTIFRAME_FIRST): First frame of multi-frame sequence
     * - 10 (MULTIFRAME_LAST): Last frame of multi-frame sequence
     * - 11 (MULTIFRAME_MIDDLE): Middle frame(s) of sequence
     *
     * Frame Sequence Rules:
     * - Single-frame: ONLY flag, 0-8 bytes payload
     * - Multi-frame addressed: FIRST (6 bytes) → MIDDLE(s) (6 bytes each) → LAST (0-6 bytes)
     * - Multi-frame global: FIRST (8 bytes) → MIDDLE(s) (8 bytes each) → LAST (0-8 bytes)
     * - First and middle frames must contain maximum data
     * - Last frame contains remaining data (may be 0 bytes)
     *
     * Payload Index Management:
     * Handlers maintain a payload index tracking the current position in the OpenLCB message:
     * - Index passed by pointer to allow handler to update after each frame
     * - On successful transmission, index advanced by bytes transmitted
     * - On failed transmission, index unchanged (caller can retry)
     * - Caller checks index against total payload to determine completion
     *
     * Hardware Interface Integration:
     * The transmit_can_frame callback provides the interface to hardware CAN controller:
     * - Called after frame is fully constructed
     * - Pre-checked by Tx state machine via is_tx_buffer_empty
     * - Expected to succeed unless hardware failure
     * - Returns true on success, false on failure
     *
     * Transmission Flow:
     * 1. CAN Tx state machine checks is_tx_buffer_empty
     * 2. If buffer available, calls appropriate handler for message type
     * 3. Handler builds CAN frame with proper header and framing
     * 4. Handler copies appropriate payload chunk
     * 5. Handler calls transmit_can_frame
     * 6. If successful, handler updates payload index
     * 7. If failed, handler returns false, index unchanged for retry
     * 8. Handler invokes on_transmit callback if provided
     *
     * Optional Transmission Notification:
     * The on_transmit callback allows applications to be notified after successful
     * transmission for:
     * - Logging transmitted frames
     * - Incrementing statistics counters
     * - Activity indicators (LEDs)
     * - Protocol analyzers
     * - Debug monitoring
     *
     * Only 1 required callback (transmit_can_frame) must be provided. The on_transmit
     * callback is optional and can be NULL if notification is not needed.
     *
     * @note transmit_can_frame is REQUIRED - must not be NULL
     * @note on_transmit is Optional - can be NULL if notification not needed
     * @note Handlers called from Tx state machine context
     * @note All handlers return success/failure status
     *
     * @see CanTxMessageHandler_initialize
     * @see can_tx_statemachine.h - Invokes these handlers
     */
typedef struct {

        /**
         * @brief Callback to transmit CAN frame to physical bus
         *
         * @details This required callback writes a fully constructed CAN frame to the hardware
         * CAN controller and initiates transmission. The CAN Tx state machine pre-checks buffer
         * availability via is_tx_buffer_empty before calling this function, so transmission
         * is expected to succeed unless a hardware error occurs.
         *
         * The callback receives a complete CAN frame containing:
         * - 29-bit extended CAN identifier with proper bit encoding
         * - 0-8 payload data bytes
         * - Payload byte count
         *
         * The callback should:
         * - Write CAN identifier to controller ID registers
         * - Write payload bytes to controller data registers
         * - Write payload count to controller DLC (Data Length Code)
         * - Set transmit request bit to initiate transmission
         * - Return true if initiated successfully, false on hardware error
         *
         * Common hardware implementations:
         * - Microcontroller CAN peripheral: Write to CAN registers, set TXREQx bit
         * - External CAN controller (MCP2515): SPI write to TX buffer, send transmit command
         * - CAN driver library: Call library transmit function
         *
         * Hardware errors (rare since buffer pre-checked):
         * - CAN controller offline or in error state
         * - Bus-off condition
         * - Transmit error counter exceeded
         * - Controller reset/failure
         *
         * Typical implementation:
         * - Direct register writes for embedded MCU CAN peripheral
         * - CAN driver library call for abstracted hardware
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Buffer availability pre-checked - failures should be rare
         */
        bool (*transmit_can_frame)(can_msg_t *can_msg);

        /**
         * @brief Optional callback for transmission notification
         *
         * @details This optional callback provides immediate notification after successful
         * CAN frame transmission. Called after transmit_can_frame returns true, allowing
         * applications to monitor, log, or react to transmitted frames.
         *
         * The callback receives the CAN frame that was just transmitted, allowing inspection
         * of identifier, payload, and payload count for logging or analysis.
         *
         * Common uses:
         * - Logging: Write frame to file, console, or network
         * - Statistics: Increment transmission counters, calculate throughput
         * - Activity indicators: Toggle LEDs, update displays
         * - Protocol analyzers: Forward to monitoring tools
         * - Debug monitoring: Print frame details for debugging
         * - Timestamping: Record transmission time for performance analysis
         *
         * The callback should:
         * - Execute very quickly (microseconds preferred)
         * - Avoid blocking operations
         * - Not call functions that could fail and require retry
         * - Consider queuing data for background processing if needed
         *
         * @note Optional - can be NULL if notification is not needed
         * @note Called in transmission path - keep processing minimal
         * @note Avoid lengthy operations that delay subsequent transmissions
         */
        void (*on_transmit)(can_msg_t *can_msg);

} interface_can_tx_message_handler_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN transmit message handler module
     *
     * @details Registers the application's callback interface with the transmit message
     * handlers. The interface provides the hardware transmission function and optional
     * notification callback. Must be called during application startup before any CAN
     * transmission occurs.
     *
     * Use cases:
     * - Called once during application initialization
     * - Required before any CAN message transmission
     *
     * @param interface_can_tx_message_handler Pointer to interface structure containing
     * required transmit_can_frame callback and optional on_transmit callback
     *
     * @warning interface_can_tx_message_handler must remain valid for lifetime of application
     * @warning transmit_can_frame callback must be valid (non-NULL)
     * @warning MUST be called during application initialization before any transmit operations
     * @warning NOT thread-safe - call only from main initialization context
     *
     * @attention Call after CAN hardware initialization but before CAN traffic begins
     * @attention Call before CanTxStatemachine_initialize
     *
     * @see interface_can_tx_message_handler_t - Interface structure definition
     * @see CanTxStatemachine_initialize - Initialize transmit state machine
     */
    extern void CanTxMessageHandler_initialize(const interface_can_tx_message_handler_t *interface_can_tx_message_handler);


    /**
     * @brief Converts and transmits an addressed OpenLCB message as CAN frame(s)
     *
     * @details Handles fragmentation of addressed OpenLCB messages into one or more CAN frames
     * with proper framing bit encoding. Addressed messages include a 12-bit destination alias
     * in the first 2 bytes of each frame, leaving 6 bytes per frame for payload data.
     *
     * For messages ≤6 bytes:
     * - Single frame with MULTIFRAME_ONLY (00) framing flags
     * - Bytes 0-1: Destination alias with framing bits
     * - Bytes 2-7: OpenLCB payload (up to 6 bytes)
     *
     * For messages >6 bytes:
     * - FIRST frame: Bytes 0-1 destination, Bytes 2-7 first 6 payload bytes
     * - MIDDLE frame(s): Bytes 0-1 destination, Bytes 2-7 next 6 payload bytes each
     * - LAST frame: Bytes 0-1 destination, Bytes 2-X remaining payload (0-6 bytes)
     *
     * Use cases:
     * - Sending Protocol Support Inquiry to specific node
     * - Sending Verify Node ID to specific node
     * - Sending any message requiring destination address
     *
     * @param openlcb_msg Pointer to OpenLCB message to transmit (must have dest_alias set)
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in OpenLCB payload (updated after successful transmission)
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning openlcb_msg must NOT be NULL
     * @warning can_msg_worker must NOT be NULL
     * @warning openlcb_start_index must NOT be NULL
     * @warning Transmission failure leaves payload index unchanged - caller must retry
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention dest_alias in openlcb_msg must be valid (0x001-0xFFF)
     * @attention First two payload bytes reserved for destination alias in all frames
     * @attention Multi-frame messages use framing flags: only/first/middle/last
     * @attention Index is only updated on successful transmission
     *
     * @note Caller checks if (*openlcb_start_index == payload_count) to detect completion
     * @note May need multiple calls to transmit complete multi-frame message
     *
     * @see CanTxMessageHandler_unaddressed_msg_frame - For broadcast messages
     * @see CanUtilities_copy_openlcb_payload_to_can_payload - Payload copying helper
     */
    extern bool CanTxMessageHandler_addressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Converts and transmits an unaddressed OpenLCB message as CAN frame(s)
     *
     * @details Handles transmission of broadcast (unaddressed) OpenLCB messages that are
     * received by all nodes on the network. These messages do not include a destination
     * alias, allowing all 8 bytes of CAN frame payload for OpenLCB data.
     *
     * Currently supports single-frame messages only:
     * - Bytes 0-7: OpenLCB payload (up to 8 bytes)
     * - No framing bits required for single-frame
     *
     * Use cases:
     * - Broadcasting Initialization Complete
     * - Broadcasting Producer/Consumer Event Reports
     * - Broadcasting Verified Node ID
     *
     * @param openlcb_msg Pointer to OpenLCB message to transmit (no dest_alias required)
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in OpenLCB payload (updated after successful transmission)
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning openlcb_msg must NOT be NULL
     * @warning can_msg_worker must NOT be NULL
     * @warning openlcb_start_index must NOT be NULL
     * @warning Multi-frame unaddressed messages not currently implemented
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention All 8 payload bytes available (no destination alias overhead)
     * @attention Messages >8 bytes will fail - check payload_count before calling
     *
     * @note Most broadcast messages fit in single frame (events, status reports)
     *
     * @see CanTxMessageHandler_addressed_msg_frame - For targeted messages
     */
    extern bool CanTxMessageHandler_unaddressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Converts and transmits a datagram OpenLCB message as CAN frame(s)
     *
     * @details Handles fragmentation of datagram messages (up to 72 bytes maximum) into
     * multiple CAN frames using datagram frame format. Datagrams are used by protocols
     * that need to transfer more data than fits in basic messages but don't require the
     * high throughput of streams.
     *
     * Frame sequence for datagrams:
     * - If ≤8 bytes: Single ONLY frame
     * - If >8 bytes: FIRST frame → MIDDLE frame(s) → LAST frame
     *
     * Each frame carries maximum payload:
     * - FIRST frame: 8 bytes
     * - MIDDLE frames: 8 bytes each
     * - LAST frame: Remaining bytes (1-8)
     *
     * Use cases:
     * - Sending Memory Configuration Protocol requests/replies
     * - Sending Remote Button Protocol commands
     * - Transmitting Configuration Definition Info (CDI)
     * - Sending any datagram-based protocol data
     *
     * @param openlcb_msg Pointer to OpenLCB datagram message to transmit
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in datagram payload (updated after successful transmission)
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning openlcb_msg must NOT be NULL
     * @warning can_msg_worker must NOT be NULL
     * @warning openlcb_start_index must NOT be NULL
     * @warning Maximum datagram size is 72 bytes on CAN transport
     * @warning Transmission failure leaves payload index unchanged
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention Frame sequence: only OR first→middle(s)→last
     * @attention All frames carry maximum 8 bytes except possibly last frame
     * @attention Datagrams require Datagram Received OK/Rejected acknowledgment
     *
     * @note Caller must check for Datagram OK/Rejected after complete transmission
     * @note May require multiple calls to transmit complete datagram
     *
     * @see CanTxMessageHandler_stream_frame - For streaming data
     * @see protocol_datagram_handler.h - Datagram protocol details
     */
    extern bool CanTxMessageHandler_datagram_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Converts and transmits a stream OpenLCB message as CAN frame(s)
     *
     * @details Handles transmission of streaming data messages for high-throughput
     * continuous data transfer. Stream protocol is designed for applications requiring
     * efficient transfer of large amounts of data, such as firmware upgrades.
     *
     * Stream protocol features (when fully implemented):
     * - Flow control for preventing receiver buffer overflow
     * - High-throughput continuous transfer
     * - Error detection and recovery
     * - Progress monitoring
     *
     * Use cases (future):
     * - Firmware upgrade data transfer
     * - Large configuration file transfers
     * - Continuous sensor data streaming
     * - Log file downloads
     *
     * @param openlcb_msg Pointer to OpenLCB stream message to transmit
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in stream payload (updated after successful transmission)
     *
     * @return Currently always returns true (placeholder implementation)
     *
     * @warning openlcb_msg must NOT be NULL
     * @warning can_msg_worker must NOT be NULL
     * @warning openlcb_start_index must NOT be NULL
     * @warning Stream protocol NOT fully implemented - placeholder only
     * @warning Do not rely on this function for production stream transfers
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention Function requires full implementation before production use
     * @attention Stream protocol complex - requires flow control, error handling
     *
     * @note Placeholder returns success without transmitting
     *
     * @see CanTxMessageHandler_datagram_frame - For datagram transfers
     */
    extern bool CanTxMessageHandler_stream_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Transmits a pre-built CAN frame to the physical bus
     *
     * @details Transmits a fully constructed CAN frame without any OpenLCB message processing
     * or fragmentation. Used for CAN control frames and other low-level CAN operations that
     * don't involve OpenLCB message conversion.
     *
     * The frame must be completely built before calling:
     * - CAN identifier fully populated with correct bits
     * - Payload bytes filled
     * - Payload count set correctly
     *
     * Use cases:
     * - Transmitting CID frames during alias allocation (CID7, CID6, CID5, CID4)
     * - Transmitting RID (Reserve ID) frame
     * - Transmitting AMD (Alias Map Definition) frame
     * - Sending AME (Alias Map Enquiry) responses
     * - Direct CAN bus operations
     * - Low-level protocol testing
     *
     * @param can_msg Pointer to CAN message buffer containing frame to transmit
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning can_msg must NOT be NULL
     * @warning Frame must be fully constructed before calling
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention No OpenLCB processing performed - raw CAN transmission
     * @attention No framing bit handling - transmits frame as-is
     * @attention Caller responsible for correct frame construction
     *
     * @note Used primarily for CAN control frames during login
     *
     * @see CanLoginMessageHandler_state_load_cid07 - Builds CID frames
     * @see CanLoginMessageHandler_state_load_rid - Builds RID frame
     * @see CanLoginMessageHandler_state_load_amd - Builds AMD frame
     */
    extern bool CanTxMessageHandler_can_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TX_MESSAGE_HANDLER__ */
