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
 * - Configuration memory read/write abstrain
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

    /// Initialization  
    extern void OpenLcbApplication_initialize(const interface_openlcb_application_t *interface_openlcb_application);

    // Event Registration
    extern void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t *openlcb_node);
    extern void OpenLcbApplication_clear_producer_eventids(openlcb_node_t *openlcb_node);
    extern uint16_t OpenLcbApplication_register_consumer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status);
    extern uint16_t OpenLcbApplication_register_producer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status);

    // Event Range Registration 
    extern void OpenLcbApplication_clear_consumer_ranges(openlcb_node_t *openlcb_node);
    extern void OpenLcbApplication_clear_producer_ranges(openlcb_node_t *openlcb_node);
    extern bool OpenLcbApplication_register_consumer_range(openlcb_node_t *openlcb_node, event_id_t event_id_base, event_range_count_enum range_size);
    extern bool OpenLcbApplication_register_producer_range(openlcb_node_t *openlcb_node, event_id_t event_id_base, event_range_count_enum range_size);

    // Event Transmission
    extern bool OpenLcbApplication_send_event_pc_report(openlcb_node_t *openlcb_node, event_id_t event_id);
    extern bool OpenLcbApplication_send_event_with_mti(openlcb_node_t *openlcb_node, event_id_t event_id, uint16_t mti);
    extern bool OpenLcbApplication_send_teach_event(openlcb_node_t *openlcb_node, event_id_t event_id);
    extern bool OpenLcbApplication_send_initialization_event(openlcb_node_t *openlcb_node);

    // Configuration Memory Access
    extern uint16_t OpenLcbApplication_read_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);
    extern uint16_t OpenLcbApplication_write_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_APPLICATION__ */
