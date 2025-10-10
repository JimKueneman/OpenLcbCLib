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

static can_statemachine_info_t _can_statemachine_info;
static can_msg_t _can_msg;
static openlcb_msg_t _openlcb_msg;
static payload_basic_t _openlcb_payload;

void CanMainStatemachine_initialize(const interface_can_main_statemachine_t *interface_can_main_statemachine) {

    _interface = (interface_can_main_statemachine_t*) interface_can_main_statemachine;

    CanUtilities_clear_can_message(&_can_msg);
    OpenLcbUtilities_clear_openlcb_message(&_openlcb_msg);
    _openlcb_msg.payload = (openlcb_payload_t*) & _openlcb_payload;
    _openlcb_msg.state.allocated = true;
    _openlcb_msg.payload_type = BASIC;
    
    _can_statemachine_info.login_outgoing_can_msg = &_can_msg;
    _can_statemachine_info.login_outgoing_openlcb_msg = &_openlcb_msg;
    _can_statemachine_info.openlcb_node = NULL;
    _can_statemachine_info.login_outgoing_can_msg_valid = false;
    _can_statemachine_info.login_outgoing_openlcb_msg_valid = false;
    _can_statemachine_info.enumerating = false;
    _can_statemachine_info.outgoing_can_msg = NULL;

}

static void _reset_node(openlcb_node_t *openlcb_node) {
 
    if (!openlcb_node) {
        
        return;
        
    }

    openlcb_node->alias = 0x00;
    openlcb_node->state.permitted = false;
    openlcb_node->state.initalized = false;
    openlcb_node->state.duplicate_id_detected = false;
    openlcb_node->state.firmware_upgrade_active = false;
    openlcb_node->state.resend_datagram = false;
    openlcb_node->state.openlcb_datagram_ack_sent = false;
    if (openlcb_node->last_received_datagram) {

        OpenLcbBufferStore_free_buffer(openlcb_node->last_received_datagram);
        openlcb_node->last_received_datagram = NULL;

    }

    openlcb_node->state.run_state = RUNSTATE_GENERATE_SEED; // Re-log in with a new generated Alias  

}

static void _run_statemachine(can_statemachine_info_t *can_statemachine_info) {



}

static bool _process_duplicate_aliases(alias_mapping_info_t *alias_mapping_info) {
    
    bool result = false;

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        uint16_t alias = alias_mapping_info->list[i].alias;

        if ((alias > 0) && alias_mapping_info->list[i].is_duplicate) {
 
            _interface->alias_mapping_unregister(alias);
    
            _reset_node(_interface->openlcb_node_find_by_alias(alias));
            
            result = true;

        }
        
    }
    
    alias_mapping_info->has_duplicate_alias = false;
    
    return result;

}

static void _handle_duplicate_aliases(void) {

    _interface->lock_shared_resources();

    alias_mapping_info_t *alias_mapping_info = _interface->alias_mapping_get_alias_mapping_info();

    if (alias_mapping_info->has_duplicate_alias) {

        _process_duplicate_aliases(alias_mapping_info);

    }

    _interface->unlock_shared_resources();

}

static bool _handle_login_outgoing_can_message(void) {

    if (_can_statemachine_info.login_outgoing_can_msg_valid) {

        if (_interface->send_can_message(_can_statemachine_info.login_outgoing_can_msg)) {

            _can_statemachine_info.login_outgoing_can_msg_valid = false;

        } else {

            return true; // done for this loop, try again next time
        }

    }

    return false;

}

static bool _handle_login_outgoing_openlcb_message(void) {

    if (_can_statemachine_info.login_outgoing_openlcb_msg_valid) {

        if (_interface->send_openlcb_message(_can_statemachine_info.login_outgoing_openlcb_msg)) {

            _can_statemachine_info.login_outgoing_openlcb_msg_valid = false;

        } else {

            return true; // done for this loop, try again next time

        }

    }

    return false;

}

static bool _handle_outgoing_can_message(void) {

    if (!_can_statemachine_info.outgoing_can_msg) {

        _interface->lock_shared_resources();

        _can_statemachine_info.outgoing_can_msg = CanBufferFifo_pop();

        _interface->unlock_shared_resources();

    }

    if (_can_statemachine_info.outgoing_can_msg) {

        if (_interface->send_can_message(_can_statemachine_info.outgoing_can_msg)) {

            _interface->lock_shared_resources();

            CanBufferStore_free_buffer(_can_statemachine_info.outgoing_can_msg);

            _interface->unlock_shared_resources();

            _can_statemachine_info.outgoing_can_msg = NULL;

        } else {

            return true; // done for this loop, try again next time

        }

    }

    return false;

}

static bool _handle_reenumerate_openlcb_message(void) {

    if (_can_statemachine_info.enumerating) {

        // Need to make sure the correct state-machine is run depending of if the Node had finished the login process

        if (_can_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

            _run_statemachine(&_can_statemachine_info);

        } else {

            _interface->login_statemachine_run(&_can_statemachine_info);

        }

        return true; // done

    }

    return false;

}

static bool _handle_try_enumerate_first_node(void) {

    if (!_can_statemachine_info.openlcb_node) {

        _can_statemachine_info.openlcb_node = _interface->openlcb_node_get_first(CAN_STATEMACHINE_NODE_ENUMRATOR_KEY);

        if (!_can_statemachine_info.openlcb_node) {

            return true; // done, nothing to do

        }
       
        // Need to make sure the correct state-machine is run depending of if the Node had finished the login process

        if (_can_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

      //        printf("_handle_try_enumerate_first_node: _run_statemachine\n");
            
            _run_statemachine(&_can_statemachine_info);

        } else {

       //     printf("_handle_try_enumerate_first_node: login_statemachine_run\n");
            
            _interface->login_statemachine_run(&_can_statemachine_info);

        }

        return true; // done

    }

    return false;

}

static bool _handle_try_enumerate_next_node(void) {
  
    _can_statemachine_info.openlcb_node = _interface->openlcb_node_get_next(CAN_STATEMACHINE_NODE_ENUMRATOR_KEY);

    if (!_can_statemachine_info.openlcb_node) {

        return true; // done, nothing to do

    }

    // Need to make sure the correct state-machine is run depending of if the Node had finished the login process

    if (_can_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {
        
    //    printf("_handle_try_enumerate_next_node: _run_statemachine\n");

        _run_statemachine(&_can_statemachine_info);

    } else {

     //   printf("_handle_try_enumerate_next_node: _run_statemachine\n");
        
        _interface->login_statemachine_run(&_can_statemachine_info);

    }

    return false;

}

void CanMainStateMachine_run(void) {

 //   fprintf(stderr, "01\n");
    
   if (_handle_duplicate_aliases()) {
       
       return;
   }
    
 //  fprintf(stderr, "02\n");
 
    if (_handle_outgoing_can_message()) {

        return;

    }
    
  //  fprintf(stderr, "03\n");

    if (_handle_login_outgoing_can_message()) {

        return;

    }
    
  //  fprintf(stderr, "04\n");

    if (_handle_login_outgoing_openlcb_message()) {

        return;

    }
    
  //  fprintf(stderr, "05\n");

    if (_handle_reenumerate_openlcb_message()) {

        return;

    }
    
  //  fprintf(stderr, "06\n");

    if (_handle_try_enumerate_first_node()) {

        return;

    }
    
  //  fprintf(stderr, "07\n");

    if (_handle_try_enumerate_next_node()) {

        return;

    }
    
  //  fprintf(stderr, "08\n");

}

can_statemachine_info_t *CanMainStateMachine_get_can_statemachine_info(void) {
    
    return (&_can_statemachine_info);
    
}
