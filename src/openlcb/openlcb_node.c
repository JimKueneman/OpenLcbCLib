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
 * @file openlcb_node.c
 * @brief OpenLCB node allocation, enumeration, and lifecycle management.
 *
 * @details Fixed-size pool of @ref openlcb_node_t structures with allocation,
 * multi-key enumeration, alias/ID lookup, and auto-generated event IDs.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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

/** @brief Bit shift to convert a 48-bit node ID into a 64-bit event ID base. */
#define OPENLCB_EVENT_ID_OFFSET 16

/** @brief Pool of all node structures. */
static openlcb_nodes_t _openlcb_nodes;

/** @brief Per-key enumeration indices for independent node iteration. */
static uint8_t _node_enum_index_array[MAX_NODE_ENUM_KEY_VALUES];

/** @brief Stored interface pointer for optional application callbacks. */
static const interface_openlcb_node_t *_interface;

    /**
     * @brief Clears all fields in a single node structure.
     *
     * @details Algorithm:
     * -# Zero all state fields, alias, ID, seed, timerticks, and owner
     * -# Clear all consumer and producer event entries and range entries
     * -# Stop any running enumerations
     *
     * @verbatim
     * @param openlcb_node Pointer to @ref openlcb_node_t to clear
     * @endverbatim
     */
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
    for (int i = 0; i < USER_DEFINED_CONSUMER_COUNT; i++) {

        openlcb_node->consumers.list[i].event = 0;
        openlcb_node->consumers.list[i].status = EVENT_STATUS_UNKNOWN;

    }

    openlcb_node->producers.count = 0;
    for (int j = 0; j < USER_DEFINED_PRODUCER_COUNT; j++) {

        openlcb_node->producers.list[j].event = 0;
        openlcb_node->producers.list[j].status = EVENT_STATUS_UNKNOWN;

    }

    openlcb_node->consumers.range_count = 0;
    for (int i = 0; i < USER_DEFINED_CONSUMER_RANGE_COUNT; i++)
    {

        openlcb_node->consumers.range_list[i].start_base = NULL_EVENT_ID;
        openlcb_node->consumers.range_list[i].event_count = 0;
    }

    openlcb_node->producers.range_count = 0;
    for (int j = 0; j < USER_DEFINED_PRODUCER_RANGE_COUNT; j++)
    {

        openlcb_node->producers.range_list[j].start_base = NULL_EVENT_ID;
        openlcb_node->producers.range_list[j].event_count = 0;
    }

    openlcb_node->producers.enumerator.running = false;
    openlcb_node->consumers.enumerator.running = false;


}

    /**
     * @brief Initializes the node management module.
     *
     * @details Algorithm:
     * -# Store interface pointer (may be NULL)
     * -# Clear all node structures via _clear_node()
     * -# Reset node count to zero
     * -# Zero all enumeration index entries
     *
     * @verbatim
     * @param interface Pointer to @ref interface_openlcb_node_t with optional callbacks, or NULL
     * @endverbatim
     */
void OpenLcbNode_initialize(const interface_openlcb_node_t *interface)
{

    _interface = interface;

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        _clear_node(&_openlcb_nodes.node[i]);

    }

    _openlcb_nodes.count = 0;

    for (int j = 0; j < MAX_NODE_ENUM_KEY_VALUES; j++) {

        _node_enum_index_array[j] = 0;

    }

}

    /**
     * @brief Returns the first allocated node for enumeration.
     *
     * @details Algorithm:
     * -# Validate key is within range
     * -# Reset enumeration index for this key to 0
     * -# Return first node, or NULL if no nodes allocated
     *
     * @verbatim
     * @param key Enumerator index (0 to MAX_NODE_ENUM_KEY_VALUES - 1)
     * @endverbatim
     *
     * @return Pointer to the first @ref openlcb_node_t, or NULL if empty or key invalid
     */
openlcb_node_t *OpenLcbNode_get_first(uint8_t key)
{

    if (key >= MAX_NODE_ENUM_KEY_VALUES) {

        return NULL;

    }

    _node_enum_index_array[key] = 0;

    if (_openlcb_nodes.count == 0) {

        return NULL;

    }

    return &_openlcb_nodes.node[_node_enum_index_array[key]];
}

    /**
     * @brief Returns the next allocated node for the given enumerator key.
     *
     * @details Algorithm:
     * -# Validate key is within range
     * -# Increment enumeration index
     * -# Return node at new index, or NULL if past the end
     *
     * @verbatim
     * @param key Same enumerator index used in the corresponding get_first call
     * @endverbatim
     *
     * @return Pointer to the next @ref openlcb_node_t, or NULL if at end or key invalid
     */
openlcb_node_t *OpenLcbNode_get_next(uint8_t key)
{

    if (key >= MAX_NODE_ENUM_KEY_VALUES) {

        return NULL;

    }

    _node_enum_index_array[key]++;

    if (_node_enum_index_array[key] >= _openlcb_nodes.count) {

        return NULL;

    }

    return &_openlcb_nodes.node[_node_enum_index_array[key]];
}

    /**
     * @brief Generates auto-created event IDs for a node's producers and consumers.
     *
     * @details Algorithm:
     * -# Compute base event ID by shifting node ID left by OPENLCB_EVENT_ID_OFFSET bits
     * -# For each consumer to auto-create, assign base + index (bounded by USER_DEFINED_CONSUMER_COUNT)
     * -# For each producer to auto-create, assign base + index (bounded by USER_DEFINED_PRODUCER_COUNT)
     * -# Clear consumer and producer enumeration states
     *
     * @verbatim
     * @param openlcb_node Pointer to @ref openlcb_node_t to generate event IDs for
     * @endverbatim
     */
static void _generate_event_ids(openlcb_node_t *openlcb_node)
{

    uint64_t node_id = openlcb_node->id << OPENLCB_EVENT_ID_OFFSET;
    uint16_t indexer = 0;

    openlcb_node->consumers.count = 0;
    for (int i = 0; i < openlcb_node->parameters->consumer_count_autocreate; i++) {

        if (i < USER_DEFINED_CONSUMER_COUNT) { // safety net

            openlcb_node->consumers.list[i].event = node_id + indexer;
            openlcb_node->consumers.count++;
            indexer++;

        }

    }

    indexer = 0;
    openlcb_node->producers.count = 0;
    for (int j = 0; j < openlcb_node->parameters->producer_count_autocreate; j++) {

        if (j < USER_DEFINED_PRODUCER_COUNT) { // safety net

            openlcb_node->producers.list[j].event = node_id + indexer;
            openlcb_node->producers.count++;
            indexer++;

        }

    }

    openlcb_node->consumers.enumerator.running = false;
    openlcb_node->consumers.enumerator.enum_index = 0;

    openlcb_node->producers.enumerator.running = false;
    openlcb_node->producers.enumerator.enum_index = 0;

}

    /**
     * @brief Allocates a new node with the given ID and configuration.
     *
     * @details Algorithm:
     * -# Search pool for first unallocated slot
     * -# Clear the slot via _clear_node()
     * -# Store node_parameters pointer (not copied) and node_id
     * -# Generate auto-created event IDs via _generate_event_ids()
     * -# Increment node count and mark as allocated (last step)
     * -# Return pointer, or NULL if pool is full
     *
     * @verbatim
     * @param node_id 64-bit unique OpenLCB node identifier
     * @param node_parameters Pointer to @ref node_parameters_t defining node capabilities
     * @endverbatim
     *
     * @return Pointer to the allocated @ref openlcb_node_t, or NULL if the pool is full
     */
openlcb_node_t *OpenLcbNode_allocate(uint64_t node_id, const node_parameters_t *node_parameters)
{

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        if (!_openlcb_nodes.node[i].state.allocated) {

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

    /**
     * @brief Finds a node by its 12-bit CAN alias.
     *
     * @details Algorithm:
     * -# Search all allocated nodes for matching alias
     * -# Return first match, or NULL if not found
     *
     * @verbatim
     * @param alias 12-bit CAN alias to search for
     * @endverbatim
     *
     * @return Pointer to matching @ref openlcb_node_t, or NULL if not found
     */
openlcb_node_t *OpenLcbNode_find_by_alias(uint16_t alias)
{

    for (int i = 0; i < _openlcb_nodes.count; i++) {

        if (_openlcb_nodes.node[i].alias == alias) {

            return &_openlcb_nodes.node[i];

        }

    }

    return NULL;
}

    /**
     * @brief Finds a node by its 64-bit OpenLCB node ID.
     *
     * @details Algorithm:
     * -# Search all allocated nodes for matching node ID
     * -# Return first match, or NULL if not found
     *
     * @verbatim
     * @param node_id 64-bit OpenLCB node identifier to search for
     * @endverbatim
     *
     * @return Pointer to matching @ref openlcb_node_t, or NULL if not found
     */
openlcb_node_t *OpenLcbNode_find_by_node_id(uint64_t node_id)
{

    for (int i = 0; i < _openlcb_nodes.count; i++) {

        if (_openlcb_nodes.node[i].id == node_id) {

            return &_openlcb_nodes.node[i];

        }

    }

    return NULL;
}

    /**
     * @brief 100ms timer tick handler for all allocated nodes.
     *
     * @details Algorithm:
     * -# Increment timerticks on each allocated node
     * -# Call the interface on_100ms_timer_tick callback if registered
     */
void OpenLcbNode_100ms_timer_tick(void)
{

    for (int i = 0; i < _openlcb_nodes.count; i++) {

        _openlcb_nodes.node[i].timerticks++;

    }

    if (_interface && _interface->on_100ms_timer_tick) {

        _interface->on_100ms_timer_tick();

    }

}

    /**
     * @brief Resets all allocated nodes to their initial login state.
     *
     * @details Algorithm:
     * -# For each allocated node, set run_state to RUNSTATE_INIT
     * -# Clear permitted and initialized flags
     */
void OpenLcbNode_reset_state(void)
{

    for (int i = 0; i < _openlcb_nodes.count; i++) {

        _openlcb_nodes.node[i].state.run_state = RUNSTATE_INIT;
        _openlcb_nodes.node[i].state.permitted = false;
        _openlcb_nodes.node[i].state.initialized = false;

    }

}
