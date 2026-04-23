/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * @file openlcb_config.c
 * @brief Library-internal wiring module for config facade
 *
 * @details Reads from openlcb_config_t and compile-time feature guards
 * (OPENLCB_COMPILE_*), builds internal interface structs, and calls all
 * Module_initialize() functions in the correct order.
 *
 * @author Jim Kueneman
 * @date 23 Apr 2026
 */

#include "openlcb_config.h"

#include <string.h>

// All module headers
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_list.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_node.h"
#include "openlcb_application.h"
#include "openlcb_main_statemachine.h"
#include "openlcb_login_statemachine.h"
#include "openlcb_login_statemachine_handler.h"
#include "protocol_message_network.h"
#include "protocol_snip.h"

#ifdef OPENLCB_COMPILE_EVENTS
#include "protocol_event_transport.h"
#endif

#ifdef OPENLCB_COMPILE_DATAGRAMS
#include "protocol_datagram_handler.h"
#endif

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#include "protocol_config_mem_read_handler.h"
#include "protocol_config_mem_write_handler.h"
#include "protocol_config_mem_operations_handler.h"
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
#include "protocol_broadcast_time_handler.h"
#include "openlcb_application_broadcast_time.h"
#endif

#ifdef OPENLCB_COMPILE_TRAIN
#include "protocol_train_handler.h"
#include "openlcb_application_train.h"
#endif

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
#include "protocol_train_search_handler.h"
#endif

#ifdef OPENLCB_COMPILE_STREAM
#include "protocol_stream_handler.h"
#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#ifndef OPENLCB_COMPILE_BOOTLOADER
#include "protocol_config_mem_stream_handler.h"
#endif
#endif
#endif

// Transport-specific includes
#ifdef OPENLCB_COMPILE_CAN
#include "../drivers/canbus/can_tx_statemachine.h"
#include "../drivers/canbus/can_main_statemachine.h"
#endif

#ifdef OPENLCB_COMPILE_TCP
#include "../drivers/tcp_ip/tcp_config.h"
#endif

// ---- Internal storage for built interface structs ----

static interface_openlcb_main_statemachine_t _main_sm;
static interface_openlcb_login_state_machine_t _login_sm;
static interface_openlcb_login_message_handler_t _login_msg;
static interface_openlcb_node_t _node;
static interface_openlcb_application_t _app;
static interface_openlcb_protocol_snip_t _snip;
static interface_openlcb_protocol_message_network_t _msg_network;

#ifdef OPENLCB_COMPILE_EVENTS
static interface_openlcb_protocol_event_transport_t _event_transport;
#endif

#ifdef OPENLCB_COMPILE_DATAGRAMS
static interface_protocol_datagram_handler_t _datagram;
#endif

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
static interface_protocol_config_mem_read_handler_t _config_read;
static interface_protocol_config_mem_write_handler_t _config_write;
static interface_protocol_config_mem_operations_handler_t _config_ops;
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
static interface_openlcb_protocol_broadcast_time_handler_t _broadcast_time;
static interface_openlcb_application_broadcast_time_t _app_broadcast_time;
#endif

#ifdef OPENLCB_COMPILE_TRAIN
static interface_protocol_train_handler_t _train_handler;
static interface_openlcb_application_train_t _app_train;
#endif

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
static interface_protocol_train_search_handler_t _train_search;
#endif

#ifdef OPENLCB_COMPILE_STREAM
static interface_protocol_stream_handler_t _stream_handler;
#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#ifndef OPENLCB_COMPILE_BOOTLOADER
static interface_protocol_config_mem_stream_handler_t _config_mem_stream;
#endif
#endif
#endif

static const openlcb_config_t *_config;

    /**
     * @brief Global 100ms tick counter — the sole action of the timer interrupt.
     *
     * @details This counter is the library's timekeeping foundation. It is
     * incremented once per 100ms timer tick (which may be an interrupt or a
     * separate thread depending on the platform).
     *
     * ALL other modules compute elapsed time by snapshotting this counter at a
     * start event and later subtracting: elapsed = (current - snapshot). The
     * uint8_t wraps at 256, and unsigned subtraction handles the wrap correctly
     * for durations up to 255 ticks (25.5 seconds).
     *
     * Platform safety: volatile uint8_t reads and writes are single-instruction
     * operations on all target architectures (8-bit PIC through 64-bit ARM).
     * Only the timer interrupt performs the increment (read-modify-write); all
     * other contexts only read. Timer interrupts do not re-enter, so the
     * increment is safe without locking.
     *
     * Module isolation: no module calls this directly. The wiring code reads it
     * once per main-loop iteration and passes the value down as a parameter.
     * The Rx handler receives it via an interface function pointer. This
     * preserves the library's dependency-injection pattern.
     */
static volatile uint8_t _global_100ms_tick = 0;

    /**
     * @brief Returns the current value of the global 100ms tick counter.
     *
     * @details Used by wiring code (openlcb_config.c, can_config.c) to read
     * the clock and inject it into modules via parameters or interface
     * function pointers. Individual modules should NOT call this directly —
     * they receive the tick through their function parameters or interface.
     *
     * @return Current tick count (wraps at 255).
     */
uint8_t OpenLcbConfig_get_global_100ms_tick(void) {

    return _global_100ms_tick;

}

// ---- Build functions ----

#ifdef OPENLCB_COMPILE_EVENTS

    /** @brief Wires user event callbacks into the event transport interface struct. */
static void _build_event_transport(void) {

    memset(&_event_transport, 0, sizeof(_event_transport));

    // Map user callbacks directly
    _event_transport.on_consumed_event_identified    = _config->on_consumed_event_identified;
    _event_transport.on_consumed_event_pcer          = _config->on_consumed_event_pcer;
    _event_transport.on_event_learn                  = _config->on_event_learn;
    _event_transport.on_consumer_range_identified    = _config->on_consumer_range_identified;
    _event_transport.on_consumer_identified_unknown  = _config->on_consumer_identified_unknown;
    _event_transport.on_consumer_identified_set      = _config->on_consumer_identified_set;
    _event_transport.on_consumer_identified_clear    = _config->on_consumer_identified_clear;
    _event_transport.on_consumer_identified_reserved = _config->on_consumer_identified_reserved;
    _event_transport.on_producer_range_identified    = _config->on_producer_range_identified;
    _event_transport.on_producer_identified_unknown  = _config->on_producer_identified_unknown;
    _event_transport.on_producer_identified_set      = _config->on_producer_identified_set;
    _event_transport.on_producer_identified_clear    = _config->on_producer_identified_clear;
    _event_transport.on_producer_identified_reserved = _config->on_producer_identified_reserved;
    _event_transport.on_pc_event_report              = _config->on_pc_event_report;
    _event_transport.on_pc_event_report_with_payload = _config->on_pc_event_report_with_payload;

}

#endif /* OPENLCB_COMPILE_EVENTS */

#ifdef OPENLCB_COMPILE_BROADCAST_TIME

    /** @brief Wires user broadcast-time callbacks into the handler interface struct. */
static void _build_broadcast_time(void) {

    memset(&_broadcast_time, 0, sizeof(_broadcast_time));

    _broadcast_time.on_time_received = _config->on_broadcast_time_received;
    _broadcast_time.on_date_received = _config->on_broadcast_date_received;
    _broadcast_time.on_year_received = _config->on_broadcast_year_received;
    _broadcast_time.on_rate_received = _config->on_broadcast_rate_received;
    _broadcast_time.on_clock_started = _config->on_broadcast_clock_started;
    _broadcast_time.on_clock_stopped = _config->on_broadcast_clock_stopped;
    _broadcast_time.on_date_rollover = _config->on_broadcast_date_rollover;

}

    /** @brief Wires user broadcast-time callbacks into the application interface struct. */
static void _build_app_broadcast_time(void) {

    memset(&_app_broadcast_time, 0, sizeof(_app_broadcast_time));

    _app_broadcast_time.on_time_changed = _config->on_broadcast_time_changed;
    _app_broadcast_time.on_time_received = _config->on_broadcast_time_received;
    _app_broadcast_time.on_date_received = _config->on_broadcast_date_received;
    _app_broadcast_time.on_year_received = _config->on_broadcast_year_received;
    _app_broadcast_time.on_date_rollover = _config->on_broadcast_date_rollover;

}

#endif /* OPENLCB_COMPILE_BROADCAST_TIME */

#ifdef OPENLCB_COMPILE_TRAIN

    /** @brief Wires user train callbacks into the train handler interface struct. */
static void _build_train_handler(void) {

    memset(&_train_handler, 0, sizeof(_train_handler));

    // Train-node side: notifiers
    _train_handler.on_speed_changed       = _config->on_train_speed_changed;
    _train_handler.on_function_changed    = _config->on_train_function_changed;
    _train_handler.on_emergency_entered   = _config->on_train_emergency_entered;
    _train_handler.on_emergency_exited    = _config->on_train_emergency_exited;
    _train_handler.on_controller_assigned = _config->on_train_controller_assigned;
    _train_handler.on_controller_released = _config->on_train_controller_released;
    _train_handler.on_listener_changed    = _config->on_train_listener_changed;
    _train_handler.on_heartbeat_timeout   = _config->on_train_heartbeat_timeout;

    // Train-node side: decision callbacks
    _train_handler.on_controller_assign_request  = _config->on_train_controller_assign_request;
    _train_handler.on_controller_changed_request = _config->on_train_controller_changed_request;

    // Throttle-side: reply notifiers
    _train_handler.on_query_speeds_reply              = _config->on_train_query_speeds_reply;
    _train_handler.on_query_function_reply            = _config->on_train_query_function_reply;
    _train_handler.on_controller_assign_reply         = _config->on_train_controller_assign_reply;
    _train_handler.on_controller_query_reply          = _config->on_train_controller_query_reply;
    _train_handler.on_controller_changed_notify_reply = _config->on_train_controller_changed_notify_reply;
    _train_handler.on_listener_attach_reply           = _config->on_train_listener_attach_reply;
    _train_handler.on_listener_detach_reply           = _config->on_train_listener_detach_reply;
    _train_handler.on_listener_query_reply            = _config->on_train_listener_query_reply;
    _train_handler.on_reserve_reply                   = _config->on_train_reserve_reply;
    _train_handler.on_heartbeat_request               = _config->on_train_heartbeat_request;

}

    /** @brief Wires train send function and heartbeat callback into the application train interface. */
static void _build_app_train(void) {

    memset(&_app_train, 0, sizeof(_app_train));

    _app_train.send_openlcb_msg = &OpenLcbMainStatemachine_send_with_sibling_dispatch;
    _app_train.on_heartbeat_timeout = _config->on_train_heartbeat_timeout;

}

#endif /* OPENLCB_COMPILE_TRAIN */

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)

    /** @brief Wires user train-search callbacks into the search handler interface struct. */
static void _build_train_search_handler(void) {

    memset(&_train_search, 0, sizeof(_train_search));

    _train_search.on_search_matched  = _config->on_train_search_matched;
    _train_search.on_search_no_match_with_allocate = _config->on_train_search_no_match_with_allocate;

}

#endif /* OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH */

#ifdef OPENLCB_COMPILE_STREAM

    /** @brief Wires stream handler callbacks.
     *
     * When config-mem-stream is active, the router callbacks intercept
     * stream events and forward non-config-mem streams to the user.
     * Otherwise, user callbacks are wired directly. */
static void _build_stream_handler(void) {

    memset(&_stream_handler, 0, sizeof(_stream_handler));

#if defined(OPENLCB_COMPILE_MEMORY_CONFIGURATION) && !defined(OPENLCB_COMPILE_BOOTLOADER)

    _stream_handler.on_initiate_request = &ProtocolConfigMemStreamHandler_on_initiate_request;
    _stream_handler.on_initiate_reply   = &ProtocolConfigMemStreamHandler_on_initiate_reply;
    _stream_handler.on_data_received    = &ProtocolConfigMemStreamHandler_on_data_received;
    _stream_handler.on_data_proceed     = &ProtocolConfigMemStreamHandler_on_data_proceed;
    _stream_handler.on_complete         = &ProtocolConfigMemStreamHandler_on_complete;

#else

    _stream_handler.on_initiate_request = _config->on_stream_initiate_request;
    _stream_handler.on_initiate_reply   = _config->on_stream_initiate_reply;
    _stream_handler.on_data_received    = _config->on_stream_data_received;
    _stream_handler.on_data_proceed     = _config->on_stream_data_proceed;
    _stream_handler.on_complete         = _config->on_stream_complete;

#endif

}

#if defined(OPENLCB_COMPILE_MEMORY_CONFIGURATION) && !defined(OPENLCB_COMPILE_BOOTLOADER)

// ---- Per-space stream read-request callbacks ----

    /** @brief Stream read from CDI (0xFF): copy from node->parameters->cdi[]. */
static uint16_t _stream_read_request_config_definition_info(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    if (!node->parameters->cdi) {

        return 0;

    }

    memcpy(buffer, &node->parameters->cdi[address], count);

    return count;

}

    /** @brief Stream read from All (0xFE): same layout as CDI. */
static uint16_t _stream_read_request_all(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    if (!node->parameters->cdi) {

        return 0;

    }

    memcpy(buffer, &node->parameters->cdi[address], count);

    return count;

}

    /** @brief Stream read from Config Memory (0xFD): delegate to user callback. */
static uint16_t _stream_read_request_configuration_memory(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    if (!_config->config_mem_read) {

        return 0;

    }

    return _config->config_mem_read(node, address, count, (configuration_memory_buffer_t *) buffer);

}

    /**
     * @brief Stream read from ACDI Manufacturer (0xFC): flat sequential reader.
     *
     * @details The 0xFC space is a flat image of the manufacturer SNIP fields:
     * - 0x00: mfg_version (1 byte)
     * - 0x01-0x29: name (41 bytes)
     * - 0x2A-0x52: model (41 bytes)
     * - 0x53-0x67: hardware_version (21 bytes)
     * - 0x68-0x7C: software_version (21 bytes)
     *
     * Reads an arbitrary byte range across field boundaries.
     */
typedef struct {

    uint32_t start;
    uint16_t len;
    const void *data;

} _config_field_t;

static uint16_t _read_field_array(const _config_field_t *fields,
        uint16_t num_fields, uint32_t address, uint16_t count, uint8_t *buffer) {

    uint16_t filled = 0;

    for (uint16_t i = 0; i < num_fields && filled < count; i++) {

        uint32_t field_end = fields[i].start + fields[i].len;

        if (address + filled >= field_end) {

            continue;

        }

        if (address + filled < fields[i].start) {

            uint32_t gap = fields[i].start - (address + filled);

            if (gap > (uint32_t) (count - filled)) {

                gap = count - filled;

            }

            memset(&buffer[filled], 0, gap);
            filled += (uint16_t) gap;

            if (filled >= count) {

                break;

            }

        }

        uint32_t offset_in_field = (address + filled) - fields[i].start;
        uint32_t avail = fields[i].len - offset_in_field;
        uint16_t to_copy = (uint16_t) ((avail < (uint32_t) (count - filled)) ? avail : (count - filled));

        memcpy(&buffer[filled], (const uint8_t *) fields[i].data + offset_in_field, to_copy);
        filled += to_copy;

    }

    return filled;

}

static uint16_t _stream_read_request_acdi_manufacturer(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    const _config_field_t fields[] = {

        { CONFIG_MEM_ACDI_MANUFACTURER_VERSION_ADDRESS, CONFIG_MEM_ACDI_VERSION_LEN,
          &node->parameters->snip.mfg_version },
        { CONFIG_MEM_ACDI_MANUFACTURER_ADDRESS, CONFIG_MEM_ACDI_MANUFACTURER_LEN,
          node->parameters->snip.name },
        { CONFIG_MEM_ACDI_MODEL_ADDRESS, CONFIG_MEM_ACDI_MODEL_LEN,
          node->parameters->snip.model },
        { CONFIG_MEM_ACDI_HARDWARE_VERSION_ADDRESS, CONFIG_MEM_ACDI_HARDWARE_VERSION_LEN,
          node->parameters->snip.hardware_version },
        { CONFIG_MEM_ACDI_SOFTWARE_VERSION_ADDRESS, CONFIG_MEM_ACDI_SOFTWARE_VERSION_LEN,
          node->parameters->snip.software_version },

    };

    uint16_t num_fields = sizeof(fields) / sizeof(fields[0]);

    return _read_field_array(fields, num_fields, address, count, buffer);

}

    /**
     * @brief Stream read from ACDI User (0xFB): flat sequential reader.
     *
     * @details The 0xFB space layout:
     * - 0x00: user_version (1 byte)
     * - 0x01-0x3F: user_name (63 bytes, from config memory)
     * - 0x40-0x7F: user_description (64 bytes, from config memory)
     *
     * User name and description are stored in configuration memory (0xFD space)
     * starting at CONFIG_MEM_CONFIG_USER_NAME_OFFSET.
     */
static uint16_t _stream_read_request_acdi_user(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    uint16_t filled = 0;

    while (filled < count) {

        uint32_t pos = address + filled;

        if (pos == CONFIG_MEM_ACDI_USER_VERSION_ADDRESS) {

            buffer[filled] = node->parameters->snip.user_version;
            filled++;

        } else if (pos >= CONFIG_MEM_ACDI_USER_NAME_ADDRESS &&  // GCOV_EXCL_BR_LINE
                pos < CONFIG_MEM_ACDI_USER_NAME_ADDRESS + CONFIG_MEM_ACDI_USER_NAME_LEN) {

            if (!_config->config_mem_read) {

                buffer[filled] = 0;
                filled++;
                continue;

            }

            uint32_t offset_in_name = pos - CONFIG_MEM_ACDI_USER_NAME_ADDRESS;
            uint16_t name_avail = (uint16_t) (CONFIG_MEM_ACDI_USER_NAME_LEN - offset_in_name);
            uint16_t to_read = (uint16_t) ((name_avail < (count - filled)) ? name_avail : (count - filled));

            uint32_t config_addr = CONFIG_MEM_CONFIG_USER_NAME_OFFSET + offset_in_name;

            if (node->parameters->address_space_config_memory.low_address_valid) {

                config_addr += node->parameters->address_space_config_memory.low_address;

            }

            uint16_t actual = _config->config_mem_read(node, config_addr, to_read, (configuration_memory_buffer_t *) &buffer[filled]);

            filled += actual;

            if (actual < to_read) {

                break;

            }

        } else if (pos >= CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS &&  // GCOV_EXCL_BR_LINE
                pos < CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS + CONFIG_MEM_ACDI_USER_DESCRIPTION_LEN) {

            if (!_config->config_mem_read) {

                buffer[filled] = 0;
                filled++;
                continue;

            }

            uint32_t offset_in_desc = pos - CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS;
            uint16_t desc_avail = (uint16_t) (CONFIG_MEM_ACDI_USER_DESCRIPTION_LEN - offset_in_desc);
            uint16_t to_read = (uint16_t) ((desc_avail < (count - filled)) ? desc_avail : (count - filled));

            uint32_t config_addr = CONFIG_MEM_CONFIG_USER_DESCRIPTION_OFFSET + offset_in_desc;

            if (node->parameters->address_space_config_memory.low_address_valid) {

                config_addr += node->parameters->address_space_config_memory.low_address;

            }

            uint16_t actual = _config->config_mem_read(node, config_addr, to_read, (configuration_memory_buffer_t *) &buffer[filled]);

            filled += actual;

            if (actual < to_read) {

                break;

            }

        } else {

            break;

        }

    }

    return filled;

}

#ifdef OPENLCB_COMPILE_TRAIN

    /** @brief Stream read from Train FDI (0xFA): copy from node->parameters->fdi[]. */
static uint16_t _stream_read_request_train_function_definition_info(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    if (!node->parameters->fdi) {

        return 0;

    }

    memcpy(buffer, &node->parameters->fdi[address], count);

    return count;

}

    /**
     * @brief Stream read from Train Fn Config (0xF9): map flat byte address to functions[].
     *
     * @details Each 16-bit function value occupies 2 bytes big-endian:
     * address/2 = fn_index, address%2 selects high/low byte.
     */
static uint16_t _stream_read_request_train_function_config_memory(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    if (!state) {

        return 0;

    }

    for (uint16_t i = 0; i < count; i++) {

        uint32_t pos = address + i;
        uint16_t fn_index = (uint16_t) (pos / 2);
        uint8_t byte_sel = (uint8_t) (pos % 2);

        if (fn_index < USER_DEFINED_MAX_TRAIN_FUNCTIONS) {

            buffer[i] = (byte_sel == 0)
                    ? (uint8_t) (state->functions[fn_index] >> 8)
                    : (uint8_t) (state->functions[fn_index] & 0xFF);

        } else {

            buffer[i] = 0;

        }

    }

    return count;

}

#endif /* OPENLCB_COMPILE_TRAIN */

    /** @brief Stream write to Config Memory (0xFD): delegates to user config_mem_write. */
static uint16_t _stream_write_request_configuration_memory(openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer) {

    return _config->config_mem_write(node, address, count, (configuration_memory_buffer_t *) buffer);

}

    /** @brief Stream write to ACDI User (0xFB): delegates to user config_mem_write with low_address offset. */
static uint16_t _stream_write_request_acdi_user(openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer) {

    uint32_t config_address = address + node->parameters->address_space_acdi_user.low_address;

    return _config->config_mem_write(node, config_address, count, (configuration_memory_buffer_t *) buffer);

}

#ifdef OPENLCB_COMPILE_TRAIN

    /**
     * @brief Stream write to Train Fn Config (0xF9): map flat byte address to functions[].
     *
     * @details Each 16-bit function value occupies 2 bytes big-endian:
     * address/2 = fn_index, address%2 selects high/low byte.
     */
static uint16_t _stream_write_request_train_function_config_memory(openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer) {

    train_state_t *state = OpenLcbApplicationTrain_get_state(node);

    if (!state) {

        return 0;

    }

    for (uint16_t i = 0; i < count; i++) {

        uint32_t pos = address + i;
        uint16_t fn_index = (uint16_t) (pos / 2);
        uint8_t byte_sel = (uint8_t) (pos % 2);

        if (fn_index < USER_DEFINED_MAX_TRAIN_FUNCTIONS) {

            if (byte_sel == 0) {

                state->functions[fn_index] =
                        (state->functions[fn_index] & 0x00FF) | ((uint16_t) buffer[i] << 8);

            } else {

                state->functions[fn_index] =
                        (state->functions[fn_index] & 0xFF00) | buffer[i];

            }

        }

    }

    return count;

}

#endif /* OPENLCB_COMPILE_TRAIN */

    /** @brief Wires the config-mem-stream handler DI interface. */
static void _build_config_mem_stream_handler(void) {

    memset(&_config_mem_stream, 0, sizeof(_config_mem_stream));

#ifdef OPENLCB_COMPILE_CAN
    _config_mem_stream.send_openlcb_msg = &CanTxStatemachine_send_openlcb_message;
#endif
#ifdef OPENLCB_COMPILE_TCP
    _config_mem_stream.send_openlcb_msg = TcpConfig_get_send_openlcb_msg();
#endif

    _config_mem_stream.load_datagram_received_ok_message =
            &ProtocolDatagramHandler_load_datagram_received_ok_message;
    _config_mem_stream.load_datagram_received_rejected_message =
            &ProtocolDatagramHandler_load_datagram_rejected_message;

    _config_mem_stream.stream_initiate_outbound = &ProtocolStreamHandler_initiate_outbound;
    _config_mem_stream.stream_send_data         = &ProtocolStreamHandler_send_data;
    _config_mem_stream.stream_send_complete     = &ProtocolStreamHandler_send_complete;
    _config_mem_stream.stream_send_terminate    = &ProtocolStreamHandler_send_terminate;

    // Per-space read request callbacks
    _config_mem_stream.read_request_config_definition_info  = &_stream_read_request_config_definition_info;
    _config_mem_stream.read_request_all                     = &_stream_read_request_all;
    _config_mem_stream.read_request_configuration_memory    = &_stream_read_request_configuration_memory;
    _config_mem_stream.read_request_acdi_manufacturer       = &_stream_read_request_acdi_manufacturer;
    _config_mem_stream.read_request_acdi_user               = &_stream_read_request_acdi_user;

#ifdef OPENLCB_COMPILE_TRAIN
    _config_mem_stream.read_request_train_function_definition_info = &_stream_read_request_train_function_definition_info;
    _config_mem_stream.read_request_train_function_config_memory   = &_stream_read_request_train_function_config_memory;
#endif

    // Per-space write request callbacks
    _config_mem_stream.write_request_configuration_memory = &_stream_write_request_configuration_memory;
    _config_mem_stream.write_request_acdi_user            = &_stream_write_request_acdi_user;

#ifdef OPENLCB_COMPILE_TRAIN
    _config_mem_stream.write_request_train_function_config_memory = &_stream_write_request_train_function_config_memory;
#endif

    // User stream callbacks
    _config_mem_stream.user_on_initiate_request = _config->on_stream_initiate_request;
    _config_mem_stream.user_on_initiate_reply   = _config->on_stream_initiate_reply;
    _config_mem_stream.user_on_data_received    = _config->on_stream_data_received;
    _config_mem_stream.user_on_data_proceed     = _config->on_stream_data_proceed;
    _config_mem_stream.user_on_complete         = _config->on_stream_complete;

}

#endif /* OPENLCB_COMPILE_MEMORY_CONFIGURATION && !OPENLCB_COMPILE_BOOTLOADER */

#endif /* OPENLCB_COMPILE_STREAM */

    /** @brief Wires the user 100ms timer callback into the node interface struct. */
static void _build_node(void) {

    memset(&_node, 0, sizeof(_node));

    _node.on_100ms_timer_tick = _config->on_100ms_timer;

}

    /** @brief Wires event-state extraction helpers into the login message handler interface. */
static void _build_login_message_handler(void) {

    memset(&_login_msg, 0, sizeof(_login_msg));

    // Library-internal wiring -- event state extraction only needed when events compiled
#ifdef OPENLCB_COMPILE_EVENTS
    _login_msg.extract_producer_event_state_mti = &ProtocolEventTransport_extract_producer_event_status_mti;
    _login_msg.extract_consumer_event_state_mti = &ProtocolEventTransport_extract_consumer_event_status_mti;
#endif

}

    /** @brief Wires CAN send, node iteration, and login helpers into the login state machine interface. */
static void _build_login_statemachine(void) {

    memset(&_login_sm, 0, sizeof(_login_sm));

    // Direct transport send — login has its own inline sibling dispatch (Phase 2)
#ifdef OPENLCB_COMPILE_CAN
    _login_sm.send_openlcb_msg = &CanTxStatemachine_send_openlcb_message;
#endif
#ifdef OPENLCB_COMPILE_TCP
    _login_sm.send_openlcb_msg = TcpConfig_get_send_openlcb_msg();
#endif

    // Library-internal wiring
    _login_sm.openlcb_node_get_first          = &OpenLcbNode_get_first;
    _login_sm.openlcb_node_get_next           = &OpenLcbNode_get_next;
    _login_sm.openlcb_node_get_count          = &OpenLcbNode_get_count;
    _login_sm.process_main_statemachine       = &OpenLcbMainStatemachine_process_main_statemachine;
    _login_sm.load_initialization_complete    = &OpenLcbLoginStatemachineHandler_load_initialization_complete;
    _login_sm.load_producer_events            = &OpenLcbLoginStatemachineHandler_load_producer_event;
    _login_sm.load_consumer_events            = &OpenLcbLoginStatemachineHandler_load_consumer_event;
    _login_sm.process_login_statemachine      = &OpenLcbLoginStatemachine_process;
    _login_sm.handle_outgoing_openlcb_message = &OpenLcbLoginStatemachine_handle_outgoing_openlcb_message;
    _login_sm.handle_try_reenumerate          = &OpenLcbLoginStatemachine_handle_try_reenumerate;
    _login_sm.handle_try_enumerate_first_node = &OpenLcbLoginStatemachine_handle_try_enumerate_first_node;
    _login_sm.handle_try_enumerate_next_node  = &OpenLcbLoginStatemachine_handle_try_enumerate_next_node;

    // User callback
    _login_sm.on_login_complete = _config->on_login_complete;

}

    /** @brief Wires OIR/TDE application callbacks into the message network interface. */
static void _build_msg_network(void) {

    memset(&_msg_network, 0, sizeof(_msg_network));

    _msg_network.on_optional_interaction_rejected = _config->on_optional_interaction_rejected;
    _msg_network.on_terminate_due_to_error        = _config->on_terminate_due_to_error;

}

    /** @brief Wires the config memory read callback into the SNIP interface struct. */
static void _build_snip(void) {

    memset(&_snip, 0, sizeof(_snip));

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
    _snip.config_memory_read = _config->config_mem_read;
#endif

}

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION

    /** @brief Wires read callbacks, SNIP helpers, and address-space handlers into the config read interface. */
static void _build_config_mem_read(void) {

    memset(&_config_read, 0, sizeof(_config_read));

    // Library-internal wiring
    _config_read.load_datagram_received_ok_message       = &ProtocolDatagramHandler_load_datagram_received_ok_message;
    _config_read.load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message;
    _config_read.config_memory_read = _config->config_mem_read;

    // ACDI/SNIP support -- library standard implementations
    _config_read.snip_load_manufacturer_version_id = &ProtocolSnip_load_manufacturer_version_id;
    _config_read.snip_load_name                    = &ProtocolSnip_load_name;
    _config_read.snip_load_model                   = &ProtocolSnip_load_model;
    _config_read.snip_load_hardware_version        = &ProtocolSnip_load_hardware_version;
    _config_read.snip_load_software_version        = &ProtocolSnip_load_software_version;
    _config_read.snip_load_user_version_id         = &ProtocolSnip_load_user_version_id;
    _config_read.snip_load_user_name               = &ProtocolSnip_load_user_name;
    _config_read.snip_load_user_description        = &ProtocolSnip_load_user_description;

    // Address space read handlers
#ifndef OPENLCB_COMPILE_BOOTLOADER
    _config_read.read_request_config_definition_info = &ProtocolConfigMemReadHandler_read_request_config_definition_info;
    _config_read.read_request_config_mem = &ProtocolConfigMemReadHandler_read_request_config_mem;
    _config_read.read_request_acdi_manufacturer = &ProtocolConfigMemReadHandler_read_request_acdi_manufacturer;
    _config_read.read_request_acdi_user = &ProtocolConfigMemReadHandler_read_request_acdi_user;
#endif

    // Train profile: FDI + Function Config Memory read request handlers
#ifdef OPENLCB_COMPILE_TRAIN
    _config_read.read_request_train_function_config_definition_info = &ProtocolConfigMemReadHandler_read_request_train_function_definition_info;
    _config_read.read_request_train_function_config_memory = &ProtocolConfigMemReadHandler_read_request_train_function_config_memory;
    _config_read.get_train_state = &OpenLcbApplicationTrain_get_state;
#endif

    // User extension
    _config_read.delayed_reply_time = _config->config_mem_read_delayed_reply_time;

}

    /** @brief Wires write callbacks, firmware write, and address-space handlers into the config write interface. */
static void _build_config_mem_write(void) {

    memset(&_config_write, 0, sizeof(_config_write));

    _config_write.load_datagram_received_ok_message       = &ProtocolDatagramHandler_load_datagram_received_ok_message;
    _config_write.load_datagram_received_rejected_message = &ProtocolDatagramHandler_load_datagram_rejected_message;
    _config_write.config_memory_write                     = _config->config_mem_write;
    _config_write.config_memory_read                      = _config->config_mem_read;
#ifndef OPENLCB_COMPILE_BOOTLOADER
    _config_write.write_request_config_mem                = &ProtocolConfigMemWriteHandler_write_request_config_mem;
    _config_write.write_request_acdi_user                 =  &ProtocolConfigMemWriteHandler_write_request_acdi_user;
#endif

    // Train profile: Function Config Memory write request handler
    // Note: FDI (0xFA) write is intentionally NOT wired -- it is read-only.
#ifdef OPENLCB_COMPILE_TRAIN
    _config_write.write_request_train_function_config_memory = &ProtocolConfigMemWriteHandler_write_request_train_function_config_memory;
    _config_write.on_function_changed = _config->on_train_function_changed;
    _config_write.get_train_state = &OpenLcbApplicationTrain_get_state;
#endif

    // Firmware write (optional user callback)
#ifdef OPENLCB_COMPILE_FIRMWARE
    _config_write.write_request_firmware = _config->firmware_write;
#endif
    _config_write.delayed_reply_time = _config->config_mem_write_delayed_reply_time;

}

    /** @brief Wires operations commands (options, address space info, lock, reboot, factory reset) into the config ops interface. */
static void _build_config_mem_operations(void) {

    memset(&_config_ops, 0, sizeof(_config_ops));

    _config_ops.load_datagram_received_ok_message         = &ProtocolDatagramHandler_load_datagram_received_ok_message;
    _config_ops.load_datagram_received_rejected_message   = &ProtocolDatagramHandler_load_datagram_rejected_message;

    _config_ops.operations_request_options_cmd            = &ProtocolConfigMemOperationsHandler_request_options_cmd;
    _config_ops.operations_request_get_address_space_info = &ProtocolConfigMemOperationsHandler_request_get_address_space_info;
#ifndef OPENLCB_COMPILE_BOOTLOADER
    _config_ops.operations_request_reserve_lock           = &ProtocolConfigMemOperationsHandler_request_reserve_lock;
#endif

#ifdef OPENLCB_COMPILE_FIRMWARE
    _config_ops.cleanup_before_handoff                    = _config->cleanup_before_handoff;
    _config_ops.operations_request_freeze                 = _config->freeze;
    _config_ops.operations_request_unfreeze               = _config->unfreeze;
#endif
    _config_ops.operations_request_reset_reboot           = _config->reboot;
#ifndef OPENLCB_COMPILE_BOOTLOADER
    _config_ops.operations_request_factory_reset          = _config->factory_reset;
#endif

}

#endif /* OPENLCB_COMPILE_MEMORY_CONFIGURATION */

#ifdef OPENLCB_COMPILE_DATAGRAMS

    /** @brief Wires lock callbacks, address-space dispatchers, and operations handlers into the datagram interface. */
static void _build_datagram_handler(void) {

    memset(&_datagram, 0, sizeof(_datagram));

    _datagram.lock_shared_resources   = _config->lock_shared_resources;
    _datagram.unlock_shared_resources = _config->unlock_shared_resources;

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION

#ifndef OPENLCB_COMPILE_BOOTLOADER
    // Read address spaces -- standard library implementations
    _datagram.memory_read_space_config_description_info = &ProtocolConfigMemReadHandler_read_space_config_description_info;
    _datagram.memory_read_space_all                     = &ProtocolConfigMemReadHandler_read_space_all;
    _datagram.memory_read_space_configuration_memory    = &ProtocolConfigMemReadHandler_read_space_config_memory;
    _datagram.memory_read_space_acdi_manufacturer       = &ProtocolConfigMemReadHandler_read_space_acdi_manufacturer;
    _datagram.memory_read_space_acdi_user               = &ProtocolConfigMemReadHandler_read_space_acdi_user;

    // Train profile: FDI + Function Config Memory read spaces
#ifdef OPENLCB_COMPILE_TRAIN
    _datagram.memory_read_space_train_function_definition_info = &ProtocolConfigMemReadHandler_read_space_train_function_definition_info;
    _datagram.memory_read_space_train_function_config_memory   = &ProtocolConfigMemReadHandler_read_space_train_function_config_memory;
#endif

    // Write address spaces
    _datagram.memory_write_space_configuration_memory = &ProtocolConfigMemWriteHandler_write_space_config_memory;
    _datagram.memory_write_space_acdi_user            = &ProtocolConfigMemWriteHandler_write_space_acdi_user;
#endif /* OPENLCB_COMPILE_BOOTLOADER */

#ifdef OPENLCB_COMPILE_FIRMWARE
    _datagram.memory_write_space_firmware_upgrade     = &ProtocolConfigMemWriteHandler_write_space_firmware;
#endif

#ifndef OPENLCB_COMPILE_BOOTLOADER
    // Train profile: Function Config Memory write space
#ifdef OPENLCB_COMPILE_TRAIN
    _datagram.memory_write_space_train_function_config_memory = &ProtocolConfigMemWriteHandler_write_space_train_function_config_memory;
#endif
#endif /* OPENLCB_COMPILE_BOOTLOADER */

    // Operations commands -- bootloader needs options, address space info, freeze/unfreeze, reset/reboot
    _datagram.memory_options_cmd                                = &ProtocolConfigMemOperationsHandler_options_cmd;
    _datagram.memory_options_reply                              = &ProtocolConfigMemOperationsHandler_options_reply;
    _datagram.memory_get_address_space_info                     = &ProtocolConfigMemOperationsHandler_get_address_space_info;
    _datagram.memory_get_address_space_info_reply_not_present   = &ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present;
    _datagram.memory_get_address_space_info_reply_present       = &ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present;
    _datagram.memory_unfreeze                                   = &ProtocolConfigMemOperationsHandler_unfreeze;
    _datagram.memory_freeze                                     = &ProtocolConfigMemOperationsHandler_freeze;
    _datagram.memory_reset_reboot                               = &ProtocolConfigMemOperationsHandler_reset_reboot;

#ifndef OPENLCB_COMPILE_BOOTLOADER
    _datagram.memory_reserve_lock                               = &ProtocolConfigMemOperationsHandler_reserve_lock;
    _datagram.memory_reserve_lock_reply                         = &ProtocolConfigMemOperationsHandler_reserve_lock_reply;
    _datagram.memory_get_unique_id                              = &ProtocolConfigMemOperationsHandler_get_unique_id;
    _datagram.memory_get_unique_id_reply                        = &ProtocolConfigMemOperationsHandler_get_unique_id_reply;
    _datagram.memory_update_complete                            = &ProtocolConfigMemOperationsHandler_update_complete;
    _datagram.memory_factory_reset                              = &ProtocolConfigMemOperationsHandler_factory_reset;

    // Write-under-mask address spaces
    _datagram.memory_write_under_mask_space_config_description_info      = &ProtocolConfigMemWriteHandler_write_under_mask_space_config_description_info;
    _datagram.memory_write_under_mask_space_all                          = &ProtocolConfigMemWriteHandler_write_under_mask_space_all;
    _datagram.memory_write_under_mask_space_configuration_memory         = &ProtocolConfigMemWriteHandler_write_under_mask_space_config_memory;
    _datagram.memory_write_under_mask_space_acdi_manufacturer            = &ProtocolConfigMemWriteHandler_write_under_mask_space_acdi_manufacturer;
    _datagram.memory_write_under_mask_space_acdi_user                    = &ProtocolConfigMemWriteHandler_write_under_mask_space_acdi_user;
    _datagram.memory_write_under_mask_space_train_function_definition_info = &ProtocolConfigMemWriteHandler_write_under_mask_space_train_function_definition_info;
    _datagram.memory_write_under_mask_space_train_function_config_memory = &ProtocolConfigMemWriteHandler_write_under_mask_space_train_function_config_memory;
    _datagram.memory_write_under_mask_space_firmware_upgrade             = &ProtocolConfigMemWriteHandler_write_under_mask_space_firmware;

    // Stream-transport read handlers
#ifdef OPENLCB_COMPILE_STREAM
    _datagram.memory_read_stream_space_config_description_info =
            &ProtocolConfigMemStreamHandler_handle_read_stream_space_config_description_info;
    _datagram.memory_read_stream_space_all =
            &ProtocolConfigMemStreamHandler_handle_read_stream_space_all;
    _datagram.memory_read_stream_space_configuration_memory =
            &ProtocolConfigMemStreamHandler_handle_read_stream_space_configuration_memory;
    _datagram.memory_read_stream_space_acdi_manufacturer =
            &ProtocolConfigMemStreamHandler_handle_read_stream_space_acdi_manufacturer;
    _datagram.memory_read_stream_space_acdi_user =
            &ProtocolConfigMemStreamHandler_handle_read_stream_space_acdi_user;

#ifdef OPENLCB_COMPILE_TRAIN
    _datagram.memory_read_stream_space_train_function_definition_info =
            &ProtocolConfigMemStreamHandler_handle_read_stream_space_train_function_definition_info;
    _datagram.memory_read_stream_space_train_function_config_memory =
            &ProtocolConfigMemStreamHandler_handle_read_stream_space_train_function_config_memory;
#endif /* OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_FIRMWARE
    // Firmware stream read intentionally not wired -- firmware space is write-only for upgrades
#endif /* OPENLCB_COMPILE_FIRMWARE */

    // Stream-transport write handlers
    _datagram.memory_write_stream_space_config_description_info =
            &ProtocolConfigMemStreamHandler_handle_write_stream_space_config_description_info;
    _datagram.memory_write_stream_space_all =
            &ProtocolConfigMemStreamHandler_handle_write_stream_space_all;
    _datagram.memory_write_stream_space_configuration_memory =
            &ProtocolConfigMemStreamHandler_handle_write_stream_space_configuration_memory;
    _datagram.memory_write_stream_space_acdi_manufacturer =
            &ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_manufacturer;
    _datagram.memory_write_stream_space_acdi_user =
            &ProtocolConfigMemStreamHandler_handle_write_stream_space_acdi_user;

#ifdef OPENLCB_COMPILE_TRAIN
    _datagram.memory_write_stream_space_train_function_definition_info =
            &ProtocolConfigMemStreamHandler_handle_write_stream_space_train_function_definition_info;
    _datagram.memory_write_stream_space_train_function_config_memory =
            &ProtocolConfigMemStreamHandler_handle_write_stream_space_train_function_config_memory;
#endif /* OPENLCB_COMPILE_TRAIN */

#ifdef OPENLCB_COMPILE_FIRMWARE
    _datagram.memory_write_stream_space_firmware_upgrade =
            &ProtocolConfigMemStreamHandler_handle_write_stream_space_firmware;
#endif /* OPENLCB_COMPILE_FIRMWARE */

#endif /* OPENLCB_COMPILE_STREAM */

#endif /* OPENLCB_COMPILE_BOOTLOADER */

#endif /* OPENLCB_COMPILE_MEMORY_CONFIGURATION */

    // All remaining fields stay NULL (unimplemented stream spaces, reply handlers, etc.)

}

#endif /* OPENLCB_COMPILE_DATAGRAMS */

    /** @brief Wires all protocol handlers into the main state machine dispatch interface. */
static void _build_main_statemachine(void) {

    memset(&_main_sm, 0, sizeof(_main_sm));

    // Hardware bindings
    _main_sm.lock_shared_resources   = _config->lock_shared_resources;
    _main_sm.unlock_shared_resources = _config->unlock_shared_resources;
#ifdef OPENLCB_COMPILE_CAN
    _main_sm.send_openlcb_msg        = &CanTxStatemachine_send_openlcb_message;
#endif
#ifdef OPENLCB_COMPILE_TCP
    _main_sm.send_openlcb_msg        = TcpConfig_get_send_openlcb_msg();
#endif

    // Clock access (injected to maintain decoupling)
    _main_sm.get_current_tick = &OpenLcbConfig_get_global_100ms_tick;

    // Library-internal wiring -- always the same
    _main_sm.openlcb_node_get_first    = &OpenLcbNode_get_first;
    _main_sm.openlcb_node_get_next     = &OpenLcbNode_get_next;
    _main_sm.openlcb_node_is_last      = &OpenLcbNode_is_last;
    _main_sm.openlcb_node_get_count    = &OpenLcbNode_get_count;
    _main_sm.load_interaction_rejected = &OpenLcbMainStatemachine_load_interaction_rejected;

    // Required Message Network handlers
    _main_sm.message_network_initialization_complete = &ProtocolMessageNetwork_handle_initialization_complete;
    _main_sm.message_network_initialization_complete_simple = &ProtocolMessageNetwork_handle_initialization_complete_simple;
    _main_sm.message_network_verify_node_id_addressed =&ProtocolMessageNetwork_handle_verify_node_id_addressed;
    _main_sm.message_network_verify_node_id_global =&ProtocolMessageNetwork_handle_verify_node_id_global;
    _main_sm.message_network_verified_node_id =&ProtocolMessageNetwork_handle_verified_node_id;
    _main_sm.message_network_optional_interaction_rejected = &ProtocolMessageNetwork_handle_optional_interaction_rejected;
    _main_sm.message_network_terminate_due_to_error  = &ProtocolMessageNetwork_handle_terminate_due_to_error;

    // Required PIP handlers
    _main_sm.message_network_protocol_support_inquiry = &ProtocolMessageNetwork_handle_protocol_support_inquiry;
    _main_sm.message_network_protocol_support_reply = &ProtocolMessageNetwork_handle_protocol_support_reply;

    // Required internal handlers (for testability)
    _main_sm.process_main_statemachine                    = &OpenLcbMainStatemachine_process_main_statemachine;
    _main_sm.does_node_process_msg                        = &OpenLcbMainStatemachine_does_node_process_msg;
    _main_sm.handle_outgoing_openlcb_message              = &OpenLcbMainStatemachine_handle_outgoing_openlcb_message;
    _main_sm.handle_try_reenumerate                       = &OpenLcbMainStatemachine_handle_try_reenumerate;
    _main_sm.handle_try_pop_next_incoming_openlcb_message = &OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message;
    _main_sm.handle_try_enumerate_first_node              = &OpenLcbMainStatemachine_handle_try_enumerate_first_node;
    _main_sm.handle_try_enumerate_next_node               = &OpenLcbMainStatemachine_handle_try_enumerate_next_node;

    // SNIP -- always enabled (part of every profile)
    _main_sm.snip_simple_node_info_request = &ProtocolSnip_handle_simple_node_info_request;
    _main_sm.snip_simple_node_info_reply   = &ProtocolSnip_handle_simple_node_info_reply;

#ifdef OPENLCB_COMPILE_EVENTS
    _main_sm.event_transport_consumer_identify            = &ProtocolEventTransport_handle_consumer_identify;
    _main_sm.event_transport_consumer_range_identified    = &ProtocolEventTransport_handle_consumer_range_identified;
    _main_sm.event_transport_consumer_identified_unknown  = &ProtocolEventTransport_handle_consumer_identified_unknown;
    _main_sm.event_transport_consumer_identified_set      = &ProtocolEventTransport_handle_consumer_identified_set;
    _main_sm.event_transport_consumer_identified_clear    = &ProtocolEventTransport_handle_consumer_identified_clear;
    _main_sm.event_transport_consumer_identified_reserved = &ProtocolEventTransport_handle_consumer_identified_reserved;
    _main_sm.event_transport_producer_identify            = &ProtocolEventTransport_handle_producer_identify;
    _main_sm.event_transport_producer_range_identified    = &ProtocolEventTransport_handle_producer_range_identified;
    _main_sm.event_transport_producer_identified_unknown  = &ProtocolEventTransport_handle_producer_identified_unknown;
    _main_sm.event_transport_producer_identified_set      = &ProtocolEventTransport_handle_producer_identified_set;
    _main_sm.event_transport_producer_identified_clear    = &ProtocolEventTransport_handle_producer_identified_clear;
    _main_sm.event_transport_producer_identified_reserved = &ProtocolEventTransport_handle_producer_identified_reserved;
    _main_sm.event_transport_identify_dest                = &ProtocolEventTransport_handle_events_identify_dest;
    _main_sm.event_transport_identify                     = &ProtocolEventTransport_handle_events_identify;
    _main_sm.event_transport_learn                        = &ProtocolEventTransport_handle_event_learn;
    _main_sm.event_transport_pc_report                    = &ProtocolEventTransport_handle_pc_event_report;
    _main_sm.event_transport_pc_report_with_payload       = &ProtocolEventTransport_handle_pc_event_report_with_payload;
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
    _main_sm.broadcast_time_event_handler = &ProtocolBroadcastTimeHandler_handle_time_event;
    _main_sm.is_broadcast_time_event      = &ProtocolBroadcastTimeHandler_is_time_event;
#endif

#ifdef OPENLCB_COMPILE_DATAGRAMS
    _main_sm.datagram                = &ProtocolDatagramHandler_datagram;
    _main_sm.datagram_ok_reply       = &ProtocolDatagramHandler_datagram_received_ok;
    _main_sm.datagram_rejected_reply = &ProtocolDatagramHandler_datagram_rejected;
    _main_sm.load_datagram_rejected  = &ProtocolDatagramHandler_load_datagram_rejected_message;
#endif

#ifdef OPENLCB_COMPILE_TRAIN
    _main_sm.train_control_command         = &ProtocolTrainHandler_handle_train_command;
    _main_sm.train_control_reply           = &ProtocolTrainHandler_handle_train_reply;
    _main_sm.train_emergency_event_handler = &ProtocolTrainHandler_handle_emergency_event;
    _main_sm.is_emergency_event            = &ProtocolTrainHandler_is_emergency_event;
#endif

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
    _main_sm.train_search_event_handler    = &ProtocolTrainSearchHandler_handle_search_event;
    _main_sm.train_search_no_match_handler = &ProtocolTrainSearchHandler_handle_search_no_match;
    _main_sm.train_search_reply_handler    = &ProtocolTrainSearchHandler_handle_search_reply;
    _main_sm.is_train_search_event         = &ProtocolTrainSearchHandler_is_search_event;
#endif

#ifdef OPENLCB_COMPILE_STREAM
    _main_sm.stream_initiate_request        = &ProtocolStreamHandler_initiate_request;
    _main_sm.stream_initiate_reply          = &ProtocolStreamHandler_initiate_reply;
    _main_sm.stream_send_data               = &ProtocolStreamHandler_data_send;
    _main_sm.stream_data_proceed            = &ProtocolStreamHandler_data_proceed;
    _main_sm.stream_data_complete           = &ProtocolStreamHandler_data_complete;
    _main_sm.stream_terminate_due_to_error  = &ProtocolStreamHandler_handle_terminate_due_to_error;
#endif

}

    /** @brief Wires the CAN send function and config memory callbacks into the application interface. */
static void _build_application(void) {

    memset(&_app, 0, sizeof(_app));

    _app.send_openlcb_msg    = &OpenLcbMainStatemachine_send_with_sibling_dispatch;

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
    _app.config_memory_read  = _config->config_mem_read;
    _app.config_memory_write = _config->config_mem_write;
#endif

}

// ---- Public API ----

    /**
    * @brief Initializes the entire OpenLCB stack from the user configuration.
    *
    * @details Algorithm:
    * -# Store config pointer
    * -# Initialize buffer infrastructure (store, list, FIFO)
    * -# Build all internal interface structs from user config and compile flags
    * -# Initialize all compiled-in protocol modules in dependency order
    *
    * @verbatim
    * @param config Pointer to the @ref openlcb_config_t to use
    * @endverbatim
    */
void OpenLcbConfig_initialize(const openlcb_config_t *config) {

    _config = config;

    // 1. Buffer infrastructure -- always needed
    OpenLcbBufferStore_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbBufferFifo_initialize();

    // 2. Build all internal interface structs from user config
    _build_node();
    _build_login_message_handler();
    _build_login_statemachine();
    _build_application();
    _build_snip();
    _build_msg_network();

#ifdef OPENLCB_COMPILE_EVENTS
    _build_event_transport();
#endif

#ifdef OPENLCB_COMPILE_DATAGRAMS
    _build_datagram_handler();
#endif

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
    _build_config_mem_read();
    _build_config_mem_write();
    _build_config_mem_operations();
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
    _build_broadcast_time();
    _build_app_broadcast_time();
#endif

#ifdef OPENLCB_COMPILE_TRAIN
    _build_train_handler();
    _build_app_train();
#endif

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
    _build_train_search_handler();
#endif

#ifdef OPENLCB_COMPILE_STREAM
    _build_stream_handler();
#if defined(OPENLCB_COMPILE_MEMORY_CONFIGURATION) && !defined(OPENLCB_COMPILE_BOOTLOADER)
    _build_config_mem_stream_handler();
#endif
#endif

    _build_main_statemachine();

    // 3. Initialize modules in dependency order
    ProtocolSnip_initialize(&_snip);

#ifdef OPENLCB_COMPILE_DATAGRAMS
    ProtocolDatagramHandler_initialize(&_datagram);
#endif

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#ifndef OPENLCB_COMPILE_BOOTLOADER
    ProtocolConfigMemReadHandler_initialize(&_config_read);
#endif
    ProtocolConfigMemWriteHandler_initialize(&_config_write);
    ProtocolConfigMemOperationsHandler_initialize(&_config_ops);
#endif

#ifdef OPENLCB_COMPILE_EVENTS
    ProtocolEventTransport_initialize(&_event_transport);
#endif

    ProtocolMessageNetwork_initialize(&_msg_network);

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
    ProtocolBroadcastTimeHandler_initialize(&_broadcast_time);
    OpenLcbApplicationBroadcastTime_initialize(&_app_broadcast_time);
#endif

#ifdef OPENLCB_COMPILE_TRAIN
    ProtocolTrainHandler_initialize(&_train_handler);
    OpenLcbApplicationTrain_initialize(&_app_train);
#endif

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
    ProtocolTrainSearchHandler_initialize(&_train_search);
#endif

#ifdef OPENLCB_COMPILE_STREAM
    ProtocolStreamHandler_initialize(&_stream_handler);
#if defined(OPENLCB_COMPILE_MEMORY_CONFIGURATION) && !defined(OPENLCB_COMPILE_BOOTLOADER)
    ProtocolConfigMemStreamHandler_initialize(&_config_mem_stream);
#endif
#endif

    OpenLcbNode_initialize(&_node);

    OpenLcbLoginStatemachineHandler_initialize(&_login_msg);
    OpenLcbLoginStatemachine_initialize(&_login_sm);
    OpenLcbMainStatemachine_initialize(&_main_sm);

    OpenLcbApplication_initialize(&_app);

}

    /**
    * @brief Allocates a node slot and assigns its ID and parameters.
    *
    * @verbatim
    * @param node_id     Unique 48-bit @ref node_id_t for the new node
    * @param parameters  Pointer to @ref node_parameters_t (SNIP, protocol flags, events)
    * @endverbatim
    *
    * @return Pointer to the allocated @ref openlcb_node_t, or NULL if no slots available
    */
openlcb_node_t *OpenLcbConfig_create_node(node_id_t node_id, const node_parameters_t *parameters) {

    return OpenLcbNode_allocate(node_id, parameters);

}

    /**
     * @brief Runs all periodic service tasks from the main loop.
     *
     * @details Reads the global clock once and passes it to each module.
     * All work happens in the main loop context where it is safe to send
     * messages, free buffers, and call application callbacks.
     */
static void _run_periodic_services(void) {

    uint8_t tick = _global_100ms_tick;

    OpenLcbNode_100ms_timer_tick(tick);

#ifdef OPENLCB_COMPILE_DATAGRAMS
    ProtocolDatagramHandler_100ms_timer_tick(tick);
    ProtocolDatagramHandler_check_timeouts(tick);
#endif

#ifdef OPENLCB_COMPILE_STREAM
#if defined(OPENLCB_COMPILE_MEMORY_CONFIGURATION) && !defined(OPENLCB_COMPILE_BOOTLOADER)
    ProtocolConfigMemStreamHandler_check_timeouts(tick);
#endif
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
    OpenLcbApplicationBroadcastTime_100ms_time_tick(tick);
#endif

#ifdef OPENLCB_COMPILE_TRAIN
    OpenLcbApplicationTrain_100ms_timer_tick(tick);
#endif

}

    /** @brief Runs one iteration of all state machines and periodic services. */
void OpenLcbConfig_run(void) {

#ifdef OPENLCB_COMPILE_CAN
    CanMainStatemachine_run();
#endif
    OpenLcbLoginStatemachine_run();
    OpenLcbMainStatemachine_run();

#ifdef OPENLCB_COMPILE_STREAM
#if defined(OPENLCB_COMPILE_MEMORY_CONFIGURATION) && !defined(OPENLCB_COMPILE_BOOTLOADER)
    ProtocolConfigMemStreamHandler_run();
#endif
#endif

    _run_periodic_services();

}

    /** @brief Increments the global 100ms tick counter. This is the ONLY action
     *  performed by the timer interrupt — all real work runs in the main loop. */
void OpenLcbConfig_100ms_timer_tick(void) {

    _global_100ms_tick++;

}

// =============================================================================
// GTEST access wrappers -- expose static callbacks for unit-test coverage
// =============================================================================

#ifdef GTEST
#if defined(OPENLCB_COMPILE_STREAM) && defined(OPENLCB_COMPILE_MEMORY_CONFIGURATION) && !defined(OPENLCB_COMPILE_BOOTLOADER)

uint16_t OpenLcbConfigTest_stream_read_request_config_definition_info(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    return _stream_read_request_config_definition_info(node, address, count, buffer);

}

uint16_t OpenLcbConfigTest_stream_read_request_all(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    return _stream_read_request_all(node, address, count, buffer);

}

uint16_t OpenLcbConfigTest_stream_read_request_configuration_memory(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    return _stream_read_request_configuration_memory(node, address, count, buffer);

}

uint16_t OpenLcbConfigTest_read_field_array(const _config_field_t *fields,
        uint16_t num_fields, uint32_t address, uint16_t count, uint8_t *buffer) {

    return _read_field_array(fields, num_fields, address, count, buffer);

}

uint16_t OpenLcbConfigTest_stream_read_request_acdi_manufacturer(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    return _stream_read_request_acdi_manufacturer(node, address, count, buffer);

}

uint16_t OpenLcbConfigTest_stream_read_request_acdi_user(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    return _stream_read_request_acdi_user(node, address, count, buffer);

}

#ifdef OPENLCB_COMPILE_TRAIN

uint16_t OpenLcbConfigTest_stream_read_request_train_function_definition_info(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    return _stream_read_request_train_function_definition_info(node, address, count, buffer);

}

uint16_t OpenLcbConfigTest_stream_read_request_train_function_config_memory(openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer) {

    return _stream_read_request_train_function_config_memory(node, address, count, buffer);

}

#endif /* OPENLCB_COMPILE_TRAIN */

uint16_t OpenLcbConfigTest_stream_write_request_configuration_memory(openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer) {

    return _stream_write_request_configuration_memory(node, address, count, buffer);

}

uint16_t OpenLcbConfigTest_stream_write_request_acdi_user(openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer) {

    return _stream_write_request_acdi_user(node, address, count, buffer);

}

#ifdef OPENLCB_COMPILE_TRAIN

uint16_t OpenLcbConfigTest_stream_write_request_train_function_config_memory(openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer) {

    return _stream_write_request_train_function_config_memory(node, address, count, buffer);

}

#endif /* OPENLCB_COMPILE_TRAIN */

#endif /* OPENLCB_COMPILE_STREAM && OPENLCB_COMPILE_MEMORY_CONFIGURATION && !OPENLCB_COMPILE_BOOTLOADER */
#endif /* GTEST */
