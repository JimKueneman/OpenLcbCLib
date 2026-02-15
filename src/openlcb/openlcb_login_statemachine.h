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
    * @details This structure defines the callback interface for the OpenLCB login state
    * machine, which orchestrates the complete node initialization sequence after successful
    * CAN alias allocation. The login state machine manages the four-phase process of
    * announcing nodes on the network:
    *
    * 1. Send Initialization Complete message
    * 2. Send Producer Event Identified messages for all produced events
    * 3. Send Consumer Event Identified messages for all consumed events
    * 4. Call on_login_complete callback (optional) for final setup before entering RUNSTATE_RUN
    *
    * The interface provides callbacks organized into several functional categories:
    *
    * **Required Message Transmission Callbacks:**
    * - send_openlcb_msg: Transmit OpenLCB messages to the network
    *
    * **Required Node Enumeration Callbacks:**
    * - openlcb_node_get_first: Begin node enumeration
    * - openlcb_node_get_next: Continue node enumeration
    *
    * **Required Message Handler Callbacks:**
    * - load_initialization_complete: Construct Initialization Complete message
    * - load_producer_events: Construct Producer Event Identified messages
    * - load_consumer_events: Construct Consumer Event Identified messages
    *
    * **Internal Functions (for testing):**
    * - process_login_statemachine: State dispatcher
    * - handle_outgoing_openlcb_message: Message transmission handler
    * - handle_try_reenumerate: Re-enumeration controller
    * - handle_try_enumerate_first_node: First node enumerator
    * - handle_try_enumerate_next_node: Next node enumerator
    *
    * The state machine operates in a non-blocking manner, performing one step of processing
    * per call to OpenLcbLoginMainStatemachine_run(). It automatically enumerates all nodes
    * that require login processing and dispatches them through the appropriate handlers based
    * on their run_state.
    *
    * @note All required callbacks must be set before calling OpenLcbLoginStateMachine_initialize
    * @note Internal function pointers are for unit testing - they reference module functions
    *
    * @see OpenLcbLoginStateMachine_initialize
    * @see OpenLcbLoginMainStatemachine_run
    * @see openlcb_login_statemachine_handler.h - Message construction handlers
    */
typedef struct {

        /**
         * @brief Callback to send an OpenLCB message to the network
         *
         * @details This required callback transmits an OpenLCB message to the network.
         * The implementation should attempt to queue the message for transmission and
         * return true if successful. If the transmission buffer is full or transmission
         * fails, the callback should return false, causing the state machine to retry
         * on the next iteration.
         *
         * The message includes source alias, source Node ID, MTI, and payload data.
         * The callback is responsible for converting the OpenLCB message to the
         * appropriate CAN frames via the CAN TX state machine.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    bool (*send_openlcb_msg)(openlcb_msg_t *outgoing_msg);

        /**
         * @brief Callback to get the first OpenLCB node for enumeration
         *
         * @details This required callback initiates node enumeration by returning the first
         * node in the node list. The enumeration uses a key parameter to maintain separate
         * iteration contexts for different state machines (login vs main state machine).
         *
         * The callback should return the first allocated node in the node pool, or NULL
         * if no nodes exist. The login state machine will process nodes that are in
         * initialization states (not yet in RUNSTATE_RUN).
         *
         * Typical implementation: Return first entry from node pool array.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);

        /**
         * @brief Callback to get the next OpenLCB node for enumeration
         *
         * @details This required callback continues node enumeration by returning the next
         * node in the node list. The enumeration uses a key parameter to maintain separate
         * iteration contexts for different state machines.
         *
         * The callback should return the next allocated node in the node pool, or NULL
         * when the end of the list is reached. The login state machine will process nodes
         * that are in initialization states (not yet in RUNSTATE_RUN).
         *
         * Typical implementation: Increment index and return next entry from node pool array.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);

        /**
         * @brief Callback to load an Initialization Complete message
         *
         * @details This required callback constructs an Initialization Complete message
         * for the node currently being processed. The message announces the node's presence
         * on the network and signals readiness to participate in OpenLCB operations.
         *
         * The callback should:
         * - Build the message with MTI 0x0100 (Full) or 0x0101 (Simple)
         * - Include the node's 6-byte Node ID in the payload
         * - Set outgoing_msg_info.valid to true to trigger transmission
         * - Mark the node as initialized
         * - Transition to RUNSTATE_LOAD_PRODUCER_EVENTS
         *
         * Typical implementation: OpenLcbLoginMessageHandler_load_initialization_complete
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*load_initialization_complete)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /**
         * @brief Callback to load Producer Event Identified messages
         *
         * @details This required callback constructs Producer Identified messages for the
         * node's produced events. The callback may be invoked multiple times via the
         * enumeration mechanism to send one message per produced event.
         *
         * The callback should:
         * - Check if producers exist (count > 0)
         * - Get MTI for current event state (Valid/Invalid/Unknown)
         * - Build message with 8-byte Event ID in payload
         * - Increment enum_index
         * - Set enumerate flag if more events remain
         * - Transition to RUNSTATE_LOAD_CONSUMER_EVENTS when complete
         *
         * Typical implementation: OpenLcbLoginMessageHandler_load_producer_event
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*load_producer_events)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /**
         * @brief Callback to load Consumer Event Identified messages
         *
         * @details This required callback constructs Consumer Identified messages for the
         * node's consumed events. The callback may be invoked multiple times via the
         * enumeration mechanism to send one message per consumed event. This is the final
         * step in the login sequence.
         *
         * The callback should:
         * - Check if consumers exist (count > 0)
         * - Get MTI for current event state (Valid/Invalid/Unknown)
         * - Build message with 8-byte Event ID in payload
         * - Increment enum_index
         * - Set enumerate flag if more events remain
         * - Transition to RUNSTATE_LOGIN_COMPLETE when complete
         *
         * Typical implementation: OpenLcbLoginMessageHandler_load_consumer_event
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*load_consumer_events)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /**
         * @brief Internal function pointer for state machine processing
         *
         * @details This callback dispatches to the appropriate message handler based on the
         * node's current run_state. It is exposed through the interface to enable unit testing
         * of the state dispatch logic.
         *
         * Typical implementation: OpenLcbLoginStateMachine_process
         *
         * This callback examines node->run_state and calls:
         * - load_initialization_complete if RUNSTATE_LOAD_INITIALIZATION_COMPLETE
         * - load_producer_events if RUNSTATE_LOAD_PRODUCER_EVENTS
         * - load_consumer_events if RUNSTATE_LOAD_CONSUMER_EVENTS
         * - on_login_complete if RUNSTATE_LOGIN_COMPLETE (then transitions to RUNSTATE_RUN)
         *
         * @note This is a REQUIRED callback - must not be NULL (set to module function)
         */
    void (*process_login_statemachine)(openlcb_login_statemachine_info_t *statemachine_info);

        /**
         * @brief Internal function pointer for message transmission handling
         *
         * @details This callback attempts to transmit any pending outgoing message by
         * checking the valid flag and calling send_openlcb_msg. It is exposed through
         * the interface to enable unit testing of transmission logic.
         *
         * Typical implementation: OpenLcbLoginStatemachine_handle_outgoing_openlcb_message
         *
         * The callback should:
         * - Check if outgoing_msg_info.valid is true
         * - Call send_openlcb_msg if message is pending
         * - Clear valid flag if transmission succeeds
         * - Keep valid flag set if transmission fails (retry on next iteration)
         *
         * @note This is a REQUIRED callback - must not be NULL (set to module function)
         */
    bool (*handle_outgoing_openlcb_message)(void);

        /**
         * @brief Internal function pointer for re-enumeration handling
         *
         * @details This callback handles the re-enumeration mechanism for multi-message
         * sequences. When message handlers set the enumerate flag, this callback re-invokes
         * the handler to generate the next message in the sequence. It is exposed through
         * the interface to enable unit testing.
         *
         * Typical implementation: OpenLcbLoginStatemachine_handle_try_reenumerate
         *
         * The callback should:
         * - Check if outgoing_msg_info.enumerate is true
         * - Call process_login_statemachine if enumerate flag is set
         * - Return true if re-enumeration occurred
         *
         * @note This is a REQUIRED callback - must not be NULL (set to module function)
         */
    bool (*handle_try_reenumerate)(void);

        /**
         * @brief Internal function pointer for first node enumeration
         *
         * @details This callback attempts to get and process the first node in the node pool.
         * If no current node is being processed, it calls openlcb_node_get_first and dispatches
         * the node if it requires login processing. It is exposed through the interface to
         * enable unit testing.
         *
         * Typical implementation: OpenLcbLoginStatemachine_handle_try_enumerate_first_node
         *
         * The callback should:
         * - Check if openlcb_node is NULL (no current node)
         * - Call openlcb_node_get_first if needed
         * - Skip nodes already in RUNSTATE_RUN
         * - Call process_login_statemachine for nodes needing initialization
         *
         * @note This is a REQUIRED callback - must not be NULL (set to module function)
         */
    bool (*handle_try_enumerate_first_node)(void);

        /**
         * @brief Internal function pointer for next node enumeration
         *
         * @details This callback attempts to get and process the next node in the node pool.
         * If a current node exists, it calls openlcb_node_get_next and dispatches the node
         * if it requires login processing. It is exposed through the interface to enable
         * unit testing.
         *
         * Typical implementation: OpenLcbLoginStatemachine_handle_try_enumerate_next_node
         *
         * The callback should:
         * - Check if openlcb_node is not NULL (current node exists)
         * - Call openlcb_node_get_next to advance enumeration
         * - Skip nodes already in RUNSTATE_RUN
         * - Call process_login_statemachine for nodes needing initialization
         * - Clear openlcb_node to NULL when end of list reached
         *
         * @note This is a REQUIRED callback - must not be NULL (set to module function)
         */
    bool (*handle_try_enumerate_next_node)(void);


       /**
        * @brief Optional callback after login sequence completes
        *   
        * @details This optional callback is invoked after the login sequence completes for a node,
        * just before transitioning to RUNSTATE_RUN. It allows the application to perform any final
        * setup or send any final messages before the node enters normal operation mode.
        */
    bool (*on_login_complete)(openlcb_node_t *openlcb_node); // Optional callback after login sequence completes

} interface_openlcb_login_state_machine_t;


#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
        * @brief Initializes the login state machine with callback interface
        *
        * @details Registers the application's callback interface with the login state machine.
        * The interface provides all necessary callbacks for message transmission, node
        * enumeration, message handler dispatch, and internal state machine operations.
        *
        * This function must be called once during system initialization after the login
        * message handler has been initialized. The interface pointer is stored internally
        * and must remain valid for the lifetime of the application.
        *
        * Typical initialization sequence:
        * 1. OpenLcbLoginMessageHandler_initialize (message handlers)
        * 2. OpenLcbLoginStateMachine_initialize (this function)
        * 3. Begin calling OpenLcbLoginMainStatemachine_run from main loop
        *
        * Use cases:
        * - Called once during application startup
        * - Must be called before OpenLcbLoginMainStatemachine_run
        *
        * @param interface_openlcb_login_state_machine Pointer to callback interface structure
        *
        * @warning interface_openlcb_login_state_machine must remain valid for lifetime of application
        * @warning All required callbacks in the interface must be valid (non-NULL)
        * @warning MUST be called exactly once during initialization
        * @warning NOT thread-safe - call during single-threaded initialization only
        *
        * @attention Call after OpenLcbLoginMessageHandler_initialize
        * @attention Call before starting main state machine loop
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
        * per call and returns immediately, implementing a non-blocking cooperative
        * multitasking design.
        *
        * Processing sequence per call:
        * 1. Attempt to send any pending outgoing message
        * 2. If message pending, return (retry transmission next iteration)
        * 3. Handle re-enumeration if enumerate flag is set
        * 4. If re-enumerating, return (handler called again)
        * 5. Attempt to process first node if no current node
        * 6. If first node processed, return
        * 7. Attempt to process next node if current node exists
        *
        * The state machine automatically enumerates all allocated nodes and processes
        * each node through its login sequence based on run_state. Nodes already in
        * RUNSTATE_RUN (fully initialized) are skipped.
        *
        * Use cases:
        * - Called continuously from main application loop
        * - Automatically processes all nodes requiring initialization
        *
        * @warning Must be called from non-interrupt context
        *
        * @attention Call repeatedly - this function does NOT block
        * @attention Safe to call even when no nodes need processing
        *
        * @note Non-blocking - returns immediately after one step
        * @note Multiple nodes may require several loop iterations to complete
        * @note Enumerate flag mechanism allows multi-message sequences
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
        * interface handler function to construct the appropriate message. This function
        * implements the state dispatch logic for the login sequence.
        *
        * State dispatch mapping:
        * - RUNSTATE_LOAD_INITIALIZATION_COMPLETE → load_initialization_complete
        * - RUNSTATE_LOAD_PRODUCER_EVENTS → load_producer_events
        * - RUNSTATE_LOAD_CONSUMER_EVENTS → load_consumer_events
        * - RUNSTATE_LOGIN_COMPLETE → on_login_complete (if set), then RUNSTATE_RUN
        *
        * This function is exposed for unit testing but is normally called internally
        * by the main state machine loop.
        *
        * Use cases:
        * - Called internally by main state machine
        * - Called directly in unit tests
        *
        * @param openlcb_statemachine_info Pointer to state machine context containing node and message buffer
        *
        * @warning openlcb_statemachine_info must NOT be NULL
        * @warning openlcb_node within statemachine_info must be valid
        *
        * @note This is primarily an internal function exposed for testing
        * @note Does nothing if node run_state is RUNSTATE_RUN or unrecognized
        *
        * @see interface_openlcb_login_state_machine_t - Handler callbacks
        * @see RUNSTATE_LOAD_INITIALIZATION_COMPLETE in openlcb_defines.h
        */
    extern void OpenLcbLoginStateMachine_process(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /**
        * @brief Handles transmission of pending outgoing message
        *
        * @details Checks if there is a valid outgoing message (valid flag set to true)
        * and attempts to send it via the send_openlcb_msg callback. If transmission
        * succeeds, clears the valid flag. If transmission fails, keeps the valid flag
        * set to retry on the next iteration.
        *
        * This function is exposed for unit testing but is normally called as the first
        * step in the main state machine loop.
        *
        * Use cases:
        * - Called as first step in main state machine loop
        * - Called directly in unit tests
        *
        * @return true if a message was pending (sent or queued for retry), false if no message pending
        *
        * @note This is primarily an internal function exposed for testing
        * @note Returning true causes main loop to retry on next iteration
        * @note Message remains valid until transmission succeeds
        *
        * @see interface_openlcb_login_state_machine_t - send_openlcb_msg callback
        * @see OpenLcbLoginMainStatemachine_run - Main loop caller
        */
    extern bool OpenLcbLoginStatemachine_handle_outgoing_openlcb_message(void);

        /**
        * @brief Handles re-enumeration for multi-message sequences
        *
        * @details Checks if the enumerate flag is set and re-enters the state processor
        * if needed. The enumerate flag is set by message handlers when they have more
        * messages to send in the same state (e.g., multiple producer events or multiple
        * consumer events).
        *
        * This function is exposed for unit testing but is normally called internally
        * after message transmission handling.
        *
        * Use cases:
        * - Called after message transmission in main state machine loop
        * - Called directly in unit tests
        *
        * @return true if re-enumeration occurred (handler was called), false if enumeration complete
        *
        * @note This is primarily an internal function exposed for testing
        * @note Handlers control enumeration by setting/clearing the enumerate flag
        * @note Re-enumeration allows sending multiple events without changing run_state
        *
        * @see OpenLcbLoginMessageHandler_load_producer_event - Sets enumerate flag
        * @see OpenLcbLoginMessageHandler_load_consumer_event - Sets enumerate flag
        */
    extern bool OpenLcbLoginStatemachine_handle_try_reenumerate(void);

        /**
        * @brief Attempts to get and process the first node in enumeration
        *
        * @details If the current node pointer is NULL (no node currently being processed),
        * calls openlcb_node_get_first to start node enumeration and processes the first
        * node if its run_state indicates initialization is needed.
        *
        * Nodes already in RUNSTATE_RUN (fully initialized) are skipped. The function
        * continues enumeration until a node needing initialization is found or the end
        * of the list is reached.
        *
        * This function is exposed for unit testing but is normally called internally.
        *
        * Use cases:
        * - Called in main state machine loop to start node enumeration
        * - Called directly in unit tests
        *
        * @return true if first node attempt was made, false if current node already exists
        *
        * @note This is primarily an internal function exposed for testing
        * @note Skips nodes already in RUNSTATE_RUN (already initialized)
        * @note Uses OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX as enumeration key
        *
        * @see interface_openlcb_login_state_machine_t - openlcb_node_get_first callback
        * @see OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX in openlcb_defines.h
        */
    extern bool OpenLcbLoginStatemachine_handle_try_enumerate_first_node(void);

        /**
        * @brief Attempts to get and process the next node in enumeration
        *
        * @details If the current node pointer is not NULL (a node is currently being processed),
        * calls openlcb_node_get_next to advance enumeration and processes the next node if its
        * run_state indicates initialization is needed.
        *
        * Nodes already in RUNSTATE_RUN (fully initialized) are skipped. When the end of the
        * node list is reached (openlcb_node_get_next returns NULL), the current node pointer
        * is set to NULL to allow re-enumeration from the beginning.
        *
        * This function is exposed for unit testing but is normally called internally.
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
        * @note Uses OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX as enumeration key
        *
        * @see interface_openlcb_login_state_machine_t - openlcb_node_get_next callback
        * @see OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX in openlcb_defines.h
        */
    extern bool OpenLcbLoginStatemachine_handle_try_enumerate_next_node(void);

        /**
        * @brief Returns pointer to internal state machine info structure
        *
        * @details Provides access to the internal state machine info structure for
        * debugging and unit testing purposes. The returned structure contains the
        * current node being processed, outgoing message buffer, and state flags.
        *
        * Use cases:
        * - Unit testing to verify internal state
        * - Debugging to inspect current state machine status
        *
        * @return Pointer to internal static state machine info structure
        *
        * @warning The returned pointer is to static memory - do not free
        * @warning Modifying the returned structure can cause undefined behavior
        *
        * @attention Use only for testing and debugging
        * @attention Do not modify the structure contents
        *
        * @note This is primarily for unit testing
        * @note The structure persists across all calls to the state machine
        * @note Contains: openlcb_node pointer, outgoing message, valid flag, enumerate flag
        */
    extern openlcb_login_statemachine_info_t *OpenLcbLoginStatemachine_get_statemachine_info(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_OPENLCB_LOGIN_STATEMACHINE__ */
