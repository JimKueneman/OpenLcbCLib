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
 * @brief Implementation of the 10-state CAN alias allocation login handlers.
 *
 * @details Includes LFSR-based seed generation and 12-bit alias extraction
 * per the OpenLCB CAN Frame Transfer Standard.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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
     * @brief Stores the interface pointer for use by all state handlers.
     *
     * @verbatim
     * @param interface Pointer to the populated dependency-injection interface.
     * @endverbatim
     */
void CanLoginMessageHandler_initialize(const interface_can_login_message_handler_t *interface) {

    _interface = (interface_can_login_message_handler_t *) interface;

}

    /**
     * @brief Advances a 48-bit seed one step using the OpenLCB LFSR algorithm.
     *
     * @details Splits the seed into two 24-bit halves (lfsr1 = upper, lfsr2 = lower),
     * applies shift-and-add with magic constants 0x1B0CA3 and 0x7A4BA9 per TN §6.1.3,
     * then recombines. Ensures a different alias is produced on each conflict retry.
     *
     * @param start_seed Current 48-bit seed.
     *
     * @return New 48-bit seed.
     *
     * @see CanLoginMessageHandler_state_generate_seed
     */
static uint64_t _generate_seed(uint64_t start_seed) {

    uint32_t lfsr2 = start_seed & 0xFFFFFF;         // lower 24 bits
    uint32_t lfsr1 = (start_seed >> 24) & 0xFFFFFF; // upper 24 bits

    uint32_t temp1 = ((lfsr1 << 9) | ((lfsr2 >> 15) & 0x1FF)) & 0xFFFFFF;
    uint32_t temp2 = (lfsr2 << 9) & 0xFFFFFF;

    lfsr1 = lfsr1 + temp1 + 0x1B0CA3L;
    lfsr2 = lfsr2 + temp2 + 0x7A4BA9L;

    lfsr1 = (lfsr1 & 0xFFFFFF) + ((lfsr2 & 0xFF000000) >> 24);
    lfsr2 = lfsr2 & 0xFFFFFF;

    return ( (uint64_t) lfsr1 << 24) | lfsr2;

}

    /**
     * @brief Extracts a 12-bit alias from a 48-bit seed.
     *
     * @details XORs the two 24-bit halves of the seed and their upper 12 bits,
     * then masks to 12 bits. Returns 0x000-0xFFF; alias 0x000 is invalid per spec.
     *
     * @param seed 48-bit seed value.
     *
     * @return 12-bit alias (0x000-0xFFF).
     *
     * @see CanLoginMessageHandler_state_generate_alias
     */
static uint16_t _generate_alias(uint64_t seed) {

    uint32_t lfsr2 = seed & 0xFFFFFF;
    uint32_t lfsr1 = (seed >> 24) & 0xFFFFFF;

    return ( lfsr1 ^ lfsr2 ^ (lfsr1 >> 12) ^ (lfsr2 >> 12)) & 0x0FFF;

}

    /**
     * @brief State 1: Seeds the PRNG with the Node ID, then jumps directly to GENERATE_ALIAS.
     *
     * @details On first login the Node ID itself is the initial seed, so GENERATE_SEED
     * (which advances the PRNG one step) is skipped. GENERATE_SEED is only entered on
     * alias conflict retry, when _reset_node() sets run_state back to RUNSTATE_GENERATE_SEED.
     *
     * @verbatim
     * @param can_statemachine_info State machine context.
     * @endverbatim
     */
void CanLoginMessageHandler_state_init(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->seed = can_statemachine_info->openlcb_node->id;
    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS; // Skip GENERATE_SEED — only used on alias conflict retry

}

    /** @brief State 2: Advances the seed one LFSR step, then transitions to GENERATE_ALIAS. */
void CanLoginMessageHandler_state_generate_seed(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->seed = _generate_seed(can_statemachine_info->openlcb_node->seed);
    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_GENERATE_ALIAS;

}

    /** @brief State 3: Derives a 12-bit alias from the seed, registers it, and transitions to LOAD_CID07. */
void CanLoginMessageHandler_state_generate_alias(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->alias = _generate_alias(can_statemachine_info->openlcb_node->seed);

    while (can_statemachine_info->openlcb_node->alias == 0) {

        can_statemachine_info->openlcb_node->seed = _generate_seed(can_statemachine_info->openlcb_node->seed);
        can_statemachine_info->openlcb_node->alias = _generate_alias(can_statemachine_info->openlcb_node->seed);

    }

    _interface->alias_mapping_register(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    if (_interface->on_alias_change) {

        _interface->on_alias_change(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    }

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;

}

    /** @brief State 4: Loads a CID7 frame (Node ID bits 47-36) into the outgoing buffer. */
void CanLoginMessageHandler_state_load_cid07(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((can_statemachine_info->openlcb_node->id >> 24) & 0xFFF000) | can_statemachine_info->openlcb_node->alias); // AA0203040506
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_06;

}

    /** @brief State 5: Loads a CID6 frame (Node ID bits 35-24) into the outgoing buffer. */
void CanLoginMessageHandler_state_load_cid06(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((can_statemachine_info->openlcb_node->id >> 12) & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;

}

    /** @brief State 6: Loads a CID5 frame (Node ID bits 23-12) into the outgoing buffer. */
void CanLoginMessageHandler_state_load_cid05(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((can_statemachine_info->openlcb_node->id & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_04;

}

    /** @brief State 7: Loads a CID4 frame (Node ID bits 11-0) and resets the 200 ms timer. */
void CanLoginMessageHandler_state_load_cid04(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((can_statemachine_info->openlcb_node->id << 12) & 0xFFF000) | can_statemachine_info->openlcb_node->alias);
    can_statemachine_info->openlcb_node->timerticks = 0;
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;

}

    /** @brief State 8: Waits until timerticks > 2, then transitions to LOAD_RESERVE_ID. */
void CanLoginMessageHandler_state_wait_200ms(can_statemachine_info_t *can_statemachine_info) {

    if (can_statemachine_info->openlcb_node->timerticks > 2) {

        can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_RESERVE_ID;

    }

}

    /** @brief State 9: Loads an RID frame to claim the alias, then transitions to LOAD_AMD. */
void CanLoginMessageHandler_state_load_rid(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_statemachine_info->openlcb_node->alias;
    can_statemachine_info->login_outgoing_can_msg->payload_count = 0;
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_ALIAS_MAP_DEFINITION;

}

    /** @brief State 10: Loads an AMD frame, marks the node permitted, and updates the alias mapping. */
void CanLoginMessageHandler_state_load_amd(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->login_outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_statemachine_info->openlcb_node->alias;
    CanUtilities_copy_node_id_to_payload(can_statemachine_info->login_outgoing_can_msg, can_statemachine_info->openlcb_node->id, 0);
    can_statemachine_info->login_outgoing_can_msg_valid = true;
    can_statemachine_info->openlcb_node->state.permitted = true;

    alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(can_statemachine_info->openlcb_node->alias);
    alias_mapping->is_permitted = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

}
