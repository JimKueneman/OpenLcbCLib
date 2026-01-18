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
 * @brief OpenLCB node allocation, enumeration, and lifecycle management
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

/**
 * @brief Dependency injection interface for OpenLCB Node module
 *
 * @details Provides callback hooks for application integration with the node
 * management system. Allows the application to respond to timer events.
 *
 * Use cases:
 * - Application needs notification of 100ms timer ticks
 * - Custom processing during node lifecycle events
 *
 * @see OpenLcbNode_initialize - Accepts this interface structure
 */
typedef struct
{

    /**
     * @brief Callback invoked every 100ms timer tick
     *
     * @details Optional callback that is called by OpenLcbNode_100ms_timer_tick()
     * after all node timer counters have been incremented. Set to NULL if not needed.
     *
     * Use cases:
     * - Application-specific timer-based processing
     * - Periodic status monitoring
     *
     * @warning NOT thread-safe
     *
     * @see OpenLcbNode_100ms_timer_tick - Invokes this callback
     */
    void (*on_100ms_timer_tick)(void);

} interface_openlcb_node_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLCB Node management module
     *
     * @details Sets up the node pool and enumeration system for use. This function
     * clears all node structures and prepares the system for node allocation.
     * Must be called once during application startup before any node operations.
     *
     * Use cases:
     * - Called once during application initialization
     * - Required before OpenLcbNode_allocate() can be used
     *
     * @param interface Pointer to interface structure containing optional callbacks,
     *                  or NULL if no callbacks needed
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call before any other OpenLcbNode functions
     *
     * @see OpenLcbNode_allocate - Allocates nodes after initialization
     * @see interface_openlcb_node_t - Structure definition for callbacks
     */
    extern void OpenLcbNode_initialize(const interface_openlcb_node_t *interface);

    /**
     * @brief Allocates a new OpenLCB node with specified configuration
     *
     * @details Searches the node pool for an available slot and initializes a new
     * node with the given 64-bit node ID and configuration parameters. The node is
     * returned ready for use with auto-generated event IDs based on the node ID.
     * Returns NULL if the node pool is exhausted.
     *
     * Use cases:
     * - Creating virtual nodes for the application
     * - Setting up nodes with specific event producer/consumer configurations
     *
     * @param node_id 64-bit unique OpenLCB node identifier for this node
     * @param node_parameters Pointer to configuration structure defining node capabilities
     *                        (producer count, consumer count, etc.)
     *
     * @return Pointer to allocated and initialized node structure, or NULL if pool is full
     *
     * @warning Returns NULL when node pool exhausted - caller MUST check for NULL
     * @warning node_parameters pointer is stored, not copied - must remain valid
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL before use
     * @attention Each node must have a unique node_id
     *
     * @see OpenLcbNode_initialize - Must be called first
     * @see OpenLcbNode_find_by_node_id - Finds previously allocated nodes
     * @see node_parameters_t - Configuration structure definition
     */
    extern openlcb_node_t *OpenLcbNode_allocate(uint64_t node_id, const node_parameters_t *node_parameters);

    /**
     * @brief Gets the first node in the pool for enumeration
     *
     * @details Returns the first allocated node and initializes the enumeration state
     * for the specified key. Supports multiple simultaneous enumerators by using different
     * key values (0 to MAX_NODE_ENUM_KEY_VALUES-1). Each enumerator maintains independent
     * state, allowing different parts of the system to enumerate nodes without interfering.
     *
     * Use cases:
     * - Beginning iteration through all allocated nodes
     * - Main state machine enumeration (typically key=0)
     * - Login state machine enumeration (typically key=1)
     *
     * @param key Enumeration key identifying which enumerator is requesting (0 to MAX_NODE_ENUM_KEY_VALUES-1)
     *
     * @return Pointer to the first allocated node, or NULL if no nodes exist or key invalid
     *
     * @warning Returns NULL if key >= MAX_NODE_ENUM_KEY_VALUES
     *
     * @attention Use unique key values for independent simultaneous enumerations
     * @attention Must use same key value for get_first and subsequent get_next calls
     *
     * @see OpenLcbNode_get_next - Gets subsequent nodes in enumeration
     * @see MAX_NODE_ENUM_KEY_VALUES - Maximum number of simultaneous enumerators
     */
    extern openlcb_node_t *OpenLcbNode_get_first(uint8_t key);

    /**
     * @brief Gets the next node in the enumeration sequence
     *
     * @details Advances the enumeration state for the specified key and returns the
     * next allocated node. Returns NULL when the end of the node list is reached.
     * Must be called with the same key value used in the corresponding get_first call.
     *
     * Use cases:
     * - Continuing iteration through allocated nodes
     * - Processing all nodes in the pool
     *
     * @param key Enumeration key identifying which enumerator is requesting (same as get_first)
     *
     * @return Pointer to the next allocated node, or NULL if at end of list or key invalid
     *
     * @warning Returns NULL if key >= MAX_NODE_ENUM_KEY_VALUES
     *
     * @attention Use same key value as the corresponding get_first call
     *
     * @see OpenLcbNode_get_first - Initializes enumeration
     */
    extern openlcb_node_t *OpenLcbNode_get_next(uint8_t key);

    /**
     * @brief Finds a node by its CAN alias
     *
     * @details Searches the allocated node pool for a node with the specified CAN alias.
     * Uses linear search through all allocated nodes. Returns the first matching node
     * or NULL if no node with that alias exists.
     *
     * Use cases:
     * - Resolving incoming CAN messages to their source node
     * - Mapping CAN alias to full node structure
     *
     * @param alias 12-bit CAN alias to search for
     *
     * @return Pointer to node with matching alias, or NULL if not found
     *
     * @note Linear search through all allocated nodes
     *
     * @see OpenLcbNode_find_by_node_id - Finds node by 64-bit ID instead
     * @see openlcb_node_t - Node structure containing alias field
     */
    extern openlcb_node_t *OpenLcbNode_find_by_alias(uint16_t alias);

    /**
     * @brief Finds a node by its 64-bit OpenLCB node ID
     *
     * @details Searches the allocated node pool for a node with the specified 64-bit
     * unique identifier. Uses linear search through all allocated nodes. Returns the
     * first matching node or NULL if no node with that ID exists.
     *
     * Use cases:
     * - Resolving full OpenLCB messages to their source node
     * - Looking up node by globally unique identifier
     *
     * @param node_id 64-bit OpenLCB node identifier to search for
     *
     * @return Pointer to node with matching node ID, or NULL if not found
     *
     * @note Linear search through all allocated nodes
     *
     * @see OpenLcbNode_find_by_alias - Finds node by CAN alias instead
     * @see OpenLcbNode_allocate - Creates nodes with specified node_id
     */
    extern openlcb_node_t *OpenLcbNode_find_by_node_id(uint64_t node_id);

    /**
     * @brief Resets all nodes to initial uninitialized state
     *
     * @details Resets the runtime state of all allocated nodes to their initial conditions.
     * Sets run_state to RUNSTATE_INIT, clears permitted and initialized flags. Does not
     * deallocate nodes or clear their configuration. Used when restarting the network
     * login sequence.
     *
     * Use cases:
     * - Restarting network login after error
     * - Resetting all nodes to power-up state
     * - Recovering from network disconnect
     *
     * @warning Does not deallocate nodes or free memory
     * @warning NOT thread-safe
     *
     * @see RUNSTATE_INIT - Initial run state value
     * @see openlcb_node_t - Node structure containing state fields
     */
    extern void OpenLcbNode_reset_state(void);

    /**
     * @brief 100ms timer tick handler for all nodes
     *
     * @details Increments the timer tick counter for each allocated node and invokes
     * the optional application callback if provided. Should be called every 100ms by
     * the system timer. The timer tick counters are used for protocol timing requirements.
     *
     * Use cases:
     * - Called by system timer every 100ms
     * - Provides timing for protocol state machines
     *
     * @warning NOT thread-safe
     *
     * @attention Must be called every 100ms for proper protocol timing
     *
     * @see interface_openlcb_node_t - Contains optional callback
     * @see openlcb_node_t - Node structure containing timerticks field
     */
    extern void OpenLcbNode_100ms_timer_tick(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_NODE__ */
