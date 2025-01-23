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
 * \file turnoutboss_hardware_handler_occupancy.c
 *
 * Scans the TurnoutBoss Hardware Occupancy Detectors and set the States and Events to send
 *
 * @author Jim Kueneman
 * @date 23 Jan 2025
 */

#include "xc.h"
#include <string.h>
#include "stdio.h"  // printf

#include "turnoutboss_hardware_handler_occupancy.h"

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

void TurnoutBossHardwareHandlerOccupancy_check_state_changes(input_filters_t* filters, signaling_state_t* signaling_state, send_event_engine_t* event_engine, board_type_enum_t board_location) {

    if (TRACK_DETECT_1_PIN) {

        if (filters->inc_filter(&filters->occupancy_1)) {
            // State has changed to high

            if (board_location == BL) {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_OCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OTL = TRUE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 1 Occupied\n");
#endif

            } else {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OTR = TRUE;
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 1 Occupied\n");
#endif

            }

        }

    } else {

        if (filters->dec_filter(&filters->occupancy_1)) {

            // State has changed to low
            if (board_location == BL) {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OTL = FALSE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 1 Unoccupied\n");
#endif

            } else {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OTR = FALSE;
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 1 Unoccupied\n");
#endif

            }

        }
    }


    if (TRACK_DETECT_2_PIN) {

        if (filters->inc_filter(&filters->occupancy_2)) {
            // State has changed to high

            if (board_location == BL) {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OMC = TRUE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 2 Occupied\n");
#endif

            } else {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_OCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OMR = TRUE;
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 2 Occupied\n");
#endif

            }

        }

    } else {

        if (filters->dec_filter(&filters->occupancy_2)) {
            // State has changed to low

            if (board_location == BL) {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OMC = FALSE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 2 Unoccupied\n");
#endif

            } else {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OMR = FALSE;
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 2 Unoccupied\n");
#endif

            }

        }

    }


    if (TRACK_DETECT_3_PIN) {

        if (filters->inc_filter(&filters->occupancy_3)) {
            // State has changed to high

            if (board_location == BL) {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OSC = TRUE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 3 Occupied\n");
#endif

            } else {

                // unused on a BR node
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 3 Occupied\n");
#endif

            }

        }

    } else {

        if (filters->dec_filter(&filters->occupancy_3)) {
            // State has changed to low

            if (board_location == BL) {

                event_engine->events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED].state.send = TRUE;
                signaling_state->occupancy.OSC = FALSE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 3 Occupied\n");
#endif

            } else {

                // unused on a BR node
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 3 Occupied\n");
#endif

            }

        }

    }


}
