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
 * @brief State machine for logging nodes into the OpenLCB/LCC network
 *
 * @details Orchestrates the 10-state CAN login sequence required for a node to
 * obtain a valid 12-bit alias for its 48-bit Node ID. The sequence follows the
 * OpenLCB CAN Frame Transfer Standard: INIT → GENERATE_SEED → GENERATE_ALIAS →
 * CID7 → CID6 → CID5 → CID4 → WAIT_200ms → RID → AMD. State handlers are
 * provided via dependency injection.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

    /**
     * @brief Interface structure for CAN login state machine handler callbacks
     *
     * @details This structure defines the callback interface for the CAN login state machine,
     * which orchestrates the 10-state alias allocation sequence defined by the OpenLCB CAN
     * Frame Transfer Standard. The interface provides function pointers to all state handlers
     * required to complete the login sequence.
     *
     * The CAN login sequence transitions through the following states in order:
     * 1. INIT - Initialize seed to Node ID
     * 2. GENERATE_SEED - Generate new seed (only used for conflict resolution)
     * 3. GENERATE_ALIAS - Extract 12-bit alias from seed via LFSR
     * 4. LOAD_CHECK_ID_07 - Build CID7 frame with Node ID bits 47-36
     * 5. LOAD_CHECK_ID_06 - Build CID6 frame with Node ID bits 35-24
     * 6. LOAD_CHECK_ID_05 - Build CID5 frame with Node ID bits 23-12
     * 7. LOAD_CHECK_ID_04 - Build CID4 frame with Node ID bits 11-0
     * 8. WAIT_200ms - Mandatory 200ms wait for conflict detection
     * 9. LOAD_RESERVE_ID - Build RID frame to claim alias
     * 10. LOAD_ALIAS_MAP_DEFINITION - Build AMD frame with full Node ID
     *
     * Each state handler:
     * - Receives can_statemachine_info_t structure with node and message buffer
     * - Performs state-specific operations (generate alias, build CAN frame, wait)
     * - Sets login_outgoing_can_msg_valid flag if frame needs transmission
     * - Transitions node to next run_state
     *
     * After successful completion of all states, the node has:
     * - Valid 12-bit CAN alias for its 48-bit Node ID
     * - Permitted status on the CAN network
     * - Alias/Node ID mapping registered in alias mapping table
     *
     * All 10 state handler callbacks are REQUIRED and must be provided before calling
     * CanLoginStateMachine_initialize. The state machine dispatches to handlers based
     * on the node's current run_state.
     *
     * @note All callbacks are REQUIRED - none can be NULL
     * @note Handlers typically provided by can_login_message_handler.h
     * @note State machine is non-blocking - each handler executes quickly
     *
     * @see CanLoginStateMachine_initialize
     * @see CanLoginStateMachine_run
     * @see can_login_message_handler.h - Provides standard implementations
     */
typedef struct {

        /**
         * @brief Handler for INIT state (state 1 of 10)
         *
         * @details Initializes the node's seed value to its 48-bit Node ID and transitions
         * to GENERATE_ALIAS state. This is the entry point for the login sequence.
         *
         * State responsibilities:
         * - Set node seed to Node ID value
         * - Transition to RUNSTATE_GENERATE_ALIAS
         * - Prepare for alias generation
         *
         * This state is entered when:
         * - Node first begins login sequence
         * - Node restarts after going offline
         *
         * Typical implementation: CanLoginMessageHandler_state_init
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Normal login skips GENERATE_SEED and goes directly to GENERATE_ALIAS
         */
    void (*state_init)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for GENERATE_SEED state (state 2 of 10)
         *
         * @details Generates a new seed value using LFSR algorithm when alias conflict
         * is detected. This state is normally skipped during initial login.
         *
         * State responsibilities:
         * - Apply LFSR algorithm to generate new seed
         * - Overwrite previous seed value
         * - Transition to RUNSTATE_GENERATE_ALIAS
         *
         * This state is entered when:
         * - Alias conflict detected during CID sequence
         * - Need to generate different alias from previous attempt
         *
         * Typical implementation: CanLoginMessageHandler_state_generate_seed
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Only used when conflicts occur, not during normal login
         */
    void (*state_generate_seed)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for GENERATE_ALIAS state (state 3 of 10)
         *
         * @details Generates 12-bit alias from current seed using LFSR algorithm and
         * registers the alias/Node ID mapping as tentative (not yet permitted).
         *
         * State responsibilities:
         * - Extract 12-bit alias from seed via LFSR
         * - Ensure alias is non-zero (0x001-0xFFF range)
         * - Register tentative alias mapping
         * - Invoke on_alias_change callback if provided
         * - Transition to RUNSTATE_LOAD_CHECK_ID_07
         *
         * This state is entered:
         * - After INIT state (normal login)
         * - After GENERATE_SEED state (conflict recovery)
         *
         * Typical implementation: CanLoginMessageHandler_state_generate_alias
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Alias value must be in range 0x001-0xFFF (zero not allowed)
         */
    void (*state_generate_alias)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for LOAD_CHECK_ID_07 state (state 4 of 10)
         *
         * @details Constructs CID7 (Check ID frame 7) containing bits 47-36 of the
         * Node ID in the CAN header. First of four CID frames.
         *
         * State responsibilities:
         * - Build CID7 frame with Node ID bits 47-36
         * - Set payload_count to 0 (no data bytes)
         * - Set login_outgoing_can_msg_valid flag
         * - Transition to RUNSTATE_LOAD_CHECK_ID_06
         *
         * Frame format:
         * - CAN header contains MMM=7 and bits 47-36 of Node ID
         * - No payload data bytes
         *
         * Typical implementation: CanLoginMessageHandler_state_load_cid07
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note CID frames announce Node ID fragments to detect conflicts
         */
    void (*state_load_cid07)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for LOAD_CHECK_ID_06 state (state 5 of 10)
         *
         * @details Constructs CID6 (Check ID frame 6) containing bits 35-24 of the
         * Node ID in the CAN header. Second of four CID frames.
         *
         * State responsibilities:
         * - Build CID6 frame with Node ID bits 35-24
         * - Set payload_count to 0 (no data bytes)
         * - Set login_outgoing_can_msg_valid flag
         * - Transition to RUNSTATE_LOAD_CHECK_ID_05
         *
         * Frame format:
         * - CAN header contains MMM=6 and bits 35-24 of Node ID
         * - No payload data bytes
         *
         * Typical implementation: CanLoginMessageHandler_state_load_cid06
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*state_load_cid06)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for LOAD_CHECK_ID_05 state (state 6 of 10)
         *
         * @details Constructs CID5 (Check ID frame 5) containing bits 23-12 of the
         * Node ID in the CAN header. Third of four CID frames.
         *
         * State responsibilities:
         * - Build CID5 frame with Node ID bits 23-12
         * - Set payload_count to 0 (no data bytes)
         * - Set login_outgoing_can_msg_valid flag
         * - Transition to RUNSTATE_LOAD_CHECK_ID_04
         *
         * Frame format:
         * - CAN header contains MMM=5 and bits 23-12 of Node ID
         * - No payload data bytes
         *
         * Typical implementation: CanLoginMessageHandler_state_load_cid05
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*state_load_cid05)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for LOAD_CHECK_ID_04 state (state 7 of 10)
         *
         * @details Constructs CID4 (Check ID frame 4) containing bits 11-0 of the
         * Node ID in the CAN header. Fourth and final CID frame. Also resets the
         * node's timer for the mandatory 200ms wait period.
         *
         * State responsibilities:
         * - Build CID4 frame with Node ID bits 11-0
         * - Set payload_count to 0 (no data bytes)
         * - Reset timerticks to 0 for 200ms wait
         * - Set login_outgoing_can_msg_valid flag
         * - Transition to RUNSTATE_WAIT_200ms
         *
         * Frame format:
         * - CAN header contains MMM=4 and bits 11-0 of Node ID
         * - No payload data bytes
         *
         * Typical implementation: CanLoginMessageHandler_state_load_cid04
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Timer reset prepares for mandatory 200ms wait before RID
         */
    void (*state_load_cid04)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for WAIT_200ms state (state 8 of 10)
         *
         * @details Enforces the mandatory 200ms wait period required by the OpenLCB
         * CAN Frame Transfer Standard between CID4 and RID frames. Uses the node's
         * timerticks counter incremented by 100ms timer.
         *
         * State responsibilities:
         * - Wait until timerticks exceeds 2 (200ms elapsed)
         * - Transition to RUNSTATE_LOAD_RESERVE_ID when wait completes
         * - Do not set valid flag (no message to transmit)
         *
         * This wait period allows:
         * - Other nodes to detect alias conflicts
         * - Network to process all four CID frames
         * - Time for AMR/error frames if conflicts exist
         *
         * Typical implementation: CanLoginMessageHandler_state_wait_200ms
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Requires OpenLcbNode_100ms_timer_tick to be running
         * @note OpenLCB spec mandates minimum 200ms wait
         */
    void (*state_wait_200ms)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for LOAD_RESERVE_ID state (state 9 of 10)
         *
         * @details Constructs RID (Reserve ID) frame to claim the alias after successful
         * conflict checking. Indicates no conflicts detected during CID sequence and wait.
         *
         * State responsibilities:
         * - Build RID frame
         * - Set payload_count to 0 (no data bytes)
         * - Set login_outgoing_can_msg_valid flag
         * - Transition to RUNSTATE_LOAD_ALIAS_MAP_DEFINITION
         *
         * Frame format:
         * - CAN header 0x0700 + alias
         * - No payload data bytes
         *
         * After RID transmission:
         * - Alias is reserved (but not yet permitted)
         * - Ready to send AMD frame
         *
         * Typical implementation: CanLoginMessageHandler_state_load_rid
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note RID indicates successful alias allocation
         */
    void (*state_load_rid)(can_statemachine_info_t *can_statemachine_info);

        /**
         * @brief Handler for LOAD_ALIAS_MAP_DEFINITION state (state 10 of 10)
         *
         * @details Constructs AMD (Alias Map Definition) frame containing the full
         * 48-bit Node ID. Marks the node as permitted and completes the CAN login
         * sequence. This is the final state in the login process.
         *
         * State responsibilities:
         * - Build AMD frame with full 6-byte Node ID
         * - Set payload_count to 6
         * - Set node permitted flag to true
         * - Set alias mapping is_permitted flag to true
         * - Set login_outgoing_can_msg_valid flag
         * - Transition to RUNSTATE_LOAD_INITIALIZATION_COMPLETE
         *
         * Frame format:
         * - CAN header 0x0701 + alias
         * - Payload contains 6 bytes of Node ID
         *
         * After AMD transmission:
         * - Node is permitted on CAN network
         * - Can send/receive OpenLCB messages
         * - Alias mapping is complete and valid
         * - Ready for OpenLCB login sequence
         *
         * Typical implementation: CanLoginMessageHandler_state_load_amd
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Final state in CAN login - node transitions to OpenLCB login
         * @note After this, node can send OpenLCB messages on network
         */
    void (*state_load_amd)(can_statemachine_info_t *can_statemachine_info);

} interface_can_login_state_machine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Login State Machine
     *
     * @details Registers the application's state handler interface with the CAN login
     * state machine. The interface must contain valid function pointers for all 10
     * state handlers required to complete the alias allocation sequence. Must be called
     * during system initialization before any login sequence processing begins.
     *
     * Use cases:
     * - Called once during application initialization
     * - Must be called before CanLoginStateMachine_run
     * - Registers handlers before nodes begin login
     *
     * @param interface_can_login_state_machine Pointer to callback interface structure
     * containing all 10 required state handler function pointers
     *
     * @warning interface_can_login_state_machine must remain valid for lifetime of application
     * @warning All 10 state handler function pointers must be non-NULL
     * @warning NOT thread-safe - call during single-threaded initialization only
     *
     * @attention Call after CanLoginMessageHandler_initialize
     * @attention Call before any nodes attempt login
     * @attention All handlers must be valid before this call
     *
     * @see interface_can_login_state_machine_t - Interface structure definition
     * @see CanLoginMessageHandler_initialize - Initialize message handlers first
     * @see CanLoginStateMachine_run - Main state machine execution function
     */
    extern void CanLoginStateMachine_initialize(const interface_can_login_state_machine_t *interface_can_login_state_machine);


    /**
     * @brief Runs the CAN login state machine for one iteration
     *
     * @details Dispatches to the appropriate state handler based on the node's current
     * run_state. Progresses the node through the 10-state login sequence to allocate a
     * CAN alias. Should be called repeatedly from the main application loop until the
     * node reaches permitted state.
     *
     * The state machine operates as follows:
     * 1. Checks node's current run_state
     * 2. Dispatches to corresponding handler from interface
     * 3. Handler performs state-specific operations
     * 4. Handler transitions node to next run_state
     * 5. Returns immediately (non-blocking)
     *
     * State progression:
     * - INIT → GENERATE_ALIAS (normal path, skip GENERATE_SEED)
     * - GENERATE_SEED → GENERATE_ALIAS (conflict recovery only)
     * - GENERATE_ALIAS → LOAD_CHECK_ID_07
     * - LOAD_CHECK_ID_07 → LOAD_CHECK_ID_06
     * - LOAD_CHECK_ID_06 → LOAD_CHECK_ID_05
     * - LOAD_CHECK_ID_05 → LOAD_CHECK_ID_04
     * - LOAD_CHECK_ID_04 → WAIT_200ms
     * - WAIT_200ms → LOAD_RESERVE_ID (after timer expires)
     * - LOAD_RESERVE_ID → LOAD_ALIAS_MAP_DEFINITION
     * - LOAD_ALIAS_MAP_DEFINITION → LOAD_INITIALIZATION_COMPLETE (OpenLCB login begins)
     *
     * Use cases:
     * - Called from main application loop for nodes in login sequence
     * - Called repeatedly until node permitted flag set
     * - Processes one state transition per call
     *
     * @param can_statemachine_info Pointer to state machine context structure containing:
     * - openlcb_node: Node undergoing login
     * - login_outgoing_can_msg: Buffer for CAN control frames
     * - login_outgoing_can_msg_valid: Flag indicating frame ready for transmission
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning openlcb_node pointer within structure must be valid
     * @warning login_outgoing_can_msg buffer must be valid
     * @warning NOT thread-safe - call from single context only
     *
     * @attention Modifies node run_state as it progresses through sequence
     * @attention May set login_outgoing_can_msg_valid flag if frame needs transmission
     * @attention Returns immediately after dispatching to handler (non-blocking)
     *
     * @note Call from main application loop as fast as possible
     * @note Each call processes exactly one state transition
     * @note State machine returns immediately - no blocking waits
     * @note WAIT_200ms state uses timer, not busy-wait
     *
     * @see CanLoginMessageHandler_state_init - First state handler (entry point)
     * @see CanLoginMessageHandler_state_load_amd - Final state handler
     * @see can_statemachine_info_t - State machine context structure
     */
    extern void CanLoginStateMachine_run(can_statemachine_info_t *can_statemachine_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_LOGIN_STATEMACHINE__ */
