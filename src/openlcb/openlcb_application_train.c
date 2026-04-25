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
 * @date 25 Apr 2026
 */

#include "openlcb_application_train.h"

#ifdef OPENLCB_COMPILE_TRAIN

#include <stddef.h>
#include <string.h>

#include "openlcb_application.h"
#include "openlcb_defines.h"
#include "openlcb_float16.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"


static train_state_t _train_pool[USER_DEFINED_TRAIN_NODE_COUNT];
static uint8_t _train_pool_count;
static const interface_openlcb_application_train_t *_interface;

    /** @brief Tracks the last tick value to gate heartbeat processing. */
static uint8_t _last_heartbeat_tick = 0;


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
    _last_heartbeat_tick = 0;

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
train_state_t *OpenLcbApplicationTrain_setup(openlcb_node_t *openlcb_node) {

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
train_state_t *OpenLcbApplicationTrain_get_state(openlcb_node_t *openlcb_node) {

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
     * -# Set payload bytes 2-4 to the 3-byte remaining-deadline-in-seconds value
     *    (heartbeat_counter_100ms / 10, rounded up).  Per TrainControlS §6.6 the
     *    argument must be the time the Controller has to reply, not the full
     *    configured heartbeat_timeout_s — the train fires at the halfway point
     *    of the countdown so only the remainder is available before timeout.
     * -# Call _interface->send_openlcb_msg() and return its result.
     *
     * @verbatim
     * @param state  Pointer to the train_state_t to send the request for.
     * @endverbatim
     *
     * @return true if the send succeeded, false if transport busy or precondition not met.
     */
static bool _send_heartbeat_request(train_state_t *state) {

    openlcb_node_t *node = state->owner_node;

    if (!node || !_interface || !_interface->send_openlcb_msg) {

        return false;

    }

    if (state->controller_node_id == 0) {

        return false;

    }

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) &payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(&msg, node->alias, node->id, state->controller_alias, state->controller_node_id, MTI_TRAIN_REPLY);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MGMT_NOOP, 1);

    // TrainControlS §6.6: the deadline argument is the time the Controller
    // has to reply, NOT the train's full configured heartbeat period.  Since
    // we fire at the halfway point of the countdown, only the remaining
    // counter time is available before the internal timeout-handler runs.
    // Send remaining time (rounded up so a partial tick still leaves the
    // controller a full second's worth of headroom).
    uint32_t remaining_s = (state->heartbeat_counter_100ms + 9) / 10;
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (remaining_s >> 16) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (remaining_s >> 8) & 0xFF, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, remaining_s & 0xFF, 4);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Forwards a Set Speed 0 (with P bit) to one listener.
     *
     * @details Sends the e-stop to the listener at state->listener_enum_index.
     * Called repeatedly by the timer tick run-loop until all listeners have
     * been notified.  Per TrainControlS 6.6 the implied Set Speed 0 "shall be
     * forwarded to all registered Listeners at the same time, including the
     * Controller node, if it is registered as a Listener."
     *
     * @verbatim
     * @param state  Pointer to the train_state_t whose listener receives the forward.
     * @endverbatim
     *
     * @return true if the send succeeded, false if transport busy or precondition not met.
     */
static bool _forward_estop_to_one_listener(train_state_t *state) {

    if (!state || !_interface || !_interface->send_openlcb_msg) {

        return false;

    }

    openlcb_node_t *node = state->owner_node;

    if (!node || state->listener_enum_index >= state->listener_count) {

        return false;

    }

    train_listener_entry_t *entry = &state->listeners[state->listener_enum_index];

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) &payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(&msg, node->alias, node->id, 0, entry->node_id, MTI_TRAIN_PROTOCOL);

    // Speed is already zeroed with direction preserved in state->set_speed
    uint16_t speed = state->set_speed;

    if (entry->flags & TRAIN_LISTENER_FLAG_REVERSE) {

        speed ^= 0x8000;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg,
            TRAIN_SET_SPEED_DIRECTION | TRAIN_INSTRUCTION_P_BIT, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(&msg, speed, 1);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Decrements the heartbeat countdown for all active train nodes.
     *
     * @details Algorithm:
     * -# Compute ticks elapsed since last call via subtraction.
     * -# Skip if no time has elapsed (deduplication).
     * -# For each pool slot with heartbeat_timeout_s > 0:
     *    - Retry any pending heartbeat send or e-stop listener forwarding.
     *    - Decrement heartbeat_counter_100ms by ticks_elapsed (saturate at 0).
     *    - At the halfway point, attempt _send_heartbeat_request(); if the
     *      transport is busy, set heartbeat_send_pending for retry next tick.
     *    - At zero, set estop_active = true, zero set_speed preserving direction,
     *      set estop_forward_pending to forward Set Speed 0 to each listener
     *      one per tick, and fire on_heartbeat_timeout.
     *
     * @verbatim
     * @param current_tick  Current value of the global 100ms tick counter.
     * @endverbatim
     */
void OpenLcbApplicationTrain_100ms_timer_tick(uint8_t current_tick) {

    uint8_t ticks_elapsed = (uint8_t)(current_tick - _last_heartbeat_tick);

    if (ticks_elapsed == 0) {

        return;

    }

    _last_heartbeat_tick = current_tick;

    for (int i = 0; i < _train_pool_count; i++) {

        train_state_t *state = &_train_pool[i];

        if (state->heartbeat_timeout_s == 0) {

            continue;

        }

        // Retry pending heartbeat send from a previous tick.
        // TrainControlS §6.6: drop the pending send if set_speed has gone
        // to zero between the original attempt and this retry — a stopped
        // train no longer needs to verify controller liveness.
        if (state->heartbeat_send_pending) {

            if (OpenLcbFloat16_is_zero(state->set_speed)) {

                state->heartbeat_send_pending = 0;

            } else if (_send_heartbeat_request(state)) {

                state->heartbeat_send_pending = 0;

            }

        }

        // Retry pending e-stop listener forwarding from a previous tick
        if (state->estop_forward_pending) {

            if (_forward_estop_to_one_listener(state)) {

                state->listener_enum_index++;

            }

            if (state->listener_enum_index >= state->listener_count) {

                state->estop_forward_pending = 0;

            }

            // Don't process countdown while forwarding is in progress
            continue;

        }

        uint32_t old_counter = state->heartbeat_counter_100ms;

        if (state->heartbeat_counter_100ms > ticks_elapsed) {

            state->heartbeat_counter_100ms -= ticks_elapsed;

        } else {

            state->heartbeat_counter_100ms = 0;

        }

        uint32_t halfway = (state->heartbeat_timeout_s * 10) / 2;

        // TrainControlS §6.6: do not initiate a Heartbeat Request when the
        // last Set Speed is zero (including the Emergency Stop state).  A
        // stationary train doesn't need to verify controller liveness — the
        // protocol's safety case is preventing a moving train from running
        // away if the throttle disappears.  Both signed-zero values
        // (FLOAT16_POSITIVE_ZERO / FLOAT16_NEGATIVE_ZERO) count as zero.
        bool speed_is_zero = OpenLcbFloat16_is_zero(state->set_speed);

        if (old_counter > halfway && state->heartbeat_counter_100ms <= halfway && !speed_is_zero) {

            if (!_send_heartbeat_request(state)) {

                state->heartbeat_send_pending = 1;

            }

        }

        if (state->heartbeat_counter_100ms == 0 && old_counter > 0) {

            state->estop_active = true;

            // Preserve direction, set speed magnitude to zero
            bool reverse = OpenLcbFloat16_get_direction(state->set_speed);
            state->set_speed = reverse ? FLOAT16_NEGATIVE_ZERO : FLOAT16_POSITIVE_ZERO;

            // TrainControlS 6.6: forward the implied Set Speed 0 to all
            // registered Listeners, including the Controller if it is a Listener.
            if (state->listener_count > 0) {

                state->estop_forward_pending = 1;
                state->listener_enum_index = 0;

            }

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     *
     * @return true if the message is ready to fill in, false if a prerequisite is missing.
     */
static bool _prepare_train_command(openlcb_msg_t *msg, payload_basic_t *payload, openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    if (!openlcb_node || !_interface || !_interface->send_openlcb_msg) {

        return false;

    }

    msg->payload = (openlcb_payload_t *) payload;
    msg->payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(msg, openlcb_node->alias, openlcb_node->id, train_alias, train_node_id, MTI_TRAIN_PROTOCOL);

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @param speed           16-bit speed/direction in OpenLCB float16 format.
     * @endverbatim
     */
bool OpenLcbApplicationTrain_send_set_speed(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id, uint16_t speed) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_SET_SPEED_DIRECTION, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(&msg, speed, 1);

    return _interface->send_openlcb_msg(&msg);

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @param fn_address      24-bit function address.
     * @param fn_value        16-bit function value.
     * @endverbatim
     */
bool OpenLcbApplicationTrain_send_set_function(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id, uint32_t fn_address, uint16_t fn_value) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_SET_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (fn_address >> 16) & 0xFF, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (fn_address >> 8) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, fn_address & 0xFF, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(&msg, fn_value, 4);

    return _interface->send_openlcb_msg(&msg);

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
bool OpenLcbApplicationTrain_send_emergency_stop(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_EMERGENCY_STOP, 0);

    return _interface->send_openlcb_msg(&msg);

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
bool OpenLcbApplicationTrain_send_query_speeds(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_QUERY_SPEEDS, 0);

    return _interface->send_openlcb_msg(&msg);

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @param fn_address      24-bit function address to query.
     * @endverbatim
     */
bool OpenLcbApplicationTrain_send_query_function(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id, uint32_t fn_address) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (fn_address >> 16) & 0xFF, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, (fn_address >> 8) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, fn_address & 0xFF, 3);

    return _interface->send_openlcb_msg(&msg);

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
bool OpenLcbApplicationTrain_send_assign_controller(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(&msg, openlcb_node->id, 2);

    return _interface->send_openlcb_msg(&msg);

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
bool OpenLcbApplicationTrain_send_release_controller(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_RELEASE, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(&msg, openlcb_node->id, 2);

    return _interface->send_openlcb_msg(&msg);

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
     * @param train_alias     12-bit CAN alias of the target train node.
     * @param train_node_id   48-bit node_id_t of the target train node.
     * @endverbatim
     */
bool OpenLcbApplicationTrain_send_noop(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MGMT_NOOP, 1);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Sends a Query Controller command to a train node.
     *
     * @details Payload per TrainControlS §4.3: byte 0 = TRAIN_CONTROLLER_CONFIG (0x20),
     * byte 1 = TRAIN_CONTROLLER_QUERY (0x03).
     */
bool OpenLcbApplicationTrain_send_query_controller(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_QUERY, 1);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Sends a Controller Changing Notify request to a train node.
     *
     * @details Payload per TrainControlS §4.3: byte 0 = TRAIN_CONTROLLER_CONFIG (0x20),
     * byte 1 = TRAIN_CONTROLLER_CHANGED (0x04), byte 2 = flags (reserved, 0),
     * bytes 3-8 = new (requesting) controller Node ID.
     */
bool OpenLcbApplicationTrain_send_controller_changing_notify(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id, node_id_t new_controller_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(&msg, new_controller_node_id, 3);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Sends a Listener Attach (or Update Flags) command to a train node.
     *
     * @details Payload per TrainControlS §4.3: byte 0 = TRAIN_LISTENER_CONFIG (0x30),
     * byte 1 = TRAIN_LISTENER_ATTACH (0x01), byte 2 = flags, bytes 3-8 = listener Node ID.
     */
bool OpenLcbApplicationTrain_send_listener_attach(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id, node_id_t listener_node_id, uint8_t flags) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, flags, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(&msg, listener_node_id, 3);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Sends a Listener Detach command to a train node.
     *
     * @details Payload per TrainControlS §4.3: byte 0 = TRAIN_LISTENER_CONFIG (0x30),
     * byte 1 = TRAIN_LISTENER_DETACH (0x02), byte 2 = flags (reserved, 0),
     * bytes 3-8 = listener Node ID.
     */
bool OpenLcbApplicationTrain_send_listener_detach(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id, node_id_t listener_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, 0x00, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(&msg, listener_node_id, 3);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Sends a Listener Query command to a train node.
     *
     * @details Payload per TrainControlS §4.3: byte 0 = TRAIN_LISTENER_CONFIG (0x30),
     * byte 1 = TRAIN_LISTENER_QUERY (0x03), byte 2 = listener index.
     * The receive-side handler always reads byte 2, so the index is always sent
     * even though the standard marks it optional.
     */
bool OpenLcbApplicationTrain_send_listener_query(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id, uint8_t listener_index) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, listener_index, 2);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Sends a Reserve command to a train node.
     *
     * @details Payload per TrainControlS §4.3: byte 0 = TRAIN_MANAGEMENT (0x40),
     * byte 1 = TRAIN_MGMT_RESERVE (0x01).
     */
bool OpenLcbApplicationTrain_send_reserve(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MGMT_RESERVE, 1);

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Sends a Release Reserve command to a train node.
     *
     * @details Payload per TrainControlS §4.3: byte 0 = TRAIN_MANAGEMENT (0x40),
     * byte 1 = TRAIN_MGMT_RELEASE (0x02).
     */
bool OpenLcbApplicationTrain_send_release_reserve(openlcb_node_t *openlcb_node, uint16_t train_alias, node_id_t train_node_id) {

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    if (!_prepare_train_command(&msg, &payload, openlcb_node, train_alias, train_node_id)) {

        return false;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, TRAIN_MGMT_RELEASE, 1);

    return _interface->send_openlcb_msg(&msg);

}


    /**
     * @brief Sends a Producer Identified Set reply echoing a train-search event id.
     *
     * @details Used by the CS application after allocating a new train node in
     * response to on_search_no_match_with_allocate.  The new train node announces itself by
     * echoing the query event id back to the network.
     */
bool OpenLcbApplicationTrain_send_search_match(openlcb_node_t *openlcb_node, event_id_t search_event_id) {

    if (!openlcb_node || !_interface || !_interface->send_openlcb_msg) {

        return false;

    }

    openlcb_msg_t msg = {0};
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) &payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(&msg, openlcb_node->alias, openlcb_node->id, 0, 0, MTI_PRODUCER_IDENTIFIED_SET);
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(&msg, search_event_id);

    return _interface->send_openlcb_msg(&msg);

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

    /**
     * @brief Configures the heartbeat-monitor deadline for a train node.
     *
     * @details Algorithm:
     * -# Return if openlcb_node or train_state is NULL.
     * -# Store seconds in train_state->heartbeat_timeout_s.
     * -# Reset train_state->heartbeat_counter_100ms to seconds * 10 so the
     *    countdown restarts cleanly from this configuration call (avoids a
     *    spurious early Heartbeat Request fired by a stale partial countdown
     *    when the timeout is being raised).  When seconds is zero, also
     *    clear the counter so the per-tick handler skips this slot.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @param seconds       Reply deadline in seconds (0 disables monitoring).
     * @endverbatim
     */
void OpenLcbApplicationTrain_set_heartbeat_timeout(openlcb_node_t *openlcb_node, uint32_t seconds) {

    if (!openlcb_node || !openlcb_node->train_state) {

        return;

    }

    openlcb_node->train_state->heartbeat_timeout_s = seconds;
    openlcb_node->train_state->heartbeat_counter_100ms = seconds * 10;

}

    /**
     * @brief Returns the configured heartbeat deadline for a train node.
     *
     * @details Algorithm:
     * -# Return 0 if openlcb_node or train_state is NULL.
     * -# Return train_state->heartbeat_timeout_s.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @endverbatim
     *
     * @return Configured deadline in seconds, or 0 if disabled / no train state.
     */
uint32_t OpenLcbApplicationTrain_get_heartbeat_timeout(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) {

        return 0;

    }

    return openlcb_node->train_state->heartbeat_timeout_s;

}

    /**
     * @brief Returns the Node ID currently holding the train's reservation.
     *
     * @details Algorithm:
     * -# Return 0 if openlcb_node or train_state is NULL.
     * -# Return 0 if reserved_node_count is zero (no active reservation).
     * -# Return train_state->reserved_by_node_id.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @endverbatim
     *
     * @return Reserving node ID, or 0 if no reservation is held.
     */
node_id_t OpenLcbApplicationTrain_get_reserved_by_node_id(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) {

        return 0;

    }

    if (openlcb_node->train_state->reserved_node_count == 0) {

        return 0;

    }

    return openlcb_node->train_state->reserved_by_node_id;

}

    /**
     * @brief Returns the number of listener nodes attached to a train.
     *
     * @details Algorithm:
     * -# Return 0 if openlcb_node or train_state is NULL.
     * -# Return train_state->listener_count.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @endverbatim
     *
     * @return Listener count, or 0 if the node has no train state.
     */
uint8_t OpenLcbApplicationTrain_get_listener_count(openlcb_node_t *openlcb_node) {

    if (!openlcb_node || !openlcb_node->train_state) {

        return 0;

    }

    return openlcb_node->train_state->listener_count;

}

    /**
     * @brief Reads one listener entry from a train's listener list.
     *
     * @details Algorithm:
     * -# Return false if openlcb_node, train_state, out_node_id, or out_flags is NULL.
     * -# Return false if index is greater than or equal to listener_count.
     * -# Copy listeners[index].node_id into *out_node_id.
     * -# Copy listeners[index].flags into *out_flags.
     * -# Return true.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t.
     * @param index         Zero-based listener slot.
     * @param out_node_id   Pointer to receive the listener's node_id_t.
     * @param out_flags     Pointer to receive the listener's flag byte.
     * @endverbatim
     *
     * @return true on success, false on out-of-range index or NULL inputs.
     */
bool OpenLcbApplicationTrain_get_listener_at(openlcb_node_t *openlcb_node, uint8_t index, node_id_t *out_node_id, uint8_t *out_flags) {

    if (!openlcb_node || !openlcb_node->train_state) {

        return false;

    }

    if (!out_node_id || !out_flags) {

        return false;

    }

    if (index >= openlcb_node->train_state->listener_count) {

        return false;

    }

    *out_node_id = openlcb_node->train_state->listeners[index].node_id;
    *out_flags = openlcb_node->train_state->listeners[index].flags;

    return true;

}

#endif /* OPENLCB_COMPILE_TRAIN */
