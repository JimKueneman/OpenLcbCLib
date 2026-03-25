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
* - load_consumer_event: RUNSTATE_LOAD_CONSUMER_EVENTS -> (enumerate) -> RUNSTATE_LOGIN_COMPLETE
*
* @author Jim Kueneman
* @date 28 Feb 2026
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
    * @brief Stores the callback interface.  Call once at startup.
    *
    * @verbatim
    * @param interface Pointer to interface structure containing callback functions
    * @endverbatim
    */
void OpenLcbLoginMessageHandler_initialize(const interface_openlcb_login_message_handler_t *interface) {

    _interface = interface;

}

    /**
    * @brief Builds the Initialization Complete message and transitions to producer
    *        event enumeration.
    *
    * @details Algorithm:
    * -# Pick MTI_INITIALIZATION_COMPLETE or _SIMPLE based on PSI_SIMPLE flag
    * -# Load message header and copy 6-byte Node ID into payload
    * -# Mark node initialized, set up producer enumerator, set valid flag
    * -# Transition to RUNSTATE_LOAD_PRODUCER_EVENTS
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine info containing node and message buffer
    * @endverbatim
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
    * @brief Builds one Producer Identified message; sets enumerate flag if more remain.
    *
    * @details Algorithm:
    * -# If no producers, skip to RUNSTATE_LOAD_CONSUMER_EVENTS
    * -# Emit range events first, then normal events
    * -# For each event: get MTI from callback, copy Event ID to payload
    * -# Set enumerate=true and valid=true for each message
    * -# When all done, reset enumerator and transition to consumer events
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine info containing node and message buffer
    * @endverbatim
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
    * @brief Builds one Consumer Identified message; sets enumerate flag if more remain.
    *
    * @details Algorithm:
    * -# If no consumers, skip to RUNSTATE_LOGIN_COMPLETE
    * -# Emit range events first, then normal events
    * -# For each event: get MTI from callback, copy Event ID to payload
    * -# Set enumerate=true and valid=true for each message
    * -# When all done, reset enumerator and transition to RUNSTATE_LOGIN_COMPLETE
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine info containing node and message buffer
    * @endverbatim
    */
void OpenLcbLoginMessageHandler_load_consumer_event(openlcb_login_statemachine_info_t *statemachine_info) {

    // No consumers - we are done

    if ((statemachine_info->openlcb_node->consumers.count == 0) && (statemachine_info->openlcb_node->consumers.range_count == 0)) {

        statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOGIN_COMPLETE;

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

    statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOGIN_COMPLETE;
}
