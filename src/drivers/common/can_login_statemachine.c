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

void CanLoginStateMachine_initialize(const interface_can_login_state_machine_t *interface_can_login_state_machine) {

    _interface = (interface_can_login_state_machine_t*) interface_can_login_state_machine;

}

void CanLoginStateMachine_run(openlcb_node_t* openlcb_node) {



    switch (openlcb_node->state.run_state) {

        case RUNSTATE_INIT:

            _interface->init(openlcb_node);

            return;

        case RUNSTATE_GENERATE_SEED:

            _interface->generate_seed(openlcb_node);

            return;

        case RUNSTATE_GENERATE_ALIAS:

            _interface->generate_alias(openlcb_node);

            return;

        case RUNSTATE_SEND_CHECK_ID_07:

            _interface->transmit_cid07(openlcb_node);

            return;

        case RUNSTATE_SEND_CHECK_ID_06:

            _interface->transmit_cid06(openlcb_node);

            return;

        case RUNSTATE_SEND_CHECK_ID_05:

            _interface->transmit_cid05(openlcb_node);

            return;

        case RUNSTATE_SEND_CHECK_ID_04:

            _interface->transmit_cid04(openlcb_node);

            return;

        case RUNSTATE_WAIT_200ms:

            _interface->wait_200ms(openlcb_node);

            return;

        case RUNSTATE_TRANSMIT_RESERVE_ID:

            _interface->transmit_rid(openlcb_node);

            return;

        case RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION:

            _interface->transmit_amd(openlcb_node);

            return;

        case RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE:

            _interface->transmit_initialization_complete(openlcb_node);

            return;

        case RUNSTATE_TRANSMIT_PRODUCER_EVENTS:

            _interface->transmit_producer_events(openlcb_node);

            return;

        case RUNSTATE_TRANSMIT_CONSUMER_EVENTS:

            _interface->transmit_consumer_events(openlcb_node);

            return;

        case RUNSTATE_RUN:

            return;

    }

}