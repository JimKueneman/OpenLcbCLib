
/** \copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * \file callbacks_broadcast_time.c
 *
 * Broadcast time callback implementations for the ComplianceTestNode.
 *
 * @author Jim Kueneman
 * @date 15 Mar 2026
 */

#include "callbacks_broadcast_time.h"

#include <stdio.h>

#include "../openlcb_c_lib/openlcb/openlcb_defines.h"
#include "../openlcb_c_lib/openlcb/openlcb_application_broadcast_time.h"

void CallbacksBroadcastTime_on_time_changed(broadcast_clock_t *clock) {

    printf("Time: %2d:%d, rate: %d\n", clock->state.time.hour, clock->state.time.minute, clock->state.rate.rate);

}

bool CallbacksBroadcastTime_on_login_complete_consumer(openlcb_node_t *openlcb_node) {

    OpenLcbApplicationBroadcastTime_start(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    return OpenLcbApplicationBroadcastTime_send_query(openlcb_node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

}

bool CallbacksBroadcastTime_on_login_complete_producer(openlcb_node_t *openlcb_node) {

    // Initialize producer clock with sane defaults
    broadcast_clock_state_t *clock = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);

    if (clock) {

        clock->time.hour = 0;
        clock->time.minute = 0;
        clock->time.valid = true;
        clock->date.month = 1;
        clock->date.day = 1;
        clock->date.valid = true;
        clock->year.year = 2026;
        clock->year.valid = true;
        clock->rate.rate = 4;      // 1.0x real-time
        clock->rate.valid = true;

    }

    // Library auto-triggers the §6.1 startup sync sequence when the
    // producer node transitions to RUNSTATE_RUN.
    return true;

}
