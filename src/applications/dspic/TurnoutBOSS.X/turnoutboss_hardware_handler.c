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
 * \file turnoutboss_hardware_handler.c
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

#include "turnoutboss_hardware_handler_occupancy.h"
#include "turnoutboss_hardware_handler_pushbuttons.h"
#include "turnoutboss_hardware_handler_turnout_feedback.h"
#include "turnoutboss_drivers.h"
#include "turnoutboss_signaling_states.h"
#include "turnoutboss_event_engine.h"
#include "turnoutboss_board_configuration.h"

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

//#define PRINT_DEBUG


input_filters_t _input_filters;
outputs_t _outputs;


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

void TurnoutBossHardwareHandler_initalize(void) {

    memset(&_input_filters, 0x00, sizeof (_input_filters));
    _input_filters.dec_filter = &_decrement_filter;
    _input_filters.inc_filter = &_increment_filter;
    
    memset(&_outputs, 0x00, sizeof (_outputs));

}



void TurnoutBossHardwareHandler_scan_for_changes(void) {

    TurnoutBossHardwareHandlerOccupancy_check_state_changes(
            &_input_filters, 
            &TurnoutBossSignalingStates, 
            &TurnoutBossEventEngine, 
            TurnoutBossBoardConfiguration_board_location
            );
    TurnoutBossHardwareHandler_Pushbuttons_check_state_changes(
            &_input_filters, 
            &TurnoutBossSignalingStates, 
            &TurnoutBossEventEngine, 
            TurnoutBossBoardConfiguration_board_location, 
            TurnoutBossBoardConfiguration_pushbutton_type
            );
    TurnoutBossHardwareHandlerTurnoutFeedback_check_state_changes(
            &_input_filters,
            &TurnoutBossSignalingStates, 
            &TurnoutBossEventEngine, 
            TurnoutBossBoardConfiguration_board_location, 
            TurnoutBossBoardConfiguration_feedback_type
            );

}
