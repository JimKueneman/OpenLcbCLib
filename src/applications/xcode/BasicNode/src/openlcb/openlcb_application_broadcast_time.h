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
 * @brief Application-level Broadcast Time Protocol module.
 *
 * @details Provides a fixed-size array of clock slots and the API for the
 * OpenLCB Broadcast Time Protocol.  Supports up to
 * BROADCAST_TIME_TOTAL_CLOCK_COUNT simultaneous clocks (four well-known
 * plus BROADCAST_TIME_MAX_CUSTOM_CLOCKS user-defined clocks).
 *
 * The protocol handler (protocol_broadcast_time_handler.c) updates clock
 * state when time events are received from the network.  This module is
 * optional — applications that do not use broadcast time should not include it.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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
     * @brief Application-provided callbacks for broadcast time events.
     *
     * @details Any callback that is not needed may be left NULL.  The module
     * checks each pointer before calling it.
     *
     * @see OpenLcbApplicationBroadcastTime_initialize
     */
    typedef struct {

        /** @brief Called each time the fast clock advances by one minute. */
        void (*on_time_changed)(broadcast_clock_t *clock);

        /** @brief Called when a Report Time event is received from the network. */
        void (*on_time_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when a Report Date event is received from the network. */
        void (*on_date_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when a Report Year event is received from the network. */
        void (*on_year_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Called when the clock rolls over from 23:59 to 00:00. */
        void (*on_date_rollover)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

    } interface_openlcb_application_broadcast_time_t;

#ifdef __cplusplus
extern "C" {
#endif

        /**
         * @brief Initialises the broadcast time module and stores the callback interface.
         *
         * @details Clears all clock slots to zero and stores the interface pointer.
         * Must be called once before any other function in this module.
         *
         * @param interface  Pointer to a @ref interface_openlcb_application_broadcast_time_t
         *                   with the desired callbacks (NULL callbacks are safe).
         *
         * @warning Must be called before any setup or send functions.
         */
    extern void OpenLcbApplicationBroadcastTime_initialize(const interface_openlcb_application_broadcast_time_t *interface);

        /**
         * @brief Allocates a clock slot as a consumer and registers event ranges on the node.
         *
         * @details Finds or allocates a clock slot for clock_id and marks it as a consumer.
         * Registers the full 32 768-event consumer and producer ranges on the node so the
         * node can receive Report events and send the Query event.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t that will consume this clock.
         *                      May be NULL to allocate the slot without registering ranges.
         * @param clock_id      64-bit @ref event_id_t that identifies the clock.
         *
         * @return Pointer to the clock's @ref broadcast_clock_state_t, or NULL if the
         *         clock array is full.
         *
         * @warning Returns NULL if no free clock slots are available.
         */
    extern broadcast_clock_state_t* OpenLcbApplicationBroadcastTime_setup_consumer(openlcb_node_t *openlcb_node, event_id_t clock_id);

        /**
         * @brief Allocates a clock slot as a producer and registers event ranges on the node.
         *
         * @details Finds or allocates a clock slot for clock_id and marks it as a producer.
         * Registers the full 32 768-event producer and consumer ranges on the node so the
         * node can send Report events and receive Set/Query commands.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t that will produce this clock.
         *                      May be NULL to allocate the slot without registering ranges.
         * @param clock_id      64-bit @ref event_id_t that identifies the clock.
         *
         * @return Pointer to the clock's @ref broadcast_clock_state_t, or NULL if the
         *         clock array is full.
         *
         * @warning Returns NULL if no free clock slots are available.
         */
    extern broadcast_clock_state_t* OpenLcbApplicationBroadcastTime_setup_producer(openlcb_node_t *openlcb_node, event_id_t clock_id);

        /**
         * @brief Returns the state for a registered clock.
         *
         * @param clock_id  64-bit @ref event_id_t that identifies the clock.
         *
         * @return Pointer to the @ref broadcast_clock_state_t, or NULL if not found.
         */
    extern broadcast_clock_state_t* OpenLcbApplicationBroadcastTime_get_clock(event_id_t clock_id);

        /**
         * @brief Returns whether the given clock is registered as a consumer.
         *
         * @param clock_id  64-bit @ref event_id_t that identifies the clock.
         *
         * @return true if registered as a consumer, false if not found or not a consumer.
         */
    extern bool OpenLcbApplicationBroadcastTime_is_consumer(event_id_t clock_id);

        /**
         * @brief Returns whether the given clock is registered as a producer.
         *
         * @param clock_id  64-bit @ref event_id_t that identifies the clock.
         *
         * @return true if registered as a producer, false if not found or not a producer.
         */
    extern bool OpenLcbApplicationBroadcastTime_is_producer(event_id_t clock_id);

        /**
         * @brief Marks the given clock as running so the 100 ms tick will advance it.
         *
         * @param clock_id  64-bit @ref event_id_t that identifies the clock.
         */
    extern void OpenLcbApplicationBroadcastTime_start(event_id_t clock_id);

        /**
         * @brief Marks the given clock as stopped so the 100 ms tick will not advance it.
         *
         * @param clock_id  64-bit @ref event_id_t that identifies the clock.
         */
    extern void OpenLcbApplicationBroadcastTime_stop(event_id_t clock_id);

        /**
         * @brief Advances all running consumer clocks by one 100 ms step.
         *
         * @details Must be called every 100 ms from the application's timer interrupt or
         * main loop.  Uses fixed-point accumulation to handle fractional rates without
         * floating-point arithmetic.  Fires the on_time_changed callback each time a
         * fast-clock minute boundary is crossed.
         *
         * @note Clocks with rate 0 or that are stopped are skipped.
         * @note Only consumer clocks are advanced; producer clocks manage their own time.
         */
    extern void OpenLcbApplicationBroadcastTime_100ms_time_tick(void);

        /**
         * @brief Sends a Report Time event (Producer Identified Set) for a producer clock.
         *
         * @details Builds a time event ID from the given hour and minute and sends it as
         * a PC Event Report.  No-op if the clock is not found or is not a producer.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         * @param hour          Hour value to report (0–23).
         * @param minute        Minute value to report (0–59).
         *
         * @return true if queued or clock not a producer, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_report_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute);

        /**
         * @brief Sends a Report Date event (Producer Identified Set) for a producer clock.
         *
         * @details Builds a date event ID from the given month and day and sends it as
         * a PC Event Report.  No-op if the clock is not found or is not a producer.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         * @param month         Month value to report (1–12).
         * @param day           Day value to report (1–31).
         *
         * @return true if queued or clock not a producer, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_report_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day);

        /**
         * @brief Sends a Report Year event (Producer Identified Set) for a producer clock.
         *
         * @details Builds a year event ID and sends it as a PC Event Report.  No-op
         * if the clock is not found or is not a producer.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         * @param year          Year value to report.
         *
         * @return true if queued or clock not a producer, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_report_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year);

        /**
         * @brief Sends a Report Rate event (Producer Identified Set) for a producer clock.
         *
         * @details Builds a rate event ID from the 12-bit fixed-point rate and sends it
         * as a PC Event Report.  No-op if the clock is not found or is not a producer.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         * @param rate          12-bit signed fixed-point rate (4 = 1.0x real-time).
         *
         * @return true if queued or clock not a producer, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_report_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate);

        /**
         * @brief Sends a Start event for a producer clock.
         *
         * @details Tells all consumers on the network that this clock is now running.
         * No-op if the clock is not found or is not a producer.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         *
         * @return true if queued or clock not a producer, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_start(openlcb_node_t *openlcb_node, event_id_t clock_id);

        /**
         * @brief Sends a Stop event for a producer clock.
         *
         * @details Tells all consumers on the network that this clock has stopped.
         * No-op if the clock is not found or is not a producer.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         *
         * @return true if queued or clock not a producer, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_stop(openlcb_node_t *openlcb_node, event_id_t clock_id);

        /**
         * @brief Sends a Date Rollover event for a producer clock.
         *
         * @details Notifies consumers that the clock has crossed midnight and the date
         * has changed.  No-op if the clock is not found or is not a producer.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         *
         * @return true if queued or clock not a producer, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_date_rollover(openlcb_node_t *openlcb_node, event_id_t clock_id);

        /**
         * @brief Sends the full query reply sequence for a producer clock.
         *
         * @details The reply sequence (per the OpenLCB Broadcast Time Standard) consists
         * of six messages: Start/Stop, Rate, Year, Date, current Time (all as
         * Producer Identified Set), and the next-minute Time as a PC Event Report.
         * Because the transmit buffer may fill during the sequence, this function uses a
         * static state variable and must be called repeatedly until it returns true.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         * @param next_hour     Hour of the next scheduled time event (0–23).
         * @param next_minute   Minute of the next scheduled time event (0–59).
         *
         * @return true when all six messages have been queued, false if more calls are needed.
         *
         * @warning Uses a static state variable — only one query reply may be in progress at
         *          a time across the entire application.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_query_reply(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t next_hour, uint8_t next_minute);

        /**
         * @brief Sends a Query event for a consumer clock.
         *
         * @details Asks the clock generator to respond with its full current state
         * (start/stop, rate, year, date, time).  The generator replies with a
         * six-message sequence.  No-op if the clock is not found or is not a consumer.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the clock.
         *
         * @return true if queued or clock not a consumer, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_query(openlcb_node_t *openlcb_node, event_id_t clock_id);

        /**
         * @brief Sends a Set Time command to a clock generator.
         *
         * @details Asks the clock generator to change its current time to the given
         * hour and minute.  The generator will broadcast the updated time to all consumers.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the target clock.
         * @param hour          Desired hour (0–23).
         * @param minute        Desired minute (0–59).
         *
         * @return true if queued, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_set_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute);

        /**
         * @brief Sends a Set Date command to a clock generator.
         *
         * @details Asks the clock generator to change its current date to the given
         * month and day.  The generator will broadcast the updated date to all consumers.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the target clock.
         * @param month         Desired month (1–12).
         * @param day           Desired day (1–31).
         *
         * @return true if queued, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_set_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day);

        /**
         * @brief Sends a Set Year command to a clock generator.
         *
         * @details Asks the clock generator to change its current year.  The generator
         * will broadcast the updated year to all consumers.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the target clock.
         * @param year          Desired year.
         *
         * @return true if queued, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_set_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year);

        /**
         * @brief Sends a Set Rate command to a clock generator.
         *
         * @details Asks the clock generator to change its fast-clock rate.  The rate is
         * a 12-bit signed fixed-point value where 4 means 1.0x real-time and negative
         * values run the clock backward.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the target clock.
         * @param rate          12-bit signed fixed-point rate (4 = 1.0x real-time).
         *
         * @return true if queued, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_set_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate);

        /**
         * @brief Sends a Start command to a clock generator.
         *
         * @details Asks the clock generator to start running.  The generator will
         * broadcast a Start event to all consumers when it complies.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the target clock.
         *
         * @return true if queued, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_command_start(openlcb_node_t *openlcb_node, event_id_t clock_id);

        /**
         * @brief Sends a Stop command to a clock generator.
         *
         * @details Asks the clock generator to stop running.  The generator will
         * broadcast a Stop event to all consumers when it complies.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param clock_id      64-bit @ref event_id_t identifying the target clock.
         *
         * @return true if queued, false if the transmit buffer is full.
         */
    extern bool OpenLcbApplicationBroadcastTime_send_command_stop(openlcb_node_t *openlcb_node, event_id_t clock_id);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_APPLICATION_BROADCAST_TIME__ */
