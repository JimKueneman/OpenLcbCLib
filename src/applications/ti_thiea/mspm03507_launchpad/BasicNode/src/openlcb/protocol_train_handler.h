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
 * @date 17 Feb 2026
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
     * @brief Callback interface for train protocol events
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

        /** Speed was set. State already updated. */
        void (*on_speed_changed)(openlcb_node_t *openlcb_node, uint16_t speed_float16);

        /** Function was set. Standard functions (F0-F28) stored in train_state.functions[]. */
        void (*on_function_changed)(openlcb_node_t *openlcb_node,
                uint32_t fn_address, uint16_t fn_value);

        /** Emergency stop. estop_active and speed already updated in state. */
        void (*on_emergency_stopped)(openlcb_node_t *openlcb_node);

        /** Controller was assigned or changed. State already updated. */
        void (*on_controller_assigned)(openlcb_node_t *openlcb_node,
                uint64_t controller_node_id);

        /** Controller was released. State already cleared. */
        void (*on_controller_released)(openlcb_node_t *openlcb_node);

        /** Listener list was modified (attach or detach). */
        void (*on_listener_changed)(openlcb_node_t *openlcb_node);

        /** Heartbeat timed out. estop_active and speed already updated. */
        void (*on_heartbeat_timeout)(openlcb_node_t *openlcb_node);

        // ---- Train-node side: decision callbacks ----

        /**
         * Another controller wants to take over. Return 0 to accept,
         * non-zero result byte to reject. If NULL, default = accept (0).
         */
        uint8_t (*on_controller_assign_request)(openlcb_node_t *openlcb_node,
                uint64_t current_controller, uint64_t requesting_controller);

        /**
         * Old controller receiving Controller Changed Notify from train.
         * Return 0 to accept handoff, non-zero to reject.
         * If NULL, default = accept (0).
         */
        uint8_t (*on_controller_changed_request)(openlcb_node_t *openlcb_node,
                uint64_t new_controller);

        /**
         * Query function value. Return the 16-bit value for the given
         * function address. If NULL, default = return 0.
         */
        uint16_t (*on_query_function_request)(openlcb_node_t *openlcb_node,
                uint32_t fn_address);

        // ---- Throttle side: notifiers (receiving replies from train) ----

        void (*on_query_speeds_reply)(openlcb_node_t *openlcb_node,
                uint16_t set_speed, uint8_t status,
                uint16_t commanded_speed, uint16_t actual_speed);

        void (*on_query_function_reply)(openlcb_node_t *openlcb_node,
                uint32_t fn_address, uint16_t fn_value);

        void (*on_controller_assign_reply)(openlcb_node_t *openlcb_node,
                uint8_t result);

        void (*on_controller_query_reply)(openlcb_node_t *openlcb_node,
                uint8_t flags, uint64_t controller_node_id);

        void (*on_controller_changed_notify_reply)(openlcb_node_t *openlcb_node,
                uint8_t result);

        void (*on_listener_attach_reply)(openlcb_node_t *openlcb_node,
                uint64_t node_id, uint8_t result);

        void (*on_listener_detach_reply)(openlcb_node_t *openlcb_node,
                uint64_t node_id, uint8_t result);

        void (*on_listener_query_reply)(openlcb_node_t *openlcb_node,
                uint8_t count, uint8_t index, uint8_t flags, uint64_t node_id);

        void (*on_reserve_reply)(openlcb_node_t *openlcb_node, uint8_t result);

        void (*on_heartbeat_request)(openlcb_node_t *openlcb_node,
                uint32_t timeout_seconds);

    } interface_protocol_train_handler_t;

#ifdef __cplusplus
extern "C" {
#endif

    extern void ProtocolTrainHandler_initialize(
            const interface_protocol_train_handler_t *interface);

    extern const interface_protocol_train_handler_t* ProtocolTrainHandler_get_interface(void);

    extern void ProtocolTrainHandler_handle_train_command(
            openlcb_statemachine_info_t *statemachine_info);

    extern void ProtocolTrainHandler_handle_train_reply(
            openlcb_statemachine_info_t *statemachine_info);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_PROTOCOL_TRAIN_HANDLER__ */
