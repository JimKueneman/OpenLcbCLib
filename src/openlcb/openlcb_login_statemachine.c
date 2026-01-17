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
 * @file openlcb_login_statemachine.c
 * @brief Implementation of the login state machine
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "openlcb_login_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_utilities.h"


static interface_openlcb_login_state_machine_t *_interface;

static openlcb_login_statemachine_info_t _statemachine_info;


void OpenLcbLoginStateMachine_initialize(const interface_openlcb_login_state_machine_t *interface_openlcb_login_state_machine) {

    _interface = (interface_openlcb_login_state_machine_t*) interface_openlcb_login_state_machine;

    _statemachine_info.outgoing_msg_info.msg_ptr = &_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload = (openlcb_payload_t*) _statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload_type = BASIC;
    OpenLcbUtilities_clear_openlcb_message(_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_statemachine_info.outgoing_msg_info.msg_ptr);
    _statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _statemachine_info.openlcb_node = NULL;

}

void OpenLcbLoginStateMachine_process(openlcb_login_statemachine_info_t *openlcb_statemachine_info) {

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

bool OpenLcbLoginStatemachine_handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg_info.msg_ptr)) {

            _statemachine_info.outgoing_msg_info.valid = false; // done

        }

        return true; // keep trying till it can get sent

    }

    return false;

}

bool OpenLcbLoginStatemachine_handle_try_reenumerate(void) {

    if (_statemachine_info.outgoing_msg_info.enumerate) {

        _interface->process_login_statemachine(&_statemachine_info); // Continue the processing of the incoming message on the node

        return true; // keep going until target clears the enumerate flag

    }

    return false;

}

bool OpenLcbLoginStatemachine_handle_try_enumerate_first_node(void) {

    if (!_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = _interface->openlcb_node_get_first(OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) {

            return true; // done

        }

        if (_statemachine_info.openlcb_node->state.run_state < RUNSTATE_RUN) {

            _interface->process_login_statemachine(&_statemachine_info); // Do the processing of the incoming message on the node

        }

        return true; // done

    }

    return false;

}

bool OpenLcbLoginStatemachine_handle_try_enumerate_next_node(void) {

    if (_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = _interface->openlcb_node_get_next(OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) { // reached the end of the list, free the incoming message

            return true; // done

        }

        if (_statemachine_info.openlcb_node->state.run_state < RUNSTATE_RUN) {

            _interface->process_login_statemachine(&_statemachine_info); // Do the processing of the incoming message on the node

        }

        return true; // done
    }

    return false;

}

void OpenLcbLoginMainStatemachine_run(void) {

    // Get any pending message out first
    if (_interface->handle_outgoing_openlcb_message()) {

        return;

    }

    // If the message handler needs to send multiple messages then enumerate the same incoming/login outgoing message again
    if (_interface->handle_try_reenumerate()) {

        return;

    }

    // Grab the first OpenLcb Node
    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    // Enumerate all the OpenLcb Nodes
    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}

openlcb_login_statemachine_info_t *OpenLcbLoginStatemachine_get_statemachine_info(void) {

    return &_statemachine_info;

}

