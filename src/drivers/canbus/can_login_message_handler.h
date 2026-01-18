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
 * @brief A structure to hold pointers to functions for dependencies this module requires \ref can_login_message_handler.h
 *
 * @details OpenLcbCLib uses dependency injection to allow for writing full coverage tests as the
 * functions that are used can be modeled in the test and return valid OR invalid results to fully
 * test all program flows in the module.  It also allows for reducing the program size. If a particular
 * protocol does not need to be implemented simply filling in the dependency for that handler with a NULL
 * will strip out code for that protocols handlers and minimize the application size (bootloader is an example).
 * The library will automatically reply with the correct error/reply codes if the handler is defined as NULL.
 */
typedef struct {
    // REQUIRED FUNCTIONS

    /** @brief Pointer to a function for access into the \ref alias_mappings.h functions to register an Mapping between the passed alias and node ID
     * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_register(). */
    alias_mapping_t *(*alias_mapping_register)(uint16_t alias, node_id_t node_id);

    /** @brief Pointer to a function for access into the \ref alias_mappings.h functions to find a Mapping structure that matches the alias passed to it
     * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_find_mapping_by_alias(). */
    alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

    // OPTIONAL FUNCTION

    // CALLBACK FUNCTIONS

    /** @brief Pointer to a function for an Application to be notified when an Alias has been successfully registered
     *@note <b>Optional</b> application callback.  Defaults to NULL. */
    void (*on_alias_change)(uint16_t alias, node_id_t node_id);

} interface_can_login_message_handler_t;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Login Message Handler module
     *
     * @details Stores the interface pointer for use by all state handler functions.
     * Must be called before any state handlers are invoked.
     *
     * Use cases:
     * - Called once during application initialization
     * - Must be called before login state machine starts
     *
     * @param interface Pointers to function dependencies this module requires
     *
     * @warning Interface pointer must remain valid for lifetime of application
     * @warning NOT thread-safe
     *
     * @attention This must always be called during application initialization
     *
     * @see CanLoginStateMachine_initialize - Initializes the state machine that calls these handlers
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning NOT thread-safe
     *
     * @attention Node seed is set to Node ID value
     * @attention Transitions node to RUNSTATE_GENERATE_ALIAS
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning NOT thread-safe
     *
     * @attention New seed value overwrites previous seed
     * @attention Transitions node to RUNSTATE_GENERATE_ALIAS
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning Alias must not be zero per OpenLCB spec
     * @warning NOT thread-safe
     *
     * @attention Alias is registered as tentative mapping
     * @attention Transitions node to RUNSTATE_LOAD_CHECK_ID_07
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (CID frames have no data bytes)
     * @attention Transitions node to RUNSTATE_LOAD_CHECK_ID_06
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (CID frames have no data bytes)
     * @attention Transitions node to RUNSTATE_LOAD_CHECK_ID_05
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (CID frames have no data bytes)
     * @attention Transitions node to RUNSTATE_LOAD_CHECK_ID_04
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (CID frames have no data bytes)
     * @attention Resets timerticks to 0 for 200ms wait
     * @attention Transitions node to RUNSTATE_WAIT_200ms
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning Requires 100ms timer to be running
     * @warning NOT thread-safe
     *
     * @attention Transitions to RUNSTATE_LOAD_RESERVE_ID when timerticks exceeds 2
     * @attention Does not modify any buffers during wait
     *
     * @note Timer value of 2 represents 200ms (2 x 100ms ticks)
     *
     * @see CanLoginMessageHandler_state_load_rid - Next state in sequence
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
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Sets payload_count to 0 (RID frames have no data bytes)
     * @attention Transitions node to RUNSTATE_LOAD_ALIAS_MAP_DEFINITION
     *
     * @see CanLoginMessageHandler_state_load_amd - Next state in sequence
     */
    extern void CanLoginMessageHandler_state_load_rid(can_statemachine_info_t *can_statemachine_info);


    /**
     * @brief Handles the tenth and final state in the CAN login sequence
     *
     * @details Loads the CAN output buffer with an AMD (Alias Map Definition) frame
     * containing the full 48-bit Node ID. Marks the node as permitted on the network
     * and updates the alias mapping to permitted status.
     *
     * Use cases:
     * - Completes CAN login sequence
     * - Announces full Node ID to network
     * - Transitions node to permitted state
     *
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     *
     * @warning Structure pointer must not be NULL
     * @warning Outgoing message buffer must be valid
     * @warning NOT thread-safe
     *
     * @attention Copies 6-byte Node ID into payload
     * @attention Sets node permitted flag to true
     * @attention Sets alias mapping is_permitted flag to true
     * @attention Transitions node to RUNSTATE_LOAD_INITIALIZATION_COMPLETE
     *
     * @note After this state, node can send OpenLCB messages on CAN network
     *
     * @see CanUtilities_copy_node_id_to_payload - Helper used to copy Node ID
     */
    extern void CanLoginMessageHandler_state_load_amd(can_statemachine_info_t *can_statemachine_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_LOGIN_MESSAGE_HANDLER__ */
