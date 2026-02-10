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
     * @file openlcb_application.c
     * @brief Implementation of the application layer interface
     *
     * @details This module implements the high-level application programming interface
     * for the OpenLCB protocol stack. It provides functions for event registration,
     * event transmission, and configuration memory access.
     *
     * The implementation maintains a static interface pointer to application-provided
     * callbacks for message transmission and memory operations. All functions are
     * designed to be simple wrappers that either manipulate node structures directly
     * or call through to the registered callbacks.
     *
     * Event registration functions modify the node's producer/consumer lists in place.
     * Event transmission functions build OpenLCB messages and queue them via the
     * send_openlcb_msg callback. Memory access functions provide pass-through to
     * application-defined memory handlers with NULL checking.
     *
     * @author Jim Kueneman
     * @date 17 Jan 2026
     *
     * @see openlcb_application.h
     * @see protocol_event_transport.c - Event protocol handling
     * @see protocol_config_mem.c - Configuration memory protocol handling
     */

#include "openlcb_application.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"

    /** @brief Static pointer to application interface functions */
    static interface_openlcb_application_t *_interface;
    
    /**
     * @brief Initializes the OpenLCB Application interface
     *
     * @details Algorithm:
     * Stores the application-provided interface pointer in static memory for use by
     * all application layer functions throughout the program's lifetime.
     * -# Cast away const qualifier from interface pointer
     * -# Store pointer in static _interface variable
     * -# Interface is now available to all application layer functions
     *
     * @verbatim
     * @param interface_openlcb_application Pointer to interface structure with callbacks
     *
     * @endverbatim
     * @warning MUST be called exactly once during application startup before any
     *          other OpenLCB application functions. Calling multiple times will
     *          reset the interface pointer and may cause loss of program context.
     *
     * @warning The interface pointer is stored in static memory. The pointed-to
     *          structure must remain valid for the lifetime of the program.
     *
     * @warning All function pointers in the interface must be non-NULL.
     *          NULL pointers will cause crashes when library attempts callbacks.
     *
     * @attention Must be called during single-threaded initialization phase.
     *
     * @note The const cast is safe because the interface structure should be
     *       defined as a static const in the application code.
     *
     * @see interface_openlcb_application_t
     */
void OpenLcbApplication_initialize(const interface_openlcb_application_t *interface_openlcb_application) {

    _interface = (interface_openlcb_application_t*) interface_openlcb_application;

}

static bool _send_event_with_mti(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t mti)
{

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *)&payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
        &msg,
        openlcb_node->alias,
        openlcb_node->id,
        0,
        NULL_NODE_ID,
        mti);

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(&msg, event_id);

    if (_interface->send_openlcb_msg)
    {

        return _interface->send_openlcb_msg(&msg);
    }

    return false;
}

    /**
     * @brief Clears all consumer event IDs registered for the specified node
     *
     * @details Algorithm:
     * Resets the consumer event count to zero, effectively clearing the list.
     * -# Set openlcb_node->consumers.count = 0
     * -# Consumer list is now empty (existing event IDs remain in memory but inaccessible)
     *
     * @verbatim
     * @param openlcb_node Pointer to the OpenLCB node
     *
     * @endverbatim
     * @warning Node pointer must not be NULL. No NULL check performed.
     *          Passing NULL will cause immediate crash on dereference.
     *
     * @note Non-destructive operation - Event IDs remain in memory but are
     *       inaccessible due to zero count.
     *
     * @see OpenLcbApplication_register_consumer_eventid
     * @see OpenLcbApplication_clear_producer_eventids
     */
void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t *openlcb_node) {

    openlcb_node->consumers.count = 0;

}

    /**
     * @brief Clears all producer event IDs registered for the specified node
     *
     * @details Algorithm:
     * Resets the producer event count to zero, effectively clearing the list.
     * -# Set openlcb_node->producers.count = 0
     * -# Producer list is now empty (existing event IDs remain in memory but inaccessible)
     *
     * @verbatim
     * @param openlcb_node Pointer to the OpenLCB node
     *
     * @endverbatim
     * @warning Node pointer must not be NULL. No NULL check performed.
     *          Passing NULL will cause immediate crash on dereference.
     *
     * @note Non-destructive operation - Event IDs remain in memory but are
     *       inaccessible due to zero count.
     *
     * @see OpenLcbApplication_register_producer_eventid
     * @see OpenLcbApplication_clear_consumer_eventids
     */
void OpenLcbApplication_clear_producer_eventids(openlcb_node_t *openlcb_node) {

    openlcb_node->producers.count = 0;

}

    /**
     * @brief Registers a consumer event ID with the specified node
     *
     * @details Algorithm:
     * Adds event to consumer list if space available, otherwise returns error.
     * -# Check if count < USER_DEFINED_CONSUMER_COUNT (room in array)
     * -# If room: Store event_id at consumers.list[count].event
     * -# Store event_status at consumers.list[count].status
     * -# Increment consumers.count by 1
     * -# Return count - 1 (0-based array index where event was stored)
     * -# If no room: Return 0xFFFF error code
     *
     * @verbatim
     * @param openlcb_node Pointer to the OpenLCB node
     * @param event_id 64-bit Event ID to register (8 bytes MSB first per OpenLCB spec)
     * @param event_status Initial status: EVENT_UNKNOWN, EVENT_VALID, or EVENT_INVALID
     *
     * @endverbatim
     * @return 0-based array index where event was stored (0 to USER_DEFINED_CONSUMER_COUNT-1), or
     *         0xFFFF if consumer array is full (registration failed)
     *
     * @warning Node pointer must not be NULL. No NULL check performed.
     *
     * @warning Consumer array has fixed size USER_DEFINED_CONSUMER_COUNT.
     *          Caller must check for 0xFFFF return to detect full array.
     *
     * @attention Uses direct comparison (count < MAX) to check for available space.
     *            Valid indices are 0 to USER_DEFINED_CONSUMER_COUNT-1.
     *
     * @note Return value is 0-based array index. Can be used directly to access
     *       the event: openlcb_node->consumers.list[return_value]
     *
     * @see OpenLcbApplication_clear_consumer_eventids
     * @see OpenLcbApplication_register_producer_eventid
     * @see protocol_event_transport.c - Consumer Identified message handling
     */
uint16_t OpenLcbApplication_register_consumer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status) {

    if (openlcb_node->consumers.count < USER_DEFINED_CONSUMER_COUNT) {

        openlcb_node->consumers.list[openlcb_node->consumers.count].event = event_id;
        openlcb_node->consumers.list[openlcb_node->consumers.count].status = event_status;
        openlcb_node->consumers.count = openlcb_node->consumers.count + 1;

        return (openlcb_node->consumers.count - 1);
    }

    return 0xFFFF;

}

    /**
     * @brief Registers a producer event ID with the specified node
     *
     * @details Algorithm:
     * Adds event to producer list if space available, otherwise returns error.
     * -# Check if count < USER_DEFINED_PRODUCER_COUNT (room in array)
     * -# If room: Store event_id at producers.list[count].event
     * -# Store event_status at producers.list[count].status
     * -# Increment producers.count by 1
     * -# Return count - 1 (0-based array index where event was stored)
     * -# If no room: Return 0xFFFF error code
     *
     * @verbatim
     * @param openlcb_node Pointer to the OpenLCB node
     * @param event_id 64-bit Event ID to register (8 bytes MSB first per OpenLCB spec)
     * @param event_status Initial status: EVENT_UNKNOWN, EVENT_VALID, or EVENT_INVALID
     *
     * @endverbatim
     * @return 0-based array index where event was stored (0 to USER_DEFINED_PRODUCER_COUNT-1), or
     *         0xFFFF if producer array is full (registration failed)
     *
     * @warning Node pointer must not be NULL. No NULL check performed.
     *
     * @warning Producer array has fixed size USER_DEFINED_PRODUCER_COUNT.
     *          Caller must check for 0xFFFF return to detect full array.
     *
     * @attention Uses direct comparison (count < MAX) to check for available space.
     *            Valid indices are 0 to USER_DEFINED_PRODUCER_COUNT-1.
     *
     * @note Return value is 0-based array index. Can be used directly to access
     *       the event: openlcb_node->producers.list[return_value]
     *
     * @see OpenLcbApplication_clear_producer_eventids
     * @see OpenLcbApplication_register_consumer_eventid
     * @see OpenLcbApplication_send_event_pc_report - To send registered events
     * @see protocol_event_transport.c - Producer Identified message handling
     */
uint16_t OpenLcbApplication_register_producer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status) {

    if (openlcb_node->producers.count < USER_DEFINED_PRODUCER_COUNT) {

        openlcb_node->producers.list[openlcb_node->producers.count].event = event_id;
        openlcb_node->producers.list[openlcb_node->producers.count].status = event_status;
        openlcb_node->producers.count = openlcb_node->producers.count + 1;

        return (openlcb_node->producers.count - 1);
    }

    return 0xFFFF;
}

void OpenLcbApplication_clear_consumer_ranges(openlcb_node_t *openlcb_node) {

    openlcb_node->consumers.range_count = 0;

}

void OpenLcbApplication_clear_producer_ranges(openlcb_node_t *openlcb_node) {

    openlcb_node->producers.range_count = 0;

}

bool OpenLcbApplication_register_consumer_range(openlcb_node_t *openlcb_node, event_id_t event_id_base, event_range_count_enum range_size)
{

    if (openlcb_node->consumers.range_count < USER_DEFINED_CONSUMER_RANGE_COUNT) {

        openlcb_node->consumers.range_list[openlcb_node->consumers.range_count].start_base = event_id_base;
        openlcb_node->consumers.range_list[openlcb_node->consumers.range_count].event_count = range_size;
        openlcb_node->consumers.range_count++;

        return true;
    }

    return false;
}

bool OpenLcbApplication_register_producer_range(openlcb_node_t *openlcb_node, event_id_t event_id_base, event_range_count_enum range_size)
{

    if (openlcb_node->producers.range_count < USER_DEFINED_PRODUCER_RANGE_COUNT) {

        openlcb_node->producers.range_list[openlcb_node->producers.range_count].start_base = event_id_base;
        openlcb_node->producers.range_list[openlcb_node->producers.range_count].event_count = range_size;
        openlcb_node->producers.range_count++;

        return true;
    }

    return false;
}

/**
 * @brief Sends a Producer/Consumer event report message
 *
 * @details Algorithm:
 * Builds a PC Event Report message and queues it for transmission.
 * -# Declare msg and payload structures on stack
 * -# Set msg.payload to point to local payload structure
 * -# Set msg.payload_type = BASIC (no extended payload)
 * -# Call OpenLcbUtilities_load_openlcb_message() with:
 *    - Source alias from openlcb_node->alias
 *    - Source ID from openlcb_node->id
 *    - Destination alias = 0 (global message)
 *    - Destination ID = NULL_NODE_ID (global)
 *    - MTI = MTI_PC_EVENT_REPORT (0x05B4)
 * -# Call OpenLcbUtilities_copy_event_id_to_openlcb_payload() to add event_id to payload
 * -# Check if _interface->send_openlcb_msg is non-NULL
 * -# If NULL: Return false (callback not initialized)
 * -# If non-NULL: Call _interface->send_openlcb_msg() to queue message
 * -# Return true if send succeeded, false if buffer full or callback NULL
 *
 * @verbatim
 * @param openlcb_node Pointer to the OpenLCB node sending the report
 * @param event_id 64-bit Event ID to report (8 bytes MSB first)
 *
 * @endverbatim
 * @return true if message was successfully queued for transmission,
 *         false if message buffer is full or callback is NULL
 *
 * @warning Node pointer must not be NULL. No NULL check performed.
 *
 * @warning Returns false if _interface->send_openlcb_msg is NULL.
 *          Ensure OpenLcbApplication_initialize() was called with valid callbacks.
 *
 * @attention Per OpenLCB Event Transport Protocol Section 7, events should be
 *            advertised via Producer Identified before sending PCER (except
 *            automatically-routed well-known events).
 *
 * @note This sends a global (unaddressed) message with MTI 0x05B4.
 *       Event payload is 8 bytes (64-bit event ID).
 *
 * @note Function is non-blocking. Message is queued, not transmitted immediately.
 *
 * @see OpenLcbApplication_register_producer_eventid
 * @see OpenLcbUtilities_load_openlcb_message
 * @see OpenLcbUtilities_copy_event_id_to_openlcb_payload
 * @see protocol_event_transport.c - PCER message handling
 */
bool OpenLcbApplication_send_event_pc_report(openlcb_node_t *openlcb_node, event_id_t event_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) &payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &msg,
            openlcb_node->alias,
            openlcb_node->id,
            0,
            NULL_NODE_ID,
            MTI_PC_EVENT_REPORT);

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            &msg,
            event_id);

    if (_interface->send_openlcb_msg) {

        return _interface->send_openlcb_msg(&msg);

    }

    return false;
}

    /**
     * @brief Sends a teach event message
     *
     * @details Algorithm:
     * Builds a Learn Event message and queues it for transmission.
     * -# Declare msg and payload structures on stack
     * -# Set msg.payload to point to local payload structure
     * -# Set msg.payload_type = BASIC (no extended payload)
     * -# Call OpenLcbUtilities_load_openlcb_message() with:
     *    - Source alias from openlcb_node->alias
     *    - Source ID from openlcb_node->id
     *    - Destination alias = 0 (global message)
     *    - Destination ID = NULL_NODE_ID (global)
     *    - MTI = MTI_EVENT_LEARN (0x0594)
     * -# Call OpenLcbUtilities_copy_event_id_to_openlcb_payload() to add event_id to payload
     * -# Check if _interface->send_openlcb_msg is non-NULL
     * -# If NULL: Return false (callback not initialized)
     * -# If non-NULL: Call _interface->send_openlcb_msg() to queue message
     * -# Return true if send succeeded, false if buffer full or callback NULL
     *
     * @verbatim
     * @param openlcb_node Pointer to the OpenLCB node sending the teach event
     * @param event_id 64-bit Event ID to teach (8 bytes MSB first)
     *
     * @endverbatim
     * @return true if message was successfully queued for transmission,
     *         false if message buffer is full or callback is NULL
     *
     * @warning Node pointer must not be NULL. No NULL check performed.
     *
     * @warning Returns false if _interface->send_openlcb_msg is NULL.
     *          Ensure OpenLcbApplication_initialize() was called with valid callbacks.
     *
     * @attention Per OpenLCB Event Transport Protocol, Learn Event (0x0594) is used
     *            to teach other nodes about event IDs during configuration.
     *
     * @note This sends a global (unaddressed) message with MTI 0x0594.
     *       Event payload is 8 bytes (64-bit event ID).
     *
     * @note Function is non-blocking. Message is queued, not transmitted immediately.
     *
     * @see OpenLcbApplication_send_event_pc_report
     * @see OpenLcbUtilities_load_openlcb_message
     * @see OpenLcbUtilities_copy_event_id_to_openlcb_payload
     * @see protocol_event_transport.c - Learn Event message handling
     */
bool OpenLcbApplication_send_teach_event(openlcb_node_t *openlcb_node, event_id_t event_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) &payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &msg,
            openlcb_node->alias,
            openlcb_node->id,
            0,
            NULL_NODE_ID,
            MTI_EVENT_LEARN);

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            &msg,
            event_id);

    if (_interface->send_openlcb_msg) {

        return _interface->send_openlcb_msg(&msg);

    }

    return false;

}

    /**
     * @brief Sends an initialization complete event for the specified node
     *
     * @details Algorithm:
     * Builds an Initialization Complete message with node ID payload and queues it.
     * -# Declare msg and payload structures on stack
     * -# Set msg.payload to point to local payload structure
     * -# Set msg.payload_type = BASIC (no extended payload)
     * -# Call OpenLcbUtilities_load_openlcb_message() with:
     *    - Source alias from openlcb_node->alias
     *    - Source ID from openlcb_node->id
     *    - Destination alias = 0 (global message)
     *    - Destination ID = NULL_NODE_ID (global)
     *    - MTI = MTI_INITIALIZATION_COMPLETE (0x0100)
     * -# Call OpenLcbUtilities_copy_node_id_to_openlcb_payload() to add node ID to payload at offset 0
     * -# Check if _interface->send_openlcb_msg is non-NULL
     * -# If NULL: Return false (callback not initialized)
     * -# If non-NULL: Call _interface->send_openlcb_msg() to queue message
     * -# Return true if send succeeded, false if buffer full or callback NULL
     *
     * @verbatim
     * @param openlcb_node Pointer to the OpenLCB node sending the initialization event
     *
     * @endverbatim
     * @return true if message was successfully queued for transmission,
     *         false if message buffer is full or callback is NULL
     *
     * @warning Node pointer must not be NULL. No NULL check performed.
     *
     * @warning Returns false if _interface->send_openlcb_msg is NULL.
     *          Ensure OpenLcbApplication_initialize() was called with valid callbacks.
     *
     * @attention Per OpenLCB Message Network Protocol, Initialization Complete (0x0100)
     *            must be sent after alias negotiation completes and before sending any
     *            other OpenLCB messages (except alias negotiation frames).
     *
     * @attention The payload contains the node's full 48-bit (6-byte) Node ID.
     *            This allows other nodes to map the CAN alias to the full Node ID.
     *
     * @note This sends a global (unaddressed) message with MTI 0x0100.
     *       Payload is 6 bytes (48-bit node ID, MSB first).
     *
     * @note Function is non-blocking. Message is queued, not transmitted immediately.
     *
     * @see openlcb_login_statemachine.c - Login sequence that calls this function
     * @see OpenLcbUtilities_load_openlcb_message
     * @see OpenLcbUtilities_copy_node_id_to_openlcb_payload
     * @see protocol_message_network.c - Initialization Complete handling
     */
bool OpenLcbApplication_send_initialization_event(openlcb_node_t *openlcb_node) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) &payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &msg,
            openlcb_node->alias,
            openlcb_node->id,
            0,
            NULL_NODE_ID,
            MTI_INITIALIZATION_COMPLETE);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            &msg,
            openlcb_node->id,
            0);

    if (_interface->send_openlcb_msg) {

        return _interface->send_openlcb_msg(&msg);

    }

    return false;

}

    /**
     * @brief Reads data from the node's configuration memory
     *
     * @details Algorithm:
     * Calls application-provided read callback if available, otherwise returns error.
     * -# Check if _interface->config_memory_read is non-NULL
     * -# If NULL: Return 0xFFFF error code
     * -# If non-NULL: Call _interface->config_memory_read() with all parameters
     * -# Return result from callback (byte count or error code)
     *
     * @verbatim
     * @param openlcb_node Pointer to the OpenLCB node
     * @param address Starting address to read from (address space defined by CDI)
     * @param count Number of bytes to read
     * @param buffer Pointer to buffer to store the read data
     *
     * @endverbatim
     * @return Number of bytes successfully read from callback, or
     *         0xFFFF if callback is NULL or read operation failed
     *
     * @warning openlcb_node must not be NULL. Not validated before callback.
     *
     * @warning Buffer must have sufficient space for the requested byte count.
     *          Application callback is responsible for buffer overflow prevention.
     *
     * @warning Returns 0xFFFF if _interface->config_memory_read is NULL.
     *          Application must initialize interface before calling.
     *
     * @attention This is a pass-through function. All validation (address range,
     *            memory space, access permissions) is done by application callback.
     *
     * @note This is a synchronous call. Blocks until callback completes.
     *
     * @remark Application callback determines actual read behavior, including:
     *         - Which memory spaces are available (CDI, ACDI, user config, etc.)
     *         - Address range validation
     *         - Read-only vs read-write space handling
     *         - Error code returns for invalid operations
     *
     * @see OpenLcbApplication_write_configuration_memory
     * @see interface_openlcb_application_t::config_memory_read
     * @see protocol_config_mem.c - Configuration Memory Protocol handler
     */
uint16_t OpenLcbApplication_read_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

    if (_interface->config_memory_read) {

        return (_interface->config_memory_read(
                openlcb_node,
                address,
                count,
                buffer)
                );

    }

    return 0xFFFF;
}

    /**
     * @brief Writes data to the node's configuration memory
     *
     * @details Algorithm:
     * Calls application-provided write callback if available, otherwise returns error.
     * -# Check if _interface->config_memory_write is non-NULL
     * -# If NULL: Return 0xFFFF error code
     * -# If non-NULL: Call _interface->config_memory_write() with all parameters
     * -# Return result from callback (byte count or error code)
     *
     * @verbatim
     * @param openlcb_node Pointer to the OpenLCB node
     * @param address Starting address to write to (address space defined by CDI)
     * @param count Number of bytes to write
     * @param buffer Pointer to buffer containing data to write
     *
     * @endverbatim
     * @return Number of bytes successfully written from callback, or
     *         0xFFFF if callback is NULL or write operation failed
     *
     * @warning openlcb_node must not be NULL. Not validated before callback.
     *
     * @warning Buffer must contain sufficient valid data for the requested operation.
     *          Application callback is responsible for bounds checking.
     *
     * @warning Returns 0xFFFF if _interface->config_memory_write is NULL.
     *          Application must initialize interface before calling.
     *
     * @attention This is a pass-through function. All validation (address range,
     *            memory space, access permissions, read-only checks) is done by
     *            application callback.
     *
     * @attention Some memory spaces are read-only (CDI, ACDI, manufacturer data).
     *            Application callback should return 0 or error for read-only writes.
     *
     * @note This is a synchronous call. Blocks until callback completes.
     *
     * @remark Application callback determines actual write behavior, including:
     *         - Which memory spaces are writable (user config, etc.)
     *         - Address range validation
     *         - Read-only space rejection
     *         - Error code returns for invalid operations
     *         - Persistence mechanism (EEPROM, flash, RAM, etc.)
     *
     * @see OpenLcbApplication_read_configuration_memory
     * @see interface_openlcb_application_t::config_memory_write
     * @see protocol_config_mem.c - Configuration Memory Protocol handler
     */
uint16_t OpenLcbApplication_write_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

    if (_interface->config_memory_write) {

        return (_interface->config_memory_write(
                openlcb_node,
                address,
                count,
                buffer)
                );

    }

    return 0xFFFF;
}

bool OpenLcbApplication_setup_clock_consumer(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    openlcb_node->is_clock_consumer = 1;
    openlcb_node->clock_state.clock_id = clock_id;

    if (!OpenLcbApplication_register_consumer_range(openlcb_node, clock_id | 0x0000, EVENT_RANGE_COUNT_32768))
        return false;

    if (!OpenLcbApplication_register_consumer_range(openlcb_node, clock_id | 0x8000, EVENT_RANGE_COUNT_32768))
        return false;

    return true;
}

bool OpenLcbApplication_setup_clock_producer(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    openlcb_node->is_clock_producer = 1;
    openlcb_node->clock_state.clock_id = clock_id;

    if (!OpenLcbApplication_register_producer_range(openlcb_node, clock_id | 0x0000, EVENT_RANGE_COUNT_32768))
        return false;

    if (!OpenLcbApplication_register_producer_range(openlcb_node, clock_id | 0x8000, EVENT_RANGE_COUNT_32768))
        return false;

    return true;
}

bool OpenLcbApplication_send_clock_report_time(openlcb_node_t *openlcb_node, uint8_t hour, uint8_t minute) {

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(openlcb_node->clock_state.clock_id, hour, minute, false);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}

bool OpenLcbApplication_send_clock_report_date(openlcb_node_t *openlcb_node, uint8_t month, uint8_t day) {

    event_id_t event_id = OpenLcbUtilities_create_date_event_id(openlcb_node->clock_state.clock_id, month, day, false);

    return _send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET);
}

bool OpenLcbApplication_send_clock_report_year(openlcb_node_t *openlcb_node, uint16_t year) {

    event_id_t event_id = OpenLcbUtilities_create_year_event_id(openlcb_node->clock_state.clock_id, year, false);

    return _send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET);
}

bool OpenLcbApplication_send_clock_report_rate(openlcb_node_t *openlcb_node, int16_t rate) {

    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(openlcb_node->clock_state.clock_id, rate, false);

    return _send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET);
}

bool OpenLcbApplication_send_clock_start(openlcb_node_t *openlcb_node) {

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(openlcb_node->clock_state.clock_id, BROADCAST_TIME_EVENT_START);

    return _send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET);
}

bool OpenLcbApplication_send_clock_stop(openlcb_node_t *openlcb_node) {

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(openlcb_node->clock_state.clock_id, BROADCAST_TIME_EVENT_STOP);

    return _send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET);
}

bool OpenLcbApplication_send_clock_date_rollover(openlcb_node_t *openlcb_node) {

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(openlcb_node->clock_state.clock_id, BROADCAST_TIME_EVENT_DATE_ROLLOVER);

    return _send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET);
}

bool OpenLcbApplication_send_clock_full_sync(openlcb_node_t *openlcb_node, uint8_t next_hour, uint8_t next_minute) {

    event_id_t event_id;

    // 1. Start or Stop
    if (openlcb_node->clock_state.is_running)
        event_id = OpenLcbUtilities_create_command_event_id(openlcb_node->clock_state.clock_id, BROADCAST_TIME_EVENT_START);
    else
        event_id = OpenLcbUtilities_create_command_event_id(openlcb_node->clock_state.clock_id, BROADCAST_TIME_EVENT_STOP);

    if (!_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET))
        return false;

    // 2. Rate
    event_id = OpenLcbUtilities_create_rate_event_id(openlcb_node->clock_state.clock_id, openlcb_node->clock_state.rate.rate, false);

    if (!_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET))
        return false;

    // 3. Year
    event_id = OpenLcbUtilities_create_year_event_id(openlcb_node->clock_state.clock_id, openlcb_node->clock_state.year.year, false);

    if (!_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET))
        return false;

    // 4. Date
    event_id = OpenLcbUtilities_create_date_event_id(openlcb_node->clock_state.clock_id, openlcb_node->clock_state.date.month, openlcb_node->clock_state.date.day, false);

    if (!_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET))
        return false;

    // 5. Time (Producer Identified Valid)
    event_id = OpenLcbUtilities_create_time_event_id(openlcb_node->clock_state.clock_id, openlcb_node->clock_state.time.hour, openlcb_node->clock_state.time.minute, false);

    if (!_send_event_with_mti(openlcb_node, event_id, MTI_PRODUCER_IDENTIFIED_SET))
        return false;

    // 6. Next minute (PC Event Report)
    event_id = OpenLcbUtilities_create_time_event_id(openlcb_node->clock_state.clock_id, next_hour, next_minute, false);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}

bool OpenLcbApplication_send_clock_query(openlcb_node_t *openlcb_node) {

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(openlcb_node->clock_state.clock_id, BROADCAST_TIME_EVENT_QUERY);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}

bool OpenLcbApplication_send_clock_set_time(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t hour, uint8_t minute) {

    event_id_t event_id = OpenLcbUtilities_create_time_event_id(clock_id, hour, minute, true);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}

bool OpenLcbApplication_send_clock_set_date(openlcb_node_t *openlcb_node, event_id_t clock_id, uint8_t month, uint8_t day) {

    event_id_t event_id = OpenLcbUtilities_create_date_event_id(clock_id, month, day, true);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}

bool OpenLcbApplication_send_clock_set_year(openlcb_node_t *openlcb_node, event_id_t clock_id, uint16_t year) {

    event_id_t event_id = OpenLcbUtilities_create_year_event_id(clock_id, year, true);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}

bool OpenLcbApplication_send_clock_set_rate(openlcb_node_t *openlcb_node, event_id_t clock_id, int16_t rate){

    event_id_t event_id = OpenLcbUtilities_create_rate_event_id(clock_id, rate, true);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}

bool OpenLcbApplication_send_clock_command_start(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(clock_id, BROADCAST_TIME_EVENT_START);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}

bool OpenLcbApplication_send_clock_command_stop(openlcb_node_t *openlcb_node, event_id_t clock_id) {

    event_id_t event_id = OpenLcbUtilities_create_command_event_id(clock_id, BROADCAST_TIME_EVENT_STOP);

    return OpenLcbApplication_send_event_pc_report(openlcb_node, event_id);
}
