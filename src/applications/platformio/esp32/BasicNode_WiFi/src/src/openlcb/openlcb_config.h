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
 * @file openlcb_config.h
 * @brief User-facing configuration struct and initialization API for OpenLcbCLib.
 *
 * @details Users populate one @ref openlcb_config_t struct with hardware driver
 * functions and optional application callbacks, then call OpenLcb_initialize()
 * to bring up the entire stack.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __OPENLCB_CONFIG__
#define __OPENLCB_CONFIG__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

// =============================================================================
// Compile-time feature dependency validation
// =============================================================================

#if defined(OPENLCB_COMPILE_CONFIG_MEMORY) && !defined(OPENLCB_COMPILE_DATAGRAMS)
#error "OPENLCB_COMPILE_CONFIG_MEMORY requires OPENLCB_COMPILE_DATAGRAMS"
#endif

#if defined(OPENLCB_COMPILE_BROADCAST_TIME) && !defined(OPENLCB_COMPILE_EVENTS)
#error "OPENLCB_COMPILE_BROADCAST_TIME requires OPENLCB_COMPILE_EVENTS"
#endif

#if defined(OPENLCB_COMPILE_TRAIN_SEARCH) && !defined(OPENLCB_COMPILE_EVENTS)
#error "OPENLCB_COMPILE_TRAIN_SEARCH requires OPENLCB_COMPILE_EVENTS"
#endif

#if defined(OPENLCB_COMPILE_TRAIN_SEARCH) && !defined(OPENLCB_COMPILE_TRAIN)
#error "OPENLCB_COMPILE_TRAIN_SEARCH requires OPENLCB_COMPILE_TRAIN"
#endif

// =============================================================================
// No-features warning
// =============================================================================

#if !defined(OPENLCB_COMPILE_EVENTS) && !defined(OPENLCB_COMPILE_DATAGRAMS) && \
    !defined(OPENLCB_COMPILE_TRAIN)
#warning "No optional features enabled. This node will only support SNIP identification."
#endif

// =============================================================================
// Buffer sanity check
// =============================================================================

#if (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + \
     USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_STREAM_BUFFER_DEPTH) > 126
#error "Total buffer count exceeds 126 — reduce buffer depths for 8-bit targets"
#endif

// =============================================================================
// Verbose compile-time summary (opt-in via OPENLCB_COMPILE_VERBOSE)
// =============================================================================

#ifdef OPENLCB_COMPILE_VERBOSE

#ifdef OPENLCB_COMPILE_EVENTS
#pragma message "OpenLcbCLib: EVENTS = ON"
#else
#pragma message "OpenLcbCLib: EVENTS = OFF"
#endif

#ifdef OPENLCB_COMPILE_DATAGRAMS
#pragma message "OpenLcbCLib: DATAGRAMS = ON"
#else
#pragma message "OpenLcbCLib: DATAGRAMS = OFF"
#endif

#ifdef OPENLCB_COMPILE_CONFIG_MEMORY
#pragma message "OpenLcbCLib: CONFIG_MEMORY = ON"
#else
#pragma message "OpenLcbCLib: CONFIG_MEMORY = OFF"
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
#pragma message "OpenLcbCLib: BROADCAST_TIME = ON"
#else
#pragma message "OpenLcbCLib: BROADCAST_TIME = OFF"
#endif

#ifdef OPENLCB_COMPILE_TRAIN
#pragma message "OpenLcbCLib: TRAIN = ON"
#else
#pragma message "OpenLcbCLib: TRAIN = OFF"
#endif

#ifdef OPENLCB_COMPILE_TRAIN_SEARCH
#pragma message "OpenLcbCLib: TRAIN_SEARCH = ON"
#else
#pragma message "OpenLcbCLib: TRAIN_SEARCH = OFF"
#endif

#endif /* OPENLCB_COMPILE_VERBOSE */

    /**
     * @brief User configuration for OpenLcbCLib.
     *
     * @details Populate this struct with hardware driver functions and optional
     * application callbacks, then pass it to OpenLcb_initialize(). Required
     * fields are marked REQUIRED and must be non-NULL.
     *
     * @code
     * static const openlcb_config_t my_config = {
     *     .lock_shared_resources   = &MyDriver_lock,
     *     .unlock_shared_resources = &MyDriver_unlock,
     *     .config_mem_read         = &MyDriver_eeprom_read,
     *     .config_mem_write        = &MyDriver_eeprom_write,
     *     .reboot                  = &MyDriver_reboot,
     *     .on_login_complete       = &my_login_handler,
     *     .on_consumed_event_pcer  = &my_event_handler,
     * };
     * OpenLcb_initialize(&my_config);
     * @endcode
     */
typedef struct {

    // =========================================================================
    // REQUIRED: Hardware Driver Functions
    // =========================================================================

        /** @brief Disable interrupts / acquire mutex for shared resource access. REQUIRED. */
    void (*lock_shared_resources)(void);

        /** @brief Re-enable interrupts / release mutex. REQUIRED. */
    void (*unlock_shared_resources)(void);

        /**
         * @brief Read from configuration memory (EEPROM/Flash/file). REQUIRED.
         *
         * @param openlcb_node The @ref openlcb_node_t requesting the read
         * @param address Starting address in configuration memory
         * @param count Number of bytes to read
         * @param buffer Destination @ref configuration_memory_buffer_t
         *
         * @return Number of bytes actually read
         */
    uint16_t (*config_mem_read)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

        /**
         * @brief Write to configuration memory (EEPROM/Flash/file). REQUIRED.
         *
         * @param openlcb_node The @ref openlcb_node_t requesting the write
         * @param address Starting address in configuration memory
         * @param count Number of bytes to write
         * @param buffer Source @ref configuration_memory_buffer_t
         *
         * @return Number of bytes actually written
         */
    uint16_t (*config_mem_write)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

        /**
         * @brief Reboot the processor. REQUIRED.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_operations_request_info @ref config_mem_operations_request_info_t context
         */
    void (*reboot)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

#ifdef OPENLCB_COMPILE_CONFIG_MEMORY

    // =========================================================================
    // OPTIONAL: Hardware Driver Extensions (requires CONFIG_MEMORY)
    // =========================================================================

        /**
         * @brief Freeze the node for firmware upgrade. Optional.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_operations_request_info @ref config_mem_operations_request_info_t context
         */
    void (*freeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Unfreeze the node after firmware upgrade. Optional.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_operations_request_info @ref config_mem_operations_request_info_t context
         */
    void (*unfreeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Write firmware data during upgrade. Optional.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_write_request_info @ref config_mem_write_request_info_t context
         */
    void (*firmware_write)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

        /**
         * @brief Factory reset -- erase user config and restore defaults. Optional.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_operations_request_info @ref config_mem_operations_request_info_t context
         */
    void (*factory_reset)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Return delayed reply time flag for config memory reads. Optional.
         *
         * @details Return 0 for no delay, or (0x80 | N) for 2^N second reply pending.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_read_request_info @ref config_mem_read_request_info_t context
         *
         * @return Delay flag byte
         */
    uint16_t (*config_mem_read_delayed_reply_time)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Return delayed reply time flag for config memory writes. Optional.
         *
         * @param statemachine_info @ref openlcb_statemachine_info_t context
         * @param config_mem_write_request_info @ref config_mem_write_request_info_t context
         *
         * @return Delay flag byte
         */
    uint16_t (*config_mem_write_delayed_reply_time)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

#endif /* OPENLCB_COMPILE_CONFIG_MEMORY */

    // =========================================================================
    // OPTIONAL: Core Application Callbacks
    // =========================================================================

        /** @brief 100ms periodic timer callback. Optional. */
    void (*on_100ms_timer)(void);

        /** @brief Called when a node completes login and enters RUN state. Optional.
         *  Return true if login can complete, false to delay. */
    bool (*on_login_complete)(openlcb_node_t *openlcb_node);

#ifdef OPENLCB_COMPILE_EVENTS

    // =========================================================================
    // OPTIONAL: Event Transport Callbacks (requires EVENTS)
    // =========================================================================

        /** @brief An event this node consumes was identified on the network. Optional. */
    void (*on_consumed_event_identified)(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_status_enum status, event_payload_t *payload);

        /** @brief A PC Event Report was received for a consumed event. Optional.
         *  This is the primary "an event happened" notification. */
    void (*on_consumed_event_pcer)(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_payload_t *payload);

        /** @brief Event learn/teach message received. Optional. */
    void (*on_event_learn)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Range Identified received. Optional. */
    void (*on_consumer_range_identified)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Identified Unknown received. Optional. */
    void (*on_consumer_identified_unknown)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Identified Set received. Optional. */
    void (*on_consumer_identified_set)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Identified Clear received. Optional. */
    void (*on_consumer_identified_clear)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Identified Reserved received. Optional. */
    void (*on_consumer_identified_reserved)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Range Identified received. Optional. */
    void (*on_producer_range_identified)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Identified Unknown received. Optional. */
    void (*on_producer_identified_unknown)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Identified Set received. Optional. */
    void (*on_producer_identified_set)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Identified Clear received. Optional. */
    void (*on_producer_identified_clear)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Identified Reserved received. Optional. */
    void (*on_producer_identified_reserved)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief PC Event Report received (unfiltered -- any event). Optional. */
    void (*on_pc_event_report)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief PC Event Report with payload received. Optional. */
    void (*on_pc_event_report_with_payload)(openlcb_node_t *openlcb_node, event_id_t *event_id, uint16_t count, event_payload_t *payload);

#endif /* OPENLCB_COMPILE_EVENTS */

#ifdef OPENLCB_COMPILE_BROADCAST_TIME

    // =========================================================================
    // OPTIONAL: Broadcast Time Callbacks (requires BROADCAST_TIME)
    // =========================================================================

        /** @brief Broadcast time changed (clock minute advanced). Optional. */
    void (*on_broadcast_time_changed)(broadcast_clock_t *clock);

        /** @brief Time event received from clock generator. Optional. */
    void (*on_broadcast_time_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Date event received from clock generator. Optional. */
    void (*on_broadcast_date_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Year event received from clock generator. Optional. */
    void (*on_broadcast_year_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Rate event received from clock generator. Optional. */
    void (*on_broadcast_rate_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Clock started event received. Optional. */
    void (*on_broadcast_clock_started)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Clock stopped event received. Optional. */
    void (*on_broadcast_clock_stopped)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

        /** @brief Date rollover event received. Optional. */
    void (*on_broadcast_date_rollover)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

#endif /* OPENLCB_COMPILE_BROADCAST_TIME */

#ifdef OPENLCB_COMPILE_TRAIN

    // =========================================================================
    // OPTIONAL: Train Control Callbacks (requires TRAIN)
    // All are optional (NULL = use handler defaults).
    // Notifiers fire AFTER state is updated. Decision callbacks return a value.
    // =========================================================================

    // ---- Train-node side: notifiers (fire after state updated) ----

        /** @brief Speed was set on this train node. State already updated. */
    void (*on_train_speed_changed)(openlcb_node_t *openlcb_node, uint16_t speed_float16);

        /** @brief Function was set on this train node. */
    void (*on_train_function_changed)(openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value);

        /** @brief An emergency state was entered. State flags already updated. */
    void (*on_train_emergency_entered)(openlcb_node_t *openlcb_node, train_emergency_type_enum emergency_type);

        /** @brief An emergency state was exited. State flags already updated. */
    void (*on_train_emergency_exited)(openlcb_node_t *openlcb_node, train_emergency_type_enum emergency_type);

        /** @brief Controller was assigned or changed. State already updated. */
    void (*on_train_controller_assigned)(openlcb_node_t *openlcb_node, node_id_t controller_node_id);

        /** @brief Controller was released. State already cleared. */
    void (*on_train_controller_released)(openlcb_node_t *openlcb_node);

        /** @brief Listener list was modified (attach or detach). */
    void (*on_train_listener_changed)(openlcb_node_t *openlcb_node);

        /** @brief Heartbeat timed out. estop_active and speed already updated. */
    void (*on_train_heartbeat_timeout)(openlcb_node_t *openlcb_node);

    // ---- Train-node side: decision callbacks ----

        /** @brief Another controller wants to take over. Return true to accept. If NULL, default = accept. */
    bool (*on_train_controller_assign_request)(openlcb_node_t *openlcb_node, node_id_t current_controller, node_id_t requesting_controller);

        /** @brief Controller Changed Notify received. Return true to accept. If NULL, default = accept. */
    bool (*on_train_controller_changed_request)(openlcb_node_t *openlcb_node, node_id_t new_controller);

    // ---- Throttle side: notifiers (receiving replies from train) ----

        /** @brief Query speeds reply received. */
    void (*on_train_query_speeds_reply)(openlcb_node_t *openlcb_node, uint16_t set_speed, uint8_t status, uint16_t commanded_speed, uint16_t actual_speed);

        /** @brief Query function reply received. */
    void (*on_train_query_function_reply)(openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value);

        /** @brief Controller assign reply received. 0 = success. */
    void (*on_train_controller_assign_reply)(openlcb_node_t *openlcb_node, uint8_t result);

        /** @brief Controller query reply received. */
    void (*on_train_controller_query_reply)(openlcb_node_t *openlcb_node, uint8_t flags, node_id_t controller_node_id);

        /** @brief Controller changed notify reply received. */
    void (*on_train_controller_changed_notify_reply)(openlcb_node_t *openlcb_node, uint8_t result);

        /** @brief Listener attach reply received. */
    void (*on_train_listener_attach_reply)(openlcb_node_t *openlcb_node, node_id_t node_id, uint8_t result);

        /** @brief Listener detach reply received. */
    void (*on_train_listener_detach_reply)(openlcb_node_t *openlcb_node, node_id_t node_id, uint8_t result);

        /** @brief Listener query reply received. */
    void (*on_train_listener_query_reply)(openlcb_node_t *openlcb_node, uint8_t count, uint8_t index, uint8_t flags, node_id_t node_id);

        /** @brief Reserve reply received. 0 = success. */
    void (*on_train_reserve_reply)(openlcb_node_t *openlcb_node, uint8_t result);

        /** @brief Heartbeat request received from train. timeout_seconds is deadline. */
    void (*on_train_heartbeat_request)(openlcb_node_t *openlcb_node, uint32_t timeout_seconds);

#endif /* OPENLCB_COMPILE_TRAIN */

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)

    // =========================================================================
    // OPTIONAL: Train Search Callbacks (requires TRAIN + TRAIN_SEARCH)
    // =========================================================================

        /** @brief A train search matched this node. Optional. */
    void (*on_train_search_matched)(openlcb_node_t *openlcb_node, uint16_t search_address, uint8_t flags);

        /** @brief No train node matched the search. Return a newly created train node, or NULL to decline. Optional. */
    openlcb_node_t* (*on_train_search_no_match)(uint16_t search_address, uint8_t flags);

#endif /* OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH */

} openlcb_config_t;

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Initializes the entire OpenLCB stack with one call.
     *
     * @details Initializes all buffer infrastructure, builds internal interface
     * structs from the user config, and starts only the protocol modules selected
     * by OPENLCB_COMPILE_* defines.  After this returns, call OpenLcb_create_node()
     * to allocate nodes, then call OpenLcb_run() in your main loop.
     *
     * @param config  Pointer to the @ref openlcb_config_t to use.  Must remain valid
     *                for the lifetime of the application (use static or global storage).
     *
     * @warning All required function pointers in the config struct must be non-NULL.
     */
extern void OpenLcb_initialize(const openlcb_config_t *config);

    /**
     * @brief Drives all time-dependent protocol modules.
     *
     * @details Call from a 100ms hardware timer interrupt or periodic task.
     * Dispatches to all compiled-in modules that require regular timer ticks.
     */
extern void OpenLcb_100ms_timer_tick(void);

    /**
     * @brief Allocates and registers a new node on the OpenLCB network.
     *
     * @param node_id     Unique 48-bit @ref node_id_t for the new node.
     * @param parameters  Pointer to the @ref node_parameters_t (SNIP info, protocol flags, events).
     *
     * @return Pointer to the allocated @ref openlcb_node_t, or NULL if no slots available.
     */
extern openlcb_node_t *OpenLcb_create_node(node_id_t node_id, const node_parameters_t *parameters);

    /**
     * @brief Runs one iteration of all state machines.
     *
     * @details Call as fast as possible in your main loop.  Non-blocking —
     * returns after processing one operation per call.
     */
extern void OpenLcb_run(void);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_CONFIG__ */
