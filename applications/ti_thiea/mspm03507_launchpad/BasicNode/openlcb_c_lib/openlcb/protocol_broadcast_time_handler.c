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
 * @date 20 Mar 2026
 */

#include "protocol_broadcast_time_handler.h"

#include "openlcb_config.h"

#ifdef OPENLCB_COMPILE_BROADCAST_TIME

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_application_broadcast_time.h"


    /** @brief Stored callback interface pointer. */
static const interface_openlcb_protocol_broadcast_time_handler_t *_interface;


    /**
     * @brief Stores the callback interface.  Call once at startup.
     *
     * @verbatim
     * @param interface_openlcb_protocol_broadcast_time_handler  Populated callback table.
     * @endverbatim
     */
void ProtocolBroadcastTime_initialize(const interface_openlcb_protocol_broadcast_time_handler_t *interface_openlcb_protocol_broadcast_time_handler) {

    _interface = interface_openlcb_protocol_broadcast_time_handler;

}

    /** @brief Decode and store time-of-day from event ID, fire callback. */
static void _handle_report_time(openlcb_node_t *node, broadcast_clock_state_t *clock, event_id_t event_id) {

    uint8_t hour;
    uint8_t minute;

    if (ProtocolBroadcastTime_extract_time(event_id, &hour, &minute)) {

        clock->time.hour = hour;
        clock->time.minute = minute;
        clock->time.valid = 1;
        clock->ms_accumulator = 0;

        if (_interface && _interface->on_time_received) {

            _interface->on_time_received(node, clock);

        }

    }

}


    /** @brief Decode and store date from event ID, fire callback. */
static void _handle_report_date(openlcb_node_t *node, broadcast_clock_state_t *clock, event_id_t event_id) {

    uint8_t month;
    uint8_t day;

    if (ProtocolBroadcastTime_extract_date(event_id, &month, &day)) {

        clock->date.month = month;
        clock->date.day = day;
        clock->date.valid = 1;

        if (_interface && _interface->on_date_received) {

            _interface->on_date_received(node, clock);

        }

    }

}


    /** @brief Decode and store year from event ID, fire callback. */
static void _handle_report_year(openlcb_node_t *node, broadcast_clock_state_t *clock, event_id_t event_id) {

    uint16_t year;

    if (ProtocolBroadcastTime_extract_year(event_id, &year)) {

        clock->year.year = year;
        clock->year.valid = 1;

        if (_interface && _interface->on_year_received) {

            _interface->on_year_received(node, clock);

        }

    }

}


    /** @brief Decode and store clock rate from event ID, fire callback. */
static void _handle_report_rate(openlcb_node_t *node, broadcast_clock_state_t *clock, event_id_t event_id) {

    int16_t rate;

    if (ProtocolBroadcastTime_extract_rate(event_id, &rate)) {

        clock->rate.rate = rate;
        clock->rate.valid = 1;
        clock->ms_accumulator = 0;

        if (_interface && _interface->on_rate_received) {

            _interface->on_rate_received(node, clock);

        }

    }

}


    /** @brief Set clock running flag and fire callback. */
static void _handle_start(openlcb_node_t *node, broadcast_clock_state_t *clock) {

    clock->is_running = true;
    clock->ms_accumulator = 0;

    if (_interface && _interface->on_clock_started) {

        _interface->on_clock_started(node, clock);

    }

}


    /** @brief Clear clock running flag and fire callback. */
static void _handle_stop(openlcb_node_t *node, broadcast_clock_state_t *clock) {

    clock->is_running = false;

    if (_interface && _interface->on_clock_stopped) {

        _interface->on_clock_stopped(node, clock);

    }

}


    /** @brief Fire date-rollover callback. */
static void _handle_date_rollover(openlcb_node_t *node, broadcast_clock_state_t *clock) {

    if (_interface && _interface->on_date_rollover) {

        _interface->on_date_rollover(node, clock);

    }

}


    /**
     * @brief Handles incoming broadcast time events.
     *
     * @details Decodes the event type, looks up the matching clock, and
     * dispatches to the appropriate sub-handler.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param event_id           Full 64-bit event_id_t containing encoded time data.
     * @endverbatim
     */
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

    clock_id = ProtocolBroadcastTime_extract_clock_id(event_id);
    clock = OpenLcbApplicationBroadcastTime_get_clock(clock_id);

    if (!clock) {

        return;

    }

    event_type = ProtocolBroadcastTime_get_event_type(event_id);

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
            if (OpenLcbApplicationBroadcastTime_is_producer(clock_id)) {

                _handle_report_time(node, clock, event_id);

                // Send immediate Report Time PCER so consumers see the change right away
                OpenLcbApplicationBroadcastTime_send_report_time(node, clock_id,
                    clock->time.hour, clock->time.minute);

                // Start/reset 3-second coalescing timer for full sync sequence
                OpenLcbApplicationBroadcastTime_trigger_sync_delay(clock_id);

            }
            break;

        case BROADCAST_TIME_EVENT_SET_DATE:
            if (OpenLcbApplicationBroadcastTime_is_producer(clock_id)) {

                _handle_report_date(node, clock, event_id);
                OpenLcbApplicationBroadcastTime_trigger_sync_delay(clock_id);

            }
            break;

        case BROADCAST_TIME_EVENT_SET_YEAR:
            if (OpenLcbApplicationBroadcastTime_is_producer(clock_id)) {

                _handle_report_year(node, clock, event_id);
                OpenLcbApplicationBroadcastTime_trigger_sync_delay(clock_id);

            }
            break;

        case BROADCAST_TIME_EVENT_SET_RATE:
            if (OpenLcbApplicationBroadcastTime_is_producer(clock_id)) {

                _handle_report_rate(node, clock, event_id);
                OpenLcbApplicationBroadcastTime_trigger_sync_delay(clock_id);

            }
            break;

        case BROADCAST_TIME_EVENT_START:
            _handle_start(node, clock);
            if (OpenLcbApplicationBroadcastTime_is_producer(clock_id)) {

                OpenLcbApplicationBroadcastTime_trigger_sync_delay(clock_id);

            }
            break;

        case BROADCAST_TIME_EVENT_STOP:
            _handle_stop(node, clock);
            if (OpenLcbApplicationBroadcastTime_is_producer(clock_id)) {

                OpenLcbApplicationBroadcastTime_trigger_sync_delay(clock_id);

            }
            break;

        case BROADCAST_TIME_EVENT_DATE_ROLLOVER:
            _handle_date_rollover(node, clock);
            break;

        case BROADCAST_TIME_EVENT_QUERY:
            if (OpenLcbApplicationBroadcastTime_is_producer(clock_id)) {

                OpenLcbApplicationBroadcastTime_trigger_query_reply(clock_id);

            }
            break;

        default:
            break;

    }

}

// =============================================================================
// Broadcast Time Event ID Utilities
//
// Moved from openlcb_utilities.c so the linker only pulls in broadcast-time
// code when OPENLCB_COMPILE_BROADCAST_TIME is defined.
// =============================================================================

    /** @brief Returns true if the event ID belongs to the broadcast time event space. */
bool ProtocolBroadcastTime_is_time_event(event_id_t event_id) {

    uint64_t clock_id;

    clock_id = event_id & BROADCAST_TIME_MASK_CLOCK_ID;

    if (clock_id == BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK) {

        return true;

    }

    if (clock_id == BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK) {

        return true;

    }

    if (clock_id == BROADCAST_TIME_ID_ALTERNATE_CLOCK_1) {

        return true;

    }

    if (clock_id == BROADCAST_TIME_ID_ALTERNATE_CLOCK_2) {

        return true;

    }

    // Check registered custom clocks
    return OpenLcbApplicationBroadcastTime_get_clock(clock_id) != NULL;

}

    /** @brief Extracts the 48-bit clock ID (upper 6 bytes) from a broadcast time event ID. */
uint64_t ProtocolBroadcastTime_extract_clock_id(event_id_t event_id) {

    return event_id & BROADCAST_TIME_MASK_CLOCK_ID;

}

    /** @brief Returns the @ref broadcast_time_event_type_enum for a broadcast time event ID. */
broadcast_time_event_type_enum ProtocolBroadcastTime_get_event_type(event_id_t event_id) {

    uint16_t command_data;

    command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

    if (command_data == BROADCAST_TIME_QUERY) {

        return BROADCAST_TIME_EVENT_QUERY;

    }

    if (command_data == BROADCAST_TIME_STOP) {

        return BROADCAST_TIME_EVENT_STOP;

    }

    if (command_data == BROADCAST_TIME_START) {

        return BROADCAST_TIME_EVENT_START;

    }

    if (command_data == BROADCAST_TIME_DATE_ROLLOVER) {

        return BROADCAST_TIME_EVENT_DATE_ROLLOVER;

    }

    // Set Rate: 0xC000-0xCFFF
    if (command_data >= BROADCAST_TIME_SET_RATE_BASE && command_data <= 0xCFFF) {

        return BROADCAST_TIME_EVENT_SET_RATE;

    }

    // Set Year: 0xB000-0xBFFF
    if (command_data >= BROADCAST_TIME_SET_YEAR_BASE && command_data <= 0xBFFF) {

        return BROADCAST_TIME_EVENT_SET_YEAR;

    }

    // Set Date: 0xA100-0xACFF
    if (command_data >= BROADCAST_TIME_SET_DATE_BASE && command_data <= 0xACFF) {

        return BROADCAST_TIME_EVENT_SET_DATE;

    }

    // Set Time: 0x8000-0x97FF
    if (command_data >= BROADCAST_TIME_SET_TIME_BASE && command_data <= 0x97FF) {

        return BROADCAST_TIME_EVENT_SET_TIME;

    }

    // Report Rate: 0x4000-0x4FFF
    if (command_data >= BROADCAST_TIME_REPORT_RATE_BASE && command_data <= 0x4FFF) {

        return BROADCAST_TIME_EVENT_REPORT_RATE;

    }

    // Report Year: 0x3000-0x3FFF
    if (command_data >= BROADCAST_TIME_REPORT_YEAR_BASE && command_data <= 0x3FFF) {

        return BROADCAST_TIME_EVENT_REPORT_YEAR;

    }

    // Report Date: 0x2100-0x2CFF
    if (command_data >= BROADCAST_TIME_REPORT_DATE_BASE && command_data <= 0x2CFF) {

        return BROADCAST_TIME_EVENT_REPORT_DATE;

    }

    // Report Time: 0x0000-0x17FF
    if (command_data <= 0x17FF) {

        return BROADCAST_TIME_EVENT_REPORT_TIME;

    }

    return BROADCAST_TIME_EVENT_UNKNOWN;

}

    /** @brief Extracts hour and minute from a broadcast time event ID. Returns false if out of range. */
bool ProtocolBroadcastTime_extract_time(event_id_t event_id, uint8_t *hour, uint8_t *minute) {

    uint16_t command_data;
    uint8_t h;
    uint8_t m;

    if (!hour || !minute) {

        return false;

    }

    command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

    // Strip the set command offset if present
    if (command_data >= BROADCAST_TIME_SET_COMMAND_OFFSET) {

        command_data = command_data - BROADCAST_TIME_SET_COMMAND_OFFSET;

    }

    h = (uint8_t)(command_data >> 8);
    m = (uint8_t)(command_data & 0xFF);

    if (h >= 24 || m >= 60) {

        return false;

    }

    *hour = h;
    *minute = m;

    return true;

}

    /** @brief Extracts month and day from a broadcast time event ID. Returns false if out of range. */
bool ProtocolBroadcastTime_extract_date(event_id_t event_id, uint8_t *month, uint8_t *day) {

    uint16_t command_data;
    uint8_t mon;
    uint8_t d;

    if (!month || !day) {

        return false;

    }

    command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

    // Strip the set command offset if present
    if (command_data >= BROADCAST_TIME_SET_COMMAND_OFFSET) {

        command_data = command_data - BROADCAST_TIME_SET_COMMAND_OFFSET;

    }

    // Date format: byte 6 = 0x20 + month, byte 7 = day
    // So command_data upper byte = 0x20 + month
    mon = (uint8_t)((command_data >> 8) - 0x20);
    d = (uint8_t)(command_data & 0xFF);

    if (mon < 1 || mon > 12 || d < 1 || d > 31) {

        return false;

    }

    *month = mon;
    *day = d;

    return true;

}

    /** @brief Extracts year from a broadcast time event ID. Returns false if out of range. */
bool ProtocolBroadcastTime_extract_year(event_id_t event_id, uint16_t *year) {

    uint16_t command_data;
    uint16_t y;

    if (!year) {

        return false;

    }

    command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

    // Strip the set command offset if present
    if (command_data >= BROADCAST_TIME_SET_COMMAND_OFFSET) {

        command_data = command_data - BROADCAST_TIME_SET_COMMAND_OFFSET;

    }

    // Year format: 0x3000 + year (0-4095)
    y = command_data - BROADCAST_TIME_REPORT_YEAR_BASE;

    if (y > 4095) {

        return false;

    }

    *year = y;

    return true;

}

    /**
     * @brief Extracts the 12-bit signed fixed-point rate from a broadcast time event ID.
     *
     * @details Rate format is 10.2 fixed point. Sign-extends bit 11 for negative rates.
     */
bool ProtocolBroadcastTime_extract_rate(event_id_t event_id, int16_t *rate) {

    uint16_t command_data;
    uint16_t raw_rate;

    if (!rate) {

        return false;

    }

    command_data = (uint16_t)(event_id & BROADCAST_TIME_MASK_COMMAND_DATA);

    // Strip the set command offset if present
    if (command_data >= BROADCAST_TIME_SET_COMMAND_OFFSET) {

        command_data = command_data - BROADCAST_TIME_SET_COMMAND_OFFSET;

    }

    // Rate format: 0x4000 + 12-bit signed fixed point
    raw_rate = command_data - BROADCAST_TIME_REPORT_RATE_BASE;

    // 12-bit signed: sign extend if bit 11 is set
    if (raw_rate & 0x0800) {

        *rate = (int16_t)(raw_rate | 0xF000);

    } else {

        *rate = (int16_t)raw_rate;

    }

    return true;

}

    /** @brief Creates a Report/Set Time event ID from clock_id, hour, minute. */
event_id_t ProtocolBroadcastTime_create_time_event_id(uint64_t clock_id, uint8_t hour, uint8_t minute, bool is_set) {

    uint16_t command_data;

    command_data = ((uint16_t)hour << 8) | (uint16_t)minute;

    if (is_set) {

        command_data = command_data + BROADCAST_TIME_SET_COMMAND_OFFSET;

    }

    return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;

}

    /** @brief Creates a Report/Set Date event ID from clock_id, month, day. */
event_id_t ProtocolBroadcastTime_create_date_event_id(uint64_t clock_id, uint8_t month, uint8_t day, bool is_set) {

    uint16_t command_data;

    command_data = ((uint16_t)(0x20 + month) << 8) | (uint16_t)day;

    if (is_set) {

        command_data = command_data + BROADCAST_TIME_SET_COMMAND_OFFSET;

    }

    return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;

}

    /** @brief Creates a Report/Set Year event ID from clock_id, year. */
event_id_t ProtocolBroadcastTime_create_year_event_id(uint64_t clock_id, uint16_t year, bool is_set) {

    uint16_t command_data;

    command_data = BROADCAST_TIME_REPORT_YEAR_BASE + year;

    if (is_set) {

        command_data = command_data + BROADCAST_TIME_SET_COMMAND_OFFSET;

    }

    return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;

}

    /** @brief Creates a Report/Set Rate event ID from clock_id, rate. */
event_id_t ProtocolBroadcastTime_create_rate_event_id(uint64_t clock_id, int16_t rate, bool is_set) {

    uint16_t command_data;

    command_data = BROADCAST_TIME_REPORT_RATE_BASE + ((uint16_t)rate & 0x0FFF);

    if (is_set) {

        command_data = command_data + BROADCAST_TIME_SET_COMMAND_OFFSET;

    }

    return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;

}

    /** @brief Creates a command event ID (Query, Start, Stop, Date Rollover) for the given clock. */
event_id_t ProtocolBroadcastTime_create_command_event_id(uint64_t clock_id, broadcast_time_event_type_enum command) {

    uint16_t command_data;

    switch (command) {

        case BROADCAST_TIME_EVENT_QUERY:

            command_data = BROADCAST_TIME_QUERY;
            break;

        case BROADCAST_TIME_EVENT_STOP:

            command_data = BROADCAST_TIME_STOP;
            break;

        case BROADCAST_TIME_EVENT_START:

            command_data = BROADCAST_TIME_START;
            break;

        case BROADCAST_TIME_EVENT_DATE_ROLLOVER:

            command_data = BROADCAST_TIME_DATE_ROLLOVER;
            break;

        default:

            command_data = 0;
            break;

    }

    return (clock_id & BROADCAST_TIME_MASK_CLOCK_ID) | (uint64_t)command_data;

}

#endif /* OPENLCB_COMPILE_BROADCAST_TIME */
