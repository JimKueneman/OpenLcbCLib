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
 * \file can_main_statemachine.c
 *
 * Where the real work in dispatching the incoming OpenLcb messages to the various
 * handlers to process.  It will call the OpenLcb main statemachine when needed.  
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_main_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_buffer_store.h"

// TODO Get this out of here
#include "../../openlcb/openlcb_node.h"



static interface_can_main_statemachine_t *_interface;

static can_statemachine_info_t can_statemachine_info = {
    .incoming_msg = NULL,
    .openlcb_node = NULL,
    .outgoing_can_msg = NULL,
    .outgoing_can_msg_valid = false,
    .outgoing_openlcb_msg = NULL,
    .outgoing_openlcb_msg_valid = false
};


static can_msg_t _can_msg;
static openlcb_msg_t _openlcb_msg;
static payload_basic_t _openlcb_payload;

void CanMainStatemachine_initialize(const interface_can_main_statemachine_t *interface_can_main_statemachine) {

    _interface = (interface_can_main_statemachine_t*) interface_can_main_statemachine;

    can_statemachine_info.outgoing_can_msg = &_can_msg;
    can_statemachine_info.outgoing_openlcb_msg = &_openlcb_msg;

    CanUtilities_clear_can_message(&_can_msg);
    OpenLcbUtilities_clear_openlcb_message(&_openlcb_msg);
    _openlcb_msg.payload = (openlcb_payload_t*) & _openlcb_payload;
    _openlcb_msg.state.allocated = true;
    _openlcb_msg.payload_type = BASIC;

}

void _run_statemachine(can_statemachine_info_t *can_statemachine_info) {

    if (!can_statemachine_info->incoming_msg) {
        
        return;
        
    }
    
    // CAN Control Messages

    switch (can_statemachine_info->incoming_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

        case 0:

            switch (can_statemachine_info->incoming_msg->identifier & MASK_CAN_VARIABLE_FIELD) {


                case CAN_CONTROL_FRAME_RID: // Reserve ID

                    if (_interface->handle_rid) {

                        _interface->handle_rid(can_statemachine_info);

                    }

                    break;

                case CAN_CONTROL_FRAME_AMD: // Alias Map Definition

                    if (_interface->handle_amd) {

                        _interface->handle_amd(can_statemachine_info);

                    }

                    break;

                case CAN_CONTROL_FRAME_AME:

                    if (_interface->handle_ame) {

                        _interface->handle_ame(can_statemachine_info);

                    }

                    break;

                case CAN_CONTROL_FRAME_AMR:

                    if (_interface->handle_amr) {

                        _interface->handle_amr(can_statemachine_info);

                    }

                    break;

                case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0: // Advanced feature for gateways/routers/etc.
                case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1:
                case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2:
                case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3:

                    if (_interface->handle_error_information_report) {

                        _interface->handle_error_information_report(can_statemachine_info);

                    }

                    break;

                default:

                    // Do nothing
                    break; // default

            }

            break;

        default:

            switch (can_statemachine_info->incoming_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

                case CAN_CONTROL_FRAME_CID7:
                case CAN_CONTROL_FRAME_CID6:
                case CAN_CONTROL_FRAME_CID5:
                case CAN_CONTROL_FRAME_CID4:


                    if (_interface->handle_cid) {

                        _interface->handle_cid(can_statemachine_info);

                    }

                    break;

                case CAN_CONTROL_FRAME_CID3:
                case CAN_CONTROL_FRAME_CID2:
                case CAN_CONTROL_FRAME_CID1:

                    break;
            }

            break; // default

    } // CAN control messages

}

void CanMainStateMachine_run(void) {

    // First get pending outgoing messages sent
    if (can_statemachine_info.outgoing_can_msg_valid) {

        if (_interface->send_can_message(can_statemachine_info.outgoing_can_msg)) {

            can_statemachine_info.outgoing_can_msg_valid = false;

            return; // done for this loop

        }

    }

    if (can_statemachine_info.outgoing_openlcb_msg_valid) {

        if (_interface->send_openlcb_message) {

            can_statemachine_info.outgoing_openlcb_msg_valid = false;

            return; // done for this loop

        }

    }


    // Second see if we have an incoming message, if not see if we have one on the stack waiting
    // Note still need to run login state-machine so don't return here
    if (!can_statemachine_info.incoming_msg) {

        _interface->lock_can_buffer_fifo();
        can_statemachine_info.incoming_msg = CanBufferFifo_pop();
        _interface->unlock_can_buffer_fifo();
        
    }

    // Third, if enumerating the message to the same node call it again
    if (can_statemachine_info.enumerating) {

        if (can_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

            _run_statemachine(&can_statemachine_info);

        } else {

            _interface->login_statemachine_run(&can_statemachine_info);

        }

        return; // done

    }

    // Fourth if the current node is null then time to re-enumerate nodes with the next incoming message 
    if (!can_statemachine_info.openlcb_node) {

        can_statemachine_info.openlcb_node = _interface->node_get_first(CAN_STATEMACHINE_NODE_ENUMRATOR_KEY);

        if (!can_statemachine_info.openlcb_node) {

            CanBufferStore_free_buffer(can_statemachine_info.incoming_msg); // only get here if there is a valid incoming message
            can_statemachine_info.incoming_msg = NULL;

            return; // done, nothing to do

        }


        if (can_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

            _run_statemachine(&can_statemachine_info);

        } else {

            _interface->login_statemachine_run(&can_statemachine_info);

        }

        return; // done

    }

    // Fifth enumerate to the next node
    can_statemachine_info.openlcb_node = _interface->node_get_next(CAN_STATEMACHINE_NODE_ENUMRATOR_KEY);

    if (!can_statemachine_info.openlcb_node) {

        CanBufferStore_free_buffer(can_statemachine_info.incoming_msg); // only get here if there is a valid incoming message
        can_statemachine_info.incoming_msg = NULL;

        return; // done, nothing to do

    }

    if (can_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

            _run_statemachine(&can_statemachine_info);

        } else {

            _interface->login_statemachine_run(&can_statemachine_info);

        }
}
