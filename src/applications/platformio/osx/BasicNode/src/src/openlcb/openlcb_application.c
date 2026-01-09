



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

void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t *openlcb_node) {

    openlcb_node->consumers.count = 0;

}

void OpenLcbApplication_clear_producer_eventids(openlcb_node_t *openlcb_node) {

    openlcb_node->producers.count = 0;

}

uint16_t OpenLcbApplication_register_consumer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status) {

    if (openlcb_node->consumers.count + 1 < USER_DEFINED_CONSUMER_COUNT) {
        
        openlcb_node->consumers.list[openlcb_node->consumers.count].event = event_id;
        openlcb_node->consumers.list[openlcb_node->consumers.count].status = event_status;
        openlcb_node->consumers.count = openlcb_node->consumers.count + 1;

        return (openlcb_node->consumers.count);
    }
    
    return 0xFFFF;

}

uint16_t OpenLcbApplication_register_producer_eventid(openlcb_node_t *openlcb_node, event_id_t event_id, event_status_enum event_status) {

    if (openlcb_node->producers.count + 1 < USER_DEFINED_PRODUCER_COUNT) {

        openlcb_node->producers.list[openlcb_node->producers.count].event = event_id;
        openlcb_node->producers.list[openlcb_node->producers.count].status = event_status;
        openlcb_node->producers.count = openlcb_node->producers.count + 1;

        return (openlcb_node->producers.count);
    }

    return 0xFFFF;
}

bool OpenLcbApplication_send_event_pc_report(openlcb_node_t *openlcb_node, event_id_t event_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) & payload;
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

    if (_interface->send_openlcb_msg(&msg)) {

        return true;

    }

    return false;
}

bool OpenLcbApplication_send_teach_event(openlcb_node_t* openlcb_node, event_id_t event_id) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) & payload;
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

    if (_interface->send_openlcb_msg(&msg)) {

        return true;

    }

    return false;

}

bool OpenLcbApplication_send_initialization_event(openlcb_node_t* openlcb_node) {
    
    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) & payload;
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

    if (_interface->send_openlcb_msg(&msg)) {

        return true;

    }

    return false;
    
}

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
