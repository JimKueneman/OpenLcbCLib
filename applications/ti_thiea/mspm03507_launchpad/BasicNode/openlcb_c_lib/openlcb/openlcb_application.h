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
 * @brief Application layer interface for the OpenLCB library.
 *
 * @details Application layer interface for the OpenLCB protocol stack.  Provides
 * functions for producer/consumer event registration, event transmission, and
 * configuration memory access.  Fill in a @ref interface_openlcb_application_t
 * with valid callback pointers and pass it to OpenLcbApplication_initialize()
 * before making any other call in this module.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_APPLICATION__
#define __OPENLCB_OPENLCB_APPLICATION__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Application-provided callbacks required by the OpenLCB library.
     *
     * @details All three function pointers must be set before calling
     * OpenLcbApplication_initialize().  The library calls these to send messages
     * and to read or write configuration memory.  The structure must remain valid
     * for the entire lifetime of the program.
     *
     * @warning All three function pointers must be non-NULL before initialization.
     *
     * @see OpenLcbApplication_initialize
     */
typedef struct {

        /**
         * @brief Queues an outgoing OpenLCB message for transmission.
         *
         * @details The implementation must queue the message for the transport layer
         * (CAN, TCP/IP, etc.) and return immediately without blocking.
         *
         * @param openlcb_msg  Pointer to the @ref openlcb_msg_t to transmit.
         *
         * @return true if queued successfully, false if the transmit buffer is full.
         *
         * @warning NULL pointer causes a crash — this field must always be set.
         */
    bool (*send_openlcb_msg)(openlcb_msg_t *openlcb_msg);

        /**
         * @brief Reads bytes from the node's configuration memory.
         *
         * @details Called when the library needs to read from the node's persistent
         * storage (CDI, ACDI, SNIP data, user-configurable parameters).  The callback
         * is responsible for address validation and bounds checking.
         *
         * @param openlcb_node  Pointer to the requesting @ref openlcb_node_t.
         * @param address       Starting address within the configuration address space.
         * @param count         Number of bytes to read (64 or fewer for network operations).
         * @param buffer        Destination @ref configuration_memory_buffer_t (must hold at least count bytes).
         *
         * @return Number of bytes read, or 0xFFFF on error.
         *
         * @warning NULL pointer causes a crash — this field must always be set.
         * @warning The buffer must have room for at least count bytes.
         */
    uint16_t (*config_memory_read)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

        /**
         * @brief Writes bytes to the node's configuration memory.
         *
         * @details Called when the library needs to write to the node's persistent
         * storage.  Read-only spaces (CDI, ACDI, manufacturer data) should return 0xFFFF.
         * The application is responsible for the persistence mechanism (EEPROM, flash,
         * etc.) and for address validation.
         *
         * @param openlcb_node  Pointer to the requesting @ref openlcb_node_t.
         * @param address       Starting address within the configuration address space.
         * @param count         Number of bytes to write (64 or fewer for network operations).
         * @param buffer        Source @ref configuration_memory_buffer_t containing the data.
         *
         * @return Number of bytes written, or 0xFFFF on error.
         *
         * @warning NULL pointer causes a crash — this field must always be set.
         * @warning Read-only address spaces must return 0xFFFF, not silently succeed.
         */
    uint16_t (*config_memory_write)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

} interface_openlcb_application_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the application callback interface for use by all application layer functions.
         *
         * @details Must be called exactly once during startup, before any other call in this
         * module.  The pointed-to structure must remain valid for the lifetime of the program.
         *
         * @param interface_openlcb_application  Pointer to a fully initialised @ref interface_openlcb_application_t.
         *
         * @warning All three function pointers in the structure must be non-NULL.
         * @warning Calling more than once replaces the stored pointer and loses the previous context.
         */
    extern void OpenLcbApplication_initialize(const interface_openlcb_application_t *interface_openlcb_application);

        /**
         * @brief Clears the consumer event list for a node by resetting its count to zero.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t to clear.
         *
         * @warning NULL pointer causes a crash — no NULL check is performed.
         */
    extern void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t *openlcb_node);

        /**
         * @brief Clears the producer event list for a node by resetting its count to zero.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t to clear.
         *
         * @warning NULL pointer causes a crash — no NULL check is performed.
         */
    extern void OpenLcbApplication_clear_producer_eventids(openlcb_node_t *openlcb_node);

        /**
         * @brief Adds a consumer event ID to the node's consumer list.
         *
         * @details Stores the event and its initial status at the next available slot
         * in the fixed-size consumer array.  Returns the 0-based array index on success
         * so the caller can reference the entry directly later.
         *
         * @param openlcb_node   Pointer to the @ref openlcb_node_t receiving the registration.
         * @param event_id       64-bit @ref event_id_t to register (8 bytes, MSB first).
         * @param event_status   Initial status: one of the @ref event_status_enum values.
         *
         * @return 0-based index of the newly registered entry, or 0xFFFF if the array is full.
         *
         * @warning NULL pointer causes a crash — no NULL check is performed on the node.
         * @warning Check the return value for 0xFFFF before assuming registration succeeded.
         */
    extern uint16_t OpenLcbApplication_register_consumer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status);

        /**
         * @brief Adds a producer event ID to the node's producer list.
         *
         * @details Stores the event and its initial status at the next available slot
         * in the fixed-size producer array.  Returns the 0-based array index on success
         * so the caller can reference the entry directly later.
         *
         * @param openlcb_node   Pointer to the @ref openlcb_node_t receiving the registration.
         * @param event_id       64-bit @ref event_id_t to register (8 bytes, MSB first).
         * @param event_status   Initial status: one of the @ref event_status_enum values.
         *
         * @return 0-based index of the newly registered entry, or 0xFFFF if the array is full.
         *
         * @warning NULL pointer causes a crash — no NULL check is performed on the node.
         * @warning Check the return value for 0xFFFF before assuming registration succeeded.
         */
    extern uint16_t OpenLcbApplication_register_producer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status);

        /**
         * @brief Clears the consumer event-range list for a node by resetting its range count to zero.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t to clear.
         *
         * @warning NULL pointer causes a crash — no NULL check is performed.
         */
    extern void OpenLcbApplication_clear_consumer_ranges(openlcb_node_t *openlcb_node);

        /**
         * @brief Clears the producer event-range list for a node by resetting its range count to zero.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t to clear.
         *
         * @warning NULL pointer causes a crash — no NULL check is performed.
         */
    extern void OpenLcbApplication_clear_producer_ranges(openlcb_node_t *openlcb_node);

        /**
         * @brief Registers an event ID range in the node's consumer range list.
         *
         * @details Stores the base event ID and range size at the next available slot
         * in the fixed-size consumer range array.
         *
         * @param openlcb_node    Pointer to the @ref openlcb_node_t receiving the registration.
         * @param event_id_base   Base @ref event_id_t of the range.
         * @param range_size      Number of consecutive event IDs in the range (@ref event_range_count_enum).
         *
         * @return true if registered successfully, false if the range array is full.
         *
         * @warning NULL pointer causes a crash — no NULL check is performed on the node.
         */
    extern bool OpenLcbApplication_register_consumer_range(openlcb_node_t *openlcb_node, event_id_t event_id_base, event_range_count_enum range_size);

        /**
         * @brief Registers an event ID range in the node's producer range list.
         *
         * @details Stores the base event ID and range size at the next available slot
         * in the fixed-size producer range array.
         *
         * @param openlcb_node    Pointer to the @ref openlcb_node_t receiving the registration.
         * @param event_id_base   Base @ref event_id_t of the range.
         * @param range_size      Number of consecutive event IDs in the range (@ref event_range_count_enum).
         *
         * @return true if registered successfully, false if the range array is full.
         *
         * @warning NULL pointer causes a crash — no NULL check is performed on the node.
         */
    extern bool OpenLcbApplication_register_producer_range(openlcb_node_t *openlcb_node, event_id_t event_id_base, event_range_count_enum range_size);

        /**
         * @brief Sends a Producer/Consumer Event Report (PCER) message to the network.
         *
         * @details Builds a global PCER message (MTI 0x05B4) with the given event ID and
         * queues it via the send_openlcb_msg callback.  Returns immediately; actual
         * transmission happens when the transport layer drains its queue.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param event_id      64-bit @ref event_id_t to report.
         *
         * @return true if queued successfully, false if the transmit buffer is full or
         *         the callback is NULL.
         *
         * @warning NULL pointer causes a crash on the node — no NULL check is performed.
         * @warning Returns false if OpenLcbApplication_initialize() was not called first.
         */
    extern bool OpenLcbApplication_send_event_pc_report(openlcb_node_t *openlcb_node, event_id_t event_id);

        /**
         * @brief Sends an event message with an arbitrary MTI.
         *
         * @details Builds a global OpenLCB message carrying the given event ID and MTI,
         * then queues it via the send_openlcb_msg callback.  Useful for sending event
         * MTIs that do not have a dedicated helper function.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param event_id      64-bit @ref event_id_t to include in the payload.
         * @param mti           16-bit MTI value for the message.
         *
         * @return true if queued successfully, false if the transmit buffer is full or
         *         the callback is NULL.
         *
         * @warning NULL pointer causes a crash on the node — no NULL check is performed.
         */
    extern bool OpenLcbApplication_send_event_with_mti(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t mti);

        /**
         * @brief Sends a Learn Event (teach) message to the network.
         *
         * @details Builds a global Learn Event message (MTI 0x0594) with the given event
         * ID and queues it via the send_openlcb_msg callback.  Used during teach/learn
         * configuration to inform other nodes of a new event association.
         *
         * @param openlcb_node  Pointer to the sending @ref openlcb_node_t.
         * @param event_id      64-bit @ref event_id_t to teach.
         *
         * @return true if queued successfully, false if the transmit buffer is full or
         *         the callback is NULL.
         *
         * @warning NULL pointer causes a crash on the node — no NULL check is performed.
         * @warning Returns false if OpenLcbApplication_initialize() was not called first.
         */
    extern bool OpenLcbApplication_send_teach_event(openlcb_node_t *openlcb_node, event_id_t event_id);

        /**
         * @brief Sends an Initialization Complete message for the specified node.
         *
         * @details Builds a global Initialization Complete message (MTI 0x0100) whose
         * 6-byte payload carries the node's full 48-bit Node ID, then queues it via
         * the send_openlcb_msg callback.  Per the OpenLCB Message Network Standard this
         * message must be sent after alias negotiation completes and before any other
         * OpenLCB messages.
         *
         * @param openlcb_node  Pointer to the @ref openlcb_node_t that has just finished login.
         *
         * @return true if queued successfully, false if the transmit buffer is full or
         *         the callback is NULL.
         *
         * @warning NULL pointer causes a crash on the node — no NULL check is performed.
         * @warning Returns false if OpenLcbApplication_initialize() was not called first.
         */
    extern bool OpenLcbApplication_send_initialization_event(openlcb_node_t *openlcb_node);

        /**
         * @brief Reads bytes from the node's configuration memory via the application callback.
         *
         * @details Passes the request through to the config_memory_read callback.  Returns
         * 0xFFFF immediately if the callback pointer is NULL.
         *
         * @param openlcb_node  Pointer to the requesting @ref openlcb_node_t.
         * @param address       Starting address within the configuration address space.
         * @param count         Number of bytes to read.
         * @param buffer        Destination @ref configuration_memory_buffer_t.
         *
         * @return Number of bytes read, or 0xFFFF if the callback is NULL or the read failed.
         *
         * @warning NULL pointer on the node is passed directly to the callback unchecked.
         */
    extern uint16_t OpenLcbApplication_read_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

        /**
         * @brief Writes bytes to the node's configuration memory via the application callback.
         *
         * @details Passes the request through to the config_memory_write callback.  Returns
         * 0xFFFF immediately if the callback pointer is NULL.
         *
         * @param openlcb_node  Pointer to the requesting @ref openlcb_node_t.
         * @param address       Starting address within the configuration address space.
         * @param count         Number of bytes to write.
         * @param buffer        Source @ref configuration_memory_buffer_t.
         *
         * @return Number of bytes written, or 0xFFFF if the callback is NULL or the write failed.
         *
         * @warning NULL pointer on the node is passed directly to the callback unchecked.
         * @warning Writes to read-only address spaces are rejected by the application callback, not here.
         */
    extern uint16_t OpenLcbApplication_write_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_APPLICATION__ */
