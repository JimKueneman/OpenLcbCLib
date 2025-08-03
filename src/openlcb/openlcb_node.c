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
 * \file openlcb_node.c
 *
 * Implementation of the OpenLcb node structures and buffers with functions to manipulate them
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "openlcb_node.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_defines.h"

openlcb_nodes_t openlcb_nodes;

uint16_olcb_t node_enum_index_array[6];

void _clear_node(openlcb_node_t* openlcb_node) {

    openlcb_node->alias = 0;
    openlcb_node->id = 0;
    openlcb_node->seed = 0;
    openlcb_node->state.run_state = RUNSTATE_INIT;
    openlcb_node->state.allocated = false;
    openlcb_node->state.duplicate_id_detected = false;
    openlcb_node->state.initial_events_broadcast_complete = false;
    openlcb_node->state.initalized = false;
    openlcb_node->state.permitted = false;
    openlcb_node->state.can_msg_handled = false;
    openlcb_node->state.openlcb_msg_handled = false;
    openlcb_node->state.openlcb_datagram_ack_sent = false;
    openlcb_node->state.resend_datagram = false;
    openlcb_node->state.resend_optional_message = false;
    openlcb_node->state.firmware_upgrade_active = false;
    openlcb_node->timerticks = 0;
    openlcb_node->lock_node = 0;
    openlcb_node->index = 0;

    openlcb_node->last_received_datagram = NULL;
    openlcb_node->last_received_optional_interaction = NULL;

    openlcb_node->consumers.count = 0;
    for (int_olcb_t i = 0; i < USER_DEFINED_CONSUMER_COUNT; i++) {

        openlcb_node->consumers.list[i] = 0;

    }


    openlcb_node->producers.count = 0;
    for (int_olcb_t i = 0; i < USER_DEFINED_PRODUCER_COUNT; i++) {

        openlcb_node->producers.list[i] = 0;

    }


    openlcb_node->producers.enumerator.running = false;
    openlcb_node->consumers.enumerator.running = false;

    for (int_olcb_t i = 0; i < sizeof (openlcb_node->producers.event_state_array); i++) {// Unknown State

        openlcb_node->producers.event_state_array[i] = 0;

    }

    for (int_olcb_t i = 0; i < sizeof (openlcb_node->consumers.event_state_array); i++) {

        openlcb_node->consumers.event_state_array[i] = 0;

    }

}

void Node_initialize(void) {

    for (int_olcb_t i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        _clear_node(&openlcb_nodes.node[i]);

    }

    openlcb_nodes.count = 0;


}

openlcb_node_t* Node_get_first(uint8_olcb_t key) {

    node_enum_index_array[key] = 0;


    if (openlcb_nodes.count == 0) {
        
        return NULL;
        
    }

    return (&openlcb_nodes.node[ node_enum_index_array[key] ]);

}

openlcb_node_t* Node_get_next(uint8_olcb_t key) {

    node_enum_index_array[key] = node_enum_index_array[key] + 1;

    if (node_enum_index_array[key] >= openlcb_nodes.count) {
        
        return NULL;
        
    }

    return (&openlcb_nodes.node[node_enum_index_array[key]]);

}

void _generate_event_ids(openlcb_node_t* openlcb_node) {

    uint64_olcb_t node_id = openlcb_node->id << 16;
    uint16_olcb_t indexer = 0;

    openlcb_node->consumers.count = 0;
    for (int_olcb_t i = 0; i < openlcb_node->parameters->consumer_count_autocreate; i++) {

        if (i < USER_DEFINED_CONSUMER_COUNT) { // safety net

            openlcb_node->consumers.list[i] = node_id + indexer;
            openlcb_node->consumers.count = openlcb_node->consumers.count + 1;

        }
        
        indexer++;

    }

    indexer = 0;
    openlcb_node->producers.count = 0;
    for (int_olcb_t i = 0; i < openlcb_node->parameters->producer_count_autocreate; i++) {

        if (i < USER_DEFINED_PRODUCER_COUNT) { // safety net

            openlcb_node->producers.list[i] = node_id + indexer;
            openlcb_node->producers.count = openlcb_node->producers.count + 1;

        }
        
        indexer++;

    }


    openlcb_node->consumers.enumerator.running = false;
    openlcb_node->consumers.enumerator.enum_index = 0;

    openlcb_node->producers.enumerator.running = false;
    openlcb_node->producers.enumerator.enum_index = 0;



}

openlcb_node_t* Node_allocate(uint64_olcb_t node_id, const node_parameters_t* node_parameters) {

    for (int_olcb_t i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        if (!openlcb_nodes.node[i].state.allocated) {

            _clear_node(&openlcb_nodes.node[i]);

            openlcb_nodes.node[i].parameters = node_parameters;
            openlcb_nodes.node[i].state.allocated = true;
            openlcb_nodes.node[i].id = node_id;
            openlcb_nodes.node[i].index = (uint8_olcb_t) i;

            _generate_event_ids(&openlcb_nodes.node[i]);

            openlcb_nodes.count = openlcb_nodes.count + 1;

            return &openlcb_nodes.node[i];

        }

    }

    return NULL;

}

openlcb_node_t* Node_find_by_alias(uint16_olcb_t alias) {

    for (int_olcb_t i = 0; i < openlcb_nodes.count; i++) {

        if (openlcb_nodes.node[i].state.permitted) {

            if (openlcb_nodes.node[i].alias == alias)

                return &openlcb_nodes.node[i];

        }

    };

    return NULL;

}

openlcb_node_t* Node_find_by_node_id(uint64_olcb_t nodeid) {

    for (int_olcb_t i = 0; i < openlcb_nodes.count; i++) {

        if (openlcb_nodes.node[i].state.permitted) {

            if (openlcb_nodes.node[i].id == nodeid)

                return &openlcb_nodes.node[i];

        }

    };

    return NULL;
}

uint64_olcb_t Node_generate_seed(uint64_olcb_t start_seed) {

    uint32_olcb_t lfsr1 = start_seed & 0xFFFFFF;
    uint32_olcb_t lfsr2 = (start_seed >> 24) & 0xFFFFFF;

    uint32_olcb_t temp1 = ((lfsr1 << 9) | ((lfsr2 >> 15) & 0x1FF)) & 0xFFFFFF;
    uint32_olcb_t temp2 = (lfsr2 << 9) & 0xFFFFFF;

    lfsr1 = lfsr1 + temp1 + 0x1B0CA3L;
    lfsr2 = lfsr2 + temp2 + 0x7A4BA9L;

    lfsr1 = (lfsr1 & 0xFFFFFF) + ((lfsr2 & 0xFF000000) >> 24);
    lfsr2 = lfsr2 & 0xFFFFFF;

    return ( (uint64_olcb_t) lfsr1 << 24) | lfsr2;

}

uint16_olcb_t Node_generate_alias(uint64_olcb_t seed) {

    uint32_olcb_t lfsr2 = seed & 0xFFFFFF;
    uint32_olcb_t lfsr1 = (seed >> 24) & 0xFFFFFF;

    return ( lfsr1 ^ lfsr2 ^ (lfsr1 >> 12) ^ (lfsr2 >> 12)) & 0x0FFF;

}

void Node_100ms_timer_tick(void) {

    for (int_olcb_t i = 0; i < openlcb_nodes.count; i++) {

        openlcb_nodes.node[i].timerticks = openlcb_nodes.node[i].timerticks + 1;

    };

}
