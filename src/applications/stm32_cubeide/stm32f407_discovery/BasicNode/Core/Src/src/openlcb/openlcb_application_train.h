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
 * @brief Application-level Train Control Protocol module.
 *
 * @details Provides per-node train state, a fixed-size allocation pool,
 * throttle-side send helpers, and a heartbeat countdown timer for the OpenLCB
 * Train Control Protocol.
 *
 * The protocol handler (protocol_train_handler.c) handles incoming commands
 * automatically.  This module provides the application developer API: state
 * allocation, state access, throttle send functions, and the heartbeat tick.
 * State is drawn from a pool sized by USER_DEFINED_TRAIN_NODE_COUNT.  Each
 * train node gets a slot via OpenLcbApplicationTrain_setup(), which stores a
 * pointer in node->train_state.  Non-train nodes have train_state == NULL.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 *
 * @see protocol_train_handler.h
 */

#ifndef __OPENLCB_APPLICATION_TRAIN__
#define __OPENLCB_APPLICATION_TRAIN__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Application-provided callbacks for the train module.
     *
     * @details send_openlcb_msg is required for all throttle-side send helpers.
     * on_heartbeat_timeout may be NULL if the application does not need notification.
     *
     * @warning send_openlcb_msg must be non-NULL before calling any send function.
     */
    typedef struct {

        /** @brief Queues an outgoing OpenLCB message (required). */
        bool (*send_openlcb_msg)(openlcb_msg_t *openlcb_msg);

        /** @brief Called when the heartbeat timer for a train node reaches zero (optional). */
        void (*on_heartbeat_timeout)(openlcb_node_t *openlcb_node);

    } interface_openlcb_application_train_t;

#ifdef __cplusplus
extern "C" {
#endif

        /**
         * @brief Initialises the train module and stores the callback interface.
         *
         * @details Clears the train state pool and stores the interface pointer.
         * Must be called once before any other function in this module.
         *
         * @param interface  Pointer to a @ref interface_openlcb_application_train_t.
         *
         * @warning Must be called before OpenLcbApplicationTrain_setup().
         */
    extern void OpenLcbApplicationTrain_initialize(
            const interface_openlcb_application_train_t *interface);

        /**
         * @brief Allocates a train state slot and assigns it to the node.
         *
         * @details Draws the next free slot from the pool, zeroes it, stores a pointer
         * in openlcb_node->train_state, and registers the standard train event IDs on
         * the node (Train, Emergency Off/Stop, Clear Emergency Off/Stop).
         *
         * Returns the existing slot immediately if the node already has one.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t to configure as a train.
         *
         * @return Pointer to the @ref train_state_t, or NULL if the node pointer is NULL
         *         or the pool is exhausted.
         *
         * @warning Returns NULL if the pool is full (USER_DEFINED_TRAIN_NODE_COUNT slots used).
         */
    extern train_state_t* OpenLcbApplicationTrain_setup(openlcb_node_t *openlcb_node);

        /**
         * @brief Returns the train state for a node.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t.
         *
         * @return Pointer to the @ref train_state_t, or NULL if the node pointer is NULL
         *         or the node has no train state assigned.
         */
    extern train_state_t* OpenLcbApplicationTrain_get_state(openlcb_node_t *openlcb_node);

        /**
         * @brief Decrements the heartbeat countdown for all active train nodes.
         *
         * @details Must be called every 100 ms.  At the halfway point of each node's
         * heartbeat timeout, a NOOP request is sent to the controller.  When the
         * counter reaches zero the train is emergency-stopped and the on_heartbeat_timeout
         * callback is fired.
         *
         * @note Nodes with heartbeat_timeout_s == 0 are skipped.
         */
    extern void OpenLcbApplicationTrain_100ms_timer_tick(void);

        /**
         * @brief Sends a Set Speed/Direction command to a train node.
         *
         * @details Builds and sends an addressed Train Protocol message that sets the
         * speed and direction of a target train.  The speed value uses the OpenLCB
         * float16 encoding where the sign bit encodes direction.
         *
         * @param openlcb_node    Pointer to the sending (throttle) @ref openlcb_node_t.
         * @param train_node_id   48-bit @ref node_id_t of the target train node.
         * @param speed           16-bit speed/direction value in OpenLCB float16 format.
         */
    extern void OpenLcbApplicationTrain_send_set_speed(openlcb_node_t *openlcb_node, node_id_t train_node_id, uint16_t speed);

        /**
         * @brief Sends a Set Function command to a train node.
         *
         * @details Builds and sends an addressed Train Protocol message that sets a
         * DCC function on the target train.  Function 0 is the headlight; higher
         * numbers follow the NMRA function mapping.
         *
         * @param openlcb_node    Pointer to the sending @ref openlcb_node_t.
         * @param train_node_id   48-bit @ref node_id_t of the target train node.
         * @param fn_address      24-bit function address.
         * @param fn_value        16-bit function value (0 = off, non-zero = on or analog value).
         */
    extern void OpenLcbApplicationTrain_send_set_function(openlcb_node_t *openlcb_node, node_id_t train_node_id, uint32_t fn_address, uint16_t fn_value);

        /**
         * @brief Sends an Emergency Stop command to a train node.
         *
         * @details Commands the target train to stop immediately.  The train node
         * sets its speed to zero and activates its emergency-stop flag.
         *
         * @param openlcb_node    Pointer to the sending @ref openlcb_node_t.
         * @param train_node_id   48-bit @ref node_id_t of the target train node.
         */
    extern void OpenLcbApplicationTrain_send_emergency_stop(openlcb_node_t *openlcb_node, node_id_t train_node_id);

        /**
         * @brief Sends a Query Speeds command to a train node.
         *
         * @details Asks the train node to reply with its current set speed, actual
         * speed (if available), and emergency-stop status.
         *
         * @param openlcb_node    Pointer to the sending @ref openlcb_node_t.
         * @param train_node_id   48-bit @ref node_id_t of the target train node.
         */
    extern void OpenLcbApplicationTrain_send_query_speeds(openlcb_node_t *openlcb_node, node_id_t train_node_id);

        /**
         * @brief Sends a Query Function command to a train node.
         *
         * @details Asks the train node to reply with the current value of the
         * specified function address.
         *
         * @param openlcb_node    Pointer to the sending @ref openlcb_node_t.
         * @param train_node_id   48-bit @ref node_id_t of the target train node.
         * @param fn_address      24-bit function address to query.
         */
    extern void OpenLcbApplicationTrain_send_query_function(openlcb_node_t *openlcb_node, node_id_t train_node_id, uint32_t fn_address);

        /**
         * @brief Sends an Assign Controller command to a train node.
         *
         * @details Sends the throttle node's own Node ID as the controller to assign.
         * Once assigned, the train node accepts speed/function commands only from the
         * assigned controller and begins heartbeat monitoring.
         *
         * @param openlcb_node    Pointer to the sending (throttle) @ref openlcb_node_t.
         * @param train_node_id   48-bit @ref node_id_t of the target train node.
         */
    extern void OpenLcbApplicationTrain_send_assign_controller(openlcb_node_t *openlcb_node, node_id_t train_node_id);

        /**
         * @brief Sends a Release Controller command to a train node.
         *
         * @details Sends the throttle node's own Node ID as the controller to release.
         * The train node clears its controller assignment and stops heartbeat monitoring.
         *
         * @param openlcb_node    Pointer to the sending (throttle) @ref openlcb_node_t.
         * @param train_node_id   48-bit @ref node_id_t of the target train node.
         */
    extern void OpenLcbApplicationTrain_send_release_controller(openlcb_node_t *openlcb_node, node_id_t train_node_id);

        /**
         * @brief Sends a NOOP (no-operation) management command to a train node.
         *
         * @details Resets the train's heartbeat countdown timer without changing
         * speed, functions, or any other state.  Throttle applications should send
         * this periodically to prevent heartbeat timeout and emergency stop.
         *
         * @param openlcb_node    Pointer to the sending @ref openlcb_node_t.
         * @param train_node_id   48-bit @ref node_id_t of the target train node.
         */
    extern void OpenLcbApplicationTrain_send_noop(openlcb_node_t *openlcb_node, node_id_t train_node_id);

        /**
         * @brief Sets the DCC address and address type for a train node.
         *
         * @param openlcb_node    Pointer to the @ref openlcb_node_t with an assigned train state.
         * @param dcc_address     DCC address (1–9999 for long, 1–127 for short).
         * @param is_long_address true for long (four-digit) addressing, false for short.
         *
         * @note Silently ignored if the node pointer or train_state pointer is NULL.
         */
    extern void OpenLcbApplicationTrain_set_dcc_address(openlcb_node_t *openlcb_node, uint16_t dcc_address, bool is_long_address);

        /**
         * @brief Returns the DCC address for a train node.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t.
         *
         * @return DCC address, or 0 if the node has no train state.
         */
    extern uint16_t OpenLcbApplicationTrain_get_dcc_address(openlcb_node_t *openlcb_node);

        /**
         * @brief Returns true if the train node uses long DCC addressing.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t.
         *
         * @return true for long addressing, false if short or node has no train state.
         */
    extern bool OpenLcbApplicationTrain_is_long_address(openlcb_node_t *openlcb_node);

        /**
         * @brief Sets the speed-step mode for a train node.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t with an assigned train state.
         * @param speed_steps   Speed-step count (14, 28, or 128).
         *
         * @note Silently ignored if the node pointer or train_state pointer is NULL.
         */
    extern void OpenLcbApplicationTrain_set_speed_steps(openlcb_node_t *openlcb_node, uint8_t speed_steps);

        /**
         * @brief Returns the speed-step mode for a train node.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t.
         *
         * @return Speed-step count (14, 28, or 128), or 0 if the node has no train state.
         */
    extern uint8_t OpenLcbApplicationTrain_get_speed_steps(openlcb_node_t *openlcb_node);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_APPLICATION_TRAIN__ */
