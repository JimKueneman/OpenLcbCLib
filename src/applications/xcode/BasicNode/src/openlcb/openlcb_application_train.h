/** \copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * @file openlcb_application_train.h
 * @brief Application-level Train Control Protocol module
 *
 * @details Provides per-node train state, a static allocation pool,
 * listener management, send helpers, and heartbeat timer for the OpenLCB
 * Train Control Protocol.
 *
 * The protocol handler (protocol_train_handler.c) automatically handles
 * incoming commands: updating state, building replies, and firing notifiers.
 * This module provides the application developer API: state access, listener
 * management, throttle-side send functions, and the heartbeat timer.
 *
 * State is allocated from a fixed pool sized by USER_DEFINED_TRAIN_NODE_COUNT.
 * Each train node gets state via OpenLcbApplicationTrain_setup(), which assigns
 * a pool slot to node->train_state. Non-train nodes have train_state == NULL.
 *
 * @author Jim Kueneman
 * @date 17 Feb 2026
 *
 * @see protocol_train_handler.h - Layer 1 protocol handler
 */

#ifndef __OPENLCB_APPLICATION_TRAIN__
#define __OPENLCB_APPLICATION_TRAIN__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifndef USER_DEFINED_TRAIN_NODE_COUNT
#define USER_DEFINED_TRAIN_NODE_COUNT 4
#endif

#ifndef USER_DEFINED_MAX_LISTENERS_PER_TRAIN
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN 6
#endif

#ifndef USER_DEFINED_MAX_TRAIN_FUNCTIONS
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS 29   /**< F0-F28, standard DCC range */
#endif

    /**
     * @struct train_listener_entry_t
     * @brief A single listener entry for a train consist
     */
    typedef struct {

        uint64_t node_id;   /**< Listener node ID (0 = empty/unused slot) */
        uint8_t flags;      /**< Listener flags (reverse, link F0, link Fn, hide) */

    } train_listener_entry_t;

    /**
     * @struct train_state_TAG
     * @brief Per-node train state
     *
     * @details Holds the mutable runtime state for a single train node.
     * Allocated from a static pool by OpenLcbApplicationTrain_setup().
     */
    typedef struct train_state_TAG {

        uint16_t set_speed;              /**< Last commanded speed (float16 IEEE 754) */
        uint16_t commanded_speed;        /**< Control algorithm output speed (float16) */
        uint16_t actual_speed;           /**< Measured speed, optional (float16) */
        uint8_t estop_active;            /**< Emergency stop active flag */
        uint64_t controller_node_id;     /**< Active controller node ID (0 if none) */
        uint8_t reserved_node_count;     /**< Reservation count */
        uint32_t heartbeat_timeout_s;    /**< Heartbeat deadline in seconds (0 = disabled) */
        uint32_t heartbeat_counter_100ms;/**< Heartbeat countdown in 100ms ticks */

        train_listener_entry_t listeners[USER_DEFINED_MAX_LISTENERS_PER_TRAIN];
        uint8_t listener_count;          /**< Number of active listeners */

        uint16_t functions[USER_DEFINED_MAX_TRAIN_FUNCTIONS]; /**< Function values (16-bit per function, indexed by function number) */

    } train_state_t;

    /**
     * @struct interface_openlcb_application_train_t
     * @brief Application interface for train module
     *
     * send_openlcb_msg is required for throttle-side send helpers.
     */
    typedef struct {

        bool (*send_openlcb_msg)(openlcb_msg_t *openlcb_msg);

    } interface_openlcb_application_train_t;

#ifdef __cplusplus
extern "C" {
#endif

    // Initialization

    extern void OpenLcbApplicationTrain_initialize(
            const interface_openlcb_application_train_t *interface);

    extern const interface_openlcb_application_train_t* OpenLcbApplicationTrain_get_interface(void);

    // Setup — allocates train state from pool, assigns to node->train_state

    extern train_state_t* OpenLcbApplicationTrain_setup(openlcb_node_t *openlcb_node);

    // Accessor

    extern train_state_t* OpenLcbApplicationTrain_get_state(openlcb_node_t *openlcb_node);

    // Timer tick — heartbeat countdown for all active train nodes

    extern void OpenLcbApplicationTrain_100ms_timer_tick(void);

    // Listener management (train node side)

    extern bool OpenLcbApplicationTrain_attach_listener(
            train_state_t *state, uint64_t node_id, uint8_t flags);

    extern bool OpenLcbApplicationTrain_detach_listener(
            train_state_t *state, uint64_t node_id);

    extern train_listener_entry_t* OpenLcbApplicationTrain_find_listener(
            train_state_t *state, uint64_t node_id);

    extern uint8_t OpenLcbApplicationTrain_get_listener_count(train_state_t *state);

    extern train_listener_entry_t* OpenLcbApplicationTrain_get_listener_by_index(
            train_state_t *state, uint8_t index);

    // Send helpers (throttle side — build and send train commands)

    extern void OpenLcbApplicationTrain_send_set_speed(
            openlcb_node_t *openlcb_node, uint64_t train_node_id, uint16_t speed);

    extern void OpenLcbApplicationTrain_send_set_function(
            openlcb_node_t *openlcb_node, uint64_t train_node_id,
            uint32_t fn_address, uint16_t fn_value);

    extern void OpenLcbApplicationTrain_send_emergency_stop(
            openlcb_node_t *openlcb_node, uint64_t train_node_id);

    extern void OpenLcbApplicationTrain_send_query_speeds(
            openlcb_node_t *openlcb_node, uint64_t train_node_id);

    extern void OpenLcbApplicationTrain_send_query_function(
            openlcb_node_t *openlcb_node, uint64_t train_node_id, uint32_t fn_address);

    extern void OpenLcbApplicationTrain_send_assign_controller(
            openlcb_node_t *openlcb_node, uint64_t train_node_id);

    extern void OpenLcbApplicationTrain_send_release_controller(
            openlcb_node_t *openlcb_node, uint64_t train_node_id);

    extern void OpenLcbApplicationTrain_send_noop(
            openlcb_node_t *openlcb_node, uint64_t train_node_id);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_APPLICATION_TRAIN__ */
