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
 * \file turnoutboss_signal_calculations.c
 *
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory.  It also maintains the state of the signals that are updated
 * from the TurnoutBoss_Event_Handler module for incoming events from linked Left and 
 * Right TurnoutBosses
 *
 * @author Jim Kueneman
 * @date 24 Jan 2025
 */

#include "turnoutboss_signal_calculations.h"

#include <string.h>


#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

#include "turnoutboss_types.h"

void TurnoutBossSignalCalculations_initialize(signaling_state_t* states, signaling_state_t* states_next) {

    states->ctc_control.SCB = ACTIVE; // CTC Clear for both is default

    states_next->ctc_control.SCB = ACTIVE; // CTC Clear for both is default

}

void _board_left_occupancy_handle_hardware_change_events(signaling_state_t* states, signaling_state_t* states_next,
        hardware_input_states_t* states_hardware, hardware_input_states_t* states_hardware_next, send_event_engine_t* event_engine) {

    if (states_hardware->occupany_1 != states_hardware_next->occupany_1) {

        if (states_hardware_next->occupany_1) {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_OCCUPIED].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED].state.send = TRUE;

        }

    }

    if (states_hardware->occupany_2 != states_hardware_next->occupany_2) {

        if (states_hardware_next->occupany_2) {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED].state.send = TRUE;

        }

    }

    if (states_hardware->occupany_3 != states_hardware_next->occupany_3) {

        if (states_hardware_next->occupany_3) {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED].state.send = TRUE;

        }

    }

}

void _board_right_occupancy_handle_hardware_change_events(signaling_state_t* states, signaling_state_t* states_next,
        hardware_input_states_t* states_hardware, hardware_input_states_t* states_hardware_next, send_event_engine_t* event_engine) {

    if (states_hardware->occupany_1 != states_hardware_next->occupany_1) {

        if (states_hardware_next->occupany_1) {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED].state.send = TRUE;

        }

    }

    if (states_hardware->occupany_2 != states_hardware_next->occupany_2) {

        if (states_hardware_next->occupany_2) {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_OCCUPIED].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_OCCUPIED].state.send = TRUE;

        }

    }

}

void _pushbutton_handle_hardware_change_events(signaling_state_t* states, signaling_state_t* states_next,
        hardware_input_states_t* states_hardware, hardware_input_states_t* states_hardware_next, send_event_engine_t* event_engine) {

    if (states_hardware->turnout_pushbutton_normal != states_hardware_next->turnout_pushbutton_normal) {

        if (states_hardware_next->turnout_pushbutton_normal) {

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_CLOSED].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_OPEN].state.send = TRUE;

        }

    }

    if (states_hardware->turnout_pushbutton_diverging != states_hardware_next->turnout_pushbutton_diverging) {

        if (states_hardware_next->turnout_pushbutton_diverging) {

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_CLOSED].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_OPEN].state.send = TRUE;

        }

    }

}

void _feedback_handle_hardware_change_events(signaling_state_t* states, signaling_state_t* states_next,
        hardware_input_states_t* states_hardware, hardware_input_states_t* states_hardware_next, send_event_engine_t* event_engine) {

    if (states_hardware->turnout_normal_feedback != states_hardware_next->turnout_normal_feedback) {

        if (states_hardware_next->turnout_normal_feedback) {

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_ACTIVE].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_INACTIVE].state.send = TRUE;

        }

    }

    if (states_hardware->turnout_diverging_feedback != states_hardware_next->turnout_diverging_feedback) {

        if (states_hardware_next->turnout_diverging_feedback) {

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_ACTIVE].state.send = TRUE;

        } else {

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_INACTIVE].state.send = TRUE;

        }

    }

}

void TurnoutBossSignalCalculations_run_board_left(signaling_state_t* states, signaling_state_t* states_next, hardware_input_states_t* states_hardware,
        hardware_input_states_t* states_hardware_next, send_event_engine_t * event_engine, board_configuration_t* _board_configuration) {

    // Initialize so we can see if something is different

    _board_left_occupancy_handle_hardware_change_events(states, states_next, states_hardware, states_hardware_next, event_engine);
    _pushbutton_handle_hardware_change_events(states, states_next, states_hardware, states_hardware_next, event_engine);
    _feedback_handle_hardware_change_events(states, states_next, states_hardware, states_hardware_next, event_engine);

    // states_next->occupancy.OML comes from events sent by the Board Adjacent Left
    states_next->occupancy.OTL = states_hardware_next->occupany_1;
    states_next->occupancy.OMC = states_hardware_next->occupany_2;
    states_next->occupancy.OSC = states_hardware_next->occupany_3;

    switch (_board_configuration->pushbutton_type) {

        case Pushbutton_Dual:
            
            return;
        case Pushbutton_Single:
            
            return;
    }

    switch (_board_configuration->turnout_feedback_type) {

        case TurnoutFeedbackUnused:
            
            return;
        case TurnoutFeedbackSingle:
            
            return;
        case TurnoutFeedbackDual:
            
            return;
    }
    
    switch (_board_configuration->point_signalhead_type) {
        
        case PointSignalHeadSingle:
            
            return;
        case PointSignalHeadDual:
            
            return;
                    
    }
    
}

void TurnoutBossSignalCalculations_run_board_right(signaling_state_t* states, signaling_state_t* states_next, hardware_input_states_t* states_hardware,
        hardware_input_states_t* states_hardware_next, send_event_engine_t * event_engine, board_configuration_t* _board_configuration) {

    // Initialize so we can see if something is different

    _board_right_occupancy_handle_hardware_change_events(states, states_next, states_hardware, states_hardware_next, event_engine);
    _pushbutton_handle_hardware_change_events(states, states_next, states_hardware, states_hardware_next, event_engine);
    _feedback_handle_hardware_change_events(states, states_next, states_hardware, states_hardware_next, event_engine);

    states_next->occupancy.OTR = states_hardware_next->occupany_1;
    states_next->occupancy.OMR = states_hardware_next->occupany_2;

    switch (_board_configuration->pushbutton_type) {

        case Pushbutton_Dual:
            
            return;
        case Pushbutton_Single:
            
            return;
    }

    switch (_board_configuration->turnout_feedback_type) {

        case TurnoutFeedbackUnused:
            
            return;
        case TurnoutFeedbackSingle:
            
            return;
        case TurnoutFeedbackDual:
            
            return;
    }
    
    switch (_board_configuration->point_signalhead_type) {
        
        case PointSignalHeadSingle:
            
            return;
        case PointSignalHeadDual:
            
            return;
                    
    }

}

