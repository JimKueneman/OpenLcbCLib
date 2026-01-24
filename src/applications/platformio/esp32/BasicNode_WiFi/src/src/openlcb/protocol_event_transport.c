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
* @file protocol_event_transport.c
* @brief Implementation of event transport protocol
* @author Jim Kueneman
* @date 17 Jan 2026
*/

#include "protocol_event_transport.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"

// Interface reserved for future callbacks and configuration
// Currently stores callback pointers for event notifications
static const interface_openlcb_protocol_event_transport_t *_interface;

    /**
    * @brief Initializes the Event Transport protocol layer
    *
    * @details Algorithm:
    * -# Store pointer to callback interface in static variable
    * -# Interface remains valid for application lifetime
    *
    * Use cases:
    * - Called during application startup
    * - Required before processing any event messages
    *
    * @verbatim
    * @param interface_openlcb_protocol_event_transport Pointer to callback interface structure
    * @endverbatim
    *
    * @warning Pointer must remain valid for lifetime of application
    * @warning NOT thread-safe - call during single-threaded initialization only
    *
    * @attention Call before enabling CAN message reception
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback interface structure
    */
void ProtocolEventTransport_initialize(const interface_openlcb_protocol_event_transport_t *interface_openlcb_protocol_event_transport) {

    _interface = interface_openlcb_protocol_event_transport;

}

    // Precondition: enum_index must be < producers.count (verified by caller)
    /**
    * @brief Identifies a producer event and prepares response message
    *
    * @details Algorithm:
    * -# If enumeration not already running, mark it as started
    * -# Construct Producer Identified message with appropriate MTI
    * -# Extract event status MTI based on current producer event state
    * -# Copy producer event ID to outgoing message payload
    * -# Increment enumeration index for next event
    * -# Mark outgoing message as valid for transmission
    *
    * Use cases:
    * - Called iteratively during event enumeration
    * - Responds to producer identify requests
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Caller must ensure enum_index < producers.count
    *
    * @note This is a helper function for ProtocolEventTransport_handle_events_identify
    * @note Payload count is set automatically by OpenLcbUtilities_copy_event_id_to_openlcb_payload
    *
    * @see ProtocolEventTransport_handle_events_identify - Main enumeration handler
    * @see ProtocolEventTransport_extract_producer_event_status_mti - Determines response MTI
    */
static bool _identify_producers(openlcb_statemachine_info_t *statemachine_info) {
    
    if (statemachine_info->openlcb_node->consumers.enumerator.running) {
        
        return false;
        
    }

     // Kick off enumeration if not already running
    if (!statemachine_info->openlcb_node->producers.enumerator.running) {

        statemachine_info->incoming_msg_info.enumerate = true; // Keep the enumeration going 
        statemachine_info->openlcb_node->producers.enumerator.running = true; // Kick off the enumeration next loop
        statemachine_info->openlcb_node->producers.enumerator.enum_index = 0;
        statemachine_info->openlcb_node->producers.enumerator.range_enum_index = 0;

    }

    // First handle ranges
    if (statemachine_info->openlcb_node->producers.enumerator.range_enum_index < statemachine_info->openlcb_node->producers.range_count) {

        OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_PRODUCER_RANGE_IDENTIFIED);

        event_id_t event_id = OpenLcbUtilities_generate_event_range_id(
            statemachine_info->openlcb_node->producers.range_list[statemachine_info->openlcb_node->producers.enumerator.range_enum_index].start_base,
            statemachine_info->openlcb_node->producers.range_list[statemachine_info->openlcb_node->producers.enumerator.range_enum_index].event_count);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            event_id);

        statemachine_info->openlcb_node->producers.enumerator.range_enum_index++;

        statemachine_info->outgoing_msg_info.enumerate = true;
        statemachine_info->outgoing_msg_info.valid = true;

        return true;
    }

    // Next handle individual events
    
    if (statemachine_info->openlcb_node->producers.enumerator.enum_index < statemachine_info->openlcb_node->producers.count) {
        
        OpenLcbUtilities_load_openlcb_message(
                                              statemachine_info->outgoing_msg_info.msg_ptr,
                                              statemachine_info->openlcb_node->alias,
                                              statemachine_info->openlcb_node->id,
                                              statemachine_info->incoming_msg_info.msg_ptr->source_alias,
                                              statemachine_info->incoming_msg_info.msg_ptr->source_id,
                                              ProtocolEventTransport_extract_producer_event_status_mti(statemachine_info->openlcb_node, statemachine_info->openlcb_node->producers.enumerator.enum_index));
        
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
                                                          statemachine_info->outgoing_msg_info.msg_ptr,
                                                          statemachine_info->openlcb_node->producers.list[statemachine_info->openlcb_node->producers.enumerator.enum_index].event);
        
        statemachine_info->openlcb_node->producers.enumerator.enum_index++;
        
        statemachine_info->outgoing_msg_info.enumerate = true;
        statemachine_info->outgoing_msg_info.valid = true;
        
        return true;
        
    }
    
    
    // Done rest and jump to consumers

    statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0; // Reset for next enumeration
    statemachine_info->openlcb_node->consumers.enumerator.range_enum_index = 0; // Reset for next enumeration
    statemachine_info->openlcb_node->consumers.enumerator.running = false;
    
    statemachine_info->outgoing_msg_info.enumerate = true;
    statemachine_info->outgoing_msg_info.valid = false;
    
    return false;
    
}

    // Precondition: enum_index must be < consumers.count (verified by caller)
    /**
    * @brief Identifies a consumer event and prepares response message
    *
    * @details Algorithm:
    * -# If enumeration not already running, mark it as started
    * -# Construct Consumer Identified message with appropriate MTI
    * -# Extract event status MTI based on current consumer event state
    * -# Copy consumer event ID to outgoing message payload
    * -# Increment enumeration index for next event
    * -# Mark outgoing message as valid for transmission
    *
    * Use cases:
    * - Called iteratively during event enumeration
    * - Responds to consumer identify requests
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Caller must ensure enum_index < consumers.count
    *
    * @note This is a helper function for ProtocolEventTransport_handle_events_identify
    * @note Payload count is set automatically by OpenLcbUtilities_copy_event_id_to_openlcb_payload
    *
    * @see ProtocolEventTransport_handle_events_identify - Main enumeration handler
    * @see ProtocolEventTransport_extract_consumer_event_status_mti - Determines response MTI
    */
static bool _identify_consumers(openlcb_statemachine_info_t *statemachine_info) {

    // Kick off enumeration if not already running  
    if (!statemachine_info->openlcb_node->consumers.enumerator.running) {

        statemachine_info->incoming_msg_info.enumerate = true; // Keep the enumeration going
        statemachine_info->openlcb_node->consumers.enumerator.running = true; // Kick off the enumeration next loop
        statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0;
        statemachine_info->openlcb_node->consumers.enumerator.range_enum_index = 0;

    }

    // First handle ranges
    if (statemachine_info->openlcb_node->consumers.enumerator.range_enum_index < statemachine_info->openlcb_node->consumers.range_count)
    {

        OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_CONSUMER_RANGE_IDENTIFIED);

        event_id_t event_id = OpenLcbUtilities_generate_event_range_id(
            statemachine_info->openlcb_node->consumers.range_list[statemachine_info->openlcb_node->consumers.enumerator.range_enum_index].start_base,
            statemachine_info->openlcb_node->consumers.range_list[statemachine_info->openlcb_node->consumers.enumerator.range_enum_index].event_count);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            event_id);

        statemachine_info->openlcb_node->consumers.enumerator.range_enum_index++;

        statemachine_info->outgoing_msg_info.enumerate = true;
        statemachine_info->outgoing_msg_info.valid = true;

        return true;
    }

    
    if (statemachine_info->openlcb_node->consumers.enumerator.enum_index < statemachine_info->openlcb_node->consumers.count) {
        
        OpenLcbUtilities_load_openlcb_message(
                                              statemachine_info->outgoing_msg_info.msg_ptr,
                                              statemachine_info->openlcb_node->alias,
                                              statemachine_info->openlcb_node->id,
                                              statemachine_info->incoming_msg_info.msg_ptr->source_alias,
                                              statemachine_info->incoming_msg_info.msg_ptr->source_id,
                                              ProtocolEventTransport_extract_consumer_event_status_mti(statemachine_info->openlcb_node, statemachine_info->openlcb_node->consumers.enumerator.enum_index));
        
        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
                                                          statemachine_info->outgoing_msg_info.msg_ptr,
                                                          statemachine_info->openlcb_node->consumers.list[statemachine_info->openlcb_node->consumers.enumerator.enum_index].event);
        
        statemachine_info->openlcb_node->consumers.enumerator.enum_index++;
        
        statemachine_info->outgoing_msg_info.enumerate = true;
        statemachine_info->outgoing_msg_info.valid = true;
        
        return true;
        
    }
    
    // Done rest and jump to consumers
    
    statemachine_info->openlcb_node->producers.enumerator.enum_index = 0; // Reset for next enumeration
    statemachine_info->openlcb_node->producers.enumerator.range_enum_index = 0; // Reset for next enumeration
    statemachine_info->openlcb_node->producers.enumerator.running = false;
    
    statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0; // Reset for next enumeration
    statemachine_info->openlcb_node->consumers.enumerator.range_enum_index = 0; // Reset for next enumeration
    statemachine_info->openlcb_node->consumers.enumerator.running = false;
    
    statemachine_info->incoming_msg_info.enumerate = false; // Stop the enumeration
    
    statemachine_info->outgoing_msg_info.valid = false;
    
    return false;

}

static void _test_for_consumed_event(openlcb_statemachine_info_t *statemachine_info, event_status_enum status, event_payload_t *payload)
{

    if (_interface->on_consumed_event_identified)
    {

        uint16_t event_index = 0xFFFF;
        
        event_id_t target_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        if (OpenLcbUtilities_is_event_id_in_consumer_ranges(statemachine_info->openlcb_node, target_event_id))
        {
            _interface->on_consumed_event_identified(statemachine_info->openlcb_node, -1, &target_event_id, status, payload);

            return;
        }

        if (OpenLcbUtilities_is_consumer_event_assigned_to_node(statemachine_info->openlcb_node, target_event_id, &event_index))
        {

            _interface->on_consumed_event_identified(statemachine_info->openlcb_node, event_index, &target_event_id, status, payload);

        }

    }

}

static void _test_for_consumed_event_pcer(openlcb_statemachine_info_t *statemachine_info, event_payload_t *payload)
{

    if (_interface->on_consumed_event_pcer)
    {

        uint16_t event_index = 0xFFFF;
        event_id_t target_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        if (OpenLcbUtilities_is_event_id_in_consumer_ranges(statemachine_info->openlcb_node, target_event_id)) {

            _interface->on_consumed_event_pcer(statemachine_info->openlcb_node, -1, &target_event_id, payload);

            return;
        }

        if (OpenLcbUtilities_is_consumer_event_assigned_to_node(statemachine_info->openlcb_node, target_event_id, &event_index))
        {

            _interface->on_consumed_event_pcer(statemachine_info->openlcb_node, event_index, &target_event_id, payload);

        }

    }

}

    /**
    * @brief Extracts the appropriate MTI for consumer event status
    *
    * @details Algorithm:
    * -# Access consumer event at specified index
    * -# Switch on event status field:
    *    - EVENT_STATUS_SET → return MTI_CONSUMER_IDENTIFIED_SET
    *    - EVENT_STATUS_CLEAR → return MTI_CONSUMER_IDENTIFIED_CLEAR
    *    - Default/unknown → return MTI_CONSUMER_IDENTIFIED_UNKNOWN
    *
    * Use cases:
    * - Responding to consumer identify requests
    * - Event enumeration responses
    *
    * @verbatim
    * @param openlcb_node Pointer to node containing consumer event list
    * @endverbatim
    * @verbatim
    * @param event_index Index into the node's consumer event list
    * @endverbatim
    *
    * @return MTI value corresponding to event state:
    *         - MTI_CONSUMER_IDENTIFIED_UNKNOWN for unknown state
    *         - MTI_CONSUMER_IDENTIFIED_SET for set state
    *         - MTI_CONSUMER_IDENTIFIED_CLEAR for clear state
    *
    * @warning Pointer must NOT be NULL
    * @warning Index must be valid (< consumer.count)
    *
    * @attention Caller must ensure event_index is within bounds
    *
    * @see ProtocolEventTransport_extract_producer_event_status_mti - Producer equivalent
    */
uint16_t ProtocolEventTransport_extract_consumer_event_status_mti(openlcb_node_t *openlcb_node, uint16_t event_index) {

    switch (openlcb_node->consumers.list[event_index].status) {

        case EVENT_STATUS_SET:

            return MTI_CONSUMER_IDENTIFIED_SET;

        case EVENT_STATUS_CLEAR:

            return MTI_CONSUMER_IDENTIFIED_CLEAR;

        default:

            return MTI_CONSUMER_IDENTIFIED_UNKNOWN;

    }

}

    /**
    * @brief Extracts the appropriate MTI for producer event status
    *
    * @details Algorithm:
    * -# Access producer event at specified index
    * -# Switch on event status field:
    *    - EVENT_STATUS_SET → return MTI_PRODUCER_IDENTIFIED_SET
    *    - EVENT_STATUS_CLEAR → return MTI_PRODUCER_IDENTIFIED_CLEAR
    *    - Default/unknown → return MTI_PRODUCER_IDENTIFIED_UNKNOWN
    *
    * Use cases:
    * - Responding to producer identify requests
    * - Event enumeration responses
    *
    * @verbatim
    * @param openlcb_node Pointer to node containing producer event list
    * @endverbatim
    * @verbatim
    * @param event_index Index into the node's producer event list
    * @endverbatim
    *
    * @return MTI value corresponding to event state:
    *         - MTI_PRODUCER_IDENTIFIED_UNKNOWN for unknown state
    *         - MTI_PRODUCER_IDENTIFIED_SET for set state
    *         - MTI_PRODUCER_IDENTIFIED_CLEAR for clear state
    *
    * @warning Pointer must NOT be NULL
    * @warning Index must be valid (< producer.count)
    *
    * @attention Caller must ensure event_index is within bounds
    *
    * @see ProtocolEventTransport_extract_consumer_event_status_mti - Consumer equivalent
    */
uint16_t ProtocolEventTransport_extract_producer_event_status_mti(openlcb_node_t *openlcb_node, uint16_t event_index) {

    switch (openlcb_node->producers.list[event_index].status) {

        case EVENT_STATUS_SET:

            return MTI_PRODUCER_IDENTIFIED_SET;

        case EVENT_STATUS_CLEAR:

            return MTI_PRODUCER_IDENTIFIED_CLEAR;

        default:

            return MTI_PRODUCER_IDENTIFIED_UNKNOWN;

    }

}

    /**
    * @brief Handles Consumer Identify message
    *
    * @details Algorithm:
    * -# Extract target event ID from incoming message payload
    * -# Search node's consumer list for matching event
    * -# If event not found:
    *    - Mark outgoing message as invalid (no response)
    *    - Return early
    * -# If event found:
    *    - Construct Consumer Identified response message
    *    - Set MTI based on current consumer event state
    *    - Copy event ID to response payload
    *    - Mark outgoing message as valid
    *
    * Use cases:
    * - Remote node querying if this node consumes an event
    * - Configuration tools discovering event consumers
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Incoming message must have valid event ID payload
    *
    * @note If event not consumed by this node, no response is generated
    * @note Response MTI depends on current event state (unknown/set/clear)
    * @note Payload count is set automatically by OpenLcbUtilities_copy_event_id_to_openlcb_payload
    *
    * @see ProtocolEventTransport_extract_consumer_event_status_mti - Determines response MTI
    */
void ProtocolEventTransport_handle_consumer_identify(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t event_index = 0;
    event_id_t target_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

    if (!OpenLcbUtilities_is_consumer_event_assigned_to_node(statemachine_info->openlcb_node, target_event_id, &event_index) &&
          !OpenLcbUtilities_is_event_id_in_consumer_ranges(statemachine_info->openlcb_node, target_event_id)) {

        statemachine_info->outgoing_msg_info.valid = false;

        return; //  done

    }

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            ProtocolEventTransport_extract_consumer_event_status_mti(statemachine_info->openlcb_node, event_index));

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->consumers.list[event_index].event);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Handles Consumer Range Identified message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Discovering event consumers on the network
    * - Building event routing tables
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Callback receives the base event ID of the range
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_consumer_range_identified(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_range_identified) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_range_identified(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Consumer Identified Unknown message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Tracking consumer states on the network
    * - Updating event state displays
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_identified_unknown) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_identified_unknown(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Consumer Identified Set message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Tracking consumer states on the network
    * - Synchronizing event states across nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_consumer_identified_set(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_identified_set) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_identified_set(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Consumer Identified Clear message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Tracking consumer states on the network
    * - Synchronizing event states across nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_consumer_identified_clear(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_identified_clear) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_identified_clear(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Consumer Identified Reserved message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Handling future protocol extensions
    * - Logging unusual event states
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Reserved states are defined by future OpenLCB specifications
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_identified_reserved) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_identified_reserved(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Producer Identify message
    *
    * @details Algorithm:
    * -# Extract target event ID from incoming message payload
    * -# Search node's producer list for matching event
    * -# If event not found:
    *    - Mark outgoing message as invalid (no response)
    *    - Return early
    * -# If event found:
    *    - Construct Producer Identified response message
    *    - Set MTI based on current producer event state
    *    - Copy event ID to response payload
    *    - Mark outgoing message as valid
    *
    * Use cases:
    * - Remote node querying if this node produces an event
    * - Configuration tools discovering event producers
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Incoming message must have valid event ID payload
    *
    * @note If event not produced by this node, no response is generated
    * @note Response MTI depends on current event state (unknown/set/clear)
    * @note Payload count is set automatically by OpenLcbUtilities_copy_event_id_to_openlcb_payload
    *
    * @see ProtocolEventTransport_extract_producer_event_status_mti - Determines response MTI
    */
void ProtocolEventTransport_handle_producer_identify(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t event_index = 0;
    event_id_t target_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

    if (!OpenLcbUtilities_is_producer_event_assigned_to_node(statemachine_info->openlcb_node, target_event_id, &event_index) &&
        !OpenLcbUtilities_is_event_id_in_producer_ranges(statemachine_info->openlcb_node, target_event_id))
    {

        statemachine_info->outgoing_msg_info.valid = false;

        return;
    }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            ProtocolEventTransport_extract_producer_event_status_mti(statemachine_info->openlcb_node, event_index));

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->producers.list[event_index].event);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Handles Producer Range Identified message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Discovering event producers on the network
    * - Building event routing tables
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Callback receives the base event ID of the range
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_producer_range_identified(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_producer_range_identified) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_range_identified(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Producer Identified Unknown message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Tracking producer states on the network
    * - Updating event state displays
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_producer_identified_unknown(openlcb_statemachine_info_t *statemachine_info) {

    _test_for_consumed_event(statemachine_info, EVENT_STATUS_UNKNOWN, NULL);

    if (_interface->on_producer_identified_unknown)
    {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_identified_unknown(statemachine_info->openlcb_node, &eventid);
    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Producer Identified Set message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Tracking producer states on the network
    * - Synchronizing event states across nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_producer_identified_set(openlcb_statemachine_info_t *statemachine_info) {

    _test_for_consumed_event(statemachine_info, EVENT_STATUS_SET, NULL);

    if (_interface->on_producer_identified_set) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_identified_set(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Producer Identified Clear message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Tracking producer states on the network
    * - Synchronizing event states across nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_producer_identified_clear(openlcb_statemachine_info_t *statemachine_info) {

    _test_for_consumed_event(statemachine_info, EVENT_STATUS_CLEAR, NULL);

    if (_interface->on_producer_identified_clear) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_identified_clear(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Producer Identified Reserved message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Handling future protocol extensions
    * - Logging unusual event states
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Reserved states are defined by future OpenLCB specifications
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_producer_identified_reserved(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_producer_identified_reserved) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_identified_reserved(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles global Identify Events message
    *
    * @details Algorithm:
    * -# Check if more producer events need enumeration (enum_index < count):
    *    - If yes: Call _identify_producers() to handle next producer event
    *    - Return to caller for transmission
    * -# If all producers enumerated:
    *    - Mark producer enumeration as complete
    * -# Check if more consumer events need enumeration (enum_index < count):
    *    - If yes: Call _identify_consumers() to handle next consumer event
    *    - Return to caller for transmission
    * -# If all consumers enumerated:
    *    - Reset both enumeration indices to 0
    *    - Mark consumer enumeration as complete
    *    - Clear enumeration flag
    *
    * Use cases:
    * - Network-wide event discovery
    * - Configuration tools building complete event maps
    * - Node initialization and announcement
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Enumerates all producer events first, then all consumer events
    * @note Uses enumeration state machine to handle multiple responses
    * @note Responses are generated incrementally across multiple calls
    * @note Caller must continue calling until enumerate flag becomes false
    *
    * @see ProtocolEventTransport_extract_producer_event_status_mti - Get producer response MTI
    * @see ProtocolEventTransport_extract_consumer_event_status_mti - Get consumer response MTI
    */
void ProtocolEventTransport_handle_events_identify(openlcb_statemachine_info_t *statemachine_info) {

    if (_identify_producers(statemachine_info)) {

        return;

    }
    
    _identify_consumers(statemachine_info);
   
}

    /**
    * @brief Handles Identify Events message with destination addressing
    *
    * @details Algorithm:
    * -# Check if incoming message is addressed to this node
    * -# If addressed to this node:
    *    - Call ProtocolEventTransport_handle_events_identify() for processing
    *    - Return to caller
    * -# If not addressed to this node:
    *    - Mark outgoing message as invalid (no response)
    *
    * Use cases:
    * - Configuration tools requesting complete event list from specific node
    * - Targeted event discovery without network-wide broadcast
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Checks message destination before processing
    * @note If not addressed to this node, no response is generated
    *
    * @see ProtocolEventTransport_handle_events_identify - Actual event identification logic
    */
void ProtocolEventTransport_handle_events_identify_dest(openlcb_statemachine_info_t *statemachine_info) {


    if (OpenLcbUtilities_is_addressed_message_for_node(statemachine_info->openlcb_node, statemachine_info->incoming_msg_info.msg_ptr)) {

        ProtocolEventTransport_handle_events_identify(statemachine_info);

        return;

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Event Learn message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Teaching this node a new event to produce or consume
    * - Configuration mode for event learning
    * - Dynamic event configuration
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Callback must implement actual event learning logic
    *
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_event_learn(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_event_learn) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_event_learn(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Producer/Consumer Event Report message
    *
    * @details Algorithm:
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from incoming message payload
    *    - Invoke callback with node context and event ID
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Receiving event notifications from producers
    * - Triggering consumer actions in response to events
    * - Event logging and monitoring
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Callback is responsible for consuming the event and taking action
    *
    * @see ProtocolEventTransport_handle_pc_event_report_with_payload - Event report with additional data
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_pc_event_report(openlcb_statemachine_info_t *statemachine_info) {

    _test_for_consumed_event_pcer(statemachine_info, NULL);

    if (_interface->on_pc_event_report) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_pc_event_report(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles Producer/Consumer Event Report message with payload
    *
    * @details Algorithm:
    * -# Check if payload count is greater than event ID size (8 bytes)
    * -# If payload too small:
    *    - Mark outgoing message as invalid (no response)
    *    - Return early (malformed message)
    * -# Check if callback is registered (!= NULL)
    * -# If callback registered:
    *    - Extract event ID from first 8 bytes of payload
    *    - Calculate payload data count (total - 8 bytes)
    *    - Get pointer to payload data (after event ID)
    *    - Invoke callback with node, event ID, count, and payload pointer
    * -# Mark outgoing message as invalid (no automatic response)
    *
    * Use cases:
    * - Receiving event reports with sensor data
    * - Events carrying configuration or state information
    * - Extended event notifications with context
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must NOT be NULL
    * @warning Payload must be at least sizeof(event_id_t) + 1 bytes
    *
    * @note Always sets outgoing_msg_info.valid to false (no automatic response)
    * @note Callback receives payload data pointer and byte count
    * @note Payload count excludes the event ID (8 bytes)
    *
    * @see ProtocolEventTransport_handle_pc_event_report - Event report without payload
    * @see interface_openlcb_protocol_event_transport_t - Callback registration
    */
void ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_statemachine_info_t *statemachine_info) {

    if (statemachine_info->incoming_msg_info.msg_ptr->payload_count <= sizeof (event_id_t)) {

        statemachine_info->outgoing_msg_info.valid = false;

        return;

    }

    event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);
    uint16_t payload_count = (statemachine_info->incoming_msg_info.msg_ptr->payload_count - sizeof(event_id_t));

    _test_for_consumed_event_pcer(statemachine_info, (event_payload_t *) statemachine_info->incoming_msg_info.msg_ptr->payload[sizeof (event_id_t)]);

    if (_interface->on_pc_event_report_with_payload) {

        _interface->on_pc_event_report_with_payload(statemachine_info->openlcb_node, &eventid, payload_count, (event_payload_t *) statemachine_info->incoming_msg_info.msg_ptr->payload[sizeof (event_id_t)]);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}
