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
* @file openlcb_main_statemachine.h
* @brief Main OpenLCB protocol state machine interface and dispatcher
*
* @details This file defines the main state machine dispatcher for processing
* OpenLCB protocol messages. It provides a central message routing mechanism
* that dispatches incoming messages to appropriate protocol handlers based on
* Message Type Indicator (MTI) values.
*
* The state machine operates on a node enumeration model, processing each
* incoming message against all active nodes in the system. Protocol handlers
* are provided through function pointers in the interface structure, allowing
* flexible implementation of required and optional OpenLCB protocols.
*
* Key responsibilities:
* - Message routing and MTI-based dispatch
* - Node enumeration and message filtering
* - Outgoing message queue management
* - Protocol handler invocation
* - Interaction rejected message generation
*
* Supported protocols (via interface callbacks):
* - Message Network Protocol (required)
* - Protocol Support Protocol/PIP (required)
* - Simple Node Information Protocol/SNIP (optional)
* - Event Transport Protocol (optional)
* - Traction Protocol (optional)
* - Datagram Protocol (optional)
* - Stream Protocol (optional)
*
* @author Jim Kueneman
* @date 17 Jan 2026
*
* @see openlcb_types.h - Core type definitions
* @see openlcb_utilities.h - Message manipulation utilities
*/

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_MAIN_STATEMACHINE__
#define __OPENLCB_OPENLCB_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Dependency injection interface for OpenLCB Main State Machine operations
     *
     * @details This structure defines all function pointers required and optional
     * for the main OpenLCB protocol dispatcher. The application must populate this
     * structure with appropriate callbacks before calling OpenLcbMainStatemachine_initialize().
     *
     * OpenLcbCLib uses dependency injection to enable:
     * - Comprehensive unit testing with mock implementations
     * - Protocol-specific optimization (NULL handlers strip unused code)
     * - Flexible application integration
     * - Platform-independent operation
     *
     * Function pointers are organized into categories:
     * - Resource management: Lock/unlock shared resources, message transmission
     * - Node enumeration: First/next node iteration for multi-node support
     * - Required protocol handlers: Message Network and Protocol Support (PIP)
     * - Internal handlers: Exposed for testing and debugging
     * - Optional protocol handlers: SNIP, Events, Traction, Datagram, Stream
     *
     * Required vs Optional handlers:
     * - Required handlers MUST be non-NULL and properly implemented
     * - Optional handlers may be NULL if protocol not supported by node
     * - When optional handler is NULL, dispatcher automatically sends Interaction Rejected
     * - This allows minimal nodes to omit unnecessary protocol support
     *
     * Handler invocation flow:
     * -# Main loop pops incoming message from FIFO
     * -# Enumerate all nodes in system
     * -# For each node, check if message applies (broadcast or addressed to node)
     * -# Dispatch to handler based on MTI value
     * -# Handler processes message and optionally prepares response
     * -# Response queued for transmission
     *
     * Use cases:
     * - Application initialization and configuration
     * - Dependency injection for comprehensive testing
     * - Protocol selection and node capability customization
     * - Platform-specific integration
     *
     * @warning ALL required function pointers MUST be non-NULL or undefined behavior occurs
     * @warning Optional pointers can be NULL to disable specific protocols
     * @warning Handlers must not block - use cooperative multitasking patterns
     *
     * @attention Populate all required fields before calling OpenLcbMainStatemachine_initialize()
     * @attention Ensure thread safety in lock/unlock implementations
     * @attention Handlers receive state machine context via openlcb_statemachine_info_t pointer
     *
     * @see OpenLcbMainStatemachine_initialize - Uses this interface for setup
     * @see openlcb_statemachine_info_t - State context passed to all handlers
     * @see OpenLcbMainStatemachine_run - Main processing loop using these handlers
     */
typedef struct {

    /*@{*/
    /** @name Resource Management Functions
     * Functions for thread safety and message transmission
     */

    /** 
     * @brief Disables interrupts and prevents concurrent access to shared resources
     *
     * @details Application implements this to prevent CAN receive interrupts and
     * timer ticks from accessing shared buffer structures during critical operations.
     * Typical implementations disable interrupts or acquire a mutex/semaphore.
     *
     * Resources that require protection:
     * - OpenLCB message FIFOs (shared between interrupt and main loop)
     * - Buffer stores (allocation/deallocation must be atomic)
     * - Node state structures (modified by handlers)
     *
     * Implementation examples:
     * - Bare metal: __disable_irq() or equivalent
     * - RTOS: Take mutex or enter critical section
     * - Linux: pthread_mutex_lock()
     *
     * @warning Lock duration must be minimal to avoid blocking CAN reception
     * @warning Must be paired with unlock_shared_resources() - no nested locks
     *
     * @attention Critical sections should be under 100 microseconds when possible
     * @attention Disable only necessary interrupts (CAN RX, timer tick)
     * 
     * @note This is a REQUIRED callback - must not be NULL
     *
     * @see unlock_shared_resources - Must be called after every lock
     */
    void (*lock_shared_resources)(void);

    /** 
     * @brief Re-enables interrupts and allows access to shared resources
     *
     * @details Application implements this to restore normal operation after a
     * critical section. Must re-enable the same resources that were disabled by
     * lock_shared_resources().
     *
     * @warning Must be called after every lock_shared_resources() call to prevent deadlock
     * @warning Failure to unlock causes CAN reception to stop
     *
     * @note This is a REQUIRED callback - must not be NULL
     * 
     * @attention Always call even if error occurs in critical section
     * @attention Consider using try/finally or similar pattern in application
     *
     * @see lock_shared_resources - Must be called before this
     */
    void (*unlock_shared_resources)(void);

    /** 
     * @brief Transmits an OpenLCB message to the network
     *
     * @details Application implements this to send messages via the active transport
     * layer (typically CAN bus). Implementation may queue to software buffer or write
     * directly to hardware transmit buffer. Must handle cases where transmit buffer
     * is full.
     *
     * Transport layer responsibilities:
     * - Fragment large messages into transport frames (CAN = 8 bytes per frame)
     * - Manage frame sequencing (first, middle, last frame markers)
     * - Handle transmission failures and retries
     * - Update alias mappings for outgoing messages
     *
     * @warning Must return false if transmission fails (buffer full, etc.)
     * @warning Caller retains message ownership - do not free in this function
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation typically: CanTxStatemachine_send_openlcb_message()
     * @note Return true only when message queued or transmitted successfully
     *
     * @see CanTxStatemachine_send_openlcb_message - Typical CAN transport implementation
     */
    bool (*send_openlcb_msg)(openlcb_msg_t *outgoing_msg);

    /*@}*/

    /*@{*/
    /** @name Node Enumeration Functions
     * Functions for iterating through active nodes
     */

    /** 
     * @brief Retrieves the first node for enumeration
     *
     * @details Starts enumeration of all allocated nodes in the system. OpenLcbCLib
     * supports multiple virtual nodes (limited only by USER_DEFINED_NODE_BUFFER_DEPTH).
     * Each incoming message is processed against all nodes to determine which should
     * handle it based on addressing.
     *
     * The key parameter allows multiple simultaneous enumerations without interference:
     * - Main state machine uses key=OPENLCB_STATEMACHINE_NODE_ENUMRATOR_KEY
     * - Login state machine uses different key
     * - Each key maintains independent enumeration position
     *
     * @warning Returns NULL if no nodes allocated in system
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbNode_get_first()
     * @note Enumeration position tracked per-key to allow concurrent iterations
     *
     * @see openlcb_node_get_next - Continues enumeration
     * @see OpenLcbNode_get_first - Standard implementation
     * @see USER_DEFINED_NODE_BUFFER_DEPTH - Maximum nodes supported
     */
    openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);

    /** 
     * @brief Retrieves the next node in enumeration sequence
     *
     * @details Continues enumeration started by openlcb_node_get_first(). Returns
     * successive nodes until all have been enumerated, then returns NULL.
     *
     * Enumeration continues across calls:
     * - First call after get_first() returns second node
     * - Subsequent calls return third, fourth, etc.
     * - NULL indicates enumeration complete
     *
     * @warning Returns NULL when no more nodes available
     * @warning Must use same key value as corresponding get_first() call
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbNode_get_next()
     * @note Safe to call even if get_first() returned NULL
     *
     * @see openlcb_node_get_first - Starts enumeration
     * @see OpenLcbNode_get_next - Standard implementation
     */
    openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);

    /*@}*/

    /*@{*/
    /** @name Core State Machine Functions
     * Functions for message handling and interaction management
     */

    /** 
     * @brief Constructs Optional Interaction Rejected message for unhandled MTIs
     *
     * @details Generates standardized rejection response when:
     * - Optional protocol message received with NULL handler
     * - Unknown MTI addressed to node
     * - Protocol not implemented by node
     *
     * The response includes:
     * - Error code: ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI (0x1000)
     * - Original MTI that triggered rejection
     * - Proper source and destination addressing
     *
     * This automatic rejection allows minimal nodes to safely ignore unimplemented
     * protocols without manual error handling in application code.
     *
     * @warning Do not call directly - used internally by dispatcher
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbMainStatemachine_load_interaction_rejected()
     * @note Response automatically sent by dispatcher when handler is NULL
     *
     * @see OpenLcbMainStatemachine_load_interaction_rejected - Standard implementation
     */
    void (*load_interaction_rejected)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

    /*@{*/
    /** @name Required Message Network Protocol Handlers
     * Core protocol messages - all nodes must implement these
     */

    /** 
     * @brief Handles Initialization Complete message (MTI 0x0100)
     *
     * @details Processes announcement that a node has completed initialization and
     * is entering normal operation. This message is broadcast by nodes after login
     * sequence completes and configuration is loaded.
     *
     * Standard response: None (informational message only)
     *
     * Use cases:
     * - Network discovery and monitoring
     * - Detecting when new nodes come online
     * - Tracking node state changes
     *
     * @warning Do not confuse with Initialization Complete Simple (0x0101)
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_initialization_complete()
     * @note No response required - observation only
     *
     * @see message_network_initialization_complete_simple - Simple protocol variant
     * @see ProtocolMessageNetwork_handle_initialization_complete
     */
    void (*message_network_initialization_complete)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Initialization Complete Simple message (MTI 0x0101)
     *
     * @details Processes initialization announcement from simple nodes that implement
     * only the minimal required protocol subset. Identical to standard initialization
     * complete but indicates limited protocol support.
     *
     * Standard response: None (informational message only)
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_initialization_complete_simple()
     * @note No response required - observation only
     *
     * @see message_network_initialization_complete - Full protocol variant
     * @see ProtocolMessageNetwork_handle_initialization_complete_simple
     */
    void (*message_network_initialization_complete_simple)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Verify Node ID Addressed message (MTI 0x0488)
     *
     * @details Processes request to verify node identity. Sender is asking specific
     * node to confirm it is online and respond with its Node ID. Used for network
     * discovery and verifying node presence.
     *
     * Standard response: Verified Node ID (0x0170 or 0x0171)
     *
     * Response includes:
     * - Source: this node's alias and ID
     * - Destination: requester
     * - Payload: this node's 48-bit Node ID
     *
     * @warning Must respond even if payload contains different Node ID
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_verify_node_id_addressed()
     * @note Always reply when addressed to this node
     *
     * @see message_network_verify_node_id_global - Broadcast variant
     * @see message_network_verified_node_id - Response handler
     * @see ProtocolMessageNetwork_handle_verify_node_id_addressed
     */
    void (*message_network_verify_node_id_addressed)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Verify Node ID Global message (MTI 0x0490)
     *
     * @details Processes broadcast request for all nodes or specific node to verify
     * identity. If payload is empty, all nodes respond. If payload contains Node ID,
     * only matching node responds.
     *
     * Standard response: Verified Node ID (0x0170 or 0x0171) if match or broadcast
     *
     * Payload handling:
     * - Empty payload: All nodes respond (network enumeration)
     * - Contains Node ID: Only matching node responds (targeted verification)
     *
     * @warning Check payload to determine if response required
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_verify_node_id_global()
     * @note Used for network discovery and node verification
     *
     * @see message_network_verify_node_id_addressed - Targeted variant
     * @see message_network_verified_node_id - Response handler
     * @see ProtocolMessageNetwork_handle_verify_node_id_global
     */
    void (*message_network_verify_node_id_global)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Verified Node ID message (MTI 0x0170 or 0x0171)
     *
     * @details Processes node identity announcement. Contains 48-bit Node ID of
     * responding node. Used to build network map and detect duplicate IDs.
     *
     * Standard response: None (informational message only)
     *
     * Critical duplicate detection:
     * - If payload Node ID matches this node's ID
     * - AND message is from different alias
     * - Then duplicate Node ID detected (serious error)
     * - Node should send Duplicate Node ID event and take corrective action
     *
     * @warning MUST detect and handle duplicate Node IDs
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_verified_node_id()
     * @note No standard response except for duplicate detection
     *
     * @see message_network_verify_node_id_addressed - Request message
     * @see message_network_verify_node_id_global - Broadcast request
     * @see ProtocolMessageNetwork_handle_verified_node_id
     */
    void (*message_network_verified_node_id)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Optional Interaction Rejected message (MTI 0x0068)
     *
     * @details Processes rejection response indicating remote node does not support
     * requested operation. Contains error code and rejected MTI.
     *
     * Standard response: None (informational message only)
     *
     * Payload format:
     * - Bytes 0-1: Error code (see OpenLCB error code definitions)
     * - Bytes 2-3: MTI that was rejected
     * - Bytes 4+: Optional additional error information
     *
     * Use cases:
     * - Discovering which protocols remote node supports
     * - Handling feature unavailability gracefully
     * - Debugging protocol implementation issues
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_optional_interaction_rejected()
     * @note No response required - informational only
     *
     * @see load_interaction_rejected - Generate this message
     * @see ProtocolMessageNetwork_handle_optional_interaction_rejected
     */
    void (*message_network_optional_interaction_rejected)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Terminate Due to Error message (MTI 0x00A8)
     *
     * @details Processes notification that remote node is terminating operation due
     * to unrecoverable error. Contains error code and context information.
     *
     * Standard response: None (informational message only)
     *
     * Payload format:
     * - Bytes 0-1: Error code indicating failure type
     * - Bytes 2-3: MTI that triggered error (if applicable)
     * - Bytes 4+: Optional error context
     *
     * Use cases:
     * - Detecting critical errors in remote nodes
     * - Network monitoring and diagnostics
     * - Triggering error recovery procedures
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_terminate_due_to_error()
     * @note No standard response - node may be offline after this
     *
     * @see ProtocolMessageNetwork_handle_terminate_due_to_error
     */
    void (*message_network_terminate_due_to_error)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

    /*@{*/
    /** @name Required Protocol Support Protocol (PIP) Handlers
     * Protocol capability advertisement - all nodes must implement
     */

    /** 
     * @brief Handles Protocol Support Inquiry message (MTI 0x0828)
     *
     * @details Processes request for node to advertise its protocol capabilities.
     * Node responds with bitmask of supported protocols.
     *
     * Standard response: Protocol Support Reply (MTI 0x0668)
     *
     * Response payload (6 bytes):
     * - Bit flags indicating which protocols are supported
     * - Bits defined in OpenLCB Protocol Support Standard
     * - Examples: Datagram, Stream, Configuration, Events, etc.
     *
     * @warning Must accurately reflect actual node capabilities
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_protocol_support_inquiry()
     * @note Reply built from node's protocol_support_flags configuration
     *
     * @see message_network_protocol_support_reply - Response handler
     * @see ProtocolMessageNetwork_handle_protocol_support_inquiry
     */
    void (*message_network_protocol_support_inquiry)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Protocol Support Reply message (MTI 0x0668)
     *
     * @details Processes protocol capability advertisement from remote node. Contains
     * bitmask of supported protocols.
     *
     * Standard response: None (informational message only)
     *
     * Payload interpretation:
     * - 6 bytes of protocol flags
     * - Bit positions defined in OpenLCB standard
     * - Used to determine what operations remote node supports
     *
     * Use cases:
     * - Discovering remote node capabilities before operations
     * - Building network capability map
     * - Optimizing protocol interactions
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: ProtocolMessageNetwork_handle_protocol_support_reply()
     * @note No response required - informational only
     *
     * @see message_network_protocol_support_inquiry - Request message
     * @see ProtocolMessageNetwork_handle_protocol_support_reply
     */
    void (*message_network_protocol_support_reply)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

    /*@{*/
    /** @name Internal Testing and Debugging Functions
     * Exposed for unit testing and debugging - normally called via dispatcher
     */

    /** 
     * @brief Processes message through appropriate protocol handler based on MTI
     *
     * @details Internal dispatcher function that examines incoming message MTI and
     * routes to appropriate protocol handler. Implements large switch statement
     * covering all supported MTIs.
     *
     * Handler dispatch order:
     * -# Check if node should process message (addressing filter)
     * -# Extract MTI from incoming message
     * -# Switch on MTI value
     * -# Call registered handler or load_interaction_rejected if NULL
     * -# Return to allow handler's response to be sent
     *
     * @warning Do not call directly in application - used by dispatcher
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbMainStatemachine_process_main_statemachine()
     * @note Exposed to allow unit testing of dispatch logic
     *
     * @see does_node_process_msg - Addressing filter used before dispatch
     * @see OpenLcbMainStatemachine_process_main_statemachine
     */
    void (*process_main_statemachine)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Determines if node should process incoming message
     *
     * @details Implements addressing filter to determine message applicability.
     * Checks if message is broadcast or specifically addressed to this node.
     *
     * Processing criteria (node processes if ANY true):
     * - Node is initialized
     * - Message is broadcast (no destination address)
     * - Message destination alias matches node's alias
     * - Message destination ID matches node's ID
     * - Message is Verify Node ID Global (always processed)
     *
     * @warning <b>Required</b> assignment for testing
     * @warning Must implement proper addressing logic per OpenLCB standard
     *
     * @note Default implementation: OpenLcbMainStatemachine_does_node_process_msg()
     * @note Returns true if node should handle message, false otherwise
     *
     * @see process_main_statemachine - Uses this filter before dispatch
     * @see OpenLcbMainStatemachine_does_node_process_msg
     */
    bool (*does_node_process_msg)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles transmission of pending outgoing messages
     *
     * @details Attempts to transmit any message queued in outgoing buffer. Retries
     * until successful or explicitly cleared. Part of main loop priority handling.
     *
     * Transmission flow:
     * -# Check if outgoing message valid flag is set
     * -# Call send_openlcb_msg() with message
     * -# If successful, clear valid flag
     * -# If failed, keep valid flag set for retry
     * -# Return true if message pending (continue trying)
     *
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbMainStatemachine_handle_outgoing_openlcb_message()
     * @note Exposed for testing, normally called by run loop
     *
     * @see OpenLcbMainStatemachine_handle_outgoing_openlcb_message
     */
    bool (*handle_outgoing_openlcb_message)(void);

    /** 
     * @brief Handles re-enumeration for multi-message protocol responses
     *
     * @details When protocol handler sets enumerate flag, continues processing same
     * incoming message against same node. Allows handlers to send multiple responses
     * (e.g., identifying multiple events) without losing message context.
     *
     * Re-enumeration pattern:
     * -# Handler processes message
     * -# If more responses needed, set enumerate flag
     * -# Return to dispatcher
     * -# Dispatcher calls handler again with same message
     * -# Handler sends next response
     * -# Repeat until handler clears enumerate flag
     *
     * Use cases:
     * - Event identification with multiple producer/consumer events
     * - Multi-part configuration memory reads
     * - Segmented data transfers
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbMainStatemachine_handle_try_reenumerate()
     * @note Exposed for testing, normally called by run loop
     *
     * @see OpenLcbMainStatemachine_handle_try_reenumerate
     */
    bool (*handle_try_reenumerate)(void);

    /** 
     * @brief Pops next incoming message from receive queue
     *
     * @details Retrieves next message from OpenLCB buffer FIFO if no message currently
     * being processed. Uses resource locking for thread-safe FIFO access.
     *
     * Pop operation:
     * -# Check if already have incoming message
     * -# If not, lock shared resources
     * -# Pop from OpenLcbBufferFifo
     * -# Unlock shared resources
     * -# Store message in state machine info
     * -# Return true if message retrieved
     *
     * @warning Uses lock/unlock callbacks - ensure proper implementation
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message()
     * @note Exposed for testing, normally called by run loop
     *
     * @see lock_shared_resources - Used during FIFO access
     * @see unlock_shared_resources - Released after access
     * @see OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message
     */
    bool (*handle_try_pop_next_incoming_openlcb_message)(void);

    /** 
     * @brief Enumerates first node for message processing
     *
     * @details Retrieves first node and processes current incoming message against it
     * if node is in RUN state. Starts node enumeration sequence.
     *
     * Enumeration flow:
     * -# Check if node pointer already set
     * -# If not, get first node via openlcb_node_get_first()
     * -# If no nodes exist, free incoming message and return
     * -# If node in RUN state, call process_main_statemachine()
     * -# Return true to continue with next node
     *
     * @warning Skips nodes not in RUNSTATE_RUN
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbMainStatemachine_handle_try_enumerate_first_node()
     * @note Exposed for testing, normally called by run loop
     *
     * @see handle_try_enumerate_next_node - Continues enumeration
     * @see OpenLcbMainStatemachine_handle_try_enumerate_first_node
     */
    bool (*handle_try_enumerate_first_node)(void);

    /** 
     * @brief Enumerates next node for message processing
     *
     * @details Retrieves next node and processes current incoming message against it
     * if node is in RUN state. Continues enumeration until all nodes processed.
     *
     * Enumeration flow:
     * -# Get next node via openlcb_node_get_next()
     * -# If no more nodes, free incoming message and return
     * -# If node in RUN state, call process_main_statemachine()
     * -# Return false to allow continued enumeration
     *
     * When enumeration completes:
     * - Incoming message is freed back to buffer store
     * - State machine ready for next message
     *
     * @warning Skips nodes not in RUNSTATE_RUN
     * @warning Frees message when enumeration complete
     *
     * @note This is a REQUIRED callback - must not be NULL
     * @note Default implementation: OpenLcbMainStatemachine_handle_try_enumerate_next_node()
     * @note Exposed for testing, normally called by run loop
     *
     * @see handle_try_enumerate_first_node - Starts enumeration
     * @see OpenLcbMainStatemachine_handle_try_enumerate_next_node
     */
    bool (*handle_try_enumerate_next_node)(void);

    /*@}*/

    /*@{*/
    /** @name Optional SNIP Protocol Handlers
     * Simple Node Information Protocol - set to NULL if not implemented
     */

    /** 
     * @brief Handles Simple Node Info Request message (MTI 0x0DE8)
     *
     * @details Processes request for node identification information. Node responds
     * with manufacturer details, model info, and user-configured name/description.
     *
     * Standard response: Simple Node Info Reply (MTI 0x0A08)
     *
     * Response format (null-terminated strings):
     * - Manufacturer version (1 byte)
     * - Manufacturer name
     * - Model name
     * - Hardware version
     * - Software version
     * - User version (1 byte)
     * - User name (from configuration memory)
     * - User description (from configuration memory)
     *
     * @warning Optional - set to NULL if SNIP not implemented
     * @warning If NULL, dispatcher sends Interaction Rejected automatically
     *
     * @note Default implementation: ProtocolSnip_handle_simple_node_info_request()
     * @note Response may be multi-frame on CAN (up to 256 bytes)
     *
     * @see snip_simple_node_info_reply - Response handler
     * @see ProtocolSnip_handle_simple_node_info_request
     */
    void (*snip_simple_node_info_request)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Simple Node Info Reply message (MTI 0x0A08)
     *
     * @details Processes node identification information from remote node.
     *
     * Standard response: None (informational message only)
     *
     * Use cases:
     * - Discovering node identities during network configuration
     * - Building node database for user interface
     * - Automatic node classification
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolSnip_handle_simple_node_info_reply()
     * @note No standard response - informational only
     *
     * @see snip_simple_node_info_request - Request message
     * @see ProtocolSnip_handle_simple_node_info_reply
     */
    void (*snip_simple_node_info_reply)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

    /*@{*/
    /** @name Optional Event Transport Protocol Handlers
     * Producer/Consumer event messaging - set to NULL if not implemented
     */

    /** 
     * @brief Handles Identify Consumer message (MTI 0x08F4)
     *
     * @details Processes request for node to identify if it consumes a specific event.
     *
     * Standard response: Consumer Identified (0x04C4, 0x04C5, or 0x04C7)
     *
     * Response variants:
     * - Consumer Identified Valid (0x04C4): Node consumes this event, currently valid
     * - Consumer Identified Invalid (0x04C5): Node consumes this event, currently invalid
     * - Consumer Identified Unknown (0x04C7): Node consumes this event, state unknown
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_consumer_identify()
     * @note Only respond if event ID matches configured consumer events
     *
     * @see ProtocolEventTransport_handle_consumer_identify
     */
    void (*event_transport_consumer_identify)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Consumer Range Identified message (MTI 0x04A4)
     *
     * @details Processes announcement that remote node consumes a range of events.
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_consumer_range_identified()
     *
     * @see ProtocolEventTransport_handle_consumer_range_identified
     */
    void (*event_transport_consumer_range_identified)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Consumer Identified Unknown message (MTI 0x04C7)
     *
     * @details Processes announcement that remote node consumes event but state unknown.
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_consumer_identified_unknown()
     *
     * @see ProtocolEventTransport_handle_consumer_identified_unknown
     */
    void (*event_transport_consumer_identified_unknown)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Consumer Identified Set message (MTI 0x04C4)
     *
     * @details Processes announcement that remote node consumes event and it is currently valid/set.
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_consumer_identified_set()
     *
     * @see ProtocolEventTransport_handle_consumer_identified_set
     */
    void (*event_transport_consumer_identified_set)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Consumer Identified Clear message (MTI 0x04C5)
     *
     * @details Processes announcement that remote node consumes event and it is currently invalid/clear.
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_consumer_identified_clear()
     *
     * @see ProtocolEventTransport_handle_consumer_identified_clear
     */
    void (*event_transport_consumer_identified_clear)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Consumer Identified Reserved message (MTI 0x04C6)
     *
     * @details Processes reserved consumer state message (future use).
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_consumer_identified_reserved()
     *
     * @see ProtocolEventTransport_handle_consumer_identified_reserved
     */
    void (*event_transport_consumer_identified_reserved)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Identify Producer message (MTI 0x0914)
     *
     * @details Processes request for node to identify if it produces a specific event.
     *
     * Standard response: Producer Identified (0x0544, 0x0545, or 0x0547)
     *
     * Response variants:
     * - Producer Identified Valid (0x0544): Node produces this event, currently valid
     * - Producer Identified Invalid (0x0545): Node produces this event, currently invalid
     * - Producer Identified Unknown (0x0547): Node produces this event, state unknown
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_producer_identify()
     * @note Only respond if event ID matches configured producer events
     *
     * @see ProtocolEventTransport_handle_producer_identify
     */
    void (*event_transport_producer_identify)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Producer Range Identified message (MTI 0x0524)
     *
     * @details Processes announcement that remote node produces a range of events.
     *
     * Standard response: None (informational message only)
     * 
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_producer_range_identified()
     *
     * @see ProtocolEventTransport_handle_producer_range_identified
     */
    void (*event_transport_producer_range_identified)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Producer Identified Unknown message (MTI 0x0547)
     *
     * @details Processes announcement that remote node produces event but state unknown.
     *
     * Standard response: None (informational message only)
     * 
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_producer_identified_unknown()
     *
     * @see ProtocolEventTransport_handle_producer_identified_unknown
     */
    void (*event_transport_producer_identified_unknown)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Producer Identified Set message (MTI 0x0544)
     *
     * @details Processes announcement that remote node produces event and it is currently valid/set.
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_producer_identified_set()
     *
     * @see ProtocolEventTransport_handle_producer_identified_set
     */
    void (*event_transport_producer_identified_set)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Producer Identified Clear message (MTI 0x0545)
     *
     * @details Processes announcement that remote node produces event and it is currently invalid/clear.
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_producer_identified_clear()
     *
     * @see ProtocolEventTransport_handle_producer_identified_clear
     */
    void (*event_transport_producer_identified_clear)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Producer Identified Reserved message (MTI 0x0546)
     *
     * @details Processes reserved producer state message (future use).
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_producer_identified_reserved()
     *
     * @see ProtocolEventTransport_handle_producer_identified_reserved
     */
    void (*event_transport_producer_identified_reserved)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Identify Events Addressed message (MTI 0x0968)
     *
     * @details Processes request for node to identify all events it produces or consumes.
     *
     * Standard response: Producer/Consumer Identified messages for all events
     *
     * Response pattern:
     * - Multiple Producer Identified messages (one per produced event)
     * - Multiple Consumer Identified messages (one per consumed event)
     * - Uses enumerate flag for multi-message response
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_events_identify_dest()
     * @note May generate many response messages
     *
     * @see event_transport_identify - Global variant
     * @see ProtocolEventTransport_handle_events_identify_dest
     */
    void (*event_transport_identify_dest)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Identify Events Global message (MTI 0x0970)
     *
     * @details Processes broadcast request for all nodes to identify all events.
     *
     * Standard response: Producer/Consumer Identified messages for all event.
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_events_identify()
     * @note All nodes respond simultaneously
     *
     * @see event_transport_identify_dest - Addressed variant
     * @see ProtocolEventTransport_handle_events_identify
     */
    void (*event_transport_identify)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Learn Event message (MTI 0x0594)
     *
     * @details Processes teach/learn event for configuration. Used to configure nodes
     * to produce or consume specific events through blue/gold button protocol.
     *
     * Standard response: Depends on node configuration mode
     *
     * Use cases:
     * - Teaching node which events to produce
     * - Teaching node which events to consume
     * - Blue/gold button configuration
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_event_learn()
     *
     * @see ProtocolEventTransport_handle_event_learn
     */
    void (*event_transport_learn)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Producer/Consumer Event Report message (MTI 0x05B4)
     *
     * @details Processes event occurrence notification. Producers send this when
     * event conditions change. Consumers act on events they're configured for.
     *
     * Standard response: None (consumers take action, no reply)
     *
     * Payload: 8-byte event ID
     *
     * Consumer actions:
     * - Check if configured for this event ID
     * - If match, take configured action (toggle output, trigger sequence, etc.)
     * - No protocol-level response required
     *
     * @warning Core event messaging - most event nodes need this
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_pc_event_report()
     * @note No automatic response - application handles event
     *
     * @see event_transport_pc_report_with_payload - Variant with data
     * @see ProtocolEventTransport_handle_pc_event_report
     */
    void (*event_transport_pc_report)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Producer/Consumer Event Report with Payload message (MTI 0x05F4)
     *
     * @details Processes event occurrence with additional data payload. Extended
     * event reporting allowing up to 256 bytes of event-specific data.
     *
     * Standard response: None (consumers process event and data)
     *
     * Payload:
     * - Bytes 0-7: Event ID
     * - Bytes 8+: Event-specific payload data
     *
     * Use cases:
     * - Analog sensor values with event
     * - Multi-parameter state changes
     * - Complex event data
     * 
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: ProtocolEventTransport_handle_pc_event_report_with_payload()
     * @note Payload size limited by transport (CAN: multi-frame up to 256 bytes)
     *
     * @see event_transport_pc_report - Variant without payload
     * @see ProtocolEventTransport_handle_pc_event_report_with_payload
     */
    void (*event_transport_pc_report_with_payload)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

    /*@{*/
    /** @name Optional Traction Protocol Handlers
     * Train control protocol - set to NULL if not implemented
     */

    /** 
     * @brief Handles Traction Control Command message (MTI 0x05EB)
     *
     * @details Processes train control commands (speed, function, emergency stop, etc.).
     *
     * Standard response: Traction Control Reply (MTI 0x01E9)
     *
     * @note Optional - can be NULL if this command is not supported
     * @note Default implementation: Protocol-specific handler
     *
     * @see traction_control_reply - Response message
     */
    void (*traction_control_command)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Traction Control Reply message (MTI 0x01E9)
     *
     * @details Processes command acknowledgment from train node.
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     * 
     * @see traction_control_command - Command message
     */
    void (*traction_control_reply)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

    /*@{*/
    /** @name Optional Traction SNIP Protocol Handlers
     * Train node information - set to NULL if not implemented
     */

    /** 
     * @brief Handles Simple Train Node Ident Info Request message (MTI 0x0DA8)
     *
     * @details Processes request for train-specific identification information.
     *
     * Standard response: Simple Train Node Ident Info Reply (MTI 0x0A48)
     *
     * @note Optional - can be NULL if this command is not supported
     *
     * @see simple_train_node_ident_info_reply - Response message
     */
    void (*simple_train_node_ident_info_request)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Simple Train Node Ident Info Reply message (MTI 0x0A48)
     *
     * @details Processes train identification information from remote train node.
     *
     * Standard response: None (informational message only)
     *
     * @note Optional - can be NULL if this command is not supported
     *
     * @see simple_train_node_ident_info_request - Request message
     */
    void (*simple_train_node_ident_info_reply)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

    /*@{*/
    /** @name Optional Datagram Protocol Handlers
     * Reliable point-to-point data transfer - set to NULL if not implemented
     */

    /** 
     * @brief Handles Datagram message (MTI 0x1C48)
     *
     * @details Processes datagram (up to 72 bytes of reliable addressed data).
     *
     * Standard response: Datagram Received OK (MTI 0x0A28) or Rejected (MTI 0x0A48)
     *
     * First byte of payload indicates datagram content type:
     * - 0x20: Memory Configuration Protocol
     * - 0x30: Remote Button Protocol
     * - 0x40: Display Protocol
     * - 0x50: Train Control Protocol
     *
     * @warning Must send OK or Rejected response
     * @note Optional - can be NULL if this command is not supported
     *
     * @note Default implementation: Protocol-specific handler based on content ID
     *
     * @see datagram_ok_reply - Success response
     * @see datagram_rejected_reply - Error response
     */
    void (*datagram)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Datagram Received OK message (MTI 0x0A28)
     *
     * @details Processes acknowledgment that datagram was received successfully.
     *
     * Standard response: None (informational message only)
     *
     * Optional payload flags:
     * - Bit 7: Reply Pending (more data coming)
     * - Bits 3-0: Timeout value for pending reply
     *
     * @warning Optional - set to NULL if not needed
     *
     * @see datagram - Datagram message
     */
    void (*datagram_ok_reply)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Datagram Rejected message (MTI 0x0A48)
     *
     * @details Processes rejection notification for sent datagram.
     *
     * Standard response: None (informational message only)
     *
     * Payload:
     * - Bytes 0-1: Error code
     * - Additional error information
     *
     * @note Optional - can be NULL if this command is not supported
     *
     * @see datagram - Datagram message
     */
    void (*datagram_rejected_reply)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

    /*@{*/
    /** @name Optional Stream Protocol Handlers
     * High-bandwidth data transfer - set to NULL if not implemented
     */

    /** 
     * @brief Handles Stream Initiate Request message (MTI 0x0CC8)
     *
     * @details Processes request to establish stream connection for bulk data transfer.
     *
     * Standard response: Stream Initiate Reply (MTI 0x0868)
     *
     * @warning Optional - set to NULL if Stream Protocol not implemented
     *
     * @see stream_initiate_reply - Response message
     */
    void (*stream_initiate_request)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Stream Initiate Reply message (MTI 0x0868)
     *
     * @details Processes stream establishment response.
     *
     * Standard response: None (begin stream data transfer)
     *
     * @note Optional - can be NULL if this command is not supported
     *
     * @see stream_initiate_request - Request message
     */
    void (*stream_initiate_reply)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Stream Data Send message (MTI 0x1F88)
     *
     * @details Processes stream data packet.
     *
     * Standard response: Stream Data Proceed (MTI 0x0888) when ready for more
     *
     * @note Optional - can be NULL if this command is not supported
     *
     * @see stream_data_proceed - Flow control response
     */
    void (*stream_send_data)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Stream Data Proceed message (MTI 0x0888)
     *
     * @details Processes flow control allowing more stream data to be sent.
     *
     * Standard response: None (continue sending data)
     *
     * @note Optional - can be NULL if this command is not supported
     *
     * @see stream_send_data - Data message
     */
    void (*stream_data_proceed)(openlcb_statemachine_info_t *statemachine_info);

    /** 
     * @brief Handles Stream Data Complete message (MTI 0x08A8)
     *
     * @details Processes notification that stream transfer is complete.
     *
     * Standard response: None (close stream connection)
     *
     * @note Optional - can be NULL if this command is not supported
     *
     * @see stream_send_data - Data message
     */
    void (*stream_data_complete)(openlcb_statemachine_info_t *statemachine_info);

    /*@}*/

} interface_openlcb_main_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the main OpenLCB state machine
     *
     * @details Configures the state machine with the provided interface
     * callbacks and prepares internal data structures for message processing.
     * This function must be called once during system startup before any
     * message processing operations.
     *
     * The initialization process prepares the outgoing message buffer,
     * configures message pointers, and stores the interface structure for
     * later use by the run function and protocol handlers.
     *
     * Use cases:
     * - Called once during application startup after buffer initialization
     * - Required before calling OpenLcbMainStatemachine_run()
     * - Must be called after OpenLCB login completes
     *
     * @param interface_openlcb_main_statemachine Pointer to populated dependency
     * interface structure containing all function implementations
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe - call before starting message processing
     * @warning All required function pointers must be non-NULL
     *
     * @attention Call after OpenLcbBufferStore_initialize()
     * @attention Call after OpenLcbBufferFifo_initialize()
     * @attention Validate interface structure before passing
     *
     * @see OpenLcbMainStatemachine_run - Main processing loop
     * @see interface_openlcb_main_statemachine_t - Interface structure definition
     */
    extern void OpenLcbMainStatemachine_initialize(
            const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine);

    /**
     * @brief Executes one iteration of main OpenLCB protocol state machine
     *
     * @details Performs cooperative multitasking by processing one operation per call.
     * Handles outgoing message transmission, incoming message processing, node
     * enumeration, and protocol dispatch in priority order.
     *
     * Processing order per iteration:
     * - Transmit pending outgoing messages (highest priority)
     * - Re-enumerate for multi-message responses
     * - Pop next incoming message from FIFO
     * - Enumerate and process first node
     * - Enumerate and process next nodes
     *
     * Each operation returns early if work remains, allowing cooperative
     * multitasking with other application code.
     *
     * Use cases:
     * - Called continuously from main application loop
     * - Cooperative multitasking with other tasks
     * - Non-blocking protocol message processing
     *
     * @warning Must be called frequently (as fast as possible in main loop)
     * @warning Assumes OpenLcbMainStatemachine_initialize() was already called
     *
     * @note Returns after processing one operation for cooperative multitasking
     * @note Each operation may lock shared resources briefly
     * @note Processes all nodes for each message (multi-node support)
     *
     * @see OpenLcbMainStatemachine_initialize - Must be called first
     * @see interface_openlcb_main_statemachine_t - Handler structure used
     */
    extern void OpenLcbMainStatemachine_run(void);

    /**
     * @brief Constructs Optional Interaction Rejected message
     *
     * @details Generates standardized rejection response for unhandled MTIs.
     * The response includes error code and original MTI that triggered rejection.
     *
     * This function is used by the main state machine when:
     * - Optional protocol message received with NULL handler
     * - Unknown MTI addressed to node
     * - Protocol not implemented by node
     *
     * The response is loaded into outgoing message buffer and marked
     * as valid for transmission.
     *
     * Use cases:
     * - Default handler for unimplemented optional protocols
     * - Response to unknown addressed messages
     * - Protocol capability advertisement
     *
     * @param statemachine_info Pointer to state machine context containing
     * incoming message and outgoing message buffer
     *
     * @warning Returns early if any required pointers are NULL
     * @warning Assumes outgoing message buffer is available
     *
     * @attention Used automatically by state machine for unhandled MTIs
     * @attention Do not call directly - internal use only
     *
     * @see OpenLcbMainStatemachine_process_main_statemachine - Calls this for unhandled MTIs
     */
    extern void OpenLcbMainStatemachine_load_interaction_rejected(
            openlcb_statemachine_info_t *statemachine_info);

    /**
     * @brief Handles transmission of pending outgoing messages
     *
     * @details Attempts to send any queued outgoing message through interface
     * send function. Retries until message successfully sent, then clears valid flag.
     *
     * This function is part of the main processing loop and is called first
     * to ensure outgoing messages have priority over new incoming processing.
     *
     * Use cases:
     * - Outgoing message transmission in run loop
     * - Response message handling
     * - Unit testing of send logic
     *
     * @return true if message pending (keep calling), false if no message to send
     *
     * @warning For testing/debugging - normally called by run function
     *
     * @attention Exposed for testing, do not call directly in application
     *
     * @see OpenLcbMainStatemachine_run - Main caller
     */
    extern bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void);

    /**
     * @brief Handles re-enumeration for multi-message responses
     *
     * @details When protocol handler needs to send multiple messages in
     * response to single incoming message, it sets enumerate flag. This
     * function continues processing the same incoming message against
     * current node until handler clears the flag.
     *
     * Use cases:
     * - Multi-message protocol responses (event identification)
     * - Large data transfers requiring segmentation
     * - Iterative message processing
     *
     * @return true if re-enumeration active (keep calling), false if complete
     *
     * @warning For testing/debugging - normally called by run function
     *
     * @attention Exposed for testing, do not call directly in application
     *
     * @see OpenLcbMainStatemachine_run - Main caller
     */
    extern bool OpenLcbMainStatemachine_handle_try_reenumerate(void);

    /**
     * @brief Pops next incoming message from receive queue
     *
     * @details Retrieves next incoming message from buffer FIFO if no message
     * currently being processed. Uses resource locking for thread-safe FIFO access.
     *
     * Use cases:
     * - Message queue management in run loop
     * - Incoming message retrieval
     * - Unit testing message flow
     *
     * @return true if message popped (continue processing), false if already have message
     *
     * @warning For testing/debugging - normally called by run function
     *
     * @note Uses lock/unlock callbacks for thread safety
     *
     * @attention Exposed for testing, do not call directly in application
     *
     * @see OpenLcbMainStatemachine_run - Main caller
     * @see OpenLcbBufferFifo_pop - FIFO access function
     */
    extern bool OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message(void);

    /**
     * @brief Enumerates first node for message processing
     *
     * @details Retrieves first node from node list and processes current
     * incoming message against it if node is in RUN state. If no nodes exist,
     * frees the incoming message.
     *
     * Use cases:
     * - Node enumeration in run loop
     * - First node message processing
     * - Unit testing enumeration logic
     *
     * @return true if enumeration complete (stop), false if no node yet
     *
     * @warning For testing/debugging - normally called by run function
     *
     * @note Skips nodes not in RUNSTATE_RUN
     * @note Frees message if no nodes allocated
     *
     * @attention Exposed for testing, do not call directly in application
     *
     * @see OpenLcbMainStatemachine_run - Main caller
     * @see OpenLcbMainStatemachine_handle_try_enumerate_next_node - Continues enumeration
     */
    extern bool OpenLcbMainStatemachine_handle_try_enumerate_first_node(void);

    /**
     * @brief Enumerates next node for message processing
     *
     * @details Retrieves next node from node list and processes current
     * incoming message against it if node is in RUN state. When enumeration
     * completes (no more nodes), frees the incoming message.
     *
     * Use cases:
     * - Node enumeration continuation in run loop
     * - Multi-node message processing
     * - Unit testing enumeration logic
     *
     * @return true if enumeration active (keep calling), false if no current node
     *
     * @warning For testing/debugging - normally called by run function
     *
     * @note Skips nodes not in RUNSTATE_RUN
     * @note Frees message when enumeration complete
     *
     * @attention Exposed for testing, do not call directly in application
     *
     * @see OpenLcbMainStatemachine_run - Main caller
     * @see OpenLcbMainStatemachine_handle_try_enumerate_first_node - Starts enumeration
     */
    extern bool OpenLcbMainStatemachine_handle_try_enumerate_next_node(void);

    /**
     * @brief Processes message through appropriate protocol handler
     *
     * @details Examines incoming message MTI and routes to appropriate protocol
     * handler from interface structure. Implements switch statement covering all
     * supported MTIs. Automatically sends Interaction Rejected if handler is NULL.
     *
     * Use cases:
     * - Message routing and dispatch
     * - Protocol handler invocation
     * - Unit testing dispatch logic
     *
     * @param statemachine_info Pointer to state machine context containing
     * current node, incoming message, and outgoing message buffer
     *
     * @warning For testing/debugging - normally called internally
     * @warning Do not call directly in application
     *
     * @attention Exposed for testing comprehensive MTI coverage
     *
     * @see OpenLcbMainStatemachine_run - Indirect caller via enumeration
     */
    extern void OpenLcbMainStatemachine_process_main_statemachine(
            openlcb_statemachine_info_t *statemachine_info);

    /**
     * @brief Determines if node should process incoming message
     *
     * @details Implements addressing filter based on OpenLCB specification.
     * Checks if message is broadcast or specifically addressed to this node.
     *
     * Node processes message if:
     * - Node is initialized
     * - Message is broadcast (no destination)
     * - Destination alias matches node's alias
     * - Destination ID matches node's ID
     * - Message is Verify Node ID Global
     *
     * Use cases:
     * - Message filtering before protocol dispatch
     * - Addressing validation
     * - Unit testing address matching
     *
     * @param statemachine_info Pointer to state machine context with node and message
     *
     * @return true if node should handle message, false if should skip
     *
     * @warning For testing/debugging - normally called internally
     *
     * @attention Exposed for testing addressing logic
     *
     * @see OpenLcbMainStatemachine_process_main_statemachine - Uses this filter
     */
    extern bool OpenLcbMainStatemachine_does_node_process_msg(
            openlcb_statemachine_info_t *statemachine_info);

    /**
     * @brief Returns pointer to internal state machine information structure
     *
     * @details Provides access to the internal static state machine context for
     * testing and debugging purposes. The returned structure contains:
     * - Current node being processed
     * - Incoming message information (message pointer, enumerate flag)
     * - Outgoing message information (message pointer, valid flag)
     *
     * This accessor enables comprehensive unit testing of functions that operate
     * on internal state, such as:
     * - handle_outgoing_openlcb_message() - Requires valid flag to be set
     * - handle_try_reenumerate() - Requires enumerate flag to be set
     * - handle_try_pop_next_incoming_openlcb_message() - Reads incoming message pointer
     * - handle_try_enumerate_first_node() - Reads/writes node pointer
     * - handle_try_enumerate_next_node() - Reads/writes node pointer
     * - run() - Exercises entire state machine flow
     *
     * Use cases:
     * - Unit testing of state machine functions
     * - Integration testing of message flow
     * - Debugging state machine behavior
     * - Verification of flag management
     *
     * @return Pointer to internal openlcb_statemachine_info_t structure
     *
     * @warning For testing/debugging only - do not use in production code
     * @warning Direct modification of state bypasses normal state machine flow
     * @warning State modifications must respect state machine invariants
     *
     * @attention Use only for controlled test scenarios
     * @attention Restore state to valid condition after testing
     * @attention Do not rely on internal structure layout
     *
     * @note Similar to OpenLcbLoginStatemachine_get_statemachine_info()
     *
     * @see openlcb_statemachine_info_t - Structure definition
     * @see OpenLcbMainStatemachine_initialize - Initializes this structure
     * @see OpenLcbMainStatemachine_run - Main consumer of this state
     */
    extern openlcb_statemachine_info_t *OpenLcbMainStatemachine_get_statemachine_info(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_MAIN_STATEMACHINE__ */
