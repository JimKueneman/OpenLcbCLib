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
 * @file can_login_statemachine.h
 * @brief Dispatcher for the 10-state CAN alias allocation login sequence.
 *
 * @details Transitions through INIT -> GENERATE_SEED -> GENERATE_ALIAS ->
 * CID7 -> CID6 -> CID5 -> CID4 -> WAIT_200ms -> RID -> AMD.
 * State handlers are supplied via dependency injection.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

    /**
     * @brief Dependency-injection interface for the CAN login state machine.
     *
     * @details Each field is a function pointer to one of the 10 state handlers.
     * All pointers are REQUIRED (must not be NULL).
     * Typical implementations are the CanLoginMessageHandler_state_* functions.
     *
     * @see CanLoginStateMachine_initialize
     * @see can_login_message_handler.h
     */
typedef struct {

    void (*state_init)(can_statemachine_info_t *can_statemachine_info);           /**< @brief State 1 - init seed. */
    void (*state_generate_seed)(can_statemachine_info_t *can_statemachine_info);  /**< @brief State 2 - LFSR new seed (conflict retry only). */
    void (*state_generate_alias)(can_statemachine_info_t *can_statemachine_info); /**< @brief State 3 - derive 12-bit alias. */
    void (*state_load_cid07)(can_statemachine_info_t *can_statemachine_info);     /**< @brief State 4 - CID7 frame. */
    void (*state_load_cid06)(can_statemachine_info_t *can_statemachine_info);     /**< @brief State 5 - CID6 frame. */
    void (*state_load_cid05)(can_statemachine_info_t *can_statemachine_info);     /**< @brief State 6 - CID5 frame. */
    void (*state_load_cid04)(can_statemachine_info_t *can_statemachine_info);     /**< @brief State 7 - CID4 frame + start 200 ms timer. */
    void (*state_wait_200ms)(can_statemachine_info_t *can_statemachine_info);     /**< @brief State 8 - wait for timerticks > 2 (at least 300 ms; spec requires >= 200 ms). */
    void (*state_load_rid)(can_statemachine_info_t *can_statemachine_info);       /**< @brief State 9 - RID frame. */
    void (*state_load_amd)(can_statemachine_info_t *can_statemachine_info);       /**< @brief State 10 - AMD frame, mark permitted. */

} interface_can_login_state_machine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Registers the dependency-injection interface for this module.
         *
         * @param interface_can_login_state_machine Pointer to a populated
         *        @ref interface_can_login_state_machine_t. Must remain valid for the
         *        lifetime of the application. All 10 function pointers must be non-NULL.
         *
         * @warning NOT thread-safe - call during single-threaded initialization only.
         *
         * @see CanLoginMessageHandler_initialize - initialize first
         * @see CanLoginStateMachine_run
         */
    extern void CanLoginStateMachine_initialize(const interface_can_login_state_machine_t *interface_can_login_state_machine);

        /**
         * @brief Dispatches to the handler for the node's current run_state.
         *
         * @details Non-blocking - executes exactly one state handler per call and returns.
         * Call repeatedly from the main loop until the node reaches permitted state.
         *
         * @param can_statemachine_info State machine context (node + outgoing message buffer).
         *
         * @warning can_statemachine_info must not be NULL.
         * @warning NOT thread-safe.
         *
         * @see CanLoginMessageHandler_state_init - entry point
         * @see CanLoginMessageHandler_state_load_amd - final state
         */
    extern void CanLoginStateMachine_run(can_statemachine_info_t *can_statemachine_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__ */
