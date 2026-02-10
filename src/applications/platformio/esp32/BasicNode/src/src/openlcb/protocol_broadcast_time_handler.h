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
 * @file protocol_broadcast_time_handler.h
 * @brief Broadcast Time Protocol handler for fast clock synchronization
 *
 * @details This file implements the OpenLCB Broadcast Time Protocol, which
 * allows nodes to synchronize clocks across the network for model railroad
 * operations. Time information is encoded directly in Event IDs, allowing
 * efficient distribution without payload data.
 *
 * **Key Features:**
 * - Fast clock support (accelerated time for operations)
 * - Multiple independent clocks on one network
 * - Time, date, year, and rate synchronization
 * - Clock control (start/stop, rate changes)
 * - Query mechanism for clock synchronization
 *
 * **Protocol Architecture:**
 * - Uses standard MTI_PC_EVENT_REPORT messages
 * - Time encoded in 64-bit Event ID (no payload)
 * - Called from Event Transport protocol handler
 * - Dependency injection for testability
 *
 * **Integration Pattern:**
 * 1. Event transport handler receives PC Event Report
 * 2. Checks if Event ID matches broadcast time format
 * 3. Calls ProtocolBroadcastTime_handle_time_event()
 * 4. Protocol decodes Event ID into time/date/etc
 * 5. Updates node clock_state
 * 6. Invokes application callbacks with clean data
 *
 * @author Jim Kueneman
 * @date 09 Feb 2026
 *
 * @see protocol_event_transport.h - Calls this handler
 * @see openlcb_utilities.h - Event ID encoding/decoding functions
 * @see OpenLCB Broadcast Time Protocol Standard
 */

#ifndef __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__
#define __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
    * @struct interface_openlcb_protocol_broadcast_time_t
    * @brief Callback interface for Broadcast Time Protocol
    *
    * @details Application callbacks invoked when broadcast time events are received.
    * All callbacks are optional (can be NULL). Callbacks receive decoded time data
    * and updated clock state.
    *
    * **Typical Usage:**
    * @code
    * void my_time_received(openlcb_node_t *node, broadcast_clock_state_t *clock_state) {
    *     printf("Time: %02d:%02d\n", clock_state->time.hour, clock_state->time.minute);
    *     update_display(clock_state);
    * }
    *
    * interface_openlcb_protocol_broadcast_time_t callbacks = {
    *     .on_time_received = my_time_received,
    *     .on_date_received = NULL,  // Not interested in date
    *     // ... other callbacks
    * };
    *
    * ProtocolBroadcastTime_initialize(&callbacks);
    * @endcode
    *
    * @see ProtocolBroadcastTime_initialize
    * @see ProtocolBroadcastTime_handle_time_event
    */
typedef struct {

        /** @brief Called when Report Time or Set Time event received */
        void (*on_time_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when Report Date or Set Date event received */
        void (*on_date_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when Report Year or Set Year event received */
        void (*on_year_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when Report Rate or Set Rate event received */
        void (*on_rate_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when Start event received (clock started) */
        void (*on_clock_started)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when Stop event received (clock stopped) */
        void (*on_clock_stopped)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when Date Rollover event received (midnight crossing) */
        void (*on_date_rollover)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

} interface_openlcb_protocol_broadcast_time_t;

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Initializes the Broadcast Time Protocol handler
     * 
     * @details Stores the interface function pointers for use by the protocol handler.
     * Must be called once during application startup before any time-related operations.
     * The interface structure contains optional callback functions that are invoked when
     * time events are received.
     *
     * Typical initialization sequence:
     * -# Populate interface_openlcb_protocol_broadcast_time_t structure with callbacks
     * -# Call ProtocolBroadcastTime_initialize() to register callbacks
     * -# Call OpenLcbApplication_subscribe_to_time_broadcasts() to enable reception
     *
     * All callbacks in the interface are optional (can be NULL). Only register callbacks
     * for events you want to be notified about.
     *
     * Use cases:
     * - Application startup initialization
     * - Registering clock consumer callbacks
     * - Enabling time synchronization
     * 
     * @param interface_broadcast_time Pointer to interface structure with callbacks
     * 
     * @warning Must be called exactly once during initialization
     * @warning The interface pointer is stored and must remain valid for program lifetime
     * @warning Can pass NULL for interface to disable all callbacks
     * 
     * @attention Call before subscribing to time broadcasts
     * @attention All callbacks are optional - NULL checks performed before invocation
     * 
     * @see interface_openlcb_protocol_broadcast_time_t - Callback interface definition
     * @see OpenLcbApplication_subscribe_to_time_broadcasts - Enable time reception
     */
    extern void ProtocolBroadcastTime_initialize(
        const interface_openlcb_protocol_broadcast_time_t *interface_broadcast_time
    );

    /**
     * @brief Handles incoming broadcast time events
     * 
     * @details Processes PC Event Report messages containing broadcast time Event IDs.
     * This function is called by the Event Transport protocol handler when it detects
     * a broadcast time Event ID.
     *
     * Processing flow:
     * -# Verify node is a clock consumer
     * -# Extract clock ID from Event ID (upper 6 bytes)
     * -# Determine event type (time/date/year/rate/command)
     * -# Decode event-specific data from Event ID
     * -# Update node->clock_state with decoded values
     * -# Invoke appropriate application callback if registered
     *
     * Supported event types:
     * - Report Time: Extracts hour and minute, updates clock_state.time
     * - Report Date: Extracts month and day, updates clock_state.date
     * - Report Year: Extracts year, updates clock_state.year
     * - Report Rate: Extracts clock rate, updates clock_state.rate
     * - Set Time/Date/Year/Rate: Processed same as Report for consumers
     * - Start: Sets clock_state.is_running = 1
     * - Stop: Sets clock_state.is_running = 0
     * - Date Rollover: Notification only (no state update)
     * - Query: Ignored by consumers (producers respond)
     *
     * This function should be called from the Event Transport handler, NOT directly
     * from the main state machine. The dependency injection pattern requires:
     * 
     * @verbatim
     * // In protocol_event_transport.c
     * void ProtocolEventTransport_handle_pc_event_report(openlcb_statemachine_info_t *statemachine_info) {
     *     event_id_t event_id;
     *     
     *     event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(
     *         statemachine_info->incoming_msg_info.msg_ptr);
     *     
     *     if (_interface->broadcast_time_event_handler) {
     *         if (OpenLcbUtilities_is_broadcast_time_event(event_id)) {
     *             _interface->broadcast_time_event_handler(statemachine_info, event_id);
     *             return;
     *         }
     *     }
     *     
     *     // ... existing event handling ...
     * }
     * @endverbatim
     * 
     * @param statemachine_info State machine context with incoming message and node
     * @param event_id Full 64-bit Event ID containing encoded time data
     * 
     * @warning statemachine_info must not be NULL
     * @warning Assumes event_id has already been validated as broadcast time event
     * 
     * @note Only processes messages if node->is_clock_consumer == 1
     * @note Updates node->clock_state with received time data
     * @note Callbacks are optional - NULL checks performed before invocation
     * 
     * @see OpenLcbUtilities_is_broadcast_time_event - Event ID validation
     * @see OpenLcbUtilities_get_broadcast_time_event_type - Event type detection
     * @see interface_openlcb_protocol_broadcast_time_t - Callback definitions
     */
    extern void ProtocolBroadcastTime_handle_time_event(
        openlcb_statemachine_info_t *statemachine_info,
        event_id_t event_id
    );

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__ */
