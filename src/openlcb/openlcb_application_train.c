/** \copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * @file openlcb_application_train.c
 * @brief Implementation of application-level Train Control Protocol module
 *
 * @author Jim Kueneman
 * @date 17 Feb 2026
 */

#include "openlcb_application_train.h"

#include <stddef.h>
#include <string.h>

#include "openlcb_application.h"
#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"


static train_state_t _train_pool[USER_DEFINED_TRAIN_NODE_COUNT];
static uint8_t _train_pool_count;
static const interface_openlcb_application_train_t *_interface;


// Initialization

void OpenLcbApplicationTrain_initialize(const interface_openlcb_application_train_t *interface) {

    memset(_train_pool, 0, sizeof(_train_pool));
    _train_pool_count = 0;
    _interface = interface;

}

// Setup

train_state_t* OpenLcbApplicationTrain_setup(openlcb_node_t *openlcb_node) {

    if (!openlcb_node) {

        return NULL;

    }

    if (openlcb_node->train_state) {

        return openlcb_node->train_state;

    }

    if (_train_pool_count >= USER_DEFINED_TRAIN_NODE_COUNT) {

        return NULL;

    }

    train_state_t *state = &_train_pool[_train_pool_count];
    _train_pool_count++;
    memset(state, 0, sizeof(train_state_t));
    openlcb_node->train_state = state;

    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_TRAIN, EVENT_STATUS_SET);
    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_EMERGENCY_OFF, EVENT_STATUS_SET);
    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_SET);
    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_OFF, EVENT_STATUS_SET);
    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_STOP, EVENT_STATUS_SET);

    return state;

}


// Accessor

train_state_t* OpenLcbApplicationTrain_get_state(openlcb_node_t *openlcb_node) {

    if (!openlcb_node) {

        return NULL;

    }

    return openlcb_node->train_state;

}


// Heartbeat timer

void OpenLcbApplicationTrain_100ms_timer_tick(void) {

    for (uint8_t i = 0; i < _train_pool_count; i++) {

        train_state_t *state = &_train_pool[i];

        if (state->heartbeat_timeout_s == 0) {

            continue;

        }

        if (state->heartbeat_counter_100ms > 0) {

            state->heartbeat_counter_100ms--;

        }

        if (state->heartbeat_counter_100ms == 0) {

            state->estop_active = true;
            state->set_speed = 0;

            if (_interface && _interface->on_heartbeat_timeout) {

                _interface->on_heartbeat_timeout(NULL);

            }

        }

    }

}


// Send helpers (throttle side)

static bool _prepare_train_command(openlcb_msg_t *msg, payload_basic_t *payload, openlcb_node_t *openlcb_node, node_id_t train_node_id) {

    if (!openlcb_node || !_interface || !_interface->send_openlcb_msg) {

        return false;

    }

    msg->payload = (openlcb_payload_t *) payload;
    msg->payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            msg,
            openlcb_node->alias,
            openlcb_node->id,
            0,
            train_node_id,
            MTI_TRAIN_PROTOCOL);

    return true;

}

void OpenLcbApplicationTrain_send_set_speed(openlcb_node_t *openlcb_node, node_id_t train_node_id, uint16_t speed) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { 
        
        return; 
    
    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(&msg, speed, 1);

    _interface->send_openlcb_msg(&msg);

}

void OpenLcbApplicationTrain_send_set_function(
        openlcb_node_t *openlcb_node, node_id_t train_node_id,
        uint32_t fn_address, uint16_t fn_value) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { 
        
        return; 
    
    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (fn_address >> 16) & 0xFF, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (fn_address >> 8) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, fn_address & 0xFF, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(&msg, fn_value, 4);

    _interface->send_openlcb_msg(&msg);

}

void OpenLcbApplicationTrain_send_emergency_stop(
        openlcb_node_t *openlcb_node, node_id_t train_node_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { 
        
        return; 
    
    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_EMERGENCY_STOP, 0);

    _interface->send_openlcb_msg(&msg);

}

void OpenLcbApplicationTrain_send_query_speeds(
        openlcb_node_t *openlcb_node, node_id_t train_node_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { 
        
        return; 
    
    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_QUERY_SPEEDS, 0);

    _interface->send_openlcb_msg(&msg);

}

void OpenLcbApplicationTrain_send_query_function(openlcb_node_t *openlcb_node, node_id_t train_node_id, uint32_t fn_address) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { 
        
        return; 
    
    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (fn_address >> 16) & 0xFF, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (fn_address >> 8) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, fn_address & 0xFF, 3);

    _interface->send_openlcb_msg(&msg);

}

void OpenLcbApplicationTrain_send_assign_controller(openlcb_node_t *openlcb_node, node_id_t train_node_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { 
        
        return;
    
    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(&msg, openlcb_node->id, 2);

    _interface->send_openlcb_msg(&msg);

}

void OpenLcbApplicationTrain_send_release_controller(openlcb_node_t *openlcb_node, node_id_t train_node_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { 
        
        return; 
    
    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(&msg, openlcb_node->id, 2);

    _interface->send_openlcb_msg(&msg);

}

void OpenLcbApplicationTrain_send_noop(openlcb_node_t *openlcb_node, node_id_t train_node_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { return; }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MGMT_NOOP, 1);

    _interface->send_openlcb_msg(&msg);

}


// Train search properties

void OpenLcbApplicationTrain_set_dcc_address(openlcb_node_t *openlcb_node, uint16_t dcc_address, bool is_long_address) {

    if (!openlcb_node || !openlcb_node->train_state) {
        
        return; 
    
    }

    openlcb_node->train_state->dcc_address = dcc_address;
    openlcb_node->train_state->is_long_address = is_long_address;

}

uint16_t OpenLcbApplicationTrain_get_dcc_address(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) { 
        
        return 0; 
    
    }

    return openlcb_node->train_state->dcc_address;

}

bool OpenLcbApplicationTrain_is_long_address(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) { 

        return false;
    
    }

    return openlcb_node->train_state->is_long_address;

}

void OpenLcbApplicationTrain_set_speed_steps(openlcb_node_t *openlcb_node, uint8_t speed_steps) {

    if (!openlcb_node || !openlcb_node->train_state) { 
        
        return;
    
    }

    openlcb_node->train_state->speed_steps = speed_steps;

}

uint8_t OpenLcbApplicationTrain_get_speed_steps(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) { 
        
        return 0; 
    
    }

    return openlcb_node->train_state->speed_steps;

}
