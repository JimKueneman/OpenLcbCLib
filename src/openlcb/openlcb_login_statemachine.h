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
* @file openlcb_login_statemachine.h
* @brief Login state machine dispatcher for OpenLCB node initialization sequence
*
* @details This module implements the main dispatcher for the OpenLCB login state machine.
* It orchestrates the complete node initialization sequence by managing node enumeration,
* dispatching to appropriate message handlers, controlling message transmission, and
* coordinating re-enumeration for multi-message sequences.
*
* The state machine is event-driven and non-blocking, designed to be called repeatedly
* by the main application loop.
*
* @author Jim Kueneman
* @date 17 Jan 2026
*
* @see openlcb_login_statemachine_handler.h - Message construction handlers
* @see OpenLCB Message Network Standard S-9.7.3.1 - Initialization Complete
* @see OpenLCB Event Transport Standard S-9.7.4 - Event Transport Protocol
*/

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_LOGIN_STATEMACHINE__
#define    __OPENLCB_OPENLCB_LOGIN_STATEMACHINE__

#include "openlcb_types.h"

    /**
    * @brief Interface structure for login state machine callback functions
    *
    * @details This structure defines all function pointers that must be provided
    * during initialization. The interface pattern allows the state machine to be
    * decoupled from specific implementations, making the code more testable and flexible.
    *
    * Required callback categories:
    * - Message transmission
    * - Node enumeration
    * - Message handlers
    * - Internal state machine functions (for testability)
    */
typedef struct {

        /**
        * @brief Callback to send an OpenLCB message to the network
        *
        * @details Called when a message is ready to transmit. The implementation should
        * attempt to send the message and return true if successful.
        *
        * @param outgoing_msg Pointer to the message to transmit
        * @return true if message was queued for transmission, false if transmission failed
        *
        * @warning outgoing_msg must NOT be NULL
        *
        * @note Returning false will cause the state machine to retry on the next iteration
        */
    bool (*send_openlcb_msg)(openlcb_msg_t *outgoing_msg);

        /**
        * @brief Callback to get the first OpenLCB node for enumeration
        *
        * @details Returns the first node in the enumeration list for the given context key.
        *
        * @param key Enumeration context key
        * @return Pointer to first node, or NULL if no nodes exist
        *
        * @note Returning NULL indicates no nodes need processing
        *
        * @see openlcb_node_get_next - Gets subsequent nodes
        */
    openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);

        /**
        * @brief Callback to get the next OpenLCB node for enumeration
        *
        * @details Returns the next node in the enumeration list for the given context key.
        *
        * @param key Enumeration context key
        * @return Pointer to next node, or NULL if no more nodes exist
        *
        * @note Returning NULL indicates enumeration is complete
        *
        * @see openlcb_node_get_first - Gets first node
        */
    openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);

        /**
        * @brief Callback to load an Initialization Complete message
        *
        * @details Constructs an Initialization Complete message for the node.
        * This is the first message sent during the login sequence.
        *
        * @param openlcb_statemachine_info Pointer to state machine info with node and message buffer
        *
        * @see OpenLcbLoginMessageHandler_load_initialization_complete
        */
    void (*load_initialization_complete)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /**
        * @brief Callback to load Producer Event Identified messages
        *
        * @details Constructs Producer Identified messages for the node's produced events.
        * May be called multiple times via re-enumeration.
        *
        * @param openlcb_statemachine_info Pointer to state machine info with node and message buffer
        *
        * @see OpenLcbLoginMessageHandler_load_producer_event
        */
    void (*load_producer_events)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /**
        * @brief Callback to load Consumer Event Identified messages
        *
        * @details Constructs Consumer Identified messages for the node's consumed events.
        * May be called multiple times via re-enumeration.
        *
        * @param openlcb_statemachine_info Pointer to state machine info with node and message buffer
        *
        * @see OpenLcbLoginMessageHandler_load_consumer_event
        */
    void (*load_consumer_events)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /**
        * @brief Internal function pointer for state machine processing
        *
        * @details Dispatches to the appropriate handler based on node run_state.
        * Exposed through interface for unit testing.
        *
        * @param statemachine_info Pointer to state machine info with node and message buffer
        *
        * @see OpenLcbLoginStateMachine_process
        */
    void (*process_login_statemachine)(openlcb_login_statemachine_info_t *statemachine_info);

        /**
        * @brief Internal function pointer for message transmission handling
        *
        * @details Attempts to send pending outgoing message. Exposed for unit testing.
        *
        * @return true if message transmission was handled, false if no message pending
        *
        * @see OpenLcbLoginStatemachine_handle_outgoing_openlcb_message
        */
    bool (*handle_outgoing_openlcb_message)(void);

        /**
        * @brief Internal function pointer for re-enumeration handling
        *
        * @details Handles re-entry into handler for multi-message sequences. Exposed for unit testing.
        *
        * @return true if re-enumeration occurred, false if enumeration complete
        *
        * @see OpenLcbLoginStatemachine_handle_try_reenumerate
        */
    bool (*handle_try_reenumerate)(void);

        /**
        * @brief Internal function pointer for first node enumeration
        *
        * @details Gets first node and processes it if not yet initialized. Exposed for unit testing.
        *
        * @return true if first node was handled, false if no action taken
        *
        * @see OpenLcbLoginStatemachine_handle_try_enumerate_first_node
        */
    bool (*handle_try_enumerate_first_node)(void);

        /**
        * @brief Internal function pointer for next node enumeration
        *
        * @details Gets next node and processes it if not yet initialized. Exposed for unit testing.
        *
        * @return true if next node was handled, false if no action taken
        *
        * @see OpenLcbLoginStatemachine_handle_try_enumerate_next_node
        */
    bool (*handle_try_enumerate_next_node)(void);

} interface_openlcb_login_state_machine_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
        * @brief Initializes the login state machine with callback interface
        *
        * @details Stores the interface function pointers and initializes the internal
        * state machine structure. Sets up the outgoing message buffer and prepares
        * for node enumeration.
        *
        * Use cases:
        * - Called once during application startup
        * - Must be called before any nodes begin login sequence
        *
        * @param interface_openlcb_login_state_machine Pointer to interface structure
        *
        * @warning MUST be called exactly once during initialization
        * @warning The interface pointer is stored in static memory - the pointed-to
        *          structure must remain valid for the lifetime of the program
        * @warning NOT thread-safe
        *
        * @attention Call during single-threaded initialization phase
        * @attention All interface function pointers must be valid (non-NULL)
        * @attention Call after message handler initialization
        *
        * @see interface_openlcb_login_state_machine_t - Interface structure definition
        * @see OpenLcbLoginMainStatemachine_run - Main processing loop
        */
    extern void OpenLcbLoginStateMachine_initialize(const interface_openlcb_login_state_machine_t *interface_openlcb_login_state_machine);

        /**
        * @brief Main state machine processing loop - call repeatedly from application loop
        *
        * @details This is the main entry point for login processing. Should be called
        * repeatedly by the application's main loop. Performs one step of processing
        * and returns immediately (non-blocking design).
        *
        * Processing handles message transmission, re-enumeration for multi-message
        * sequences, and node iteration. Nodes are automatically processed through
        * their initialization states.
        *
        * Use cases:
        * - Called continuously from main application loop
        * - Automatically processes all nodes requiring initialization
        *
        * @warning Must be called from non-interrupt context
        *
        * @attention Call repeatedly - this function does NOT block
        * @attention Nodes transition themselves through states - this just dispatches
        *
        * @note Non-blocking - returns immediately after one step
        * @note Safe to call even when no nodes need processing
        *
        * @see OpenLcbLoginStateMachine_process - State dispatcher
        * @see OpenLcbLoginStatemachine_handle_outgoing_openlcb_message - Message transmission
        * @see OpenLcbLoginStatemachine_handle_try_reenumerate - Re-enumeration control
        */
    extern void OpenLcbLoginMainStatemachine_run(void);

        /**
        * @brief Dispatches to appropriate handler based on node's run_state
        *
        * @details Examines the node's current run_state and calls the corresponding
        * interface handler function. This function is exposed for unit testing but
        * is normally called internally by the main state machine loop.
        *
        * Use cases:
        * - Called internally by main state machine
        * - Called directly in unit tests
        *
        * @param openlcb_statemachine_info Pointer to state machine info with node and message buffer
        *
        * @warning openlcb_statemachine_info must NOT be NULL
        * @warning openlcb_statemachine_info->openlcb_node must be valid
        *
        * @note This is primarily an internal function exposed for testing
        * @note Does nothing if node run_state is not one of the handled states
        *
        * @see interface_openlcb_login_state_machine_t - Handler callbacks
        * @see RUNSTATE_LOAD_INITIALIZATION_COMPLETE in openlcb_defines.h
        */
    extern void OpenLcbLoginStateMachine_process(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /**
        * @brief Handles transmission of pending outgoing message
        *
        * @details Checks if there is a valid outgoing message and attempts to send it.
        * If transmission succeeds, clears the valid flag. If transmission fails,
        * keeps the valid flag set to retry later. This function is exposed for unit
        * testing but is normally called internally.
        *
        * Use cases:
        * - Called as first step in main state machine loop
        * - Called directly in unit tests
        *
        * @return true if a message was pending (sent or queued for retry), false if no message pending
        *
        * @note This is primarily an internal function exposed for testing
        * @note Returning true indicates state machine should retry on next iteration
        *
        * @see interface_openlcb_login_state_machine_t - send_openlcb_msg callback
        * @see OpenLcbLoginMainStatemachine_run - Main loop caller
        */
    extern bool OpenLcbLoginStatemachine_handle_outgoing_openlcb_message(void);

        /**
        * @brief Handles re-enumeration for multi-message sequences
        *
        * @details Checks if the enumerate flag is set and re-enters the handler if needed.
        * The enumerate flag is set by handlers when they have more messages to send
        * in the same state. This function is exposed for unit testing but is normally
        * called internally.
        *
        * Use cases:
        * - Called after message transmission in main state machine loop
        * - Called directly in unit tests
        *
        * @return true if re-enumeration occurred, false if enumeration complete
        *
        * @note This is primarily an internal function exposed for testing
        * @note Handlers control enumeration by setting/clearing the enumerate flag
        *
        * @see OpenLcbLoginMessageHandler_load_producer_event - Sets enumerate flag
        * @see OpenLcbLoginMessageHandler_load_consumer_event - Sets enumerate flag
        */
    extern bool OpenLcbLoginStatemachine_handle_try_reenumerate(void);

        /**
        * @brief Attempts to get and process the first node in enumeration
        *
        * @details If the current node pointer is NULL, gets the first node and processes
        * it if its run_state indicates initialization is needed. This function is exposed
        * for unit testing but is normally called internally.
        *
        * Use cases:
        * - Called in main state machine loop to start node enumeration
        * - Called directly in unit tests
        *
        * @return true if first node attempt was made, false if current node already exists
        *
        * @note This is primarily an internal function exposed for testing
        * @note Skips nodes already in RUNSTATE_RUN (already initialized)
        *
        * @see interface_openlcb_login_state_machine_t - openlcb_node_get_first callback
        * @see OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX in openlcb_defines.h
        */
    extern bool OpenLcbLoginStatemachine_handle_try_enumerate_first_node(void);

        /**
        * @brief Attempts to get and process the next node in enumeration
        *
        * @details If the current node pointer is not NULL, gets the next node and processes
        * it if its run_state indicates initialization is needed. This function is exposed
        * for unit testing but is normally called internally.
        *
        * Use cases:
        * - Called in main state machine loop to continue node enumeration
        * - Called directly in unit tests
        *
        * @return true if next node attempt was made, false if no current node exists
        *
        * @note This is primarily an internal function exposed for testing
        * @note Skips nodes already in RUNSTATE_RUN (already initialized)
        * @note When end of list is reached, sets openlcb_node to NULL
        *
        * @see interface_openlcb_login_state_machine_t - openlcb_node_get_next callback
        * @see OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX in openlcb_defines.h
        */
    extern bool OpenLcbLoginStatemachine_handle_try_enumerate_next_node(void);

        /**
        * @brief Returns pointer to internal state machine info structure
        *
        * @details Provides access to the internal state machine info structure for
        * debugging and unit testing purposes.
        *
        * Use cases:
        * - Unit testing to verify internal state
        * - Debugging to inspect current state machine status
        *
        * @return Pointer to internal state machine info structure
        *
        * @warning The returned pointer is to static memory - do not free
        * @warning Modifying the returned structure can cause undefined behavior
        *
        * @attention Use only for testing and debugging
        *
        * @note This is primarily for unit testing
        * @note The structure persists across all calls to the state machine
        */
    extern openlcb_login_statemachine_info_t *OpenLcbLoginStatemachine_get_statemachine_info(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_OPENLCB_LOGIN_STATEMACHINE__ */
