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
 * @file alias_mapping_listener.h
 * @brief On-demand alias resolution table for consist listener nodes.
 *
 * @details Maps listener Node IDs to their CAN aliases so the CAN TX path can
 * transparently resolve dest_alias when forwarding consist commands.  Table is
 * sized to LISTENER_ALIAS_TABLE_DEPTH (USER_DEFINED_MAX_LISTENERS_PER_TRAIN *
 * USER_DEFINED_TRAIN_NODE_COUNT).
 *
 * Entries are registered when a listener is attached and populated when the
 * corresponding AMD frame arrives.  AMR clears individual entries; global AME
 * flushes all aliases (node_ids preserved).
 *
 * @author Jim Kueneman
 * @date 8 Mar 2026
 */

#ifndef __DRIVERS_CANBUS_ALIAS_MAPPING_LISTENER__
#define __DRIVERS_CANBUS_ALIAS_MAPPING_LISTENER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Zeros all entries in the listener alias table.
         *
         * @details Must be called once at startup before any listener operations.
         */
    extern void ListenerAliasTable_initialize(void);

        /**
         * @brief Registers a listener Node ID in the table with alias = 0.
         *
         * @details First-fit allocation into an empty slot (node_id == 0).
         * If the node_id is already registered, returns the existing entry
         * without modification.
         *
         * @param node_id  48-bit OpenLCB Node ID of the listener.
         *
         * @return Pointer to the @ref listener_alias_entry_t, or NULL if full or invalid.
         */
    extern listener_alias_entry_t *ListenerAliasTable_register(node_id_t node_id);

        /**
         * @brief Removes the entry matching node_id from the table.
         *
         * @details Clears both node_id and alias fields. Safe to call with a
         * node_id that is not registered.
         *
         * @param node_id  48-bit OpenLCB Node ID to remove.
         */
    extern void ListenerAliasTable_unregister(node_id_t node_id);

        /**
         * @brief Stores a resolved alias for a registered listener Node ID.
         *
         * @details Called when an AMD frame arrives. If node_id is not in the
         * table, this is a no-op (the node is not one of our listeners).
         *
         * @param node_id  48-bit OpenLCB Node ID from the AMD payload.
         * @param alias    12-bit CAN alias from the AMD source.
         */
    extern void ListenerAliasTable_set_alias(node_id_t node_id, uint16_t alias);

        /**
         * @brief Finds the table entry for a given listener Node ID.
         *
         * @details Primary TX-path query. Caller checks entry->alias != 0
         * to confirm the alias has been resolved.
         *
         * @param node_id  48-bit OpenLCB Node ID to look up.
         *
         * @return Pointer to the @ref listener_alias_entry_t, or NULL if not found.
         */
    extern listener_alias_entry_t *ListenerAliasTable_find_by_node_id(node_id_t node_id);

        /**
         * @brief Zeros all alias fields but preserves registered node_ids.
         *
         * @details Called when a global AME (empty payload) is received per
         * CanFrameTransferS. The AMD replies triggered by the global AME will
         * re-populate the aliases via ListenerAliasTable_set_alias().
         */
    extern void ListenerAliasTable_flush_aliases(void);

        /**
         * @brief Zeros the alias field for the entry matching a specific alias value.
         *
         * @details Called when an AMR frame is received — the node owning that
         * alias is releasing it. When the node re-claims an alias and sends AMD,
         * ListenerAliasTable_set_alias() will re-populate.
         *
         * @param alias  12-bit CAN alias being released.
         */
    extern void ListenerAliasTable_clear_alias_by_alias(uint16_t alias);

        /**
         * @brief Probes one listener entry for alias staleness (round-robin).
         *
         * @details Rate-limited to once per USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL
         * ticks. At most one entry probed per call.  Returns the Node ID whose
         * alias needs verification, or 0 if nothing to do.
         *
         * State transitions:
         * - Resolved + probe interval elapsed -> Verifying (AME sent)
         * - Verifying + timeout elapsed -> Stale (alias cleared)
         * - Verifying + AMD arrives (via set_alias) -> Resolved
         *
         * @param current_tick  Current value of the global 100ms tick counter.
         *
         * @return @ref node_id_t to probe (caller queues targeted AME), or 0.
         */
    extern node_id_t ListenerAliasTable_check_one_verification(uint8_t current_tick);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_ALIAS_MAPPING_LISTENER__ */
