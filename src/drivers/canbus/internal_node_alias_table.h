/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file internal_node_alias_table.h
 * @brief Lock-free snapshot of THIS device's own (alias, Node ID) pairs.
 *
 * SCOPE — IMPORTANT, READ BEFORE USE
 * ----------------------------------
 * This table holds ONLY the aliases of nodes this device hosts (i.e. the
 * aliases we have allocated for our own openlcb_node_t instances).  It
 * exists so the CAN RX interrupt / RX thread can perform alias-collision
 * and duplicate-NodeID checks WITHOUT reaching into the openlcb_node_t
 * structures (which would require a mutex / synchronization point with the
 * main loop).  It is essentially a thread-safe shadow copy of "our own"
 * identity bindings.
 *
 * It is NOT a general-purpose alias resolution table:
 *  - It does NOT cache remote nodes' aliases learned from AMD frames on
 *    the bus.
 *  - It does NOT support "look up arbitrary NodeID for a remote alias I
 *    just received."  To learn a remote node's NodeID from its alias,
 *    send an MTI_VERIFY_NODE_ID_ADDRESSED to that alias and handle the
 *    Verified Node ID reply.
 *  - It is NOT used by the consist Listener Attach mechanism — that uses
 *    the separate alias_mapping_listener module.
 *
 * @details Bidirectional lookup (by alias or by NodeID) is provided so the
 * RX path can answer "does this incoming alias collide with one of ours?"
 * and "do we already host this NodeID under a different alias?".  Must be
 * initialized before any node operations.
 *
 * @author Jim Kueneman
 * @date 4 Mar 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_INTERNAL_NODE_ALIAS_TABLE__
#define __DRIVERS_CANBUS_INTERNAL_NODE_ALIAS_TABLE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Initializes the alias mapping buffer, clearing all entries and flags.
         *
         * @warning Calling during active operation loses all existing mappings and
         *          will cause communication failures.
         *
         * @see InternalNodeAliasTable_flush - Runtime equivalent
         */
    extern void InternalNodeAliasTable_initialize(void);

        /**
         * @brief Returns a pointer to the internal alias mapping info structure.
         *
         * @details Intended for diagnostics and testing.  Prefer the specific API
         * functions for normal use — direct modification can break internal consistency.
         *
         * @return Pointer to the @ref alias_mapping_info_t structure (never NULL).
         *
         * @see InternalNodeAliasTable_set_has_duplicate_alias_flag
         * @see InternalNodeAliasTable_clear_has_duplicate_alias_flag
         */
    extern alias_mapping_info_t *InternalNodeAliasTable_get_alias_mapping_info(void);

        /**
         * @brief Signals the main loop that an incoming message used one of our reserved aliases.
         *
         * @details Sets the has_duplicate_alias flag.  The main state machine must detect
         * this flag and restart alias allocation with a new alias.  Setting this flag does
         * NOT automatically trigger reallocation.
         *
         * @attention The main loop must clear this flag after resolving the conflict.
         *
         * @see InternalNodeAliasTable_clear_has_duplicate_alias_flag
         */
    extern void InternalNodeAliasTable_set_has_duplicate_alias_flag(void);

        /** @brief Clears the duplicate alias flag after the conflict has been resolved. */
    extern void InternalNodeAliasTable_clear_has_duplicate_alias_flag(void);

        /**
         * @brief Registers a CAN alias / Node ID pair in the buffer.
         *
         * @details Searches for an empty slot or an existing entry with the same Node ID.
         * If the Node ID already exists its alias is updated (correct behavior during
         * conflict recovery).  Returns NULL if the buffer is full or either parameter is
         * outside its valid range.
         *
         * Use cases:
         * - Storing a newly allocated alias during one of OUR nodes' login
         * - Updating an alias after conflict resolution / re-login
         *
         * Does NOT record remote nodes' aliases — see file header.
         *
         * @param alias    12-bit CAN alias (valid range: 0x001–0xFFF).
         * @param node_id  48-bit OpenLCB @ref node_id_t (valid range: 0x000000000001–0xFFFFFFFFFFFF).
         *
         * @return Pointer to the registered @ref alias_mapping_t entry, or NULL on failure.
         *
         * @warning Returns NULL when the buffer is full — caller MUST check before use.
         * @warning Out-of-range alias or node_id values return NULL.
         * @warning An existing Node ID entry will have its alias silently replaced.
         *
         * @see InternalNodeAliasTable_unregister
         * @see InternalNodeAliasTable_find_mapping_by_alias
         * @see InternalNodeAliasTable_find_mapping_by_node_id
         */
    extern alias_mapping_t *InternalNodeAliasTable_register(uint16_t alias, node_id_t node_id);

        /**
         * @brief Removes the entry matching the given alias from the buffer.
         *
         * @details Safe to call with an alias that does not exist — no error is raised.
         * All fields in the matching slot are cleared so it can be reused.
         *
         * @param alias  12-bit CAN alias to remove.
         *
         * @attention Pointers previously returned for this entry become invalid after removal.
         *
         * @see InternalNodeAliasTable_register
         * @see InternalNodeAliasTable_flush
         */
    extern void InternalNodeAliasTable_unregister(uint16_t alias);

        /**
         * @brief Finds the mapping entry for the given alias.
         *
         * @param alias  12-bit CAN alias to search for.
         *
         * @return Pointer to the matching @ref alias_mapping_t entry, or NULL if not found.
         *
         * @warning NULL is returned when not found — caller MUST check before use.
         *
         * @see InternalNodeAliasTable_find_mapping_by_node_id
         */
    extern alias_mapping_t *InternalNodeAliasTable_find_mapping_by_alias(uint16_t alias);

        /**
         * @brief Finds the mapping entry for the given Node ID.
         *
         * @param node_id  48-bit OpenLCB @ref node_id_t to search for.
         *
         * @return Pointer to the matching @ref alias_mapping_t entry, or NULL if not found.
         *
         * @warning NULL is returned when not found — caller MUST check before use.
         *
         * @see InternalNodeAliasTable_find_mapping_by_alias
         */
    extern alias_mapping_t *InternalNodeAliasTable_find_mapping_by_node_id(node_id_t node_id);

        /**
         * @brief Clears all alias mappings and resets all flags.
         *
         * @details Functionally identical to InternalNodeAliasTable_initialize() but intended for
         * runtime use (system reset, test teardown).  Use initialize() at startup.
         *
         * @warning Invalidates ALL pointers previously returned by register or find functions.
         * @warning All nodes lose their CAN bus identity and must re-login.
         *
         * @see InternalNodeAliasTable_initialize
         * @see InternalNodeAliasTable_unregister
         */
    extern void InternalNodeAliasTable_flush(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_ALIAS_MAPPINGS__ */
