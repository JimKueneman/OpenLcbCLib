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
* @file protocol_event_transport.h
* @brief Event transport protocol implementation
* @author Jim Kueneman
* @date 17 Jan 2026
*/

/*
 * NOTE:  All Function for all Protocols expect that the incoming CAN messages have been
 *        blocked so there is not a race on the incoming message buffer.
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_EVENT_TRANSPORT__
#define __OPENLCB_PROTOCOL_EVENT_TRANSPORT__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
    * @brief Interface structure for Event Transport protocol callbacks
    *
    * @details This structure defines the callback interface for handling OpenLCB
    * Event Transport protocol notifications. It contains function pointers for
    * receiving notifications about producer and consumer events on the OpenLCB network
    * as defined in the OpenLCB Event Transport Protocol specification.
    *
    * The interface allows the application layer to receive notifications about:
    * - Consumer event identifications and state changes
    * - Producer event identifications and state changes
    * - Event learn/teach operations
    * - Producer/Consumer event reports (with or without payload)
    *
    * All callback functions are optional and may be set to NULL if the application
    * does not need to handle that particular event type. The protocol layer will
    * check for NULL before invoking any callback.
    *
    * @note All callbacks are optional - can be NULL if notification is not needed
    * @note Callbacks receive the node context and event ID that triggered the notification
    * @note Callbacks are invoked from within the protocol handler - keep processing brief
    * @note Callbacks must not block or perform lengthy operations
    *
    * @see ProtocolEventTransport_initialize - Registers this interface with the protocol
    * @see openlcb_node_t - Node context structure
    * @see event_id_t - Event identifier type
    */
typedef struct
{

        /**
         * @brief Optional callback for Consumer Range Identified notification
         *
         * @details Invoked when a remote node on the network identifies itself as consuming
         * a range of events. The event_id parameter contains the base event ID of the range.
         * Applications can use this to build event routing tables or discover consumers.
         *
         * @note Optional - can be NULL if Consumer Range Identified notifications are not needed
         */
    void (*on_consumer_range_identified)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Consumer Identified Unknown notification
         *
         * @details Invoked when a remote node identifies itself as consuming an event
         * but reports the current state as unknown. This typically occurs during node
         * initialization or when state cannot be determined.
         *
         * @note Optional - can be NULL if Consumer Identified Unknown notifications are not needed
         */
    void (*on_consumer_identified_unknown)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Consumer Identified Set notification
         *
         * @details Invoked when a remote node identifies itself as consuming an event
         * and reports the current state as SET (active/true). Applications can use this
         * to track consumer states and update displays or control logic.
         *
         * @note Optional - can be NULL if Consumer Identified Set notifications are not needed
         */
    void (*on_consumer_identified_set)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Consumer Identified Clear notification
         *
         * @details Invoked when a remote node identifies itself as consuming an event
         * and reports the current state as CLEAR (inactive/false). Applications can use
         * this to track consumer states and update displays or control logic.
         *
         * @note Optional - can be NULL if Consumer Identified Clear notifications are not needed
         */
    void (*on_consumer_identified_clear)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Consumer Identified Reserved notification
         *
         * @details Invoked when a remote node identifies itself as consuming an event
         * with a reserved state indicator. This is for future protocol extensions and
         * special event types.
         *
         * @note Optional - can be NULL if Consumer Identified Reserved notifications are not needed
         */
    void (*on_consumer_identified_reserved)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Producer Range Identified notification
         *
         * @details Invoked when a remote node on the network identifies itself as producing
         * a range of events. The event_id parameter contains the base event ID of the range.
         * Applications can use this to build event routing tables or discover producers.
         *
         * @note Optional - can be NULL if Producer Range Identified notifications are not needed
         */
    void (*on_producer_range_identified)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Producer Identified Unknown notification
         *
         * @details Invoked when a remote node identifies itself as producing an event
         * but reports the current state as unknown. This typically occurs during node
         * initialization or when state cannot be determined.
         *
         * @note Optional - can be NULL if Producer Identified Unknown notifications are not needed
         */
    void (*on_producer_identified_unknown)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Producer Identified Set notification
         *
         * @details Invoked when a remote node identifies itself as producing an event
         * and reports the current state as SET (active/true). Applications can use this
         * to track producer states and update displays or anticipate future events.
         *
         * @note Optional - can be NULL if Producer Identified Set notifications are not needed
         */
    void (*on_producer_identified_set)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Producer Identified Clear notification
         *
         * @details Invoked when a remote node identifies itself as producing an event
         * and reports the current state as CLEAR (inactive/false). Applications can use
         * this to track producer states and update displays or anticipate future events.
         *
         * @note Optional - can be NULL if Producer Identified Clear notifications are not needed
         */
    void (*on_producer_identified_clear)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Producer Identified Reserved notification
         *
         * @details Invoked when a remote node identifies itself as producing an event
         * with a reserved state indicator. This is for future protocol extensions and
         * special event types.
         *
         * @note Optional - can be NULL if Producer Identified Reserved notifications are not needed
         */
    void (*on_producer_identified_reserved)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Event Learn notification
         *
         * @details Invoked when a teach/learn message is received for event configuration.
         * The application can implement logic to learn a new event ID from a producer or
         * consumer, typically used during configuration mode or blue/gold button programming.
         *
         * The callback must implement the actual event learning logic, such as storing the
         * event ID in the node's producer or consumer list and saving to non-volatile memory.
         *
         * @note Optional - can be NULL if Event Learn operations are not supported
         */
    void (*on_event_learn)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Producer/Consumer Event Report notification
         *
         * @details Invoked when an event report is received indicating that an event has
         * occurred on the network. This is the fundamental event notification mechanism in
         * OpenLCB. The callback is responsible for consuming the event and taking appropriate
         * action based on the application's requirements.
         *
         * Common actions include:
         * - Checking if this node consumes the reported event
         * - Triggering outputs, relays, or other hardware
         * - Updating display state or logging the event
         * - Forwarding the event to other systems
         *
         * @note Optional - can be NULL if Event Report notifications are not needed
         */
    void (*on_pc_event_report)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /**
         * @brief Optional callback for Producer/Consumer Event Report with payload notification
         *
         * @details Invoked when an event report with additional payload data is received.
         * The payload can contain sensor readings, state information, or other data associated
         * with the event. The count parameter indicates the number of payload bytes (excluding
         * the 8-byte event ID), and the payload parameter points to the payload data.
         *
         * Common payload uses include:
         * - Sensor readings (temperature, voltage, current)
         * - Multi-state event information
         * - Configuration or context data
         * - Extended event notifications
         *
         * @note Optional - can be NULL if Event Report with payload notifications are not needed
         */
    void (*on_pc_event_report_with_payload)(openlcb_node_t *openlcb_node, event_id_t *event_id, uint16_t count, event_payload_t *payload);

} interface_openlcb_protocol_event_transport_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
        * @brief Initializes the Event Transport protocol layer
        *
        * @details Registers the application's callback interface with the Event Transport
        * protocol handler. Must be called once during system initialization before any
        * event-related messages are processed.
        *
        * Use cases:
        * - Called during application startup
        * - Required before processing any event messages
        *
        * @param interface_openlcb_protocol_event_transport Pointer to callback interface structure
        *
        * @warning interface_openlcb_protocol_event_transport must remain valid for lifetime of application
        * @warning NOT thread-safe - call during single-threaded initialization only
        *
        * @attention Call before enabling CAN message reception
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback interface structure
        */
    extern void ProtocolEventTransport_initialize(const interface_openlcb_protocol_event_transport_t *interface_openlcb_protocol_event_transport);

        /**
        * @brief Handles Consumer Identify message
        *
        * @details Processes a request to identify if this node consumes a specific event.
        * Checks if the requested event is in the node's consumer list and responds with
        * the current state (unknown/set/clear).
        *
        * Use cases:
        * - Remote node querying if this node consumes an event
        * - Configuration tools discovering event consumers
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        * @warning Incoming message must have valid event ID payload
        *
        * @note If event not consumed by this node, no response is generated
        * @note Response MTI depends on current event state (unknown/set/clear)
        *
        * @see ProtocolEventTransport_extract_consumer_event_status_mti - Determines response MTI
        */
    extern void ProtocolEventTransport_handle_consumer_identify(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Consumer Range Identified message
        *
        * @details Processes notification that a remote node has identified itself as consuming
        * a range of events. Invokes the registered callback if configured.
        *
        * Use cases:
        * - Discovering event consumers on the network
        * - Building event routing tables
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Callback receives the base event ID of the range
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_consumer_range_identified(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Consumer Identified Unknown message
        *
        * @details Processes notification that a remote node consumes an event but its current
        * state is unknown. Invokes the registered callback if configured.
        *
        * Use cases:
        * - Tracking consumer states on the network
        * - Updating event state displays
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Consumer Identified Set message
        *
        * @details Processes notification that a remote node consumes an event and its current
        * state is SET (active/true). Invokes the registered callback if configured.
        *
        * Use cases:
        * - Tracking consumer states on the network
        * - Updating event state displays
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_consumer_identified_set(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Consumer Identified Clear message
        *
        * @details Processes notification that a remote node consumes an event and its current
        * state is CLEAR (inactive/false). Invokes the registered callback if configured.
        *
        * Use cases:
        * - Tracking consumer states on the network
        * - Updating event state displays
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_consumer_identified_clear(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Consumer Identified Reserved message
        *
        * @details Processes notification that a remote node consumes an event with a reserved
        * state indicator. Invokes the registered callback if configured.
        *
        * Use cases:
        * - Supporting future protocol extensions
        * - Handling special event types
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Producer Identify message
        *
        * @details Processes a request to identify if this node produces a specific event.
        * Checks if the requested event is in the node's producer list and responds with
        * the current state (unknown/set/clear).
        *
        * Use cases:
        * - Remote node querying if this node produces an event
        * - Configuration tools discovering event producers
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        * @warning Incoming message must have valid event ID payload
        *
        * @note If event not produced by this node, no response is generated
        * @note Response MTI depends on current event state (unknown/set/clear)
        *
        * @see ProtocolEventTransport_extract_producer_event_status_mti - Determines response MTI
        */
    extern void ProtocolEventTransport_handle_producer_identify(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Producer Range Identified message
        *
        * @details Processes notification that a remote node has identified itself as producing
        * a range of events. Invokes the registered callback if configured.
        *
        * Use cases:
        * - Discovering event producers on the network
        * - Building event routing tables
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Callback receives the base event ID of the range
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_producer_range_identified(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Producer Identified Unknown message
        *
        * @details Processes notification that a remote node produces an event but its current
        * state is unknown. Invokes the registered callback if configured.
        *
        * Use cases:
        * - Tracking producer states on the network
        * - Updating event state displays
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_producer_identified_unknown(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Producer Identified Set message
        *
        * @details Processes notification that a remote node produces an event and its current
        * state is SET (active/true). Invokes the registered callback if configured.
        *
        * Use cases:
        * - Tracking producer states on the network
        * - Updating event state displays
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_producer_identified_set(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Producer Identified Clear message
        *
        * @details Processes notification that a remote node produces an event and its current
        * state is CLEAR (inactive/false). Invokes the registered callback if configured.
        *
        * Use cases:
        * - Tracking producer states on the network
        * - Updating event state displays
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_producer_identified_clear(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Producer Identified Reserved message
        *
        * @details Processes notification that a remote node produces an event with a reserved
        * state indicator. Invokes the registered callback if configured.
        *
        * Use cases:
        * - Supporting future protocol extensions
        * - Handling special event types
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_producer_identified_reserved(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles addressed Identify Events message
        *
        * @details Processes an addressed request to identify all events this node produces and
        * consumes. Verifies the message is addressed to this node before delegating to the
        * actual event identification logic.
        *
        * Use cases:
        * - Targeted event discovery for a specific node
        * - Configuration tools querying this node's events
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Checks message destination before processing
        * @note If not addressed to this node, no response is generated
        *
        * @see ProtocolEventTransport_handle_events_identify - Actual event identification logic
        */
    extern void ProtocolEventTransport_handle_events_identify_dest(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles global Identify Events message
        *
        * @details Processes a global request to identify all events this node produces and
        * consumes. Responds with Producer/Consumer Identified messages for each event in
        * the node's event lists.
        *
        * Use cases:
        * - Network-wide event discovery
        * - Configuration tools building complete event maps
        * - Node initialization and announcement
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Enumerates all producer events first, then all consumer events
        * @note Uses enumeration state machine to handle multiple responses
        * @note Responses are generated incrementally across multiple calls
        *
        * @see ProtocolEventTransport_extract_producer_event_status_mti - Get producer response MTI
        * @see ProtocolEventTransport_extract_consumer_event_status_mti - Get consumer response MTI
        */
    extern void ProtocolEventTransport_handle_events_identify(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Event Learn message
        *
        * @details Processes a teach/learn message for event configuration. The node can use
        * this to learn a new event ID from a producer or consumer. Invokes the registered
        * callback if configured.
        *
        * Use cases:
        * - Teaching this node a new event to produce or consume
        * - Configuration mode for event learning
        * - Dynamic event configuration
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Callback must implement actual event learning logic
        *
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_event_learn(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Producer/Consumer Event Report message
        *
        * @details Processes an event report indicating that an event has occurred on the network.
        * This is the fundamental event notification mechanism in OpenLCB. Invokes the registered
        * callback if configured.
        *
        * Use cases:
        * - Receiving event notifications from producers
        * - Triggering consumer actions in response to events
        * - Event logging and monitoring
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Callback is responsible for consuming the event and taking action
        *
        * @see ProtocolEventTransport_handle_pc_event_report_with_payload - Event report with additional data
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_pc_event_report(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles Producer/Consumer Event Report message with payload
        *
        * @details Processes an event report that includes additional payload data beyond the
        * event ID. The payload can contain sensor readings, state information, or other data
        * associated with the event. Invokes the registered callback if configured.
        *
        * Use cases:
        * - Receiving event reports with sensor data
        * - Events carrying configuration or state information
        * - Extended event notifications with context
        *
        * @param statemachine_info Pointer to state machine context containing incoming message
        *
        * @warning statemachine_info must NOT be NULL
        * @warning Payload must be at least sizeof(event_id_t) + 1 bytes
        *
        * @note Always sets outgoing_msg_info.valid to false (no automatic response)
        * @note Callback receives payload data pointer and byte count
        * @note Payload count excludes the event ID (8 bytes)
        *
        * @see ProtocolEventTransport_handle_pc_event_report - Event report without payload
        * @see interface_openlcb_protocol_event_transport_t - Callback registration
        */
    extern void ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Extracts the appropriate MTI for consumer event status
        *
        * @details Determines the correct Message Type Indicator (MTI) to use when responding
        * to a consumer identification request, based on the current state of the consumer
        * event (unknown/set/clear).
        *
        * Use cases:
        * - Responding to consumer identify requests
        * - Event enumeration responses
        *
        * @param openlcb_node Pointer to node containing consumer event list
        * @param event_index Index into the node's consumer event list
        *
        * @return MTI value corresponding to event state:
        *         - MTI_CONSUMER_IDENTIFIED_UNKNOWN for unknown state
        *         - MTI_CONSUMER_IDENTIFIED_SET for set state
        *         - MTI_CONSUMER_IDENTIFIED_CLEAR for clear state
        *
        * @warning openlcb_node must NOT be NULL
        * @warning event_index must be valid (< consumer.count)
        *
        * @attention Caller must ensure event_index is within bounds
        *
        * @see ProtocolEventTransport_extract_producer_event_status_mti - Producer equivalent
        */
    extern uint16_t ProtocolEventTransport_extract_consumer_event_status_mti(openlcb_node_t *openlcb_node, uint16_t event_index);

        /**
        * @brief Extracts the appropriate MTI for producer event status
        *
        * @details Determines the correct Message Type Indicator (MTI) to use when responding
        * to a producer identification request, based on the current state of the producer
        * event (unknown/set/clear).
        *
        * Use cases:
        * - Responding to producer identify requests
        * - Event enumeration responses
        *
        * @param openlcb_node Pointer to node containing producer event list
        * @param event_index Index into the node's producer event list
        *
        * @return MTI value corresponding to event state:
        *         - MTI_PRODUCER_IDENTIFIED_UNKNOWN for unknown state
        *         - MTI_PRODUCER_IDENTIFIED_SET for set state
        *         - MTI_PRODUCER_IDENTIFIED_CLEAR for clear state
        *
        * @warning openlcb_node must NOT be NULL
        * @warning event_index must be valid (< producer.count)
        *
        * @attention Caller must ensure event_index is within bounds
        *
        * @see ProtocolEventTransport_extract_consumer_event_status_mti - Consumer equivalent
        */
    extern uint16_t ProtocolEventTransport_extract_producer_event_status_mti(openlcb_node_t *openlcb_node, uint16_t event_index);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_PROTOCOL_EVENT_TRANSPORT__ */
