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
 * @file protocol_train_handler.h
 * @brief Train Control Protocol message handler (Layer 1)
 *
 * @details Handles incoming MTI_TRAIN_PROTOCOL (0x05EB) commands and
 * MTI_TRAIN_REPLY (0x01E9) replies. Automatically updates train state,
 * builds protocol replies, and forwards consist commands to listeners.
 * Fires optional notifier callbacks after state is updated.
 *
 * Train-node side callbacks are split into:
 * - Notifiers: fire AFTER state is updated (all optional, NULL = ignored)
 * - Decision callbacks: return a value the handler uses to build a reply
 *   (optional, NULL = handler uses default behavior)
 *
 * Throttle-side callbacks are all notifiers that fire when a reply is received.
 *
 * Called from the main statemachine when a train protocol message is received.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 *
 * @see openlcb_application_train.h - Layer 2 state and API
 */

#ifndef __OPENLCB_PROTOCOL_TRAIN_HANDLER__
#define __OPENLCB_PROTOCOL_TRAIN_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @struct interface_protocol_train_handler_t
     * @brief Callback interface for train protocol events.
     *
     * All callbacks are optional (can be NULL).
     *
     * Train-node side:
     * - Notifiers fire AFTER the handler has updated train_state and built
     *   any reply. The application can use these to drive hardware (DCC output,
     *   motor controller, etc).
     * - Decision callbacks return a value the handler uses. If NULL the handler
     *   uses a default policy.
     *
     * Throttle side:
     * - Notifiers fire when a reply is received from the train node.
     */
    typedef struct {

        // ---- Train-node side: notifiers (fire after state updated) ----

            /** @brief Speed was set.  State already updated. */
        void (*on_speed_changed)(openlcb_node_t *openlcb_node, uint16_t speed_float16);

            /** @brief Function was set.  Standard functions stored in train_state.functions[]. */
        void (*on_function_changed)(openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value);

            /** @brief Emergency state entered.  State flags already updated. */
        void (*on_emergency_entered)(openlcb_node_t *openlcb_node, train_emergency_type_enum emergency_type);

            /** @brief Emergency state exited.  State flags already updated. */
        void (*on_emergency_exited)(openlcb_node_t *openlcb_node, train_emergency_type_enum emergency_type);

            /** @brief Controller assigned or changed.  State already updated. */
        void (*on_controller_assigned)(openlcb_node_t *openlcb_node, node_id_t controller_node_id);

            /** @brief Controller released.  State already cleared. */
        void (*on_controller_released)(openlcb_node_t *openlcb_node);

            /** @brief Listener list modified (attach or detach). */
        void (*on_listener_changed)(openlcb_node_t *openlcb_node);

            /** @brief Heartbeat timed out.  State already updated. */
        void (*on_heartbeat_timeout)(openlcb_node_t *openlcb_node);

        // ---- Train-node side: decision callbacks ----

            /** @brief Another controller wants to take over.  Return true to accept.  NULL = accept. */
        bool (*on_controller_assign_request)(openlcb_node_t *openlcb_node, node_id_t current_controller, node_id_t requesting_controller);

            /** @brief Controller Changed Notify received.  Return true to accept handoff.  NULL = accept. */
        bool (*on_controller_changed_request)(openlcb_node_t *openlcb_node, node_id_t new_controller);

        // ---- Throttle side: notifiers (receiving replies from train) ----

            /** @brief Query Speeds reply received. */
        void (*on_query_speeds_reply)(openlcb_node_t *openlcb_node, uint16_t set_speed, uint8_t status, uint16_t commanded_speed, uint16_t actual_speed);

            /** @brief Query Function reply received. */
        void (*on_query_function_reply)(openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value);

            /** @brief Controller Assign reply received. */
        void (*on_controller_assign_reply)(openlcb_node_t *openlcb_node, uint8_t result);

            /** @brief Controller Query reply received. */
        void (*on_controller_query_reply)(openlcb_node_t *openlcb_node, uint8_t flags, node_id_t controller_node_id);

            /** @brief Controller Changed Notify reply received. */
        void (*on_controller_changed_notify_reply)(openlcb_node_t *openlcb_node, uint8_t result);

            /** @brief Listener Attach reply received. */
        void (*on_listener_attach_reply)(openlcb_node_t *openlcb_node, node_id_t node_id, uint8_t result);

            /** @brief Listener Detach reply received. */
        void (*on_listener_detach_reply)(openlcb_node_t *openlcb_node, node_id_t node_id, uint8_t result);

            /** @brief Listener Query reply received. */
        void (*on_listener_query_reply)(openlcb_node_t *openlcb_node, uint8_t count, uint8_t index, uint8_t flags, node_id_t node_id);

            /** @brief Reserve reply received. */
        void (*on_reserve_reply)(openlcb_node_t *openlcb_node, uint8_t result);

            /** @brief Heartbeat request from train node. */
        void (*on_heartbeat_request)(openlcb_node_t *openlcb_node, uint32_t timeout_seconds);

    } interface_protocol_train_handler_t;

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Initializes the Train Control Protocol handler.
     *
     * @param interface  Pointer to @ref interface_protocol_train_handler_t callbacks (may be NULL).
     */
    extern void ProtocolTrainHandler_initialize(const interface_protocol_train_handler_t *interface);

    /**
     * @brief Handles an incoming Train Control Protocol command (MTI_TRAIN_PROTOCOL).
     *
     * @details Decodes the sub-command byte, updates train_state, builds the
     * appropriate reply, and fires notifier or decision callbacks.
     *
     * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
     */
    extern void ProtocolTrainHandler_handle_train_command(openlcb_statemachine_info_t *statemachine_info);

    /**
     * @brief Handles an incoming Train Control Protocol reply (MTI_TRAIN_REPLY).
     *
     * @details Decodes the reply sub-command and fires the matching throttle-side
     * notifier callback.
     *
     * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
     */
    extern void ProtocolTrainHandler_handle_train_reply(openlcb_statemachine_info_t *statemachine_info);

    /**
     * @brief Handles a global or addressed emergency event for a train node.
     *
     * @details Activates or clears the emergency state based on the well-known
     * Event ID and fires the appropriate notifier callback.
     *
     * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
     * @param event_id           Well-known @ref event_id_t for the emergency condition.
     */
    extern void ProtocolTrainHandler_handle_emergency_event(
            openlcb_statemachine_info_t *statemachine_info, event_id_t event_id);


#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_PROTOCOL_TRAIN_HANDLER__ */
