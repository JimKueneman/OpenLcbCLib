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

    /**
     * @struct interface_openlcb_application_train_t
     * @brief Application interface for train module
     *
     * send_openlcb_msg is required for throttle-side send helpers.
     */
    typedef struct {

        bool (*send_openlcb_msg)(openlcb_msg_t *openlcb_msg);
        void (*on_heartbeat_timeout)(openlcb_node_t *openlcb_node);

    } interface_openlcb_application_train_t;

#ifdef __cplusplus
extern "C" {
#endif

    // Initialization

    extern void OpenLcbApplicationTrain_initialize(
            const interface_openlcb_application_train_t *interface);

    // Setup — allocates train state from pool, assigns to node->train_state

    extern train_state_t* OpenLcbApplicationTrain_setup(openlcb_node_t *openlcb_node);

    // Accessor

    extern train_state_t* OpenLcbApplicationTrain_get_state(openlcb_node_t *openlcb_node);

    // Timer tick — heartbeat countdown for all active train nodes

    extern void OpenLcbApplicationTrain_100ms_timer_tick(void);

    // Send helpers (throttle side — build and send train commands)

    extern void OpenLcbApplicationTrain_send_set_speed(openlcb_node_t *openlcb_node, node_id_t train_node_id, uint16_t speed);

    extern void OpenLcbApplicationTrain_send_set_function(openlcb_node_t *openlcb_node, node_id_t train_node_id, uint32_t fn_address, uint16_t fn_value);

    extern void OpenLcbApplicationTrain_send_emergency_stop(openlcb_node_t *openlcb_node, node_id_t train_node_id);

    extern void OpenLcbApplicationTrain_send_query_speeds(openlcb_node_t *openlcb_node, node_id_t train_node_id);

    extern void OpenLcbApplicationTrain_send_query_function(openlcb_node_t *openlcb_node, node_id_t train_node_id, uint32_t fn_address);

    extern void OpenLcbApplicationTrain_send_assign_controller(openlcb_node_t *openlcb_node, node_id_t train_node_id);

    extern void OpenLcbApplicationTrain_send_release_controller(openlcb_node_t *openlcb_node, node_id_t train_node_id);

    extern void OpenLcbApplicationTrain_send_noop(openlcb_node_t *openlcb_node, node_id_t train_node_id);

    // Train search properties — set during setup, used by search handler

    extern void OpenLcbApplicationTrain_set_dcc_address(openlcb_node_t *openlcb_node, uint16_t dcc_address, bool is_long_address);

    extern uint16_t OpenLcbApplicationTrain_get_dcc_address(openlcb_node_t *openlcb_node);

    extern bool OpenLcbApplicationTrain_is_long_address(openlcb_node_t *openlcb_node);

    extern void OpenLcbApplicationTrain_set_speed_steps(openlcb_node_t *openlcb_node, uint8_t speed_steps);

    extern uint8_t OpenLcbApplicationTrain_get_speed_steps(openlcb_node_t *openlcb_node);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_APPLICATION_TRAIN__ */
