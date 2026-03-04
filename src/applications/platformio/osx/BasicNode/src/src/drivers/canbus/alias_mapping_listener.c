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
 * @file alias_mapping_listener.c
 * @brief On-demand alias resolution table for consist listener nodes.
 *
 * @details Single static table instance using a linear array. Empty slots have
 * node_id == 0. First-fit allocation. NOT thread-safe — callers must use
 * lock_shared_resources / unlock_shared_resources around shared access.
 *
 * @author Jim Kueneman
 * @date 4 Mar 2026
 */

#include "alias_mapping_listener.h"

#include <stdint.h>
#include <stddef.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

/** @brief Static storage for the listener alias table. */
static listener_alias_entry_t _table[LISTENER_ALIAS_TABLE_DEPTH];

    /** @brief Zeros all entries in the listener alias table. */
void ListenerAliasTable_initialize(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        _table[i].node_id = 0;
        _table[i].alias = 0;

    }

}

    /**
     * @brief Registers a listener Node ID in the table with alias = 0.
     *
     * @details Algorithm:
     * -# Validate node_id is non-zero, return NULL if invalid
     * -# Scan for existing entry with matching node_id, return it if found
     * -# Scan for first empty slot (node_id == 0), store node_id with alias = 0
     * -# Return NULL if no empty slot (table full)
     *
     * @verbatim
     * @param node_id  48-bit OpenLCB Node ID of the listener.
     * @endverbatim
     *
     * @return Pointer to the @ref listener_alias_entry_t, or NULL if full or invalid.
     */
listener_alias_entry_t *ListenerAliasTable_register(node_id_t node_id) {

    if (node_id == 0) {

        return NULL;

    }

    // Check if already registered
    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        if (_table[i].node_id == node_id) {

            return &_table[i];

        }

    }

    // Find first empty slot
    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        if (_table[i].node_id == 0) {

            _table[i].node_id = node_id;
            _table[i].alias = 0;
            return &_table[i];

        }

    }

    return NULL;

}

    /**
     * @brief Removes the entry matching node_id from the table.
     *
     * @details Algorithm:
     * -# Scan for entry with matching node_id
     * -# Clear both node_id and alias fields
     *
     * @verbatim
     * @param node_id  48-bit OpenLCB Node ID to remove.
     * @endverbatim
     */
void ListenerAliasTable_unregister(node_id_t node_id) {

    if (node_id == 0) {

        return;

    }

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        if (_table[i].node_id == node_id) {

            _table[i].node_id = 0;
            _table[i].alias = 0;
            return;

        }

    }

}

    /**
     * @brief Stores a resolved alias for a registered listener Node ID.
     *
     * @details Algorithm:
     * -# Validate alias is in range 0x001-0xFFF, return if invalid
     * -# Scan for entry with matching node_id
     * -# Store alias if found; no-op if node_id is not in the table
     *
     * @verbatim
     * @param node_id  48-bit OpenLCB Node ID from the AMD payload.
     * @param alias    12-bit CAN alias from the AMD source.
     * @endverbatim
     */
void ListenerAliasTable_set_alias(node_id_t node_id, uint16_t alias) {

    if (alias == 0 || alias > 0xFFF) {

        return;

    }

    if (node_id == 0) {

        return;

    }

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        if (_table[i].node_id == node_id) {

            _table[i].alias = alias;
            return;

        }

    }

}

    /**
     * @brief Finds the table entry for a given listener Node ID.
     *
     * @details Algorithm:
     * -# Scan for entry with matching node_id
     * -# Return pointer to entry, or NULL if not found
     *
     * @verbatim
     * @param node_id  48-bit OpenLCB Node ID to look up.
     * @endverbatim
     *
     * @return Pointer to the @ref listener_alias_entry_t, or NULL if not found.
     */
listener_alias_entry_t *ListenerAliasTable_find_by_node_id(node_id_t node_id) {

    if (node_id == 0) {

        return NULL;

    }

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        if (_table[i].node_id == node_id) {

            return &_table[i];

        }

    }

    return NULL;

}

    /**
     * @brief Zeros all alias fields but preserves registered node_ids.
     *
     * @details Algorithm:
     * -# Iterate all LISTENER_ALIAS_TABLE_DEPTH entries
     * -# Set alias = 0 on each; leave node_id untouched
     */
void ListenerAliasTable_flush_aliases(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        _table[i].alias = 0;

    }

}

    /**
     * @brief Zeros the alias field for the entry matching a specific alias value.
     *
     * @details Algorithm:
     * -# Validate alias is non-zero
     * -# Scan for entry with matching alias
     * -# Set alias = 0 if found; leave node_id intact
     *
     * @verbatim
     * @param alias  12-bit CAN alias being released.
     * @endverbatim
     */
void ListenerAliasTable_clear_alias_by_alias(uint16_t alias) {

    if (alias == 0) {

        return;

    }

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        if (_table[i].alias == alias) {

            _table[i].alias = 0;
            return;

        }

    }

}
