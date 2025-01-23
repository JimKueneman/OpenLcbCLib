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
 * \file turnoutboss_hardware_handler_pushbuttons.c
 *
 * Scans the TurnoutBoss Hardware Pushbuttons and set the States and Events to send
 *
 * @author Jim Kueneman
 * @date 19 Jan 2025
 */


#include "xc.h"
#include <string.h>
#include "stdio.h"  // printf

#include "turnoutboss_hardware_handler_pushbuttons.h"

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


void _check_pushbutton_state_changes_dualpushbuttons(input_filters_t* filters, signaling_state_t* signaling_state, send_event_engine_t* event_engine, board_type_enum_t board_location) {

    if (TURNOUT_PUSHBUTTON_NORMAL_PIN) {

        if (filters->inc_filter(&filters->pushbutton_normal)) {
            // State has changed to high

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_CLOSED].state.send = TRUE;

            if (board_location == BL) {

                signaling_state->turnout.TLC = NORMAL;

            } else {

                signaling_state->turnout.TRC = NORMAL;

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Normal Closed (dual)\n");
#endif

        }

    } else {

        if (filters->dec_filter(&filters->pushbutton_normal)) {
            // State has changed to low

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_OPEN].state.send = TRUE;

            if (board_location == BL) {

            } else {

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Normal Open (dual)\n");
#endif

        }

    }

    if (TURNOUT_PUSHBUTTON_DIVERGING_PIN) {

        if (filters->inc_filter(&filters->pushbutton_diverging)) {
            // State has changed to high

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_CLOSED].state.send = TRUE;

            if (board_location == BL) {

                signaling_state->turnout.TLC = DIVERGING;

            } else {

                signaling_state->turnout.TRC = DIVERGING;

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Diverging Closed (dual)\n");
#endif

        }

    } else {

        if (filters->dec_filter(&filters->pushbutton_diverging)) {
            // State has changed to low

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_OPEN].state.send = TRUE;

            if (board_location == BL) {

            } else {

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Diverging Open (dual)\n");
#endif

        }

    }

}

void _toggle_turnout_command_state(signaling_state_t* signaling_state, board_type_enum_t board_location) {

    if (board_location == BL) {

        if (signaling_state->turnout.TLC == NORMAL) {

            signaling_state->turnout.TLC = DIVERGING;

        } else {

            signaling_state->turnout.TLC = NORMAL;

        }

    } else {

        if (signaling_state->turnout.TRC == NORMAL) {

            signaling_state->turnout.TRC = DIVERGING;

        } else {

            signaling_state->turnout.TRC = NORMAL;

        }

    }

}

void _check_pushbutton_state_changes_singlepushbutton(input_filters_t* filters, signaling_state_t* signaling_state, send_event_engine_t* event_engine, board_type_enum_t board_location) {

    if (TURNOUT_PUSHBUTTON_NORMAL_PIN) {

        if (filters->inc_filter(&filters->pushbutton_normal)) {
            // State has changed to high

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_CLOSED].state.send = TRUE;

            _toggle_turnout_command_state(signaling_state, board_location);

#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Normal Closed (single)\n");
#endif

        }

    } else {

        if (filters->dec_filter(&filters->pushbutton_normal)) {
            // State has changed to low

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_OPEN].state.send = TRUE;

#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Normal Open (single)\n");
#endif

        }

    }

    if (TURNOUT_PUSHBUTTON_DIVERGING_PIN) {

        if (filters->inc_filter(&filters->pushbutton_diverging)) {
            // State has changed to high

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_CLOSED].state.send = TRUE;

            _toggle_turnout_command_state(signaling_state, board_location);

#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Diverging Closed (single)\n");
#endif

        }

    } else {

        if (filters->dec_filter(&filters->pushbutton_diverging)) {
            // State has changed to low

            event_engine->events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_OPEN].state.send = TRUE;

#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Diverging Open (single)\n");
#endif

        }

    }

}

void TurnoutBossHardwareHandler_Pushbuttons_check_state_changes(input_filters_t* filters, signaling_state_t* signaling_state, send_event_engine_t* event_engine, board_type_enum_t board_location, pushbutton_type_enum_t pushbutton_type) {

    switch (pushbutton_type) {

        case dualPushbuttons:
            _check_pushbutton_state_changes_dualpushbuttons(filters, signaling_state, event_engine, board_location);
            return;
        case singlePushbutton:
            _check_pushbutton_state_changes_singlepushbutton(filters, signaling_state, event_engine, board_location);
            return;
    }

}