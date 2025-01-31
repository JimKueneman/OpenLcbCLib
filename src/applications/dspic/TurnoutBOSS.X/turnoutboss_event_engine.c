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

#ifdef MPLAB
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/application.h"
#else
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/application.h"
#endif

#include "turnoutboss_types.h"

const event_suffix_t _event_suffix = {

    EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_OCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_OCCUPIED,
    EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED,

    EVENT_SUFFIX_TURNOUT_COMMAND_NORMAL,
    EVENT_SUFFIX_TURNOUT_COMMAND_DIVERGING,
    EVENT_SUFFIX_TURNOUT_FEEDBACK_NORMAL_ACTIVE,
    EVENT_SUFFIX_TURNOUT_FEEDBACK_NORMAL_INACTIVE,
    EVENT_SUFFIX_TURNOUT_FEEDBACK_DIVERGING_ACTIVE,
    EVENT_SUFFIX_TURNOUT_FEEDBACK_DIVERGING_INACTIVE,
    EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_OPEN,
    EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_CLOSED,
    EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_OPEN,
    EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_CLOSED,
    EVENT_SUFFIX_TURNOUT_OBSERVED_NORMAL,
    EVENT_SUFFIX_TURNOUT_OBSERVED_DIVERGING,
    EVENT_SUFFIX_TURNOUT_OBSERVED_IN_MOTION,

    EVENT_SUFFIX_SIGNAL_A_RED,
    EVENT_SUFFIX_SIGNAL_A_YELLOW,
    EVENT_SUFFIX_SIGNAL_A_GREEN,
    EVENT_SUFFIX_SIGNAL_A_DARK,

    EVENT_SUFFIX_SIGNAL_B_RED,
    EVENT_SUFFIX_SIGNAL_B_YELLOW,
    EVENT_SUFFIX_SIGNAL_B_GREEN,
    EVENT_SUFFIX_SIGNAL_B_DARK,

    EVENT_SUFFIX_SIGNAL_C_RED,
    EVENT_SUFFIX_SIGNAL_C_YELLOW,
    EVENT_SUFFIX_SIGNAL_C_GREEN,
    EVENT_SUFFIX_SIGNAL_C_DARK,

    EVENT_SUFFIX_SIGNAL_D_RED,
    EVENT_SUFFIX_SIGNAL_D_YELLOW,
    EVENT_SUFFIX_SIGNAL_D_GREEN,
    EVENT_SUFFIX_SIGNAL_D_DARK,

    EVENT_SUFFIX_SIGNAL_STATE_A_STOP,
    EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP,
    EVENT_SUFFIX_SIGNAL_STATE_B_STOP,
    EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP,
    EVENT_SUFFIX_SIGNAL_STATE_CD_STOP,
    EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP,

    EVENT_SUFFIX_VITAL_LOGIC_STATE_HELD,
    EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_LEFT,
    EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_RIGHT,
    EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_BOTH
};



void TurnoutBossEventEngine_initialize(send_event_engine_t* event_engine) {

    for (int i = 0; i < MAX_EVENT_SUFFIX; i++) {
        event_engine->events[i].suffix = _event_suffix[i];
        event_engine->events[i].state.send = FALSE;
        event_engine->events[i].state.valid_producer = FALSE;

    }

}

void _next_event(send_event_engine_t* event_engine) {

    event_engine->current_index = event_engine->current_index + 1;
    if (event_engine->current_index >= MAX_EVENT_SUFFIX)
        event_engine->current_index = 0;

}


 uint8_olcb_t TurnoutBossEventEngine_is_flushed(send_event_engine_t *event_engine) {

     for (int i = 0; i < MAX_EVENT_SUFFIX; i++)
     {

         if (event_engine->events[event_engine->current_index].state.send)

             return FALSE;
     }

     return TRUE;

 }

void TurnoutBossEventEngine_run(openlcb_node_t *node, send_event_engine_t *event_engine)
{

    event_id_t event_id;

    if (!(node->state.permitted || node->state.initalized))

        return;

    if (event_engine->events[event_engine->current_index].state.send) {

        event_id = (node->id << 16) + event_engine->events[event_engine->current_index].suffix;

        if (Application_send_event_pc_report(node, event_id)) {

            event_engine->events[event_engine->current_index].state.send = FALSE;

            _next_event(event_engine);

        }


    } else {

        _next_event(event_engine);

    }

}
