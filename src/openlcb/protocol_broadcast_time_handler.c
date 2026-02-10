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
 * @brief Implementation of Broadcast Time Protocol handler
 *
 * @details This file implements the protocol handler for OpenLCB Broadcast Time
 * messages. It decodes time information from Event IDs and updates node clock state.
 *
 * **Architecture:**
 * - Static interface pointer for application callbacks (dependency injection)
 * - Internal helper functions for each event type
 * - Main dispatch function called from Event Transport handler
 *
 * **Event ID Decoding:**
 * All time data is encoded in the 64-bit Event ID:
 * - Bits 63-16: Clock ID (which clock this event belongs to)
 * - Bits 15-0: Time/Date/Year/Rate/Command encoded data
 *
 * **Processing Flow:**
 * 1. Check if node is clock consumer
 * 2. Extract and update clock ID in node state
 * 3. Determine event type from Event ID
 * 4. Call appropriate internal handler
 * 5. Handler decodes data and updates node->clock_state
 * 6. Handler invokes application callback if registered
 *
 * @author Jim Kueneman
 * @date 09 Feb 2026
 *
 * @see protocol_broadcast_time_handler.h - Public interface
 * @see openlcb_utilities.h - Event ID encoding/decoding
 */

#include "protocol_broadcast_time_handler.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"


// Static interface pointer for application callbacks
static const interface_openlcb_protocol_broadcast_time_t *_interface_broadcast_time;


void ProtocolBroadcastTime_initialize(
    const interface_openlcb_protocol_broadcast_time_t *interface_broadcast_time
) {
    
    _interface_broadcast_time = interface_broadcast_time;
    
}


// Internal helper functions for each event type


static void _handle_report_time(
    openlcb_node_t *node,
    event_id_t event_id
) {
    uint8_t hour;
    uint8_t minute;

    if (OpenLcbUtilities_extract_time_from_event_id(event_id, &hour, &minute)) {
        
        node->clock_state.time.hour = hour;
        node->clock_state.time.minute = minute;
        node->clock_state.time_valid = 1;
        
        if (_interface_broadcast_time && _interface_broadcast_time->on_time_received) {
            
            _interface_broadcast_time->on_time_received(node, &node->clock_state);
            
        }
        
    }
    
}


static void _handle_report_date(
    openlcb_node_t *node,
    event_id_t event_id
) {
    uint8_t month;
    uint8_t day;

    if (OpenLcbUtilities_extract_date_from_event_id(event_id, &month, &day)) {
        
        node->clock_state.date.month = month;
        node->clock_state.date.day = day;
        node->clock_state.date_valid = 1;
        
        if (_interface_broadcast_time && _interface_broadcast_time->on_date_received) {
            
            _interface_broadcast_time->on_date_received(node, &node->clock_state);
            
        }
        
    }
    
}


static void _handle_report_year(
    openlcb_node_t *node,
    event_id_t event_id
) {
    uint16_t year;

    if (OpenLcbUtilities_extract_year_from_event_id(event_id, &year)) {
        
        node->clock_state.year.year = year;
        node->clock_state.year_valid = 1;
        
        if (_interface_broadcast_time && _interface_broadcast_time->on_year_received) {
            
            _interface_broadcast_time->on_year_received(node, &node->clock_state);
            
        }
        
    }
    
}


static void _handle_report_rate(
    openlcb_node_t *node,
    event_id_t event_id
) {
    int16_t rate;

    if (OpenLcbUtilities_extract_rate_from_event_id(event_id, &rate)) {
        
        node->clock_state.rate.rate = rate;
        node->clock_state.rate_valid = 1;
        
        if (_interface_broadcast_time && _interface_broadcast_time->on_rate_received) {
            
            _interface_broadcast_time->on_rate_received(node, &node->clock_state);
            
        }
        
    }
    
}


static void _handle_start(
    openlcb_node_t *node,
    event_id_t event_id
) {

    node->clock_state.is_running = 1;
    
    if (_interface_broadcast_time && _interface_broadcast_time->on_clock_started) {
        
        _interface_broadcast_time->on_clock_started(node, &node->clock_state);
        
    }
    
}


static void _handle_stop(
    openlcb_node_t *node,
    event_id_t event_id
) {

    node->clock_state.is_running = 0;
    
    if (_interface_broadcast_time && _interface_broadcast_time->on_clock_stopped) {
        
        _interface_broadcast_time->on_clock_stopped(node, &node->clock_state);
        
    }
    
}


static void _handle_date_rollover(
    openlcb_node_t *node,
    event_id_t event_id
) {

    if (_interface_broadcast_time && _interface_broadcast_time->on_date_rollover) {
        
        _interface_broadcast_time->on_date_rollover(node, &node->clock_state);
        
    }
    
}


// Main handler


void ProtocolBroadcastTime_handle_time_event(
    openlcb_statemachine_info_t *statemachine_info,
    event_id_t event_id
) {
    broadcast_time_event_type_enum event_type;
    openlcb_node_t *node;
    
    if (!statemachine_info) {
        
        return;
        
    }
    
    node = statemachine_info->openlcb_node;
    
    if (!node) {
        
        return;
        
    }
    
    if (node->is_clock_consumer == 0) {
        
        return;
        
    }
    
    node->clock_state.clock_id = 
        OpenLcbUtilities_extract_clock_id_from_time_event(event_id);
    
    event_type = OpenLcbUtilities_get_broadcast_time_event_type(event_id);
    
    switch (event_type) {
        
        case BROADCAST_TIME_EVENT_REPORT_TIME:
            _handle_report_time(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_REPORT_DATE:
            _handle_report_date(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_REPORT_YEAR:
            _handle_report_year(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_REPORT_RATE:
            _handle_report_rate(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_SET_TIME:
            _handle_report_time(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_SET_DATE:
            _handle_report_date(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_SET_YEAR:
            _handle_report_year(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_SET_RATE:
            _handle_report_rate(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_START:
            _handle_start(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_STOP:
            _handle_stop(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_DATE_ROLLOVER:
            _handle_date_rollover(node, event_id);
            break;
            
        case BROADCAST_TIME_EVENT_QUERY:
            break;
            
        default:
            break;
            
    }
    
}
