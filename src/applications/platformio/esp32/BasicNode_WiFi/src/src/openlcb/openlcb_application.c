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
 * @brief Implementation of the application layer interface.
 *
 * @details Implements the high-level application programming interface for the
 * OpenLCB protocol stack.  Maintains a single static pointer to the
 * application-provided callback structure set by OpenLcbApplication_initialize().
 * All public functions are thin wrappers that either manipulate node data
 * directly or call through to those callbacks.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "openlcb_application.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"

/** @brief Static pointer to application interface functions. */
static interface_openlcb_application_t *_interface;

    /**
     * @brief Stores the application callback interface for use by all application layer functions.
     *
     * @details Algorithm:
     * -# Cast away the const qualifier from the interface pointer.
     * -# Store the pointer in the static _interface variable.
     *
     * @verbatim
     * @param interface_openlcb_application  Pointer to a fully initialised interface_openlcb_application_t.
     * @endverbatim
     *
     * @warning MUST be called exactly once before any other function in this module.
     * @warning The pointed-to structure must remain valid for the lifetime of the program.
     * @warning All three function pointers in the structure must be non-NULL.
     */
void OpenLcbApplication_initialize(const interface_openlcb_application_t *interface_openlcb_application) {

    _interface = (interface_openlcb_application_t*) interface_openlcb_application;

}

    /**
     * @brief Sends an event message with an arbitrary MTI.
     *
     * @details Algorithm:
     * -# Declare msg and payload on the stack.
     * -# Point msg.payload at the local payload buffer; set payload_type to BASIC.
     * -# Call OpenLcbUtilities_load_openlcb_message() with the node's alias/ID, a zero
     *    destination alias, NULL_NODE_ID destination, and the caller-supplied MTI.
     * -# Call OpenLcbUtilities_copy_event_id_to_openlcb_payload() to place event_id in the payload.
     * -# If the send callback is non-NULL, call it and return its result; otherwise return false.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param event_id      64-bit event_id_t to include in the payload.
     * @param mti           16-bit MTI for the message.
     * @endverbatim
     *
     * @return true if queued successfully, false if the transmit buffer is full or
     *         the callback is NULL.
     *
     * @warning NULL pointer on the node causes a crash — no NULL check is performed.
     */
bool OpenLcbApplication_send_event_with_mti(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t mti)
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
     * @brief Clears the consumer event list for a node by resetting its count to zero.
     *
     * @details Algorithm:
     * -# Set openlcb_node->consumers.count to 0.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t to clear.
     * @endverbatim
     *
     * @warning NULL pointer causes a crash — no NULL check is performed.
     */
void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t *openlcb_node) {

    openlcb_node->consumers.count = 0;

}

    /**
     * @brief Clears the producer event list for a node by resetting its count to zero.
     *
     * @details Algorithm:
     * -# Set openlcb_node->producers.count to 0.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t to clear.
     * @endverbatim
     *
     * @warning NULL pointer causes a crash — no NULL check is performed.
     */
void OpenLcbApplication_clear_producer_eventids(openlcb_node_t *openlcb_node) {

    openlcb_node->producers.count = 0;

}

    /**
     * @brief Adds a consumer event ID to the node's consumer list.
     *
     * @details Algorithm:
     * -# If consumers.count < USER_DEFINED_CONSUMER_COUNT:
     *    - Store event_id and event_status at consumers.list[count].
     *    - Increment consumers.count.
     *    - Return the new entry's 0-based index (count - 1).
     * -# Otherwise return 0xFFFF.
     *
     * @verbatim
     * @param openlcb_node   Pointer to the openlcb_node_t receiving the registration.
     * @param event_id       64-bit event_id_t to register (8 bytes, MSB first).
     * @param event_status   Initial status: one of the event_status_enum values.
     * @endverbatim
     *
     * @return 0-based index of the newly registered entry, or 0xFFFF if the array is full.
     *
     * @warning NULL pointer causes a crash — no NULL check is performed on the node.
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
     * @brief Adds a producer event ID to the node's producer list.
     *
     * @details Algorithm:
     * -# If producers.count < USER_DEFINED_PRODUCER_COUNT:
     *    - Store event_id and event_status at producers.list[count].
     *    - Increment producers.count.
     *    - Return the new entry's 0-based index (count - 1).
     * -# Otherwise return 0xFFFF.
     *
     * @verbatim
     * @param openlcb_node   Pointer to the openlcb_node_t receiving the registration.
     * @param event_id       64-bit event_id_t to register (8 bytes, MSB first).
     * @param event_status   Initial status: one of the event_status_enum values.
     * @endverbatim
     *
     * @return 0-based index of the newly registered entry, or 0xFFFF if the array is full.
     *
     * @warning NULL pointer causes a crash — no NULL check is performed on the node.
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

    /**
     * @brief Clears the consumer event-range list by resetting its range count to zero.
     *
     * @details Algorithm:
     * -# Set openlcb_node->consumers.range_count to 0.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t to clear.
     * @endverbatim
     *
     * @warning NULL pointer causes a crash — no NULL check is performed.
     */
void OpenLcbApplication_clear_consumer_ranges(openlcb_node_t *openlcb_node) {

    openlcb_node->consumers.range_count = 0;

}

    /**
     * @brief Clears the producer event-range list by resetting its range count to zero.
     *
     * @details Algorithm:
     * -# Set openlcb_node->producers.range_count to 0.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t to clear.
     * @endverbatim
     *
     * @warning NULL pointer causes a crash — no NULL check is performed.
     */
void OpenLcbApplication_clear_producer_ranges(openlcb_node_t *openlcb_node) {

    openlcb_node->producers.range_count = 0;

}

    /**
     * @brief Registers an event ID range in the node's consumer range list.
     *
     * @details Algorithm:
     * -# If consumers.range_count < USER_DEFINED_CONSUMER_RANGE_COUNT:
     *    - Store event_id_base and range_size in the next range_list slot.
     *    - Increment consumers.range_count.
     *    - Return true.
     * -# Otherwise return false.
     *
     * @verbatim
     * @param openlcb_node    Pointer to the openlcb_node_t receiving the registration.
     * @param event_id_base   Base event_id_t of the range.
     * @param range_size      Number of consecutive event IDs (event_range_count_enum).
     * @endverbatim
     *
     * @return true if registered successfully, false if the range array is full.
     *
     * @warning NULL pointer causes a crash — no NULL check is performed on the node.
     */
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

    /**
     * @brief Registers an event ID range in the node's producer range list.
     *
     * @details Algorithm:
     * -# If producers.range_count < USER_DEFINED_PRODUCER_RANGE_COUNT:
     *    - Store event_id_base and range_size in the next range_list slot.
     *    - Increment producers.range_count.
     *    - Return true.
     * -# Otherwise return false.
     *
     * @verbatim
     * @param openlcb_node    Pointer to the openlcb_node_t receiving the registration.
     * @param event_id_base   Base event_id_t of the range.
     * @param range_size      Number of consecutive event IDs (event_range_count_enum).
     * @endverbatim
     *
     * @return true if registered successfully, false if the range array is full.
     *
     * @warning NULL pointer causes a crash — no NULL check is performed on the node.
     */
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
     * @brief Sends a Producer/Consumer Event Report (PCER) message to the network.
     *
     * @details Algorithm:
     * -# Declare msg and payload on the stack.
     * -# Point msg.payload at the local payload buffer; set payload_type to BASIC.
     * -# Call OpenLcbUtilities_load_openlcb_message() with the node's alias/ID,
     *    a zero destination alias, NULL_NODE_ID destination, and MTI_PC_EVENT_REPORT (0x05B4).
     * -# Call OpenLcbUtilities_copy_event_id_to_openlcb_payload() to place event_id in the payload.
     * -# If the send callback is non-NULL, call it and return its result; otherwise return false.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param event_id      64-bit event_id_t to report.
     * @endverbatim
     *
     * @return true if queued successfully, false if the transmit buffer is full or
     *         the callback is NULL.
     *
     * @warning NULL pointer on the node causes a crash — no NULL check is performed.
     * @warning Returns false if OpenLcbApplication_initialize() was not called first.
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
     * @brief Sends a Learn Event (teach) message to the network.
     *
     * @details Algorithm:
     * -# Declare msg and payload on the stack.
     * -# Point msg.payload at the local payload buffer; set payload_type to BASIC.
     * -# Call OpenLcbUtilities_load_openlcb_message() with the node's alias/ID,
     *    a zero destination alias, NULL_NODE_ID destination, and MTI_EVENT_LEARN (0x0594).
     * -# Call OpenLcbUtilities_copy_event_id_to_openlcb_payload() to place event_id in the payload.
     * -# If the send callback is non-NULL, call it and return its result; otherwise return false.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the sending openlcb_node_t.
     * @param event_id      64-bit event_id_t to teach.
     * @endverbatim
     *
     * @return true if queued successfully, false if the transmit buffer is full or
     *         the callback is NULL.
     *
     * @warning NULL pointer on the node causes a crash — no NULL check is performed.
     * @warning Returns false if OpenLcbApplication_initialize() was not called first.
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
     * @brief Sends an Initialization Complete message for the specified node.
     *
     * @details Algorithm:
     * -# Declare msg and payload on the stack.
     * -# Point msg.payload at the local payload buffer; set payload_type to BASIC.
     * -# Call OpenLcbUtilities_load_openlcb_message() with the node's alias/ID,
     *    a zero destination alias, NULL_NODE_ID destination, and MTI_INITIALIZATION_COMPLETE (0x0100).
     * -# Call OpenLcbUtilities_copy_node_id_to_openlcb_payload() to place the 6-byte Node ID
     *    at payload offset 0.
     * -# If the send callback is non-NULL, call it and return its result; otherwise return false.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the openlcb_node_t that has just finished login.
     * @endverbatim
     *
     * @return true if queued successfully, false if the transmit buffer is full or
     *         the callback is NULL.
     *
     * @warning NULL pointer on the node causes a crash — no NULL check is performed.
     * @warning Returns false if OpenLcbApplication_initialize() was not called first.
     * @warning Per the OpenLCB Message Network Standard this message must be sent before
     *          any other OpenLCB messages after alias negotiation completes.
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
     * @brief Reads bytes from the node's configuration memory via the application callback.
     *
     * @details Algorithm:
     * -# If config_memory_read callback is non-NULL, call it with all parameters and return the result.
     * -# Otherwise return 0xFFFF.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the requesting openlcb_node_t.
     * @param address       Starting address within the configuration address space.
     * @param count         Number of bytes to read.
     * @param buffer        Destination configuration_memory_buffer_t.
     * @endverbatim
     *
     * @return Number of bytes read, or 0xFFFF if the callback is NULL or the read failed.
     *
     * @warning NULL pointer on the node is passed directly to the callback unchecked.
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
     * @brief Writes bytes to the node's configuration memory via the application callback.
     *
     * @details Algorithm:
     * -# If config_memory_write callback is non-NULL, call it with all parameters and return the result.
     * -# Otherwise return 0xFFFF.
     *
     * @verbatim
     * @param openlcb_node  Pointer to the requesting openlcb_node_t.
     * @param address       Starting address within the configuration address space.
     * @param count         Number of bytes to write.
     * @param buffer        Source configuration_memory_buffer_t.
     * @endverbatim
     *
     * @return Number of bytes written, or 0xFFFF if the callback is NULL or the write failed.
     *
     * @warning NULL pointer on the node is passed directly to the callback unchecked.
     * @warning Writes to read-only address spaces are rejected by the application callback, not here.
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
