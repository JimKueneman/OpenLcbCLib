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
    * @brief Interface structure for main state machine dependencies
    *
    * @details This structure defines all function pointers required and optional
    * for the main state machine operation. The application must populate this
    * structure with appropriate callbacks before calling the initialize function.
    *
    * Function pointers are organized into categories:
    * - Resource management (lock/unlock, send)
    * - Node enumeration (get_first, get_next)
    * - Required protocol handlers (Message Network, PIP)
    * - Internal handlers (exposed for testing)
    * - Optional protocol handlers (SNIP, Events, Traction, Datagram, Stream)
    *
    * Required vs Optional handlers:
    * - Required handlers must be provided (non-NULL)
    * - Optional handlers can be NULL if protocol not implemented
    * - When optional handler is NULL, MTI generates Interaction Rejected
    *
    * Use cases:
    * - Application initialization and configuration
    * - Dependency injection for testability
    * - Protocol selection and customization
    *
    * @warning ALL required function pointers MUST be non-NULL
    * @warning Optional pointers can be NULL to disable protocols
    *
    * @attention Populate all required fields before calling initialize
    *
    * @see OpenLcbMainStatemachine_initialize - Uses this interface
    * @see openlcb_statemachine_info_t - State information passed to handlers
    */
typedef struct {

    // Required function assignments
    void (*lock_shared_resources)(void);              /**< Acquire exclusive access to shared resources */
    void (*unlock_shared_resources)(void);            /**< Release exclusive access to shared resources */
    bool (*send_openlcb_msg)(openlcb_msg_t *outgoing_msg);  /**< Transmit outgoing OpenLCB message */
    openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);  /**< Get first node in enumeration */
    openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);   /**< Get next node in enumeration */
    void (*load_interaction_rejected)(openlcb_statemachine_info_t *statemachine_info);  /**< Load Interaction Rejected response */

    // Required Message Network Protocol Handler function assignments
    void (*message_network_initialization_complete)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Initialization Complete message */
    void (*message_network_initialization_complete_simple)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Initialization Complete Simple message */
    void (*message_network_verify_node_id_addressed)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Verify Node ID Addressed message */
    void (*message_network_verify_node_id_global)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Verify Node ID Global message */
    void (*message_network_verified_node_id)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Verified Node ID message */
    void (*message_network_optional_interaction_rejected)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Optional Interaction Rejected message */
    void (*message_network_terminate_due_to_error)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Terminate Due to Error message */

    // Required Protocol Support Protocol (PIP) Handler function assignments
    void (*message_network_protocol_support_inquiry)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Protocol Support Inquiry message */
    void (*message_network_protocol_support_reply)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Protocol Support Reply message */

    // Required internal function assignments (for testability)
    void (*process_main_statemachine)(openlcb_statemachine_info_t *statemachine_info);  /**< Process message through state machine */
    bool (*does_node_process_msg)(openlcb_statemachine_info_t *statemachine_info);  /**< Check if node should process message */
    bool (*handle_outgoing_openlcb_message)(void);  /**< Handle outgoing message transmission */
    bool (*handle_try_reenumerate)(void);  /**< Handle re-enumeration of current node */
    bool (*handle_try_pop_next_incoming_openlcb_message)(void);  /**< Pop next incoming message from queue */
    bool (*handle_try_enumerate_first_node)(void);  /**< Enumerate first node for message processing */
    bool (*handle_try_enumerate_next_node)(void);  /**< Enumerate next node for message processing */


    // Optional SNIP Protocol Handler function assignments
    void (*snip_simple_node_info_request)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle SNIP request (NULL if not implemented) */
    void (*snip_simple_node_info_reply)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle SNIP reply (NULL if not implemented) */

    // Optional Event Transport Protocol Handler function assignments
    void (*event_transport_consumer_identify)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Consumer Identify (NULL if not implemented) */
    void (*event_transport_consumer_range_identified)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Consumer Range Identified (NULL if not implemented) */
    void (*event_transport_consumer_identified_unknown)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Consumer Identified Unknown (NULL if not implemented) */
    void (*event_transport_consumer_identified_set)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Consumer Identified Set (NULL if not implemented) */
    void (*event_transport_consumer_identified_clear)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Consumer Identified Clear (NULL if not implemented) */
    void (*event_transport_consumer_identified_reserved)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Consumer Identified Reserved (NULL if not implemented) */
    void (*event_transport_producer_identify)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Producer Identify (NULL if not implemented) */
    void (*event_transport_producer_range_identified)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Producer Range Identified (NULL if not implemented) */
    void (*event_transport_producer_identified_unknown)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Producer Identified Unknown (NULL if not implemented) */
    void (*event_transport_producer_identified_set)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Producer Identified Set (NULL if not implemented) */
    void (*event_transport_producer_identified_clear)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Producer Identified Clear (NULL if not implemented) */
    void (*event_transport_producer_identified_reserved)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Producer Identified Reserved (NULL if not implemented) */
    void (*event_transport_identify_dest)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Identify Destination (NULL if not implemented) */
    void (*event_transport_identify)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Identify Events (NULL if not implemented) */
    void (*event_transport_learn)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Learn Event (NULL if not implemented) */
    void (*event_transport_pc_report)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Producer/Consumer Event Report (NULL if not implemented) */
    void (*event_transport_pc_report_with_payload)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle P/C Event Report with Payload (NULL if not implemented) */

    // Optional Traction Protocol Handler function assignments
    void (*traction_control_command)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Traction Control Command (NULL if not implemented) */
    void (*traction_control_reply)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Traction Control Reply (NULL if not implemented) */

    // Optional Traction SNIP Protocol Handler function assignments
    void (*simple_train_node_ident_info_request)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Simple Train Info Request (NULL if not implemented) */
    void (*simple_train_node_ident_info_reply)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Simple Train Info Reply (NULL if not implemented) */

    // Optional Datagram Protocol Handler function assignments
    void (*datagram)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Datagram (NULL if not implemented) */
    void (*datagram_ok_reply)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Datagram OK Reply (NULL if not implemented) */
    void (*datagram_rejected_reply)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Datagram Rejected Reply (NULL if not implemented) */

    // Optional Stream Protocol Handler function assignments
    void (*stream_initiate_request)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Stream Initiate Request (NULL if not implemented) */
    void (*stream_initiate_reply)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Stream Initiate Reply (NULL if not implemented) */
    void (*stream_send_data)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Stream Send Data (NULL if not implemented) */
    void (*stream_data_proceed)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Stream Data Proceed (NULL if not implemented) */
    void (*stream_data_complete)(openlcb_statemachine_info_t *statemachine_info);  /**< Handle Stream Data Complete (NULL if not implemented) */


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
        * - Application startup initialization
        * - System configuration during boot
        * - Test harness setup
        *
        * @param interface_openlcb_main_statemachine Pointer to populated interface structure
        *
        * @warning Must be called before OpenLcbMainStatemachine_run
        * @warning Interface structure must remain valid for application lifetime
        * @warning All required function pointers in interface must be non-NULL
        *
        * @attention Call only once during initialization
        * @attention Ensure interface structure is fully populated
        *
        * @see OpenLcbMainStatemachine_run - Main processing loop
        * @see interface_openlcb_main_statemachine_t - Interface structure definition
        */
    extern void OpenLcbMainStatemachine_initialize(
            const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine);

        /**
        * @brief Main state machine processing loop
        *
        * @details Executes one iteration of the state machine processing cycle.
        * Should be called as frequently as possible from the main application
        * loop to ensure responsive message handling.
        *
        * The processing sequence:
        * 1. Attempts to send any pending outgoing messages
        * 2. Handles node re-enumeration if needed for multi-message responses
        * 3. Pops next incoming message from the queue
        * 4. Enumerates nodes and dispatches message to each active node
        *
        * Each step returns early if it has work to do, ensuring fair processing
        * across all operations without blocking.
        *
        * Use cases:
        * - Called repeatedly from main application loop
        * - Invoked from RTOS task
        * - Triggered by timer interrupt
        *
        * @warning Call as fast as possible for low latency
        * @warning Do not block in this function
        *
        * @attention Requires OpenLcbMainStatemachine_initialize called first
        *
        * @see OpenLcbMainStatemachine_initialize - Must be called first
        * @see OpenLcbMainStatemachine_process_main_statemachine - Message dispatcher
        */
    extern void OpenLcbMainStatemachine_run(void);

        /**
        * @brief Processes incoming message through protocol handlers
        *
        * @details Dispatches the current incoming message to the appropriate
        * protocol handler based on its MTI value. Performs message filtering
        * to determine if the current node should process the message based on
        * addressing and node state.
        *
        * This function is exposed primarily for unit testing but can be called
        * directly if custom message processing logic is needed.
        *
        * Use cases:
        * - Internal message dispatching (called by run)
        * - Unit testing of message handling
        * - Custom message processing scenarios
        *
        * @param statemachine_info Pointer to state machine context with message and node information
        *
        * @warning Dereferences statemachine_info - must not be NULL
        * @warning Calls interface function pointers - interface must be initialized
        *
        * @attention Typically called internally by run function
        *
        * @see OpenLcbMainStatemachine_run - Main caller of this function
        * @see OpenLcbMainStatemachine_does_node_process_msg - Message filtering
        */
    extern void OpenLcbMainStatemachine_process_main_statemachine(
            openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Determines if current node should process the message
        *
        * @details Evaluates message addressing and node state to determine if
        * the message should be processed by the current node. Checks both
        * addressed and global messages, handling special cases like Verify
        * Node ID Global.
        *
        * Message is processed if:
        * - Node is initialized AND
        * - Message is not addressed (global), OR
        * - Message is addressed to this node (by alias or ID), OR
        * - Message is Verify Node ID Global (special handling)
        *
        * This function is exposed for unit testing and custom filtering logic.
        *
        * Use cases:
        * - Message filtering in main state machine
        * - Unit testing address matching
        * - Custom routing logic
        *
        * @param statemachine_info Pointer to state machine context
        *
        * @return true if node should process message, false otherwise
        *
        * @warning Returns false if statemachine_info or openlcb_node is NULL
        *
        * @note Returns false for uninitialized nodes
        *
        * @see OpenLcbMainStatemachine_process_main_statemachine - Uses this for filtering
        */
    extern bool OpenLcbMainStatemachine_does_node_process_msg(
            openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Loads an Optional Interaction Rejected response message
        *
        * @details Constructs an Optional Interaction Rejected message in response
        * to an incoming message for which no handler is registered. The response
        * includes the error code and the MTI of the message that triggered the
        * rejection.
        *
        * This function is used by the main state machine when:
        * - An optional protocol message is received with NULL handler
        * - An unknown MTI is addressed to the node
        * - A protocol is not implemented
        *
        * The response is loaded into the outgoing message buffer and marked
        * as valid for transmission.
        *
        * Use cases:
        * - Default handler for unimplemented optional protocols
        * - Response to unknown addressed messages
        * - Protocol capability advertisement
        *
        * @param statemachine_info Pointer to state machine context
        *
        * @warning Returns early if any required pointers are NULL
        * @warning Assumes outgoing message buffer is available
        *
        * @attention Used automatically by state machine for unhandled MTIs
        *
        * @see OpenLcbMainStatemachine_process_main_statemachine - Calls this for unhandled MTIs
        */
    extern void OpenLcbMainStatemachine_load_interaction_rejected(
            openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles transmission of pending outgoing messages
        *
        * @details Attempts to send any queued outgoing message through the
        * interface send function. Keeps trying until the message is successfully
        * sent, then clears the valid flag.
        *
        * This function is part of the main processing loop and is called first
        * to ensure outgoing messages have priority.
        *
        * Use cases:
        * - Outgoing message transmission in run loop
        * - Response message handling
        * - Unit testing of send logic
        *
        * @return true if message pending (keep calling), false if no message to send
        *
        * @attention Exposed for testing, normally called by run function
        *
        * @see OpenLcbMainStatemachine_run - Main caller
        */
    extern bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void);

        /**
        * @brief Handles re-enumeration for multi-message responses
        *
        * @details When a protocol handler needs to send multiple messages in
        * response to a single incoming message, it sets the enumerate flag.
        * This function continues processing the same incoming message against
        * the current node until the handler clears the flag.
        *
        * Use cases:
        * - Multi-message protocol responses
        * - Event identification with multiple events
        * - Large data transfers requiring segmentation
        *
        * @return true if re-enumeration active (keep calling), false if complete
        *
        * @attention Exposed for testing, normally called by run function
        *
        * @see OpenLcbMainStatemachine_run - Main caller
        */
    extern bool OpenLcbMainStatemachine_handle_try_reenumerate(void);

        /**
        * @brief Pops next incoming message from receive queue
        *
        * @details Retrieves the next incoming message from the buffer FIFO
        * if no message is currently being processed. Uses resource locking
        * to ensure thread-safe access to the shared FIFO.
        *
        * Use cases:
        * - Message queue management in run loop
        * - Incoming message retrieval
        * - Unit testing message flow
        *
        * @return true if message popped (continue processing), false if already have message
        *
        * @note Uses lock/unlock callbacks for thread safety
        *
        * @attention Exposed for testing, normally called by run function
        *
        * @see OpenLcbMainStatemachine_run - Main caller
        * @see OpenLcbBufferFifo_pop - FIFO access function
        */
    extern bool OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message(void);

        /**
        * @brief Enumerates first node for message processing
        *
        * @details Retrieves the first node from the node list and processes
        * the current incoming message against it if the node is in RUN state.
        * If no nodes exist, frees the incoming message.
        *
        * Use cases:
        * - Node enumeration in run loop
        * - First node message processing
        * - Unit testing enumeration logic
        *
        * @return true if enumeration complete (stop), false if no node yet
        *
        * @note Skips nodes not in RUNSTATE_RUN
        * @note Frees message if no nodes allocated
        *
        * @attention Exposed for testing, normally called by run function
        *
        * @see OpenLcbMainStatemachine_run - Main caller
        * @see OpenLcbMainStatemachine_handle_try_enumerate_next_node - Continues enumeration
        */
    extern bool OpenLcbMainStatemachine_handle_try_enumerate_first_node(void);

        /**
        * @brief Enumerates next node for message processing
        *
        * @details Retrieves the next node from the node list and processes
        * the current incoming message against it if the node is in RUN state.
        * When enumeration completes (no more nodes), frees the incoming message.
        *
        * Use cases:
        * - Node enumeration continuation in run loop
        * - Multi-node message processing
        * - Unit testing enumeration logic
        *
        * @return true if enumeration active (keep calling), false if no current node
        *
        * @note Skips nodes not in RUNSTATE_RUN
        * @note Frees message when enumeration complete
        *
        * @attention Exposed for testing, normally called by run function
        *
        * @see OpenLcbMainStatemachine_run - Main caller
        * @see OpenLcbMainStatemachine_handle_try_enumerate_first_node - Starts enumeration
        */
    extern bool OpenLcbMainStatemachine_handle_try_enumerate_next_node(void);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_MAIN_STATEMACHINE__ */
