/*
 * File:   application.c
 * Author: jimkueneman
 *
 * Created on January 16, 2025, 12:00 PM
 */

#include "application.h"

#include "openlcb_types.h"
#include "../drivers/driver_configuration_memory.h"
#include "openlcb_utilities.h"
#include "openlcb_tx_driver.h"

void Application_clear_consumer_eventids(openlcb_node_t *node) {

    if (node)
        node->consumers.count = 0;
}

void Application_clear_producer_eventids(openlcb_node_t *node) {

    if (node)
        node->producers.count = 0;
}

uint16_olcb_t Application_register_consumer_eventid(openlcb_node_t *node, event_id_t eventid) {

    if (node) {

        if (node->consumers.count + 1 < USER_DEFINED_CONSUMER_COUNT) {
            node->consumers.list[node->consumers.count] = eventid;
            node->consumers.count = node->consumers.count + 1;

            return (node->consumers.count);
        }
    }

    return 0xFFFF;
}

uint16_olcb_t Application_register_producer_eventid(openlcb_node_t *node, event_id_t eventid) {

    if (node) {

        if (node->producers.count + 1 < USER_DEFINED_PRODUCER_COUNT) {

            node->producers.list[node->producers.count] = eventid;
            node->producers.count = node->producers.count + 1;

            return (node->producers.count);
        }
    }

    return 0xFFFF;
}

uint8_olcb_t Application_send_event_pc_report(openlcb_node_t *node, event_id_t eventid) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) & payload;
    msg.payload_type = BASIC;

    Utilities_load_openlcb_message(&msg, node->alias, node->id, 0, NULL_NODE_ID, MTI_PC_EVENT_REPORT, 0);

    Utilities_copy_event_id_to_openlcb_payload(&msg, eventid);

    if (OpenLcbTxDriver_try_transmit(node, &msg)) {

        return TRUE;

    }

    return FALSE;
}

uint8_olcb_t Application_send_teach_event(openlcb_node_t* node, event_id_t eventid) {

    openlcb_msg_t msg;
    payload_basic_t payload;

    msg.payload = (openlcb_payload_t *) & payload;
    msg.payload_type = BASIC;

    Utilities_load_openlcb_message(&msg, node->alias, node->id, 0, NULL_NODE_ID, MTI_EVENT_LEARN, 0);

    Utilities_copy_event_id_to_openlcb_payload(&msg, eventid);

    if (OpenLcbTxDriver_try_transmit(node, &msg)) {

        return TRUE;

    }

    return FALSE;

}

uint16_olcb_t Application_read_configuration_memory(openlcb_node_t *node, uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer) {

    configuration_mem_callback_t _getmem_callback = DriverConfigurationMemory_get_read_callback();

    if (_getmem_callback)
        return (_getmem_callback(Utilities_calculate_memory_offset_into_node_space(node) + address, count, buffer));

    return FALSE;
}

uint16_olcb_t Application_write_configuration_memory(openlcb_node_t *node, uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer) {

    configuration_mem_callback_t _getmem_callback = DriverConfigurationMemory_get_write_callback();

    if (_getmem_callback)
        return (_getmem_callback(Utilities_calculate_memory_offset_into_node_space(node) + address, count, buffer));

    return FALSE;
}