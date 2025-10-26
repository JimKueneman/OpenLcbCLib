/*
 * File:   application.c
 * Author: jimkueneman
 *
 * Created on January 16, 2025, 12:00 PM
 */

#include "openlcb_application.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"

static interface_openlcb_application_t *_interface;

void OpenLcbApplication_initialize(const interface_openlcb_application_t *interface_openlcb_application) {
    
    _interface = (interface_openlcb_application_t*) interface_openlcb_application;
    
}

void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t *node) {

    if (node) {
        node->consumers.count = 0;

    }

}

void OpenLcbApplication_clear_producer_eventids(openlcb_node_t *node) {

    if (node) {

        node->producers.count = 0;

    }

}

uint16_t OpenLcbApplication_register_consumer_eventid(openlcb_node_t *node, event_id_t event_id, event_status_enum event_status) {

    if (node) {

        if (node->consumers.count + 1 < USER_DEFINED_CONSUMER_COUNT) {
            node->consumers.list[node->consumers.count].event = event_id;
            node->consumers.list[node->consumers.count].status = event_status;
            node->consumers.count = node->consumers.count + 1;

            return (node->consumers.count);
        }
    }

    return 0xFFFF;
}

uint16_t OpenLcbApplication_register_producer_eventid(openlcb_node_t *node, event_id_t event_id, event_status_enum event_status) {

    if (node) {

        if (node->producers.count + 1 < USER_DEFINED_PRODUCER_COUNT) {

            node->producers.list[node->producers.count].event = event_id;
            node->producers.list[node->producers.count].status = event_status;
            node->producers.count = node->producers.count + 1;

            return (node->producers.count);
        }
    }

    return 0xFFFF;
}

bool OpenLcbApplication_send_event_pc_report(openlcb_node_t *node, event_id_t event_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) & payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &msg, 
            node->alias, 
            node->id, 
            0, 
            NULL_NODE_ID,
            MTI_PC_EVENT_REPORT, 
            0);

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            &msg, 
            event_id);
    
    msg.payload_count = 8;

    if (_interface->transmit_openlcb_message(&msg)) {

        return true;

    }

    return false;
}

bool OpenLcbApplication_send_teach_event(openlcb_node_t* node, event_id_t event_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) & payload;
    msg.payload_type = BASIC;

    OpenLcbUtilities_load_openlcb_message(
            &msg, 
            node->alias, 
            node->id,
            0, 
            NULL_NODE_ID,
            MTI_EVENT_LEARN, 
            0);

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            &msg, 
            event_id);
    
    msg.payload_count = 8;

    if (_interface->transmit_openlcb_message(&msg)) {

        return true;

    }

    return false;

}

uint16_t OpenLcbApplication_read_configuration_memory(openlcb_node_t *node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

    if (_interface->configuration_memory_read) {

        return (_interface->configuration_memory_read(OpenLcbUtilities_calculate_memory_offset_into_node_space(node) + address, count, buffer));

    }

    return false;
}

uint16_t OpenLcbApplication_write_configuration_memory(openlcb_node_t *node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

    if (_interface->configuration_memory_write) {

        return (_interface->configuration_memory_write(OpenLcbUtilities_calculate_memory_offset_into_node_space(node) + address, count, buffer));

    }

    return false;
}