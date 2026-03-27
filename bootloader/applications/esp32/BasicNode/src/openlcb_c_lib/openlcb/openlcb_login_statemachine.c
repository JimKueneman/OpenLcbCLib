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
* - RUNSTATE_LOGIN_COMPLETE -> on_login_complete() if set, then RUNSTATE_RUN
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
* @date 18 Mar 2026
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

    /** @brief Second context for sibling dispatch of login outgoing messages.
     *  Uses main statemachine type because receiving siblings process login
     *  messages through the main protocol handlers, not login handlers. */
static openlcb_statemachine_info_t _sibling_statemachine_info;

    /** @brief TRUE while we are iterating siblings for a login outgoing message. */
static bool _sibling_dispatch_active;


    /**
    * @brief Stores the callback interface and wires up the outgoing message buffer.
    *
    * @details Algorithm:
    * -# Store interface pointer
    * -# Link outgoing message buffer pointers in state machine info
    * -# Set payload type to BASIC, clear message and payload
    * -# Mark buffer as allocated, set current node to NULL
    *
    * @verbatim
    * @param interface_openlcb_login_state_machine Pointer to interface structure
    * @endverbatim
    */
void OpenLcbLoginStateMachine_initialize(const interface_openlcb_login_state_machine_t *interface_openlcb_login_state_machine) {

    _interface = interface_openlcb_login_state_machine;

    // Main login context
    _statemachine_info.outgoing_msg_info.msg_ptr = &_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload = (openlcb_payload_t *) _statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload_type = BASIC;
    OpenLcbUtilities_clear_openlcb_message(_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_statemachine_info.outgoing_msg_info.msg_ptr);
    _statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _statemachine_info.openlcb_node = NULL;

    // Sibling context — uses main statemachine type so receiving siblings
    // can process login messages through the main protocol handlers.
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr = &_sibling_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->payload =
            (openlcb_payload_t *) _sibling_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->payload_type = WORKER;
    OpenLcbUtilities_clear_openlcb_message(_sibling_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_sibling_statemachine_info.outgoing_msg_info.msg_ptr);
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _sibling_statemachine_info.incoming_msg_info.msg_ptr = NULL;
    _sibling_statemachine_info.incoming_msg_info.enumerate = false;
    _sibling_statemachine_info.openlcb_node = NULL;

    _sibling_dispatch_active = false;

}

    /**
    * @brief Dispatches to the handler matching node->run_state.
    *
    * @details Algorithm:
    * -# Switch on run_state
    * -# RUNSTATE_LOAD_INITIALIZATION_COMPLETE → load_initialization_complete()
    * -# RUNSTATE_LOAD_PRODUCER_EVENTS → load_producer_events()
    * -# RUNSTATE_LOAD_CONSUMER_EVENTS → load_consumer_events()
    * -# RUNSTATE_LOGIN_COMPLETE → on_login_complete() if set, then RUNSTATE_RUN
    * -# All other states → return without action
    *
    * @verbatim
    * @param openlcb_statemachine_info Pointer to state machine info with node and message buffer
    * @endverbatim
    */
void OpenLcbLoginStateMachine_process(openlcb_login_statemachine_info_t *openlcb_statemachine_info) {

    switch (openlcb_statemachine_info->openlcb_node->state.run_state) {

        case RUNSTATE_LOAD_INITIALIZATION_COMPLETE:

            _interface->load_initialization_complete(openlcb_statemachine_info);

            return;

        case RUNSTATE_LOAD_CONSUMER_EVENTS:

            _interface->load_consumer_events(openlcb_statemachine_info);

            return;

        case RUNSTATE_LOAD_PRODUCER_EVENTS:

            _interface->load_producer_events(openlcb_statemachine_info);

            return;

        case RUNSTATE_LOGIN_COMPLETE:

            if (_interface->on_login_complete) {

                if (!_interface->on_login_complete(openlcb_statemachine_info->openlcb_node)) {

                    return;

                }

            }

            openlcb_statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;

            return;

        default:

            return;

    }

}

// ============================================================================
// Sibling Dispatch Functions
// ============================================================================

    /**
    * @brief Begins sibling dispatch of the login outgoing message.
    *
    * @details Called after handle_outgoing sends the message to the wire.
    * Points the sibling context's incoming_msg_info at the login outgoing
    * message, sets the loopback flag for self-skip, and fetches the first
    * node for sibling iteration.
    *
    * @return true if sibling dispatch started, false if only 1 node (no siblings)
    */
static bool _sibling_dispatch_begin(void) {

    if (_interface->openlcb_node_get_count() <= 1) {

        return false;

    }

    // Point sibling's incoming at the login outgoing message we just sent
    _sibling_statemachine_info.incoming_msg_info.msg_ptr =
            _statemachine_info.outgoing_msg_info.msg_ptr;
    _sibling_statemachine_info.incoming_msg_info.enumerate = false;

    // Mark as loopback so self-skip works in does_node_process_msg
    _sibling_statemachine_info.incoming_msg_info.msg_ptr->state.loopback = true;

    // Get first sibling node
    _sibling_statemachine_info.openlcb_node =
            _interface->openlcb_node_get_first(OPENLCB_LOGIN_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX);

    _sibling_dispatch_active = true;

    return true;

}

    /**
    * @brief Sends the sibling's pending outgoing response to the wire.
    *
    * @details If a sibling handler produced a response during dispatch,
    * this function sends it to the wire.  No response queue is used —
    * login messages are informational and do not generate multi-level
    * sibling response chains.
    *
    * @return true if a message was pending (caller should retry), false if idle
    */
static bool _sibling_handle_outgoing(void) {

    if (_sibling_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_sibling_statemachine_info.outgoing_msg_info.msg_ptr)) {

            _sibling_statemachine_info.outgoing_msg_info.valid = false;

        }

        return true;

    }

    return false;

}

    /**
    * @brief Re-enters the sibling handler for multi-message enumerate responses.
    *
    * @details If a sibling handler set the enumerate flag (e.g. to send
    * multiple P/C Identified messages in response to Init Complete),
    * this function re-enters the main protocol handler to produce the
    * next message.
    *
    * @return true if re-enumeration active, false if complete
    */
static bool _sibling_handle_reenumerate(void) {

    if (_sibling_statemachine_info.incoming_msg_info.enumerate) {

        _interface->process_main_statemachine(&_sibling_statemachine_info);

        return true;

    }

    return false;

}

    /**
    * @brief Dispatches the login outgoing message to the current sibling node.
    *
    * @details Skips the originating node (self-skip handled by
    * does_node_process_msg via source_id comparison and loopback flag).
    * Dispatches only to nodes in RUNSTATE_RUN — nodes still logging in
    * are not ready to process protocol messages.
    *
    * @return true if dispatch occurred or node skipped, false if no node
    */
static bool _sibling_dispatch_current(void) {

    if (!_sibling_statemachine_info.openlcb_node) {

        _sibling_dispatch_active = false;

        return false;

    }

    if (_sibling_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

        _interface->process_main_statemachine(&_sibling_statemachine_info);

    }

    return true;

}

    /**
    * @brief Advances to the next sibling node.
    *
    * @details Moves the sibling enumerator to the next node.  When no
    * more nodes remain, clears _sibling_dispatch_active and NULLs the
    * incoming message pointer.
    *
    * @return true if advanced (more siblings or reached end), false if not active
    */
static bool _sibling_dispatch_advance(void) {

    if (!_sibling_statemachine_info.openlcb_node) {

        return false;

    }

    _sibling_statemachine_info.openlcb_node =
            _interface->openlcb_node_get_next(OPENLCB_LOGIN_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX);

    if (!_sibling_statemachine_info.openlcb_node) {

        // All siblings processed — sibling dispatch complete
        _sibling_dispatch_active = false;
        _sibling_statemachine_info.incoming_msg_info.msg_ptr = NULL;

        return true;

    }

    return true;

}

// ============================================================================
// Outgoing and Enumeration Functions
// ============================================================================

    /**
    * @brief Tries to send the pending outgoing message.
    *
    * @details Algorithm:
    * -# If valid flag not set, return false
    * -# Call send_openlcb_msg(); on success start sibling dispatch
    * -# If no siblings, clear valid immediately
    * -# Return true (caller should keep retrying until sent)
    *
    * @return true if a message was pending, false if idle
    */
bool OpenLcbLoginStatemachine_handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg_info.msg_ptr)) {

            // Start sibling dispatch if multiple nodes exist.
            // The outgoing slot stays valid until sibling dispatch completes.
            if (!_sibling_dispatch_begin()) {

                // Single node — no siblings, clear immediately
                _statemachine_info.outgoing_msg_info.valid = false;

            }

        }

        return true; // keep trying till it can get sent

    }

    return false;

}

    /**
    * @brief Re-enters the current handler if the enumerate flag is set.
    *
    * @details Algorithm:
    * -# If enumerate flag not set, return false
    * -# Call process_login_statemachine() to generate next message
    * -# Return true (keep going until handler clears the flag)
    *
    * @return true if re-enumeration occurred, false if complete
    */
bool OpenLcbLoginStatemachine_handle_try_reenumerate(void) {

    if (_statemachine_info.outgoing_msg_info.enumerate) {

        _interface->process_login_statemachine(&_statemachine_info); // Continue processing

        return true; // keep going until target clears the enumerate flag

    }

    return false;

}

    /**
    * @brief Gets the first node and processes it if login is not complete.
    *
    * @details Algorithm:
    * -# If current node already set, return false
    * -# Get first node; return true if NULL (no nodes)
    * -# If run_state < RUNSTATE_RUN, call process_login_statemachine()
    * -# Return true
    *
    * @return true if first node attempt was made, false if current node already exists
    */
bool OpenLcbLoginStatemachine_handle_try_enumerate_first_node(void) {

    if (!_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = _interface->openlcb_node_get_first(OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) {

            return true;

        }

        if (_statemachine_info.openlcb_node->state.run_state < RUNSTATE_RUN) {

            _interface->process_login_statemachine(&_statemachine_info); // Do the processing

        }

        return true; // done

    }

    return false;

}

    /**
    * @brief Advances to the next node and processes it if login is not complete.
    *
    * @details Algorithm:
    * -# If no current node, return false
    * -# Get next node; return true if NULL (end of list)
    * -# If run_state < RUNSTATE_RUN, call process_login_statemachine()
    * -# Return true
    *
    * @return true if next node attempt was made, false if no current node exists
    */
bool OpenLcbLoginStatemachine_handle_try_enumerate_next_node(void) {

    if (_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = _interface->openlcb_node_get_next(OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) {

            return true;

        }

        if (_statemachine_info.openlcb_node->state.run_state < RUNSTATE_RUN) {

            _interface->process_login_statemachine(&_statemachine_info); // Do the processing

        }

        return true; // done

    }

    return false;

}

    /**
    * @brief Runs one non-blocking step of login processing.  Call from main loop.
    *
    * @details Priority order:
    * -# Send pending outgoing (skip if held for sibling dispatch)
    * -# Sibling dispatch: send sibling response, reenumerate, dispatch current, advance
    * -# Re-enumerate login handler for multi-message sequences
    * -# Get first node if none active
    * -# Advance to next node
    * Each step returns immediately after one operation.
    */
void OpenLcbLoginMainStatemachine_run(void) {

    // ── Priority 1: Send pending login outgoing message ─────────────
    // If valid and NOT in sibling dispatch, try to send to wire.
    // If valid and IN sibling dispatch, skip (held for siblings to read).
    if (!_sibling_dispatch_active) {

        if (_interface->handle_outgoing_openlcb_message()) {

            return;

        }

    }

    // ── Priority 2: Sibling dispatch of the login outgoing message ──
    // After sending to wire, show the outgoing msg to each sibling.
    // One step per _run() call — same cadence as node enumeration.
    if (_sibling_dispatch_active) {

        // 2a: Send any pending sibling response to wire first
        if (_sibling_handle_outgoing()) {

            return;

        }

        // 2b: If sibling handler is mid-enumerate, continue it
        if (_sibling_handle_reenumerate()) {

            return;

        }

        // 2c: Dispatch to current sibling node
        if (_sibling_dispatch_current()) {

            // After dispatch, advance to next sibling for next _run()
            _sibling_dispatch_advance();

            // If dispatch just completed (no more siblings), clear login slot
            if (!_sibling_dispatch_active) {

                _statemachine_info.outgoing_msg_info.msg_ptr->state.loopback = false;
                _statemachine_info.outgoing_msg_info.valid = false;

            }

            return;

        }

    }

    // ── Priority 3: Re-enumerate login handler for multi-message ────
    if (_interface->handle_try_reenumerate()) {

        return;

    }

    // ── Priority 4: Get first node if none active ───────────────────
    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    // ── Priority 5: Advance to next node ────────────────────────────
    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}

    /** @brief Returns pointer to internal state machine info.  For unit testing only. */
openlcb_login_statemachine_info_t *OpenLcbLoginStatemachine_get_statemachine_info(void) {

    return &_statemachine_info;

}
