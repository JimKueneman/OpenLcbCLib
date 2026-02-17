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
 * @brief Implementation of application-level Broadcast Time Protocol module
 *
 * @author Jim Kueneman
 * @date 14 Feb 2026
 */

#include "openlcb_application_broadcast_time.h"

#include <stddef.h>
#include <string.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_application.h"
#include "protocol_broadcast_time_handler.h"


static broadcast_clock_t _clocks[BROADCAST_TIME_TOTAL_CLOCK_COUNT];
static const interface_openlcb_application_broadcast_time_t *_interface;

// Static helpers

static broadcast_clock_t *_find_clock_by_id(event_id_t clock_id) {

  for (int i = 0; i < BROADCAST_TIME_TOTAL_CLOCK_COUNT; i++) {

    if (_clocks[i].is_allocated && _clocks[i].state.clock_id == clock_id) {

      return &_clocks[i];
    }
  }

  return NULL;
}

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


// Initialization

void OpenLcbApplicationBroadcastTime_initialize(const interface_openlcb_application_broadcast_time_t *interface) {

  memset(_clocks, 0, sizeof(_clocks));
  _interface = interface;

}


// Setup

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

broadcast_clock_state_t *OpenLcbApplicationBroadcastTime_setup_producer(openlcb_node_t *openlcb_node, event_id_t clock_id) {

  broadcast_clock_t *clock = _find_or_allocate_clock(clock_id);

  if (!clock) {

    return NULL;

  }

  clock->is_producer = 1;

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

void OpenLcbApplicationBroadcastTime_start(event_id_t clock_id) {

  broadcast_clock_t *broadcast_clock = _find_clock_by_id(clock_id);

  if (!broadcast_clock) {

    return;

  }

  broadcast_clock->state.is_running = true;

}

void OpenLcbApplicationBroadcastTime_stop(event_id_t clock_id) {

  broadcast_clock_t *broadcast_clock = _find_clock_by_id(clock_id);

  if (!broadcast_clock) {

    return;

  }

  broadcast_clock->state.is_running = false;

}


// Accessors

broadcast_clock_state_t *OpenLcbApplicationBroadcastTime_get_clock(event_id_t clock_id) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    return &clock->state;

  }

  return NULL;

}

bool OpenLcbApplicationBroadcastTime_is_consumer(event_id_t clock_id) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    return clock->is_consumer;

  }

  return 0;

}

bool OpenLcbApplicationBroadcastTime_is_producer(event_id_t clock_id) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    return clock->is_producer;

  }

  return 0;

}

// Time tick helpers

static const uint8_t _days_in_month_table[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static bool _is_leap_year(uint16_t year) {

  return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);

}

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

static void _advance_minute_forward(broadcast_clock_state_t *clock, openlcb_node_t *node) {

  clock->time.minute++;

  if (clock->time.minute >= 60) {

    clock->time.minute = 0;
    clock->time.hour++;

    if (clock->time.hour >= 24) {

      clock->time.hour = 0;

      if (ProtocolBroadcastTime_get_interface() && ProtocolBroadcastTime_get_interface()->on_date_rollover) {

        ProtocolBroadcastTime_get_interface()->on_date_rollover(node, clock);

      }

      clock->date.day++;

      uint8_t dim = _days_in_month(clock->date.month, clock->year.year);

      if (clock->date.day > dim) {

        clock->date.day = 1;
        clock->date.month++;

        if (clock->date.month > 12) {

          clock->date.month = 1;
          clock->year.year++;

          if (ProtocolBroadcastTime_get_interface() && ProtocolBroadcastTime_get_interface()->on_year_received) {

            ProtocolBroadcastTime_get_interface()->on_year_received(node, clock);

          }

        }

        if (ProtocolBroadcastTime_get_interface() && ProtocolBroadcastTime_get_interface()->on_date_received) {

          ProtocolBroadcastTime_get_interface()->on_date_received(node, clock);
        }

      }

    }

  }

  if (ProtocolBroadcastTime_get_interface() && ProtocolBroadcastTime_get_interface()->on_time_received) {

    ProtocolBroadcastTime_get_interface()->on_time_received(node, clock);

  }

}

static void _advance_minute_backward(broadcast_clock_state_t *clock, openlcb_node_t *node) {

  if (clock->time.minute == 0) {

    clock->time.minute = 59;

    if (clock->time.hour == 0) {

      clock->time.hour = 23;

      if (ProtocolBroadcastTime_get_interface() && ProtocolBroadcastTime_get_interface()->on_date_rollover) {

        ProtocolBroadcastTime_get_interface()->on_date_rollover(node, clock);

      }

      if (clock->date.day <= 1) {

        if (clock->date.month <= 1) {

          clock->date.month = 12;
          clock->year.year--;

          if (ProtocolBroadcastTime_get_interface() && ProtocolBroadcastTime_get_interface()->on_year_received) {

            ProtocolBroadcastTime_get_interface()->on_year_received(node, clock);

          }

        } else {

          clock->date.month--;

        }

        clock->date.day = _days_in_month(clock->date.month, clock->year.year);

        if (ProtocolBroadcastTime_get_interface() && ProtocolBroadcastTime_get_interface()->on_date_received) {

          ProtocolBroadcastTime_get_interface()->on_date_received(node, clock);

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

  if (ProtocolBroadcastTime_get_interface() && ProtocolBroadcastTime_get_interface()->on_time_received) {

    ProtocolBroadcastTime_get_interface()->on_time_received(node, clock);

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

void OpenLcbApplicationBroadcastTime_100ms_time_tick(void) {

  for (int i = 0; i < BROADCAST_TIME_TOTAL_CLOCK_COUNT; i++) {

    broadcast_clock_t *clock = &_clocks[i];

    if (!clock->is_allocated || !clock->is_consumer || !clock->state.is_running) {

      continue;

    }

    int16_t rate = clock->state.rate.rate;

    if (rate == 0) {

      continue;

    }

    uint16_t abs_rate = (rate < 0) ? (uint16_t)(-rate) : (uint16_t)(rate);

    clock->state.ms_accumulator += (uint32_t)(50) * abs_rate;

    while (clock->state.ms_accumulator >= BROADCAST_TIME_MS_PER_MINUTE_FIXED_POINT) {

      clock->state.ms_accumulator -= BROADCAST_TIME_MS_PER_MINUTE_FIXED_POINT;

      if (rate > 0) {

        _advance_minute_forward(&clock->state, NULL);

      } else {

        _advance_minute_backward(&clock->state, NULL);

      }

      if (_interface->on_time_changed) {

        _interface->on_time_changed(clock);

      }

    }

  }

}


// Producer send functions

bool OpenLcbApplicationBroadcastTime_send_report_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    if (clock->is_allocated && clock->is_producer) {

      event_id_t event_id = OpenLcbUtilities_create_time_event_id(clock->state.clock_id, hour, minute, false);

      return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

    }

  }

  return true;  // done

}

bool OpenLcbApplicationBroadcastTime_send_report_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    if (clock->is_allocated && clock->is_producer) {

      event_id_t event_id = OpenLcbUtilities_create_date_event_id(clock->state.clock_id, month, day, false);

      return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

    }

  }

  return true;  // done

}

bool OpenLcbApplicationBroadcastTime_send_report_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    if (clock->is_allocated && clock->is_producer) {

      event_id_t event_id = OpenLcbUtilities_create_year_event_id(clock->state.clock_id, year, false);

      return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

    }

  }

  return true;  // done

}

bool OpenLcbApplicationBroadcastTime_send_report_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    if (clock->is_allocated && clock->is_producer) {

      event_id_t event_id = OpenLcbUtilities_create_rate_event_id(clock->state.clock_id, rate, false);

      return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

    }

  }

  return true;  // done

}

bool OpenLcbApplicationBroadcastTime_send_start(openlcb_node_t *openlcb_node, event_id_t clock_id) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    if (clock->is_allocated && clock->is_producer) {

      event_id_t event_id = OpenLcbUtilities_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_START);

      return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

    }

  }

  return true;  // done

}

bool OpenLcbApplicationBroadcastTime_send_stop(openlcb_node_t *openlcb_node, event_id_t clock_id) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    if (clock->is_allocated && clock->is_producer) {

      event_id_t event_id = OpenLcbUtilities_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_STOP);

      return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

    }

  }

  return true;  // done

}

bool OpenLcbApplicationBroadcastTime_send_date_rollover(openlcb_node_t *openlcb_node, event_id_t clock_id) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    if (clock->is_allocated && clock->is_producer) {

      event_id_t event_id = OpenLcbUtilities_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_DATE_ROLLOVER);

      return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

    }

  }

  return true;  // done

}

bool OpenLcbApplicationBroadcastTime_send_query_reply(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t next_hour, uint8_t next_minute) {

  static int _send_query_reply_state = 0;

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);
  event_id_t event_id;

  if (clock) {

    if (clock->is_allocated && clock->is_producer) {

      switch (_send_query_reply_state) {

        case 0:  // 1. Start or Stop ------------------------------------------

          if (clock->state.is_running) {

            event_id = OpenLcbUtilities_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_START);

          } else {

            event_id = OpenLcbUtilities_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_STOP);

          }

          if (!OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

            _send_query_reply_state = 1;

            return false; // not done

          }

          break;

        case 1:  // 2. Rate ------------------------------------------

          event_id = OpenLcbUtilities_create_rate_event_id(clock->state.clock_id, clock->state.rate.rate, false);

          if (!OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

            _send_query_reply_state = 2;

            return false; // not done

          }

          break;

        case 2:  // 3. Year ------------------------------------------

          event_id = OpenLcbUtilities_create_year_event_id(clock->state.clock_id, clock->state.year.year, false);

          if (!OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

            _send_query_reply_state = 3;

            return false; // not done

          }

          break;

        case 3:  // 4. Date ------------------------------------------

          event_id = OpenLcbUtilities_create_date_event_id(clock->state.clock_id, clock->state.date.month, clock->state.date.day, false);

          if (!OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

            _send_query_reply_state = 4;

            return false; // not done

          }

          break;

        case 4:  // 5. Time (Producer Identified Valid) ------------------------------------------

          event_id = OpenLcbUtilities_create_time_event_id(clock->state.clock_id, clock->state.time.hour, clock->state.time.minute, false);

          if (!OpenLcbApplication_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET)) {

            _send_query_reply_state = 5;

            return false; // not done

          }

          break;

        case 5:  // 6. Next minute (PC Event Report) ------------------------------------------

          event_id = OpenLcbUtilities_create_time_event_id(clock->state.clock_id, next_hour, next_minute, false);

          if (OpenLcbApplication_send_event_pc_report(openlcb_node, event_id)) {

            _send_query_reply_state = 0;

            return true; // done

          }

          break;
      }

    }  // if (clock->is_allocated && clock->is_producer)

  }  // if (clock)

  return true;  // done

}


// Consumer send functions

bool OpenLcbApplicationBroadcastTime_send_query(openlcb_node_t *openlcb_node, event_id_t clock_id) {

  broadcast_clock_t *clock = _find_clock_by_id(clock_id);

  if (clock) {

    if (clock->is_allocated && clock->is_consumer) {

      event_id_t event_id = OpenLcbUtilities_create_command_event_id(clock->state.clock_id, BROADCAST_TIME_EVENT_QUERY);

      return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

    }

  }

  return true;  // done
}


// Controller send functions

bool OpenLcbApplicationBroadcastTime_send_set_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute) {

  event_id_t event_id = OpenLcbUtilities_create_time_event_id(clock_id, hour, minute, true);

  return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

bool OpenLcbApplicationBroadcastTime_send_set_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day) {

  event_id_t event_id = OpenLcbUtilities_create_date_event_id(clock_id, month, day, true);

  return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

bool OpenLcbApplicationBroadcastTime_send_set_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year) {

  event_id_t event_id = OpenLcbUtilities_create_year_event_id(clock_id, year, true);

  return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

bool OpenLcbApplicationBroadcastTime_send_set_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate) {

  event_id_t event_id = OpenLcbUtilities_create_rate_event_id(clock_id, rate, true);

  return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

bool OpenLcbApplicationBroadcastTime_send_command_start(openlcb_node_t *openlcb_node, event_id_t clock_id) {

  event_id_t event_id = OpenLcbUtilities_create_command_event_id(clock_id, BROADCAST_TIME_EVENT_START);

  return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}

bool OpenLcbApplicationBroadcastTime_send_command_stop(openlcb_node_t *openlcb_node, event_id_t clock_id) {

  event_id_t event_id = OpenLcbUtilities_create_command_event_id(clock_id, BROADCAST_TIME_EVENT_STOP);

  return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);

}
