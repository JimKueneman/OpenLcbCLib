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
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"

static openlcb_nodes_t _openlcb_nodes;
static uint8_t _node_enum_index_array[MAX_NODE_ENUM_KEY_VALUES];

static interface_openlcb_node_t *_interface;

static void _clear_node(openlcb_node_t *openlcb_node)
{

    openlcb_node->alias = 0;
    openlcb_node->id = 0;
    openlcb_node->seed = 0;
    openlcb_node->state.run_state = RUNSTATE_INIT;
    openlcb_node->state.allocated = false;
    openlcb_node->state.duplicate_id_detected = false;
    openlcb_node->state.initialized = false;
    openlcb_node->state.permitted = false;
    openlcb_node->state.openlcb_datagram_ack_sent = false;
    openlcb_node->state.resend_datagram = false;
    openlcb_node->state.firmware_upgrade_active = false;
    openlcb_node->timerticks = 0;
    openlcb_node->owner_node = 0;
    openlcb_node->index = 0;

    openlcb_node->last_received_datagram = NULL;

    openlcb_node->consumers.count = 0;
    for (int i = 0; i < USER_DEFINED_CONSUMER_COUNT; i++)
    {

        openlcb_node->consumers.list[i].event = 0;
        openlcb_node->consumers.list[i].status = EVENT_STATUS_UNKNOWN;
    }

    openlcb_node->producers.count = 0;
    for (int i = 0; i < USER_DEFINED_PRODUCER_COUNT; i++)
    {

        openlcb_node->producers.list[i].event = 0;
        openlcb_node->producers.list[i].status = EVENT_STATUS_UNKNOWN;
    }

    openlcb_node->producers.enumerator.running = false;
    openlcb_node->consumers.enumerator.running = false;

    for (int i = 0; i < MAX_NODE_ENUM_KEY_VALUES; i++)
    {

        _node_enum_index_array[i] = 0;
    }
}

void OpenLcbNode_initialize(const interface_openlcb_node_t *interface)
{

    _interface = (interface_openlcb_node_t *)interface;

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++)
    {

        _clear_node(&_openlcb_nodes.node[i]);
    }

    _openlcb_nodes.count = 0;
}

openlcb_node_t *OpenLcbNode_get_first(uint8_t key)
{

    if (key >= MAX_NODE_ENUM_KEY_VALUES)
    {

        return NULL;
    }

    _node_enum_index_array[key] = 0;

    if (_openlcb_nodes.count == 0)
    {

        return NULL;
    }

    return (&_openlcb_nodes.node[_node_enum_index_array[key]]);
}

openlcb_node_t *OpenLcbNode_get_next(uint8_t key)
{

    if (key >= MAX_NODE_ENUM_KEY_VALUES)
    {

        return NULL;
    }

    _node_enum_index_array[key] = _node_enum_index_array[key] + 1;

    if (_node_enum_index_array[key] >= _openlcb_nodes.count)
    {

        return NULL;
    }

    return (&_openlcb_nodes.node[_node_enum_index_array[key]]);
}

static void _generate_event_ids(openlcb_node_t *openlcb_node)
{

    uint64_t node_id = openlcb_node->id << 16;
    uint16_t indexer = 0;

    openlcb_node->consumers.count = 0;
    for (int i = 0; i < openlcb_node->parameters->consumer_count_autocreate; i++)
    {

        if (i < USER_DEFINED_CONSUMER_COUNT)
        { // safety net

            openlcb_node->consumers.list[i].event = node_id + indexer;
            openlcb_node->consumers.count = openlcb_node->consumers.count + 1;
        }

        indexer++;
    }

    indexer = 0;
    openlcb_node->producers.count = 0;
    for (int i = 0; i < openlcb_node->parameters->producer_count_autocreate; i++)
    {

        if (i < USER_DEFINED_PRODUCER_COUNT)
        { // safety net

            openlcb_node->producers.list[i].event = node_id + indexer;
            openlcb_node->producers.count = openlcb_node->producers.count + 1;
        }

        indexer++;
    }

    openlcb_node->consumers.enumerator.running = false;
    openlcb_node->consumers.enumerator.enum_index = 0;

    openlcb_node->producers.enumerator.running = false;
    openlcb_node->producers.enumerator.enum_index = 0;
}

openlcb_node_t *OpenLcbNode_allocate(uint64_t node_id, const node_parameters_t *node_parameters)
{

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++)
    {

        if (!_openlcb_nodes.node[i].state.allocated)
        {

            _clear_node(&_openlcb_nodes.node[i]);

            _openlcb_nodes.node[i].parameters = node_parameters;
            _openlcb_nodes.node[i].id = node_id;
            _openlcb_nodes.node[i].index = (uint8_t)i;

            _generate_event_ids(&_openlcb_nodes.node[i]);

            _openlcb_nodes.count++;

            // last step is to mark it allocated
            _openlcb_nodes.node[i].state.allocated = true;

            return &_openlcb_nodes.node[i];
        }
    }

    return NULL;
}

openlcb_node_t *OpenLcbNode_find_by_alias(uint16_t alias)
{

    for (int i = 0; i < _openlcb_nodes.count; i++)
    {

        if (_openlcb_nodes.node[i].alias == alias)
        {

            return &_openlcb_nodes.node[i];
        }
    };

    return NULL;
}

openlcb_node_t *OpenLcbNode_find_by_node_id(uint64_t nodeid)
{

    for (int i = 0; i < _openlcb_nodes.count; i++)
    {

        if (_openlcb_nodes.node[i].id == nodeid)
        {

            return &_openlcb_nodes.node[i];
        }
    };

    return NULL;
}

void OpenLcbNode_100ms_timer_tick(void)
{

    for (int i = 0; i < _openlcb_nodes.count; i++)
    {

        _openlcb_nodes.node[i].timerticks++;
    };

    if (_interface->on_100ms_timer_tick)
    {

        _interface->on_100ms_timer_tick();
    }
}

void OpenLcbNode_reset_state(void)
{

    for (int i = 0; i < _openlcb_nodes.count; i++)
    {

        _openlcb_nodes.node[i].state.run_state = RUNSTATE_INIT;
        _openlcb_nodes.node[i].state.permitted = false;
        _openlcb_nodes.node[i].state.initialized = false;
    };
}
