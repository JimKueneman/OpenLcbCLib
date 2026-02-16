/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * @file openlcb_application_broadcast_time.h
 * @brief Application-level Broadcast Time Protocol module
 *
 * @details Provides a singleton clock array and application API for the
 * OpenLCB Broadcast Time Protocol. This module owns the clock state and
 * provides setup, accessor, send, and time-tick functions.
 *
 * The protocol handler (protocol_broadcast_time_handler.c) updates state
 * in this module when events are received from the network.
 *
 * This module is optional — applications that don't need broadcast time
 * should not include it.
 *
 * @author Jim Kueneman
 * @date 14 Feb 2026
 */

#ifndef __OPENLCB_APPLICATION_BROADCAST_TIME__
#define __OPENLCB_APPLICATION_BROADCAST_TIME__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifndef BROADCAST_TIME_MAX_CUSTOM_CLOCKS
#define BROADCAST_TIME_MAX_CUSTOM_CLOCKS 4
#endif

#define BROADCAST_TIME_WELLKNOWN_CLOCK_COUNT 4
#define BROADCAST_TIME_TOTAL_CLOCK_COUNT (BROADCAST_TIME_WELLKNOWN_CLOCK_COUNT + BROADCAST_TIME_MAX_CUSTOM_CLOCKS)

    /**
     * @struct broadcast_clock_t
     * @brief A clock slot with state and subscription flags
     */
    typedef struct {

        broadcast_clock_state_t state;
        bool is_consumer : 1;
        bool is_producer : 1;
        bool is_allocated : 1;

    } broadcast_clock_t;

#ifdef __cplusplus
extern "C" {
#endif

    // Initialization

    extern void OpenLcbApplicationBroadcastTime_initialize(void);

    // Setup — registers a clock slot and event ranges on the node

    extern broadcast_clock_state_t* OpenLcbApplicationBroadcastTime_setup_consumer(openlcb_node_t *openlcb_node, event_id_t clock_id);
    extern broadcast_clock_state_t* OpenLcbApplicationBroadcastTime_setup_producer(openlcb_node_t *openlcb_node, event_id_t clock_id);

    // Accessors

    extern broadcast_clock_state_t* OpenLcbApplicationBroadcastTime_get_clock(event_id_t clock_id);
    extern bool OpenLcbApplicationBroadcastTime_is_consumer(event_id_t clock_id);
    extern bool OpenLcbApplicationBroadcastTime_is_producer(event_id_t clock_id);

    extern void OpenLcbApplicationBroadcastTime_start(event_id_t clock_id);
    extern void OpenLcbApplicationBroadcastTime_stop(event_id_t clock_id);

    // Timer tick — advances all active running clocks

    extern void OpenLcbApplicationBroadcastTime_100ms_time_tick(void);

    // Producer send "reporting" functions (for clock generators)

    extern bool OpenLcbApplicationBroadcastTime_send_report_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute);
    extern bool OpenLcbApplicationBroadcastTime_send_report_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day);
    extern bool OpenLcbApplicationBroadcastTime_send_report_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year);
    extern bool OpenLcbApplicationBroadcastTime_send_report_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate);
    extern bool OpenLcbApplicationBroadcastTime_send_start(openlcb_node_t *openlcb_node, event_id_t clock_id);
    extern bool OpenLcbApplicationBroadcastTime_send_stop(openlcb_node_t *openlcb_node, event_id_t clock_id);
    extern bool OpenLcbApplicationBroadcastTime_send_date_rollover(openlcb_node_t *openlcb_node, event_id_t clock_id);
    extern bool OpenLcbApplicationBroadcastTime_send_query_reply(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t next_hour, uint8_t next_minute);

    // Consumer send functions

    extern bool OpenLcbApplicationBroadcastTime_send_query(openlcb_node_t *openlcb_node, event_id_t clock_id);

    // Controller send "set clock source" functions (any node can set a clock generator)

    extern bool OpenLcbApplicationBroadcastTime_send_set_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute);
    extern bool OpenLcbApplicationBroadcastTime_send_set_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day);
    extern bool OpenLcbApplicationBroadcastTime_send_set_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year);
    extern bool OpenLcbApplicationBroadcastTime_send_set_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate);
    extern bool OpenLcbApplicationBroadcastTime_send_command_start(openlcb_node_t *openlcb_node, event_id_t clock_id);
    extern bool OpenLcbApplicationBroadcastTime_send_command_stop(openlcb_node_t *openlcb_node, event_id_t clock_id);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_APPLICATION_BROADCAST_TIME__ */
