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
 * @brief Implements handlers for the messages and alias creation tasks a node must
 * follow to login to the OpenLcb/LCC network.
 *
 * When a node is logging into the network on a CAN bus it must follow a specific
 * flow to generate/allocate/notify a unique alias ID then broadcast its events.
 * This module defines the specific handlers for each step of this login procedure.<br>
 *
 * These functions are used in the \ref interface_can_login_state_machine_t which
 * are called from within the CAN login statemachine switch statement.<br>
 *
 * @note Any handler may be overridden by assigning a custom function pointer to the
 * \ref interface_can_login_state_machine_t field during initialization of the application.<br>
 * see: \ref CanLoginStateMachine_initialize();
 *
 * @file can_login_message_handler.h
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__
#define __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @struct interface_can_login_message_handler_t
     * @brief A structure to define function dependencies that this module requires, \ref can_login_message_handler.h.
     */
    typedef struct
    {

        // Required functions

        /** Pointer to a function that registers an Mapping between the passed alias and node ID
         * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_register()
         */
        alias_mapping_t *(*alias_mapping_register)(uint16_t alias, node_id_t node_id);
        /** Pointer to a function that finds a Mapping structure that matches the alias passed to it
         * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_find_mapping_by_alias()
         */
        alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

        // Callback events

        /** Pointer to a function for an Application to be notified when an Alias has been successfully registered
         *@note <b>Optional</b> application callback.  Defaults to NULL
         */
        void (*on_alias_change)(uint16_t alias, node_id_t node_id);

    } interface_can_login_message_handler_t;

    /**
     * @brief Initializes the CAN Login Message Handler module
     *
     * @param const interface_can_login_message_handler_t *interface - Pointers to function dependencies this module requires
     *
     * @return none
     *
     * @attention This must always be called during application initialization
     */
    extern void CanLoginMessageHandler_initialize(const interface_can_login_message_handler_t *interface);

    /**
     * @brief Handles the first state in the CAN login sequence.  It resets internal structures and moves on to the next state
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_init(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the second state in the CAN login sequence.  It generates a seed to create the Alias and moves on to the next state
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_generate_seed(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the third state in the CAN login sequence.  It generates an Alias from the seed and moves on to the next state
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_generate_alias(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the fourth state in the CAN login sequence.  It loads the output buffer with a CID message with the top 3 bytes of the NodeID
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_load_cid07(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the fifth state in the CAN login sequence.  It loads the output buffer with a CID message with the next 3 bytes of the NodeID
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_load_cid06(can_statemachine_info_t *can_statemachine_infog);

    /**
     * @brief Handles the sixth state in the CAN login sequence.  It loads the output buffer with a CID message with the next 3 bytes of the NodeID
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_load_cid05(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the seventh state in the CAN login sequence.  It loads the output buffer with a CID message with the last 3 bytes of the NodeID
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_load_cid04(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the eight state in the CAN login sequence.  It waits for a minimum of 200ms before moving to the next state
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_wait_200ms(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the ninth state in the CAN login sequence.  It loads the output buffer with a RID message
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_load_rid(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the tenth state in the CAN login sequence.  It loads the output buffer with an AMD message
     *
     * @param can_statemachine_info_t *can_statemachine_info - Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary.  The incoming message is not used.
     *
     * @return none
     */
    extern void CanLoginMessageHandler_state_load_amd(can_statemachine_info_t *can_statemachine_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__ */
