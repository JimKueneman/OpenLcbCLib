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
 * @brief Implementation of the main CAN state machine dispatcher
 *
 * @details This module coordinates all CAN-level operations including duplicate alias
 * detection, message transmission queuing, login sequence management, and node enumeration.
 * It implements a cooperative multitasking pattern where each function returns after
 * completing one discrete operation, allowing other application code to execute.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
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
     * @brief Initializes the CAN Main State Machine
     *
     * @details Algorithm:
     * -# Store pointer to dependency interface for later use
     * -# Clear login outgoing message buffer to zero
     * -# Link login message buffer to state machine info structure
     * -# Clear all state machine info fields:
     *    - Set openlcb_node to NULL (no active node)
     *    - Set login_outgoing_can_msg_valid to false
     *    - Set enumerating to false
     *    - Set outgoing_can_msg to NULL
     *
     * The login message buffer is statically allocated to avoid runtime allocation
     * overhead during login processing.
     *
     * Use cases:
     * - Called once during application startup
     * - Required before any CAN communication can occur
     * - Must be called after buffer stores are initialized
     *
     * @verbatim
     * @param interface_can_main_statemachine Pointer to populated dependency interface
     * structure containing all required function implementations
     * @endverbatim
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe - call before starting interrupts/threads
     *
     * @attention Call after CanBufferStore_initialize() and CanBufferFifo_initialize()
     * @attention All function pointers in interface must be non-NULL
     *
     * @see CanMainStateMachine_run
     * @see interface_can_main_statemachine_t
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
     * @brief Resets a node to force alias reallocation
     *
     * @details Algorithm:
     * -# Check if node pointer is NULL:
     *    - If NULL, return immediately (defensive programming)
     * -# Clear node alias to 0x00
     * -# Reset all node state flags:
     *    - Set permitted = false (node goes to inhibited state)
     *    - Set initialized = false
     *    - Set duplicate_id_detected = false
     *    - Set firmware_upgrade_active = false
     *    - Set resend_datagram = false
     *    - Set openlcb_datagram_ack_sent = false
     * -# Check if node has pending datagram:
     *    - If last_received_datagram is not NULL, free it to buffer store
     *    - Set last_received_datagram = NULL
     * -# Set run_state to RUNSTATE_GENERATE_SEED to restart login
     *
     * This forces the node to reallocate a new alias through the complete CAN login
     * sequence (seed generation, alias generation, CID frames, RID, AMD).
     *
     * Use cases:
     * - Handling duplicate alias detection
     * - Recovering from alias conflicts
     * - Forcing node reinitialization
     *
     * @verbatim
     * @param openlcb_node Pointer to node structure to reset, may be NULL
     * @endverbatim
     *
     * @note Handles NULL pointer gracefully by returning early
     * @note Node will temporarily go offline during reallocation
     * @note Frees any pending datagram to prevent memory leak
     *
     * @see CanMainStatemachine_handle_duplicate_aliases
     * @see OpenLcbBufferStore_free_buffer
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
     * @brief Processes all entries in alias mapping table with duplicate flag set
     *
     * @details Algorithm:
     * -# Initialize result flag to false
     * -# Iterate through entire alias mapping buffer (0 to ALIAS_MAPPING_BUFFER_DEPTH):
     *    - Get alias from current entry
     *    - Check if alias is valid (> 0) AND is_duplicate flag is set
     *    - If duplicate found:
     *      - Call interface unregister function to remove mapping
     *      - Find node by alias using interface lookup function
     *      - Call _reset_node() to force node reallocation
     *      - Set result flag to true
     * -# Clear global has_duplicate_alias flag
     * -# Return result flag
     *
     * The duplicate alias flag is set by the CAN receive state machine when it detects
     * conflicting alias usage on the bus (same alias used by different Node IDs).
     *
     * Use cases:
     * - Processing alias conflicts detected during reception
     * - Clearing duplicate alias table entries
     * - Forcing affected nodes to reallocate
     *
     * @verbatim
     * @param alias_mapping_info Pointer to alias mapping buffer structure
     * @endverbatim
     *
     * @return true if one or more duplicate aliases were processed, false if none found
     *
     * @note Clears has_duplicate_alias global flag after processing
     * @note Multiple duplicates may be processed in single call
     * @note Each affected node will restart login sequence
     *
     * @see _reset_node
     * @see CanMainStatemachine_handle_duplicate_aliases
     * @see ALIAS_MAPPING_BUFFER_DEPTH
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

    /**
     * @brief Provides read access to internal state machine context
     *
     * @details Algorithm:
     * -# Return pointer to static _can_statemachine_info structure
     *
     * The returned structure contains live state machine context including current
     * node being processed, login message buffer, and outgoing message pointer.
     *
     * Use cases:
     * - Unit test verification of state machine behavior
     * - Debugging state machine operation
     * - Test coverage of edge cases
     *
     * @return Pointer to internal can_statemachine_info_t structure
     *
     * @warning For debugging and testing only - do not modify returned structure
     * @warning NOT thread-safe - lock resources before accessing
     *
     * @note Provides read-only access to live state machine context
     *
     * @see can_statemachine_info_t
     */
can_statemachine_info_t *CanMainStateMachine_get_can_statemachine_info(void) {

    return (&_can_statemachine_info);

}

    /**
     * @brief Handles all detected duplicate alias conflicts
     *
     * @details Algorithm:
     * -# Initialize result to false
     * -# Lock shared resources to prevent concurrent access
     * -# Get alias mapping info structure from interface
     * -# Check if has_duplicate_alias flag is set:
     *    - If set, call _process_duplicate_aliases() to handle them
     *    - Set result to true
     * -# Unlock shared resources
     * -# Return result flag
     *
     * Locking is required because alias mapping table is shared between CAN receive
     * interrupt (which sets duplicate flags) and main loop (which processes them).
     *
     * Use cases:
     * - Called by main state machine run loop
     * - Unit testing of duplicate alias handling
     * - Debugging alias conflicts
     *
     * @return true if duplicate aliases were found and processed, false if none detected
     *
     * @warning Locks shared resources during operation
     * @warning Affected nodes will temporarily go offline during realias
     *
     * @note For testing/debugging - normally called via interface function pointer
     * @note Clears has_duplicate_alias flag after processing
     *
     * @see interface_can_main_statemachine_t.handle_duplicate_aliases
     * @see AliasMappings_unregister
     * @see _process_duplicate_aliases
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
     * @brief Transmits pending outgoing CAN messages from FIFO
     *
     * @details Algorithm:
     * -# Check if working buffer already has a message:
     *    - If NULL, need to pop from FIFO
     *    - Lock shared resources
     *    - Pop message from CAN buffer FIFO
     *    - Unlock shared resources
     *    - Store popped message in working buffer
     * -# Check if working buffer has message:
     *    - Call interface send_can_message() with message
     *    - If transmission successful:
     *      - Lock shared resources
     *      - Free buffer back to CAN buffer store
     *      - Unlock shared resources
     *      - Clear working buffer pointer to NULL
     *    - Return true (message was pending)
     * -# Return false (no message pending)
     *
     * The working buffer prevents message loss if transmission fails due to full
     * hardware buffer. Message remains in working buffer for retry on next call.
     *
     * Use cases:
     * - Called by main state machine run loop
     * - Unit testing of message transmission
     * - Debugging message flow
     *
     * @return true if message was in FIFO (whether sent or not), false if FIFO empty
     *
     * @warning Locks shared resources during FIFO access
     *
     * @note For testing/debugging - normally called via interface function pointer
     * @note Frees buffer only after successful transmission
     * @note May be called multiple times until transmission succeeds
     *
     * @see interface_can_main_statemachine_t.handle_outgoing_can_message
     * @see CanBufferFifo_pop
     * @see CanBufferStore_free_buffer
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
     * @brief Transmits pending login-related CAN messages
     *
     * @details Algorithm:
     * -# Check if login_outgoing_can_msg_valid flag is set:
     *    - If set, message is pending transmission
     *    - Call interface send_can_message() with login message buffer
     *    - If transmission successful:
     *      - Clear login_outgoing_can_msg_valid flag
     *    - Return true (message was pending)
     * -# Return false (no message pending)
     *
     * The login message buffer is statically allocated and reused for each login frame
     * (CID7, CID6, CID5, CID4, RID, AMD). Valid flag indicates message is ready to send.
     *
     * Use cases:
     * - Called by main state machine run loop
     * - Unit testing of login message transmission
     * - Debugging alias allocation sequence
     *
     * @return true if login message was pending (whether sent or not), false if no message pending
     *
     * @note For testing/debugging - normally called via interface function pointer
     * @note Clears valid flag only after successful transmission
     * @note May be called multiple times until transmission succeeds
     *
     * @see interface_can_main_statemachine_t.handle_login_outgoing_can_message
     * @see CanLoginStateMachine_run
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
     * @brief Begins node enumeration and processes first node
     *
     * @details Algorithm:
     * -# Check if currently enumerating a node:
     *    - If openlcb_node is NULL, no active enumeration
     *    - Get first node using interface->openlcb_node_get_first()
     *    - Store node pointer in state machine info
     *    - If no nodes exist (NULL returned):
     *      - Return true (done, nothing to process)
     *    - Check if node is still in login sequence:
     *      - If run_state < RUNSTATE_LOAD_INITIALIZATION_COMPLETE
     *      - Call login state machine via interface
     *    - Return true (first node processed)
     * -# Return false (already enumerating)
     *
     * Node enumeration allows processing multiple virtual nodes. Each node may be
     * in different states (logging in, initialized, running).
     *
     * Use cases:
     * - Called by main state machine run loop
     * - Unit testing of node enumeration
     * - Debugging multi-node operation
     *
     * @return true if first node was found and processed (or none exist), false if enumeration already active
     *
     * @note For testing/debugging - normally called via interface function pointer
     * @note Only processes node if not already enumerating
     * @note Supports multiple virtual nodes up to USER_DEFINED_NODE_BUFFER_DEPTH
     *
     * @see interface_can_main_statemachine_t.handle_try_enumerate_first_node
     * @see CanMainStatemachine_handle_try_enumerate_next_node
     * @see USER_DEFINED_NODE_BUFFER_DEPTH
     * @see CAN_STATEMACHINE_NODE_ENUMRATOR_KEY
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
     * @brief Continues node enumeration to next node
     *
     * @details Algorithm:
     * -# Get next node using interface->openlcb_node_get_next()
     * -# Store node pointer in state machine info
     * -# If no more nodes (NULL returned):
     *    - Return true (enumeration complete)
     * -# Check if node is still in login sequence:
     *    - If run_state < RUNSTATE_LOAD_INITIALIZATION_COMPLETE
     *    - Call login state machine via interface
     * -# Return false (more nodes to process)
     *
     * Called after handle_try_enumerate_first_node() to process remaining nodes in
     * the node pool. Continues until all nodes have been processed.
     *
     * Use cases:
     * - Called by main state machine run loop
     * - Unit testing of node enumeration
     * - Debugging multi-node operation
     *
     * @return true if no more nodes available (enumeration complete), false if more nodes to process
     *
     * @note For testing/debugging - normally called via interface function pointer
     * @note Works in conjunction with handle_try_enumerate_first_node()
     * @note Supports multiple virtual nodes up to USER_DEFINED_NODE_BUFFER_DEPTH
     *
     * @see interface_can_main_statemachine_t.handle_try_enumerate_next_node
     * @see CanMainStatemachine_handle_try_enumerate_first_node
     * @see USER_DEFINED_NODE_BUFFER_DEPTH
     * @see CAN_STATEMACHINE_NODE_ENUMRATOR_KEY
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
     * @brief Executes one iteration of the main CAN state machine
     *
     * @details Algorithm:
     * -# Call handle_duplicate_aliases():
     *    - If returns true (duplicates processed), return
     * -# Call handle_outgoing_can_message():
     *    - If returns true (message transmitted or pending), return
     * -# Call handle_login_outgoing_can_message():
     *    - If returns true (login message transmitted or pending), return
     * -# Call handle_try_enumerate_first_node():
     *    - If returns true (first node processed or none exist), return
     * -# Call handle_try_enumerate_next_node():
     *    - If returns true (no more nodes), return
     *    - If returns false (more nodes exist), will continue on next iteration
     *
     * Each function processes one discrete operation and returns. This cooperative
     * multitasking pattern allows other application code to execute between operations.
     *
     * Priority order ensures critical operations (duplicate alias handling, message
     * transmission) complete before lower-priority operations (node enumeration).
     *
     * Use cases:
     * - Called continuously from main application loop
     * - Cooperative multitasking with other application code
     * - Non-blocking state machine advancement
     *
     * @warning Must be called frequently (as fast as possible in main loop)
     * @warning Assumes CanMainStatemachine_initialize() was already called
     *
     * @note Returns after processing one operation for cooperative multitasking
     * @note Each operation may lock shared resources briefly
     *
     * @see CanMainStatemachine_initialize
     * @see CanMainStatemachine_handle_duplicate_aliases
     * @see CanMainStatemachine_handle_outgoing_can_message
     * @see CanMainStatemachine_handle_login_outgoing_can_message
     * @see CanMainStatemachine_handle_try_enumerate_first_node
     * @see CanMainStatemachine_handle_try_enumerate_next_node
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
