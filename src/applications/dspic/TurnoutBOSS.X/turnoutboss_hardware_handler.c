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


#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

#include "turnoutboss_types.h"
#include "turnoutboss_drivers.h"


#define INPUT_FILTER_COUNT 10



uint8_olcb_t _run_filter_inc(uint8_olcb_t filter) {

    if (filter >= INPUT_FILTER_COUNT)

        return INPUT_FILTER_COUNT;

    return filter + 1;

}

uint8_olcb_t _run_filter_dec(uint8_olcb_t filter) {

    if (filter == 0)

        return 0;

    return filter - 1;

}

void TurnoutBossHardwareHandler_scan_for_changes(hardware_input_states_t* hardware_states) {

    // *******************
    uint8_olcb_t filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (OCCUPANCY_DETECT_1_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    hardware_states->occupany_1 = (filter > INPUT_FILTER_COUNT / 2);

    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (OCCUPANCY_DETECT_2_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    hardware_states->occupany_2 = (filter > INPUT_FILTER_COUNT / 2);


    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (OCCUPANCY_DETECT_3_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    hardware_states->occupany_3 = (filter > INPUT_FILTER_COUNT / 2);

    
    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TURNOUT_POSITION_NORMAL_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    hardware_states->turnout_normal_feedback = (filter > INPUT_FILTER_COUNT / 2);
    
    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TURNOUT_POSITION_DIVERGING_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    hardware_states->turnout_diverging_feedback = (filter > INPUT_FILTER_COUNT / 2);
    
    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TURNOUT_PUSHBUTTON_NORMAL_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    hardware_states->turnout_pushbutton_normal = (filter > INPUT_FILTER_COUNT / 2);
    
    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TURNOUT_PUSHBUTTON_DIVERGING_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    hardware_states->turnout_pushbutton_diverging = (filter > INPUT_FILTER_COUNT / 2);


}
