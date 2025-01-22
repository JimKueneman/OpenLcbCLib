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
 * \file turnoutboss_hardware_handler.h
 *
 * Scans the TurnoutBoss Hardware States and updates the TurnoutBoss_Signaling_States
 *
 * @author Jim Kueneman
 * @date 19 Jan 2025
 */

#include "xc.h"
#include <string.h>
#include "stdio.h"  // printf

#include "turnoutboss_hardware_handler.h"
#include "turnoutboss_drivers.h"
#include "turnoutboss_signaling_states.h"
#include "turnoutboss_event_engine.h"

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

#define PRINT_DEBUG

typedef struct {
    uint8_olcb_t state : 1;
    uint8_olcb_t filter : 4;

} input_filter_t;

typedef struct {
    input_filter_t turnout_normal;
    input_filter_t turnout_diverging;
    input_filter_t pushbutton_normal;
    input_filter_t pushbutton_diverging;
    input_filter_t occupancy_1;
    input_filter_t occupancy_2;
    input_filter_t occupancy_3;

} input_filters_t;

typedef struct {
    uint8_olcb_t turnout : 1;
    uint8_olcb_t signal_a;
    uint8_olcb_t signal_b;
    uint8_olcb_t signal_c;
    uint8_olcb_t signal_d;

} outputs_t;

input_filters_t _input_filters;
outputs_t _outputs;

void TurnoutBoss_Hardware_Handler_initalize(void) {

    memset(&_input_filters, 0x00, sizeof (_input_filters));
    memset(&_outputs, 0x00, sizeof (_outputs));

}

// Returns True if the state was changed 

uint8_olcb_t _increment_filter(input_filter_t* filter) {

    if (filter->state == 1) // is the state already 1? If so done.
        return FALSE;

    if (filter->filter < 5)
        filter->filter = filter->filter + 1;

    if (filter->filter < 5)
        return FALSE;

    // Change State
    filter->state = 1;

    return TRUE;

}

// Returns True if the state was changed 

uint8_olcb_t _decrement_filter(input_filter_t* filter) {

    if (filter->state == 0) // is the state already 0? If so done.
        return FALSE;

    if (filter->filter > 0)
        filter->filter = filter->filter - 1;

    if (filter->filter > 0)
        return FALSE;

    // Change State
    filter->state = 0;

    return TRUE;

}

void _check_occupancy_state_changes(void) {

    if (TRACK_DETECT_1_PIN) {

        if (_increment_filter(&_input_filters.occupancy_1)) {
            // State has changed to high

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_OCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OTL = TRUE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 1 Occupied\n");
#endif

            } else {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OTR = TRUE;
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 1 Occupied\n");
#endif

            }

        }

    } else {

        if (_decrement_filter(&_input_filters.occupancy_1)) {

            // State has changed to low
            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OTL = FALSE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 1 Unoccupied\n");
#endif

            } else {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OTR = FALSE;
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 1 Unoccupied\n");
#endif

            }

        }
    }


    if (TRACK_DETECT_2_PIN) {

        if (_increment_filter(&_input_filters.occupancy_2)) {
            // State has changed to high

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OMC = TRUE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 2 Occupied\n");
#endif

            } else {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_OCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OMR = TRUE;
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 2 Occupied\n");
#endif

            }

        }

    } else {

        if (_decrement_filter(&_input_filters.occupancy_2)) {
            // State has changed to low

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OMC = FALSE;
#ifdef PRINT_DEBUG 
                printf("Left Occupancy 2 Unoccupied\n");
#endif

            } else {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OMR = FALSE;
#ifdef PRINT_DEBUG 
                printf("Right Occupancy 2 Unoccupied\n");
#endif

            }

        }

    }


    if (TRACK_DETECT_3_PIN) {

        if (_increment_filter(&_input_filters.occupancy_3)) {
            // State has changed to high

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OSC = TRUE;
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

        if (_decrement_filter(&_input_filters.occupancy_3)) {
            // State has changed to low

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Event_Engine.events[OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED].state.send = TRUE;
                TurnoutBoss_Signaling_States.occupancy.OSC = FALSE;
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

void _check_pushbutton_state_changes_dualpushbuttons(void) {

    if (TURNOUT_PUSHBUTTON_NORMAL_PIN) {

        if (_increment_filter(&_input_filters.pushbutton_normal)) {
            // State has changed to high

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_CLOSED].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Signaling_States.turnout.TLC = NORMAL;

            } else {

                TurnoutBoss_Signaling_States.turnout.TRC = NORMAL;

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Normal Closed (dual)\n");
#endif

        }

    } else {

        if (_decrement_filter(&_input_filters.pushbutton_normal)) {
            // State has changed to low

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_OPEN].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

            } else {

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Normal Open (dual)\n");
#endif

        }

    }

    if (TURNOUT_PUSHBUTTON_DIVERGING_PIN) {

        if (_increment_filter(&_input_filters.pushbutton_diverging)) {
            // State has changed to high

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_CLOSED].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Signaling_States.turnout.TLC = DIVERGING;

            } else {

                TurnoutBoss_Signaling_States.turnout.TRC = DIVERGING;

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Diverging Closed (dual)\n");
#endif

        }

    } else {

        if (_decrement_filter(&_input_filters.pushbutton_diverging)) {
            // State has changed to low

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_OPEN].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

            } else {

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Diverging Open (dual)\n");
#endif

        }

    }

}

void _toggle_turnout_command_state(void) {

    if (TurnoutBoss_Signaling_States_board_location == BL) {

        if (TurnoutBoss_Signaling_States.turnout.TLC == NORMAL) {

            TurnoutBoss_Signaling_States.turnout.TLC = DIVERGING;

        } else {

            TurnoutBoss_Signaling_States.turnout.TLC = NORMAL;

        }

    } else {

        if (TurnoutBoss_Signaling_States.turnout.TRC == NORMAL) {

            TurnoutBoss_Signaling_States.turnout.TRC = DIVERGING;

        } else {

            TurnoutBoss_Signaling_States.turnout.TRC = NORMAL;

        }

    }

}

void _check_pushbutton_state_changes_singlepushbutton(void) {

    if (TURNOUT_PUSHBUTTON_NORMAL_PIN) {

        if (_increment_filter(&_input_filters.pushbutton_normal)) {
            // State has changed to high

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_CLOSED].state.send = TRUE;

            _toggle_turnout_command_state();

#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Normal Closed (single)\n");
#endif

        }

    } else {

        if (_decrement_filter(&_input_filters.pushbutton_normal)) {
            // State has changed to low

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_OPEN].state.send = TRUE;

#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Normal Open (single)\n");
#endif

        }

    }

    if (TURNOUT_PUSHBUTTON_DIVERGING_PIN) {

        if (_increment_filter(&_input_filters.pushbutton_diverging)) {
            // State has changed to high

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_CLOSED].state.send = TRUE;

            _toggle_turnout_command_state();

#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Diverging Closed (single)\n");
#endif

        }

    } else {

        if (_decrement_filter(&_input_filters.pushbutton_diverging)) {
            // State has changed to low

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_OPEN].state.send = TRUE;

#ifdef PRINT_DEBUG 
            printf("Turnout Pushbutton Diverging Open (single)\n");
#endif

        }

    }

}

void _check_pushbutton_state_changes(void) {

    switch (TurnoutBoss_Turnout_Pushbutton_Type) {

        case dualPushbuttons:
            _check_pushbutton_state_changes_dualpushbuttons();
            return;
        case singlePushbutton:
            _check_pushbutton_state_changes_singlepushbutton();
            return;
    }

}

void _check_turnout_position_state_changes_dualfeedback(void) {

    if (TURNOUT_POSITION_NORMAL_PIN) {

        if (_increment_filter(&_input_filters.turnout_normal)) {
            // State has changed to high

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_ACTIVE].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Signaling_States.turnout.TLO = NORMAL;

            } else {

                TurnoutBoss_Signaling_States.turnout.TRO = NORMAL;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Normal Active (dual)\n");
#endif

        }

    } else {

        if (_decrement_filter(&_input_filters.turnout_normal)) {
            // State has changed to low

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_INACTIVE].state.send = TRUE;

#ifdef PRINT_DEBUG 
            printf("Turnout Position Normal InActive (dual)\n");
#endif

        }

    }

    if (TURNOUT_POSITION_DIVERGING_PIN) {

        if (_increment_filter(&_input_filters.turnout_diverging)) {
            // State has changed to high

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_ACTIVE].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Signaling_States.turnout.TLO = DIVERGING;

            } else {

                TurnoutBoss_Signaling_States.turnout.TRO = DIVERGING;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Diverging Active (dual)\n");
#endif

        }

    } else {

        if (_decrement_filter(&_input_filters.turnout_diverging)) {
            // State has changed to low

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_INACTIVE].state.send = TRUE;

#ifdef PRINT_DEBUG 
            printf("Turnout Position Diverging InActive (dual)\n");
#endif

        }

    }

}

void _check_turnout_position_state_changes_singlefeedback(void) {

    if (TURNOUT_POSITION_NORMAL_PIN) {

        if (_increment_filter(&_input_filters.turnout_normal)) {
            // State has changed to high

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_ACTIVE].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Signaling_States.turnout.TLO = NORMAL;

            } else {

                TurnoutBoss_Signaling_States.turnout.TRO = NORMAL;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Normal Active (single)\n");
#endif

        }

    } else {

        if (_decrement_filter(&_input_filters.turnout_normal)) {
            // State has changed to low

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_INACTIVE].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Signaling_States.turnout.TLO = DIVERGING;

            } else {

                TurnoutBoss_Signaling_States.turnout.TRO = DIVERGING;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Normal InActive (single)\n");
#endif

        }

    }

    if (TURNOUT_POSITION_DIVERGING_PIN) {

        if (_increment_filter(&_input_filters.turnout_diverging)) {
            // State has changed to high

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_ACTIVE].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Signaling_States.turnout.TLO = NORMAL;

            } else {

                TurnoutBoss_Signaling_States.turnout.TRO = NORMAL;

            }
#ifdef PRINT_DEBUG 
            printf("Turnout Position Diverging Active (single)\n");
#endif

        }

    } else {

        if (_decrement_filter(&_input_filters.turnout_diverging)) {
            // State has changed to low

            TurnoutBoss_Event_Engine.events[OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_INACTIVE].state.send = TRUE;

            if (TurnoutBoss_Signaling_States_board_location == BL) {

                TurnoutBoss_Signaling_States.turnout.TLO = DIVERGING;

            } else {

                TurnoutBoss_Signaling_States.turnout.TRO = DIVERGING;

            }

#ifdef PRINT_DEBUG 
            printf("Turnout Position Diverging InActive (single)\n");
#endif

        }

    }

}

void _check_turnout_position_state_changes(void) {

    switch (TurnoutBoss_Turnout_Feedback_Type) {

        case unusedTurnoutFeedback:
            return;
        case dualTurnoutFeedback:
            _check_turnout_position_state_changes_dualfeedback();
            return;
        case singleTurnoutFeedback:
            _check_turnout_position_state_changes_singlefeedback();
            return;
            
    }
}

void TurnoutBoss_Hardware_Handler_scan_for_changes(void) {

    _check_occupancy_state_changes();
    _check_pushbutton_state_changes();
    _check_turnout_position_state_changes();

}
