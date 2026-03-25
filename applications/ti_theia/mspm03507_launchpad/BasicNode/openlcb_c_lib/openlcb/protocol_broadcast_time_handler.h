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
 * @brief Broadcast Time Protocol message handler
 *
 * @details Handles incoming broadcast time Event IDs from the network.
 * Decodes time/date/year/rate/command data and updates the singleton clock
 * state in openlcb_application_broadcast_time module.
 *
 * Called from the main statemachine when a broadcast time event is detected.
 * Only processes events for node index 0 (broadcast time events are global).
 *
 * @author Jim Kueneman
 * @date 20 Mar 2026
 *
 * @see openlcb_application_broadcast_time.h - Singleton clock state and API
 * @see openlcb_utilities.h - General message utility functions
 */

#ifndef __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__
#define __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @struct interface_openlcb_protocol_broadcast_time_handler_t
     * @brief Application callbacks for broadcast time events.
     *
     * All callbacks are optional (can be NULL).
     */
    typedef struct {

            /** @brief Time-of-day updated.  Optional. */
        void (*on_time_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Date updated.  Optional. */
        void (*on_date_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Year updated.  Optional. */
        void (*on_year_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Clock rate changed.  Optional. */
        void (*on_rate_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Clock started.  Optional. */
        void (*on_clock_started)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Clock stopped.  Optional. */
        void (*on_clock_stopped)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Date rollover occurred.  Optional. */
        void (*on_date_rollover)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

    } interface_openlcb_protocol_broadcast_time_handler_t;

#ifdef __cplusplus
extern "C" {
#endif

        /**
         * @brief Initializes the Broadcast Time Protocol handler.
         *
         * @param interface_openlcb_protocol_broadcast_time_handler  Pointer to @ref interface_openlcb_protocol_broadcast_time_handler_t (must remain valid for application lifetime).
         */
    extern void ProtocolBroadcastTime_initialize(const interface_openlcb_protocol_broadcast_time_handler_t *interface_openlcb_protocol_broadcast_time_handler);

        /**
         * @brief Handles incoming broadcast time events.
         *
         * @details Decodes the Event ID and updates the singleton clock state.
         * Only processes if the node has index == 0 and a matching clock is
         * registered in the application broadcast time module.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         * @param event_id           Full 64-bit @ref event_id_t containing encoded time data.
         */
    extern void ProtocolBroadcastTime_handle_time_event(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id);

    // =========================================================================
    // Broadcast Time Event ID Utilities
    //
    // Moved from openlcb_utilities to this module so the linker only pulls in
    // broadcast-time code when OPENLCB_COMPILE_BROADCAST_TIME is defined.
    // Bootloader and other minimal builds that omit the flag link none of this.
    // =========================================================================

        /**
         * @brief Returns true if the event ID belongs to the broadcast time event space.
         *
         * @param event_id 64-bit @ref event_id_t to test.
         *
         * @return true if the event ID is a broadcast time event.
         */
    extern bool ProtocolBroadcastTime_is_time_event(event_id_t event_id);

        /**
         * @brief Extracts the 48-bit clock ID (upper 6 bytes) from a broadcast time event ID.
         *
         * @param event_id Broadcast time @ref event_id_t to extract from.
         *
         * @return 48-bit clock identifier masked from the event ID.
         */
    extern uint64_t ProtocolBroadcastTime_extract_clock_id(event_id_t event_id);

        /**
         * @brief Returns the @ref broadcast_time_event_type_enum for a broadcast time event ID.
         *
         * @param event_id Broadcast time @ref event_id_t to classify.
         *
         * @return The @ref broadcast_time_event_type_enum indicating the event type.
         */
    extern broadcast_time_event_type_enum ProtocolBroadcastTime_get_event_type(event_id_t event_id);

        /**
         * @brief Extracts hour and minute from a broadcast time event ID.
         *
         * @param event_id Broadcast time event ID
         * @param hour Receives hour (0-23)
         * @param minute Receives minute (0-59)
         *
         * @return true if values are valid, false if out of range.
         */
    extern bool ProtocolBroadcastTime_extract_time(event_id_t event_id, uint8_t *hour, uint8_t *minute);

        /**
         * @brief Extracts month and day from a broadcast time event ID.
         *
         * @param event_id Broadcast time event ID
         * @param month Receives month (1-12)
         * @param day Receives day (1-31)
         *
         * @return true if values are valid, false if out of range.
         */
    extern bool ProtocolBroadcastTime_extract_date(event_id_t event_id, uint8_t *month, uint8_t *day);

        /**
         * @brief Extracts year from a broadcast time event ID.
         *
         * @param event_id Broadcast time event ID
         * @param year Receives year (0-4095)
         *
         * @return true if extraction successful.
         */
    extern bool ProtocolBroadcastTime_extract_year(event_id_t event_id, uint16_t *year);

        /**
         * @brief Extracts the 12-bit signed fixed-point rate from a broadcast time event ID.
         *
         * @param event_id Broadcast time event ID
         * @param rate Receives rate (10.2 fixed point, e.g. 0x0004 = 1.00)
         *
         * @return true if extraction successful.
         */
    extern bool ProtocolBroadcastTime_extract_rate(event_id_t event_id, int16_t *rate);

        /**
         * @brief Creates a Report/Set Time event ID from clock_id, hour, minute.
         *
         * @param clock_id 48-bit clock identifier.
         * @param hour     Hour value (0-23).
         * @param minute   Minute value (0-59).
         * @param is_set   true for a Set command, false for a Report.
         *
         * @return Encoded @ref event_id_t for the time event.
         */
    extern event_id_t ProtocolBroadcastTime_create_time_event_id(uint64_t clock_id, uint8_t hour, uint8_t minute, bool is_set);

        /**
         * @brief Creates a Report/Set Date event ID from clock_id, month, day.
         *
         * @param clock_id 48-bit clock identifier.
         * @param month    Month value (1-12).
         * @param day      Day value (1-31).
         * @param is_set   true for a Set command, false for a Report.
         *
         * @return Encoded @ref event_id_t for the date event.
         */
    extern event_id_t ProtocolBroadcastTime_create_date_event_id(uint64_t clock_id, uint8_t month, uint8_t day, bool is_set);

        /**
         * @brief Creates a Report/Set Year event ID from clock_id, year.
         *
         * @param clock_id 48-bit clock identifier.
         * @param year     Year value (0-4095).
         * @param is_set   true for a Set command, false for a Report.
         *
         * @return Encoded @ref event_id_t for the year event.
         */
    extern event_id_t ProtocolBroadcastTime_create_year_event_id(uint64_t clock_id, uint16_t year, bool is_set);

        /**
         * @brief Creates a Report/Set Rate event ID from clock_id, rate.
         *
         * @param clock_id 48-bit clock identifier.
         * @param rate     12-bit signed fixed-point rate (10.2 format).
         * @param is_set   true for a Set command, false for a Report.
         *
         * @return Encoded @ref event_id_t for the rate event.
         */
    extern event_id_t ProtocolBroadcastTime_create_rate_event_id(uint64_t clock_id, int16_t rate, bool is_set);

        /**
         * @brief Creates a command event ID (Query, Start, Stop, Date Rollover) for the given clock.
         *
         * @param clock_id 48-bit clock identifier.
         * @param command  @ref broadcast_time_event_type_enum specifying the command type.
         *
         * @return Encoded @ref event_id_t for the command event.
         */
    extern event_id_t ProtocolBroadcastTime_create_command_event_id(uint64_t clock_id, broadcast_time_event_type_enum command);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__ */
