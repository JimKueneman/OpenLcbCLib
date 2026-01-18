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
* @file openlcb_login_statemachine.c
* @brief Implementation of the login state machine dispatcher
*
* @details This module implements the main dispatcher for the OpenLCB login state machine.
* It manages the complete node initialization sequence by coordinating:
* - Node enumeration across all virtual nodes in the system
* - State-based dispatching to appropriate message handlers
* - Message transmission and retry logic
* - Re-enumeration control for multi-message sequences
*
* Architecture Overview:
* The state machine uses a polling architecture where OpenLcbLoginMainStatemachine_run()
* is called repeatedly from the main application loop. Each call performs one atomic
* operation and returns immediately, maintaining a non-blocking design.
*
* Processing Flow (per call to run function):
* -# Check for pending outgoing message and attempt transmission
* -# If enumerate flag set, re-enter current handler for next message
* -# If no current node, get first node from enumeration list
* -# If current node exists, get next node from enumeration list
* -# Process node if run_state indicates initialization in progress
*
* State Dispatch Logic:
* The process function examines the node's run_state and calls the appropriate
* handler from the interface:
* - RUNSTATE_LOAD_INITIALIZATION_COMPLETE -> load_initialization_complete()
* - RUNSTATE_LOAD_PRODUCER_EVENTS -> load_producer_events()
* - RUNSTATE_LOAD_CONSUMER_EVENTS -> load_consumer_events()
* - RUNSTATE_RUN or higher -> skip (already initialized)
*
* Multi-Message Sequences:
* For nodes with multiple producer or consumer events, handlers set the enumerate
* flag to trigger re-entry without advancing to the next node. This allows sending
* multiple messages in sequence while maintaining the non-blocking design.
*
* Internal State:
* The module maintains static state including:
* - Interface function pointers (set during initialization)
* - State machine info structure (current node, outgoing message buffer, flags)
* - Outgoing message buffer (pre-allocated, reused for all messages)
*
* @author Jim Kueneman
* @date 17 Jan 2026
*
* @see openlcb_login_statemachine_handler.c - Message construction handlers
* @see OpenLCB Message Network Standard S-9.7.3.1 - Initialization Complete
* @see OpenLCB Event Transport Standard S-9.7.4 - Event Transport Protocol
*/

#include "openlcb_login_statemachine.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_utilities.h"


    /** @brief Static pointer to interface callbacks for state machine operations */
static const interface_openlcb_login_state_machine_t *_interface;

    /** @brief Static state machine info structure maintaining current state and message buffer */
static openlcb_login_statemachine_info_t _statemachine_info;


    /**
    * @brief Initializes the login state machine with callback interface
    *
    * @details Algorithm:
    * -# Stores interface pointer in static memory for callbacks
    * -# Links outgoing message buffer pointers in state machine info
    * -# Sets message pointer to embedded buffer structure
    * -# Sets payload pointer to embedded payload structure
    * -# Sets payload type to BASIC (standard message size)
    * -# Clears message structure to zeros
    * -# Clears message payload to zeros
    * -# Marks message buffer as allocated (ready for use)
    * -# Sets current node pointer to NULL (no node being processed)
    *
    * Memory Layout:
    * The state machine info contains an embedded openlcb_msg_buffer_t which holds
    * both the message structure and payload buffer. The pointers are set up to
    * reference these embedded structures for efficient memory use.
    *
    * Buffer Lifecycle:
    * The message buffer is allocated once during initialization and reused for
    * all login messages. It is never freed as it is statically allocated within
    * the state machine info structure.
    *
    * Use cases:
    * - Called once during application startup
    * - Must be called before any nodes begin login sequence
    * - Must be called after message handler initialization
    *
    * @verbatim
    * @param interface_openlcb_login_state_machine Pointer to interface structure
    * @endverbatim
    *
    * @warning MUST be called exactly once during initialization
    * @warning The interface pointer is stored in static memory - the pointed-to
    *          structure must remain valid for the lifetime of the program
    * @warning NOT thread-safe
    *
    * @attention Call during single-threaded initialization phase
    * @attention All interface function pointers must be valid (non-NULL)
    * @attention Call after OpenLcbLoginMessageHandler_initialize()
    *
    * @note The message buffer allocated flag is set to prevent accidental deallocation
    *
    * @see interface_openlcb_login_state_machine_t - Interface structure definition
    * @see openlcb_login_statemachine_info_t - State machine info structure
    * @see OpenLcbLoginMainStatemachine_run - Main processing loop
    * @see OpenLcbUtilities_clear_openlcb_message - Message clearing utility
    */
void OpenLcbLoginStateMachine_initialize(const interface_openlcb_login_state_machine_t *interface_openlcb_login_state_machine) {

    _interface = interface_openlcb_login_state_machine;

    _statemachine_info.outgoing_msg_info.msg_ptr = &_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload = (openlcb_payload_t *) _statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload_type = BASIC;
    OpenLcbUtilities_clear_openlcb_message(_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_statemachine_info.outgoing_msg_info.msg_ptr);
    _statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _statemachine_info.openlcb_node = NULL;

}

    /**
    * @brief Dispatches to appropriate handler based on node's run_state
    *
    * @details Algorithm:
    * -# Examines openlcb_node->state.run_state value
    * -# Switches on run_state to determine which handler to call
    * -# For RUNSTATE_LOAD_INITIALIZATION_COMPLETE: Calls load_initialization_complete()
    * -# For RUNSTATE_LOAD_PRODUCER_EVENTS: Calls load_producer_events()
    * -# For RUNSTATE_LOAD_CONSUMER_EVENTS: Calls load_consumer_events()
    * -# For all other states (including RUNSTATE_RUN): Returns without action
    *
    * State-to-Handler Mapping:
    * - RUNSTATE_LOAD_INITIALIZATION_COMPLETE (0x01) -> load_initialization_complete()
    *   Generates Initialization Complete message (MTI 0x0100 or 0x0101)
    *
    * - RUNSTATE_LOAD_PRODUCER_EVENTS (0x02) -> load_producer_events()
    *   Generates Producer Identified messages (MTI 0x0914/0x0915/0x0917)
    *   May be called multiple times via re-enumeration
    *
    * - RUNSTATE_LOAD_CONSUMER_EVENTS (0x03) -> load_consumer_events()
    *   Generates Consumer Identified messages (MTI 0x04C4/0x04C5/0x04C7)
    *   May be called multiple times via re-enumeration
    *
    * Handler Responsibilities:
    * Each handler is responsible for:
    * - Constructing the appropriate OpenLCB message
    * - Setting the valid flag to trigger transmission
    * - Setting the enumerate flag if more messages are needed
    * - Advancing the node to the next run_state when complete
    *
    * This function is exposed for unit testing but is normally called internally
    * by the main state machine loop.
    *
    * Use cases:
    * - Called internally by handle_try_enumerate_first_node()
    * - Called internally by handle_try_enumerate_next_node()
    * - Called internally by handle_try_reenumerate()
    * - Called directly in unit tests
    *
    * @verbatim
    * @param openlcb_statemachine_info Pointer to state machine info with node and message buffer
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Node pointer within info structure must be valid
    *
    * @note This is primarily an internal function exposed for testing
    * @note Does nothing if node run_state is not one of the handled states
    * @note Handlers may modify the node's run_state to advance the sequence
    *
    * @see interface_openlcb_login_state_machine_t - Handler callbacks
    * @see RUNSTATE_LOAD_INITIALIZATION_COMPLETE in openlcb_defines.h
    * @see RUNSTATE_LOAD_PRODUCER_EVENTS in openlcb_defines.h
    * @see RUNSTATE_LOAD_CONSUMER_EVENTS in openlcb_defines.h
    * @see OpenLcbLoginMessageHandler_load_initialization_complete
    * @see OpenLcbLoginMessageHandler_load_producer_event
    * @see OpenLcbLoginMessageHandler_load_consumer_event
    */
void OpenLcbLoginStateMachine_process(openlcb_login_statemachine_info_t *openlcb_statemachine_info) {

    switch (openlcb_statemachine_info->openlcb_node->state.run_state) {

        case RUNSTATE_LOAD_INITIALIZATION_COMPLETE:

            _interface->load_initialization_complete(openlcb_statemachine_info);

            return;

        case RUNSTATE_LOAD_PRODUCER_EVENTS:

            _interface->load_producer_events(openlcb_statemachine_info);

            return;

        case RUNSTATE_LOAD_CONSUMER_EVENTS:

            _interface->load_consumer_events(openlcb_statemachine_info);

            return;

        default:

            return;

    }

}

    /**
    * @brief Handles transmission of pending outgoing message
    *
    * @details Algorithm:
    * -# Checks if outgoing_msg_info.valid flag is set
    * -# If not valid, returns false (no message to send)
    * -# Calls interface->send_openlcb_msg() with message pointer
    * -# If send returns true (message queued successfully):
    *    - Clears valid flag (message sent, no longer pending)
    * -# If send returns false (transmit queue full):
    *    - Keeps valid flag set (message still pending, retry later)
    * -# Returns true (message was pending, whether sent or not)
    *
    * Message Transmission Flow:
    * The valid flag serves as a semaphore indicating that a message is ready to send.
    * The state machine will repeatedly call this function until the message is
    * successfully transmitted. Once sent, the valid flag is cleared and the state
    * machine can proceed to the next step.
    *
    * Retry Logic:
    * If the transmit queue is full (send returns false), the function keeps the
    * valid flag set and returns true. This causes the main loop to retry on the
    * next iteration without processing any other nodes or generating new messages.
    *
    * Priority:
    * This function is called first in the main loop, ensuring that pending messages
    * are transmitted before any new messages are generated. This prevents message
    * buffer overflow and maintains correct sequencing.
    *
    * This function is exposed for unit testing but is normally called internally
    * by the main state machine loop.
    *
    * Use cases:
    * - Called as first step in OpenLcbLoginMainStatemachine_run()
    * - Called directly in unit tests to verify transmission logic
    *
    * @return true if a message was pending (sent or queued for retry), false if no message pending
    *
    * @note This is primarily an internal function exposed for testing
    * @note Returning true indicates state machine should retry on next iteration
    * @note The message buffer is reused, so valid flag must be cleared after transmission
    *
    * @see interface_openlcb_login_state_machine_t - send_openlcb_msg callback
    * @see OpenLcbLoginMainStatemachine_run - Main loop caller
    * @see openlcb_login_statemachine_info_t - Contains outgoing_msg_info structure
    */
bool OpenLcbLoginStatemachine_handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg_info.msg_ptr)) {

            _statemachine_info.outgoing_msg_info.valid = false; // done

        }

        return true; // keep trying till it can get sent

    }

    return false;

}

    /**
    * @brief Handles re-enumeration for multi-message sequences
    *
    * @details Algorithm:
    * -# Checks if outgoing_msg_info.enumerate flag is set
    * -# If not set, returns false (enumeration complete for this node)
    * -# Calls interface->process_login_statemachine() to re-enter handler
    * -# Returns true (continue enumeration, don't advance to next node)
    *
    * Re-enumeration Purpose:
    * The enumerate flag allows handlers to send multiple messages in sequence
    * without the state machine advancing to the next node. This is used for:
    * - Sending Producer Identified for each produced event
    * - Sending Consumer Identified for each consumed event
    *
    * How It Works:
    * When a handler has more messages to send (e.g., more producer events),
    * it sets the enumerate flag before returning. The state machine detects
    * this flag and calls the handler again instead of moving to the next node.
    * This continues until the handler clears the enumerate flag, indicating
    * all messages in the sequence have been generated.
    *
    * Example Flow for Node with 3 Producer Events:
    * -# Handler called for event 0, sets enumerate=true
    * -# Message sent, enumerate=true detected
    * -# Handler called again for event 1, sets enumerate=true
    * -# Message sent, enumerate=true detected
    * -# Handler called again for event 2, sets enumerate=false (last event)
    * -# Message sent, enumerate=false detected
    * -# State machine advances to next phase
    *
    * This function is exposed for unit testing but is normally called internally
    * by the main state machine loop.
    *
    * Use cases:
    * - Called in OpenLcbLoginMainStatemachine_run() after message transmission
    * - Called directly in unit tests to verify re-enumeration logic
    *
    * @return true if re-enumeration occurred, false if enumeration complete
    *
    * @note This is primarily an internal function exposed for testing
    * @note Handlers control enumeration by setting/clearing the enumerate flag
    * @note Re-enumeration happens on the same node, not advancing to next node
    *
    * @see OpenLcbLoginMessageHandler_load_producer_event - Sets enumerate flag
    * @see OpenLcbLoginMessageHandler_load_consumer_event - Sets enumerate flag
    * @see OpenLcbLoginStateMachine_process - Called for re-enumeration
    */
bool OpenLcbLoginStatemachine_handle_try_reenumerate(void) {

    if (_statemachine_info.outgoing_msg_info.enumerate) {

        _interface->process_login_statemachine(&_statemachine_info); // Continue processing

        return true; // keep going until target clears the enumerate flag

    }

    return false;

}

    /**
    * @brief Attempts to get and process the first node in enumeration
    *
    * @details Algorithm:
    * -# Checks if _statemachine_info.openlcb_node is NULL
    * -# If not NULL, returns false (already have a current node)
    * -# Calls interface->openlcb_node_get_first() with enumerator index
    * -# If returns NULL (no nodes exist):
    *    - Returns true (done, no nodes to process)
    * -# Checks if node->state.run_state < RUNSTATE_RUN
    * -# If node needs initialization:
    *    - Calls interface->process_login_statemachine()
    * -# Returns true (first node handled)
    *
    * First Node Initialization:
    * This function is responsible for starting the node enumeration process.
    * It is called when the state machine has no current node and needs to
    * begin or restart the enumeration sequence.
    *
    * Node Selection Criteria:
    * Nodes are processed if their run_state is less than RUNSTATE_RUN,
    * indicating they are still in the initialization phase:
    * - RUNSTATE_LOAD_INITIALIZATION_COMPLETE (0x01)
    * - RUNSTATE_LOAD_PRODUCER_EVENTS (0x02)
    * - RUNSTATE_LOAD_CONSUMER_EVENTS (0x03)
    *
    * Nodes already in RUNSTATE_RUN or higher are skipped as they have
    * completed the login sequence.
    *
    * Empty List Handling:
    * If no nodes exist (get_first returns NULL), the function returns
    * true to indicate completion. This prevents the state machine from
    * attempting to process non-existent nodes.
    *
    * This function is exposed for unit testing but is normally called internally
    * by the main state machine loop.
    *
    * Use cases:
    * - Called in OpenLcbLoginMainStatemachine_run() to start enumeration
    * - Called directly in unit tests to verify first node logic
    *
    * @return true if first node attempt was made, false if current node already exists
    *
    * @note This is primarily an internal function exposed for testing
    * @note Skips nodes already in RUNSTATE_RUN (already initialized)
    * @note Sets _statemachine_info.openlcb_node to first node
    *
    * @see interface_openlcb_login_state_machine_t - openlcb_node_get_first callback
    * @see OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX in openlcb_defines.h
    * @see RUNSTATE_RUN in openlcb_defines.h
    * @see OpenLcbLoginStateMachine_process - Processes the node
    */
bool OpenLcbLoginStatemachine_handle_try_enumerate_first_node(void) {

    if (!_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = _interface->openlcb_node_get_first(OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) {

            return true; // done

        }

        if (_statemachine_info.openlcb_node->state.run_state < RUNSTATE_RUN) {

            _interface->process_login_statemachine(&_statemachine_info); // Do the processing

        }

        return true; // done

    }

    return false;

}

    /**
    * @brief Attempts to get and process the next node in enumeration
    *
    * @details Algorithm:
    * -# Checks if _statemachine_info.openlcb_node is not NULL
    * -# If NULL, returns false (no current node, can't get next)
    * -# Calls interface->openlcb_node_get_next() with enumerator index
    * -# If returns NULL (end of list reached):
    *    - Returns true (done, enumeration complete)
    * -# Checks if node->state.run_state < RUNSTATE_RUN
    * -# If node needs initialization:
    *    - Calls interface->process_login_statemachine()
    * -# Returns true (next node handled)
    *
    * Node Iteration:
    * This function advances the enumeration to the next node in the list.
    * It is called repeatedly by the main loop to process each node that
    * requires initialization.
    *
    * End of List Detection:
    * When get_next returns NULL, it indicates that all nodes have been
    * examined. The function returns true to signal completion, allowing
    * the main loop to restart enumeration on the next iteration if needed.
    *
    * Automatic Node Skipping:
    * Nodes already in RUNSTATE_RUN or higher are automatically skipped
    * since the run_state check prevents calling the process function.
    * This ensures only nodes requiring initialization are processed.
    *
    * State Persistence:
    * The _statemachine_info.openlcb_node pointer maintains the current
    * position in the enumeration across multiple calls to the main loop.
    * This allows the state machine to pause at any point and resume
    * on the next iteration.
    *
    * This function is exposed for unit testing but is normally called internally
    * by the main state machine loop.
    *
    * Use cases:
    * - Called in OpenLcbLoginMainStatemachine_run() to continue enumeration
    * - Called directly in unit tests to verify next node logic
    *
    * @return true if next node attempt was made, false if no current node exists
    *
    * @note This is primarily an internal function exposed for testing
    * @note Skips nodes already in RUNSTATE_RUN (already initialized)
    * @note When end of list is reached, openlcb_node remains pointing to last node
    *
    * @see interface_openlcb_login_state_machine_t - openlcb_node_get_next callback
    * @see OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX in openlcb_defines.h
    * @see RUNSTATE_RUN in openlcb_defines.h
    * @see OpenLcbLoginStateMachine_process - Processes the node
    */
bool OpenLcbLoginStatemachine_handle_try_enumerate_next_node(void) {

    if (_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = _interface->openlcb_node_get_next(OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) { // reached the end of the list

            return true; // done

        }

        if (_statemachine_info.openlcb_node->state.run_state < RUNSTATE_RUN) {

            _interface->process_login_statemachine(&_statemachine_info); // Do the processing

        }

        return true; // done
    }

    return false;

}

    /**
    * @brief Main state machine processing loop - call repeatedly from application loop
    *
    * @details Algorithm:
    * -# Calls handle_outgoing_openlcb_message() to attempt message transmission
    *    If returns true (message pending), returns immediately to retry
    * -# Calls handle_try_reenumerate() to check for multi-message sequences
    *    If returns true (re-enumeration active), returns immediately to continue
    * -# Calls handle_try_enumerate_first_node() to start or restart enumeration
    *    If returns true (first node handled), returns immediately
    * -# Calls handle_try_enumerate_next_node() to advance to next node
    *    If returns true (next node handled), returns immediately
    *
    * Processing Priority:
    * The function processes steps in strict priority order:
    * 1. Message transmission (highest priority - clear pending messages first)
    * 2. Re-enumeration (continue multi-message sequences on same node)
    * 3. First node enumeration (start new enumeration cycle)
    * 4. Next node enumeration (continue enumeration cycle)
    *
    * Non-Blocking Design:
    * Each call performs exactly one operation and returns immediately.
    * The main application loop calls this function repeatedly, allowing
    * other system tasks to execute between login operations.
    *
    * Enumeration Cycle:
    * The function cycles through all nodes repeatedly. On each pass:
    * - Nodes in RUNSTATE_RUN are skipped (already initialized)
    * - Nodes in initialization states are processed one step
    * - After processing all nodes, enumeration restarts at first node
    *
    * Message Flow Example (2 nodes, each with 1 producer):
    * Call 1: Get first node, generate Init Complete for node 0
    * Call 2: Send Init Complete for node 0
    * Call 3: Generate Producer Identified for node 0
    * Call 4: Send Producer Identified for node 0
    * Call 5: Node 0 complete, get next node (node 1)
    * Call 6: Generate Init Complete for node 1
    * Call 7: Send Init Complete for node 1
    * Call 8: Generate Producer Identified for node 1
    * Call 9: Send Producer Identified for node 1
    * Call 10+: All nodes complete, state machine idles
    *
    * Use cases:
    * - Called continuously from main application loop
    * - Automatically processes all nodes requiring initialization
    * - Continues operating as new nodes are added
    *
    * @warning Must be called from non-interrupt context
    *
    * @attention Call repeatedly in main loop - this function does NOT block
    * @attention Nodes transition themselves through states - this just dispatches
    *
    * @note Non-blocking - returns immediately after one step
    * @note Safe to call even when no nodes need processing
    * @note If message handler needs multiple messages, re-enumerate the same message
    *
    * @see OpenLcbLoginStateMachine_process - State dispatcher
    * @see OpenLcbLoginStatemachine_handle_outgoing_openlcb_message - Message transmission
    * @see OpenLcbLoginStatemachine_handle_try_reenumerate - Re-enumeration control
    * @see OpenLcbLoginStatemachine_handle_try_enumerate_first_node - First node enumeration
    * @see OpenLcbLoginStatemachine_handle_try_enumerate_next_node - Next node enumeration
    */
void OpenLcbLoginMainStatemachine_run(void) {

    // Get any pending message out first
    if (_interface->handle_outgoing_openlcb_message()) {

        return;

    }

    // If message handler needs multiple messages, re-enumerate the same message
    if (_interface->handle_try_reenumerate()) {

        return;

    }

    // Grab the first OpenLcb Node
    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    // Enumerate all the OpenLcb Nodes
    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}

    /**
    * @brief Returns pointer to internal state machine info structure
    *
    * @details Provides read access to the internal state machine info structure.
    * The structure contains:
    * - openlcb_node: Pointer to node currently being processed (NULL if none)
    * - outgoing_msg_info: Message buffer and control flags
    *   - msg_ptr: Pointer to message structure
    *   - valid: Flag indicating message is ready to send
    *   - enumerate: Flag indicating multi-message sequence in progress
    *
    * Testing Usage:
    * Unit tests can use this function to:
    * - Verify current node being processed
    * - Check message buffer contents
    * - Inspect control flags (valid, enumerate)
    * - Validate state transitions
    *
    * Debugging Usage:
    * Developers can use this function to:
    * - Monitor login progress
    * - Inspect pending messages
    * - Diagnose state machine issues
    *
    * Use cases:
    * - Unit testing to verify internal state
    * - Debugging to inspect current state machine status
    * - System monitoring for login progress tracking
    *
    * @return Pointer to internal state machine info structure
    *
    * @warning The returned pointer is to static memory - do not free
    * @warning Modifying the returned structure can cause undefined behavior
    * @warning Do not call from interrupt context
    *
    * @attention Use only for testing and debugging
    * @attention Read-only access recommended - modifications may break state machine
    *
    * @note This is primarily for unit testing
    * @note The structure persists across all calls to the state machine
    * @note Structure contents are only valid after OpenLcbLoginStateMachine_initialize()
    *
    * @see openlcb_login_statemachine_info_t - Structure definition
    * @see OpenLcbLoginStateMachine_initialize - Initializes the structure
    */
openlcb_login_statemachine_info_t *OpenLcbLoginStatemachine_get_statemachine_info(void) {

    return &_statemachine_info;

}
