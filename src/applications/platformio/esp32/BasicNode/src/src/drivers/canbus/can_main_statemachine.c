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
 * @file can_main_statemachine.c
 * @brief Implementation of the main CAN state machine dispatcher.
 *
 * @details Coordinates duplicate alias detection, CAN and login frame transmission,
 * and node enumeration.  Uses a cooperative multitasking pattern — each function
 * does one unit of work and returns so other application code can run.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "can_main_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_buffer_store.h"

// Note: This include provides access to openlcb_node_t structure for _reset_node()
// Used when handling duplicate alias errors
#include "../../openlcb/openlcb_node.h"



static interface_can_main_statemachine_t *_interface;

static can_statemachine_info_t _can_statemachine_info;
static can_msg_t _can_msg;

    /**
     * @brief Initializes the CAN main state machine.
     *
     * @details Stores the interface pointer, clears the static login frame buffer,
     * links it to the state machine context, and zeroes all context flags.
     *
     * @verbatim
     * @param interface_can_main_statemachine Pointer to the populated dependency interface.
     * @endverbatim
     *
     * @warning Must be called once at startup after CanBufferStore_initialize().
     */
void CanMainStatemachine_initialize(const interface_can_main_statemachine_t *interface_can_main_statemachine) {

    _interface = (interface_can_main_statemachine_t*) interface_can_main_statemachine;

    CanUtilities_clear_can_message(&_can_msg);

    _can_statemachine_info.login_outgoing_can_msg = &_can_msg;
    _can_statemachine_info.openlcb_node = NULL;
    _can_statemachine_info.login_outgoing_can_msg_valid = false;
    _can_statemachine_info.enumerating = false;
    _can_statemachine_info.outgoing_can_msg = NULL;

}

    /**
     * @brief Resets a node to force it through alias reallocation from GENERATE_SEED.
     *
     * @details Clears alias, all state flags, frees any pending datagram, and sets
     * run_state to RUNSTATE_GENERATE_SEED. Safe to call with NULL.
     *
     * @verbatim
     * @param openlcb_node Node to reset. NULL is safely ignored.
     * @endverbatim
     *
     * @see CanMainStatemachine_handle_duplicate_aliases
     */
static void _reset_node(openlcb_node_t *openlcb_node) {

    if (!openlcb_node) {

        return;

    }

    openlcb_node->alias = 0x00;
    openlcb_node->state.permitted = false;
    openlcb_node->state.initialized = false;
    openlcb_node->state.duplicate_id_detected = false;
    openlcb_node->state.firmware_upgrade_active = false;
    openlcb_node->state.resend_datagram = false;
    openlcb_node->state.openlcb_datagram_ack_sent = false;
    if (openlcb_node->last_received_datagram) {

        OpenLcbBufferStore_free_buffer(openlcb_node->last_received_datagram);
        openlcb_node->last_received_datagram = NULL;

    }

    openlcb_node->state.run_state = RUNSTATE_GENERATE_SEED; // Re-log in with a new generated Alias

}

    /**
     * @brief Scans the alias table for duplicate entries and resets each affected node.
     *
     * @details Algorithm:
     * -# Iterate all ALIAS_MAPPING_BUFFER_DEPTH entries; for each with is_duplicate set:
     *    unregister the alias, find the owning node, call _reset_node().
     * -# Clear the has_duplicate_alias flag.
     * -# Return true if any were found.
     *
     * @verbatim
     * @param alias_mapping_info Pointer to the alias mapping table.
     * @endverbatim
     *
     * @return true if at least one duplicate was resolved, false if none.
     */
static bool _process_duplicate_aliases(alias_mapping_info_t *alias_mapping_info) {

    bool result = false;

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        uint16_t alias = alias_mapping_info->list[i].alias;

        if ((alias > 0) && alias_mapping_info->list[i].is_duplicate) {

            _interface->alias_mapping_unregister(alias);

            _reset_node(_interface->openlcb_node_find_by_alias(alias));

            result = true;

        }

    }

    alias_mapping_info->has_duplicate_alias = false;

    return result;

}

    /** @brief Returns a pointer to the internal state machine context (for testing/debugging). */
can_statemachine_info_t *CanMainStateMachine_get_can_statemachine_info(void) {

    return (&_can_statemachine_info);

}

    /**
     * @brief Checks for the has_duplicate_alias flag; resolves any duplicates found.
     *
     * @details Locks shared resources, reads the flag, calls _process_duplicate_aliases()
     * if needed, then unlocks.
     *
     * @return true if duplicates were found and processed, false if none.
     */
bool CanMainStatemachine_handle_duplicate_aliases(void) {

    bool result = false;

    _interface->lock_shared_resources();

    alias_mapping_info_t *alias_mapping_info = _interface->alias_mapping_get_alias_mapping_info();

    if (alias_mapping_info->has_duplicate_alias) {

        _process_duplicate_aliases(alias_mapping_info);

        result = true;

    }

    _interface->unlock_shared_resources();

    return result;

}

    /**
     * @brief Pops and transmits one CAN frame from the outgoing FIFO.
     *
     * @details Algorithm:
     * -# If no frame is held in the working slot, pop one from the FIFO (under lock).
     * -# If a frame is held, try to send it; free it (under lock) only on success.
     * -# Return true if a frame was pending (sent or not), false if FIFO was empty.
     *
     * @return true if a frame was pending, false if the FIFO was empty.
     *
     * @note The frame is retried on the next call if the hardware buffer was busy.
     */
bool CanMainStatemachine_handle_outgoing_can_message(void) {

    if (!_can_statemachine_info.outgoing_can_msg) {

        _interface->lock_shared_resources();
        _can_statemachine_info.outgoing_can_msg = CanBufferFifo_pop();
        _interface->unlock_shared_resources();

    }

    if (_can_statemachine_info.outgoing_can_msg) {

        if (_interface->send_can_message(_can_statemachine_info.outgoing_can_msg)) {

            _interface->lock_shared_resources();
            CanBufferStore_free_buffer(_can_statemachine_info.outgoing_can_msg);
            _interface->unlock_shared_resources();

            _can_statemachine_info.outgoing_can_msg = NULL;

        }

        return true; // done for this loop, try again next time

    }

    return false;

}

    /**
     * @brief Transmits the pending login frame (CID/RID/AMD) if one is flagged as valid.
     *
     * @details Clears login_outgoing_can_msg_valid only after successful transmission.
     * Returns true if a login frame was pending (sent or retried), false if none.
     */
bool CanMainStatemachine_handle_login_outgoing_can_message(void) {

    if (_can_statemachine_info.login_outgoing_can_msg_valid) {

        if (_interface->send_can_message(_can_statemachine_info.login_outgoing_can_msg)) {

            _can_statemachine_info.login_outgoing_can_msg_valid = false;

        }

        return true; // done for this loop, try again next time

    }

    return false;

}

    /**
     * @brief Starts node enumeration by fetching and processing the first node.
     *
     * @details Returns false if enumeration is already active (node pointer non-NULL).
     * Otherwise fetches the first node, runs its login state machine if still logging in,
     * and returns true.
     *
     * @return true if enumeration was started (or no nodes exist), false if already active.
     */
bool CanMainStatemachine_handle_try_enumerate_first_node(void) {

    if (!_can_statemachine_info.openlcb_node) {

        _can_statemachine_info.openlcb_node = _interface->openlcb_node_get_first(CAN_STATEMACHINE_NODE_ENUMRATOR_KEY);

        if (!_can_statemachine_info.openlcb_node) {

            return true; // done, nothing to do

        }

        // Need to make sure the correct state-machine is run depending of if the Node had finished the login process

        if (_can_statemachine_info.openlcb_node->state.run_state < RUNSTATE_LOAD_INITIALIZATION_COMPLETE) {

            _interface->login_statemachine_run(&_can_statemachine_info);

        }

        return true; // done

    }

    return false;

}

    /**
     * @brief Advances node enumeration to the next node.
     *
     * @details Fetches the next node and runs its login state machine if still logging in.
     * Returns true when there are no more nodes (enumeration complete), false otherwise.
     *
     * @return true when all nodes have been processed, false if more nodes remain.
     */
bool CanMainStatemachine_handle_try_enumerate_next_node(void) {

    _can_statemachine_info.openlcb_node = _interface->openlcb_node_get_next(CAN_STATEMACHINE_NODE_ENUMRATOR_KEY);

    if (!_can_statemachine_info.openlcb_node) {

        return true; // done, nothing to do

    }

    // Need to make sure the correct state-machine is run depending of if the Node had finished the login process

    if (_can_statemachine_info.openlcb_node->state.run_state < RUNSTATE_LOAD_INITIALIZATION_COMPLETE) {

        _interface->login_statemachine_run(&_can_statemachine_info);

    }

    return false;

}

    /**
     * @brief Executes one cooperative iteration of the main CAN state machine.
     *
     * @details Calls each handler in priority order, returning after the first one
     * that does work. Priority: duplicate aliases → outgoing CAN frame →
     * login frame → enumerate first node → enumerate next node.
     */
void CanMainStateMachine_run(void) {

    if (_interface->handle_duplicate_aliases()) {

        return;

    }

    if (_interface->handle_outgoing_can_message()) {

        return;

    }

    if (_interface->handle_login_outgoing_can_message()) {

        return;

    }

    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}
