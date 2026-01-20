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
 * @brief Message handlers for CAN login sequence
 *
 * @details Provides state handlers for the 10-state CAN login sequence that
 * allocates a 12-bit alias for a node's 48-bit Node ID. Each handler builds
 * the appropriate CAN control frame (CID, RID, AMD) according to the OpenLCB
 * CAN Frame Transfer Standard.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__
#define __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

    /**
     * @brief Interface structure for CAN login message handler callback functions
     *
     * @details This structure defines the callback interface for the CAN login message
     * handler, which constructs CAN control frames during the alias allocation sequence.
     * The interface provides callbacks for alias mapping management and optional application
     * notification when aliases are successfully allocated.
     *
     * The CAN login sequence requires alias/Node ID mapping functionality to:
     * - Register tentative alias mappings during alias generation
     * - Check for alias conflicts by searching existing mappings
     * - Update mapping status when alias is successfully reserved
     *
     * The login message handler constructs the following CAN control frames in sequence:
     * 1. CID7, CID6, CID5, CID4 (Check ID frames - announce Node ID fragments)
     * 2. RID (Reserve ID frame - claim the alias after 200ms wait)
     * 3. AMD (Alias Map Definition frame - announce full Node ID mapping)
     *
     * All required callbacks must be provided for proper alias allocation and conflict detection.
     * The optional callback allows applications to receive notification when aliases are
     * successfully registered.
     *
     * @note Required callbacks must be set before calling CanLoginMessageHandler_initialize
     * @note All required callbacks are REQUIRED - none can be NULL
     *
     * @see CanLoginMessageHandler_initialize
     * @see can_login_statemachine.h - State machine that invokes these handlers
     */
typedef struct {

        /**
         * @brief Callback to register an alias/Node ID mapping
         *
         * @details This required callback registers a new alias/Node ID pair in the alias
         * mapping table. During the alias generation phase, this callback is invoked to create
         * a tentative mapping that will be marked as permitted after successful AMD transmission.
         *
         * The callback should:
         * - Search for an available slot in the alias mapping table
         * - Store the alias and Node ID in the slot
         * - Initialize mapping flags (is_duplicate, is_permitted)
         * - Return pointer to the created mapping entry
         *
         * Typical implementation: AliasMappings_register
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    alias_mapping_t *(*alias_mapping_register)(uint16_t alias, node_id_t node_id);

        /**
         * @brief Callback to find alias mapping by alias
         *
         * @details This required callback searches the alias mapping table for an entry
         * matching the specified 12-bit alias. Used during conflict detection to determine
         * if another node is using the same alias.
         *
         * The callback should:
         * - Search the alias mapping table for matching alias
         * - Return pointer to mapping entry if found
         * - Return NULL if alias not found in table
         *
         * Typical implementation: AliasMappings_find_mapping_by_alias
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

        /**
         * @brief Optional callback for alias registration notification
         *
         * @details This optional callback provides application notification when an alias
         * has been successfully registered (after AMD frame transmission). Applications can
         * use this to track alias allocations, update displays, or perform logging.
         *
         * The callback receives:
         * - alias: The 12-bit CAN alias that was registered
         * - node_id: The 48-bit Node ID associated with the alias
         *
         * Common uses:
         * - Logging alias allocation events
         * - Updating network monitoring displays
         * - Tracking node login completion
         * - Application-specific bookkeeping
         *
         * @note Optional - can be NULL if notification is not needed
         */
    void (*on_alias_change)(uint16_t alias, node_id_t node_id);

} interface_can_login_message_handler_t;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Login Message Handler module
     *
     * @details Registers the application's callback interface with the login message handler.
     * The interface provides callbacks for alias mapping management and optional application
     * notification. Must be called during system initialization before any login sequence
     * processing begins.
     *
     * Use cases:
     * - Called once during application initialization
     * - Must be called before login state machine starts
     *
     * @param interface Pointer to callback interface structure containing required function pointers
     *
     * @warning interface pointer must remain valid for lifetime of application
     * @warning Both required callbacks must be valid (non-NULL)
     * @warning NOT thread-safe - call during single-threaded initialization only
     *
     * @attention Call before CanLoginStateMachine_initialize
     * @attention Both alias mapping callbacks are required
     *
     * @see interface_can_login_message_handler_t - Interface structure definition
     * @see CanLoginStateMachine_initialize - Initializes state machine that calls these handlers
     */
    extern void CanLoginMessageHandler_initialize(const interface_can_login_message_handler_t *interface);


    /**
     * @brief Handles the first state in the CAN login sequence
     *
     * @details Initializes the node's seed to its Node ID and transitions to the
     * GENERATE_ALIAS state, skipping GENERATE_SEED which is only used when handling
     * alias conflicts.
     *
     * Use cases:
     * - First state called when node begins login sequence
     * - Prepares node for alias generation
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Node pointer within structure must be valid
     * @warning NOT thread-safe
     *
     * @attention Node seed is set to Node ID value
     * @attention Transitions node to RUNSTATE_GENERATE_ALIAS
     *
     * @note First state in normal login sequence
     * @note GENERATE_SEED is skipped unless conflict occurs
     *
     * @see CanLoginMessageHandler_state_generate_alias - Next state in sequence
     * @see CanLoginMessageHandler_state_generate_seed - Used only for alias conflicts
     */
    extern void CanLoginMessageHandler_state_init(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the second state in the CAN login sequence
     *
     * @details Generates a new seed value using an LFSR algorithm and transitions to
     * GENERATE_ALIAS state. This state is only reached when an alias conflict occurs
     * and a new alias must be generated.
     *
     * Use cases:
     * - Called when alias conflict is detected
     * - Generates new seed to produce different alias
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning NOT thread-safe
     *
     * @attention New seed value overwrites previous seed
     * @attention Transitions node to RUNSTATE_GENERATE_ALIAS
     *
     * @note Only used when alias conflict detected
     * @note LFSR ensures different alias on each iteration
     *
     * @see CanLoginMessageHandler_state_generate_alias - Next state in sequence
     */
    extern void CanLoginMessageHandler_state_generate_seed(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the third state in the CAN login sequence
     *
     * @details Generates a 12-bit alias from the current seed using an LFSR algorithm,
     * registers the alias/Node ID mapping, and invokes the optional alias change callback.
     *
     * Use cases:
     * - Generates alias from seed value
     * - Registers tentative alias mapping
     * - Prepares to send CID frames
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Generated alias must not be zero per OpenLCB spec
     * @warning NOT thread-safe
     *
     * @attention Alias is registered as tentative mapping (not yet permitted)
     * @attention Transitions node to RUNSTATE_LOAD_CHECK_ID_07
     * @attention Invokes on_alias_change callback if provided
     *
     * @note Uses LFSR algorithm to generate alias from seed
     * @note Alias value must be in range 0x001-0xFFF (zero not allowed)
     *
     * @see CanLoginMessageHandler_state_load_cid07 - Next state in sequence
     * @see CanLoginMessageHandler_state_generate_seed - Called if conflict detected
     */
    extern void CanLoginMessageHandler_state_generate_alias(can_statemachine_info_t *can_statemachine_info);

    /**
     * @brief Handles the fourth state in the CAN login sequence
     *
     * @details Loads the CAN output buffer with a CID7 frame containing bits 47-36
     * (top 12 bits) of the Node ID. This is the first Check ID frame in the sequence.
     *
     * Use cases:
     * - Sends first CID frame to check for alias conflicts
     * - Announces bits 47-36 of Node ID to network
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (CID frames have no data bytes)
     * @attention Transitions node to RUNSTATE_LOAD_CHECK_ID_06
     * @attention Sets login_outgoing_can_msg_valid flag for transmission
     *
     * @note CID frames carry Node ID fragments in CAN header only
     * @note No payload data bytes in CID frames
     *
     * @see CanLoginMessageHandler_state_load_cid06 - Next state in sequence
     */
    extern void CanLoginMessageHandler_state_load_cid07(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the fifth state in the CAN login sequence
     *
     * @details Loads the CAN output buffer with a CID6 frame containing bits 35-24
     * (second 12 bits) of the Node ID.
     *
     * Use cases:
     * - Sends second CID frame to check for alias conflicts
     * - Announces bits 35-24 of Node ID to network
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (CID frames have no data bytes)
     * @attention Transitions node to RUNSTATE_LOAD_CHECK_ID_05
     * @attention Sets login_outgoing_can_msg_valid flag for transmission
     *
     * @note CID frames carry Node ID fragments in CAN header only
     *
     * @see CanLoginMessageHandler_state_load_cid05 - Next state in sequence
     */
    extern void CanLoginMessageHandler_state_load_cid06(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the sixth state in the CAN login sequence
     *
     * @details Loads the CAN output buffer with a CID5 frame containing bits 23-12
     * (third 12 bits) of the Node ID.
     *
     * Use cases:
     * - Sends third CID frame to check for alias conflicts
     * - Announces bits 23-12 of Node ID to network
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (CID frames have no data bytes)
     * @attention Transitions node to RUNSTATE_LOAD_CHECK_ID_04
     * @attention Sets login_outgoing_can_msg_valid flag for transmission
     *
     * @note CID frames carry Node ID fragments in CAN header only
     *
     * @see CanLoginMessageHandler_state_load_cid04 - Next state in sequence
     */
    extern void CanLoginMessageHandler_state_load_cid05(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the seventh state in the CAN login sequence
     *
     * @details Loads the CAN output buffer with a CID4 frame containing bits 11-0
     * (bottom 12 bits) of the Node ID. Also resets the node's timer for the mandatory
     * 200ms wait period.
     *
     * Use cases:
     * - Sends fourth and final CID frame to check for alias conflicts
     * - Announces bits 11-0 of Node ID to network
     * - Starts 200ms wait timer
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (CID frames have no data bytes)
     * @attention Resets timerticks to 0 for 200ms wait
     * @attention Transitions node to RUNSTATE_WAIT_200ms
     * @attention Sets login_outgoing_can_msg_valid flag for transmission
     *
     * @note Final CID frame completes Node ID announcement
     * @note Timer reset prepares for mandatory 200ms wait
     *
     * @see CanLoginMessageHandler_state_wait_200ms - Next state in sequence
     */
    extern void CanLoginMessageHandler_state_load_cid04(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the eighth state in the CAN login sequence
     *
     * @details Waits for a minimum of 200 milliseconds as required by the OpenLCB
     * CAN Frame Transfer Standard before sending the RID frame. Uses the node's
     * timerticks counter which is incremented by the 100ms timer.
     *
     * Use cases:
     * - Enforces mandatory 200ms wait between CID4 and RID frames
     * - Allows network time to detect alias conflicts
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Requires 100ms timer to be running (OpenLcbNode_100ms_timer_tick)
     * @warning NOT thread-safe
     *
     * @attention Transitions to RUNSTATE_LOAD_RESERVE_ID when timerticks exceeds 2
     * @attention Does not modify any buffers during wait
     * @attention Does not set valid flag (no message to transmit)
     *
     * @note Timer value of 2 represents 200ms (2 × 100ms ticks)
     * @note OpenLCB spec requires minimum 200ms wait
     *
     * @see CanLoginMessageHandler_state_load_rid - Next state in sequence
     * @see OpenLcbNode_100ms_timer_tick - Timer that increments timerticks
     */
    extern void CanLoginMessageHandler_state_wait_200ms(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the ninth state in the CAN login sequence
     *
     * @details Loads the CAN output buffer with an RID (Reserve ID) frame to claim
     * the alias. This indicates no conflicts were detected during the CID sequence
     * and 200ms wait period.
     *
     * Use cases:
     * - Reserves the alias after successful conflict check
     * - Indicates node is ready to send AMD frame
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (RID frames have no data bytes)
     * @attention Transitions node to RUNSTATE_LOAD_ALIAS_MAP_DEFINITION
     * @attention Sets login_outgoing_can_msg_valid flag for transmission
     *
     * @note RID frame has no payload data
     * @note Indicates alias is now reserved (but not yet permitted)
     *
     * @see CanLoginMessageHandler_state_load_amd - Next and final state in sequence
     */
    extern void CanLoginMessageHandler_state_load_rid(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the tenth and final state in the CAN login sequence
     *
     * @details Loads the CAN output buffer with an AMD (Alias Map Definition) frame
     * containing the full 48-bit Node ID. Marks the node as permitted on the network
     * and updates the alias mapping to permitted status. This completes the CAN login
     * sequence and allows the node to send OpenLCB messages.
     *
     * Use cases:
     * - Completes CAN login sequence
     * - Announces full Node ID to network
     * - Transitions node to permitted state
     *
     * @param can_statemachine_info Pointer to state machine context containing node and CAN message buffer
     *
     * @warning can_statemachine_info must NOT be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Copies 6-byte Node ID into payload
     * @attention Sets node permitted flag to true
     * @attention Sets alias mapping is_permitted flag to true
     * @attention Transitions node to RUNSTATE_LOAD_INITIALIZATION_COMPLETE
     * @attention Sets login_outgoing_can_msg_valid flag for transmission
     *
     * @note After this state, node can send OpenLCB messages on CAN network
     * @note Node transitions from CAN login to OpenLCB login
     *
     * @see CanUtilities_copy_node_id_to_payload - Helper used to copy Node ID
     * @see openlcb_login_statemachine.h - OpenLCB login begins after CAN login
     */
    extern void CanLoginMessageHandler_state_load_amd(can_statemachine_info_t *can_statemachine_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__ */
