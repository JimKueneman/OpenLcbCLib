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


#ifdef MPLAB
#include "../../../openlcb/openlcb_types.h"
#include "../TurnoutBossCommon/common_loader_app.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

#include "turnoutboss_types.h"
#include "turnoutboss_signal_calculations_board_left.h"
#include "turnoutboss_signal_calculations_board_right.h"
#include "turnoutboss_drivers.h"

void TurnoutBossSignalCalculations_send_hardware_pushbutton_change_events(signaling_state_t* states, board_configuration_t* board_configuration, send_event_engine_t* event_engine) {

    if (states->hardware.turnout_pushbutton_normal != states->next.hardware.turnout_pushbutton_normal) {

        switch (states->next.hardware.turnout_pushbutton_normal) {

            case CLOSED:

                event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_CLOSED].state.send = TRUE;
                break;
            case OPEN:

                event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_OPEN].state.send = TRUE;
                break;

        }

        states->pushbutton_normal_toggled = TRUE;
        states->hardware.turnout_pushbutton_normal = states->next.hardware.turnout_pushbutton_normal;

    } else {

        states->pushbutton_normal_toggled = FALSE;

    }


    if (states->hardware.turnout_pushbutton_diverging != states->next.hardware.turnout_pushbutton_diverging) {

        switch (states->next.hardware.turnout_pushbutton_diverging) {

            case CLOSED:

                event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_CLOSED].state.send = TRUE;
                break;

            case OPEN:

                event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_OPEN].state.send = TRUE;
                break;

        }

        states->pushbutton_diverging_toggled = TRUE;
        states->hardware.turnout_pushbutton_diverging = states->next.hardware.turnout_pushbutton_diverging;

    } else {

        states->pushbutton_diverging_toggled = FALSE;

    }

}

void TurnoutBossSignalCalculations_send_hardware_turnout_feedback_change_events(signaling_state_t* states, board_configuration_t* board_configuration, send_event_engine_t* event_engine) {

    if (states->hardware.turnout_feedback_normal != states->next.hardware.turnout_feedback_normal) {

        switch (states->next.hardware.turnout_feedback_normal) {

            case ACTIVE:

                event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_ACTIVE].state.send = TRUE;
                break;

            case INACTIVE:

                event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_INACTIVE].state.send = TRUE;
                break;

        }

        states->hardware.turnout_feedback_normal = states->next.hardware.turnout_feedback_normal;

    }

    if (states->hardware.turnout_feedback_diverging != states->next.hardware.turnout_feedback_diverging) {

        switch (states->next.hardware.turnout_feedback_diverging) {

            case ACTIVE:

                event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_ACTIVE].state.send = TRUE;
                break;

            case INACTIVE:

                event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_INACTIVE].state.send = TRUE;
                break;

        }

        states->hardware.turnout_feedback_diverging = states->next.hardware.turnout_feedback_diverging;

    }

}

void TurnoutBossSignalCalculations_recalculate_states(signaling_state_t* signal_calculation_states, board_configuration_t* board_configuration, send_event_engine_t* event_engine) {

    // Pause the timer so we don't re-calculate the state of the signals and stomp on the signals being set in the Signal Update Timer
    TurnoutBossDrivers_pause_signal_calculation_timer();
            
    if (board_configuration->board_location == BL) {

        TurnoutBossSignalCalculationsBoardLeft_run(signal_calculation_states, board_configuration, event_engine);

        TURNOUT_DRIVER_PIN = signal_calculation_states->turnout.TLC;

    } else {

        TurnoutBossSignalCalculationsBoardRight_run(signal_calculation_states, board_configuration, event_engine);

        TURNOUT_DRIVER_PIN = signal_calculation_states->turnout.TRC;

    }
    
    TurnoutBossDrivers_resume_signal_calculation_timer();
    
}

