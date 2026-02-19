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
 * @brief User-facing configuration struct for OpenLcbCLib
 *
 * @details Provides a single config struct and init call that replaces the
 * per-application dependency_injection.c pattern. Users populate one
 * openlcb_config_t struct with hardware driver functions and optional
 * application callbacks, then call OpenLcb_initialize() with a feature
 * flags mask to bring up the entire stack.
 *
 * @author Jim Kueneman
 * @date 17 Feb 2026
 *
 * @see docs/option_d_implementation.md - Design document
 * @see docs/profile_function_pointer_mapping.md - Per-profile callback tables
 */

#ifndef __OPENLCB_CONFIG__
#define __OPENLCB_CONFIG__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

// =============================================================================
// Feature Flags -- select which protocol modules to enable
// =============================================================================

/** @brief Core features always enabled (SNIP + Message Network + PIP) */
#define OPENLCB_FEATURE_SNIP             (1 << 0)
/** @brief Event Transport protocol */
#define OPENLCB_FEATURE_EVENTS           (1 << 1)
/** @brief Datagram Transport protocol */
#define OPENLCB_FEATURE_DATAGRAMS        (1 << 2)
/** @brief Configuration Memory protocol (requires DATAGRAMS) */
#define OPENLCB_FEATURE_CONFIG_MEMORY    (1 << 3)
/** @brief Stream Transport protocol (requires DATAGRAMS) */
#define OPENLCB_FEATURE_STREAMS          (1 << 4)
/** @brief Broadcast Time protocol (requires EVENTS) */
#define OPENLCB_FEATURE_BROADCAST_TIME   (1 << 5)
/** @brief Train Control protocol */
#define OPENLCB_FEATURE_TRAIN         (1 << 6)
/** @brief Firmware Upgrade protocol (requires DATAGRAMS) */
#define OPENLCB_FEATURE_FIRMWARE_UPGRADE (1 << 7)
/** @brief Train Search protocol (requires EVENTS + TRAIN) */
#define OPENLCB_FEATURE_TRAIN_SEARCH    (1 << 8)

// =============================================================================
// Predefined Profiles -- convenience combinations
// =============================================================================

/** @brief Bootloader -- minimal node, just enough to receive firmware */
#define OPENLCB_PROFILE_BOOTLOADER  (OPENLCB_FEATURE_SNIP | \
                                     OPENLCB_FEATURE_DATAGRAMS | \
                                     OPENLCB_FEATURE_FIRMWARE_UPGRADE)

/** @brief Simple node -- SNIP + Events, no configuration memory or CDI */
#define OPENLCB_PROFILE_SIMPLE      (OPENLCB_FEATURE_SNIP | \
                                     OPENLCB_FEATURE_EVENTS)

/** @brief Standard node -- SNIP + Events + Datagrams + Config Memory, no Streams */
#define OPENLCB_PROFILE_STANDARD    (OPENLCB_FEATURE_SNIP | \
                                     OPENLCB_FEATURE_EVENTS | \
                                     OPENLCB_FEATURE_DATAGRAMS | \
                                     OPENLCB_FEATURE_CONFIG_MEMORY)

/** @brief Train node -- Standard + Train Control + FDI/Function Config spaces */
#define OPENLCB_PROFILE_TRAIN       (OPENLCB_PROFILE_STANDARD | \
                                     OPENLCB_FEATURE_TRAIN)

/** @brief Full node -- everything in Standard + Streams */
#define OPENLCB_PROFILE_FULL        (OPENLCB_PROFILE_STANDARD | \
                                     OPENLCB_FEATURE_STREAMS)

/**
 * @brief User configuration for OpenLcbCLib.
 *
 * @details Populate this struct with your hardware driver functions and
 * optional application callbacks, then pass it to OpenLcb_initialize()
 * along with a feature flags mask selecting which protocols to enable.
 *
 * Required fields are marked REQUIRED and must be non-NULL.
 * All other fields are optional and default to NULL (disabled).
 *
 * Example:
 * @code
 * static const openlcb_config_t my_config = {
 *     // Hardware -- REQUIRED
 *     .lock_shared_resources   = &MyDriver_lock,
 *     .unlock_shared_resources = &MyDriver_unlock,
 *     .config_mem_read         = &MyDriver_eeprom_read,
 *     .config_mem_write        = &MyDriver_eeprom_write,
 *     .reboot                  = &MyDriver_reboot,
 *
 *     // Application callbacks -- optional
 *     .on_login_complete       = &my_login_handler,
 *     .on_consumed_event_pcer  = &my_event_handler,
 * };
 *
 * // Standard node with broadcast time support
 * OpenLcb_initialize(&my_config, OPENLCB_PROFILE_STANDARD | OPENLCB_FEATURE_BROADCAST_TIME);
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

    /** @brief Read from configuration memory (EEPROM/Flash/file). REQUIRED.
     *  @param openlcb_node The node requesting the read
     *  @param address Starting address in configuration memory
     *  @param count Number of bytes to read
     *  @param buffer Destination buffer (configuration_memory_buffer_t = uint8_t[64])
     *  @return Number of bytes actually read */
    uint16_t (*config_mem_read)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

    /** @brief Write to configuration memory (EEPROM/Flash/file). REQUIRED.
     *  @param openlcb_node The node requesting the write
     *  @param address Starting address in configuration memory
     *  @param count Number of bytes to write
     *  @param buffer Source buffer (configuration_memory_buffer_t = uint8_t[64])
     *  @return Number of bytes actually written */
    uint16_t (*config_mem_write)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

    /** @brief Reboot the processor. REQUIRED.
     *  Called by the Memory Config "Reset/Reboot" command.
     *  @param statemachine_info State machine context
     *  @param config_mem_operations_request_info Operations request context */
    void (*reboot)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    // =========================================================================
    // OPTIONAL: Hardware Driver Extensions
    // =========================================================================

    /** @brief Freeze the node for firmware upgrade. Optional.
     *  @param statemachine_info State machine context
     *  @param config_mem_operations_request_info Operations request context */
    void (*freeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    /** @brief Unfreeze the node after firmware upgrade. Optional.
     *  @param statemachine_info State machine context
     *  @param config_mem_operations_request_info Operations request context */
    void (*unfreeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    /** @brief Write firmware data during upgrade. Optional.
     *  @param statemachine_info State machine context
     *  @param config_mem_write_request_info Write request context with address and data */
    void (*firmware_write)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

    /** @brief Factory reset handler -- erase user config and restore defaults. Optional.
     *  @param statemachine_info State machine context
     *  @param config_mem_operations_request_info Operations request context */
    void (*factory_reset)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    /** @brief Return delayed reply time flag for config memory reads. Optional.
     *  Return 0 for no delay, or (0x80 | N) for 2^N second reply pending.
     *  @param statemachine_info State machine context
     *  @param config_mem_read_request_info Read request context */
    uint16_t (*config_mem_read_delayed_reply_time)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

    /** @brief Return delayed reply time flag for config memory writes. Optional.
     *  @param statemachine_info State machine context
     *  @param config_mem_write_request_info Write request context */
    uint16_t (*config_mem_write_delayed_reply_time)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

    // =========================================================================
    // OPTIONAL: Core Application Callbacks
    // =========================================================================

    /** @brief 100ms periodic timer callback. Optional.
     *  Called once per 100ms tick for each allocated node. */
    void (*on_100ms_timer)(void);

    /** @brief Called when a node completes login and enters RUN state. Optional.
     *  Return true if login can complete, false to delay. */
    bool (*on_login_complete)(openlcb_node_t *openlcb_node);

    // =========================================================================
    // OPTIONAL: Event Transport Callbacks
    // =========================================================================

    /** @brief An event this node consumes has been identified on the network. Optional.
     *  Called when a Consumer Identified message matches one of this node's consumed events. */
    void (*on_consumed_event_identified)(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_status_enum status, event_payload_t *payload);

    /** @brief A PC Event Report was received for an event this node consumes. Optional.
     *  This is the primary "an event happened" notification. */
    void (*on_consumed_event_pcer)(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_payload_t *payload);

    /** @brief Event learn/teach message received. Optional.
     *  Implement to support blue/gold button configuration. */
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

    /** @brief PC Event Report received (unfiltered -- for any event, not just consumed). Optional. */
    void (*on_pc_event_report)(openlcb_node_t *openlcb_node, event_id_t *event_id);

    /** @brief PC Event Report with payload received. Optional. */
    void (*on_pc_event_report_with_payload)(openlcb_node_t *openlcb_node, event_id_t *event_id, uint16_t count, event_payload_t *payload);

    // =========================================================================
    // OPTIONAL: Broadcast Time Callbacks
    // =========================================================================

    /** @brief Broadcast time changed (clock minute advanced). Optional.
     *  Called each time the fast clock advances by one minute. */
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

    // =========================================================================
    // OPTIONAL: Train Control Callbacks (OPENLCB_FEATURE_TRAIN)
    // All are optional (NULL = use handler defaults).
    // Notifiers fire AFTER state is updated. Decision callbacks return a value.
    // =========================================================================

    // ---- Train-node side: notifiers (fire after state updated) ----

    /** @brief Speed was set on this train node. State already updated. */
    void (*on_train_speed_changed)(openlcb_node_t *openlcb_node, uint16_t speed_float16);

    /** @brief Function was set on this train node. Standard functions stored in train_state.functions[]. */
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

    /** @brief Another controller wants to take over. Return true to accept, false to reject. If NULL, default = accept. */
    bool (*on_train_controller_assign_request)(openlcb_node_t *openlcb_node, node_id_t current_controller, node_id_t requesting_controller);

    /** @brief Old controller receiving Controller Changed Notify. Return true to accept, false to reject. If NULL, default = accept. */
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

    // =========================================================================
    // OPTIONAL: Train Search Callbacks (OPENLCB_FEATURE_TRAIN_SEARCH)
    // =========================================================================

    /** @brief A train search matched this node. Optional. */
    void (*on_train_search_matched)(openlcb_node_t *openlcb_node, uint16_t search_address, uint8_t flags);

    /** @brief No train node matched the search. If allocate bit set,
     *  return a newly created train node, or NULL to decline. Optional. */
    openlcb_node_t* (*on_train_search_no_match)(uint16_t search_address, uint8_t flags);

} openlcb_config_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the entire OpenLcbCLib stack with one call.
 *
 * @details This function:
 *   1. Initializes all buffer infrastructure
 *   2. Builds all internal interface structs from the user config
 *   3. Initializes only the protocol modules enabled by the features mask
 *
 * After this returns, call OpenLcb_create_node() to allocate nodes,
 * then call OpenLcb_run() in your main loop.
 *
 * Protocols not included in the features mask are not wired. Incoming
 * messages for unwired protocols are automatically rejected with
 * Optional Interaction Rejected (for addressed messages) or silently
 * ignored (for global messages), per the OpenLCB specification.
 *
 * @param config   User configuration. Must remain valid for the lifetime of
 *                 the application (use static or global storage).
 * @param features Bitmask of OPENLCB_FEATURE_* flags, or a predefined
 *                 OPENLCB_PROFILE_* combination.
 */
extern void OpenLcb_initialize(const openlcb_config_t *config, uint32_t features);

/**
 * @brief Allocate and register a node on the OpenLCB network.
 *
 * @param node_id    Unique 48-bit Node ID
 * @param parameters Node parameter structure (SNIP info, protocol flags, events)
 * @return Pointer to allocated node, or NULL if no slots available
 */
extern openlcb_node_t *OpenLcb_create_node(node_id_t node_id, const node_parameters_t *parameters);

/**
 * @brief Run one iteration of all state machines.
 *
 * @details Call this as fast as possible in your main loop. It handles:
 *   - CAN frame reception and transmission
 *   - OpenLCB login sequence
 *   - Protocol message dispatch
 *
 * Non-blocking -- returns after processing one operation.
 */
extern void OpenLcb_run(void);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_CONFIG__ */
