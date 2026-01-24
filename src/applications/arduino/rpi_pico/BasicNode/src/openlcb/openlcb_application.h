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
 * @file openlcb_application.h
 * @brief Application layer interface for OpenLCB library
 *
 * @details This header defines the application programming interface (API) for
 * interacting with the OpenLCB protocol stack. It provides high-level functions
 * for event registration, event transmission, and configuration memory access.
 *
 * The application layer sits above the protocol handlers and provides:
 * - Producer/Consumer event registration and management
 * - Event transmission (PC Event Report, Learn Event, Initialization Complete)
 * - Configuration memory read/write abstraction
 * - Application callback interface for message transmission and memory operations
 *
 * This is the primary interface used by application code to interact with the
 * OpenLCB library without needing to understand lower-level protocol details.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 *
 * @see openlcb_application.c
 * @see protocol_event_transport.c - Event protocol implementation
 * @see protocol_config_mem.c - Configuration memory protocol
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_APPLICATION__
#define __OPENLCB_OPENLCB_APPLICATION__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Interface structure for application layer callbacks
     *
     * @details This structure contains function pointers that the application must
     * provide to the OpenLCB library. These callbacks allow the library to:
     * - Send OpenLCB messages to the network
     * - Read from configuration memory
     * - Write to configuration memory
     *
     * All function pointers MUST be assigned valid function addresses before calling
     * OpenLcbApplication_initialize(). NULL function pointers will cause crashes when
     * the library attempts to use them.
     *
     * @warning All function pointers must be non-NULL before initialization
     * @warning The structure must remain valid for the lifetime of the program
     *
     * @see OpenLcbApplication_initialize
     */
typedef struct {

        /**
         * @brief Callback to send an OpenLCB message to the network
         *
         * @details This callback is invoked by the OpenLCB library whenever it needs to
         * transmit a message to the network. The application must implement this function
         * to queue the message for transmission via the appropriate transport layer (CAN,
         * TCP/IP, etc.). The callback should return immediately after queueing.
         *
         * @param openlcb_msg Pointer to the message to send (contains MTI, source/dest, payload)
         *
         * @return true if message was successfully queued for transmission,
         *         false if message buffer is full (library will retry or report error)
         *
         * @warning Must be non-NULL. Library will crash if this callback is not provided.
         *
         * @attention Callback should be non-blocking. Do not perform lengthy operations.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    bool (*send_openlcb_msg)(openlcb_msg_t *openlcb_msg);

        /**
         * @brief Callback to read from configuration memory
         *
         * @details This callback is invoked when the OpenLCB library needs to read data from
         * the node's configuration memory. Configuration memory contains persistent settings,
         * CDI (Configuration Description Information), ACDI (Abbreviated CDI), SNIP (Simple
         * Node Information Protocol) data, and user-configurable parameters.
         *
         * @param openlcb_node Pointer to the node requesting the read operation
         * @param address Starting address to read from (address space defined by CDI)
         * @param count Number of bytes to read (typically ≤64 for network operations)
         * @param buffer Pointer to buffer to store the read data (must have space for count bytes)
         *
         * @return Number of bytes successfully read (may be less than count if near end of space),
         *         or 0xFFFF on error (invalid address, hardware error)
         *
         * @warning Must be non-NULL. Library will crash if this callback is not provided.
         * @warning Buffer must have sufficient space for the requested byte count.
         *
         * @attention Some memory spaces are read-only (CDI, ACDI). Attempting to write
         *            to these spaces should return error.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    uint16_t (*config_memory_read)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

        /**
         * @brief Callback to write to configuration memory
         *
         * @details This callback is invoked when the OpenLCB library needs to write data to
         * the node's configuration memory. Configuration memory contains persistent settings
         * and user-configurable parameters. Some memory spaces (CDI, ACDI, manufacturer data)
         * are read-only and should reject write attempts.
         *
         * @param openlcb_node Pointer to the node requesting the write operation
         * @param address Starting address to write to (address space defined by CDI)
         * @param count Number of bytes to write (typically ≤64 for network operations)
         * @param buffer Pointer to buffer containing data to write
         *
         * @return Number of bytes successfully written (may be less than count if near end of space),
         *         or 0xFFFF on error (invalid address, read-only space, hardware error)
         *
         * @warning Must be non-NULL. Library will crash if this callback is not provided.
         * @warning Buffer must contain sufficient valid data for the requested operation.
         *
         * @attention Read-only memory spaces (CDI, ACDI, etc.) should return 0 or 0xFFFF error.
         *
         * @note Callback is synchronous and should complete quickly.
         * @note Application is responsible for persistence mechanism (EEPROM, Flash, etc.)
         * @note This is a REQUIRED callback - must not be NULL
         */
    uint16_t (*config_memory_write)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

} interface_openlcb_application_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Initializes the OpenLCB Application interface
         *
         * @details Stores the interface function pointers for use by all application
         * layer functions. This allows the library to call back into application code
         * when needed for message transmission and configuration memory operations.
         *
         * Use cases:
         * - Called once during application startup before any OpenLCB operations
         * - Must be called after buffer stores are initialized
         *
         * @param interface_openlcb_application Pointer to interface structure with callbacks
         *
         * @warning MUST be called exactly once during application startup before any
         *          other OpenLCB application functions. Calling multiple times will
         *          reset all state and cause loss of program context.
         *
         * @warning The interface pointer is stored in static memory. The pointed-to
         *          structure must remain valid for the lifetime of the program.
         *
         * @warning All function pointers in the interface must be non-NULL.
         *          NULL pointers will cause crashes when library attempts callbacks.
         *
         * @attention Must be called during single-threaded initialization phase.
         *            Do NOT call from interrupt context or after main loop starts.
         *
         * @attention Call after buffer stores are initialized but before allocating nodes.
         *
         * @note This is a one-time initialization. There is no corresponding cleanup function.
         *
         * @see interface_openlcb_application_t
         */
    extern void OpenLcbApplication_initialize(const interface_openlcb_application_t *interface_openlcb_application);

        /**
         * @brief Clears all consumer event IDs registered for the specified node
         *
         * @details Resets the consumer event list to empty state. This is typically used
         * during reconfiguration, factory reset, or dynamic event list rebuilding. The event
         * IDs remain in memory but become inaccessible due to the count being set to zero.
         *
         * Use cases:
         * - Reconfiguration of node event subscriptions
         * - Factory reset operations
         * - Dynamic event list rebuilding before re-registration
         *
         * @param openlcb_node Pointer to the OpenLCB node
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *          Passing NULL pointer will cause immediate crash.
         *
         * @attention Does not free any resources, only resets the count.
         *            Event IDs remain in memory but are inaccessible.
         *
         * @note Non-destructive operation - original event IDs remain in array.
         *
         * @see OpenLcbApplication_register_consumer_eventid
         * @see OpenLcbApplication_clear_producer_eventids
         */
    extern void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t *openlcb_node);

        /**
         * @brief Clears all producer event IDs registered for the specified node
         *
         * @details Resets the producer event list to empty state. This is typically used
         * during reconfiguration, factory reset, or dynamic event list rebuilding. The event
         * IDs remain in memory but become inaccessible due to the count being set to zero.
         *
         * Use cases:
         * - Reconfiguration of node event subscriptions
         * - Factory reset operations
         * - Dynamic event list rebuilding before re-registration
         *
         * @param openlcb_node Pointer to the OpenLCB node
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *          Passing NULL pointer will cause immediate crash.
         *
         * @attention Does not free any resources, only resets the count.
         *            Event IDs remain in memory but are inaccessible.
         *
         * @note Non-destructive operation - original event IDs remain in array.
         *
         * @see OpenLcbApplication_register_producer_eventid
         * @see OpenLcbApplication_clear_consumer_eventids
         */
    extern void OpenLcbApplication_clear_producer_eventids(openlcb_node_t *openlcb_node);

        /**
         * @brief Registers a consumer event ID with the specified node
         *
         * @details Adds a new consumer event to the node's consumer list if space is
         * available. Consumer events are those the node will respond to when received
         * from other nodes on the network. The node will send Consumer Identified messages
         * during event enumeration per OpenLCB Event Transport Protocol.
         *
         * Use cases:
         * - Registering events the node should respond to (e.g., turnout commands)
         * - Building initial event subscription list at startup
         * - Dynamic event registration during runtime configuration
         *
         * @param openlcb_node Pointer to the OpenLCB node
         * @param event_id 64-bit Event ID to register (8 bytes MSB first per OpenLCB spec)
         * @param event_status Initial status: EVENT_UNKNOWN, EVENT_VALID, or EVENT_INVALID
         *
         * @return 0-based array index where event was stored (0 to USER_DEFINED_CONSUMER_COUNT-1), or
         *         0xFFFF if consumer array is full (registration failed)
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *
         * @warning Consumer array has fixed size USER_DEFINED_CONSUMER_COUNT.
         *          Returns 0xFFFF when array is full. Caller must check return value.
         *
         * @attention Event IDs should follow OpenLCB Event Identifier format.
         *            See OpenLCB specification Section 6 for Event ID allocation rules.
         *
         * @note Return value is 0-based array index. Can be used directly to access
         *       the event: openlcb_node->consumers.list[return_value]
         *
         * @see OpenLcbApplication_clear_consumer_eventids
         * @see OpenLcbApplication_register_producer_eventid
         * @see protocol_event_transport.c - Event handling implementation
         */
    extern uint16_t OpenLcbApplication_register_consumer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status);

        /**
         * @brief Registers a producer event ID with the specified node
         *
         * @details Adds a new producer event to the node's producer list if space is
         * available. Producer events are those the node will send to the network when
         * conditions occur. The node will send Producer Identified messages during event
         * enumeration per OpenLCB Event Transport Protocol.
         *
         * Use cases:
         * - Registering events the node will produce (e.g., sensor state changes)
         * - Building initial event production list at startup
         * - Dynamic event registration during runtime configuration
         *
         * @param openlcb_node Pointer to the OpenLCB node
         * @param event_id 64-bit Event ID to register (8 bytes MSB first per OpenLCB spec)
         * @param event_status Initial status: EVENT_UNKNOWN, EVENT_VALID, or EVENT_INVALID
         *
         * @return 0-based array index where event was stored (0 to USER_DEFINED_PRODUCER_COUNT-1), or
         *         0xFFFF if producer array is full (registration failed)
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *
         * @warning Producer array has fixed size USER_DEFINED_PRODUCER_COUNT.
         *          Returns 0xFFFF when array is full. Caller must check return value.
         *
         * @attention Event IDs should follow OpenLCB Event Identifier format.
         *            See OpenLCB specification Section 6 for Event ID allocation rules.
         *
         * @note Return value is 0-based array index. Can be used directly to access
         *       the event: openlcb_node->producers.list[return_value]
         *
         * @see OpenLcbApplication_clear_producer_eventids
         * @see OpenLcbApplication_register_consumer_eventid
         * @see OpenLcbApplication_send_event_pc_report - To actually send the event
         * @see protocol_event_transport.c - Event handling implementation
         */
    extern uint16_t OpenLcbApplication_register_producer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status);

        /**
         * @brief Sends a Producer/Consumer event report message
         *
         * @details Sends a PC Event Report (PCER) message to the network indicating
         * that an event has occurred. This is used by producers to notify consumers
         * of state changes. Per OpenLCB Event Transport Protocol (Section 7), the
         * event must have been previously advertised via Producer Identified message
         * (except for automatically-routed events).
         *
         * Use cases:
         * - Reporting sensor state changes
         * - Notifying turnout position changes
         * - Signaling any event condition to interested consumers
         *
         * @param openlcb_node Pointer to the OpenLCB node sending the report
         * @param event_id 64-bit Event ID to report (8 bytes MSB first)
         *
         * @return true if message was successfully queued for transmission,
         *         false if message buffer is full or callback is NULL
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *
         * @warning Returns false if send callback is NULL. Ensure
         *          OpenLcbApplication_initialize() was called with valid callbacks.
         *
         * @attention The event should be registered as a producer event before sending,
         *            unless it is an automatically-routed well-known event.
         *
         * @attention Uses MTI_PC_EVENT_REPORT (0x05B4) for events without payload.
         *            For events with payload, use a different function (not implemented here).
         *
         * @note This function is non-blocking. Returns immediately after queueing.
         *
         * @see OpenLcbApplication_register_producer_eventid
         * @see OpenLcbApplication_send_teach_event
         * @see protocol_event_transport.c - Event protocol implementation
         */


    extern void OpenLcbApplication_clear_consumer_ranges(openlcb_node_t *openlcb_node);

    extern void OpenLcbApplication_clear_producer_ranges(openlcb_node_t *openlcb_node);

    extern bool OpenLcbApplication_register_consumer_range(openlcb_node_t *openlcb_node, event_id_t event_id_base, event_range_count_enum range_size);

    extern bool OpenLcbApplication_register_producer_range(openlcb_node_t *openlcb_node, event_id_t event_id_base, event_range_count_enum range_size);

    extern bool OpenLcbApplication_send_event_pc_report(openlcb_node_t *openlcb_node, event_id_t event_id);

        /**
         * @brief Sends a teach event message
         *
         * @details Sends a Learn Event (MTI 0x0594) message to teach other nodes about
         * an event ID. This is typically used during configuration to associate event
         * IDs with producers and consumers. Per OpenLCB Event Transport Protocol, this
         * allows nodes to learn which events to produce or consume.
         *
         * Use cases:
         * - Teaching a consumer which event to respond to
         * - Teaching a producer which event to send
         * - Event ID configuration during setup mode
         *
         * @param openlcb_node Pointer to the OpenLCB node sending the teach event
         * @param event_id 64-bit Event ID to teach (8 bytes MSB first)
         *
         * @return true if message was successfully queued for transmission,
         *         false if message buffer is full or callback is NULL
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *
         * @warning Returns false if send callback is NULL. Ensure
         *          OpenLcbApplication_initialize() was called with valid callbacks.
         *
         * @attention Uses MTI_EVENT_LEARN (0x0594) as specified in OpenLCB Event Transport.
         *
         * @note This function is non-blocking. Returns immediately after queueing.
         *
         * @see OpenLcbApplication_send_event_pc_report
         * @see protocol_event_transport.c - Event protocol implementation
         */
    extern bool OpenLcbApplication_send_teach_event(openlcb_node_t *openlcb_node, event_id_t event_id);

        /**
         * @brief Sends an initialization complete event for the specified node
         *
         * @details Sends an Initialization Complete message (MTI 0x0100) with the node's
         * full Node ID as payload. This message is required per OpenLCB Message Network
         * Protocol after a node completes its login sequence and is ready to participate
         * in the network. Other nodes use this to detect new nodes joining the network.
         *
         * Use cases:
         * - Final step of node login sequence after CAN alias negotiation
         * - Announcing node presence to the network
         * - Triggering event enumeration from other nodes
         *
         * @param openlcb_node Pointer to the OpenLCB node sending the initialization event
         *
         * @return true if message was successfully queued for transmission,
         *         false if message buffer is full or callback is NULL
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *
         * @warning Returns false if send callback is NULL. Ensure
         *          OpenLcbApplication_initialize() was called with valid callbacks.
         *
         * @attention Must be sent after successful alias negotiation and before sending
         *            any other OpenLCB messages (except alias negotiation frames).
         *
         * @attention Uses MTI_INITIALIZATION_COMPLETE (0x0100) with 6-byte Node ID payload.
         *
         * @note This function is non-blocking. Returns immediately after queueing.
         *
         * @see openlcb_login_statemachine.c - Login sequence implementation
         * @see protocol_message_network.c - Message network protocol
         */
    extern bool OpenLcbApplication_send_initialization_event(openlcb_node_t *openlcb_node);

        /**
         * @brief Reads data from the node's configuration memory
         *
         * @details Provides access to the node's configuration memory by calling the
         * application-provided config_memory_read callback. Configuration memory contains
         * node settings, event IDs, and other persistent data as defined by the OpenLCB
         * Memory Configuration Protocol.
         *
         * Use cases:
         * - Reading CDI (Configuration Description Information) data
         * - Accessing node configuration settings
         * - Retrieving stored event IDs
         * - Reading ACDI (Abbreviated CDI) or SNIP data
         *
         * @param openlcb_node Pointer to the OpenLCB node
         * @param address Starting address to read from (address space defined by CDI)
         * @param count Number of bytes to read (max 64 per datagram for network reads)
         * @param buffer Pointer to buffer to store the read data
         *
         * @return Number of bytes successfully read, or
         *         0xFFFF if callback is NULL or read operation failed
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *
         * @warning Buffer must have sufficient space for the requested byte count.
         *          @warning Buffer overflow will occur if insufficient space allocated.
         *
         * @warning Function pointer must be non-NULL.
         *          Returns 0xFFFF if callback is NULL.
         *
         * @attention Address and count must be valid for the node's memory configuration.
         *            Invalid addresses may return error code or partial reads.
         *
         * @note This is a synchronous call. Callback must complete before returning.
         *
         * @see OpenLcbApplication_write_configuration_memory
         * @see protocol_config_mem.c - Configuration memory protocol
         * @see interface_openlcb_application_t::config_memory_read
         */
    extern uint16_t OpenLcbApplication_read_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

        /**
         * @brief Writes data to the node's configuration memory
         *
         * @details Provides access to write the node's configuration memory by calling the
         * application-provided config_memory_write callback. Configuration memory contains
         * node settings, event IDs, and other persistent data as defined by the OpenLCB
         * Memory Configuration Protocol. Some memory spaces may be read-only.
         *
         * Use cases:
         * - Writing node configuration settings
         * - Storing event IDs during configuration
         * - Updating firmware parameters
         * - Modifying user-configurable data
         *
         * @param openlcb_node Pointer to the OpenLCB node
         * @param address Starting address to write to (address space defined by CDI)
         * @param count Number of bytes to write (max 64 per datagram for network writes)
         * @param buffer Pointer to buffer containing data to write
         *
         * @return Number of bytes successfully written, or
         *         0xFFFF if callback is NULL or write operation failed
         *
         * @warning Node pointer must not be NULL. No NULL check performed.
         *
         * @warning Buffer must contain sufficient valid data for the requested operation.
         *          Reading past allocated buffer size causes undefined behavior.
         *
         * @warning Function pointer must be non-NULL.
         *          Returns 0xFFFF if callback is NULL.
         *
         * @attention Some memory spaces are read-only (CDI, ACDI, etc.).
         *            Writes to read-only spaces should return error or 0 bytes written.
         *
         * @attention Address and count must be valid for the node's memory configuration.
         *            Invalid addresses may return error code or partial writes.
         *
         * @note This is a synchronous call. Callback must complete before returning.
         *
         * @see OpenLcbApplication_read_configuration_memory
         * @see protocol_config_mem.c - Configuration memory protocol
         * @see interface_openlcb_application_t::config_memory_write
         */
    extern uint16_t OpenLcbApplication_write_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_APPLICATION__ */
