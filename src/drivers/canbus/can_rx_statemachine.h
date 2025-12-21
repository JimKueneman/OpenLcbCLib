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
 * @brief Implements a state machine, actually a large switch statement that decodes the 
 * incoming message and calls the correct the can_rx_message_handler.h handler functions.
 * 
 * This is the core receive functionality of the CAN frames.  It decodes incoming frames and 
 * decides what type of message it then dispatches to the appropriate handler.  This includes
 * assembling multi-frame OpenLcb/LCC messages, automatically handling CAN only frames by generating
 * the correct response and placing it into the outgoing CAN frame FIFO in can_buffer_fifo.h
 *
 * @file can_rx_statemachine.h
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

/**
 * @brief A structure to hold pointers to functions for dependencies this module requires, \ref can_rx_statemachine.h.
 *
 * @details OpenLcbCLib uses dependency injection to allow for writing full coverage tests as the
 * functions that are used can be modeled in the test and return valid OR invalid results to fully
 * test all program flows in the module.  It also allows for reducing the program size. If a particular
 * protocol does not need to be implemented simply filling in the dependency for that handler with a NULL
 * will strip out code for that protocols handlers and minimize the application size (bootloader is an example).
 * The library will automatically reply with the correct error/reply codes if the handler is defined as NULL
 */
typedef struct
{

    /*@{*/

    // REQUIRED FUNCTIONS

    /** Pointer to a function to handle receiving a legacy SNIP CAN frame.
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_can_legacy_snip() */
    void (*handle_can_legacy_snip)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);
  
    
    /** Pointer to a function to handle receiving a CAN frame that is a complete message using a single CAN frame.
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_single_frame() */
    void (*handle_single_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);
   
    
    /** Pointer to a function to handle receiving a CAN frame that is the first of multiple frames required to make a complete message.
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_first_frame() */
    void (*handle_first_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);
    
    
    /** Pointer to a function to handle receiving a CAN frame that is in the middle of several frames required to make a complete message.
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_middle_frame() */
    void (*handle_middle_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index);
    
    /** Pointer to a function to handle receiving a CAN frame that is the last of multiple required to make a complete message.
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_last_frame() */
    void (*handle_last_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index);
    
    /** Pointer to a function to handle receiving a CAN frame that is used in the Stream Protocol.
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_stream_frame() */ 
    void (*handle_stream_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);
    
    
    /** Pointer to a function to handle receiving a CAN frame that in the CAN Frame Transfer Protocol for Reserve ID .
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_rid_frame() */ 
    void (*handle_rid_frame)(can_msg_t *can_msg);
    
    
    /** Pointer to a function to handle receiving a CAN frame that in the CAN Frame Transfer Protocol for Alias Map Definition .
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_amd_frame() */ 
    void (*handle_amd_frame)(can_msg_t *can_msg);
    
    /** Pointer to a function to handle receiving a CAN frame that in the CAN Frame Transfer Protocol for Alias Mapping Enquiry .
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_ame_frame() */
    void (*handle_ame_frame)(can_msg_t *can_msg);
    
    
    /** Pointer to a function to handle receiving a CAN frame that in the CAN Frame Transfer Protocol for Alias Mapping Reply .
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_amr_frame() */ 
    void (*handle_amr_frame)(can_msg_t *can_msg);
    
    
    /** Pointer to a function to handle receiving a CAN frame that in the CAN Frame Transfer Protocol for Error Info Reporting .
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_error_info_report_frame() */ 
    void (*handle_error_info_report_frame)(can_msg_t *can_msg);
    
    /** Pointer to a function to handle receiving a CAN frame that in the CAN Frame Transfer Check ID
     * @warning <b>Required</b> assignment.  Defaults to CanRxMessageHandler_cid_frame() */ 
    void (*handle_cid_frame)(can_msg_t *can_msg);

    /** Pointer to a function for access into the \ref alias_mappings.h functions to access the mapping pairs
     * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_find_mapping_by_alias() */
    alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

    // OPTIONAL FUNCTION

    // CALLBACK FUNCTIONS

    /** Pointer to a function for an Application to be notified when a CAN frame is received.
     *@note <b>Optional</b> application callback.  Defaults to NULL
     *@warning This is called in the context of the Application defined interface to the CAN hardware 
     *so it it typically called from an interrupt of thread. Be care of what is done in the callback
     *and make sure it does not consume too many clock cycles.*/
    void (*on_receive)(can_msg_t *can_msg);

    /*@}*/

} interface_can_rx_statemachine_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Receive (Rx) state machine
     *
     * @param const interface_can_rx_statemachine_t *interface_can_rx_statemachine - Pointer to a
     * interface_can_rx_statemachine_t struct containing the functions that this module requires.
     *
     * @return none
     *
     * @attention This must always be called during application initialization.
     */
    extern void CanRxStatemachine_initialize(const interface_can_rx_statemachine_t *interface_can_rx_statemachine);

    
    /**
     * @brief Handles a first frame of an OpenLcb/LCC message that is sent multiple CAN frames.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame that was received
     *
     * @return none
     * 
     * @note When the Application driver receives a CAN frame it calls this function, unless the 
     * library has called the Application defined function lock_shared_resources asking this function not
     * be called until after the unlock_shared_resources. The time when the lock is enabled is minimal
     * and will never be more than a fraction of the time required for the next CAN message to be received.
     * Also be aware the implementation of the interrupt or thread must not block if lock_shared_resources is enabled,
     * the main loop thread must be able to continue execution to be able to call unlock_shared_resources. 
     * 
     * The common way to handle this with interrupts is when lock_shared_resources is called the microcontroller 
     * disabled interrupts for the CAN and 100ms timer so they can't be called during the time the library executes
     * lock_shared_resources/lock_unshared_resources.  If using threads they can either be suspended (if using RTOS)
     * or store the incoming CAN frame and have it call this function after unlock_shared_resources is called with the
     * stored frame.
     * 
     * @attention <b>Required<\b> This is the entry into the OpenLcbCLib from the Application hardware interface driver. 
     * 
     */
    extern void CanRxStatemachine_incoming_can_driver_callback(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__ */
