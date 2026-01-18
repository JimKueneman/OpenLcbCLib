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
 * @brief Implementation of the Alias/NodeID mapping buffer
 *
 * @details This module implements a fixed-size buffer that maintains the bidirectional
 * mapping between OpenLCB 48-bit Node IDs and their corresponding 12-bit CAN aliases.
 * The implementation uses a simple linear search strategy suitable for small to medium
 * buffer sizes (typically 8-16 entries).
 *
 * Internal design:
 * - Single static buffer instance (_alias_mapping_info)
 * - Linear array storage (not hash table or tree)
 * - Empty slots marked by alias = 0 and node_id = 0
 * - First-fit allocation strategy
 * - Node ID uniqueness enforced (one alias per Node ID)
 * - Duplicate alias detection flag support
 *
 * Performance characteristics:
 * - Register: O(n) search for empty slot or existing Node ID
 * - Find by alias: O(n) linear search
 * - Find by Node ID: O(n) linear search
 * - Unregister: O(n) search with early termination
 * - Initialize/Flush: O(n) iteration
 *
 * Thread safety:
 * - NOT thread-safe. All functions assume single-threaded access.
 * - Caller must provide external synchronization if needed.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "alias_mappings.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

/** @brief Static storage for the alias mapping buffer and control flags */
static alias_mapping_info_t _alias_mapping_info;

/**
 * @brief Internal function to reset all mappings to default state
 *
 * @details Algorithm:
 * -# Iterate through all ALIAS_MAPPING_BUFFER_DEPTH entries in the buffer
 * -# For each entry, set alias to 0 (empty marker)
 * -# For each entry, set node_id to 0 (empty marker)
 * -# For each entry, set is_duplicate flag to false
 * -# For each entry, set is_permitted flag to false
 * -# Clear the global has_duplicate_alias flag to false
 *
 * This function is used by both initialization and flush operations to ensure
 * consistent buffer state. Using 0 for alias and node_id allows the search
 * functions to quickly identify empty slots.
 *
 * @note This is an internal static function and cannot be called from outside
 *       this compilation unit.
 *
 * @remark Time complexity is O(n) where n = ALIAS_MAPPING_BUFFER_DEPTH.
 *         For typical values (8-16), this executes in microseconds.
 *
 * @see AliasMappings_initialize - Public initialization function
 * @see AliasMappings_flush - Public flush function
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

/**
 * @brief Initializes the Alias Mapping buffers
 *
 * @details Algorithm:
 * -# Call internal _reset_mappings() function
 * -# This clears all buffer entries to default state
 * -# Clears the duplicate alias flag
 * -# Buffer is ready for node registration
 *
 * This function must be called during application initialization before any
 * CAN communication begins. It prepares the buffer for storing alias mappings
 * as nodes log in to the network.
 *
 * Use cases:
 * - Application initialization sequence
 * - System reset or reinitialization
 * - Test harness setup
 *
 * @warning MUST be called exactly once during application startup before any
 *          node operations. Calling during active operations will lose all
 *          existing alias mappings and cause communication failures.
 *
 * @attention Call this function before AliasMappings_register() or any other
 *            alias mapping operations.
 *
 * @attention This function is NOT thread-safe. Call only during single-threaded
 *            initialization phase.
 *
 * @remark Execution time is O(n) where n = ALIAS_MAPPING_BUFFER_DEPTH.
 *
 * @see _reset_mappings - Internal reset implementation
 * @see AliasMappings_flush - Runtime clear operation
 */
void AliasMappings_initialize(void) {

    _reset_mappings();

}

/**
 * @brief Allows access to the Alias Mapping Buffer itself
 *
 * @details Algorithm:
 * -# Return pointer to static _alias_mapping_info structure
 * -# No validation or error checking needed (pointer always valid)
 *
 * This provides direct access to the internal buffer structure for debugging,
 * diagnostics, or advanced inspection operations. The structure contains the
 * array of all alias/Node ID mappings plus the duplicate alias flag.
 * While direct modification is possible, callers should use the provided API
 * functions to maintain data consistency.
 *
 * Use cases:
 * - Debugging alias allocation issues
 * - Diagnostic logging of buffer state
 * - Advanced buffer inspection for testing
 * - Checking duplicate alias flag status
 *
 * @return Pointer to the alias mapping info structure (never returns NULL)
 *
 * @warning The returned pointer points to static internal data. Modifications should
 *          only be done through the provided API functions to maintain consistency.
 *
 * @attention The structure remains valid for the lifetime of the program.
 *            Do not cache individual mapping pointers as they may be invalidated
 *            by unregister or flush operations.
 *
 * @note This function always succeeds and never returns NULL.
 *
 * @remark Execution time is O(1) - immediate return of pointer.
 *
 * @see alias_mapping_info_t - Structure definition
 * @see AliasMappings_set_has_duplicate_alias_flag - Sets the duplicate flag
 * @see AliasMappings_clear_has_duplicate_alias_flag - Clears the duplicate flag
 */
alias_mapping_info_t *AliasMappings_get_alias_mapping_info(void) {

    return &_alias_mapping_info;

}

/**
 * @brief Sets a flag that tells the main loop that a received message has
 * been found to be using an Alias we have reserved
 *
 * @details Algorithm:
 * -# Set the has_duplicate_alias flag in _alias_mapping_info to true
 * -# No validation or error checking needed
 *
 * This function is called when the CAN message reception code detects that an
 * incoming CID, RID, or AMD frame is using an alias that matches one we have
 * locally reserved. The flag signals the main state machine to initiate alias
 * conflict resolution (selecting a new alias and restarting login).
 *
 * Use cases:
 * - During CAN message reception when incoming CID/RID frames match our alias
 * - When processing AMD frames that conflict with locally reserved aliases
 * - During alias conflict resolution in the login state machine
 *
 * @attention This flag must be checked and cleared by the main loop after taking
 *            appropriate action (typically restarting alias allocation).
 *
 * @attention Setting this flag does NOT automatically trigger alias reallocation.
 *            The main state machine must detect and handle the flag.
 *
 * @note The flag is cleared by AliasMappings_initialize() and AliasMappings_flush().
 *
 * @remark Execution time is O(1) - single flag assignment.
 *
 * @see AliasMappings_get_alias_mapping_info - Access the flag for reading
 * @see alias_mapping_info_t::has_duplicate_alias - The flag field
 */
void AliasMappings_set_has_duplicate_alias_flag(void) {

    _alias_mapping_info.has_duplicate_alias = true;

}

/**
 * @brief Clears the duplicate alias flag
 *
 * @details Algorithm:
 * -# Set the has_duplicate_alias flag in _alias_mapping_info to false
 * -# No validation or error checking needed
 *
 * This function provides proper encapsulation for clearing the duplicate alias flag
 * after the main loop has handled an alias conflict. This is cleaner than requiring
 * direct access to the internal structure fields.
 *
 * Use cases:
 * - After successfully handling alias conflict and selecting new alias
 * - After restarting login sequence with new alias
 * - In test cleanup to reset state
 *
 * @attention Call this only after the alias conflict has been fully resolved.
 *            Clearing prematurely could cause the conflict to be ignored.
 *
 * @note The flag is also cleared by AliasMappings_initialize() and AliasMappings_flush().
 *
 * @remark Execution time is O(1) - single flag assignment.
 *
 * @see AliasMappings_set_has_duplicate_alias_flag - Sets the flag
 * @see AliasMappings_get_alias_mapping_info - Access the flag for reading
 * @see alias_mapping_info_t::has_duplicate_alias - The flag field
 */
void AliasMappings_clear_has_duplicate_alias_flag(void) {

    _alias_mapping_info.has_duplicate_alias = false;

}

/**
 * @brief Registers a new Alias/NodeID pair
 *
 * @details Algorithm:
 * -# Validate alias is within valid OpenLCB range (0x001-0xFFF), return NULL if invalid
 * -# Validate node_id is within valid OpenLCB range (0x000000000001-0xFFFFFFFFFFFF), return NULL if invalid
 * -# Iterate through all ALIAS_MAPPING_BUFFER_DEPTH entries in the buffer
 * -# For each entry, check if it's empty (alias == 0) OR Node ID matches
 * -# If condition met:
 *    - Store the new alias value
 *    - Store the new node_id value
 *    - Return pointer to this entry
 * -# If loop completes without finding slot, return NULL (buffer full)
 *
 * The function implements two behaviors:
 * 1. If Node ID already exists: Update its alias (alias change/recovery)
 * 2. If empty slot found: Register new alias/Node ID pair
 *
 * This design ensures that each Node ID can only have one alias in the buffer,
 * which is critical for correct OpenLCB operation.
 *
 * Use cases:
 * - Registering a newly allocated alias during node login
 * - Updating an alias after conflict resolution
 * - Storing remote node alias/Node ID pairs learned from AMD frames
 *
 * @param alias The 12-bit CAN alias to store (valid range: 0x001-0xFFF, 0x000 reserved for empty)
 * @param node_id The 48-bit OpenLCB Node ID to associate with the alias
 *
 * @return Pointer to the newly registered alias_mapping_t entry, or NULL if buffer is full
 *
 * @warning Returns NULL when buffer is completely full. Caller MUST check return
 *          value before dereferencing. Dereferencing NULL will cause immediate crash.
 *
 * @warning Returns NULL if alias is invalid (0 or > 0xFFF). OpenLCB requires 12-bit
 *          aliases in range 0x001-0xFFF. Alias 0 is reserved for empty slots.
 *
 * @warning Returns NULL if node_id is invalid (0 or > 0xFFFFFFFFFFFF). OpenLCB Node IDs
 *          must be 48-bit values in range 0x000000000001-0xFFFFFFFFFFFF. Node ID 0 is reserved.
 *
 * @warning If the Node ID already exists, the OLD alias is silently replaced. This
 *          is correct behavior for alias updates but could mask programming errors.
 *
 * @attention Buffer capacity is ALIAS_MAPPING_BUFFER_DEPTH entries. Plan node count
 *            accordingly or handle registration failures gracefully.
 *
 * @attention The returned pointer remains valid until the entry is unregistered or
 *            the buffer is flushed. Do not cache pointers across these operations.
 *
 * @remark Time complexity is O(n) worst case where n = ALIAS_MAPPING_BUFFER_DEPTH.
 *         Best case is O(1) if first slot is available.
 *
 * @see AliasMappings_unregister - Removes a mapping
 * @see AliasMappings_find_mapping_by_alias - Finds existing mapping by alias
 * @see AliasMappings_find_mapping_by_node_id - Finds existing mapping by Node ID
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
 * @brief Deregisters an existing Alias/NodeID pair
 *
 * @details Algorithm:
 * -# Iterate through all ALIAS_MAPPING_BUFFER_DEPTH entries in the buffer
 * -# For each entry, check if alias matches the search value
 * -# If match found:
 *    - Set alias to 0 (empty marker)
 *    - Set node_id to 0 (empty marker)
 *    - Set is_duplicate flag to false
 *    - Set is_permitted flag to false
 *    - Break out of loop (stop searching)
 * -# If no match found, function returns without action
 *
 * The function uses early termination (break) after finding the first match,
 * which is correct since aliases should be unique in the buffer. Clearing all
 * fields ensures the slot can be reused by future register operations.
 *
 * Use cases:
 * - Cleaning up after node logout (AMR frame received)
 * - Removing mappings when nodes disconnect
 * - Test cleanup between test cases
 *
 * @param alias The 12-bit CAN alias to unregister
 *
 * @attention This function is safe to call with aliases that don't exist in the buffer.
 *            No error is generated in this case.
 *
 * @attention After unregistering, any pointers previously obtained from
 *            AliasMappings_register() or find functions for this entry become invalid.
 *
 * @note The function stops searching after finding the first match, so if duplicate
 *       aliases exist in the buffer (should never happen), only the first is removed.
 *
 * @remark Time complexity is O(n) worst case where n = ALIAS_MAPPING_BUFFER_DEPTH.
 *         Best case is O(1) if alias is in first slot. Average case is O(n/2).
 *
 * @see AliasMappings_register - Adds a mapping
 * @see AliasMappings_flush - Removes all mappings
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
 * @brief Finds an Alias/NodeID pair that matches the Alias passed
 *
 * @details Algorithm:
 * -# Iterate through all ALIAS_MAPPING_BUFFER_DEPTH entries in the buffer
 * -# For each entry, check if alias field matches the search value
 * -# If match found, immediately return pointer to that entry
 * -# If loop completes without match, return NULL
 *
 * The function performs a simple linear search. Empty slots (alias = 0) will
 * not match unless searching for alias 0 (which would be a programming error).
 * This is the primary mechanism for resolving CAN aliases to Node IDs during
 * message reception.
 *
 * Use cases:
 * - Resolving source alias to Node ID in received messages
 * - Validating alias uniqueness before allocation
 * - Looking up node information during message routing
 *
 * @param alias The 12-bit CAN alias to search for
 *
 * @return Pointer to the matching alias_mapping_t entry, or NULL if not found
 *
 * @warning Returns NULL if alias not found. Caller MUST check return value before
 *          dereferencing. Dereferencing NULL will cause immediate crash.
 *
 * @attention Alias 0 is reserved for "empty slot" and will never be found by this
 *            function (entries with alias = 0 are skipped).
 *
 * @note This is a linear search with O(n) time complexity where n = ALIAS_MAPPING_BUFFER_DEPTH.
 *       For typical buffer sizes (8-16 entries), this is acceptably fast.
 *
 * @remark Time complexity is O(n) worst case. Best case is O(1) if alias is in
 *         first slot. Average case is O(n/2).
 *
 * @see AliasMappings_find_mapping_by_node_id - Reverse lookup by Node ID
 * @see AliasMappings_register - Adds a mapping
 */
alias_mapping_t *AliasMappings_find_mapping_by_alias(uint16_t alias) {

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (_alias_mapping_info.list[i].alias == alias) {

            return &_alias_mapping_info.list[i];

        }

    }

    return NULL;

}

/**
 * @brief Finds an Alias/NodeID pair that matches the NodeID passed
 *
 * @details Algorithm:
 * -# Iterate through all ALIAS_MAPPING_BUFFER_DEPTH entries in the buffer
 * -# For each entry, check if node_id field matches the search value
 * -# If match found, immediately return pointer to that entry
 * -# If loop completes without match, return NULL
 *
 * The function performs a simple linear search. Empty slots (node_id = 0) will
 * not match unless searching for node_id 0 (which would be a programming error).
 * This is used to find what alias is currently assigned to a local or remote node.
 *
 * Use cases:
 * - Looking up the current alias for a local node before sending messages
 * - Checking if a Node ID is already registered before login
 * - Finding node entries for updates during alias reallocation
 *
 * @param node_id The 48-bit OpenLCB Node ID to search for
 *
 * @return Pointer to the matching alias_mapping_t entry, or NULL if not found
 *
 * @warning Returns NULL if Node ID not found. Caller MUST check return value before
 *          dereferencing. Dereferencing NULL will cause immediate crash.
 *
 * @attention Node ID 0 is reserved and will never be found by this function
 *            (entries with node_id = 0 are considered empty).
 *
 * @note This is a linear search with O(n) time complexity where n = ALIAS_MAPPING_BUFFER_DEPTH.
 *       For typical buffer sizes (8-16 entries), this is acceptably fast.
 *
 * @remark Time complexity is O(n) worst case. Best case is O(1) if Node ID is in
 *         first slot. Average case is O(n/2).
 *
 * @see AliasMappings_find_mapping_by_alias - Reverse lookup by alias
 * @see AliasMappings_register - Adds a mapping
 */
alias_mapping_t *AliasMappings_find_mapping_by_node_id(node_id_t node_id) {

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (_alias_mapping_info.list[i].node_id == node_id) {

            return &_alias_mapping_info.list[i];

        }

    }

    return NULL;

}

/**
 * @brief Releases all stored Alias Mapping pairs
 *
 * @details Algorithm:
 * -# Call internal _reset_mappings() function
 * -# This clears all buffer entries to default state
 * -# Clears the duplicate alias flag
 * -# Buffer is ready for new registrations
 *
 * This function is identical in implementation to AliasMappings_initialize() but
 * serves a different semantic purpose. Initialize is for startup, flush is for
 * runtime clearing of the buffer. Calling this during active communication will
 * break all existing alias mappings.
 *
 * Use cases:
 * - System-wide reset or reinitialization
 * - Clearing state between test cases
 * - Emergency recovery from corrupted buffer state
 * - Batch logout of all nodes
 *
 * @warning This invalidates ALL pointers previously returned by register or find
 *          functions. Dereferencing these pointers after flush causes undefined behavior.
 *
 * @warning Any nodes that were using these aliases will lose their CAN bus identity
 *          and must re-login with new aliases to communicate.
 *
 * @attention This function is NOT thread-safe. Ensure no other code is accessing
 *            the buffer during flush operation.
 *
 * @note Functionally identical to AliasMappings_initialize() but semantically different.
 *       Use initialize() at startup, use flush() to clear at runtime.
 *
 * @remark Execution time is O(n) where n = ALIAS_MAPPING_BUFFER_DEPTH.
 *
 * @see AliasMappings_initialize - Initial setup
 * @see AliasMappings_unregister - Removes a single mapping
 * @see _reset_mappings - Internal reset implementation
 */
void AliasMappings_flush(void) {

    _reset_mappings();

}
