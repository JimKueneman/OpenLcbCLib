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
 * @brief Application layer interface where most application code interfaces with the library
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_APPLICATION__
#define    __OPENLCB_OPENLCB_APPLICATION__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {

    // Required function assignments
    bool (*send_openlcb_msg)(openlcb_msg_t* openlcb_msg);
    uint16_t (*config_memory_read)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);
    uint16_t (*config_memory_write) (openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

} interface_openlcb_application_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLcb Application interface
     * @param interface_openlcb_application Pointer to the interface structure containing required callbacks
     * @return None
     */
    extern void OpenLcbApplication_initialize(const interface_openlcb_application_t *interface_openlcb_application);

    /**
     * @brief Clears all consumer event IDs registered for the specified node
     * @param openlcb_node Pointer to the OpenLcb node
     * @return None
     */
    extern void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t* openlcb_node);

    /**
     * @brief Clears all producer event IDs registered for the specified node
     * @param openlcb_node Pointer to the OpenLcb node
     * @return None
     */
    extern void OpenLcbApplication_clear_producer_eventids(openlcb_node_t* openlcb_node);

    /**
     * @brief Registers a consumer event ID with the specified node
     * @param openlcb_node Pointer to the OpenLcb node
     * @param event_id Event ID to register
     * @param event_status Initial status of the event (unknown, set, or clear)
     * @return Index of the registered event, or error code if registration fails
     */
    extern uint16_t OpenLcbApplication_register_consumer_eventid(openlcb_node_t* openlcb_node, event_id_t event_id, event_status_enum event_status);

    /**
     * @brief Registers a producer event ID with the specified node
     * @param openlcb_node Pointer to the OpenLcb node
     * @param event_id Event ID to register
     * @param event_status Initial status of the event (unknown, set, or clear)
     * @return Index of the registered event, or error code if registration fails
     */
    extern uint16_t OpenLcbApplication_register_producer_eventid(openlcb_node_t* openlcb_node, event_id_t event_id, event_status_enum event_status);

    /**
     * @brief Sends a Producer/Consumer event report message
     * @param openlcb_node Pointer to the OpenLcb node sending the report
     * @param event_id Event ID to report
     * @return True if the message was successfully queued, false otherwise
     */
    extern bool OpenLcbApplication_send_event_pc_report(openlcb_node_t* openlcb_node, event_id_t event_id);

    /**
     * @brief Sends a teach event message
     * @param openlcb_node Pointer to the OpenLcb node sending the teach event
     * @param event_id Event ID to teach
     * @return True if the message was successfully queued, false otherwise
     */
    extern bool OpenLcbApplication_send_teach_event(openlcb_node_t* openlcb_node, event_id_t event_id);

    /**
     * @brief Sends an initialization complete event for the specified node
     * @param openlcb_node Pointer to the OpenLcb node sending the initialization event
     * @return True if the message was successfully queued, false otherwise
     */
    extern bool OpenLcbApplication_send_initialization_event(openlcb_node_t* openlcb_node);

    /**
     * @brief Reads data from the node's configuration memory
     * @param openlcb_node Pointer to the OpenLcb node
     * @param address Starting address to read from
     * @param count Number of bytes to read
     * @param buffer Pointer to buffer to store the read data
     * @return Number of bytes successfully read
     */
    extern uint16_t OpenLcbApplication_read_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

    /**
     * @brief Writes data to the node's configuration memory
     * @param openlcb_node Pointer to the OpenLcb node
     * @param address Starting address to write to
     * @param count Number of bytes to write
     * @param buffer Pointer to buffer containing data to write
     * @return Number of bytes successfully written
     */
    extern uint16_t OpenLcbApplication_write_configuration_memory(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_OPENLCB_APPLICATION__ */
