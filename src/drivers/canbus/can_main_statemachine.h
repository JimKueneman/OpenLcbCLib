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
 * @file can_main_statemachine.h
 * @brief Main CAN layer state machine for orchestrating message dispatch and node management
 *
 * @details This module implements the primary CAN layer dispatcher that coordinates
 * alias management, login processing, and message routing across all virtual nodes.
 * It handles duplicate alias detection, manages outgoing message queues, and
 * orchestrates the login state machine for node initialization.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Dependency injection interface for CAN Main State Machine operations
     *
     * @details OpenLcbCLib uses dependency injection to enable comprehensive testing
     * by allowing function behaviors to be mocked with valid or invalid results. This
     * approach also enables protocol-specific optimization by setting unused protocol
     * handlers to NULL, which strips out unnecessary code and minimizes application size.
     * The library automatically generates appropriate error responses when handlers are NULL.
     *
     * All function pointers in this structure must be initialized during application setup
     * by calling CanMainStatemachine_initialize() with a properly configured interface.
     *
     * @see CanMainStatemachine_initialize
     */
    typedef struct {
        /*@{*/

        // REQUIRED FUNCTIONS

        /** 
         * @brief Disables interrupts and prevents concurrent access to shared resources
         *
         * @details Application must implement this to stop the 100ms timer and CAN receive
         * interrupt from accessing library buffers during critical operations. Typical
         * implementations disable interrupts or acquire a mutex.
         *
         * @warning <b>Required</b> assignment - must be implemented by application
         * @attention Lock duration must be minimal to avoid blocking CAN reception
         *
         * @see unlock_shared_resources
         */
        void (*lock_shared_resources)(void);

        /** 
         * @brief Re-enables interrupts and allows access to shared resources
         *
         * @details Application must implement this to restart the 100ms timer and CAN
         * receive interrupt after critical section completes. Must be called after every
         * lock_shared_resources() call.
         *
         * @warning <b>Required</b> assignment - must be implemented by application
         * @attention Always call after lock_shared_resources() to prevent deadlock
         *
         * @see lock_shared_resources
         */
        void (*unlock_shared_resources)(void);

        /** 
         * @brief Transmits a CAN message to the physical bus
         *
         * @details Application implements this to interface with hardware CAN controller.
         * Implementation may write directly to hardware buffer or queue to software buffer.
         * Must handle cases where hardware transmit buffer is full.
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: CanTxStatemachine_send_can_message()
         *
         * @see CanTxStatemachine_send_can_message
         */
        bool (*send_can_message)(can_msg_t *msg);

        /** 
         * @brief Retrieves the first node for enumeration
         *
         * @details Starts enumeration of all allocated nodes. Used in conjunction with
         * openlcb_node_get_next() to iterate through all virtual nodes for processing.
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: OpenLcbNode_get_first()
         *
         * @see openlcb_node_get_next
         * @see OpenLcbNode_get_first
         */
        openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);

        /** 
         * @brief Retrieves the next node in enumeration sequence
         *
         * @details Continues enumeration started by openlcb_node_get_first(). Returns NULL
         * when all nodes have been enumerated.
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: OpenLcbNode_get_next()
         *
         * @see openlcb_node_get_first
         * @see OpenLcbNode_get_next
         */
        openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);

        /** 
         * @brief Finds a node by its CAN alias
         *
         * @details Searches all allocated nodes for matching 12-bit alias. Used when
         * processing incoming messages to route them to the correct virtual node.
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: OpenLcbNode_find_by_alias()
         *
         * @see OpenLcbNode_find_by_alias
         */
        openlcb_node_t *(*openlcb_node_find_by_alias)(uint16_t alias);

        /** 
         * @brief Executes the login state machine for a node
         *
         * @details Processes the CAN alias allocation sequence for nodes that have not
         * completed login. Handles CID frame transmission, RID, and AMD frames according
         * to OpenLCB CAN frame transfer standard.
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: CanLoginStateMachine_run()
         *
         * @see CanLoginStateMachine_run
         */
        void (*login_statemachine_run)(can_statemachine_info_t *can_statemachine_info);

        /** 
         * @brief Accesses the alias mapping buffer
         *
         * @details Returns pointer to structure containing all alias-to-NodeID mappings
         * for the network. Used to detect duplicate aliases and manage alias allocations.
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: AliasMappings_get_alias_mapping_info()
         *
         * @see AliasMappings_get_alias_mapping_info
         */
        alias_mapping_info_t *(*alias_mapping_get_alias_mapping_info)(void);

        /** 
         * @brief Removes an alias mapping from the table
         *
         * @details Called when duplicate alias detected or node goes offline. Clears
         * the alias entry and allows it to be reallocated.
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: AliasMappings_unregister()
         *
         * @see AliasMappings_unregister
         */
        void (*alias_mapping_unregister)(uint16_t alias);

        /** 
         * @brief Processes detected duplicate aliases
         *
         * @details Internal function exposed for testing. Iterates through alias mapping
         * table and resets any nodes with duplicate aliases. Application typically assigns
         * this to CanMainStatemachine_handle_duplicate_aliases().
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: CanMainStatemachine_handle_duplicate_aliases()
         *
         * @see CanMainStatemachine_handle_duplicate_aliases
         */
        bool (*handle_duplicate_aliases)(void);

        /** 
         * @brief Transmits pending outgoing CAN messages
         *
         * @details Internal function exposed for testing. Pops messages from outgoing
         * FIFO and attempts transmission. Application typically assigns this to
         * CanMainStatemachine_handle_outgoing_can_message().
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: CanMainStatemachine_handle_outgoing_can_message()
         *
         * @see CanMainStatemachine_handle_outgoing_can_message
         */
        bool (*handle_outgoing_can_message)(void);

        /** 
         * @brief Transmits pending login-related CAN messages
         *
         * @details Internal function exposed for testing. Handles transmission of CID,
         * RID, and AMD frames during node login sequence. Application typically assigns
         * this to CanMainStatemachine_handle_login_outgoing_can_message().
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: CanMainStatemachine_handle_login_outgoing_can_message()
         *
         * @see CanMainStatemachine_handle_login_outgoing_can_message
         */
        bool (*handle_login_outgoing_can_message)(void);

        /** 
         * @brief Begins node enumeration for state machine processing
         *
         * @details Internal function exposed for testing. Gets first node and processes
         * it through appropriate state machine. Application typically assigns this to
         * CanMainStatemachine_handle_try_enumerate_first_node().
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: CanMainStatemachine_handle_try_enumerate_first_node()
         *
         * @see CanMainStatemachine_handle_try_enumerate_first_node
         */
        bool (*handle_try_enumerate_first_node)(void);

        /** 
         * @brief Continues node enumeration for state machine processing
         *
         * @details Internal function exposed for testing. Gets next node and processes
         * it through appropriate state machine. Application typically assigns this to
         * CanMainStatemachine_handle_try_enumerate_next_node().
         *
         * @warning <b>Required</b> assignment
         * @note Default implementation: CanMainStatemachine_handle_try_enumerate_next_node()
         *
         * @see CanMainStatemachine_handle_try_enumerate_next_node
         */
        bool (*handle_try_enumerate_next_node)(void);

        // OPTIONAL FUNCTION

        // CALLBACK FUNCTIONS

        /*@}*/

    } interface_can_main_statemachine_t;

    /**
     * @brief Initializes the CAN Main State Machine
     *
     * @details Sets up the state machine with required dependencies and prepares internal
     * buffers for operation. Must be called once during application initialization before
     * calling CanMainStateMachine_run().
     *
     * The interface structure provides all required function pointers for state machine
     * operation including resource locking, CAN transmission, node management, and alias
     * handling. All function pointers must be valid (non-NULL).
     *
     * Use cases:
     * - Called once during application startup
     * - Required before any CAN communication can occur
     * - Must be called after buffer stores are initialized
     *
     * @param interface_can_main_statemachine Pointer to populated dependency interface
     * structure containing all required function implementations
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
    extern void CanMainStatemachine_initialize(const interface_can_main_statemachine_t *interface_can_main_statemachine);


    /**
     * @brief Executes one iteration of the main CAN state machine
     *
     * @details Performs cooperative multitasking by processing one operation per call
     * and returning. Handles duplicate alias detection, outgoing message transmission,
     * login message processing, and node enumeration in priority order.
     *
     * Processing order per iteration:
     * - Check for and handle duplicate aliases
     * - Transmit pending outgoing CAN messages  
     * - Transmit pending login messages
     * - Enumerate and process first node
     * - Enumerate and process next node
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
     */
    extern void CanMainStateMachine_run(void);


    /**
     * @brief Provides read access to internal state machine context
     *
     * @details Returns pointer to the internal state structure containing current node
     * being processed, login message buffer, and outgoing message pointer. Primarily
     * used for debugging and unit testing.
     *
     * The returned structure includes:
     * - Pointer to current node being processed
     * - Login outgoing message buffer
     * - Login message valid flag
     * - Outgoing CAN message pointer
     * - Enumeration flag
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
    extern can_statemachine_info_t *CanMainStateMachine_get_can_statemachine_info(void);


    /**
     * @brief Handles all detected duplicate alias conflicts
     *
     * @details Scans alias mapping table for duplicate flags, unregisters conflicting
     * aliases, and resets affected nodes to force alias reallocation. Returns true if
     * any duplicates were found and processed.
     *
     * When duplicate detected:
     * - Alias is unregistered from mapping table
     * - Affected node is reset to inhibited state
     * - Node must reallocate new alias through login sequence
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
     */
    extern bool CanMainStatemachine_handle_duplicate_aliases(void);


    /**
     * @brief Transmits pending login-related CAN messages
     *
     * @details Attempts to transmit login message (CID, RID, AMD) if one is pending.
     * Returns true if a message was pending regardless of transmission success.
     * Message remains pending until successfully transmitted.
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
    extern bool CanMainStatemachine_handle_login_outgoing_can_message(void);


    /**
     * @brief Transmits pending outgoing CAN messages from FIFO
     *
     * @details Pops one message from outgoing FIFO and attempts transmission. If
     * transmission succeeds, message is freed back to buffer pool. If transmission
     * fails, message remains in working buffer for retry on next call.
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
    extern bool CanMainStatemachine_handle_outgoing_can_message(void);


    /**
     * @brief Begins node enumeration and processes first node
     *
     * @details Gets first node from node pool and processes it through appropriate
     * state machine based on login status. If node has not completed login (run_state
     * < RUNSTATE_LOAD_INITIALIZATION_COMPLETE), runs login state machine. Returns
     * true indicating enumeration started.
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
     */
    extern bool CanMainStatemachine_handle_try_enumerate_first_node(void);


    /**
     * @brief Continues node enumeration to next node
     *
     * @details Gets next node from node pool and processes it through appropriate
     * state machine based on login status. If node has not completed login (run_state
     * < RUNSTATE_LOAD_INITIALIZATION_COMPLETE), runs login state machine. Returns
     * true if no more nodes remain, false to continue enumeration.
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
     */
    extern bool CanMainStatemachine_handle_try_enumerate_next_node(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__ */
