/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * @file can_login_statemachine.c
 * @brief Implementation of the CAN login state machine
 *
 * @details Implements the main dispatcher for the 10-state CAN login sequence.
 * Uses a switch statement to route execution to the appropriate state handler
 * based on the node's current run_state value.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "can_login_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"


static interface_can_login_state_machine_t *_interface;

    /**
     * @brief Initializes the CAN Login State Machine
     *
     * @details Algorithm:
     * -# Cast away const qualifier from interface pointer
     * -# Store interface pointer in static variable
     *
     * Use cases:
     * - Called once during application initialization
     * - Must be called before CanLoginStateMachine_run
     *
     * @verbatim
     * @param interface_can_login_state_machine Pointer to a
     * interface_can_login_state_machine_t struct containing the functions that this module requires
     * @endverbatim
     *
     * @warning Interface pointer must remain valid for lifetime of application
     * @warning All state handler function pointers must be non-NULL
     * @warning NOT thread-safe
     *
     * @attention This must always be called during application initialization
     *
     * @see CanLoginMessageHandler_initialize - Initialize the message handlers
     * @see CanLoginStateMachine_run - Main state machine execution function
     */
void CanLoginStateMachine_initialize(const interface_can_login_state_machine_t *interface_can_login_state_machine) {

    _interface = (interface_can_login_state_machine_t *) interface_can_login_state_machine;

}

    /**
     * @brief Runs the CAN login state machine
     *
     * @details Algorithm:
     * -# Read node's current run_state
     * -# Switch on run_state value
     * -# For each state case:
     *    - Call appropriate state handler via interface
     *    - Return immediately after handler completes
     * -# Default case returns without action
     *
     * Use cases:
     * - Called from main application loop for nodes in login sequence
     * - Called repeatedly until node reaches permitted state
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a structure that contains the node
     * and output message buffer for the login sequence
     * @endverbatim
     *
     * @warning Structure pointer must not be NULL
     * @warning Node pointer within structure must not be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Modifies node run_state as it progresses through sequence
     * @attention May set login_outgoing_can_msg_valid flag
     *
     * @note Call from the main application loop as fast as possible
     * @note State machine returns immediately after dispatching to handler
     *
     * @see CanLoginMessageHandler_state_init - First state handler
     * @see CanLoginMessageHandler_state_load_amd - Final state handler
     */
void CanLoginStateMachine_run(can_statemachine_info_t *can_statemachine_info) {

    switch (can_statemachine_info->openlcb_node->state.run_state) {

        case RUNSTATE_INIT:

            _interface->state_init(can_statemachine_info);

            return;

        case RUNSTATE_GENERATE_SEED:

            _interface->state_generate_seed(can_statemachine_info);

            return;

        case RUNSTATE_GENERATE_ALIAS:

            _interface->state_generate_alias(can_statemachine_info);

            return;

        case RUNSTATE_LOAD_CHECK_ID_07:

            _interface->state_load_cid07(can_statemachine_info);

            return;

        case RUNSTATE_LOAD_CHECK_ID_06:

            _interface->state_load_cid06(can_statemachine_info);

            return;

        case RUNSTATE_LOAD_CHECK_ID_05:

            _interface->state_load_cid05(can_statemachine_info);

            return;

        case RUNSTATE_LOAD_CHECK_ID_04:

            _interface->state_load_cid04(can_statemachine_info);

            return;

        case RUNSTATE_WAIT_200ms:

            _interface->state_wait_200ms(can_statemachine_info);

            return;

        case RUNSTATE_LOAD_RESERVE_ID:

            _interface->state_load_rid(can_statemachine_info);

            return;

        case RUNSTATE_LOAD_ALIAS_MAP_DEFINITION:

            _interface->state_load_amd(can_statemachine_info);

            return;

        default:

            return;

    }

}
