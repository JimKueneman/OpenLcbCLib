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
 * @file openlcb_application_broadcast_time.c
 * @brief Implementation of the application-level Broadcast Time Protocol module.
 *
 * @details Manages the fixed-size clock array, the fixed-point time accumulator,
 * and all send functions for the OpenLCB Broadcast Time Protocol.
 *
 * @author Jim Kueneman
 * @date 4 Mar 2026
 */

#include "openlcb_application_broadcast_time.h"

#ifdef OPENLCB_COMPILE_BROADCAST_TIME

#include <stddef.h>
#include <string.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_application.h"
#include "protocol_broadcast_time_handler.h"


/** @brief Fixed-size array of clock slots. */
static broadcast_clock_t _clocks[BROADCAST_TIME_TOTAL_CLOCK_COUNT];

/** @brief Stored interface pointer for optional application callbacks. */
static const interface_openlcb_application_broadcast_time_t *_interface;

    /** @brief Tracks the last tick value to gate broadcast time processing. */
static uint8_t _last_bcast_tick = 0;

    /** @brief Forward declaration: drives the 6-message query reply state machine. */
static bool _send_query_reply_for_clock(broadcast_clock_t *clock, openlcb_node_t *openlcb_node, uint8_t next_hour, uint8_t next_minute);


    /**
     * @brief Searches the clock array for a slot matching clock_id.
     *
     * @details Algorithm:
     * -# Iterate through all clock slots.
     * -# Return the first slot where is_allocated is true and state.clock_id matches.
     * -# Return NULL if no match found.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t that identifies the clock.
     * @endverbatim
     *
     * @return Pointer to the matching broadcast_clock_t, or NULL if not found.
     */
static broadcast_clock_t *_find_clock_by_id(event_id_t clock_id) {

    broadcast_clock_t *first_match = NULL;

    for (int i = 0; i < BROADCAST_TIME_TOTAL_CLOCK_COUNT; i++) {

        if (_clocks[i].is_allocated && _clocks[i].state.clock_id == clock_id) {

            if (_clocks[i].is_producer && _clocks[i].producer_node) {

                return &_clocks[i];

            }

            if (!first_match) {

                first_match = &_clocks[i];

            }

        }

    }

    return first_match;

}

    /**
     * @brief Returns an existing clock slot for clock_id, or allocates a new one.
     *
     * @details Algorithm:
     * -# Call _find_clock_by_id(); if found, return immediately.
     * -# Otherwise scan for the first slot where is_allocated is false.
     * -# If found, zero the slot, set state.clock_id and is_allocated, then return it.
     * -# If no free slot, return NULL.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t that identifies the clock.
     * @endverbatim
     *
     * @return Pointer to the clock slot, or NULL if the array is full.
     */
static broadcast_clock_t *_find_or_allocate_clock(event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        return clock;

    }

    for (int i = 0; i < BROADCAST_TIME_TOTAL_CLOCK_COUNT; i++) {

        if (!_clocks[i].is_allocated) {

            memset(&_clocks[i], 0, sizeof(broadcast_clock_t));
            _clocks[i].state.clock_id = clock_id;
            _clocks[i].is_allocated = 1;

            return &_clocks[i];

        }

    }

    return NULL;

}


    /**
     * @brief Initialises the broadcast time module and stores the callback interface.
     *
     * @details Algorithm:
     * -# Zero all clock slots with memset.
     * -# Store the interface pointer in the static _interface variable.
     *
     * @verbatim
     * @param interface  Pointer to a interface_openlcb_application_broadcast_time_t
     *                   with the desired callbacks (NULL callbacks are safe).
     * @endverbatim
     *
     * @warning Must be called before any other function in this module.
     */
void OpenLcbApplicationBroadcastTime_initialize(const interface_openlcb_application_broadcast_time_t *interface) {

    memset(_clocks, 0, sizeof(_clocks));
    _interface = interface;
    _last_bcast_tick = 0;

}


    /**
     * @brief Allocates a clock slot as a consumer and registers event ranges on the node.
     *
     * @details Algorithm:
     * -# Call _find_or_allocate_clock() for clock_id.
     * -# If NULL, return NULL.
     * -# Set clock->is_consumer = 1.
     * -# If openlcb_node is non-NULL, register consumer and producer ranges
     *    for both halves of the clock's 65536-event range.
     * -# Return pointer to the clock state.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t; may be NULL to skip range registration.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return Pointer to the broadcast_clock_state_t, or NULL if no free slots.
     */
broadcast_clock_state_t *OpenLcbApplicationBroadcastTime_setup_consumer(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    broadcast_clock_t *clock = _find_or_allocate_clock(clock_id);

    if (!clock) {

        return NULL;

    }

    clock->is_consumer = 1;

    if (openlcb_node) {

        // Consumer ranges for receiving Report Time/Date/Year/Rate events
        OpenLcbApplication_register_consumer_range(openlcb_node, clock_id | 0x0000, EVENT_RANGE_COUNT_32768);
        OpenLcbApplication_register_consumer_range(openlcb_node, clock_id | 0x8000, EVENT_RANGE_COUNT_32768);

        // Producer ranges required by Event Transport Standard section 6:
        // a node must be in Advertised state before sending PCERs (e.g. Query event)
        OpenLcbApplication_register_producer_range(openlcb_node, clock_id | 0x0000, EVENT_RANGE_COUNT_32768);
        OpenLcbApplication_register_producer_range(openlcb_node, clock_id | 0x8000, EVENT_RANGE_COUNT_32768);

    }

    return &clock->state;

}

    /**
     * @brief Allocates a clock slot as a producer and registers event ranges on the node.
     *
     * @details Algorithm:
     * -# Call _find_or_allocate_clock() for clock_id.
     * -# If NULL, return NULL.
     * -# Set clock->is_producer = 1.
     * -# If openlcb_node is non-NULL, register producer and consumer ranges
     *    for both halves of the clock's 65536-event range.
     * -# Return pointer to the clock state.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t; may be NULL to skip range registration.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return Pointer to the broadcast_clock_state_t, or NULL if no free slots.
     */
broadcast_clock_state_t *OpenLcbApplicationBroadcastTime_setup_producer(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    broadcast_clock_t *clock = _find_or_allocate_clock(clock_id);

    if (!clock) {

        return NULL;

    }

    clock->is_producer = 1;
    clock->producer_node = openlcb_node;

    if (openlcb_node) {

        // Producer ranges for sending Report Time/Date/Year/Rate events
        OpenLcbApplication_register_producer_range(openlcb_node, clock_id | 0x0000, EVENT_RANGE_COUNT_32768);
        OpenLcbApplication_register_producer_range(openlcb_node, clock_id | 0x8000, EVENT_RANGE_COUNT_32768);

        // Consumer ranges required by Broadcast Time Standard section 6.1:
        // clock generator must consume Set Time/Date/Year/Rate/Start/Stop/Query events
        OpenLcbApplication_register_consumer_range(openlcb_node, clock_id | 0x0000, EVENT_RANGE_COUNT_32768);
        OpenLcbApplication_register_consumer_range(openlcb_node, clock_id | 0x8000, EVENT_RANGE_COUNT_32768);

    }

    return &clock->state;

}

    /**
     * @brief Marks the given clock as running.
     *
     * @details Algorithm:
     * -# Find the clock slot for clock_id; if not found, return immediately.
     * -# Set state.is_running = true.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t identifying the clock.
     * @endverbatim
     */
void OpenLcbApplicationBroadcastTime_start(event_id_t clock_id) {

    broadcast_clock_t *broadcast_clock = _find_clock_by_id(clock_id);

    if (!broadcast_clock) {

        return;

    }

    broadcast_clock->state.is_running = true;

}

    /**
     * @brief Marks the given clock as stopped.
     *
     * @details Algorithm:
     * -# Find the clock slot for clock_id; if not found, return immediately.
     * -# Set state.is_running = false.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t identifying the clock.
     * @endverbatim
     */
void OpenLcbApplicationBroadcastTime_stop(event_id_t clock_id) {

    broadcast_clock_t *broadcast_clock = _find_clock_by_id(clock_id);

    if (!broadcast_clock) {

        return;

    }

    broadcast_clock->state.is_running = false;

}


    /**
     * @brief Returns the state for a registered clock.
     *
     * @details Algorithm:
     * -# Call _find_clock_by_id(); if found, return &clock->state.
     * -# Otherwise return NULL.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return Pointer to the broadcast_clock_state_t, or NULL if not found.
     */
broadcast_clock_state_t *OpenLcbApplicationBroadcastTime_get_clock(event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        return &clock->state;

    }

    return NULL;

}

    /**
     * @brief Returns whether the given clock is registered as a consumer.
     *
     * @details Algorithm:
     * -# Find the clock slot; if not found, return false.
     * -# Return clock->is_consumer.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return true if registered as a consumer, false otherwise.
     */
bool OpenLcbApplicationBroadcastTime_is_consumer(event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        return clock->is_consumer;

    }

    return 0;

}

    /**
     * @brief Returns whether the given clock is registered as a producer.
     *
     * @details Algorithm:
     * -# Find the clock slot; if not found, return false.
     * -# Return clock->is_producer.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return true if registered as a producer, false otherwise.
     */
bool OpenLcbApplicationBroadcastTime_is_producer(event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        return clock->is_producer;

    }

    return 0;

}


/** @brief Lookup table for days in each month (non-leap year). */
static const uint8_t _days_in_month_table[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    /** @brief Returns true if the given year is a leap year. */
static bool _is_leap_year(uint16_t year) {

    return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);

}

    /** @brief Returns the number of days in a given month, accounting for leap years. */
static uint8_t _days_in_month(uint8_t month, uint16_t year) {

    if (month < 1 || month > 12) {

        return 30;

    }

    uint8_t days = _days_in_month_table[month - 1];

    if (month == 2 && _is_leap_year(year)) {

        days = 29;

    }

    return days;

}

    /**
     * @brief Advances a clock's time forward by one fast minute, rolling over date/month/year as needed.
     *
     * @details Algorithm:
     * -# Increment clock->time.minute.
     * -# On minute overflow (>=60), reset to 0 and increment hour.
     * -# On hour overflow (>=24), reset to 0, fire on_date_rollover, increment day.
     * -# On day overflow, reset to 1, increment month; on month overflow increment year.
     * -# Fire on_year_received, on_date_received, and on_time_received callbacks as appropriate.
     *
     * @verbatim
     * @param clock  Pointer to the broadcast_clock_state_t to advance.
     * @param node   Pointer to the openlcb_node_t passed to callbacks (may be NULL).
     * @endverbatim
     */
static void _advance_minute_forward(broadcast_clock_state_t *clock, openlcb_node_t *node) {

    clock->time.minute++;

    if (clock->time.minute >= 60) {

        clock->time.minute = 0;
        clock->time.hour++;

        if (clock->time.hour >= 24) {

            clock->time.hour = 0;

            if (_interface && _interface->on_date_rollover) {

                _interface->on_date_rollover(node, clock);

            }

            clock->date.day++;

            uint8_t dim = _days_in_month(clock->date.month, clock->year.year);

            if (clock->date.day > dim) {

                clock->date.day = 1;
                clock->date.month++;

                if (clock->date.month > 12) {

                    clock->date.month = 1;
                    clock->year.year++;

                    if (_interface && _interface->on_year_received) {

                        _interface->on_year_received(node, clock);

                    }

                }

                if (_interface && _interface->on_date_received) {

                    _interface->on_date_received(node, clock);

                }

            }

        }

    }

    if (_interface && _interface->on_time_received) {

        _interface->on_time_received(node, clock);

    }

}

    /**
     * @brief Advances a clock's time backward by one fast minute, rolling back date/month/year as needed.
     *
     * @details Algorithm:
     * -# If minute > 0, decrement and fire on_time_received; otherwise set minute to 59.
     * -# If hour > 0, decrement; otherwise set hour to 23, fire on_date_rollover, and decrement day.
     * -# On day underflow, decrement month and set day to the last day of the new month.
     * -# On month underflow, decrement year and set month to 12.
     * -# Fire on_year_received and on_date_received callbacks as appropriate.
     *
     * @verbatim
     * @param clock  Pointer to the broadcast_clock_state_t to reverse.
     * @param node   Pointer to the openlcb_node_t passed to callbacks (may be NULL).
     * @endverbatim
     */
static void _advance_minute_backward(broadcast_clock_state_t *clock, openlcb_node_t *node) {

    if (clock->time.minute == 0) {

        clock->time.minute = 59;

        if (clock->time.hour == 0) {

            clock->time.hour = 23;

            if (_interface && _interface->on_date_rollover) {

                _interface->on_date_rollover(node, clock);

            }

            if (clock->date.day <= 1) {

                if (clock->date.month <= 1) {

                    clock->date.month = 12;
                    clock->year.year--;

                    if (_interface && _interface->on_year_received) {

                        _interface->on_year_received(node, clock);

                    }

                } else {

                    clock->date.month--;

                }

                clock->date.day = _days_in_month(clock->date.month, clock->year.year);

                if (_interface && _interface->on_date_received) {

                    _interface->on_date_received(node, clock);

                }

            } else {

                clock->date.day--;

            }

        } else {

            clock->time.hour--;

        }

    } else {

        clock->time.minute--;

    }

    if (_interface && _interface->on_time_received) {

        _interface->on_time_received(node, clock);

    }

}


/*
 * Accumulator Math for Fixed-Point Rate
 * ======================================
 *
 * The broadcast time rate is a 12-bit signed fixed-point value with 2 fractional
 * bits (format: rrrrrrrrrr.rr). This means the integer rate value is 4x the
 * actual multiplier:
 *
 *   rate = 4  -> 1.00x real-time
 *   rate = 8  -> 2.00x real-time
 *   rate = 16 -> 4.00x real-time
 *   rate = 1  -> 0.25x real-time
 *   rate = -4 -> -1.00x (time runs backward at real-time speed)
 *
 * To avoid floating point, we keep everything in the fixed-point scale:
 *
 *   Each 100ms tick adds:  100 * abs(rate)  to the accumulator
 *   One fast-minute threshold:  4 * 60 * 1000 = 240,000
 *
 * Why 240,000? At rate=4 (1.0x real-time), one real minute is 600 ticks:
 *   600 ticks * 100ms = 60 seconds = 1 real minute
 *   600 * (100 * 4) = 240,000 = threshold  -> 1 fast-minute per real-minute
 *
 * At rate=16 (4.0x):
 *   Each tick adds 100 * 16 = 1,600
 *   240,000 / 1,600 = 150 ticks = 15 seconds real-time per fast-minute
 *
 * At rate=1 (0.25x):
 *   Each tick adds 100 * 1 = 100
 *   240,000 / 100 = 2,400 ticks = 4 real minutes per fast-minute
 *
 * The while loop handles high rates where multiple fast-minutes may elapse
 * in a single 100ms tick (rates above 40.0x, i.e. rate > 160).
 */

#define BROADCAST_TIME_MS_PER_MINUTE_FIXED_POINT 240000  // 4 * 60 * 1000

    /**
     * @brief Advances all running consumer clocks by one 100 ms step.
     *
     * @details Algorithm:
     * -# Compute ticks elapsed since last call via subtraction.
     * -# Skip if no time has elapsed (deduplication).
     * -# For each allocated, running consumer clock with a non-zero rate:
     *    - Compute abs_rate from the signed rate.
     *    - Add 100 * abs_rate * ticks_elapsed to state.ms_accumulator.
     *    - While accumulator >= BROADCAST_TIME_MS_PER_MINUTE_FIXED_POINT (240,000):
     *        - Subtract the threshold from the accumulator.
     *        - Call _advance_minute_forward() or _advance_minute_backward() depending on rate sign.
     *        - Fire the on_time_changed callback.
     * -# For each allocated producer clock with a producer_node:
     *    - Compare previous_run_state with the node's current run_state.
     *    - On transition to RUNSTATE_RUN, auto-set query_reply_pending to trigger
     *      the Standard §6.1 startup sync sequence.
     *    - Update previous_run_state.
     *
     * The threshold 240,000 equals 4 * 60 * 1000, which at rate=4 (1.0x) yields exactly
     * one fast-minute per real minute.  See the accumulator math comment above for details.
     *
     * @verbatim
     * @param current_tick  Current value of the global 100ms tick counter.
     * @endverbatim
     */
void OpenLcbApplicationBroadcastTime_100ms_time_tick(uint8_t current_tick) {

    uint8_t ticks_elapsed = (uint8_t)(current_tick - _last_bcast_tick);

    if (ticks_elapsed == 0) {

        return;

    }

    _last_bcast_tick = current_tick;

    for (int i = 0; i < BROADCAST_TIME_TOTAL_CLOCK_COUNT; i++) {

        broadcast_clock_t *clock = &_clocks[i];

        if (!clock->is_allocated) {

            continue;

        }

        // --- Time advancement (consumer AND producer clocks) ---

        if ((clock->is_consumer || clock->is_producer) && clock->state.is_running) {

            int16_t rate = clock->state.rate.rate;

            if (rate != 0) {

                uint16_t abs_rate = (rate < 0) ? (uint16_t)(-rate) : (uint16_t)(rate);

                clock->state.ms_accumulator += (uint32_t)(100) * abs_rate * ticks_elapsed;

                while (clock->state.ms_accumulator >= BROADCAST_TIME_MS_PER_MINUTE_FIXED_POINT) {

                    clock->state.ms_accumulator -= BROADCAST_TIME_MS_PER_MINUTE_FIXED_POINT;

                    uint8_t prev_hour = clock->state.time.hour;

                    if (rate > 0) {

                        _advance_minute_forward(&clock->state, (openlcb_node_t *)clock->producer_node);

                    } else {

                        _advance_minute_backward(&clock->state, (openlcb_node_t *)clock->producer_node);

                    }

                    if (_interface && _interface->on_time_changed) {

                        _interface->on_time_changed(clock);

                    }

                    // --- Producer: send events on minute boundaries ---

                    if (clock->is_producer && clock->producer_node) {

                        openlcb_node_t *node = (openlcb_node_t *)clock->producer_node;

                        // Periodic Report Time PCER (rate-limited to once per 60 real seconds)
                        if (clock->report_cooldown_ticks == 0) {

                            OpenLcbApplicationBroadcastTime_send_report_time(
                                node, clock->state.clock_id,
                                clock->state.time.hour, clock->state.time.minute);

                            clock->report_cooldown_ticks = 600;  // 60 seconds

                        }

                        // Rollover detection: hour went from 23 to 0
                        if (prev_hour == 23 && clock->state.time.hour == 0) {

                            OpenLcbApplicationBroadcastTime_send_date_rollover(node, clock->state.clock_id);
                            OpenLcbApplicationBroadcastTime_send_report_year(
                                node, clock->state.clock_id, clock->state.year.year);
                            OpenLcbApplicationBroadcastTime_send_report_date(
                                node, clock->state.clock_id, clock->state.date.month, clock->state.date.day);

                        }

                    }

                }

            }

        }

        // --- Producer: decrement report cooldown ---

        if (clock->is_producer && clock->report_cooldown_ticks > 0) {

            if (ticks_elapsed >= clock->report_cooldown_ticks) {

                clock->report_cooldown_ticks = 0;

            } else {

                clock->report_cooldown_ticks -= ticks_elapsed;

            }

        }

        // --- Producer: sync delay countdown ---

        if (clock->is_producer && clock->sync_delay_ticks > 0) {

            if (ticks_elapsed >= clock->sync_delay_ticks) {

                clock->sync_delay_ticks = 0;

                if (clock->sync_pending) {

                    clock->sync_pending = false;
                    clock->query_reply_pending = true;
                    clock->send_query_reply_state = 0;

                }

            } else {

                clock->sync_delay_ticks -= ticks_elapsed;

            }

        }

        // --- Producer: auto-trigger startup sync on login completion (Standard §6.1) ---

        if (clock->is_producer && clock->producer_node) {

            openlcb_node_t *node = (openlcb_node_t *)clock->producer_node;

            if (clock->previous_run_state < RUNSTATE_RUN && node->state.run_state >= RUNSTATE_RUN) {

                clock->query_reply_pending = true;
                clock->send_query_reply_state = 0;

            }

            clock->previous_run_state = node->state.run_state;

        }

        // --- Producer: drive query reply state machine ---

        if (clock->is_producer && clock->query_reply_pending && clock->producer_node) {

            uint8_t next_hour = clock->state.time.hour;
            uint8_t next_min = clock->state.time.minute + 1;

            if (next_min >= 60) {

                next_min = 0;
                next_hour = (next_hour + 1) % 24;

            }

            if (_send_query_reply_for_clock(clock,
                    (openlcb_node_t *)clock->producer_node,
                    next_hour, next_min)) {

                clock->query_reply_pending = false;

            }

        }

    }

}


    /**
     * @brief Sends a Report Time event (PCER) for a producer clock.
     *
     * @details Algorithm:
     * -# Find the clock; if not found or not a producer, return true (nothing to do).
     * -# Build the time event ID and send it as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @param hour          Hour value to report (0-23).
     * @param minute        Minute value to report (0-59).
     * @endverbatim
     *
     * @return true if queued or clock not applicable, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_report_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_producer && clock->producer_node) {

            event_id_t event_id = ProtocolBroadcastTime_create_time_event_id(clock->state.clock_id, hour, minute, false);

            return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

        }

    }

    return true;  // done

}

    /**
     * @brief Sends a Report Date event (PCER) for a producer clock.
     *
     * @details Algorithm:
     * -# Find the clock; if not found or not a producer, return true (nothing to do).
     * -# Build the date event ID and send it as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @param month         Month value to report (1-12).
     * @param day           Day value to report (1-31).
     * @endverbatim
     *
     * @return true if queued or clock not applicable, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_report_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_producer && clock->producer_node) {

            event_id_t event_id = ProtocolBroadcastTime_create_date_event_id(clock->state.clock_id, month, day, false);

            return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

        }

    }

    return true;  // done

}

    /**
     * @brief Sends a Report Year event (PCER) for a producer clock.
     *
     * @details Algorithm:
     * -# Find the clock; if not found or not a producer, return true (nothing to do).
     * -# Build the year event ID and send it as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @param year          Year value to report.
     * @endverbatim
     *
     * @return true if queued or clock not applicable, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_report_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_producer && clock->producer_node) {

            event_id_t event_id = ProtocolBroadcastTime_create_year_event_id(clock->state.clock_id, year, false);

            return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

        }

    }

    return true;  // done

}

    /**
     * @brief Sends a Report Rate event (PCER) for a producer clock.
     *
     * @details Algorithm:
     * -# Find the clock; if not found or not a producer, return true (nothing to do).
     * -# Build the rate event ID and send it as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @param rate          12-bit signed fixed-point rate (4 = 1.0x real-time).
     * @endverbatim
     *
     * @return true if queued or clock not applicable, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_report_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_producer && clock->producer_node) {

            event_id_t event_id = ProtocolBroadcastTime_create_rate_event_id(clock->state.clock_id, rate, false);

            return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

        }

    }

    return true;  // done

}

    /**
     * @brief Sends a Start event (PCER) for a producer clock.
     *
     * @details Algorithm:
     * -# Find the clock; if not found or not a producer, return true (nothing to do).
     * -# Build the Start command event ID and send it as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return true if queued or clock not applicable, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_start(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_producer && clock->producer_node) {

            event_id_t event_id = ProtocolBroadcastTime_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_START);

            return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

        }

    }

    return true;  // done

}

    /**
     * @brief Sends a Stop event (PCER) for a producer clock.
     *
     * @details Algorithm:
     * -# Find the clock; if not found or not a producer, return true (nothing to do).
     * -# Build the Stop command event ID and send it as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return true if queued or clock not applicable, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_stop(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_producer && clock->producer_node) {

            event_id_t event_id = ProtocolBroadcastTime_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_STOP);

            return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

        }

    }

    return true;  // done

}

    /**
     * @brief Sends a Date Rollover event (PCER) for a producer clock.
     *
     * @details Algorithm:
     * -# Find the clock; if not found or not a producer, return true (nothing to do).
     * -# Build the Date Rollover command event ID and send it as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return true if queued or clock not applicable, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_date_rollover(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_producer && clock->producer_node) {

            event_id_t event_id = ProtocolBroadcastTime_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_DATE_ROLLOVER);

            return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

        }

    }

    return true;  // done

}

    /**
     * @brief Internal: drives the 6-message query reply state machine on a known clock.
     *
     * @details Sends one message per call in the Standard §6.3 sequence:
     * -# State 0: Start or Stop (Producer Identified Set).
     * -# State 1: Rate (Producer Identified Set).
     * -# State 2: Year (Producer Identified Set).
     * -# State 3: Date (Producer Identified Set).
     * -# State 4: Current Time (Producer Identified Set).
     * -# State 5: Next minute Time (PC Event Report).
     * Each state advances only when the send succeeds.  Returns true when state 5 completes.
     *
     * @param clock         Pointer to the broadcast_clock_t to operate on.
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param next_hour     Hour of the next scheduled time event (0-23).
     * @param next_minute   Minute of the next scheduled time event (0-59).
     *
     * @return true when all six messages have been queued, false if more calls are needed.
     */
static bool _send_query_reply_for_clock(broadcast_clock_t *clock, openlcb_node_t *openlcb_node, uint8_t next_hour, uint8_t next_minute) {

    event_id_t event_id;

    switch (clock->send_query_reply_state) {

        case 0:  // 1. Start or Stop ------------------------------------------

            if (clock->state.is_running) {

                event_id = ProtocolBroadcastTime_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_START);

            } else {

                event_id = ProtocolBroadcastTime_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_STOP);

            }

            if (OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

                clock->send_query_reply_state = 1;

            }

            break;

        case 1:  // 2. Rate ------------------------------------------

            event_id = ProtocolBroadcastTime_create_rate_event_id(clock->state.clock_id, clock->state.rate.rate, false);

            if (OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

                clock->send_query_reply_state = 2;

            }

            break;

        case 2:  // 3. Year ------------------------------------------

            event_id = ProtocolBroadcastTime_create_year_event_id(clock->state.clock_id, clock->state.year.year, false);

            if (OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

                clock->send_query_reply_state = 3;

            }

            break;

        case 3:  // 4. Date ------------------------------------------

            event_id = ProtocolBroadcastTime_create_date_event_id(clock->state.clock_id, clock->state.date.month, clock->state.date.day, false);

            if (OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

                clock->send_query_reply_state = 4;

            }

            break;

        case 4:  // 5. Time (Producer Identified Valid) ------------------------------------------

            event_id = ProtocolBroadcastTime_create_time_event_id(clock->state.clock_id, clock->state.time.hour, clock->state.time.minute, false);

            if (OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

                clock->send_query_reply_state = 5;

            }

            break;

        case 5:  // 6. Next minute (PC Event Report) ------------------------------------------

            event_id = ProtocolBroadcastTime_create_time_event_id(clock->state.clock_id, next_hour, next_minute, false);

            if (OpenLcbApplication_send_event_pc_report(openlcb_node, event_id)) {

                clock->send_query_reply_state = 0;

                return true; // done

            }

            break;

    }

    return false; // not done, more states to send

}


    /**
     * @brief Sends the full query reply sequence for a producer clock.
     *
     * @details Looks up the clock by clock_id and delegates to the internal
     * state machine.  See _send_query_reply_for_clock for the full sequence.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @param next_hour     Hour of the next scheduled time event (0-23).
     * @param next_minute   Minute of the next scheduled time event (0-59).
     * @endverbatim
     *
     * @return true when all six messages have been queued, false if more calls are needed.
     *
     * @note State is stored per-clock in broadcast_clock_t.send_query_reply_state,
     *       allowing concurrent query replies for different clocks.
     */
bool OpenLcbApplicationBroadcastTime_send_query_reply(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t next_hour, uint8_t next_minute) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_producer && clock->producer_node) {

            return _send_query_reply_for_clock(clock, openlcb_node, next_hour, next_minute);

        }

    }

    return true;  // done (no clock or not a producer)

}


    /**
     * @brief Sends a Query event (PCER) for a consumer clock.
     *
     * @details Algorithm:
     * -# Find the clock; if not found or not a consumer, return true (nothing to do).
     * -# Build the Query command event ID and send it as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the clock.
     * @endverbatim
     *
     * @return true if queued or clock not applicable, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_query(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock) {

        if (clock->is_allocated && clock->is_consumer) {

            event_id_t event_id = ProtocolBroadcastTime_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_QUERY);

            return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

        }

    }

    return true;  // done

}


    /**
     * @brief Sends a Set Time command to a clock generator.
     *
     * @details Algorithm:
     * -# Build the time event ID with the set flag.
     * -# Send as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the target clock.
     * @param hour          Desired hour (0-23).
     * @param minute        Desired minute (0-59).
     * @endverbatim
     *
     * @return true if queued, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_set_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute) {

    event_id_t event_id = ProtocolBroadcastTime_create_time_event_id(clock_id, hour, minute, true);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

    /**
     * @brief Sends a Set Date command to a clock generator.
     *
     * @details Algorithm:
     * -# Build the date event ID with the set flag.
     * -# Send as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the target clock.
     * @param month         Desired month (1-12).
     * @param day           Desired day (1-31).
     * @endverbatim
     *
     * @return true if queued, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_set_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day) {

    event_id_t event_id = ProtocolBroadcastTime_create_date_event_id(clock_id, month, day, true);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

    /**
     * @brief Sends a Set Year command to a clock generator.
     *
     * @details Algorithm:
     * -# Build the year event ID with the set flag.
     * -# Send as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the target clock.
     * @param year          Desired year.
     * @endverbatim
     *
     * @return true if queued, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_set_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year) {

    event_id_t event_id = ProtocolBroadcastTime_create_year_event_id(clock_id, year, true);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

    /**
     * @brief Sends a Set Rate command to a clock generator.
     *
     * @details Algorithm:
     * -# Build the rate event ID with the set flag.
     * -# Send as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the target clock.
     * @param rate          12-bit signed fixed-point rate (4 = 1.0x real-time).
     * @endverbatim
     *
     * @return true if queued, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_set_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate) {

    event_id_t event_id = ProtocolBroadcastTime_create_rate_event_id(clock_id, rate, true);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

    /**
     * @brief Sends a Start command to a clock generator.
     *
     * @details Algorithm:
     * -# Build the Start command event ID.
     * -# Send as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the target clock.
     * @endverbatim
     *
     * @return true if queued, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_command_start(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    event_id_t event_id = ProtocolBroadcastTime_create_command_event_id(clock_id, BROADCAST_TIME_EVENT_START);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

    /**
     * @brief Sends a Stop command to a clock generator.
     *
     * @details Algorithm:
     * -# Build the Stop command event ID.
     * -# Send as a PC Event Report.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param clock_id      64-bit event_id_t identifying the target clock.
     * @endverbatim
     *
     * @return true if queued, false if the transmit buffer is full.
     */
bool OpenLcbApplicationBroadcastTime_send_command_stop(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    event_id_t event_id = ProtocolBroadcastTime_create_command_event_id(clock_id, BROADCAST_TIME_EVENT_STOP);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}


    /**
     * @brief Triggers an immediate query reply for a producer clock.
     *
     * @details Sets the query_reply_pending flag and resets the state machine
     * so the 100ms tick will drive the 6-message sync sequence.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t identifying the clock.
     * @endverbatim
     */
void OpenLcbApplicationBroadcastTime_trigger_query_reply(event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock && clock->is_producer) {

        clock->query_reply_pending = true;
        clock->send_query_reply_state = 0;

        // Cancel any pending sync delay — the query reply supersedes it
        clock->sync_pending = false;
        clock->sync_delay_ticks = 0;

    }

}


    /**
     * @brief Starts or resets the 3-second sync delay timer for a producer clock.
     *
     * @details Each call resets the timer to 30 ticks (3 seconds at 100ms).
     * When the timer expires, the query reply state machine is triggered
     * automatically, sending the 6-message sync sequence.
     *
     * @verbatim
     * @param clock_id  64-bit event_id_t identifying the clock.
     * @endverbatim
     */
void OpenLcbApplicationBroadcastTime_trigger_sync_delay(event_id_t clock_id) {

    broadcast_clock_t *clock = _find_clock_by_id(clock_id);

    if (clock && clock->is_producer) {

        clock->sync_delay_ticks = 30;  // 3 seconds at 100ms
        clock->sync_pending = true;

    }

}

event_id_t OpenLcbApplicationBroadcastTime_make_clock_id(uint64_t unique_id_48bit) {

    return (unique_id_48bit << 16) & BROADCAST_TIME_MASK_CLOCK_ID;

}

#endif /* OPENLCB_COMPILE_BROADCAST_TIME */
