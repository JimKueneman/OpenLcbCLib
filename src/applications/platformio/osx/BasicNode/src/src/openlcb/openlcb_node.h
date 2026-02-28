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
 * @brief OpenLCB node allocation, enumeration, and lifecycle management.
 *
 * @details Manages a fixed-size pool of @ref openlcb_node_t structures.  Supports
 * allocation with auto-generated event IDs, multiple simultaneous enumerators for
 * iterating through allocated nodes, and lookup by CAN alias or 64-bit node ID.
 * Must be initialized before any node operations.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_NODE__
#define __OPENLCB_OPENLCB_NODE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Dependency injection interface for the OpenLCB Node module.
     *
     * @details Provides an optional callback hook for the application to receive
     * 100ms timer tick notifications after all node counters have been incremented.
     */
typedef struct
{

        /** @brief Optional callback invoked every 100ms after node timer updates (NULL if unused). */
    void (*on_100ms_timer_tick)(void);

} interface_openlcb_node_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Initializes the node management module.
         *
         * @details Clears all node structures, resets the node count, and stores the
         * optional callback interface.  Must be called once at startup before any
         * other OpenLcbNode function.
         *
         * @param interface  Pointer to @ref interface_openlcb_node_t with optional callbacks, or NULL.
         */
    extern void OpenLcbNode_initialize(const interface_openlcb_node_t *interface);

        /**
         * @brief Allocates a new node with the given ID and configuration.
         *
         * @details Finds the first free slot in the pool, initializes it with the
         * provided node ID, stores the configuration pointer (not copied), and
         * auto-generates event IDs.  Returns NULL if the pool is full.
         *
         * @param node_id          64-bit unique OpenLCB node identifier.
         * @param node_parameters  Pointer to @ref node_parameters_t defining node capabilities.
         *
         * @return Pointer to the allocated @ref openlcb_node_t, or NULL if the pool is full.
         *
         * @warning Configuration pointer is stored, not copied — it must remain valid.
         */
    extern openlcb_node_t *OpenLcbNode_allocate(uint64_t node_id, const node_parameters_t *node_parameters);

        /**
         * @brief Returns the first allocated node for enumeration.
         *
         * @details Resets the enumeration index for the given key and returns the first
         * node.  Multiple independent enumerators are supported using different key values
         * (0 to MAX_NODE_ENUM_KEY_VALUES - 1).
         *
         * @param key  Enumerator index (0 to MAX_NODE_ENUM_KEY_VALUES - 1).
         *
         * @return Pointer to the first @ref openlcb_node_t, or NULL if no nodes or key invalid.
         */
    extern openlcb_node_t *OpenLcbNode_get_first(uint8_t key);

        /**
         * @brief Returns the next allocated node for the given enumerator key.
         *
         * @param key  Same enumerator index used in the corresponding get_first call.
         *
         * @return Pointer to the next @ref openlcb_node_t, or NULL if at end or key invalid.
         */
    extern openlcb_node_t *OpenLcbNode_get_next(uint8_t key);

        /**
         * @brief Finds a node by its 12-bit CAN alias.
         *
         * @param alias  12-bit CAN alias to search for.
         *
         * @return Pointer to matching @ref openlcb_node_t, or NULL if not found.
         */
    extern openlcb_node_t *OpenLcbNode_find_by_alias(uint16_t alias);

        /**
         * @brief Finds a node by its 64-bit OpenLCB node ID.
         *
         * @param node_id  64-bit OpenLCB node identifier to search for.
         *
         * @return Pointer to matching @ref openlcb_node_t, or NULL if not found.
         */
    extern openlcb_node_t *OpenLcbNode_find_by_node_id(uint64_t node_id);

        /**
         * @brief Resets all allocated nodes to their initial login state.
         *
         * @details Sets run_state to RUNSTATE_INIT and clears permitted and initialized
         * flags.  Does not deallocate nodes or clear configuration.
         */
    extern void OpenLcbNode_reset_state(void);

        /**
         * @brief 100ms timer tick handler for all allocated nodes.
         *
         * @details Increments each node's timerticks counter and invokes the optional
         * application callback if registered.
         */
    extern void OpenLcbNode_100ms_timer_tick(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_NODE__ */
