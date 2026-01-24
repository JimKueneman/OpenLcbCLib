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
* @file openlcb_login_statemachine_handler.c
* @brief Implementation of the login state machine message handler
*
* @details This module implements the message construction handlers for the OpenLCB
* login sequence. It builds properly formatted OpenLCB messages for:
* - Initialization Complete (Simple and Full protocol variants)
* - Producer Event Identified (with Valid, Invalid, Unknown states)
* - Consumer Event Identified (with Valid, Invalid, Unknown states)
*
* The implementation follows the OpenLCB Message Network Standard and Event Transport
* specifications. Each handler function:
* -# Determines the appropriate MTI based on node configuration or event state
* -# Loads the message structure with source alias, destination, and MTI
* -# Copies the payload data (Node ID or Event ID) into the message
* -# Sets the payload count
* -# Updates the node's state machine state
* -# Sets flags to control message transmission and enumeration
*
* Message Construction Pattern:
* All messages use OpenLcbUtilities_load_openlcb_message() to set the header fields,
* then use specific copy functions for payload data (Node ID or Event ID), and finally
* set the payload_count to indicate message size.
*
* State Transitions:
* - load_initialization_complete: RUNSTATE_LOAD_INITIALIZATION_COMPLETE -> RUNSTATE_LOAD_PRODUCER_EVENTS
* - load_producer_event: RUNSTATE_LOAD_PRODUCER_EVENTS -> (enumerate) -> RUNSTATE_LOAD_CONSUMER_EVENTS
* - load_consumer_event: RUNSTATE_LOAD_CONSUMER_EVENTS -> (enumerate) -> RUNSTATE_RUN
*
* @author Jim Kueneman
* @date 17 Jan 2026
*
* @see openlcb_login_statemachine.c - State machine dispatcher that calls these handlers
* @see OpenLCB Message Network Standard S-9.7.3.1 - Initialization Complete
* @see OpenLCB Event Transport Standard S-9.7.4 - Event Transport Protocol
*/

#include "openlcb_login_statemachine_handler.h"

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_defines.h"
#include "openlcb_utilities.h"


    /** @brief Static pointer to interface callbacks for event state extraction */
static const interface_openlcb_login_message_handler_t *_interface;

    /**
    * @brief Initializes the login message handler with callback interface
    *
    * @details Algorithm:
    * Stores the interface function pointers in static memory for use by the
    * message construction handlers. The interface provides callbacks to:
    * - Extract producer event state MTI (Valid/Invalid/Unknown)
    * - Extract consumer event state MTI (Valid/Invalid/Unknown)
    *
    * These callbacks allow the handler to be decoupled from application-specific
    * event state logic while still generating correct Producer/Consumer Identified
    * messages during the login sequence.
    *
    * Use cases:
    * - Called once during application startup
    * - Must be called before nodes begin login sequence
    *
    * @verbatim
    * @param interface Pointer to interface structure containing callback functions
    * @endverbatim
    *
    * @warning MUST be called exactly once during initialization
    * @warning The interface pointer is stored in static memory - the pointed-to
    *          structure must remain valid for the lifetime of the program
    * @warning NOT thread-safe
    *
    * @attention Call during single-threaded initialization phase
    * @attention Both callback functions in the interface must be valid (non-NULL)
    *
    * @see interface_openlcb_login_message_handler_t - Interface structure definition
    * @see OpenLcbLoginMessageHandler_load_producer_event - Uses extract_producer callback
    * @see OpenLcbLoginMessageHandler_load_consumer_event - Uses extract_consumer callback
    */
void OpenLcbLoginMessageHandler_initialize(const interface_openlcb_login_message_handler_t *interface) {

    _interface = interface;

}

    /**
    * @brief Loads an Initialization Complete message into the outgoing message buffer
    *
    * @details Algorithm:
    * -# Determines protocol type (Simple vs Full) from node's PSI_SIMPLE flag
    * -# Selects appropriate MTI (0x0100 for Full, 0x0101 for Simple)
    * -# Calls OpenLcbUtilities_load_openlcb_message() with node alias, ID, and MTI
    * -# Calls OpenLcbUtilities_copy_node_id_to_openlcb_payload() to add Node ID to payload
    * -# Sets payload_count to 6 (48-bit Node ID)
    * -# Marks node as initialized
    * -# Initializes producer event enumeration (running=true, index=0)
    * -# Clears consumer event enumeration (running=false, index=0)
    * -# Sets outgoing_msg_info.valid to trigger message transmission
    * -# Transitions state to RUNSTATE_LOAD_PRODUCER_EVENTS
    *
    * Message Structure:
    * - MTI: 0x0100 (MTI_INITIALIZATION_COMPLETE) or 0x0101 (MTI_INITIALIZATION_COMPLETE_SIMPLE)
    * - Source Alias: Node's allocated alias
    * - Payload: 6 bytes containing the 48-bit Node ID
    * - Destination: None (broadcast message)
    *
    * State Changes:
    * - node->state.initialized: false -> true
    * - node->producers.enumerator.running: -> true
    * - node->producers.enumerator.enum_index: -> 0
    * - node->consumers.enumerator.running: -> false
    * - node->consumers.enumerator.enum_index: -> 0
    * - node->state.run_state: RUNSTATE_LOAD_INITIALIZATION_COMPLETE -> RUNSTATE_LOAD_PRODUCER_EVENTS
    *
    * Use cases:
    * - Called during node login sequence after alias reservation
    * - Announces node's presence and protocol support on the network
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine info containing node and message buffer
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Node pointer within info structure must be valid
    * @warning Message buffer pointer must be allocated
    *
    * @attention This message must be sent before any event messages
    * @attention The node's alias must already be allocated and validated
    *
    * @note Sets outgoing_msg_info.valid to true to trigger transmission
    * @note Protocol type (Simple vs Full) determined by PSI_SIMPLE flag in node parameters
    *
    * @see OpenLCB Message Network Standard S-9.7.3.1 - Initialization Complete
    * @see MTI_INITIALIZATION_COMPLETE in openlcb_defines.h
    * @see PSI_SIMPLE in openlcb_defines.h
    * @see OpenLcbUtilities_load_openlcb_message - Message header construction
    * @see OpenLcbUtilities_copy_node_id_to_openlcb_payload - Payload construction
    */
void OpenLcbLoginMessageHandler_load_initialization_complete(openlcb_login_statemachine_info_t *statemachine_info) {

    uint16_t mti = MTI_INITIALIZATION_COMPLETE;

    if (statemachine_info->openlcb_node->parameters->protocol_support & PSI_SIMPLE) {

        mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            0,
            0,
            mti);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->id,
            0);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 6;

    statemachine_info->openlcb_node->state.initialized = true;
    statemachine_info->openlcb_node->producers.enumerator.running = true;
    statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
    statemachine_info->openlcb_node->producers.enumerator.range_enum_index = 0;
    statemachine_info->openlcb_node->consumers.enumerator.running = false;
    statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;
    statemachine_info->openlcb_node->consumers.enumerator.range_enum_index = 0; 
    statemachine_info->outgoing_msg_info.valid = true;

    statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

}

    /**
    * @brief Loads a Producer Event Identified message for the current producer event
    *
    * @details Algorithm:
    * -# Checks if node has any producer events (producers.count == 0)
    * -# If no producers: Sets valid=false, transitions to RUNSTATE_LOAD_CONSUMER_EVENTS, returns
    * -# Calls interface->extract_producer_event_state_mti() to get event state MTI
    * -# Retrieves event ID from producers.list[enum_index]
    * -# Calls OpenLcbUtilities_load_openlcb_message() with node alias, ID, and event MTI
    * -# Calls OpenLcbUtilities_copy_event_id_to_openlcb_payload() to add Event ID to payload
    * -# Sets payload_count to 8 (64-bit Event ID)
    * -# Increments enum_index
    * -# Sets enumerate=true and valid=true to trigger transmission and re-entry
    * -# If enum_index >= producers.count: Resets enumeration, transitions to RUNSTATE_LOAD_CONSUMER_EVENTS
    *
    * Message Structure:
    * - MTI: 0x0914 (Valid), 0x0915 (Invalid), or 0x0917 (Unknown)
    * - Source Alias: Node's allocated alias
    * - Payload: 8 bytes containing the 64-bit Event ID
    * - Destination: None (broadcast message)
    *
    * Enumeration Control:
    * The enumerate flag causes the state machine to re-enter this function for the
    * next producer event without advancing to the next state. This continues until
    * all producer events have been announced.
    *
    * State Changes (when all producers enumerated):
    * - node->producers.enumerator.enum_index: -> 0
    * - node->producers.enumerator.running: true -> false
    * - node->consumers.enumerator.enum_index: -> 0
    * - node->consumers.enumerator.running: -> true
    * - outgoing_msg_info.enumerate: true -> false
    * - node->state.run_state: RUNSTATE_LOAD_PRODUCER_EVENTS -> RUNSTATE_LOAD_CONSUMER_EVENTS
    *
    * Use cases:
    * - Called repeatedly during login to announce each produced event
    * - May be called later in response to Identify Producer messages
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine info containing node and message buffer
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Node pointer within info structure must be valid
    * @warning Enumeration index must be less than producers.count when enumerate flag is set
    *
    * @attention This function may be called multiple times (once per producer event)
    * @attention The enumerate flag controls re-entry until all events are sent
    *
    * @note If producers.count is 0, no messages are generated
    * @note Sets outgoing_msg_info.valid to false if no producers exist
    * @note Sets outgoing_msg_info.enumerate to true to process next event
    * @note Line length managed per StyleGuide 3x indentation for long function calls
    *
    * @see OpenLCB Event Transport Standard S-9.7.4.1 - Producer Identified
    * @see MTI_PRODUCER_IDENTIFIED_VALID in openlcb_defines.h
    * @see interface_openlcb_login_message_handler_t - Interface for state extraction
    * @see OpenLcbUtilities_load_openlcb_message - Message header construction
    * @see OpenLcbUtilities_copy_event_id_to_openlcb_payload - Payload construction
    */
void OpenLcbLoginMessageHandler_load_producer_event(openlcb_login_statemachine_info_t *statemachine_info) {

    // No producers - skip to consumers

    if ((statemachine_info->openlcb_node->producers.count == 0) && (statemachine_info->openlcb_node->producers.range_count == 0)) {

        statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

        statemachine_info->outgoing_msg_info.valid = false;

        return;

    }

    event_id_t event_id = NULL_EVENT_ID;

    // First attack any ranges

    if (statemachine_info->openlcb_node->producers.enumerator.range_enum_index < statemachine_info->openlcb_node->producers.range_count) {

        OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            0,
            0,
            MTI_PRODUCER_RANGE_IDENTIFIED);

        event_id = OpenLcbUtilities_generate_event_range_id(
            statemachine_info->openlcb_node->producers.range_list[statemachine_info->openlcb_node->producers.enumerator.range_enum_index].start_base,
            statemachine_info->openlcb_node->producers.range_list[statemachine_info->openlcb_node->producers.enumerator.range_enum_index].event_count);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            event_id);

        statemachine_info->openlcb_node->producers.enumerator.range_enum_index++;

        statemachine_info->outgoing_msg_info.enumerate = true;
        statemachine_info->outgoing_msg_info.valid = true;

        return;
    }

    // Now handle normal events

    if (statemachine_info->openlcb_node->producers.enumerator.enum_index < statemachine_info->openlcb_node->producers.count) {

        uint16_t event_mti = _interface->extract_producer_event_state_mti(
                statemachine_info->openlcb_node,
                statemachine_info->openlcb_node->producers.enumerator.enum_index);

        event_id = statemachine_info->openlcb_node->producers.list[statemachine_info->openlcb_node->producers.enumerator.enum_index].event;

        OpenLcbUtilities_load_openlcb_message(
                statemachine_info->outgoing_msg_info.msg_ptr,
                statemachine_info->openlcb_node->alias,
                statemachine_info->openlcb_node->id,
                0,
                0,
                event_mti);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
                event_id);

        statemachine_info->openlcb_node->producers.enumerator.enum_index++;

        statemachine_info->outgoing_msg_info.enumerate = true;
        statemachine_info->outgoing_msg_info.valid = true;

        return;

    }

    // We are done

    statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
    statemachine_info->openlcb_node->producers.enumerator.range_enum_index = 0;
    statemachine_info->openlcb_node->producers.enumerator.running = false;

    statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;
    statemachine_info->openlcb_node->consumers.enumerator.range_enum_index = 0;
    statemachine_info->openlcb_node->consumers.enumerator.running = true;

    statemachine_info->outgoing_msg_info.enumerate = false;
    statemachine_info->outgoing_msg_info.valid = false;

    statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

}

    /**
    * @brief Loads a Consumer Event Identified message for the current consumer event
    *
    * @details Algorithm:
    * -# Checks if node has any consumer events (consumers.count == 0)
    * -# If no consumers: Sets valid=false, transitions to RUNSTATE_RUN, returns
    * -# Calls interface->extract_consumer_event_state_mti() to get event state MTI
    * -# Retrieves event ID from consumers.list[enum_index]
    * -# Calls OpenLcbUtilities_load_openlcb_message() with node alias, ID, and event MTI
    * -# Calls OpenLcbUtilities_copy_event_id_to_openlcb_payload() to add Event ID to payload
    * -# Sets payload_count to 8 (64-bit Event ID)
    * -# Increments enum_index
    * -# Sets enumerate=true and valid=true to trigger transmission and re-entry
    * -# If enum_index >= consumers.count: Resets enumeration, transitions to RUNSTATE_RUN
    *
    * Message Structure:
    * - MTI: 0x04C4 (Valid), 0x04C5 (Invalid), or 0x04C7 (Unknown)
    * - Source Alias: Node's allocated alias
    * - Payload: 8 bytes containing the 64-bit Event ID
    * - Destination: None (broadcast message)
    *
    * Enumeration Control:
    * The enumerate flag causes the state machine to re-enter this function for the
    * next consumer event without advancing to the next state. This continues until
    * all consumer events have been announced.
    *
    * State Changes (when all consumers enumerated):
    * - node->consumers.enumerator.running: true -> false
    * - node->consumers.enumerator.enum_index: -> 0
    * - outgoing_msg_info.enumerate: true -> false
    * - node->state.run_state: RUNSTATE_LOAD_CONSUMER_EVENTS -> RUNSTATE_RUN
    *
    * Login Completion:
    * When this function transitions to RUNSTATE_RUN, the login sequence is complete
    * and the node is fully operational on the network.
    *
    * Use cases:
    * - Called repeatedly during login to announce each consumed event
    * - May be called later in response to Identify Consumer messages
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine info containing node and message buffer
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Node pointer within info structure must be valid
    * @warning Enumeration index must be less than consumers.count when enumerate flag is set
    *
    * @attention This function may be called multiple times (once per consumer event)
    * @attention The enumerate flag controls re-entry until all events are sent
    * @attention This is the final step of the login sequence
    *
    * @note If consumers.count is 0, no messages are generated
    * @note Sets outgoing_msg_info.valid to false if no consumers exist
    * @note Sets outgoing_msg_info.enumerate to true to process next event
    * @note Transitions to RUNSTATE_RUN when complete (node fully initialized)
    * @note Line length managed per StyleGuide 3x indentation for long function calls
    *
    * @see OpenLCB Event Transport Standard S-9.7.4.2 - Consumer Identified
    * @see MTI_CONSUMER_IDENTIFIED_VALID in openlcb_defines.h
    * @see RUNSTATE_RUN in openlcb_defines.h - Final operational state
    * @see interface_openlcb_login_message_handler_t - Interface for state extraction
    * @see OpenLcbUtilities_load_openlcb_message - Message header construction
    * @see OpenLcbUtilities_copy_event_id_to_openlcb_payload - Payload construction
    */
void OpenLcbLoginMessageHandler_load_consumer_event(openlcb_login_statemachine_info_t *statemachine_info) {

    // No consumers - we are done

    if ((statemachine_info->openlcb_node->consumers.count == 0) && (statemachine_info->openlcb_node->consumers.range_count == 0)) {

        statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;

        statemachine_info->outgoing_msg_info.valid = false;

        return;

    }

    event_id_t event_id = NULL_EVENT_ID;

    // First attack any ranges

    if (statemachine_info->openlcb_node->consumers.enumerator.range_enum_index < statemachine_info->openlcb_node->consumers.range_count) {

        OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            0,
            0,
            MTI_CONSUMER_RANGE_IDENTIFIED);

        event_id = OpenLcbUtilities_generate_event_range_id(
            statemachine_info->openlcb_node->consumers.range_list[statemachine_info->openlcb_node->consumers.enumerator.range_enum_index].start_base,
            statemachine_info->openlcb_node->consumers.range_list[statemachine_info->openlcb_node->consumers.enumerator.range_enum_index].event_count);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            event_id);

        statemachine_info->openlcb_node->consumers.enumerator.range_enum_index++;

        statemachine_info->outgoing_msg_info.enumerate = true;
        statemachine_info->outgoing_msg_info.valid = true;

        return;
    }

    // Now handle normal events

    if (statemachine_info->openlcb_node->consumers.enumerator.enum_index < statemachine_info->openlcb_node->consumers.count) {

        uint16_t event_mti = _interface->extract_consumer_event_state_mti(
                statemachine_info->openlcb_node, 
                statemachine_info->openlcb_node->consumers.enumerator.enum_index);

        event_id = statemachine_info->openlcb_node->consumers.list[statemachine_info->openlcb_node->consumers.enumerator.enum_index].event;

        OpenLcbUtilities_load_openlcb_message(
                statemachine_info->outgoing_msg_info.msg_ptr,
                statemachine_info->openlcb_node->alias,
                statemachine_info->openlcb_node->id,
                0,
                0,
                event_mti);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
                event_id);

        statemachine_info->openlcb_node->consumers.enumerator.enum_index++;

        statemachine_info->outgoing_msg_info.enumerate = true;
        statemachine_info->outgoing_msg_info.valid = true;

        return;
    }
    // We are done

    statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
    statemachine_info->openlcb_node->producers.enumerator.range_enum_index = 0;
    statemachine_info->openlcb_node->producers.enumerator.running = false;

    statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;
    statemachine_info->openlcb_node->consumers.enumerator.range_enum_index = 0;
    statemachine_info->openlcb_node->consumers.enumerator.running = false;

    statemachine_info->outgoing_msg_info.enumerate = false;
    statemachine_info->outgoing_msg_info.valid = false; 

    statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;
}
