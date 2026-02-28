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
 * @brief Implementation of the CAN login state machine dispatcher.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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

    /** @brief Stores the dependency-injection interface pointer. */
void CanLoginStateMachine_initialize(const interface_can_login_state_machine_t *interface_can_login_state_machine) {

    _interface = (interface_can_login_state_machine_t *) interface_can_login_state_machine;

}

    /**
     * @brief Dispatches to the appropriate login state handler based on the node's run_state.
     *
     * @verbatim
     * @param can_statemachine_info State machine context (node + login frame buffer).
     * @endverbatim
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
