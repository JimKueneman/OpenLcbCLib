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
 * @brief Implementation of the application-level Train Control Protocol module.
 *
 * @details Manages the train state pool, heartbeat timer, and all throttle-side
 * send helpers for the OpenLCB Train Control Protocol.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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


    /**
     * @brief Initialises the train module and stores the callback interface.
     *
     * @details Algorithm:
     * -# Zero the train state pool.
     * -# Reset _train_pool_count to 0.
     * -# Store the interface pointer.
     *
     * @verbatim
     * @param interface  Pointer to a interface_openlcb_application_train_t.
     * @endverbatim
     *
     * @warning Must be called before any other function in this module.
     */
void OpenLcbApplicationTrain_initialize(const interface_openlcb_application_train_t *interface) {

    memset(_train_pool, 0, sizeof(_train_pool));
    _train_pool_count = 0;
    _interface = interface;

}

    /**
     * @brief Allocates a train state slot and assigns it to the node.
     *
     * @details Algorithm:
     * -# Return NULL if openlcb_node is NULL.
     * -# If the node already has train_state set, return the existing pointer.
     * -# Return NULL if the pool is exhausted.
     * -# Take the next pool slot, zero it, and store a pointer in openlcb_node->train_state.
     * -# Set state->owner_node back to the node.
     * -# Register the standard train event IDs: Train producer, Emergency Off/Stop consumers,
     *    Clear Emergency Off/Stop consumers.
     * -# Return the new state pointer.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t to configure as a train.
     * @endverbatim
     *
     * @return Pointer to the train_state_t, or NULL if the node is NULL or the pool is full.
     */
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
    state->owner_node = openlcb_node;

    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_TRAIN, EVENT_STATUS_SET);
    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_EMERGENCY_OFF, EVENT_STATUS_SET);
    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_SET);
    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_OFF, EVENT_STATUS_SET);
    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_STOP, EVENT_STATUS_SET);

    return state;

}


    /**
     * @brief Returns the train state for a node.
     *
     * @details Algorithm:
     * -# Return NULL if openlcb_node is NULL.
     * -# Return openlcb_node->train_state.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @endverbatim
     *
     * @return Pointer to the train_state_t, or NULL.
     */
train_state_t* OpenLcbApplicationTrain_get_state(openlcb_node_t *openlcb_node) {

    if (!openlcb_node) {

        return NULL;

    }

    return openlcb_node->train_state;

}


    /**
     * @brief Sends a NOOP heartbeat request to the controller assigned to a train.
     *
     * @details Algorithm:
     * -# Bail out if state->owner_node, _interface, or send_openlcb_msg is NULL.
     * -# Bail out if state->controller_node_id == 0 (no controller assigned).
     * -# Build a Train Reply message (MTI_TRAIN_REPLY) addressed to controller_node_id.
     * -# Set payload bytes 0-1 to TRAIN_MANAGEMENT / TRAIN_MGMT_NOOP.
     * -# Set payload bytes 2-4 to the 3-byte heartbeat_timeout_s value.
     * -# Call _interface->send_openlcb_msg().
     *
     * @verbatim
     * @param state  Pointer to the train_state_t to send the request for.
     * @endverbatim
     */
static void _send_heartbeat_request(train_state_t *state) {

    openlcb_node_t *node = state->owner_node;

    if (!node || !_interface || !_interface->send_openlcb_msg) { return; }

    if (state->controller_node_id == 0) { return; }

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) &payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &msg,
            node->alias,
            node->id,
            0,
            state->controller_node_id,
            MTI_TRAIN_REPLY);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MGMT_NOOP, 1);

    uint32_t timeout = state->heartbeat_timeout_s;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (timeout >> 16) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (timeout >> 8) & 0xFF, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, timeout & 0xFF, 4);

    _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Decrements the heartbeat countdown for all active train nodes.
     *
     * @details Algorithm:
     * -# For each pool slot with heartbeat_timeout_s > 0:
     *    - Decrement heartbeat_counter_100ms if > 0.
     *    - At the halfway point, call _send_heartbeat_request() to ping the controller.
     *    - At zero, set estop_active = true, set_speed = 0, and fire on_heartbeat_timeout.
     */
void OpenLcbApplicationTrain_100ms_timer_tick(void) {

    for (uint8_t i = 0; i < _train_pool_count; i++) {

        train_state_t *state = &_train_pool[i];

        if (state->heartbeat_timeout_s == 0) {

            continue;

        }

        if (state->heartbeat_counter_100ms > 0) {

            state->heartbeat_counter_100ms--;

        }

        uint32_t halfway = (state->heartbeat_timeout_s * 10) / 2;

        if (state->heartbeat_counter_100ms == halfway) {

            _send_heartbeat_request(state);

        }

        if (state->heartbeat_counter_100ms == 0) {

            state->estop_active = true;
            state->set_speed = 0;

            if (_interface && _interface->on_heartbeat_timeout) {

                _interface->on_heartbeat_timeout(state->owner_node);

            }

        }

    }

}


    /**
     * @brief Builds a train command message header and validates prerequisites.
     *
     * @details Algorithm:
     * -# Return false if openlcb_node, _interface, or send_openlcb_msg is NULL.
     * -# Set msg->payload and msg->payload_type.
     * -# Call OpenLcbUtilities_load_openlcb_message() with MTI_TRAIN_PROTOCOL.
     * -# Return true.
     *
     * @verbatim
     * @param msg             Pointer to the openlcb_msg_t to fill in.
     * @param payload         Pointer to the payload_basic_t to use as the message payload.
     * @param openlcb_node    Pointer to the sending openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     *
     * @return true if the message is ready to fill in, false if a prerequisite is missing.
     */
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

    /**
     * @brief Sends a Set Speed/Direction command to a train node.
     *
     * @details Algorithm:
     * -# Call _prepare_train_command(); return if it fails.
     * -# Set payload byte 0 to TRAIN_SET_SPEED_DIRECTION.
     * -# Set payload bytes 1-2 to the 16-bit speed value.
     * -# Call send_openlcb_msg().
     *
     * @verbatim
     * @param openlcb_node    Pointer to the sending openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @param speed           16-bit speed/direction in OpenLCB float16 format.
     * @endverbatim
     */
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

    /**
     * @brief Sends a Set Function command to a train node.
     *
     * @details Algorithm:
     * -# Call _prepare_train_command(); return if it fails.
     * -# Set payload byte 0 to TRAIN_SET_FUNCTION.
     * -# Set payload bytes 1-3 to the 24-bit function address.
     * -# Set payload bytes 4-5 to the 16-bit function value.
     * -# Call send_openlcb_msg().
     *
     * @verbatim
     * @param openlcb_node    Pointer to the sending openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @param fn_address      24-bit function address.
     * @param fn_value        16-bit function value.
     * @endverbatim
     */
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

    /**
     * @brief Sends an Emergency Stop command to a train node.
     *
     * @details Algorithm:
     * -# Call _prepare_train_command(); return if it fails.
     * -# Set payload byte 0 to TRAIN_EMERGENCY_STOP.
     * -# Call send_openlcb_msg().
     *
     * @verbatim
     * @param openlcb_node    Pointer to the sending openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
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

    /**
     * @brief Sends a Query Speeds command to a train node.
     *
     * @details Algorithm:
     * -# Call _prepare_train_command(); return if it fails.
     * -# Set payload byte 0 to TRAIN_QUERY_SPEEDS.
     * -# Call send_openlcb_msg().
     *
     * @verbatim
     * @param openlcb_node    Pointer to the sending openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
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

    /**
     * @brief Sends a Query Function command to a train node.
     *
     * @details Algorithm:
     * -# Call _prepare_train_command(); return if it fails.
     * -# Set payload byte 0 to TRAIN_QUERY_FUNCTION.
     * -# Set payload bytes 1-3 to the 24-bit function address.
     * -# Call send_openlcb_msg().
     *
     * @verbatim
     * @param openlcb_node    Pointer to the sending openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @param fn_address      24-bit function address to query.
     * @endverbatim
     */
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

    /**
     * @brief Sends an Assign Controller command to a train node.
     *
     * @details Algorithm:
     * -# Call _prepare_train_command(); return if it fails.
     * -# Set payload byte 0 to TRAIN_CONTROLLER_CONFIG, byte 1 to TRAIN_CONTROLLER_ASSIGN.
     * -# Set payload bytes 2-7 to openlcb_node->id (the throttle's Node ID).
     * -# Call send_openlcb_msg().
     *
     * @verbatim
     * @param openlcb_node    Pointer to the sending (throttle) openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
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

    /**
     * @brief Sends a Release Controller command to a train node.
     *
     * @details Algorithm:
     * -# Call _prepare_train_command(); return if it fails.
     * -# Set payload byte 0 to TRAIN_CONTROLLER_CONFIG, byte 1 to TRAIN_CONTROLLER_RELEASE.
     * -# Set payload bytes 2-7 to openlcb_node->id (the throttle's Node ID).
     * -# Call send_openlcb_msg().
     *
     * @verbatim
     * @param openlcb_node    Pointer to the sending (throttle) openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
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

    /**
     * @brief Sends a NOOP management command to a train node.
     *
     * @details Algorithm:
     * -# Call _prepare_train_command(); return if it fails.
     * -# Set payload byte 0 to TRAIN_MANAGEMENT, byte 1 to TRAIN_MGMT_NOOP.
     * -# Call send_openlcb_msg().
     *
     * @verbatim
     * @param openlcb_node    Pointer to the sending openlcb_node_t.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
void OpenLcbApplicationTrain_send_noop(openlcb_node_t *openlcb_node, node_id_t train_node_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_node_id)) { return; }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MGMT_NOOP, 1);

    _interface->send_openlcb_msg(&msg);

}


    /**
     * @brief Sets the DCC address and address type for a train node.
     *
     * @details Algorithm:
     * -# Return if openlcb_node or train_state is NULL.
     * -# Store dcc_address and is_long_address in the train state.
     *
     * @verbatim
     * @param openlcb_node    Pointer to the openlcb_node_t.
     * @param dcc_address     DCC address value.
     * @param is_long_address true for long addressing, false for short.
     * @endverbatim
     */
void OpenLcbApplicationTrain_set_dcc_address(openlcb_node_t *openlcb_node, uint16_t dcc_address, bool is_long_address) {

    if (!openlcb_node || !openlcb_node->train_state) {
        
        return; 
    
    }

    openlcb_node->train_state->dcc_address = dcc_address;
    openlcb_node->train_state->is_long_address = is_long_address;

}

    /**
     * @brief Returns the DCC address for a train node.
     *
     * @details Algorithm:
     * -# Return 0 if openlcb_node or train_state is NULL.
     * -# Return train_state->dcc_address.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @endverbatim
     *
     * @return DCC address, or 0 if the node has no train state.
     */
uint16_t OpenLcbApplicationTrain_get_dcc_address(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) { 
        
        return 0; 
    
    }

    return openlcb_node->train_state->dcc_address;

}

    /**
     * @brief Returns true if the train node uses long DCC addressing.
     *
     * @details Algorithm:
     * -# Return false if openlcb_node or train_state is NULL.
     * -# Return train_state->is_long_address.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @endverbatim
     *
     * @return true for long addressing, false otherwise.
     */
bool OpenLcbApplicationTrain_is_long_address(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) { 

        return false;
    
    }

    return openlcb_node->train_state->is_long_address;

}

    /**
     * @brief Sets the speed-step mode for a train node.
     *
     * @details Algorithm:
     * -# Return if openlcb_node or train_state is NULL.
     * -# Store speed_steps in train_state->speed_steps.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @param speed_steps   Speed-step count (14, 28, or 128).
     * @endverbatim
     */
void OpenLcbApplicationTrain_set_speed_steps(openlcb_node_t *openlcb_node, uint8_t speed_steps) {

    if (!openlcb_node || !openlcb_node->train_state) { 
        
        return;
    
    }

    openlcb_node->train_state->speed_steps = speed_steps;

}

    /**
     * @brief Returns the speed-step mode for a train node.
     *
     * @details Algorithm:
     * -# Return 0 if openlcb_node or train_state is NULL.
     * -# Return train_state->speed_steps.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @endverbatim
     *
     * @return Speed-step count, or 0 if the node has no train state.
     */
uint8_t OpenLcbApplicationTrain_get_speed_steps(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) { 
        
        return 0; 
    
    }

    return openlcb_node->train_state->speed_steps;

}
