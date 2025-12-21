/*
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
 * 21 Dec 2025
 * Copyright (c) 2025, Jim Kueneman
 */

/**
 *
 * @brief Implements a state machine actually a large switch statement that decodes the 
 * incoming message and calls the correct the can_tx_message_handler.h handler functions.
 * 
 * This is the core transmit functionality of the CAN frames.  It either sends a CAN
 * message directly, which is a direct translation to the CAN physical layer or sends an
 * OpenLcb/LCC message which may require being broken up into multiple CAN frames.
 *
 * @file can_tx_statemachine.h
 *
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
     * @brief A structure to hold pointers to functions for dependencies this module requires, \ref can_tx_statemachine.h.
     *
     * @details OpenLcbCLib uses dependency injection to allow for writing full coverage tests as the
     * functions that are used can be modeled in the test and return valid OR invalid results to fully
     * test all program flows in the module.  It also allows for reducing the program size. If a particular
     * protocol does not need to be implemented simply filling in the dependency for that handler with a NULL
     * will strip out code for that protocols handlers and minimize the application size (bootloader is an example).
     * The library will automatically reply with the correct error/reply codes if the handler is defined as NULL
     */
    typedef struct {
        /*@{*/

        // REQUIRED FUNCTIONS

        /** Pointer to an Application defined function to ask if the hardware driver to send a CAN frame is empty and can take another frame to send.
         * @warning <b>Required</b> assignment. Application defined function */
        bool (*is_tx_buffer_empty)(void);


        /** Pointer to a function to handle an addressed OpenLcb/Lcc message to transmit.
         * @warning <b>Required</b> assignment.  Defaults to CanTxMessageHandler_addressed_msg_frame() */
        bool (*handle_addressed_msg_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);
        
        /** Pointer to a function to handle an unaddressed OpenLcb/Lcc message to transmit.
         * @warning <b>Required</b> assignment.  Defaults to CanTxMessageHandler_unaddressed_msg_frame() */
        bool (*handle_unaddressed_msg_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);
        
        /** Pointer to a function to handle a datagram frame OpenLcb/Lcc message to transmit.
         * @warning <b>Required</b> assignment.  Defaults to CanTxMessageHandler_datagram_frame() */
        bool (*handle_datagram_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);
        
        /** Pointer to a function to handle a stream frame OpenLcb/Lcc message to transmit.
         * @warning <b>Required</b> assignment.  Defaults to CanTxMessageHandler_stream_frame() */
        bool (*handle_stream_frame)(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index);
        
        /** Pointer to a function to handle a CAN message frame to transmit
         * @warning <b>Required</b> assignment.  Defaults to CanTxMessageHandler_can_frame() */
        bool (*handle_can_frame)(can_msg_t *can_msg);

        // OPTIONAL FUNCTION

        // CALLBACK FUNCTIONS

        /*@}*/

    } interface_can_tx_statemachine_t;

    
    /**
     * @brief Initializes the CAN Receive (Rx) state machine
     *
     * @param const interface_can_tx_statemachine_t *interface_can_tx_statemachine - Pointer to a
     * interface_can_tx_statemachine_t struct containing the functions that this module requires.
     *
     * @return none
     *
     * @attention This must always be called during application initialization.
     */
    extern void CanTxStatemachine_initialize(const interface_can_tx_statemachine_t *interface_can_tx_statemachine);

    
    /**
     * @brief Sends an OpenLcb/LCC message on the CAN physical layer
     *
     * @param openlcb_msg_t *openlcb_msg - Pointer to the OpenLcb/LCC message to transmit
     *
     * @return none
     *
     */
    extern bool CanTxStatemachine_send_openlcb_message(openlcb_msg_t *openlcb_msg);

    
    /**
     * @brief Sends an OpenLcb/LCC message on the CAN physical layer
     *
     * @param can_msg_t *can_msg - Pointer to the CAN message transmit
     *
     * @return none
     *
     */
    extern bool CanTxStatemachine_send_can_message(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TX_STATEMACHINE__ */
