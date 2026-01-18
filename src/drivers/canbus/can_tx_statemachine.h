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
     * @brief Dependency injection interface for CAN transmit state machine
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
         * @brief Pointer to Application-defined function that checks if CAN transmit buffer is available
         *
         * @details Called before attempting to transmit each CAN frame to ensure the hardware
         * transmit buffer can accept another frame. Prevents buffer overflow and lost frames.
         *
         * @warning <b>Required</b> assignment - application MUST provide this function
         */
        bool (*is_tx_buffer_empty)(void);


        /**
         * @brief Pointer to handler function for addressed OpenLCB messages
         *
         * @details Converts addressed OpenLCB messages to CAN frame format with destination
         * alias in payload. Handles multi-frame fragmentation as needed.
         *
         * @warning <b>Required</b> assignment - defaults to CanTxMessageHandler_addressed_msg_frame()
         */
        bool (*handle_addressed_msg_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /**
         * @brief Pointer to handler function for unaddressed OpenLCB messages
         *
         * @details Converts broadcast OpenLCB messages to CAN frame format. No destination
         * alias required in payload.
         *
         * @warning <b>Required</b> assignment - defaults to CanTxMessageHandler_unaddressed_msg_frame()
         */
        bool (*handle_unaddressed_msg_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /**
         * @brief Pointer to handler function for datagram OpenLCB messages
         *
         * @details Converts datagram messages to CAN datagram frame format. Handles
         * fragmentation of datagrams up to 72 bytes across multiple frames.
         *
         * @warning <b>Required</b> assignment - defaults to CanTxMessageHandler_datagram_frame()
         */
        bool (*handle_datagram_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /**
         * @brief Pointer to handler function for stream OpenLCB messages
         *
         * @details Converts stream messages to CAN stream frame format for high-throughput
         * data transfer applications.
         *
         * @warning <b>Required</b> assignment - defaults to CanTxMessageHandler_stream_frame()
         */
        bool (*handle_stream_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);

        /**
         * @brief Pointer to handler function for raw CAN frames
         *
         * @details Transmits pre-constructed CAN frames without OpenLCB message processing.
         * Used for CAN control frames and low-level bus operations.
         *
         * @warning <b>Required</b> assignment - defaults to CanTxMessageHandler_can_frame()
         */
        bool (*handle_can_frame)(can_msg_t *can_msg);

        // OPTIONAL FUNCTION

        // CALLBACK FUNCTIONS

        /*@}*/

    } interface_can_tx_statemachine_t;


    /**
     * @brief Initializes the CAN transmit state machine
     *
     * @details Stores the dependency injection interface pointer for use by the state machine.
     * Must be called during application startup before any CAN transmission operations.
     *
     * Use cases:
     * - Called once during application initialization
     * - Required before any OpenLCB message or CAN frame transmission
     *
     * @param interface_can_tx_statemachine Pointer to interface structure containing required function pointers
     *
     * @warning MUST be called during application initialization before transmission begins
     * @warning NOT thread-safe - call only from main initialization context
     *
     * @attention Call after handler modules initialized but before network traffic starts
     *
     * @see CanTxMessageHandler_initialize - Initialize message handlers first
     */
    extern void CanTxStatemachine_initialize(const interface_can_tx_statemachine_t *interface_can_tx_statemachine);


    /**
     * @brief Transmits an OpenLCB message on the CAN physical layer
     *
     * @details Converts an OpenLCB message to one or more CAN frames and transmits them sequentially.
     * Checks hardware buffer availability before transmission. Handles multi-frame messages by
     * iterating until entire payload transmitted. Dispatches to appropriate handler based on
     * message type (addressed, unaddressed, datagram, stream).
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
     * @warning Returns false immediately if transmit buffer not empty - caller must retry
     * @warning Blocks until entire multi-frame message transmitted or failure occurs
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention Multi-frame messages sent as atomic sequence - no interruption by same/lower priority
     * @attention Message must have valid MTI and addressing information
     *
     * @see CanTxStatemachine_send_can_message - For raw CAN frames
     * @see CanTxMessageHandler_addressed_msg_frame - Addressed message handler
     */
    extern bool CanTxStatemachine_send_openlcb_message(openlcb_msg_t *openlcb_msg);


    /**
     * @brief Transmits a raw CAN frame on the physical layer
     *
     * @details Sends a pre-constructed CAN frame directly to the physical CAN bus without
     * OpenLCB message processing. Used for CAN control frames (CID, RID, AMD) and other
     * low-level CAN operations.
     *
     * Use cases:
     * - Transmitting alias allocation frames during node login
     * - Sending CAN control messages
     * - Direct hardware-level CAN operations
     *
     * @param can_msg Pointer to CAN message buffer containing frame to transmit
     *
     * @return True if frame transmitted successfully, false if transmission failed
     *
     * @warning Frame must be fully constructed before calling
     * @warning No buffer availability check performed - caller responsible
     * @warning NOT thread-safe - serialize calls from multiple contexts
     *
     * @attention No OpenLCB processing - raw CAN transmission only
     *
     * @see CanTxStatemachine_send_openlcb_message - For OpenLCB messages
     */
    extern bool CanTxStatemachine_send_can_message(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TX_STATEMACHINE__ */
