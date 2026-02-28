/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file can_login_message_handler.h
 * @brief State handlers for the 10-state CAN alias allocation login sequence.
 *
 * @details Each handler builds the appropriate CAN control frame (CID, RID, AMD)
 * per the OpenLCB CAN Frame Transfer Standard.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__
#define __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

    /**
     * @brief Dependency-injection interface for the CAN login message handler.
     *
     * @details Provides alias-mapping callbacks required by the login sequence.
     * All function pointers are REQUIRED (must not be NULL) except on_alias_change.
     *
     * @see CanLoginMessageHandler_initialize
     */
typedef struct {

        /** @brief REQUIRED. Register a new alias/Node ID pair. Typical impl: AliasMappings_register. */
    alias_mapping_t *(*alias_mapping_register)(uint16_t alias, node_id_t node_id);

        /** @brief REQUIRED. Find a mapping by alias. Typical impl: AliasMappings_find_mapping_by_alias. */
    alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

        /** @brief OPTIONAL. Called when an alias is successfully registered. May be NULL. */
    void (*on_alias_change)(uint16_t alias, node_id_t node_id);

} interface_can_login_message_handler_t;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Registers the dependency-injection interface for this module.
         *
         * @param interface Pointer to a populated @ref interface_can_login_message_handler_t.
         *                  Must remain valid for the lifetime of the application.
         *
         * @warning NOT thread-safe - call during single-threaded initialization only.
         *
         * @see CanLoginStateMachine_initialize
         */
    extern void CanLoginMessageHandler_initialize(const interface_can_login_message_handler_t *interface);

        /**
         * @brief State 1: Sets node seed to its Node ID, then jumps to GENERATE_ALIAS.
         *
         * @param can_statemachine_info State machine context (node + outgoing message buffer).
         */
    extern void CanLoginMessageHandler_state_init(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 2: Generates a new seed via LFSR, then transitions to GENERATE_ALIAS.
         *
         * @details Only entered when an alias conflict forces a new alias attempt.
         *
         * @param can_statemachine_info State machine context.
         */
    extern void CanLoginMessageHandler_state_generate_seed(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 3: Derives a 12-bit alias from the seed, registers it, then transitions to LOAD_CID07.
         *
         * @param can_statemachine_info State machine context.
         */
    extern void CanLoginMessageHandler_state_generate_alias(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 4: Loads a CID7 frame (Node ID bits 47-36) into the outgoing buffer.
         *
         * @param can_statemachine_info State machine context.
         */
    extern void CanLoginMessageHandler_state_load_cid07(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 5: Loads a CID6 frame (Node ID bits 35-24) into the outgoing buffer.
         *
         * @param can_statemachine_info State machine context.
         */
    extern void CanLoginMessageHandler_state_load_cid06(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 6: Loads a CID5 frame (Node ID bits 23-12) into the outgoing buffer.
         *
         * @param can_statemachine_info State machine context.
         */
    extern void CanLoginMessageHandler_state_load_cid05(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 7: Loads a CID4 frame (Node ID bits 11-0) and resets the 200 ms timer.
         *
         * @param can_statemachine_info State machine context.
         */
    extern void CanLoginMessageHandler_state_load_cid04(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 8: Waits until timerticks > 2, then transitions to LOAD_RESERVE_ID.
         *
         * @details Non-blocking — returns immediately each call until the timer expires.
         * timerticks is reset to 0 at the end of State 7 (CID4) and incremented every 100 ms,
         * so this waits at least 300 ms — satisfying the spec minimum of 200 ms (§6.2.1).
         * Requires the 100 ms timer tick to be running (OpenLcbNode_100ms_timer_tick).
         *
         * @param can_statemachine_info State machine context.
         */
    extern void CanLoginMessageHandler_state_wait_200ms(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 9: Loads an RID frame to claim the alias, then transitions to LOAD_AMD.
         *
         * @param can_statemachine_info State machine context.
         */
    extern void CanLoginMessageHandler_state_load_rid(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief State 10: Loads an AMD frame with the full Node ID, marks the node permitted,
         * and updates the alias mapping to permitted status.
         *
         * @details This is the final state. After transmission the node may send OpenLCB messages.
         *
         * @param can_statemachine_info State machine context.
         *
         * @see CanUtilities_copy_node_id_to_payload
         */
    extern void CanLoginMessageHandler_state_load_amd(can_statemachine_info_t *can_statemachine_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__ */
