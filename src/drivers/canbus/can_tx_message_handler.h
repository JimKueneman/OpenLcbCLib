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
 * @file can_tx_message_handler.h
 * @brief Message handlers for CAN transmit operations
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
 * @brief A structure to hold pointers to functions for dependencies this module requires, \ref can_tx_message_handler.h
 * @details OpenLcbCLib uses dependency injection to allow for writing full coverage tests as the
 * functions that are used can be modeled in the test and return valid OR invalid results to fully
 * test all program flows in the module.  It also allows for reducing the program size. If a particular
 * protocol does not need to be implemented simply filling in the dependency for that handler with a NULL
 * will strip out code for that protocols handlers and minimize the application size (bootloader is an example).
 * The library will automatically reply with the correct error/reply codes if the handler is defined as NULL.
 */
typedef struct {
    /*@{*/

    // REQUIRED FUNCTIONS

    /** @brief Pointer to an Application defined function to put the passed CAN frame onto the physical CAN bus.  The
     * library will have called the Application defined function is_tx_buffer_empty before calling this function
     * is expected to succeeded in transmitting the frame unless there is problem.
     * @warning <b>Required</b> assignment.  */
    bool (*transmit_can_frame)(can_msg_t *can_msg);

    // OPTIONAL FUNCTION

    // CALLBACK FUNCTIONS

    /** @brief Pointer to a function for an Application to be notified when a CAN frame is transmitted
     *@note <b>Optional</b> application callback.  Defaults to NULL. */
    void (*on_transmit)(can_msg_t *can_msg);

    /*@}*/

} interface_can_tx_message_handler_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Transmit (Tx) state machine handlers
     * @param interface_can_tx_message_handler Pointer to a
     * interface_can_tx_message_handler_t struct containing the functions that this module requires.
     * @return None
     * @attention This must always be called during application initialization
     */
    extern void CanTxMessageHandler_initialize(const interface_can_tx_message_handler_t *interface_can_tx_message_handler);


    /**
     * @brief Sends an addressed OpenLcb/LCC message on the CAN physical layer
     * @param openlcb_msg Pointer to the OpenLcb/LCC message to transmit
     * @param can_msg_worker Pointer to a CAN frame buffer to use to complete the transmit (could need to be multiple frames)
     * @param openlcb_start_index Were in the OpenLcb Payload does the data start
     * @return None
     */
    extern bool CanTxMessageHandler_addressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Sends an unaddressed OpenLcb/LCC message on the CAN physical layer
     * @param openlcb_msg Pointer to the OpenLcb/LCC message to transmit
     * @param can_msg_worker Pointer to a CAN frame buffer to use to complete the transmit (could need to be multiple frames)
     * @param openlcb_start_index Were in the OpenLcb Payload does the data start
     * @return None
     */
    extern bool CanTxMessageHandler_unaddressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Sends a datagram OpenLcb/LCC message on the CAN physical layer
     * @param openlcb_msg Pointer to the OpenLcb/LCC message to transmit
     * @param can_msg_worker Pointer to a CAN frame buffer to use to complete the transmit (could need to be multiple frames)
     * @param openlcb_start_index Were in the OpenLcb Payload does the data start
     * @return None
     */
    extern bool CanTxMessageHandler_datagram_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Sends a stream OpenLcb/LCC message on the CAN physical layer
     * @param openlcb_msg Pointer to the OpenLcb/LCC message to transmit
     * @param can_msg_worker Pointer to a CAN frame buffer to use to complete the transmit (could need to be multiple frames)
     * @param openlcb_start_index Were in the OpenLcb Payload does the data start
     * @return None
     */
    extern bool CanTxMessageHandler_stream_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);


    /**
     * @brief Sends an CAN message on the CAN physical layer
     * @param can_msg_worker Pointer to a CAN buffer message to transmit
     * @return None
     */
    extern bool CanTxMessageHandler_can_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TX_MESSAGE_HANDLER__ */
