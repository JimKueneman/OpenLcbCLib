/*
 * File:   openlcb_node.c
 * Author: jimkueneman
 *
 * Created on November 20, 2024, 4:46 PM
 */


#include "xc.h"

#include "openlcb_types.h"
#include "openlcb_defines.h"

openlcb_nodes_t openlcb_nodes;

uint16_t node_enum_index_array[6];


void _clear_node(openlcb_node_t* openlcb_node) {

    openlcb_node->alias = 0;
    openlcb_node->id = 0;
    openlcb_node->seed = 0;
    openlcb_node->state.run_state = RUNSTATE_INIT;
    openlcb_node->state.allocated = FALSE;
    openlcb_node->state.duplicate_id_detected = FALSE;
    openlcb_node->state.initalized = FALSE;
    openlcb_node->state.permitted = FALSE;
    openlcb_node->state.can_msg_handled = FALSE;
    openlcb_node->state.openlcb_msg_handled = FALSE;
    openlcb_node->timerticks = 0;


    for (int i = 0; i < LEN_CONSUMER_MAX_COUNT; i++)
        openlcb_node->consumers.list[i] = 0;

    for (int i = 0; i < LEN_PRODUCER_MAX_COUNT; i++)
        openlcb_node->producers.list[i] = 0;


    openlcb_node->producers.enumerator.running = FALSE;
    openlcb_node->consumers.enumerator.running = FALSE;

    openlcb_node->producers.enumerator.flag = FALSE;
    openlcb_node->consumers.enumerator.flag = FALSE;

    openlcb_node->producers.event_state = 0x00; // Unknown State
    openlcb_node->consumers.event_state = 0x00;

}

void Node_initialize() {

    for (int i = 0; i < LEN_NODE_ARRAY; i++)

        _clear_node(&openlcb_nodes.node[i]);

    openlcb_nodes.count = 0;


}

openlcb_node_t* Node_get_first(uint8_t key) {

    node_enum_index_array[key] = 0;


    if (openlcb_nodes.count == 0)
        return (void*) 0;

    return (&openlcb_nodes.node[ node_enum_index_array[key] ]);

}

openlcb_node_t* Node_get_next(uint8_t key) {

    node_enum_index_array[key] = node_enum_index_array[key] + 1;

    if (node_enum_index_array[key] >= openlcb_nodes.count)
        return (void*) 0;

    return (&openlcb_nodes.node[node_enum_index_array[key]]);

}

void _generate_event_ids(openlcb_node_t* openlcb_node) {

    uint64_t node_id = openlcb_node->id << 16;

    for (int i = 0; i < openlcb_node->parameters->consumer_count; i++)

        if (i < LEN_CONSUMER_MAX_COUNT)

            openlcb_node->consumers.list[i] = node_id + i;

    for (int i = 0; i < openlcb_node->parameters->producer_count; i++)

        if (i < LEN_PRODUCER_MAX_COUNT)

            openlcb_node->producers.list[i] = node_id + i;


    openlcb_node->consumers.enumerator.running = FALSE;
    openlcb_node->producers.enumerator.flag = FALSE;


}

openlcb_node_t* Node_allocate(uint64_t node_id, const node_parameters_t* node_parameters) {

    for (int i = 0; i < LEN_NODE_ARRAY; i++) {

        if (!openlcb_nodes.node[i].state.allocated) {

            _clear_node(&openlcb_nodes.node[i]);

            openlcb_nodes.node[i].parameters = node_parameters;
            openlcb_nodes.node[i].state.allocated = TRUE;
            openlcb_nodes.node[i].id = node_id;

            _generate_event_ids(&openlcb_nodes.node[i]);

            openlcb_nodes.count = openlcb_nodes.count + 1;

            return &openlcb_nodes.node[i];

        }

    }

    return (void*) 0;

}

openlcb_node_t* Node_find_by_alias(uint16_t alias) {

    for (int i = 0; i < openlcb_nodes.count; i++) {

        if (openlcb_nodes.node[i].state.permitted) {

            if (openlcb_nodes.node[i].alias == alias)

                return &openlcb_nodes.node[i];

        }

    };

    return (void*) 0;

}

openlcb_node_t* Node_find_by_node_id(uint64_t nodeid) {

    for (int i = 0; i < openlcb_nodes.count; i++) {

        if (openlcb_nodes.node[i].state.permitted) {

            if (openlcb_nodes.node[i].id == nodeid)

                return &openlcb_nodes.node[i];

        }

    };

    return (void*) 0;
}

uint64_t Node_generate_seed(uint64_t start_seed) {

    uint32_t lfsr1 = start_seed & 0xFFFFFF;
    uint32_t lfsr2 = (start_seed >> 24) & 0xFFFFFF;

    uint32_t temp1 = ((lfsr1 << 9) | ((lfsr2 >> 15) & 0x1FF)) & 0xFFFFFF;
    uint32_t temp2 = (lfsr2 << 9) & 0xFFFFFF;

    lfsr1 = lfsr1 + temp1 + 0x1B0CA3L;
    lfsr2 = lfsr2 + temp2 + 0x7A4BA9L;

    lfsr1 = (lfsr1 & 0xFFFFFF) + ((lfsr2 & 0xFF000000) >> 24);
    lfsr2 = lfsr2 & 0xFFFFFF;

    return ( (uint64_t) lfsr1 << 24) | lfsr2;

}

uint16_t Node_generate_alias(uint64_t seed) {

    uint32_t lfsr2 = seed & 0xFFFFFF;
    uint32_t lfsr1 = (seed >> 24) & 0xFFFFFF;

    return ( lfsr1 ^ lfsr2 ^ (lfsr1 >> 12) ^ (lfsr2 >> 12)) & 0x0FFF;

}

void Node_100ms_timer_tick() {

    for (int i = 0; i < openlcb_nodes.count; i++) {

        openlcb_nodes.node[i].timerticks = openlcb_nodes.node[i].timerticks + 1;

    };

}

int Node_is_event_id_producer(openlcb_node_t* openlcb_node, uint64_t event_id) {

    for (int i = 0; i < openlcb_node->parameters->producer_count; i++) {

        if (i < LEN_PRODUCER_MAX_COUNT)

            if (openlcb_node->producers.list[i] == event_id)

                return i;

    }

    return -1;

}

int Node_is_event_id_consumer(openlcb_node_t* openlcb_node, uint64_t event_id) {


    for (int i = 0; i < openlcb_node->parameters->consumer_count; i++) {

        if (i < LEN_CONSUMER_MAX_COUNT)

            if (openlcb_node->consumers.list[i] == event_id)

                return i;


    }
    return -1;
}

uint8_t Node_is_addressed_to(openlcb_msg_t* msg, openlcb_node_t* node) {

    for (int i = 0; i < openlcb_nodes.count; i++) {

        if (node->alias == msg->dest_alias)

            return TRUE;

    }

    return FALSE;

}