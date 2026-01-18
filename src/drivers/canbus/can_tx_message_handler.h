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
     * @brief Dependency injection interface for CAN transmit message handlers
     *
     * @details OpenLcbCLib uses dependency injection to allow for writing full coverage tests as the
     * functions that are used can be modeled in the test and return valid OR invalid results to fully
     * test all program flows in the module. It also allows for reducing the program size. If a particular
     * protocol does not need to be implemented simply filling in the dependency for that handler with a NULL
     * will strip out code for that protocols handlers and minimize the application size (bootloader is an example).
     * The library will automatically reply with the correct error/reply codes if the handler is defined as NULL.
     */
typedef struct {
    /*@{*/

    // REQUIRED FUNCTIONS

    /**
     * @brief Pointer to Application-defined function that transmits a CAN frame to the physical bus
     *
     * @details The library ensures the transmit buffer is empty before calling this function by
     * first calling the is_tx_buffer_empty function. This function is expected to succeed in
     * transmitting the frame unless there is a hardware problem.
     *
     * @warning <b>Required</b> assignment - application MUST provide this function
     */
    bool (*transmit_can_frame)(can_msg_t *can_msg);

    // OPTIONAL FUNCTION

    // CALLBACK FUNCTIONS

    /**
     * @brief Pointer to Application callback function notified when a CAN frame is transmitted
     *
     * @details Provides notification after successful CAN frame transmission. Useful for
     * logging, debugging, or triggering post-transmission actions.
     *
     * @note <b>Optional</b> application callback - defaults to NULL if not needed
     */
    void (*on_transmit)(can_msg_t *can_msg);

    /*@}*/

} interface_can_tx_message_handler_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN transmit message handler module
     *
     * @details Stores the dependency injection interface pointer for use by all handler functions.
     * Must be called during application startup before any CAN transmission occurs.
     *
     * Use cases:
     * - Called once during application initialization
     * - Required before any CAN message transmission
     *
     * @param interface_can_tx_message_handler Pointer to interface structure containing required function pointers
     *
     * @warning MUST be called during application initialization before any transmit operations
     * @warning NOT thread-safe - call only from main initialization context
     *
     * @attention Call after CAN hardware initialization but before CAN traffic begins
     *
     * @see CanTxStatemachine_initialize - Initialize transmit state machine
     */
    extern void CanTxMessageHandler_initialize(const interface_can_tx_message_handler_t *interface_can_tx_message_handler);


    /**
     * @brief Converts and transmits an addressed OpenLCB message as CAN frame(s)
     *
     * @details Handles fragmentation of addressed OpenLCB messages into one or more CAN frames.
     * Addressed messages include a 12-bit destination alias in the first two payload bytes.
     * For messages exceeding 6 bytes, fragments across multiple frames with proper framing flags.
     *
     * Use cases:
     * - Sending Protocol Support Inquiry to specific node
     * - Sending Verify Node ID to specific node
     * - Sending any message requiring destination address
     *
     * @param openlcb_msg Pointer to OpenLCB message to transmit (must have dest_alias set)
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in OpenLCB payload (updated after transmission)
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Transmission failure leaves payload index unchanged - caller must retry
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention First two payload bytes reserved for destination alias in all frames
     * @attention Multi-frame messages use framing flags: only/first/middle/last
     *
     * @see CanTxMessageHandler_unaddressed_msg_frame - For broadcast messages
     * @see CanUtilities_copy_openlcb_payload_to_can_payload - Payload copying helper
     */
    extern bool CanTxMessageHandler_addressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Converts and transmits an unaddressed OpenLCB message as CAN frame(s)
     *
     * @details Handles transmission of broadcast (unaddressed) OpenLCB messages. These messages
     * do not include a destination alias and are received by all nodes on the network.
     * Currently supports single-frame messages only.
     *
     * Use cases:
     * - Broadcasting Initialization Complete
     * - Broadcasting Producer/Consumer Event Reports
     * - Broadcasting Verified Node ID
     *
     * @param openlcb_msg Pointer to OpenLCB message to transmit (no dest_alias required)
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in OpenLCB payload (updated after transmission)
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Multi-frame unaddressed messages not currently implemented
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention All 8 payload bytes available (no destination alias overhead)
     *
     * @see CanTxMessageHandler_addressed_msg_frame - For targeted messages
     */
    extern bool CanTxMessageHandler_unaddressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Converts and transmits a datagram OpenLCB message as CAN frame(s)
     *
     * @details Handles fragmentation of datagram messages (up to 72 bytes) into CAN frames
     * using datagram frame format. Uses frame type indicators: only/first/middle/last.
     * Each frame carries up to 8 bytes of datagram payload.
     *
     * Use cases:
     * - Sending Memory Configuration Protocol requests
     * - Sending Remote Button Protocol commands
     * - Transmitting any datagram-based protocol data
     *
     * @param openlcb_msg Pointer to OpenLCB datagram message to transmit
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in datagram payload (updated after transmission)
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Maximum datagram size is 72 bytes on CAN transport
     * @warning Transmission failure leaves payload index unchanged
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention Frame sequence: only OR first→middle(s)→last
     * @attention All frames carry maximum 8 bytes except possibly last frame
     *
     * @see CanTxMessageHandler_stream_frame - For streaming data
     */
    extern bool CanTxMessageHandler_datagram_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Converts and transmits a stream OpenLCB message as CAN frame(s)
     *
     * @details Handles transmission of streaming data messages. Stream protocol allows
     * high-throughput data transfer for applications like firmware updates.
     * Implementation is currently incomplete.
     *
     * Use cases:
     * - Firmware upgrade data transfer
     * - Large file transfers
     * - Continuous data streaming
     *
     * @param openlcb_msg Pointer to OpenLCB stream message to transmit
     * @param can_msg_worker Pointer to working CAN frame buffer for building frames
     * @param openlcb_start_index Pointer to current position in stream payload (updated after transmission)
     *
     * @return Currently always returns true (placeholder implementation)
     *
     * @warning Stream protocol NOT fully implemented - placeholder only
     * @warning Do not rely on this function for production stream transfers
     *
     * @attention Function requires full implementation before use
     *
     * @see CanTxMessageHandler_datagram_frame - For datagram transfers
     */
    extern bool CanTxMessageHandler_stream_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Transmits a pre-built CAN frame to the physical bus
     *
     * @details Transmits a fully constructed CAN frame without any OpenLCB message processing.
     * Used for CAN control frames (CID, RID, AMD) and other low-level CAN operations.
     *
     * Use cases:
     * - Transmitting alias allocation frames during node login
     * - Sending CAN control messages
     * - Direct CAN bus operations
     *
     * @param can_msg Pointer to CAN message buffer containing frame to transmit
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Frame must be fully constructed before calling
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention No OpenLCB processing performed - raw CAN transmission
     *
     * @see CanTxMessageHandler_addressed_msg_frame - For OpenLCB messages
     */
    extern bool CanTxMessageHandler_can_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TX_MESSAGE_HANDLER__ */
