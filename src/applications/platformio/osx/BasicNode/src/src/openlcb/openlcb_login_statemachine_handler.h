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
* @file openlcb_login_statemachine_handler.h
* @brief Login state machine message handler for OpenLCB initialization and event broadcasting
*
* @details This module provides message construction handlers for the OpenLCB login sequence.
* It handles Initialization Complete message generation, Producer Event Identified messages,
* and Consumer Event Identified messages with appropriate state indicators.
*
* The handlers work in conjunction with openlcb_login_statemachine to orchestrate the
* complete login sequence per OpenLCB Message Network Standard.
*
* @author Jim Kueneman
* @date 17 Jan 2026
*
* @see openlcb_login_statemachine.h - Main state machine dispatcher
* @see OpenLCB Message Network Standard S-9.7.3.1 - Initialization Complete
* @see OpenLCB Event Transport Standard S-9.7.4 - Event Transport Protocol
*/

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__
#define    __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
    * @brief Interface structure for login message handler callback functions
    *
    * @details This structure defines the callback interface for the OpenLCB login
    * message handler, which constructs Initialization Complete and event identification
    * messages during the node login sequence as defined in the OpenLCB Message Network
    * and Event Transport Protocol specifications.
    *
    * The interface provides callbacks that allow the login handler to query application-specific
    * event states and generate appropriate Producer Identified and Consumer Identified messages
    * with the correct state indicators (Valid, Invalid, or Unknown).
    *
    * The login sequence consists of three phases:
    * 1. Send Initialization Complete message (announces node presence)
    * 2. Send Producer Identified messages for all produced events
    * 3. Send Consumer Identified messages for all consumed events
    *
    * Both callbacks are required to support event state reporting during the login process.
    * The callbacks must examine the current state of each event (set, clear, or unknown)
    * and return the appropriate MTI value for the corresponding Producer or Consumer
    * Identified message.
    *
    * @note Required callbacks must be set before calling OpenLcbLoginMessageHandler_initialize
    * @note Both callbacks are REQUIRED - neither can be NULL
    *
    * @see OpenLcbLoginMessageHandler_initialize
    * @see OpenLcbLoginMessageHandler_load_initialization_complete
    * @see OpenLcbLoginMessageHandler_load_producer_event
    * @see OpenLcbLoginMessageHandler_load_consumer_event
    */
typedef struct {

        /**
         * @brief Callback to extract Producer Event Identified MTI for a given event
         *
         * @details This required callback examines the current state of a producer event
         * and returns the appropriate Message Type Indicator (MTI) for the Producer Identified
         * message. The returned MTI indicates whether the producer event is currently in a
         * valid (set/active), invalid (clear/inactive), or unknown state.
         *
         * The callback should examine the event at producers.list[event_index] and return:
         * - MTI_PRODUCER_IDENTIFIED_VALID (0x0594) if event state is SET/ACTIVE
         * - MTI_PRODUCER_IDENTIFIED_INVALID (0x0595) if event state is CLEAR/INACTIVE
         * - MTI_PRODUCER_IDENTIFIED_UNKNOWN (0x0597) if event state cannot be determined
         *
         * This callback is invoked during the login sequence for each producer event the
         * node supports, allowing the node to announce all produced events and their current
         * states to the network.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    uint16_t(*extract_producer_event_state_mti)(openlcb_node_t *openlcb_node, uint16_t event_index);

        /**
         * @brief Callback to extract Consumer Event Identified MTI for a given event
         *
         * @details This required callback examines the current state of a consumer event
         * and returns the appropriate Message Type Indicator (MTI) for the Consumer Identified
         * message. The returned MTI indicates whether the consumer event is currently in a
         * valid (set/active), invalid (clear/inactive), or unknown state.
         *
         * The callback should examine the event at consumers.list[event_index] and return:
         * - MTI_CONSUMER_IDENTIFIED_VALID (0x04C4) if event state is SET/ACTIVE
         * - MTI_CONSUMER_IDENTIFIED_INVALID (0x04C5) if event state is CLEAR/INACTIVE
         * - MTI_CONSUMER_IDENTIFIED_UNKNOWN (0x04C7) if event state cannot be determined
         *
         * This callback is invoked during the login sequence for each consumer event the
         * node supports, allowing the node to announce all consumed events and their current
         * states to the network.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    uint16_t(*extract_consumer_event_state_mti)(openlcb_node_t *openlcb_node, uint16_t event_index);

} interface_openlcb_login_message_handler_t;


#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
        * @brief Initializes the login message handler with callback interface
        *
        * @details Registers the application's callback interface with the login message handler.
        * The interface provides callbacks for extracting producer and consumer event state
        * information, which is used to generate appropriate Event Identified messages during
        * the login sequence.
        *
        * Must be called once during system initialization before any login sequence processing
        * begins. The interface structure is stored internally and must remain valid for the
        * lifetime of the application.
        *
        * Use cases:
        * - Called once during application startup
        * - Must be called before openlcb_login_statemachine begins processing nodes
        *
        * @param interface Pointer to callback interface structure containing event state extraction functions
        *
        * @warning interface pointer must remain valid for lifetime of application
        * @warning Both callback functions in the interface must be valid (non-NULL)
        * @warning MUST be called exactly once during initialization
        * @warning NOT thread-safe - call during single-threaded initialization only
        *
        * @attention Call during initialization phase before login processing begins
        * @attention Do not call from interrupt context
        *
        * @see interface_openlcb_login_message_handler_t - Interface structure definition
        * @see OpenLcbLoginMessageHandler_load_producer_event - Uses extract_producer_event_state_mti callback
        * @see OpenLcbLoginMessageHandler_load_consumer_event - Uses extract_consumer_event_state_mti callback
        */
    extern void OpenLcbLoginMessageHandler_initialize(const interface_openlcb_login_message_handler_t *interface);

        /**
        * @brief Loads an Initialization Complete message into the outgoing message buffer
        *
        * @details Constructs an Initialization Complete message per OpenLCB Message Network Standard.
        * The message contains the node's 48-bit Node ID and uses either Simple or Full protocol MTI
        * based on the node's configuration. After loading the message, marks the node as initialized
        * and transitions state to begin producer event enumeration.
        *
        * This is the first message in the three-phase login sequence:
        * 1. Initialization Complete (this function)
        * 2. Producer Event Identified messages (all produced events)
        * 3. Consumer Event Identified messages (all consumed events)
        *
        * The message format is:
        * - MTI: 0x0100 (Full protocol) or 0x0101 (Simple protocol)
        * - Source: Node's alias and full Node ID
        * - Payload: 6-byte Node ID
        *
        * Use cases:
        * - Called during node login sequence after CAN alias reservation
        * - Announces node's presence and readiness on the network
        *
        * @param statemachine_info Pointer to state machine context containing node and message buffer
        *
        * @warning statemachine_info must NOT be NULL
        * @warning Node's alias must already be allocated and validated
        * @warning Outgoing message buffer must be allocated
        *
        * @attention This message must be sent before any event messages
        * @attention Sets node's initialized flag to true
        * @attention Transitions to producer event enumeration state
        *
        * @note Sets outgoing_msg_info.valid to true to trigger transmission
        * @note Protocol type (Simple vs Full) determined by node's PSI_SIMPLE flag
        * @note After transmission, producer event enumeration begins
        *
        * @see OpenLCB Message Network Standard S-9.7.3.1 - Initialization Complete
        * @see MTI_INITIALIZATION_COMPLETE in openlcb_defines.h
        * @see OpenLcbLoginMessageHandler_load_producer_event - Next step in sequence
        */
    extern void OpenLcbLoginMessageHandler_load_initialization_complete(openlcb_login_statemachine_info_t *statemachine_info);

        /**
        * @brief Loads a Producer Event Identified message for the current producer event
        *
        * @details Constructs a Producer Identified message for one event in the node's
        * producer list. The message indicates whether the producer event is currently
        * valid (set/active), invalid (clear/inactive), or in an unknown state. Uses the
        * extract_producer_event_state_mti callback to determine the appropriate MTI.
        *
        * This function is called repeatedly via the enumeration mechanism to send one
        * Producer Identified message for each event in the producers.list array. The
        * enumerate flag is set to true after each message to trigger re-entry until
        * all producer events have been announced.
        *
        * Message format for each producer event:
        * - MTI: 0x0594 (Valid), 0x0595 (Invalid), or 0x0597 (Unknown)
        * - Source: Node's alias and Node ID
        * - Payload: 8-byte Event ID
        *
        * Use cases:
        * - Called repeatedly during login to announce each produced event
        * - May be called later in response to Identify Producer messages
        *
        * @param statemachine_info Pointer to state machine context containing node and message buffer
        *
        * @warning statemachine_info must NOT be NULL
        * @warning enum_index must be less than producers.count when enumerate flag is set
        * @warning extract_producer_event_state_mti callback must be valid
        *
        * @attention This function may be called multiple times (once per producer event)
        * @attention The enumerate flag controls re-entry until all events are sent
        * @attention After all producers sent, transitions to consumer enumeration
        *
        * @note If producers.count is 0, no messages are generated and state transitions immediately
        * @note Sets outgoing_msg_info.valid to false if no producers exist
        * @note Sets outgoing_msg_info.enumerate to true to process next event
        * @note Increments enum_index after each message
        *
        * @see OpenLCB Event Transport Standard S-9.7.4.1 - Producer Identified
        * @see MTI_PRODUCER_IDENTIFIED_VALID in openlcb_defines.h
        * @see OpenLcbLoginMessageHandler_load_consumer_event - Next step after all producers
        * @see interface_openlcb_login_message_handler_t - Provides extract_producer_event_state_mti callback
        */
    extern void OpenLcbLoginMessageHandler_load_producer_event(openlcb_login_statemachine_info_t *statemachine_info);

        /**
        * @brief Loads a Consumer Event Identified message for the current consumer event
        *
        * @details Constructs a Consumer Identified message for one event in the node's
        * consumer list. The message indicates whether the consumer event is currently
        * valid (set/active), invalid (clear/inactive), or in an unknown state. Uses the
        * extract_consumer_event_state_mti callback to determine the appropriate MTI.
        *
        * This function is called repeatedly via the enumeration mechanism to send one
        * Consumer Identified message for each event in the consumers.list array. The
        * enumerate flag is set to true after each message to trigger re-entry until
        * all consumer events have been announced. This is the final step of the login
        * sequence - after all consumer events are sent, the node transitions to
        * RUNSTATE_LOGIN_COMPLETE, where the state machine dispatcher will call the
        * optional on_login_complete callback before transitioning to RUNSTATE_RUN.
        *
        * Message format for each consumer event:
        * - MTI: 0x04C4 (Valid), 0x04C5 (Invalid), or 0x04C7 (Unknown)
        * - Source: Node's alias and Node ID
        * - Payload: 8-byte Event ID
        *
        * Use cases:
        * - Called repeatedly during login to announce each consumed event
        * - May be called later in response to Identify Consumer messages
        *
        * @param statemachine_info Pointer to state machine context containing node and message buffer
        *
        * @warning statemachine_info must NOT be NULL
        * @warning enum_index must be less than consumers.count when enumerate flag is set
        * @warning extract_consumer_event_state_mti callback must be valid
        *
        * @attention This function may be called multiple times (once per consumer event)
        * @attention The enumerate flag controls re-entry until all events are sent
        * @attention This is the final step of the login sequence
        * @attention After completion, node transitions to RUNSTATE_LOGIN_COMPLETE
        *
        * @note If consumers.count is 0, no messages are generated and state transitions immediately
        * @note Sets outgoing_msg_info.valid to false if no consumers exist
        * @note Sets outgoing_msg_info.enumerate to true to process next event
        * @note Increments enum_index after each message
        * @note Transitions to RUNSTATE_LOGIN_COMPLETE when complete
        *
        * @see OpenLCB Event Transport Standard S-9.7.4.2 - Consumer Identified
        * @see MTI_CONSUMER_IDENTIFIED_VALID in openlcb_defines.h
        * @see RUNSTATE_LOGIN_COMPLETE in openlcb_defines.h
        * @see interface_openlcb_login_message_handler_t - Provides extract_consumer_event_state_mti callback
        */
    extern void OpenLcbLoginMessageHandler_load_consumer_event(openlcb_login_statemachine_info_t *statemachine_info);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__ */
