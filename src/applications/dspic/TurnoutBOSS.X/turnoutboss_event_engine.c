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
 * \turnoutboss_event_engine.c
 * 
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory and then registers event IDs required by the board type.
 * It also registers an Event callback and updates the TurnoutBoss_Signaling_States with
 * any received events from nodes of interest (BAL/BAR/BL/BR).
 *
 * @author Jim Kueneman
 * @date 22 Jan 2025
 */

#include "turnoutboss_event_engine.h"

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/application.h"
#else
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/application.h"
#endif


send_event_engine_t TurnoutBoss_Event_Engine = {

    .current_index = 0,

    .events[OFFSET_EVENT_OCCUPANCY_MAIN_LEFT_OCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_MAIN_LEFT_UNOCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_OCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_OCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_OCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_OCCUPIED,
    .events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED].suffix = EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED,

    .events[OFFSET_EVENT_TURNOUT_COMMAND_NORMAL].suffix = EVENT_SUFFIX_TURNOUT_COMMAND_NORMAL,
    .events[OFFSET_EVENT_TURNOUT_COMMAND_DIVERGING].suffix = EVENT_SUFFIX_TURNOUT_COMMAND_DIVERGING,
    .events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_ACTIVE].suffix = EVENT_SUFFIX_TURNOUT_FEEDBACK_NORMAL_ACTIVE,
    .events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_INACTIVE].suffix = EVENT_SUFFIX_TURNOUT_FEEDBACK_NORMAL_INACTIVE,
    .events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_ACTIVE].suffix = EVENT_SUFFIX_TURNOUT_FEEDBACK_DIVERGING_ACTIVE,
    .events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_INACTIVE].suffix = EVENT_SUFFIX_TURNOUT_FEEDBACK_DIVERGING_INACTIVE,
    .events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_OPEN].suffix = EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_OPEN,
    .events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_CLOSED].suffix = EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_CLOSED,
    .events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_OPEN].suffix = EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_OPEN,
    .events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_CLOSED].suffix = EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_CLOSED,

    .events[OFFSET_EVENT_SIGNAL_A_RED].suffix = EVENT_SUFFIX_SIGNAL_A_RED,
    .events[OFFSET_EVENT_SIGNAL_A_YELLOW].suffix = EVENT_SUFFIX_SIGNAL_A_YELLOW,
    .events[OFFSET_EVENT_SIGNAL_A_GREEN].suffix = EVENT_SUFFIX_SIGNAL_A_GREEN,
    .events[OFFSET_EVENT_SIGNAL_A_DARK].suffix = EVENT_SUFFIX_SIGNAL_A_DARK,
    .events[OFFSET_EVENT_SIGNAL_A_LIT].suffix = EVENT_SUFFIX_SIGNAL_A_LIT,
    .events[OFFSET_EVENT_SIGNAL_A_NOT_LIT].suffix = EVENT_SUFFIX_SIGNAL_A_NOT_LIT,
    .events[OFFSET_EVENT_SIGNAL_A_HELD].suffix = EVENT_SUFFIX_SIGNAL_A_HELD,
    .events[OFFSET_EVENT_SIGNAL_A_NOT_HELD].suffix = EVENT_SUFFIX_SIGNAL_A_NOT_HELD,

    .events[OFFSET_EVENT_SIGNAL_B_RED].suffix = EVENT_SUFFIX_SIGNAL_B_RED,
    .events[OFFSET_EVENT_SIGNAL_B_YELLOW].suffix = EVENT_SUFFIX_SIGNAL_B_YELLOW,
    .events[OFFSET_EVENT_SIGNAL_B_GREEN].suffix = EVENT_SUFFIX_SIGNAL_B_GREEN,
    .events[OFFSET_EVENT_SIGNAL_B_DARK].suffix = EVENT_SUFFIX_SIGNAL_B_DARK,
    .events[OFFSET_EVENT_SIGNAL_B_LIT].suffix = EVENT_SUFFIX_SIGNAL_B_LIT,
    .events[OFFSET_EVENT_SIGNAL_B_NOT_LIT].suffix = EVENT_SUFFIX_SIGNAL_B_NOT_LIT,
    .events[OFFSET_EVENT_SIGNAL_B_HELD].suffix = EVENT_SUFFIX_SIGNAL_B_HELD,
    .events[OFFSET_EVENT_SIGNAL_B_NOT_HELD].suffix = EVENT_SUFFIX_SIGNAL_B_NOT_HELD,

    .events[OFFSET_EVENT_SIGNAL_C_RED].suffix = EVENT_SUFFIX_SIGNAL_C_RED,
    .events[OFFSET_EVENT_SIGNAL_C_YELLOW].suffix = EVENT_SUFFIX_SIGNAL_C_YELLOW,
    .events[OFFSET_EVENT_SIGNAL_C_GREEN].suffix = EVENT_SUFFIX_SIGNAL_C_GREEN,
    .events[OFFSET_EVENT_SIGNAL_C_DARK].suffix = EVENT_SUFFIX_SIGNAL_C_DARK,
    .events[OFFSET_EVENT_SIGNAL_C_LIT].suffix = EVENT_SUFFIX_SIGNAL_C_LIT,
    .events[OFFSET_EVENT_SIGNAL_C_NOT_LIT].suffix = EVENT_SUFFIX_SIGNAL_C_NOT_LIT,
    .events[OFFSET_EVENT_SIGNAL_C_HELD].suffix = EVENT_SUFFIX_SIGNAL_C_HELD,
    .events[OFFSET_EVENT_SIGNAL_C_NOT_HELD].suffix = EVENT_SUFFIX_SIGNAL_C_NOT_HELD,


    .events[OFFSET_EVENT_SIGNAL_D_RED].suffix = EVENT_SUFFIX_SIGNAL_D_RED,
    .events[OFFSET_EVENT_SIGNAL_D_YELLOW].suffix = EVENT_SUFFIX_SIGNAL_D_YELLOW,
    .events[OFFSET_EVENT_SIGNAL_D_GREEN].suffix = EVENT_SUFFIX_SIGNAL_D_GREEN,
    .events[OFFSET_EVENT_SIGNAL_D_DARK].suffix = EVENT_SUFFIX_SIGNAL_D_DARK,
    .events[OFFSET_EVENT_SIGNAL_D_LIT].suffix = EVENT_SUFFIX_SIGNAL_D_LIT,
    .events[OFFSET_EVENT_SIGNAL_D_NOT_LIT].suffix = EVENT_SUFFIX_SIGNAL_D_NOT_LIT,
    .events[OFFSET_EVENT_SIGNAL_D_HELD].suffix = EVENT_SUFFIX_SIGNAL_D_HELD,
    .events[OFFSET_EVENT_SIGNAL_D_NOT_HELD].suffix = EVENT_SUFFIX_SIGNAL_D_NOT_HELD,

    .events[OFFSET_EVENT_SIGNAL_STATE_A_STOP].suffix = EVENT_SUFFIX_SIGNAL_STATE_A_STOP,
    .events[OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP].suffix = EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP,
    .events[OFFSET_EVENT_SIGNAL_STATE_B_STOP].suffix = EVENT_SUFFIX_SIGNAL_STATE_B_STOP,
    .events[OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP].suffix = EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP,
    .events[OFFSET_EVENT_SIGNAL_STATE_CD_STOP].suffix = EVENT_SUFFIX_SIGNAL_STATE_CD_STOP,
    .events[OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP].suffix = EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP,

    .events[OFFSET_EVENT_VITAL_LOGIC_STATE_HELD].suffix = EVENT_SUFFIX_VITAL_LOGIC_STATE_HELD,
    .events[OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_LEFT].suffix = EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_LEFT,
    .events[OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_RIGHT].suffix = EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_RIGHT,
    .events[OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_BOTH].suffix = EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_BOTH,

};

void TurnoutBoss_Event_Engine_initialize() {

    for (int i = 0; i < MAX_EVENT_SUFFIX; i++) {

        TurnoutBoss_Event_Engine.events[i].state.send = FALSE;
        TurnoutBoss_Event_Engine.events[i].state.valid_producer = FALSE;

    }

}

void _next_event(void) {

    TurnoutBoss_Event_Engine.current_index = TurnoutBoss_Event_Engine.current_index + 1;
    if (TurnoutBoss_Event_Engine.current_index >= MAX_EVENT_SUFFIX)
        TurnoutBoss_Event_Engine.current_index = 0;

}

void TurnoutBoss_Event_Engine_run(openlcb_node_t *node) {

    event_id_t event_id;

    if (TurnoutBoss_Event_Engine.events[TurnoutBoss_Event_Engine.current_index].state.send) {

        event_id = (node->id << 16) + TurnoutBoss_Event_Engine.events[TurnoutBoss_Event_Engine.current_index].suffix;

        if (Application_send_event_pc_report(node, event_id)) {

            TurnoutBoss_Event_Engine.events[TurnoutBoss_Event_Engine.current_index].state.send = FALSE;

            _next_event();

        }


    } else {

        _next_event();

    }

}


