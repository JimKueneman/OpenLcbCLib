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
 * \file turnoutboss_hardware_handler_turnout_feedback.c
 *
 * Scans the TurnoutBoss Hardware Turnout Feedback and set the States and Events to send
 *
 * @author Jim Kueneman
 * @date 19 Jan 2025
 */

#include "xc.h"
#include <string.h>
#include "stdio.h"  // printf

#include "turnoutboss_hardware_handler_turnout_feedback.h"

#include "turnoutboss_hardware_handler.h"
#include "turnoutboss_drivers.h"
#include "turnoutboss_signaling_states.h"
#include "turnoutboss_event_engine.h"
#include "turnoutboss_board_configuration.h"

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

void _check_turnout_position_state_changes_dualfeedback(input_filters_t* filters, signaling_state_t* signaling_state, send_event_engine_t* event_engine, board_type_enum_t board_location) {

    if (TURNOUT_POSITION_NORMAL_PIN) {

        if (filters->inc_filter(&filters->turnout_normal)) {
            // State has changed to high

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_ACTIVE].state.send = TRUE;

            if (board_location == BL) {

                signaling_state->turnout.TLO = NORMAL;

            } else {

                signaling_state->turnout.TRO = NORMAL;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Normal Active (dual)\n");
#endif

        }

    } else {

        if (filters->dec_filter(&filters->turnout_normal)) {
            // State has changed to low

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_INACTIVE].state.send = TRUE;

#ifdef PRINT_DEBUG 
            printf("Turnout Position Normal InActive (dual)\n");
#endif

        }

    }

    if (TURNOUT_POSITION_DIVERGING_PIN) {

        if (filters->inc_filter(&filters->turnout_diverging)) {
            // State has changed to high

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_ACTIVE].state.send = TRUE;

            if (board_location == BL) {

                signaling_state->turnout.TLO = DIVERGING;

            } else {

                signaling_state->turnout.TRO = DIVERGING;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Diverging Active (dual)\n");
#endif

        }

    } else {

        if (filters->dec_filter(&filters->turnout_diverging)) {
            // State has changed to low

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_INACTIVE].state.send = TRUE;

#ifdef PRINT_DEBUG 
            printf("Turnout Position Diverging InActive (dual)\n");
#endif

        }

    }

}

void _check_turnout_position_state_changes_singlefeedback(input_filters_t* filters, signaling_state_t* signaling_state, send_event_engine_t* event_engine, board_type_enum_t board_location) {

    if (TURNOUT_POSITION_NORMAL_PIN) {

        if (filters->inc_filter(&filters->turnout_normal)) {
            // State has changed to high

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_ACTIVE].state.send = TRUE;

            if (board_location == BL) {

                signaling_state->turnout.TLO = NORMAL;

            } else {

                signaling_state->turnout.TRO = NORMAL;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Normal Active (single)\n");
#endif

        }

    } else {

        if (filters->dec_filter(&filters->turnout_normal)) {
            // State has changed to low

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_INACTIVE].state.send = TRUE;

            if (board_location == BL) {

                signaling_state->turnout.TLO = DIVERGING;

            } else {

                signaling_state->turnout.TRO = DIVERGING;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Normal InActive (single)\n");
#endif

        }

    }

    if (TURNOUT_POSITION_DIVERGING_PIN) {

        if (filters->inc_filter(&filters->turnout_diverging)) {
            // State has changed to high

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_ACTIVE].state.send = TRUE;

            if (board_location == BL) {

                signaling_state->turnout.TLO = NORMAL;

            } else {

                signaling_state->turnout.TRO = NORMAL;

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Position Diverging Active (single)\n");
#endif

        }

    } else {

        if (filters->dec_filter(&filters->turnout_diverging)) {
            // State has changed to low

            event_engine->events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_INACTIVE].state.send = TRUE;

            if (board_location == BL) {

                signaling_state->turnout.TLO = DIVERGING;

            } else {

                signaling_state->turnout.TRO = DIVERGING;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Diverging InActive (single)\n");
#endif

        }

    }

}

void TurnoutBossHardwareHandlerTurnoutFeedback_check_state_changes(input_filters_t* filters, signaling_state_t* signaling_state, send_event_engine_t* event_engine, board_type_enum_t board_location, turnout_feedback_type_enum_t feedback_type) {

    switch (feedback_type) {

        case unusedTurnoutFeedback:
            return;
        case dualTurnoutFeedback:
            _check_turnout_position_state_changes_dualfeedback(filters, signaling_state, event_engine, board_location);
            return;
        case singleTurnoutFeedback:
            _check_turnout_position_state_changes_singlefeedback(filters, signaling_state, event_engine, board_location);
            return;
            
    }
}
