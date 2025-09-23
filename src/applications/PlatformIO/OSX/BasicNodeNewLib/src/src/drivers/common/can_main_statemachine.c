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

static can_msg_t *_active_outgoing_can_msg;
static openlcb_msg_t *_active_outgoing_openlcb_msg;

static can_msg_t _login_in_can_msg;
static openlcb_msg_t _login_openlcb_msg;
static openlcb_basic_data_buffer_t _login_openlcb_payload;

void CanMainStatemachine_initialize(const interface_can_main_statemachine_t *interface_can_main_statemachine) {

    _interface = (interface_can_main_statemachine_t*) interface_can_main_statemachine;
    
    CanUtilities_clear_can_message(&_login_in_can_msg);
    
    _login_openlcb_msg.payload = (openlcb_payload_t*) &_login_openlcb_payload;
    OpenLcbUtilities_clear_openlcb_message(&_login_openlcb_msg);
    _login_openlcb_msg.payload_type = BASIC;
    
}

static void _handle_duplicate_alias_detected(openlcb_node_t *openlcb_node) {

    _interface->lock_can_buffer_fifo();

    openlcb_node->state.permitted = false;
    openlcb_node->state.initalized = false;
    openlcb_node->state.duplicate_id_detected = false;
    openlcb_node->state.duplicate_alias_detected = false;
    openlcb_node->state.firmware_upgrade_active = false;
    openlcb_node->state.resend_datagram = false;
    openlcb_node->state.openlcb_datagram_ack_sent = false;
    OpenLcbBufferStore_free_buffer(openlcb_node->last_received_datagram);
    openlcb_node->last_received_datagram = NULL;
    openlcb_node->state.run_state = RUNSTATE_GENERATE_SEED; // Re-log in with a new generated Alias   

    _interface->unlock_can_buffer_fifo();

}

void CanMainStateMachine_run(void) {

    // First if there is no can message to pending send see if there are any in the FIFO to send
    
    if (!_active_outgoing_can_msg) {

        _interface->lock_can_buffer_fifo();
        _active_outgoing_can_msg = CanBufferFifo_pop();
        _interface->unlock_can_buffer_fifo();
    }
    
    // Second see if there is a CAN message waiting to be sent, if so try to send it, if not try again on the next loop
    if (_active_outgoing_can_msg) {

        if (_interface->send_can_message(_active_outgoing_can_msg)) {
            
            // If not allocated then it is the local buffer for the CAN Login messages so we don't free it, else it was allocated on the FIFO and neesd to be freed
            if (_active_outgoing_can_msg->state.allocated) {
                
                CanBufferStore_free_buffer(_active_outgoing_can_msg);
                
            }

            // Success, reset for a new message
            _active_outgoing_can_msg->identifier = 0x00;
            _active_outgoing_can_msg = NULL;

        } else {
            
            return;  // try again on the next loop
            
        }

    }
    
    // Is there a valid openlcb message ready to send?  If so try to send it
    
    if (_active_outgoing_openlcb_msg) {

        if (_interface->send_openlcb_message(_active_outgoing_openlcb_msg)) {

            // Any OpenLcb Message is for Login In only at this level so just locally clear it
            _active_outgoing_openlcb_msg->mti = 0x00;
            _active_outgoing_openlcb_msg = NULL;

        }

    }

    // Once all pending messages that need to be sent are handled we can check for duplicate ID and run the CAN statemachine for each node
    openlcb_node_t *openlcb_node = _interface->node_get_first(0);

    while (openlcb_node) {

        if (openlcb_node->state.duplicate_alias_detected) {

            _handle_duplicate_alias_detected(openlcb_node);

        }

        if (openlcb_node->state.run_state < RUNSTATE_RUN) {
  
            _interface->login_statemachine_run(openlcb_node, &_login_in_can_msg, &_login_openlcb_msg);
            
            if (_login_in_can_msg.identifier != 0x00) {
                
                _active_outgoing_can_msg = &_login_in_can_msg;
                
            }
            
            if (_login_openlcb_msg.mti != 0x00) {
                
                _active_outgoing_openlcb_msg = &_login_openlcb_msg;
                
            }

        }

        openlcb_node = _interface->node_get_next(0);
    }

}
