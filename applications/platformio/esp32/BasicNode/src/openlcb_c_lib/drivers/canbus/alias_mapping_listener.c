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
 * @date 8 Mar 2026
 */

#include "alias_mapping_listener.h"

#include <stdint.h>
#include <stddef.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

/** @brief Static storage for the listener alias table. */
static listener_alias_entry_t _table[LISTENER_ALIAS_TABLE_DEPTH];

/** @brief Round-robin cursor for distributed verification. */
static uint16_t _verify_cursor = 0;

/** @brief Tick snapshot for rate-limiting prober calls. */
static uint8_t _verify_last_tick = 0;

/** @brief Monotonic prober counter, incremented each time the prober runs.
 *  @details Decoupled from the 8-bit global tick so that probe intervals
 *  up to 65535 work correctly without wraparound issues. */
static uint16_t _verify_counter = 0;

    /** @brief Zeros all entries in the listener alias table and resets the prober cursor. */
void ListenerAliasTable_initialize(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        _table[i].node_id = 0;
        _table[i].alias = 0;
        _table[i].verify_ticks = 0;
        _table[i].verify_pending = 0;

    }

    _verify_cursor = 0;
    _verify_last_tick = 0;
    _verify_counter = 0;

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
            _table[i].verify_ticks = 0;
            _table[i].verify_pending = 0;
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
            _table[i].verify_ticks = 0;
            _table[i].verify_pending = 0;
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
            _table[i].verify_ticks = _verify_counter;
            _table[i].verify_pending = 0;
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
        _table[i].verify_pending = 0;

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
            _table[i].verify_pending = 0;
            return;

        }

    }

}

    /**
     * @brief Probes one listener entry for alias staleness (round-robin).
     *
     * @details Called periodically from the CAN main state machine.
     * Rate-limited to once per USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL
     * ticks.  Each call advances a static cursor to the next registered
     * entry with a resolved alias and checks:
     *
     * Algorithm:
     * -# If fewer than PROBE_TICK_INTERVAL ticks have elapsed, return 0
     * -# Scan up to LISTENER_ALIAS_TABLE_DEPTH entries from the cursor
     * -# For each entry with verify_pending set: check timeout, clear alias
     *    if expired, continue to next entry
     * -# For each resolved entry not pending: check probe interval, set
     *    verify_pending and return node_id if due
     * -# Return 0 if no entry needs probing this tick
     *
     * @verbatim
     * @param current_tick  Current value of the global 100ms tick counter.
     * @endverbatim
     *
     * @return The @ref node_id_t to probe (caller builds and queues targeted
     *         AME), or 0 if nothing to do this tick.
     */
node_id_t ListenerAliasTable_check_one_verification(uint8_t current_tick) {

    // Rate-limit: at most one probe per PROBE_TICK_INTERVAL
    uint8_t elapsed = (uint8_t) (current_tick - _verify_last_tick);

    if (elapsed < USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL) {

        return 0;

    }

    _verify_last_tick = current_tick;
    _verify_counter++;

    // Scan up to one full table rotation looking for work
    for (int scanned = 0; scanned < LISTENER_ALIAS_TABLE_DEPTH; scanned++) {

        _verify_cursor = (_verify_cursor + 1) % LISTENER_ALIAS_TABLE_DEPTH;
        listener_alias_entry_t* entry = &_table[_verify_cursor];

        if (entry->node_id == 0) {

            continue;  // empty slot

        }

        if (entry->alias == 0 && !entry->verify_pending) {

            continue;  // unresolved and not pending — nothing to check

        }

        if (entry->verify_pending) {

            uint16_t age = (uint16_t) (_verify_counter - entry->verify_ticks);

            if (age >= USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS) {

                // Stale — no AMD reply within timeout
                entry->alias = 0;
                entry->verify_pending = 0;

            }

            continue;  // either timed out or still waiting — move on

        }

        // Entry is resolved (alias != 0) and not pending — check probe interval
        uint16_t age = (uint16_t) (_verify_counter - entry->verify_ticks);

        if (age >= USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS) {

            entry->verify_pending = 1;
            entry->verify_ticks = _verify_counter;

            return entry->node_id;  // caller builds targeted AME

        }

        // Not due yet — try next entry

    }

    return 0;  // nothing to do this cycle

}
