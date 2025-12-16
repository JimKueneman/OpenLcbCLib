
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
 * 14 Dec 2025
 * Copyright (c) 2025, Jim Kueneman
 */

/**
 *
 * @brief Implements the statemachine to log into the OpenLcb/LCC network.
 *
 * This is the main statemachine to process nodes as they log into the network.  An applications
 * message loop should call \ref CanLoginStateMachine_run() as fast as possible.  The dependency injection
 * interface must be passed as a parameter to the initialization call \ref CanLoginStateMachine_initialize().
 *
 *
 * @file can_login_statemachine.h
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

/**
 * @brief A structure to hold pointers to functions for dependencies this module requires \ref can_login_statemachine.h.
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
    /** Pointer to a function for the Initialization State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_init() */
    void (*state_init)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the Generate Seed State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_generate_seed() */
    void (*state_generate_seed)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the Generate Alias State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_generate_alias() */
    void (*state_generate_alias)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the CID7 State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_load_cid07() */
    void (*state_load_cid07)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the CID6 State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_load_cid06() */
    void (*state_load_cid06)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the CID5 State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_load_cid05() */
    void (*state_load_cid05)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the CID4 State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_load_cid04() */
    void (*state_load_cid04)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the 200ms Wait State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_wait_200ms() */
    void (*state_wait_200ms)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the RID State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_load_rid() */
    void (*state_load_rid)(can_statemachine_info_t *can_statemachine_info);
    /** Pointer to a function for the AMD State Handler for the Login StateMachine
     * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginMessageHandler_state_load_amd() */
    void (*state_load_amd)(can_statemachine_info_t *can_statemachine_info);

    // OPTIONAL FUNCTION

    // CALLBACK FUNCTIONS

    /*@}*/

} interface_can_login_state_machine_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Main Statemachine<br>
     *
     * @param const interface_can_login_state_machine_t *interface_can_login_state_machine - Pointer to a
     * interface_can_login_state_machine_t struct containing the functions that this module requires.
     *
     * @return none
     *
     * @attention This must always be called during application initialization
     */
    extern void CanLoginStateMachine_initialize(const interface_can_login_state_machine_t *interface_can_login_state_machine);

    /**
     * @brief Runs the main statemachine to handle incoming CAN messages and correctly respond to them through the
     * handlers in the interface \ref interface_can_login_state_machine_t<br>
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a structure that contains the incoming CAN message for the
     * passed Node.  If a handler is found it will process the message else it the appropriate error code will be returned.
     *
     * @return none
     *
     * @note Call from the main application loop as fast as possible.
     */
    extern void CanLoginStateMachine_run(can_statemachine_info_t *can_statemachine_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__ */
