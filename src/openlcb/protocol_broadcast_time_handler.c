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
 * @file protocol_broadcast_time_handler.c
 * @brief Broadcast Time Protocol message handler implementation
 *
 * @details Decodes incoming broadcast time Event IDs and updates the singleton
 * clock state in the application broadcast time module. Fires application
 * callbacks when state changes.
 *
 * @author Jim Kueneman
 * @date 14 Feb 2026
 */

#include "protocol_broadcast_time_handler.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_application_broadcast_time.h"


static const interface_openlcb_protocol_broadcast_time_handler_t *_interface;


void ProtocolBroadcastTime_initialize(const interface_openlcb_protocol_broadcast_time_handler_t *interface_openlcb_protocol_broadcast_time_handler) {

    _interface = interface_openlcb_protocol_broadcast_time_handler;

}

const interface_openlcb_protocol_broadcast_time_handler_t* ProtocolBroadcastTime_get_interface(void) {

    return _interface;

}


static void _handle_report_time(openlcb_node_t *node, broadcast_clock_state_t *clock, event_id_t event_id) {

    uint8_t hour;
    uint8_t minute;

    if (OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute)) {

        clock->time.hour = hour;
        clock->time.minute = minute;
        clock->time.valid = 1;

        if (_interface && _interface->on_time_received) {

            _interface->on_time_received(node, clock);

        }

    }

}


static void _handle_report_date(openlcb_node_t *node, broadcast_clock_state_t *clock, event_id_t event_id) {

    uint8_t month;
    uint8_t day;

    if (OpenLcbUtilities_extract_date_from_event_id(event_id, &month, &day)) {

        clock->date.month = month;
        clock->date.day = day;
        clock->date.valid = 1;

        if (_interface && _interface->on_date_received) {

            _interface->on_date_received(node, clock);

        }

    }

}


static void _handle_report_year(openlcb_node_t *node, broadcast_clock_state_t *clock, event_id_t event_id) {

    uint16_t year;

    if (OpenLcbUtilities_extract_year_from_event_id(event_id, &year)) {

        clock->year.year = year;
        clock->year.valid = 1;

        if (_interface && _interface->on_year_received) {

            _interface->on_year_received(node, clock);

        }

    }

}


static void _handle_report_rate(openlcb_node_t *node, broadcast_clock_state_t *clock, event_id_t event_id) {

    int16_t rate;

    if (OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate)) {

        clock->rate.rate = rate;
        clock->rate.valid = 1;

        if (_interface && _interface->on_rate_received) {

            _interface->on_rate_received(node, clock);

        }

    }

}


static void _handle_start(openlcb_node_t *node, broadcast_clock_state_t *clock) {

    clock->is_running = 1;

    if (_interface && _interface->on_clock_started) {

        _interface->on_clock_started(node, clock);

    }

}


static void _handle_stop(openlcb_node_t *node, broadcast_clock_state_t *clock) {

    clock->is_running = 0;

    if (_interface && _interface->on_clock_stopped) {

        _interface->on_clock_stopped(node, clock);

    }

}


static void _handle_date_rollover(openlcb_node_t *node, broadcast_clock_state_t *clock) {

    if (_interface && _interface->on_date_rollover) {

        _interface->on_date_rollover(node, clock);

    }

}


void ProtocolBroadcastTime_handle_time_event(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id) {

    broadcast_time_event_type_enum event_type;
    openlcb_node_t *node;
    uint64_t clock_id;
    broadcast_clock_state_t *clock;

    if (!statemachine_info) {

        return;

    }

    node = statemachine_info->openlcb_node;

    if (!node) {

        return;

    }

    if (node->index != 0) {

        return;

    }

    clock_id = OpenLcbUtilities_extract_clock_id_from_time_event(event_id);
    clock = OpenLcbApplicationBroadcastTime_get_clock(clock_id);

    if (!clock) {

        return;

    }

    event_type = OpenLcbUtilities_get_broadcast_time_event_type(event_id);

    switch (event_type) {

        case BROADCAST_TIME_EVENT_REPORT_TIME:
            _handle_report_time(node, clock, event_id);
            break;

        case BROADCAST_TIME_EVENT_REPORT_DATE:
            _handle_report_date(node, clock, event_id);
            break;

        case BROADCAST_TIME_EVENT_REPORT_YEAR:
            _handle_report_year(node, clock, event_id);
            break;

        case BROADCAST_TIME_EVENT_REPORT_RATE:
            _handle_report_rate(node, clock, event_id);
            break;

        case BROADCAST_TIME_EVENT_SET_TIME:
            _handle_report_time(node, clock, event_id);
            break;

        case BROADCAST_TIME_EVENT_SET_DATE:
            _handle_report_date(node, clock, event_id);
            break;

        case BROADCAST_TIME_EVENT_SET_YEAR:
            _handle_report_year(node, clock, event_id);
            break;

        case BROADCAST_TIME_EVENT_SET_RATE:
            _handle_report_rate(node, clock, event_id);
            break;

        case BROADCAST_TIME_EVENT_START:
            _handle_start(node, clock);
            break;

        case BROADCAST_TIME_EVENT_STOP:
            _handle_stop(node, clock);
            break;

        case BROADCAST_TIME_EVENT_DATE_ROLLOVER:
            _handle_date_rollover(node, clock);
            break;

        case BROADCAST_TIME_EVENT_QUERY:
            break;

        default:
            break;

    }

}
