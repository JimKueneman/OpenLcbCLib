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
 * \file turnoutboss_signal_calculations_board_left.c
 *
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory.  It also maintains the state of the signals that are updated
 * from the TurnoutBoss_Event_Handler module for incoming events from linked Left and
 * Right TurnoutBosses
 *
 * @author Jim Kueneman
 * @date 24 Jan 2025
 */

#include "turnoutboss_signal_calculations_board_left.h"

#include <string.h>

#ifdef MPLAB
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

#include "turnoutboss_types.h"
#include "turnoutboss_signal_calculations.h"

void TurnoutBossSignalCalculationsBoardLeft_initialize(signaling_state_t *states)
{

    states->ctc_control.SCB = ACTIVE;      // CTC Clear for both is default
    states->next.ctc_control.SCB = ACTIVE; // CTC Clear for both is default
}

void _send_hardware_occupancy_change_events_board_left(signaling_state_t *states, send_event_engine_t *event_engine)
{

    if (states->hardware.occupany_1 != states->next.hardware.occupany_1)
    {

        switch (states->next.hardware.occupany_1)
        {

        case OCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_OCCUPIED].state.send = TRUE;
            break;

        case UNOCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED].state.send = TRUE;
            break;
        }

        states->hardware.occupany_1 = states->next.hardware.occupany_1;
    }

    if (states->hardware.occupany_2 != states->next.hardware.occupany_2)
    {

        switch (states->next.hardware.occupany_2)
        {

        case OCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED].state.send = TRUE;
            break;

        case UNOCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED].state.send = TRUE;
            break;
        }

        states->hardware.occupany_2 = states->next.hardware.occupany_2;
    }

    if (states->hardware.occupany_3 != states->next.hardware.occupany_3)
    {

        switch (states->next.hardware.occupany_3)
        {

        case OCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED].state.send = TRUE;
            break;

        case UNOCCUPIED:

            event_engine->events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED].state.send = TRUE;
            break;
        }

        states->hardware.occupany_3 = states->next.hardware.occupany_3;
    }
}

void _calculate_occupancy_state_board_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    // states_next->occupancy.OML comes from events sent by the Board Adjacent Left from turnoutboss_event_handler.c
    states->next.occupancy.OTL = states->hardware.occupany_1;
    states->next.occupancy.OMC = states->hardware.occupany_2;
    states->next.occupancy.OSC = states->hardware.occupany_3;
}

void _calculate_turnout_commanded_state_board_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    uint8_olcb_t tlc_updated = FALSE;

    switch (board_configuration->pushbutton_type)
    {

        // The pushbuttons are momentary contact so we can only set these when the buttons are
        // first closed, else we use the current state

    case Pushbutton_Dual:

        if (states->pushbutton_normal_toggled && (states->next.hardware.turnout_pushbutton_normal == CLOSED))
        { // Has the normal button toggled to the CLOSED?

            states->next.turnout.TLC = ACTIVE;
            tlc_updated = TRUE;
        }
        else if (states->pushbutton_diverging_toggled && (states->next.hardware.turnout_pushbutton_diverging == CLOSED))
        { // Has the diverging button toggled to the CLOSED?

            states->next.turnout.TLC = INACTIVE;
            tlc_updated = TRUE;
        }

        break;

    case Pushbutton_Single:

        if (states->pushbutton_normal_toggled && (states->next.hardware.turnout_pushbutton_normal == CLOSED))
        {

            states->next.turnout.TLC = !states->turnout.TLC;
            tlc_updated = TRUE;
        }

        break;
    }

    // See if we received a remote control turnout control command and if so override the buttons
    if (states->next.remote_control.turnout_normal)
    {

        states->next.turnout.TLC = ACTIVE;
        states->next.remote_control.turnout_normal = FALSE;
        tlc_updated = TRUE;
    }

    if (states->next.remote_control.turnout_diverging)
    {

        states->next.turnout.TLC = INACTIVE;
        states->next.remote_control.turnout_diverging = FALSE;
        tlc_updated = TRUE;
    }

    if (!tlc_updated)
        states->next.turnout.TLC = states->turnout.TLC;
}

void _calculate_turnout_observed_state_board_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    switch (board_configuration->turnout_feedback_type)
    {

    case TurnoutFeedbackUnused:

        states->next.turnout.TLO = states->next.turnout.TLC;
        break;

    case TurnoutFeedbackSingle:

        states->next.turnout.TLO = states->hardware.turnout_feedback_normal == ACTIVE;
        break;

    case TurnoutFeedbackDual:

        if ((states->hardware.turnout_feedback_normal && states->hardware.turnout_feedback_diverging ) ||
            (states->hardware.turnout_feedback_normal && states->hardware.turnout_feedback_diverging) )
        {

            states->next.turnout.TLO = TURNOUT_OBSERVED_IN_MOTION;
        }
        else if (states->hardware.turnout_feedback_normal)
        {

            states->next.turnout.TLO = TURNOUT_OBSERVED_NORMAL;
        }
        else
        {

            states->next.turnout.TLO = TURNOUT_OBSERVED_DIVERGING;
        }
        break;
    }
}

void _calculate_signal_a_board_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    // Set SaBL to Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.SaBL = RED; // set red
    states->next.stop.SaBLstop = ACTIVE;

    // If (SCB?SCL) && TLO Normal && ! OML && ! OTL && ! ScdBAL at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Left
        (states->next.turnout.TLO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal (straight)
        !(states->next.occupancy.OML == OCCUPIED) &&                                              // not Left Main Occupied
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.stop.ScdBALstop == ACTIVE)                                                 // not Signal CD on the Adjacent Board Left at Stop
    )
    {

        // Set SaBL to Green, Not-Stop

        states->next.signal_lamps.SaBL = GREEN; // set green
        states->next.stop.SaBLstop = INACTIVE;
    }

    // If (SCB?SCL) && TLO Normal && ! OML && ! OTL && ScdBAL at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // CTC Clear Both or Left
        (states->next.turnout.TLO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal (straight)
        !(states->next.occupancy.OML == OCCUPIED) &&                                              // not Left Main Occupied
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        (states->next.stop.ScdBALstop == ACTIVE)                                                  // Signal CD on the Adjacent Board Left at Stop
    )
    {

        // Set SaBL to Yellow, Not-Stop
        states->next.signal_lamps.SaBL = YELLOW; // set yellow
        states->next.stop.SaBLstop = INACTIVE;
    }
}

void _calculate_signal_b_board_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    // Set SbBL at Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.SbBL = RED; // set red
    states->next.stop.SbBLstop = ACTIVE;

    // If (SCB?SCL) && TLO Diverging && ! OML && ! OTL && ! ScdBAL at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // not (CTC Clear Both or Left)
        (states->next.turnout.TLO == TURNOUT_OBSERVED_DIVERGING) &&                                // Turnout Observed set to diverging
        !(states->next.occupancy.OML == OCCUPIED) &&                                               // not Left Main Occupied
        !(states->next.occupancy.OTL == OCCUPIED) &&                                               // not Left Turnout Occupied
        !(states->next.stop.ScdBALstop == ACTIVE)                                                  // not (Signal CD on the Adjacent Board Left at Stop)
    )
    {

        // Set SbBL to Green, Not-Stop
        states->next.signal_lamps.SbBL = GREEN; // set green
        states->next.stop.SbBLstop = INACTIVE;
    }

    // If (SCB?SCL) && TLO Diverging && ! OML && ! OTL && ScdBAL at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCL == ACTIVE)) && // not (CTC Clear Both or Left)
        (states->next.turnout.TLO == TURNOUT_OBSERVED_DIVERGING) &&                                // Turnout Observed set to diverging
        !(states->next.occupancy.OML == OCCUPIED) &&                                               // not Left Main Occupied
        !(states->next.occupancy.OTL == OCCUPIED) &&                                               // not Left Turnout Occupied
        (states->next.stop.ScdBALstop == ACTIVE)                                                   // Signal CD on the Adjacent Board Left at Stop
    )
    {

        // Set SbBL to Yellow, Not-Stop
        states->next.signal_lamps.SbBL = YELLOW; // set yellow
        states->next.stop.SbBLstop = INACTIVE;
    }
}

void _calculate_signal_cd_single_head_board_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    uint8_olcb_t ScBLStop;
    uint8_olcb_t SdBLStop;

    // Set ScBL at Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.ScBL = RED; // set red
    ScBLStop = ACTIVE;

    // If (SCB?SCR) && TLO Normal && ! OTL && ! OMC && ! SaBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TLO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        !(states->next.stop.SaBRstop == ACTIVE)                                                   //  not (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBL to Green, Not-Stop
        states->next.signal_lamps.ScBL = GREEN; // set green
        ScBLStop = INACTIVE;
    }

    // If (SCB?SCR) && TLO Normal && ! OTL && ! OMC && SaBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TLO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        (states->next.stop.SaBRstop == ACTIVE)                                                    //  (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBL to Yellow, Not-Stop
        states->next.signal_lamps.ScBL = YELLOW; // set yellow
        ScBLStop = INACTIVE;
    }

    // If (SCB?SCR) && TLO Diverging && ! OTL && ! OSC && ! SbBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TLO == TURNOUT_OBSERVED_DIVERGING) &&                               // Turnout Observed set to diverging
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        !(states->next.stop.SaBRstop == ACTIVE)                                                   //  not (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBL to Green, Not-Stop
        states->next.signal_lamps.ScBL = GREEN; // set green
        ScBLStop = INACTIVE;
    }

    // If (SCB?SCR) && TLO Diverging && ! OTL && ! OSC && SbBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TLO == TURNOUT_OBSERVED_DIVERGING) &&                               // Turnout Observed set to diverging
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        (states->next.stop.SaBRstop == ACTIVE)                                                    //  (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBL to Yellow, Not-Stop
        states->next.signal_lamps.ScBL = YELLOW; // set yellow
        ScBLStop = INACTIVE;
    }

    //  Set SdBL to Dark and Stop
    states->next.signal_lamps.SdBL = DARK;
    SdBLStop = ACTIVE;

    // If ScBL at Stop and SdBL at Stop, set ScdBL to Stop Else set ScdBL to Not-Stop
    states->next.stop.ScdBLstop = (ScBLStop == ACTIVE) && (SdBLStop == ACTIVE);
}

void _calculate_signal_cd_dual_head_board_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    uint8_olcb_t ScBLStop;
    uint8_olcb_t SdBLStop;

    // Set red/stop as default
    states->next.signal_lamps.ScBL = RED; // set red
    ScBLStop = ACTIVE;

    // If (SCB?SCR) && TLO Normal && ! OTL && ! OMC && ! SaBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TLO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        !(states->next.stop.SaBRstop == ACTIVE)                                                   //  not (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBL to Green, Not-Stop
        states->next.signal_lamps.ScBL = GREEN; // set green
        ScBLStop = INACTIVE;
    }

    // If (SCB?SCR) && TLO Normal && ! OTL && ! OMC && SaBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TLO == TURNOUT_OBSERVED_NORMAL) &&                                  // Turnout Observed set to normal
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OMC == OCCUPIED) &&                                              // not Main Center Occupied
        (states->next.stop.SaBRstop == ACTIVE)                                                    //  (Signal A on the Board Right at Stop)
    )
    {

        // Set ScBL to Yellow, Not-Stop
        states->next.signal_lamps.ScBL = YELLOW; // set yellow
        ScBLStop = INACTIVE;
    }

    // Set SdBL at Red and Stop

    // Set red/stop as default
    states->next.signal_lamps.SdBL = RED; // set red
    SdBLStop = ACTIVE;

    // If (SCB?SCR) && TLO Diverging && ! OTL && ! OSC && ! SbBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TLO == TURNOUT_OBSERVED_DIVERGING) &&                               // Turnout Observed set to diverging
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OSC == OCCUPIED) &&                                              // not Main Center Occupied
        !(states->next.stop.SbBRstop == ACTIVE)                                                   //  not (Signal A on the Board Right at Stop)
    )
    {

        // Set SdBL to Green, Not-Stop
        states->next.signal_lamps.SdBL = GREEN; // set green
        SdBLStop = INACTIVE;
    }

    // If (SCB?SCR) && TLO Diverging && ! OTL && ! OSC && SbBR at Stop:
    if (((states->next.ctc_control.SCB == ACTIVE) || (states->next.ctc_control.SCR == ACTIVE)) && // CTC Clear Both or Right
        (states->next.turnout.TLO == TURNOUT_OBSERVED_DIVERGING) &&                               // Turnout Observed set to diverging
        !(states->next.occupancy.OTL == OCCUPIED) &&                                              // not Left Turnout Occupied
        !(states->next.occupancy.OSC == OCCUPIED) &&                                              // not Main Center Occupied
        (states->next.stop.SbBRstop == ACTIVE)                                                    //  (Signal A on the Board Right at Stop)
    )
    {

        // Set SdBL to Yellow, Non_stop
        states->next.signal_lamps.SdBL = YELLOW; // set yellow
        SdBLStop = INACTIVE;
    }

    // If ScBL at Stop and SdBL at Stop, set ScdBL to Stop Else set ScdBL to Not-Stop
    states->next.stop.ScdBLstop = (ScBLStop == ACTIVE) && (SdBLStop == ACTIVE);
}

void _calculate_signal_cd_board_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    switch (board_configuration->point_signalhead_type)
    {

    case PointSignalHeadDual:

        _calculate_signal_cd_dual_head_board_left(states, board_configuration, event_engine);
        break;
    case PointSignalHeadSingle:

        _calculate_signal_cd_single_head_board_left(states, board_configuration, event_engine);
        break;
    }
}

void _send_change_events_occupancy_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    if (states->occupancy.OTL != states->next.occupancy.OTL)
    {

        if (states->next.occupancy.OTL == OCCUPIED)
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_OCCUPIED].state.send = TRUE;
        }
        else
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED].state.send = TRUE;
        }
        states->occupancy.OTL = states->next.occupancy.OTL;
    }

    if (states->occupancy.OMC != states->next.occupancy.OMC)
    {

        if (states->next.occupancy.OMC == OCCUPIED)
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED].state.send = TRUE;
        }
        else
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED].state.send = TRUE;
        }
        states->occupancy.OMC = states->next.occupancy.OMC;
    }

    if (states->occupancy.OSC != states->next.occupancy.OSC)
    {

        if (states->next.occupancy.OSC == OCCUPIED)
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED].state.send = TRUE;
        }
        else
        {

            event_engine->events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED].state.send = TRUE;
        }
        states->occupancy.OSC = states->next.occupancy.OSC;
    }

    states->occupancy = states->next.occupancy;
}

void _send_change_events_turnout_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{
    if (states->turnout.TLO != states->next.turnout.TLO)

    {

        switch (states->next.turnout.TLO)
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

        states->turnout.TLO = states->next.turnout.TLO;
    }
    
    // don't need to test but it may be useful someday.
    if (states->turnout.TLC != states->next.turnout.TLC)
    {
        
        states->turnout.TLC = states->next.turnout.TLC;
        
    }
}

void _send_change_events_lamp_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    if (states->lamps.SaBL != states->next.signal_lamps.SaBL)
    {

        switch (states->next.signal_lamps.SaBL)
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

        states->lamps.SaBL = states->next.signal_lamps.SaBL;
    }

    if (states->lamps.SbBL != states->next.signal_lamps.SbBL)
    {

        switch (states->next.signal_lamps.SbBL)
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

        states->lamps.SbBL = states->next.signal_lamps.SbBL;
    }

    if (states->lamps.ScBL != states->next.signal_lamps.ScBL)
    {

        switch (states->next.signal_lamps.ScBL)
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

        states->lamps.ScBL = states->next.signal_lamps.ScBL;
    }

    if (states->lamps.SdBL != states->next.signal_lamps.SdBL)
    {

        switch (states->next.signal_lamps.SdBL)
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

        states->lamps.SdBL = states->next.signal_lamps.SdBL;
    }
}

void _send_change_events_stop_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    if (states->stop.SaBLstop != states->next.stop.SaBLstop)
    {

        switch (states->next.stop.SaBLstop)
        {

        case ACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_A_STOP].state.send = TRUE;
            break;

        case INACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP].state.send = TRUE;
            break;
        }

        states->stop.SaBLstop = states->next.stop.SaBLstop;
    }

    if (states->stop.SbBLstop != states->next.stop.SbBLstop)
    {

        switch (states->next.stop.SbBLstop)
        {

        case ACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_B_STOP].state.send = TRUE;
            break;

        case INACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP].state.send = TRUE;
            break;
        }

        states->stop.SbBLstop = states->next.stop.SbBLstop;
    }

    if (states->stop.ScdBLstop != states->next.stop.ScdBLstop)
    {

        switch (states->next.stop.ScdBLstop)
        {

        case ACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_CD_STOP].state.send = TRUE;
            break;

        case INACTIVE:

            event_engine->events[OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP].state.send = TRUE;
            break;
        }

        states->stop.ScdBLstop = states->next.stop.ScdBLstop;
    }
}

void _send_change_events_ctc_control_left(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    states->ctc_control = states->next.ctc_control;

}

    void TurnoutBossSignalCalculationsBoardLeft_run(signaling_state_t *states, board_configuration_t *board_configuration, send_event_engine_t *event_engine)
{

    TurnoutBossSignalCalculations_send_hardware_pushbutton_change_events(states, board_configuration, event_engine);
    TurnoutBossSignalCalculations_send_hardware_turnout_feedback_change_events(states, board_configuration, event_engine);

    _send_hardware_occupancy_change_events_board_left(states, event_engine);

    _calculate_occupancy_state_board_left(states, board_configuration, event_engine);
    _calculate_turnout_commanded_state_board_left(states, board_configuration, event_engine);
    _calculate_turnout_observed_state_board_left(states, board_configuration, event_engine);
    _calculate_signal_a_board_left(states, board_configuration, event_engine);
    _calculate_signal_b_board_left(states, board_configuration, event_engine);
    _calculate_signal_cd_board_left(states, board_configuration, event_engine);

    _send_change_events_occupancy_left(states, board_configuration, event_engine);
    _send_change_events_turnout_left(states, board_configuration, event_engine);
    _send_change_events_lamp_left(states, board_configuration, event_engine);
    _send_change_events_stop_left(states, board_configuration, event_engine);
    _send_change_events_ctc_control_left(states, board_configuration, event_engine);
}
