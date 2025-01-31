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
 * \file turnoutboss_signal_calculations_board_right.c
 *
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory.  It also maintains the state of the signals that are updated
 * from the TurnoutBoss_Event_Handler module for incoming events from linked Left and
 * Right TurnoutBosses
 *
 * @author Jim Kueneman
 * @date 24 Jan 2025
 */

#include "turnoutboss_signal_calculations_board_right.h"

#include <string.h>

#ifdef MPLAB
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

#include "turnoutboss_types.h"
#include "turnoutboss_signal_calculations.h"

void TurnoutBossSignalCalculationsBoardRight_initialize(signaling_state_t *states)
{

    states->ctc_control.SCB = ACTIVE;      // CTC Clear for both is default
    states->next.ctc_control.SCB = ACTIVE; // CTC Clear for both is default
}

void _send_hardware_occupancy_change_events_board_right(signaling_state_t *states, send_event_engine_t *event_engine)
{

    if (states->hardware.occupany_1 != states->next.hardware.occupany_1)
    {

        switch (states->next.hardware.occupany_1)
        {

        case OCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED].state.send = TRUE;
            break;
        case UNOCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED].state.send = TRUE;
            break;
        }
        states->hardware.occupany_1 = states->next.hardware.occupany_1;
    }

    if (states->hardware.occupany_2 != states->next.hardware.occupany_2)
    {

        switch (states->next.hardware.occupany_2)
        {

        case OCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_OCCUPIED].state.send = TRUE;
            break;
        case UNOCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED].state.send = TRUE;
            break;
        }
        states->hardware.occupany_2 = states->next.hardware.occupany_2;
    }
}

void _calculate_occupancy_state_board_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    states->next.occupancy.OTR = states->hardware.occupany_1;
    states->next.occupancy.OMR = states->hardware.occupany_2;

    // OMC/OSC are set by consuming events from the BL if assigned
}

void _calculate_turnout_commanded_state_board_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    uint8_olcb_t trc_updated = FALSE;

    switch (board_configuration->pushbutton_type)
    {

        // The pushbuttons are momentary contact so we can only set these when the buttons are
        // first closed, else we use the current state

    case Pushbutton_Dual:

        if (states->pushbutton_normal_toggled && (states->next.hardware.turnout_pushbutton_normal == CLOSED))
        { // Has the normal button toggled to the CLOSED?

            states->next.turnout.TRC = ACTIVE;
            trc_updated = TRUE;
        }
        else if (states->pushbutton_diverging_toggled && (states->next.hardware.turnout_pushbutton_diverging == CLOSED))
        { // Has the diverging button toggled to the CLOSED?

            states->next.turnout.TRC = INACTIVE;
            trc_updated = TRUE;
        }
        break;

    case Pushbutton_Single:

        if (states->pushbutton_normal_toggled)
        {

            states->next.turnout.TRC = !states->turnout.TRC;
            trc_updated = TRUE;
        }
        break;
    }

    // See if we received a remote control turnout control command and if so override the buttons
    if (states->next.remote_control.turnout_normal)
    {

        states->next.turnout.TRC = ACTIVE;
        states->next.remote_control.turnout_normal = FALSE;
        trc_updated = TRUE;
    }

    if (states->next.remote_control.turnout_diverging)
    {

        states->next.turnout.TRC = INACTIVE;
        states->next.remote_control.turnout_diverging = FALSE;
        trc_updated = TRUE;
    }

    if (!trc_updated)
        states->next.turnout.TRC = states->turnout.TRC;
}

void _calculate_turnout_observed_state_board_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    switch (board_configuration->turnout_feedback_type)
    {

    case TurnoutFeedbackUnused:

        states->next.turnout.TRO = states->next.turnout.TRC;

        break;

    case TurnoutFeedbackSingle:

        states->next.turnout.TRO = states->hardware.turnout_feedback_normal == ACTIVE;

        break;

    case TurnoutFeedbackDual:

        if (((states->hardware.turnout_feedback_normal && ACTIVE == ACTIVE) && (states->hardware.turnout_feedback_diverging && ACTIVE == ACTIVE)) ||
            ((states->hardware.turnout_feedback_normal && INACTIVE == INACTIVE) && (states->hardware.turnout_feedback_diverging && INACTIVE == INACTIVE)))
        {

            states->next.turnout.TRO = TURNOUT_OBSERVED_IN_MOTION;
        }
        else if (states->hardware.turnout_feedback_normal && ACTIVE == ACTIVE)
        {

            states->next.turnout.TRO = TURNOUT_OBSERVED_NORMAL;
        }
        else
        {

            states->next.turnout.TRO = TURNOUT_OBSERVED_DIVERGING;
        }

        break;
    }
}

void _calculate_signal_a_board_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    // Set SaBR at Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.SaBR = RED; // set red
    states->next.stop.SaBRstop = ACTIVE;

    // If ( SCB?SCR) && TRO Normal && ! OML && ! OTR && ! ScdBAL at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Left
        (states->next.turnout.TRO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal (straight)
        !(states->next.occupancy.OML == OCCUPIED) &&                                              // not Left Main Occupied
        !(states->next.occupancy.OTR == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.stop.ScdBALstop == ACTIVE)                                                 // not Signal CD on the Adjacent Board Left at Stop
    )
    {

        // Set SaBR to Green, Not-Stop

        states->next.signal_lamps.SaBR = GREEN; // set green
        states->next.stop.SaBRstop = INACTIVE;
    }

    // If ( SCB?SCR) && TRO Normal && ! OML && ! OTR && ScdBAL at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Left
        (states->next.turnout.TRO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal (straight)
        !(states->next.occupancy.OML == OCCUPIED) &&                                              // not Left Main Occupied
        !(states->next.occupancy.OTR == OCCUPIED) &&                                              // not Left Turnout Occupied
        (states->next.stop.ScdBALstop == ACTIVE)                                                  // Signal CD on the Adjacent Board Left at Stop
    )
    {

        // Set SaBR to Yellow, Not-Stop
        states->next.signal_lamps.SaBR = YELLOW; // set yellow
        states->next.stop.SaBRstop = INACTIVE;
    }
}

void _calculate_signal_b_board_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    // Set SbBR at Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.SbBR = RED; // set red
    states->next.stop.SbBRstop = ACTIVE;

    // If ( SCB?SCR) && TRO Diverging && ! OML && ! OTR && ! ScdBAL at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // not (CTC Clear Both or Left)
        (states->next.turnout.TRO == TURNOUT_OBSERVED_DIVERGING) &&                                // Turnout Observed set to diverging
        !(states->next.occupancy.OML == OCCUPIED) &&                                               // not Left Main Occupied
        !(states->next.occupancy.OTR == OCCUPIED) &&                                               // not Left Turnout Occupied
        !(states->next.stop.ScdBALstop == ACTIVE)                                                  // not (Signal CD on the Adjacent Board Left at Stop)
    )
    {

        // Set SbBR to Green, Not-Stop
        states->next.signal_lamps.SbBR = GREEN; // set green
        states->next.stop.SbBRstop = INACTIVE;
    }

    // If ( SCB?SCR) && TRO Diverging && ! OML && ! OTR && ScdBAL at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // not (CTC Clear Both or Left)
        (states->next.turnout.TRO == TURNOUT_OBSERVED_DIVERGING) &&                                // Turnout Observed set to diverging
        !(states->next.occupancy.OML == OCCUPIED) &&                                               // not Left Main Occupied
        !(states->next.occupancy.OTR == OCCUPIED) &&                                               // not Left Turnout Occupied
        (states->next.stop.ScdBALstop == ACTIVE)                                                   // Signal CD on the Adjacent Board Left at Stop
    )
    {

        // Set SbBR to Yellow, Not-Stop
        states->next.signal_lamps.SbBR = YELLOW; // set yellow
        states->next.stop.SbBRstop = INACTIVE;
    }
}

void _calculate_signal_cd_single_head_board_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    uint8_olcb_t ScBRStop;
    uint8_olcb_t SdBRStop;

    // Set ScBR at Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.ScBR = RED; // set red
    ScBRStop = ACTIVE;

    // If ( SCB?SCL) && TRO Normal && ! OTL && ! OMC && ! SaBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TRO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        !(states->next.stop.SaBRstop == ACTIVE)                                                   //  not (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBR to Green, Not-Stop
        states->next.signal_lamps.ScBR = GREEN; // set green
        ScBRStop = INACTIVE;
    }

    // If ( SCB?SCL) && TRO Normal && ! OTL && ! OMC && SaBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TRO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        (states->next.stop.SaBRstop == ACTIVE)                                                    //  (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBR to Yellow, Not-Stop
        states->next.signal_lamps.ScBR = YELLOW; // set yellow
        ScBRStop = INACTIVE;
    }

    // If ( SCB?SCL) && TRO Diverging && ! OTL && ! OSC && ! SbBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TRO == TURNOUT_OBSERVED_DIVERGING) &&                               // Turnout Observed set to diverging
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OSC == OCCUPIED) &&                                              // not Main Center Occupied
        !(states->next.stop.SbBRstop == ACTIVE)                                                   //  not (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBR to Green, Not-Stop
        states->next.signal_lamps.ScBR = GREEN; // set green
        ScBRStop = INACTIVE;
    }

    // If ( SCB?SCL) && TRO Diverging && ! OTL && ! OSC && SbBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TRO == TURNOUT_OBSERVED_DIVERGING) &&                               // Turnout Observed set to diverging
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OSC == OCCUPIED) &&                                              // not Main Center Occupied
        (states->next.stop.SbBRstop == ACTIVE)                                                    //  (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBR to Yellow, Not-Stop
        states->next.signal_lamps.ScBR = YELLOW; // set yellow
        ScBRStop = INACTIVE;
    }

    // Set SdBR to Dark and Stop
    states->next.signal_lamps.SdBR = DARK;
    SdBRStop = ACTIVE;

    // If ScBR at Stop and SdBR at Stop, set ScdBR to Stop Else set ScdBR to Not-Stop
    states->next.stop.ScdBRstop = (ScBRStop == ACTIVE) && (SdBRStop == ACTIVE);
}

void _calculate_signal_cd_dual_head_board_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    uint8_olcb_t ScBRStop;
    uint8_olcb_t SdBRStop;

    // Set ScBR at Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.ScBR = RED; // set red
    ScBRStop = ACTIVE;

    // If ( SCB?SCL) && TRO Normal && ! OTL && ! OMC && ! SaBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TRO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        !(states->next.stop.SaBRstop == ACTIVE)                                                   //  not (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBR to Green, Not-Stop
        states->next.signal_lamps.ScBR = GREEN; // set green
        ScBRStop = INACTIVE;
    }

    // If ( SCB?SCL) && TRO Normal && ! OTL && ! OMC && SaBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TRO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        (states->next.stop.SaBRstop == ACTIVE)                                                    //  (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBR to Yellow, Not-Stop
        states->next.signal_lamps.ScBR = YELLOW; // set yellow
        ScBRStop = INACTIVE;
    }

    // Set SdBR at Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.SdBR = RED; // set red
    SdBRStop = ACTIVE;

    // If ( SCB?SCL) && TRO Diverging && ! OTL && ! OSC && ! SbBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TRO == TURNOUT_OBSERVED_DIVERGING) &&                               // Turnout Observed set to diverging
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OSC == OCCUPIED) &&                                              // not Main Center Occupied
        !(states->next.stop.SbBRstop == ACTIVE)                                                   //  not (Signal A on the Board Right at Stop)
    )
    {

        // Set SdBR to Green, Not-Stop
        states->next.signal_lamps.SdBR = GREEN; // set green
        SdBRStop = INACTIVE;
    }

    // If ( SCB?SCL) && TRO Diverging && ! OTL && ! OSC && SbBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TRO == TURNOUT_OBSERVED_DIVERGING) &&                               // Turnout Observed set to diverging
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OSC == OCCUPIED) &&                                              // not Main Center Occupied
        (states->next.stop.SbBRstop == ACTIVE)                                                    //  (Signal A on the Board Right at Stop)
    )
    {

        // Set SdBR to Yellow, Not-Stop
        states->next.signal_lamps.SdBR = YELLOW; // set yellow
        SdBRStop = INACTIVE;
    }

    // If ScBR at Stop and SdBR at Stop, set ScdBR to Stop Else set ScdBR to Not-Stop
    states->next.stop.ScdBRstop = (ScBRStop == ACTIVE) && (SdBRStop == ACTIVE);
}

void _calculate_signal_cd_board_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    switch (board_configuration->point_signalhead_type)
    {

    case PointSignalHeadDual:

        _calculate_signal_cd_dual_head_board_right(states, board_configuration, event_engine);
        break;
    case PointSignalHeadSingle:

        _calculate_signal_cd_single_head_board_right(states, board_configuration, event_engine);
        break;
    }
}

void _send_change_events_occupancy_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    if (states->occupancy.OTR != states->next.occupancy.OTR)
    {

        if (states->next.occupancy.OTR == OCCUPIED)
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED].state.send = TRUE;
        }
        else
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED].state.send = TRUE;
        }

        states->occupancy.OTR = states->next.occupancy.OTR;
    }

    if (states->occupancy.OMR != states->next.occupancy.OMR)
    {

        if (states->next.occupancy.OMR == OCCUPIED)
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_OCCUPIED].state.send = TRUE;
        }
        else
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED].state.send = TRUE;
        }

        states->occupancy.OMR = states->next.occupancy.OMR;
    }

    states->occupancy = states->next.occupancy;
}

void _send_change_events_turnout_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    if (states->turnout.TRO != states->next.turnout.TRO)
    {

        switch (states->next.turnout.TRO)
        {

        case TURNOUT_OBSERVED_IN_MOTION:

            event_engine->events[OFFSET_EVENT_TURNOUT_OBSERVED_IN_MOTION].state.send = TRUE;
            break;
        case TURNOUT_OBSERVED_NORMAL:

            event_engine->events[OFFSET_EVENT_TURNOUT_OBSERVED_NORMAL].state.send = TRUE;
            break;
        case TURNOUT_OBSERVED_DIVERGING:

            event_engine->events[OFFSET_EVENT_TURNOUT_OBSERVED_DIVERGING].state.send = TRUE;
            break;
        }

        states->turnout.TRO = states->next.turnout.TRO;
    }
    
    // don't need to test but it may be useful someday.
    if (states->turnout.TRC != states->next.turnout.TRC)
    {
        
        states->turnout.TRC = states->next.turnout.TRC;
        
    }
}

void _send_change_events_lamp_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    if (states->lamps.SaBR != states->next.signal_lamps.SaBR)
    {

        switch (states->next.signal_lamps.SaBR)
        {

        case RED:

            event_engine->events[OFFSET_EVENT_SIGNAL_A_RED].state.send = TRUE;
            break;

        case YELLOW:

            event_engine->events[OFFSET_EVENT_SIGNAL_A_YELLOW].state.send = TRUE;
            break;

        case GREEN:

            event_engine->events[OFFSET_EVENT_SIGNAL_A_GREEN].state.send = TRUE;
            break;

        case DARK:

            event_engine->events[OFFSET_EVENT_SIGNAL_A_DARK].state.send = TRUE;
            break;
        }

        states->lamps.SaBR = states->next.signal_lamps.SaBR;
    }

    if (states->lamps.SbBR != states->next.signal_lamps.SbBR)
    {

        switch (states->next.signal_lamps.SbBR)
        {

        case RED:

            event_engine->events[OFFSET_EVENT_SIGNAL_B_RED].state.send = TRUE;
            break;

        case YELLOW:

            event_engine->events[OFFSET_EVENT_SIGNAL_B_YELLOW].state.send = TRUE;
            break;

        case GREEN:

            event_engine->events[OFFSET_EVENT_SIGNAL_B_GREEN].state.send = TRUE;
            break;

        case DARK:

            event_engine->events[OFFSET_EVENT_SIGNAL_B_DARK].state.send = TRUE;
            break;
        }

        states->lamps.SbBR = states->next.signal_lamps.SbBR;
    }

    if (states->lamps.ScBR != states->next.signal_lamps.ScBR)
    {

        switch (states->next.signal_lamps.ScBR)
        {

        case RED:

            event_engine->events[OFFSET_EVENT_SIGNAL_C_RED].state.send = TRUE;
            break;

        case YELLOW:

            event_engine->events[OFFSET_EVENT_SIGNAL_C_YELLOW].state.send = TRUE;
            break;

        case GREEN:

            event_engine->events[OFFSET_EVENT_SIGNAL_C_GREEN].state.send = TRUE;
            break;

        case DARK:

            event_engine->events[OFFSET_EVENT_SIGNAL_C_DARK].state.send = TRUE;
            break;
        }

        states->lamps.ScBR = states->next.signal_lamps.ScBR;
    }

    if (states->lamps.SdBR != states->next.signal_lamps.SdBR)
    {

        switch (states->next.signal_lamps.SdBR)
        {

        case RED:

            event_engine->events[OFFSET_EVENT_SIGNAL_D_RED].state.send = TRUE;
            break;

        case YELLOW:

            event_engine->events[OFFSET_EVENT_SIGNAL_D_YELLOW].state.send = TRUE;
            break;

        case GREEN:

            event_engine->events[OFFSET_EVENT_SIGNAL_D_GREEN].state.send = TRUE;
            break;

        case DARK:

            event_engine->events[OFFSET_EVENT_SIGNAL_D_DARK].state.send = TRUE;
            break;
        }

        states->lamps.SdBR = states->next.signal_lamps.SdBR;
    }
}

void _send_change_events_stop_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    if (states->stop.SaBRstop != states->next.stop.SaBRstop)
    {

        switch (states->next.stop.SaBRstop)
        {

        case ACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_A_STOP].state.send = TRUE;
            break;

        case INACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP].state.send = TRUE;
            break;
        }

        states->stop.SaBRstop = states->next.stop.SaBRstop;
    }

    if (states->stop.SbBRstop != states->next.stop.SbBRstop)
    {

        switch (states->next.stop.SbBRstop)
        {

        case ACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_B_STOP].state.send = TRUE;
            break;

        case INACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP].state.send = TRUE;
            break;
        }

        states->stop.SbBRstop = states->next.stop.SbBRstop;
    }

    if (states->stop.ScdBRstop != states->next.stop.ScdBRstop)
    {

        switch (states->next.stop.ScdBRstop)
        {

        case ACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_CD_STOP].state.send = TRUE;
            break;

        case INACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP].state.send = TRUE;
            break;
        }

        states->stop.ScdBRstop = states->next.stop.ScdBRstop;
    }
}

void _send_change_events_ctc_control_right(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    states->ctc_control = states->next.ctc_control;
}

void TurnoutBossSignalCalculationsBoardRight_run(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    TurnoutBossSignalCalculations_send_hardware_pushbutton_change_events(states, board_configuration, event_engine);
    TurnoutBossSignalCalculations_send_hardware_turnout_feedback_change_events(states, board_configuration, event_engine);

    _send_hardware_occupancy_change_events_board_right(states, event_engine);

    _calculate_occupancy_state_board_right(states, board_configuration, event_engine);
    _calculate_turnout_commanded_state_board_right(states, board_configuration, event_engine);
    _calculate_turnout_observed_state_board_right(states, board_configuration, event_engine);
    _calculate_signal_a_board_right(states, board_configuration, event_engine);
    _calculate_signal_b_board_right(states, board_configuration, event_engine);
    _calculate_signal_cd_board_right(states, board_configuration, event_engine);

    _send_change_events_occupancy_right(states, board_configuration, event_engine);
    _send_change_events_turnout_right(states, board_configuration, event_engine);
    _send_change_events_lamp_right(states, board_configuration, event_engine);
    _send_change_events_stop_right(states, board_configuration, event_engine);
    _send_change_events_ctc_control_right(states, board_configuration, event_engine);

}
