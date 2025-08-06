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
#include <stdint.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_defines.h"

#include "../drivers/driver_can.h"

static openlcb_nodes_t _openlcb_nodes;
static uint16_t _node_enum_index_array[6];

// Used by the interrupt state-machines to see if messages are for our nodes
// Makes it easier to deal with not having to block the interrupts on every access
// the node structures, this array is to be used by the incoming message interrupt only
static alias_mapping_t _alias_mapping[USER_DEFINED_NODE_BUFFER_DEPTH];

static void _clear_node(openlcb_node_t* openlcb_node) {

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
    for (int i = 0; i < USER_DEFINED_CONSUMER_COUNT; i++) {

        openlcb_node->consumers.list[i] = 0;

    }


    openlcb_node->producers.count = 0;
    for (int i = 0; i < USER_DEFINED_PRODUCER_COUNT; i++) {

        openlcb_node->producers.list[i] = 0;

    }


    openlcb_node->producers.enumerator.running = false;
    openlcb_node->consumers.enumerator.running = false;

    for (int i = 0; i < sizeof (openlcb_node->producers.event_state_array); i++) {// Unknown State

        openlcb_node->producers.event_state_array[i] = 0;

    }

    for (int i = 0; i < sizeof (openlcb_node->consumers.event_state_array); i++) {

        openlcb_node->consumers.event_state_array[i] = 0;

    }

}

void OpenLcbNode_initialize(void) {

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        _clear_node(&_openlcb_nodes.node[i]);
        _alias_mapping[i].alias = 0;
        _alias_mapping[i].node_id = 0;

    }

    _openlcb_nodes.count = 0;


}

openlcb_node_t* OpenLcbNode_get_first(uint8_t key) {

    _node_enum_index_array[key] = 0;


    if (_openlcb_nodes.count == 0) {

        return NULL;

    }

    return (&_openlcb_nodes.node[ _node_enum_index_array[key] ]);

}

openlcb_node_t* OpenLcbNode_get_next(uint8_t key) {

    _node_enum_index_array[key] = _node_enum_index_array[key] + 1;

    if (_node_enum_index_array[key] >= _openlcb_nodes.count) {

        return NULL;

    }

    return (&_openlcb_nodes.node[_node_enum_index_array[key]]);

}

static void _generate_event_ids(openlcb_node_t* openlcb_node) {

    uint64_t node_id = openlcb_node->id << 16;
    uint16_t indexer = 0;

    openlcb_node->consumers.count = 0;
    for (int i = 0; i < openlcb_node->parameters->consumer_count_autocreate; i++) {

        if (i < USER_DEFINED_CONSUMER_COUNT) { // safety net

            openlcb_node->consumers.list[i] = node_id + indexer;
            openlcb_node->consumers.count = openlcb_node->consumers.count + 1;

        }

        indexer++;

    }

    indexer = 0;
    openlcb_node->producers.count = 0;
    for (int i = 0; i < openlcb_node->parameters->producer_count_autocreate; i++) {

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

openlcb_node_t* OpenLcbNode_allocate(uint64_t node_id, const node_parameters_t* node_parameters) {

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        if (!_openlcb_nodes.node[i].state.allocated) {

            _clear_node(&_openlcb_nodes.node[i]);

            _openlcb_nodes.node[i].parameters = node_parameters;
            _openlcb_nodes.node[i].state.allocated = true;
            _openlcb_nodes.node[i].id = node_id;
            _openlcb_nodes.node[i].index = (uint8_t) i;

            _generate_event_ids(&_openlcb_nodes.node[i]);

            _openlcb_nodes.count = _openlcb_nodes.count + 1;

            return &_openlcb_nodes.node[i];

        }

    }

    return NULL;

}

openlcb_node_t* OpenLcbNode_find_by_alias(uint16_t alias) {

    for (int i = 0; i < _openlcb_nodes.count; i++) {

        if (_openlcb_nodes.node[i].state.permitted) {

            if (_openlcb_nodes.node[i].alias == alias) {

                return &_openlcb_nodes.node[i];

            }

        }

    };

    return NULL;

}

openlcb_node_t* OpenLcbNode_find_by_node_id(uint64_t nodeid) {

    for (int i = 0; i < _openlcb_nodes.count; i++) {

        if (_openlcb_nodes.node[i].state.permitted) {

            if (_openlcb_nodes.node[i].id == nodeid) {

                return &_openlcb_nodes.node[i];

            }

        }

    };

    return NULL;
}

uint64_t OpenLcbNode_generate_seed(uint64_t start_seed) {

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

uint16_t OpenLcbNode_generate_alias(uint64_t seed) {

    uint32_t lfsr2 = seed & 0xFFFFFF;
    uint32_t lfsr1 = (seed >> 24) & 0xFFFFFF;

    return ( lfsr1 ^ lfsr2 ^ (lfsr1 >> 12) ^ (lfsr2 >> 12)) & 0x0FFF;

}

void OpenLcbNode_100ms_timer_tick(void) {

    for (int i = 0; i < _openlcb_nodes.count; i++) {

        _openlcb_nodes.node[i].timerticks = _openlcb_nodes.node[i].timerticks + 1;

    };

}

void OpenLcbNode_set_alias_mapping(uint8_t index, node_id_t node_id, uint16_t alias) {

    if (index >= USER_DEFINED_NODE_BUFFER_DEPTH) {

        return;

    }

    DriverCan_pause_can_rx();
    _alias_mapping[index].alias = alias;
    _alias_mapping[index].node_id = node_id;
    DriverCan_resume_can_rx();

}

void OpenLcbNode_clear_alias_mapping(uint8_t index) {

    if (index >= USER_DEFINED_NODE_BUFFER_DEPTH) {

        return;

    }

    DriverCan_pause_can_rx();
    _alias_mapping[index].alias = 0;
    _alias_mapping[index].node_id = 0;
    DriverCan_resume_can_rx();

}

static alias_mapping_t *_find_mapping_by_alias(uint16_t alias) {

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        if (_alias_mapping[i].alias == alias) {
            
            return &_alias_mapping[i];
        }

    }
    
    return NULL;

}

static alias_mapping_t *_find_mapping_by_node_id(node_id_t node_id) {

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        if (_alias_mapping[i].node_id == node_id) {
            
            return &_alias_mapping[i];
            
        }

    }
    
    return NULL;

}

alias_mapping_t *OpenLcbNode_find_alias_mapping(node_id_t node_id, uint16_t alias) {

    if ((node_id) && (!alias)) {

        return _find_mapping_by_node_id(node_id);

    } else if ((!node_id) && (alias)) {

        return _find_mapping_by_alias(alias);

    }
    
    return NULL;
}
