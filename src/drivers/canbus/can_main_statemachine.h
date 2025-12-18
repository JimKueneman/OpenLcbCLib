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
 * @brief Implements the main state machine that dispatches incoming CAN messages to
 * the target nodes and calls any handlers assigned for those messages.
 *
 * This is the main state machine to process nodes as CAN messages are received from the network.  An applications
 * message loop should call \ref CanMainStateMachine_run() as fast as possible.  The dependency injection
 * interface must be passed as a parameter to the initialization call \ref CanMainStatemachine_initialize().
 *
 *
 * @file can_main_statemachine.h
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief A structure to hold pointers to functions for dependencies this module requires \ref can_main_statemachine.h.
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

        /** Pointer to an Application supplied function that must stop the Application supplied 100ms Timer  and the hardware CAN Frame Receive (Rx) from accessing the library.
         * @warning <b>Required</b> assignment.  Defaults to an Application defined function */
        void (*lock_shared_resources)(void);

        /** Pointer to an Application supplied function that must restart the Application supplied 100ms Timer and the hardware CAN Frame Receive (Rx) from accessing the library buffers.
         * @warning <b>Required</b> assignment.  Defaults to an Application defined function */
        void (*unlock_shared_resources)(void);

        /** Pointer to an Application supplied function that transmits the passed msg on the physical CAN line.
         * @note The implementation of this may either place the message in the hardwares transmit buffer or create an internal software buffer
         * @warning <b>Required</b> assignment.  Defaults to \ref CanTxStatemachine_send_can_message() */
        bool (*send_can_message)(can_msg_t *msg);

        /** Pointer to a function for access into the \ref openlcb_node.h functions get enumerate current nodes
         * @warning <b>Required</b> assignment.  Defaults to \ref OpenLcbNode_get_first() */
        openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);

        /** Pointer to a function for access into the \ref openlcb_node.h functions get enumerate current nodes
         * @warning <b>Required</b> assignment.  Defaults to \ref OpenLcbNode_get_next() */
        openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);

        /** Pointer to a function for access into the \ref openlcb_node.h functions get find a current node using its Alias
         * @warning <b>Required</b> assignment.  Defaults to \ref OpenLcbNode_find_by_alias() */
        openlcb_node_t *(*openlcb_node_find_by_alias)(uint16_t alias);

        /** Pointer to a function for access into the \ref can_login_statemachine.h functions to Run the Login state machine.
         * @warning <b>Required</b> assignment.  Defaults to \ref CanLoginStateMachine_run() */
        void (*login_statemachine_run)(can_statemachine_info_t *can_statemachine_info);

        /** Pointer to a function for access into the \ref alias_mappings.h functions to access the mapping pairs
         * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_get_alias_mapping_info() */
        alias_mapping_info_t *(*alias_mapping_get_alias_mapping_info)(void);

        /** Pointer to a function for access into the \ref alias_mappings.h functions to unregister the mapping pairs, typically due to a duplicate Alias error
         * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_unregister() */
        void (*alias_mapping_unregister)(uint16_t alias);

        /** Pointer to a function to handle duplicated Aliases that have been detected. This function is defined and accessed from within this modules
         * and exists to enable better test coverage as the test cases can return either true or false easily to ensure the statemachine functions properly
         * @warning <b>Required</b> assignment.  Defaults to \ref CanMainStatemachine_handle_duplicate_aliases() */
        bool (*handle_duplicate_aliases)(void);

        /** Pointer to a function to handle outgoing CAN messages. This function is defined and accessed from within this modules
         * and exists to enable better test coverage as the test cases can return either true or false easily to ensure the statemachine functions properly
         * @warning <b>Required</b> assignment.  Defaults to \ref CanMainStatemachine_handle_outgoing_can_message() */
        bool (*handle_outgoing_can_message)(void);

        /** Pointer to a function to handle outgoing login CAN messages. This function is defined and accessed from within this modules
         * and exists to enable better test coverage as the test cases can return either true or false easily to ensure the statemachine functions properly
         * @warning <b>Required</b> assignment.  Defaults to \ref CanMainStatemachine_handle_login_outgoing_can_message() */
        bool (*handle_login_outgoing_can_message)(void);

        /** Pointer to a function to handle enumerating the nodes to run through the statemachine. This function is defined and accessed from within this modules
         * and exists to enable better test coverage as the test cases can return either true or false easily to ensure the statemachine functions properly
         * @warning <b>Required</b> assignment.  Defaults to \ref CanMainStatemachine_handle_try_enumerate_first_node() */
        bool (*handle_try_enumerate_first_node)(void);

        /** Pointer to a function to handle enumerating the nodes to run through the statemachine. This function is defined and accessed from within this modules
         * and exists to enable better test coverage as the test cases can return either true or false easily to ensure the statemachine functions properly
         * @warning <b>Required</b> assignment.  Defaults to \ref CanMainStatemachine_handle_try_enumerate_next_node() */
        bool (*handle_try_enumerate_next_node)(void);

        // OPTIONAL FUNCTION

        // CALLBACK FUNCTIONS

        /*@}*/

    } interface_can_main_statemachine_t;

    /**
     * @brief Initializes the CAN Main Statemachine
     *
     * @param const interface_can_main_statemachine_t *interface_can_main_statemachine - Pointer to a
     * interface_can_main_statemachine_t struct containing the functions that this module requires.
     *
     * @return none
     *
     * @attention This must always be called during application initialization
     */
    extern void CanMainStatemachine_initialize(const interface_can_main_statemachine_t *interface_can_main_statemachine);


    /**
     * @brief Runs the main statemachine to handle incoming CAN messages and correctly respond to them through the
     * handlers in the interface \ref interface_can_login_state_machine_t
     *
     * @param none
     *
     * @return none
     *
     * @note Call from the main application loop as fast as possible.
     */
    extern void CanMainStateMachine_run(void);


    /**
     * @brief Accesses the internal structure that the CAN Main State machine uses to maintain the Node that is currently being
     * targeted, incoming CAN message and a buffer for sending a reply CAN message if necessary
     *
     * @param none
     *
     * @return Pointer to an internal structure that hold the current state of the CAN State machine
     *
     * @note This access is for debugging and Google Test access, there should be no reason to access it from an Application
     */
    extern can_statemachine_info_t *CanMainStateMachine_get_can_statemachine_info(void);


    /**
     * @brief Checks for and handles any duplicate Alias that were detected on the incoming CAN receive state machine
     *
     * @param none
     *
     * @return none
     *
     * @note This access is for debugging and Google Test access, there should be no reason to access it from an Application
     */
    extern bool CanMainStatemachine_handle_duplicate_aliases(void);


    /**
     * @brief Checks for and handles any login messages that need to be sent for a Node login
     *
     * @param none
     *
     * @return none
     *
     * @note This access is for debugging and Google Test access, there should be no reason to access it from an Application
     */
    extern bool CanMainStatemachine_handle_login_outgoing_can_message(void);


    /**
     * @brief Checks for and handles any out going messages that were a result of replying to an incoming message
     *
     * @param none
     *
     * @return none
     *
     * @note This access is for debugging and Google Test access, there should be no reason to access it from an Application
     */
    extern bool CanMainStatemachine_handle_outgoing_can_message(void);


    /**
     * @brief Enumerator to get the first Node to run the state machine on.  OpenLcbCLib is capable of running virtual Nodes 
     * limited only by memory of the device used and how many Node buffer slots where defined in the \ref USER_DEFINED_NODE_BUFFER_DEPTH
     * constant
     *
     * @param none
     *
     * @return none
     *
     * @note This access is for debugging and Google Test access, there should be no reason to access it from an Application
     */
    extern bool CanMainStatemachine_handle_try_enumerate_first_node(void);


    /**
     * @brief Enumerator to get the next Node to run the state machine on.  OpenLcbCLib is capable of running virtual Nodes 
     * limited only by memory of the device used and how many Node buffer slots where defined in the \ref USER_DEFINED_NODE_BUFFER_DEPTH
     * constant
     *
     * @param none
     *
     * @return none
     *
     * @note This access is for debugging and Google Test access, there should be no reason to access it from an Application
     */
    extern bool CanMainStatemachine_handle_try_enumerate_next_node(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__ */
