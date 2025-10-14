/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file can_login_statemachine.c
 *
 *
 * @author Jim Kueneman
 * @date 12 Aug 2025
 */

#include "openlcb_login_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_defines.h"


static interface_openlcb_login_state_machine_t *_interface;

void OpenLcbLoginStateMachine_initialize(const interface_openlcb_login_state_machine_t *interface_openlcb_login_state_machine) {

    _interface = (interface_openlcb_login_state_machine_t*) interface_openlcb_login_state_machine;

}

void OpenLcbLoginStateMachine_run(openlcb_statemachine_info_t *openlcb_statemachine_info) {


    switch (openlcb_statemachine_info->openlcb_node->state.run_state) {


        case RUNSTATE_LOAD_INITIALIZATION_COMPLETE:

            _interface->load_initialization_complete(openlcb_statemachine_info);
     
            return;

        case RUNSTATE_LOAD_PRODUCER_EVENTS:

            _interface->load_producer_events(openlcb_statemachine_info);

            return;

        case RUNSTATE_LOAD_CONSUMER_EVENTS:

            _interface->load_consumer_events(openlcb_statemachine_info);

            return;
            
        default:
            
            return;

    }

}

