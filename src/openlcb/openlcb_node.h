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
 * @file openlcb_node.h
 * @brief OpenLcb node structures and management functions
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_NODE__
#define __OPENLCB_OPENLCB_NODE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h" // include processor files - each processor file is guarded.

typedef struct
{

    // callbacks
    void (*on_100ms_timer_tick)(void);

} interface_openlcb_node_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLcb Node module
     * @param interface Pointer to the interface structure containing required callbacks
     * @return None
     */
    extern void OpenLcbNode_initialize(const interface_openlcb_node_t *interface);

    /**
     * @brief Allocates a new OpenLcb node with the specified node ID and parameters
     * @param nodeid 64-bit unique node ID
     * @param node_parameters Pointer to node configuration parameters
     * @return Pointer to the allocated node or NULL if allocation fails
     */
    extern openlcb_node_t *OpenLcbNode_allocate(uint64_t nodeid, const node_parameters_t *node_parameters);

    /**
     * @brief Gets the first node in the node list for enumeration
     * @param key Enumeration key to identify which enumerator is requesting
     * @return Pointer to the first node or NULL if no nodes exist
     */
    extern openlcb_node_t *OpenLcbNode_get_first(uint8_t key);

    /**
     * @brief Gets the next node in the node list for enumeration
     * @param key Enumeration key to identify which enumerator is requesting
     * @return Pointer to the next node or NULL if at end of list
     */
    extern openlcb_node_t *OpenLcbNode_get_next(uint8_t key);

    /**
     * @brief Finds a node by its CAN alias
     * @param alias CAN alias to search for
     * @return Pointer to the node with matching alias or NULL if not found
     */
    extern openlcb_node_t *OpenLcbNode_find_by_alias(uint16_t alias);

    /**
     * @brief Finds a node by its 64-bit node ID
     * @param nodeid Node ID to search for
     * @return Pointer to the node with matching node ID or NULL if not found
     */
    extern openlcb_node_t *OpenLcbNode_find_by_node_id(uint64_t nodeid);

    /**
     * @brief Resets the state of all nodes to initial conditions
     * @param None
     * @return None
     */
    extern void OpenLcbNode_reset_state(void);

    /**
     * @brief 100ms timer tick handler for all nodes
     * @param None
     * @return None
     */
    extern void OpenLcbNode_100ms_timer_tick(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_NODE__ */
