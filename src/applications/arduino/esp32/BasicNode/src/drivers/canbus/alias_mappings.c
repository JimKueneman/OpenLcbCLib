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
 * @file alias_mappings.c
 * @brief Implementation of the CAN alias / Node ID mapping buffer.
 *
 * @details Single static buffer instance using a linear array.  Empty slots are
 * marked by alias = 0 and node_id = 0.  First-fit allocation.  One alias per
 * Node ID enforced.  NOT thread-safe.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "alias_mappings.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

/** @brief Static storage for the alias mapping buffer and control flags. */
static alias_mapping_info_t _alias_mapping_info;

    /**
     * @brief Resets all mapping entries and clears the duplicate alias flag.
     *
     * @details Algorithm:
     * -# Iterate through all ALIAS_MAPPING_BUFFER_DEPTH entries
     * -# Set alias, node_id to 0 and both flags to false in each entry
     * -# Clear the has_duplicate_alias flag
     *
     * @see AliasMappings_initialize
     * @see AliasMappings_flush
     */
static void _reset_mappings(void) {

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        _alias_mapping_info.list[i].alias = 0;
        _alias_mapping_info.list[i].node_id = 0;
        _alias_mapping_info.list[i].is_duplicate = false;
        _alias_mapping_info.list[i].is_permitted = false;

    }

    _alias_mapping_info.has_duplicate_alias = false;

}

    /** @brief Initializes the alias mapping buffer, clearing all entries and flags. */
void AliasMappings_initialize(void) {

    _reset_mappings();

}

    /**
     * @brief Returns a pointer to the internal alias mapping info structure.
     *
     * @return Pointer to the @ref alias_mapping_info_t structure (never NULL).
     */
alias_mapping_info_t *AliasMappings_get_alias_mapping_info(void) {

    return &_alias_mapping_info;

}

    /** @brief Sets the has_duplicate_alias flag to signal an alias conflict. */
void AliasMappings_set_has_duplicate_alias_flag(void) {

    _alias_mapping_info.has_duplicate_alias = true;

}

    /** @brief Clears the has_duplicate_alias flag after conflict resolution. */
void AliasMappings_clear_has_duplicate_alias_flag(void) {

    _alias_mapping_info.has_duplicate_alias = false;

}

    /**
     * @brief Registers a CAN alias / Node ID pair in the buffer.
     *
     * @details Algorithm:
     * -# Validate alias is in range 0x001–0xFFF, return NULL if not
     * -# Validate node_id is in range 0x000000000001–0xFFFFFFFFFFFF, return NULL if not
     * -# Iterate through all ALIAS_MAPPING_BUFFER_DEPTH entries
     * -# On first empty slot (alias == 0) or matching Node ID: store alias and
     *    node_id, return pointer to that entry
     * -# If no slot found, return NULL (buffer full)
     *
     * Use cases:
     * - Storing a newly allocated alias during node login
     * - Updating an alias after conflict resolution
     * - Recording remote node aliases learned from AMD frames
     *
     * @verbatim
     * @param alias    12-bit CAN alias (valid range: 0x001–0xFFF).
     * @param node_id  48-bit OpenLCB Node ID (valid range: 0x000000000001–0xFFFFFFFFFFFF).
     * @endverbatim
     *
     * @return Pointer to the registered @ref alias_mapping_t entry, or NULL on failure.
     *
     * @warning Returns NULL when the buffer is full — caller MUST check before use.
     * @warning Out-of-range alias or node_id values return NULL.
     * @warning An existing Node ID entry will have its alias silently replaced.
     *
     * @see AliasMappings_unregister
     * @see AliasMappings_find_mapping_by_alias
     * @see AliasMappings_find_mapping_by_node_id
     */
alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id) {

    // Validate alias is within OpenLCB 12-bit range (0x001-0xFFF)
    if (alias == 0 || alias > 0xFFF) {

        return NULL;

    }

    // Validate node_id is within OpenLCB 48-bit range (non-zero and <= 48 bits)
    if (node_id == 0 || node_id > 0xFFFFFFFFFFFFULL) {

        return NULL;

    }

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if ((_alias_mapping_info.list[i].alias == 0) || (_alias_mapping_info.list[i].node_id == node_id)) {

            _alias_mapping_info.list[i].alias = alias;
            _alias_mapping_info.list[i].node_id = node_id;

            return &_alias_mapping_info.list[i];

        }

    }

    return NULL;

}

    /**
     * @brief Removes the entry matching the given alias from the buffer.
     *
     * @details Algorithm:
     * -# Iterate through all ALIAS_MAPPING_BUFFER_DEPTH entries
     * -# On matching alias: clear all four fields and break
     * -# If no match found, return without action
     *
     * @verbatim
     * @param alias  12-bit CAN alias to remove.
     * @endverbatim
     *
     * @see AliasMappings_register
     * @see AliasMappings_flush
     */
void AliasMappings_unregister(uint16_t alias) {

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (_alias_mapping_info.list[i].alias == alias) {

            _alias_mapping_info.list[i].alias = 0;
            _alias_mapping_info.list[i].node_id = 0;
            _alias_mapping_info.list[i].is_duplicate = false;
            _alias_mapping_info.list[i].is_permitted = false;

            break;

        }

    }

}

    /**
     * @brief Finds the mapping entry for the given alias.
     *
     * @details Algorithm:
     * -# Validate alias is in range 0x001–0xFFF, return NULL if not
     * -# Iterate through all entries; return pointer on first match
     * -# Return NULL if no match found
     *
     * @verbatim
     * @param alias  12-bit CAN alias to search for.
     * @endverbatim
     *
     * @return Pointer to the matching @ref alias_mapping_t entry, or NULL if not found.
     *
     * @warning NULL is returned when not found — caller MUST check before use.
     *
     * @see AliasMappings_find_mapping_by_node_id
     */
alias_mapping_t *AliasMappings_find_mapping_by_alias(uint16_t alias) {

    if (alias == 0 || alias > 0xFFF) {

        return NULL;

    }

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (_alias_mapping_info.list[i].alias == alias) {

            return &_alias_mapping_info.list[i];

        }

    }

    return NULL;

}

    /**
     * @brief Finds the mapping entry for the given Node ID.
     *
     * @details Algorithm:
     * -# Validate node_id is in range 0x000000000001–0xFFFFFFFFFFFF, return NULL if not
     * -# Iterate through all entries; return pointer on first match
     * -# Return NULL if no match found
     *
     * @verbatim
     * @param node_id  48-bit OpenLCB Node ID to search for.
     * @endverbatim
     *
     * @return Pointer to the matching @ref alias_mapping_t entry, or NULL if not found.
     *
     * @warning NULL is returned when not found — caller MUST check before use.
     *
     * @see AliasMappings_find_mapping_by_alias
     */
alias_mapping_t *AliasMappings_find_mapping_by_node_id(node_id_t node_id) {

    if (node_id == 0 || node_id > 0xFFFFFFFFFFFFULL) {

        return NULL;

    }

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (_alias_mapping_info.list[i].node_id == node_id) {

            return &_alias_mapping_info.list[i];

        }

    }

    return NULL;

}

    /** @brief Clears all alias mappings and resets all flags.  Runtime equivalent of initialize(). */
void AliasMappings_flush(void) {

    _reset_mappings();

}
