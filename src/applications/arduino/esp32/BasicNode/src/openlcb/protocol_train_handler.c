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
 * @file protocol_train_handler.c
 * @brief Train Control Protocol message handler implementation (Layer 1)
 *
 * @details Handles incoming MTI_TRAIN_PROTOCOL commands and MTI_TRAIN_REPLY
 * replies. Automatically updates train_state, builds protocol replies,
 * forwards consist commands to listeners, and fires optional notifier
 * callbacks.
 *
 * @author Jim Kueneman
 * @date 17 Feb 2026
 */

#include "protocol_train_handler.h"

#include <stddef.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_float16.h"


static const interface_protocol_train_handler_t *_interface;


void ProtocolTrainHandler_initialize(const interface_protocol_train_handler_t *interface) {

    _interface = interface;

}

// ============================================================================
// Listener management
// ============================================================================

bool ProtocolTrainHandler_attach_listener(train_state_t *state, node_id_t node_id, uint8_t flags)
{

    if (!state || node_id == 0) {

        return false;

    }

    // Check if already attached — update flags if so
    for (uint8_t i = 0; i < state->listener_count; i++) {

        if (state->listeners[i].node_id == node_id) {

            state->listeners[i].flags = flags;
            return true;

        }

    }

    // Check for capacity
    if (state->listener_count >= USER_DEFINED_MAX_LISTENERS_PER_TRAIN) {

        return false;

    }

    state->listeners[state->listener_count].node_id = node_id;
    state->listeners[state->listener_count].flags = flags;
    state->listener_count++;

    return true;

}

bool ProtocolTrainHandler_detach_listener(train_state_t *state, node_id_t node_id) {

    if (!state || node_id == 0) {

        return false;

    }

    for (uint8_t i = 0; i < state->listener_count; i++) {

        if (state->listeners[i].node_id == node_id) {

            // Shift remaining entries down
            for (uint8_t j = i; j < state->listener_count - 1; j++) {

                state->listeners[j] = state->listeners[j + 1];

            }

            state->listener_count--;

            // Clear the vacated slot
            state->listeners[state->listener_count].node_id = 0;
            state->listeners[state->listener_count].flags = 0;

            return true;

        }

    }

    return false;

}

train_listener_entry_t* ProtocolTrainHandler_find_listener(
        train_state_t *state, node_id_t node_id) {

    if (!state || node_id == 0) {

        return NULL;

    }

    for (uint8_t i = 0; i < state->listener_count; i++) {

        if (state->listeners[i].node_id == node_id) {

            return &state->listeners[i];

        }

    }

    return NULL;

}

uint8_t ProtocolTrainHandler_get_listener_count(train_state_t *state) {

    if (!state) {

        return 0;

    }

    return state->listener_count;

}

train_listener_entry_t* ProtocolTrainHandler_get_listener_by_index(train_state_t *state, uint8_t index) {

    if (!state || index >= state->listener_count) {

        return NULL;

    }

    return &state->listeners[index];

}


// ============================================================================
// Reply builder helpers (static — only used internally)
// ============================================================================

static void _load_reply_header(openlcb_statemachine_info_t *statemachine_info) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_TRAIN_REPLY);

}

static void _load_query_speeds_reply(openlcb_statemachine_info_t *statemachine_info, uint16_t set_speed, uint8_t status, uint16_t commanded_speed, uint16_t actual_speed) {

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_QUERY_SPEEDS, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, set_speed, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, status, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, commanded_speed, 4);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, actual_speed, 6);

    statemachine_info->outgoing_msg_info.valid = true;

}

static void _load_query_function_reply(openlcb_statemachine_info_t *statemachine_info, uint32_t fn_address, uint16_t fn_value) {

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_QUERY_FUNCTION, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, (fn_address >> 16) & 0xFF, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, (fn_address >> 8) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, fn_address & 0xFF, 3);
    OpenLcbUtilities_copy_word_to_openlcb_payload(msg, fn_value, 4);

    statemachine_info->outgoing_msg_info.valid = true;

}

static void _load_controller_assign_reply(openlcb_statemachine_info_t *statemachine_info, uint8_t result) {

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, result, 2);

    statemachine_info->outgoing_msg_info.valid = true;

}

static void _load_controller_query_reply(openlcb_statemachine_info_t *statemachine_info, uint8_t flags, node_id_t controller_node_id)
{

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, flags, 2);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(msg, controller_node_id, 3);

    statemachine_info->outgoing_msg_info.valid = true;
}

static void _load_controller_changed_reply(openlcb_statemachine_info_t *statemachine_info, uint8_t result) {

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_CHANGED, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, result, 2);

    statemachine_info->outgoing_msg_info.valid = true;

}

static void _load_listener_attach_reply(openlcb_statemachine_info_t *statemachine_info,node_id_t node_id, uint8_t result)
{

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_LISTENER_ATTACH, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(msg, node_id, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, result, 8);

    statemachine_info->outgoing_msg_info.valid = true;
}

static void _load_listener_detach_reply(openlcb_statemachine_info_t *statemachine_info, node_id_t node_id, uint8_t result)
{

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_LISTENER_DETACH, 1);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(msg, node_id, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, result, 8);

    statemachine_info->outgoing_msg_info.valid = true;
}

static void _load_listener_query_reply(openlcb_statemachine_info_t *statemachine_info, uint8_t count, uint8_t index, uint8_t flags, node_id_t node_id)
{

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_LISTENER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_LISTENER_QUERY, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, count, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, index, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, flags, 4);
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(msg, node_id, 5);

    statemachine_info->outgoing_msg_info.valid = true;
}

static void _load_reserve_reply(openlcb_statemachine_info_t *statemachine_info, uint8_t result) {

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_MANAGEMENT, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_MGMT_RESERVE, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, result, 2);

    statemachine_info->outgoing_msg_info.valid = true;

}


// ============================================================================
// Payload extraction helpers
// ============================================================================

static uint32_t _extract_fn_address(openlcb_msg_t *msg, uint16_t offset) {

    uint32_t addr;

    addr = ((uint32_t) OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, offset) << 16) |
           ((uint32_t) OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, offset + 1) << 8) |
           (uint32_t) OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, offset + 2);

    return addr;

}


// ============================================================================
// Command handlers (train-node side, MTI 0x05EB)
// ============================================================================

static void _handle_set_speed(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_node_t *node = statemachine_info->openlcb_node;
    train_state_t *state = statemachine_info->openlcb_node->train_state;

    uint16_t speed = OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 1);

    if (state) {

        state->set_speed = speed;
        state->estop_active = 0;

    }

    if (_interface && _interface->on_speed_changed) {

        _interface->on_speed_changed(node, speed);

    }

}

static void _handle_set_function(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_node_t *node = statemachine_info->openlcb_node;
    train_state_t *state = statemachine_info->openlcb_node->train_state;

    uint32_t fn_address = _extract_fn_address(statemachine_info->incoming_msg_info.msg_ptr, 1);
    uint16_t fn_value = OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 4);

    if (state && fn_address < USER_DEFINED_MAX_TRAIN_FUNCTIONS) {

        state->functions[fn_address] = fn_value;

    }

    if (_interface && _interface->on_function_changed) {

        _interface->on_function_changed(node, fn_address, fn_value);

    }

}

static void _handle_emergency_stop(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_node_t *node = statemachine_info->openlcb_node;
    train_state_t *state = statemachine_info->openlcb_node->train_state;

    if (state) {

        state->estop_active = 1;

        // Preserve direction, set speed magnitude to zero
        bool reverse = OpenLcbFloat16_get_direction(state->set_speed);
        state->set_speed = reverse ? FLOAT16_NEGATIVE_ZERO : FLOAT16_POSITIVE_ZERO;

    }

    if (_interface && _interface->on_emergency_stopped) {

        _interface->on_emergency_stopped(node);

    }

}

static void _handle_query_speeds(openlcb_statemachine_info_t *statemachine_info) {

    train_state_t *state = statemachine_info->openlcb_node->train_state;

    uint16_t set_speed = 0;
    uint8_t status = 0;
    uint16_t commanded_speed = FLOAT16_NAN;
    uint16_t actual_speed = FLOAT16_NAN;

    if (state) {

        set_speed = state->set_speed;
        status = (state->estop_active || state->global_estop_active || state->global_eoff_active) ? 0x01 : 0x00;
        commanded_speed = state->commanded_speed;
        actual_speed = state->actual_speed;

    }

    _load_query_speeds_reply(statemachine_info, set_speed, status, commanded_speed, actual_speed);

}

static void _handle_query_function(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_node_t *node = statemachine_info->openlcb_node;
    train_state_t *state = statemachine_info->openlcb_node->train_state;

    uint32_t fn_address = _extract_fn_address(statemachine_info->incoming_msg_info.msg_ptr, 1);

    uint16_t fn_value = 0;

    if (state && fn_address < USER_DEFINED_MAX_TRAIN_FUNCTIONS) {

        fn_value = state->functions[fn_address];

    }

    if (_interface && _interface->on_query_function_request) {

        fn_value = _interface->on_query_function_request(node, fn_address);

    }

    _load_query_function_reply(statemachine_info, fn_address, fn_value);

}

static void _handle_controller_config(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *msg = statemachine_info->incoming_msg_info.msg_ptr;
    openlcb_node_t *node = statemachine_info->openlcb_node;
    train_state_t *state = statemachine_info->openlcb_node->train_state;

    uint8_t sub_cmd = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 1);

    switch (sub_cmd) {

        case TRAIN_CONTROLLER_ASSIGN: {

            node_id_t requesting_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 2);
            uint8_t result = 0;

            if (state) {

                if (state->controller_node_id == 0 || state->controller_node_id == requesting_id) {

                    // No current controller, or same controller — accept
                    state->controller_node_id = requesting_id;

                } else {

                    // Different controller — ask app or default accept
                    if (_interface && _interface->on_controller_assign_request) {

                        result = _interface->on_controller_assign_request(node, state->controller_node_id, requesting_id);

                    }

                    if (result == 0) {

                        state->controller_node_id = requesting_id;

                    }

                }

            }

            _load_controller_assign_reply(statemachine_info, result);

            if (result == 0 && _interface && _interface->on_controller_assigned) {

                _interface->on_controller_assigned(node, requesting_id);

            }

            break;

        }

        case TRAIN_CONTROLLER_RELEASE: {

            node_id_t releasing_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 2);

            if (state && state->controller_node_id == releasing_id) {

                state->controller_node_id = 0;

                if (_interface && _interface->on_controller_released) {

                    _interface->on_controller_released(node);

                }

            }

            break;

        }

        case TRAIN_CONTROLLER_QUERY: {

            node_id_t ctrl_id = 0;
            uint8_t flags = 0;

            if (state) {

                ctrl_id = state->controller_node_id;

                if (ctrl_id != 0) {

                    flags = 0x01;

                }

            }

            _load_controller_query_reply(statemachine_info, flags, ctrl_id);

            break;

        }

        case TRAIN_CONTROLLER_CHANGED: {

            node_id_t new_controller_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 2);
            uint8_t result = 0;

            if (_interface && _interface->on_controller_changed_request) {

                result = _interface->on_controller_changed_request(node, new_controller_id);

            }

            _load_controller_changed_reply(statemachine_info, result);

            break;

        }

        default:
            break;

    }

}

static void _handle_listener_config(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *msg = statemachine_info->incoming_msg_info.msg_ptr;
    openlcb_node_t *node = statemachine_info->openlcb_node;
    train_state_t *state = statemachine_info->openlcb_node->train_state;

    uint8_t sub_cmd = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 1);

    switch (sub_cmd) {

        case TRAIN_LISTENER_ATTACH: {

            uint8_t flags = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 2);
            node_id_t listener_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 3);
            uint8_t result = 0;

            if (state) {

                bool ok = ProtocolTrainHandler_attach_listener(state, listener_id, flags);

                if (!ok) {

                    result = 0xFF;

                }

            } else {

                result = 0xFF;

            }

            _load_listener_attach_reply(statemachine_info, listener_id, result);

            if (result == 0 && _interface && _interface->on_listener_changed) {

                _interface->on_listener_changed(node);

            }

            break;

        }

        case TRAIN_LISTENER_DETACH: {

            uint8_t flags = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 2);
            node_id_t listener_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 3);
            uint8_t result = 0;

            (void) flags;

            if (state) {

                bool ok = ProtocolTrainHandler_detach_listener(state, listener_id);

                if (!ok) {

                    result = 0xFF;

                }

            } else {

                result = 0xFF;

            }

            _load_listener_detach_reply(statemachine_info, listener_id, result);

            if (result == 0 && _interface && _interface->on_listener_changed) {

                _interface->on_listener_changed(node);

            }

            break;

        }

        case TRAIN_LISTENER_QUERY: {

            // Per spec Section 6.4 / Table 4.3.7: the query command
            // carries byte 2 = NodeCount (ignored on receive) and
            // byte 3 = NodeIndex (the index the caller is requesting).
            // The reply returns the total count, the requested index,
            // and the entry at that index (flags + node_id).

            uint8_t requested_index = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 3);
            uint8_t count = 0;

            if (state) {

                count = ProtocolTrainHandler_get_listener_count(state);

            }

            if (count == 0 || requested_index >= count) {

                // No listeners or index out of range
                _load_listener_query_reply(statemachine_info, count, requested_index, 0, 0);

            } else {

                train_listener_entry_t *entry = ProtocolTrainHandler_get_listener_by_index(state, requested_index);

                if (entry) {

                    _load_listener_query_reply(statemachine_info, count, requested_index, entry->flags, entry->node_id);

                } else {

                    _load_listener_query_reply(statemachine_info, count, requested_index, 0, 0);

                }

            }

            break;

        }

        default:

            break;

    }

}

static void _handle_management(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *msg = statemachine_info->incoming_msg_info.msg_ptr;
    train_state_t *state = statemachine_info->openlcb_node->train_state;

    uint8_t sub_cmd = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 1);

    switch (sub_cmd) {

        case TRAIN_MGMT_RESERVE: {

            // Per conformance test TN 2.10: a second reserve without
            // release shall return a fail code.  Only one reservation
            // at a time is permitted.
            uint8_t result = 0;

            if (state) {

                if (state->reserved_node_count > 0) {

                    result = 0xFF;

                } else {

                    state->reserved_node_count = 1;

                }

            }

            _load_reserve_reply(statemachine_info, result);

            break;

        }

        case TRAIN_MGMT_RELEASE: {

            if (state && state->reserved_node_count > 0) {

                state->reserved_node_count--;

            }

            break;

        }

        case TRAIN_MGMT_NOOP: {

            if (state && state->heartbeat_timeout_s > 0) {

                state->heartbeat_counter_100ms = state->heartbeat_timeout_s * 10;

            }

            break;

        }

        default:

            break;

    }

}


// ============================================================================
// Reply handlers (throttle side, MTI 0x01E9)
// ============================================================================

static void _handle_query_speeds_reply(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface && _interface->on_query_speeds_reply) {

        openlcb_msg_t *msg = statemachine_info->incoming_msg_info.msg_ptr;

        uint16_t set_speed = OpenLcbUtilities_extract_word_from_openlcb_payload(msg, 1);
        uint8_t status = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 3);
        uint16_t commanded_speed = OpenLcbUtilities_extract_word_from_openlcb_payload(msg, 4);
        uint16_t actual_speed = OpenLcbUtilities_extract_word_from_openlcb_payload(msg, 6);

        _interface->on_query_speeds_reply(statemachine_info->openlcb_node, set_speed, status, commanded_speed, actual_speed);

    }

}

static void _handle_query_function_reply(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface && _interface->on_query_function_reply) {

        openlcb_msg_t *msg = statemachine_info->incoming_msg_info.msg_ptr;

        uint32_t fn_address = _extract_fn_address(msg, 1);
        uint16_t fn_value = OpenLcbUtilities_extract_word_from_openlcb_payload(msg, 4);

        _interface->on_query_function_reply(statemachine_info->openlcb_node, fn_address, fn_value);

    }

}

static void _handle_controller_config_reply(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *msg = statemachine_info->incoming_msg_info.msg_ptr;
    openlcb_node_t *node = statemachine_info->openlcb_node;

    uint8_t sub_cmd = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 1);

    switch (sub_cmd) {

        case TRAIN_CONTROLLER_ASSIGN:

            if (_interface && _interface->on_controller_assign_reply) {

                uint8_t result = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 2);
                _interface->on_controller_assign_reply(node, result);

            }

            break;

        case TRAIN_CONTROLLER_QUERY:

            if (_interface && _interface->on_controller_query_reply) {

                uint8_t flags = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 2);
                node_id_t node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 3);
                _interface->on_controller_query_reply(node, flags, node_id);

            }

            break;

        case TRAIN_CONTROLLER_CHANGED:

            if (_interface && _interface->on_controller_changed_notify_reply) {

                uint8_t result = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 2);
                _interface->on_controller_changed_notify_reply(node, result);

            }

            break;

        default:

            break;

    }

}

static void _handle_listener_config_reply(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *msg = statemachine_info->incoming_msg_info.msg_ptr;
    openlcb_node_t *node = statemachine_info->openlcb_node;

    uint8_t sub_cmd = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 1);

    switch (sub_cmd) {

        case TRAIN_LISTENER_ATTACH:

            if (_interface && _interface->on_listener_attach_reply) {

                node_id_t node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 2);
                uint8_t result = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 8);
                _interface->on_listener_attach_reply(node, node_id, result);

            }

            break;

        case TRAIN_LISTENER_DETACH:

            if (_interface && _interface->on_listener_detach_reply) {

                node_id_t node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 2);
                uint8_t result = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 8);
                _interface->on_listener_detach_reply(node, node_id, result);

            }

            break;

        case TRAIN_LISTENER_QUERY:

            if (_interface && _interface->on_listener_query_reply) {

                uint8_t count = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 2);
                uint8_t index = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 3);
                uint8_t flags = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 4);
                node_id_t node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 5);
                _interface->on_listener_query_reply(node, count, index, flags, node_id);

            }

            break;

        default:

            break;

    }

}

static void _handle_management_reply(openlcb_statemachine_info_t *statemachine_info) {

    openlcb_msg_t *msg = statemachine_info->incoming_msg_info.msg_ptr;
    openlcb_node_t *node = statemachine_info->openlcb_node;

    uint8_t sub_cmd = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 1);

    switch (sub_cmd) {

        case TRAIN_MGMT_RESERVE:

            if (_interface && _interface->on_reserve_reply) {

                uint8_t result = OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 2);
                _interface->on_reserve_reply(node, result);

            }

            break;

        case TRAIN_MGMT_NOOP:

            if (_interface && _interface->on_heartbeat_request) {

                uint32_t timeout =
                        ((uint32_t) OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 2) << 16)
                        | ((uint32_t) OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 3) << 8)
                        | (uint32_t) OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 4);

                _interface->on_heartbeat_request(node, timeout);

            }

            break;

        default:

            break;

    }

}


// ============================================================================
// Public dispatch functions
// ============================================================================

void ProtocolTrainHandler_handle_train_command(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info) { return; }

    if (!statemachine_info->incoming_msg_info.msg_ptr) { return; }

    uint8_t instruction = OpenLcbUtilities_extract_byte_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0);

    switch (instruction) {

        case TRAIN_SET_SPEED_DIRECTION:

            _handle_set_speed(statemachine_info);
            break;

        case TRAIN_SET_FUNCTION:

            _handle_set_function(statemachine_info);
            break;

        case TRAIN_EMERGENCY_STOP:

            _handle_emergency_stop(statemachine_info);
            break;

        case TRAIN_QUERY_SPEEDS:

            _handle_query_speeds(statemachine_info);
            break;

        case TRAIN_QUERY_FUNCTION:

            _handle_query_function(statemachine_info);
            break;

        case TRAIN_CONTROLLER_CONFIG:

            _handle_controller_config(statemachine_info);
            break;

        case TRAIN_LISTENER_CONFIG:

            _handle_listener_config(statemachine_info);
            break;

        case TRAIN_MANAGEMENT:

            _handle_management(statemachine_info);
            break;

        default:
            break;

    }

}

void ProtocolTrainHandler_handle_train_reply(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info)
    { 
        
        return; 
    
    }

    if (!statemachine_info->incoming_msg_info.msg_ptr) { 
        
        return; 
    
    }

    uint8_t instruction = OpenLcbUtilities_extract_byte_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0);

    switch (instruction) {

        case TRAIN_QUERY_SPEEDS:

            _handle_query_speeds_reply(statemachine_info);
            break;

        case TRAIN_QUERY_FUNCTION:

            _handle_query_function_reply(statemachine_info);
            break;

        case TRAIN_CONTROLLER_CONFIG:

            _handle_controller_config_reply(statemachine_info);
            break;

        case TRAIN_LISTENER_CONFIG:

            _handle_listener_config_reply(statemachine_info);
            break;

        case TRAIN_MANAGEMENT:

            _handle_management_reply(statemachine_info);
            break;

        default:

            break;

    }

}

void ProtocolTrainHandler_handle_emergency_event(
        openlcb_statemachine_info_t *statemachine_info, event_id_t event_id) {

    if (!statemachine_info) { return; }

    train_state_t *state = statemachine_info->openlcb_node->train_state;

    if (!state) { return; }

    // Per Train Control Standard Section 5 & 6.2:
    //
    // Three independent emergency state machines:
    //   1. Emergency Stop  (point-to-point cmd 0x02 — handled by _handle_emergency_stop)
    //   2. Global Emergency Stop  (event-based — here)
    //   3. Global Emergency Off   (event-based — here)
    //
    // Global Emergency Stop / Off do NOT change Set Speed.
    // The train remains stopped while ANY of the three states is active.
    // Upon exiting ALL emergency states the train resumes at Set Speed.
    //
    // Global Emergency Off additionally de-energizes all other outputs.
    // Upon clearing, outputs restore to their commanded state (functions[]).
    //
    // The handler only manages the flags.  The application layer checks
    // estop_active, global_estop_active, and global_eoff_active when
    // driving hardware (motor, function outputs) and acts accordingly.

    switch (event_id) {

        case EVENT_ID_EMERGENCY_STOP:

            state->global_estop_active = 1;
            break;

        case EVENT_ID_CLEAR_EMERGENCY_STOP:

            state->global_estop_active = 0;
            break;

        case EVENT_ID_EMERGENCY_OFF:

            state->global_eoff_active = 1;
            break;

        case EVENT_ID_CLEAR_EMERGENCY_OFF:

            state->global_eoff_active = 0;
            break;

        default:

            break;

    }

}
