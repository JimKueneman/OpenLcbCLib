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
 * \turnoutboss_event_hander.c
 * 
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory and then registers event IDs required by the board type.
 * It also registers an Event callback and updates the TurnoutBoss_Signaling_States with
 * any received events from nodes of interest (BAL/BAR/BL/BR).
 *
 * @author Jim Kueneman
 * @date 17 Jan 2025
 */

#include <string.h>

#include "turnoutboss_event_handler.h"

#ifdef MPLAB
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application_callbacks.h"
#else
#include "src/openlcb/openlcb_utilities.h"
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/application.h"
#include "src/openlcb/application.h"
#include "src/openlcb/application_callbacks.h"
#endif

#include "turnoutboss_types.h"

board_configuration_t* _event_handler_board_configuration;
signaling_state_t* _event_handler_signaling_state;

void _handle_event_from_board_adjecent_left_for_bl(uint16_olcb_t suffix) {

    switch (suffix) {
        
        case EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED:
        {

            _event_handler_signaling_state->next.occupancy.OML = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED:
        {

            _event_handler_signaling_state->next.occupancy.OML = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_STOP:
        {

            _event_handler_signaling_state->next.stop.ScdBALstop = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP:
        {

            _event_handler_signaling_state->next.stop.ScdBALstop = INACTIVE;
            break;

        }
    }

}

void _handle_event_from_board_to_the_right_for_bl(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_A_STOP:
        {

            _event_handler_signaling_state->next.stop.SaBRstop = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP:
        {

            _event_handler_signaling_state->next.stop.SaBRstop = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_STOP:
        {

            _event_handler_signaling_state->next.stop.SbBRstop = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP:
        {

            _event_handler_signaling_state->next.stop.SbBRstop = INACTIVE;
            break;

        }
    }

}

void _handle_event_from_board_left_for_rb(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_A_STOP:
        {

            _event_handler_signaling_state->next.stop.SaBLstop = ACTIVE;
            break;


        }
        case EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP:
        {

            _event_handler_signaling_state->next.stop.SaBLstop = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_STOP:
        {
            _event_handler_signaling_state->next.stop.SbBLstop = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP:
        {

            _event_handler_signaling_state->next.stop.SbBLstop = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED:
        {

            _event_handler_signaling_state->next.occupancy.OMC = ACTIVE;
            break;

        }

        case EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED:
        {

            _event_handler_signaling_state->next.occupancy.OMC = INACTIVE;
            break;

        }

        case EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED:
        {

            _event_handler_signaling_state->next.occupancy.OSC = ACTIVE;
            break;
        }

        case EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED:
        {

            _event_handler_signaling_state->next.occupancy.OSC = INACTIVE;
            break;

        }
    }

}

void _handle_event_from_board_adjecent_right_for_rb(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_CD_STOP:
        {

            _event_handler_signaling_state->next.stop.ScdBARstop = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP:
        {

            _event_handler_signaling_state->next.stop.ScdBARstop = INACTIVE;
            break;

        }

    }

}

void _handle_event_for_this_board(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_TURNOUT_COMMAND_NORMAL:
        {

            _event_handler_signaling_state->next.remote_control.turnout_normal = TRUE;
            break;

        }
        case EVENT_SUFFIX_TURNOUT_COMMAND_DIVERGING:
        {

            _event_handler_signaling_state->next.remote_control.turnout_diverging = TRUE;
            break;

        }
        case EVENT_SUFFIX_VITAL_LOGIC_STATE_HELD:
        {

            _event_handler_signaling_state->next.ctc_control.SHD = TRUE;
            break;
        }
        case EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_LEFT:
        {

            _event_handler_signaling_state->next.ctc_control.SCL = TRUE;
            break;
        }
        case EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_RIGHT:
        {

            _event_handler_signaling_state->next.ctc_control.SCR = TRUE;
            break;
        }
        case EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_BOTH:
        {

            _event_handler_signaling_state->next.ctc_control.SCB = TRUE;
            break;
        }

    }
}

void _event_pc_report_callback(openlcb_node_t* node, event_id_t* event_id) {

    // Start the filtering of the events to find one we care about

    node_id_t local_node_id = (node_id_t) (*event_id >> 16);
    uint16_olcb_t event_id_suffix = (uint16_olcb_t) (*event_id);

    if (local_node_id == _event_handler_board_configuration->board_to_the_left) {

        if (_event_handler_board_configuration->board_location == BL) {

            _handle_event_from_board_adjecent_left_for_bl(event_id_suffix);

        } else {

            _handle_event_from_board_left_for_rb(event_id_suffix);

        }

    } else if (local_node_id == _event_handler_board_configuration->board_to_the_right) {

        if (_event_handler_board_configuration->board_location == BL) {

            _handle_event_from_board_to_the_right_for_bl(event_id_suffix);

        } else {

            _handle_event_from_board_adjecent_right_for_rb(event_id_suffix);


        }

    } else if (local_node_id == node->id) {

        _handle_event_for_this_board(event_id_suffix);

    }


}

void _register_producer(openlcb_node_t *node, event_id_t event, uint8_olcb_t offset, uint8_olcb_t core_signaling_event, send_event_engine_t* event_engine) {

    Application_register_producer_eventid(node, event);
    event_engine->events[offset].state.valid_producer = TRUE;
    event_engine->events[offset].state.core_signaling = core_signaling_event;

}

void _register_consumer(openlcb_node_t *node, event_id_t event, uint8_olcb_t offset, uint8_olcb_t core_signaling_event, send_event_engine_t* event_engine) {

    Application_register_consumer_eventid(node, event);
    event_engine->events[offset].state.valid_consumer = TRUE;
    event_engine->events[offset].state.core_signaling = core_signaling_event;

}

void _board_left_register_core_signaling_events(openlcb_node_t *node, node_id_t board_adjacent_left, node_id_t board_right, send_event_engine_t* event_engine) {


    if (board_adjacent_left) {

        event_id_t _event_id_base_board_adjacent_left = board_adjacent_left << 16;

        _register_consumer(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_SIGNAL_STATE_CD_STOP, OFFSET_EVENT_SIGNAL_STATE_CD_STOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP, TRUE, event_engine);

    }

    if (board_right) {

        event_id_t _event_id_base_board_right = board_right << 16;

        _register_consumer(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_A_STOP, OFFSET_EVENT_SIGNAL_STATE_A_STOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_B_STOP, OFFSET_EVENT_SIGNAL_STATE_B_STOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP, TRUE, event_engine);

    }

    // LH Defined Node Only specific Producers
    event_id_t _event_id_base = node->id << 16;

    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_STOP, OFFSET_EVENT_SIGNAL_STATE_A_STOP, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_STOP, OFFSET_EVENT_SIGNAL_STATE_B_STOP, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED, TRUE, event_engine);

    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_RED,    OFFSET_EVENT_SIGNAL_A_RED,    TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_YELLOW, OFFSET_EVENT_SIGNAL_A_YELLOW, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_GREEN,  OFFSET_EVENT_SIGNAL_A_GREEN,  TRUE, event_engine);
    
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_RED,    OFFSET_EVENT_SIGNAL_B_RED,    TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_YELLOW, OFFSET_EVENT_SIGNAL_B_YELLOW, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_GREEN,  OFFSET_EVENT_SIGNAL_B_GREEN,  TRUE, event_engine);
    
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_RED,    OFFSET_EVENT_SIGNAL_C_RED,    TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_YELLOW, OFFSET_EVENT_SIGNAL_C_YELLOW, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_GREEN,  OFFSET_EVENT_SIGNAL_C_GREEN,  TRUE, event_engine);
    
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_RED,    OFFSET_EVENT_SIGNAL_D_RED,    TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_YELLOW, OFFSET_EVENT_SIGNAL_D_YELLOW, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_GREEN,  OFFSET_EVENT_SIGNAL_D_GREEN,  TRUE, event_engine);
    
}

void _board_right_register_core_signaling_events(openlcb_node_t *node, node_id_t board_left, node_id_t board_adjacent_right, send_event_engine_t* event_engine) {



    if (board_adjacent_right) {

        event_id_t _event_id_base_board_adjacent_right = board_adjacent_right << 16;

        _register_consumer(node, _event_id_base_board_adjacent_right + EVENT_SUFFIX_SIGNAL_STATE_CD_STOP, OFFSET_EVENT_SIGNAL_STATE_CD_STOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_adjacent_right + EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP, TRUE, event_engine);

    }

    if (board_left) {

        event_id_t _event_id_base_board_left = board_left << 16;

        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_A_STOP, OFFSET_EVENT_SIGNAL_STATE_A_STOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_B_STOP, OFFSET_EVENT_SIGNAL_STATE_B_STOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED, TRUE, event_engine);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED, TRUE, event_engine);

    }

    // RH Defined Node ONLY specific Producers

    event_id_t _event_id_base = node->id << 16;

    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_STOP, OFFSET_EVENT_SIGNAL_STATE_A_STOP, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_STOP, OFFSET_EVENT_SIGNAL_STATE_B_STOP, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP, TRUE, event_engine);

    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_RED,    OFFSET_EVENT_SIGNAL_A_RED,    TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_YELLOW, OFFSET_EVENT_SIGNAL_A_YELLOW, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_GREEN,  OFFSET_EVENT_SIGNAL_A_GREEN,  TRUE, event_engine);
    
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_RED,    OFFSET_EVENT_SIGNAL_B_RED,    TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_YELLOW, OFFSET_EVENT_SIGNAL_B_YELLOW, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_GREEN,  OFFSET_EVENT_SIGNAL_B_GREEN,  TRUE, event_engine);
    
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_RED,    OFFSET_EVENT_SIGNAL_C_RED,    TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_YELLOW, OFFSET_EVENT_SIGNAL_C_YELLOW, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_GREEN,  OFFSET_EVENT_SIGNAL_C_GREEN,  TRUE, event_engine);
    
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_RED,    OFFSET_EVENT_SIGNAL_D_RED,    TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_YELLOW, OFFSET_EVENT_SIGNAL_D_YELLOW, TRUE, event_engine);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_GREEN,  OFFSET_EVENT_SIGNAL_D_GREEN,  TRUE, event_engine);
    
}

void _board_register_general_events(openlcb_node_t *node) {


}

void TurnoutBossEventHandler_initialize(openlcb_node_t *node, board_configuration_t* board_configuration, signaling_state_t* signaling_state, send_event_engine_t* event_engine) {

    _event_handler_board_configuration = board_configuration;
    _event_handler_signaling_state = signaling_state;

    // Clear the events just in case
    Application_clear_consumer_eventids(node);
    Application_clear_producer_eventids(node);

    if (_event_handler_board_configuration->board_location == BL) {

        _board_left_register_core_signaling_events(node, _event_handler_board_configuration->board_to_the_left, _event_handler_board_configuration->board_to_the_right, event_engine);

    } else {

        _board_right_register_core_signaling_events(node, _event_handler_board_configuration->board_to_the_left, _event_handler_board_configuration->board_to_the_right, event_engine);

    }

    _board_register_general_events(node);

    Application_Callbacks_set_event_pc_report(&_event_pc_report_callback);

}
