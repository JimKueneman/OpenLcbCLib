/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * @file can_login_message_handler.c
 * @brief Implementation of message handlers for CAN login sequence
 *
 * @details Implements all 10 state handlers for the CAN login sequence plus
 * internal LFSR-based algorithms for generating alias seeds and extracting
 * 12-bit aliases from seeds. Follows the OpenLCB CAN Frame Transfer Standard
 * for alias allocation.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "can_login_message_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_utilities.h"


static interface_can_login_message_handler_t *_interface;


    /**
     * @brief Initializes the CAN Login Message Handler module
     *
     * @details Algorithm:
     * -# Cast away const qualifier from interface pointer
     * -# Store interface pointer in static variable
     *
     * Use cases:
     * - Called once during application initialization
     * - Must be called before login state machine starts
     *
     * @verbatim
     * @param interface Pointers to function dependencies this module requires
     * @endverbatim
     *
     * @warning Interface pointer must remain valid for lifetime of application
     * @warning NOT thread-safe
     *
     * @attention This must always be called during application initialization
     *
     * @see CanLoginStateMachine_initialize - Initializes the state machine that calls these handlers
     */
void CanLoginMessageHandler_initialize(const interface_can_login_message_handler_t *interface) {

    _interface = (interface_can_login_message_handler_t *) interface;

}

    /**
     * @brief Generates a new seed value using LFSR algorithm
     *
     * @details Algorithm:
     * -# Split 48-bit seed into two 24-bit LFSR values
     * -# Generate temporary values by shifting and masking
     * -# Add temporary values and magic constants to LFSRs
     * -# Mask to 24 bits and handle carry between LFSRs
     * -# Combine LFSRs back into 48-bit seed
     *
     * Use cases:
     * - Called to generate new alias when conflict detected
     * - Ensures different seed produces different alias
     *
     * @param start_seed Current 48-bit seed value
     *
     * @return New 48-bit seed value
     *
     * @note Uses magic constants 0x1B0CA3L and 0x7A4BA9L per OpenLCB reference implementation
     * @note LFSR algorithm ensures good distribution of alias values
     *
     * @see CanLoginMessageHandler_state_generate_seed - Uses this function
     */
static uint64_t _generate_seed(uint64_t start_seed) {

    uint32_t lfsr1 = start_seed & 0xFFFFFF;
    uint32_t lfsr2 = (start_seed >> 24) & 0xFFFFFF;

    uint32_t temp1 = ((lfsr1 << 9) | ((lfsr2 >> 15) & 0x1FF)) & 0xFFFFFF;
    uint32_t temp2 = (lfsr2 << 9) & 0xFFFFFF;

    lfsr1 = lfsr1 + temp1 + 0x1B0CA3L;
    lfsr2 = lfsr2 + temp2 + 0x7A4BA9L;

    lfsr1 = (lfsr1 & 0xFFFFFF) + ((lfsr2 & 0xFF000000) >> 24);
    lfsr2 = lfsr2 & 0xFFFFFF;

    return ( (uint64_t) lfsr1 << 24) | lfsr2;

}

    /**
     * @brief Generates a 12-bit alias from a 48-bit seed
     *
     * @details Algorithm:
     * -# Split seed into two 24-bit values
     * -# XOR the two values together
     * -# XOR with shifted versions of values
     * -# Mask result to 12 bits
     *
     * Use cases:
     * - Extracts alias from seed value
     * - Ensures alias is within valid range (0x001-0xFFF)
     *
     * @param seed 48-bit seed value
     *
     * @return 12-bit alias value (0x000-0xFFF)
     *
     * @note Returns value in range 0x000-0xFFF
     * @note Alias of 0x000 is invalid per OpenLCB spec
     *
     * @see CanLoginMessageHandler_state_generate_alias - Uses this function
     */
static uint16_t _generate_alias(uint64_t seed) {

    uint32_t lfsr2 = seed & 0xFFFFFF;
    uint32_t lfsr1 = (seed >> 24) & 0xFFFFFF;

    return ( lfsr1 ^ lfsr2 ^ (lfsr1 >> 12) ^ (lfsr2 >> 12)) & 0x0FFF;

}

    /**
     * @brief Handles the first state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Set node seed to node ID value
     * -# Set run_state to RUNSTATE_GENERATE_ALIAS
     * -# Skip GENERATE_SEED state (only used for conflicts)
     *
     * Use cases:
     * - First state called when node begins login sequence
     * - Prepares node for alias generation
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_init(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->seed = can_statemachine_info->openlcb_node->id;
    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS; // Jump over Generate Seed that only is if we have an Alias conflict and have to jump back

}

    /**
     * @brief Handles the second state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Call _generate_seed with current seed
     * -# Store new seed value in node
     * -# Set run_state to RUNSTATE_GENERATE_ALIAS
     *
     * Use cases:
     * - Called when alias conflict is detected
     * - Generates new seed to produce different alias
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
     *
     * @warning Structure pointer must not be NULL
     * @warning NOT thread-safe
     *
     * @attention New seed value overwrites previous seed
     * @attention Transitions node to RUNSTATE_GENERATE_ALIAS
     *
     * @see CanLoginMessageHandler_state_generate_alias - Next state in sequence
     */
void CanLoginMessageHandler_state_generate_seed(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->seed = _generate_seed(can_statemachine_info->openlcb_node->seed);
    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS;

}

    /**
     * @brief Handles the third state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Call _generate_alias with current seed
     * -# Store alias in node structure
     * -# Register alias/Node ID mapping via interface
     * -# If callback registered, invoke on_alias_change
     * -# Set run_state to RUNSTATE_LOAD_CHECK_ID_07
     *
     * Use cases:
     * - Generates alias from seed value
     * - Registers tentative alias mapping
     * - Prepares to send CID frames
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_generate_alias(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->alias = _generate_alias(can_statemachine_info->openlcb_node->seed);

    _interface->alias_mapping_register(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    if (_interface->on_alias_change) {

        _interface->on_alias_change(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    }

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;

}

    /**
     * @brief Handles the fourth state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Set payload_count to 0 (CID frames have no data)
     * -# Build CAN identifier with CID7 format
     * -# Extract bits 47-36 of Node ID
     * -# Combine with alias in identifier
     * -# Set outgoing message valid flag
     * -# Set run_state to RUNSTATE_LOAD_CHECK_ID_06
     *
     * Use cases:
     * - Sends first CID frame to check for alias conflicts
     * - Announces bits 47-36 of Node ID to network
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_load_cid07(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((can_statemachine_info->openlcb_node->id >> 24) & 0xFFF000) | can_statemachine_info->openlcb_node->alias); // AA0203040506
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_06;

}

    /**
     * @brief Handles the fifth state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Set payload_count to 0 (CID frames have no data)
     * -# Build CAN identifier with CID6 format
     * -# Extract bits 35-24 of Node ID
     * -# Combine with alias in identifier
     * -# Set outgoing message valid flag
     * -# Set run_state to RUNSTATE_LOAD_CHECK_ID_05
     *
     * Use cases:
     * - Sends second CID frame to check for alias conflicts
     * - Announces bits 35-24 of Node ID to network
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_load_cid06(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((can_statemachine_info->openlcb_node->id >> 12) & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;

}

    /**
     * @brief Handles the sixth state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Set payload_count to 0 (CID frames have no data)
     * -# Build CAN identifier with CID5 format
     * -# Extract bits 23-12 of Node ID
     * -# Combine with alias in identifier
     * -# Set outgoing message valid flag
     * -# Set run_state to RUNSTATE_LOAD_CHECK_ID_04
     *
     * Use cases:
     * - Sends third CID frame to check for alias conflicts
     * - Announces bits 23-12 of Node ID to network
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_load_cid05(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((can_statemachine_info->openlcb_node->id & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_04;

}

    /**
     * @brief Handles the seventh state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Set payload_count to 0 (CID frames have no data)
     * -# Build CAN identifier with CID4 format
     * -# Extract bits 11-0 of Node ID (shift left by 12)
     * -# Combine with alias in identifier
     * -# Reset timerticks to 0 for 200ms wait
     * -# Set outgoing message valid flag
     * -# Set run_state to RUNSTATE_WAIT_200ms
     *
     * Use cases:
     * - Sends fourth and final CID frame to check for alias conflicts
     * - Announces bits 11-0 of Node ID to network
     * - Starts 200ms wait timer
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_load_cid04(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((can_statemachine_info->openlcb_node->id << 12) & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->openlcb_node->timerticks = 0;
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;

}

    /**
     * @brief Handles the eighth state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Check if timerticks exceeds 2 (200ms elapsed)
     * -# If yes, set run_state to RUNSTATE_LOAD_RESERVE_ID
     * -# If no, remain in current state
     *
     * Use cases:
     * - Enforces mandatory 200ms wait between CID4 and RID frames
     * - Allows network time to detect alias conflicts
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_wait_200ms(can_statemachine_info_t *can_statemachine_info) {

    if (can_statemachine_info->openlcb_node->timerticks > 2) {

        can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_RESERVE_ID;

    }

}

    /**
     * @brief Handles the ninth state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Build CAN identifier with RID format and alias
     * -# Set payload_count to 0 (RID frames have no data)
     * -# Set outgoing message valid flag
     * -# Set run_state to RUNSTATE_LOAD_ALIAS_MAP_DEFINITION
     *
     * Use cases:
     * - Reserves the alias after successful conflict check
     * - Indicates node is ready to send AMD frame
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_load_rid(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_statemachine_info->openlcb_node->alias;
    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_ALIAS_MAP_DEFINITION;

}

    /**
     * @brief Handles the tenth and final state in the CAN login sequence
     *
     * @details Algorithm:
     * -# Build CAN identifier with AMD format and alias
     * -# Copy 6-byte Node ID into payload using utility function
     * -# Set outgoing message valid flag
     * -# Set node permitted flag to true
     * -# Find alias mapping via interface
     * -# Set is_permitted flag in mapping to true
     * -# Set run_state to RUNSTATE_LOAD_INITIALIZATION_COMPLETE
     *
     * Use cases:
     * - Completes CAN login sequence
     * - Announces full Node ID to network
     * - Transitions node to permitted state
     *
     * @verbatim
     * @param can_statemachine_info Pointer to a struct that has the node and buffer for any outgoing
     * message that may be necessary
     * @endverbatim
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
void CanLoginMessageHandler_state_load_amd(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_statemachine_info->openlcb_node->alias;
    CanUtilities_copy_node_id_to_payload(can_statemachine_info->login_outgoing_can_msg, can_statemachine_info->openlcb_node->id, 0);
    can_statemachine_info->login_outgoing_can_msg_valid = true;
    can_statemachine_info->openlcb_node->state.permitted = true;

    alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(can_statemachine_info->openlcb_node->alias);
    alias_mapping->is_permitted = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

}


