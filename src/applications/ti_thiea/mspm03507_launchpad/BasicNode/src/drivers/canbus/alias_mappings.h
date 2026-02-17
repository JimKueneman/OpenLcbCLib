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
 * @file alias_mappings.h
 * @brief Alias/NodeID mapping buffer for tracking internal node aliases
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 *
 * @details This module provides a buffer for storing and managing the mapping between
 * OpenLCB 48-bit Node IDs and their corresponding 12-bit CAN aliases. The buffer is used
 * to track which aliases are currently in use and which Node IDs they represent, enabling
 * bidirectional lookup during message processing.
 *
 * Key features:
 * - Fixed-size buffer (ALIAS_MAPPING_BUFFER_DEPTH entries)
 * - Bidirectional lookup (find by alias or by Node ID)
 * - Duplicate alias detection support
 * - Alias permission tracking
 * - Buffer flush capability
 *
 * The buffer is typically used during:
 * - Node login and alias allocation
 * - Incoming message processing (alias to Node ID resolution)
 * - Outgoing message preparation (Node ID to alias resolution)
 * - Duplicate alias conflict detection and recovery
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_ALIAS_MAPPINGS__
#define __DRIVERS_CANBUS_ALIAS_MAPPINGS__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the Alias Mapping buffers
     *
     * @details Clears all entries in the alias mapping buffer and resets all flags
     * to their initial state. This must be called during application startup before
     * any nodes attempt to log in or register aliases.
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
     * @see AliasMappings_flush - Clears mappings after initialization
     */
    extern void AliasMappings_initialize(void);

    /**
     * @brief Allows access to the Alias Mapping Buffer itself
     *
     * @details Provides direct read-only access to the internal alias mapping info structure,
     * which contains the array of all alias/Node ID pairs and the duplicate alias flag.
     * This is primarily used for debugging, diagnostics, or advanced operations that
     * need to inspect the entire buffer state.
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
     * @see alias_mapping_info_t - Structure definition
     * @see AliasMappings_set_has_duplicate_alias_flag - Sets the duplicate flag
     * @see AliasMappings_clear_has_duplicate_alias_flag - Clears the duplicate flag
     */
    extern alias_mapping_info_t *AliasMappings_get_alias_mapping_info(void);

    /**
     * @brief Sets a flag that tells the main loop that a received message has
     * been found to be using an Alias we have reserved
     *
     * @details Sets the has_duplicate_alias flag in the mapping info structure to true.
     * This flag signals the main state machine loop that an alias conflict has been
     * detected and the node needs to select a new alias and restart the login process.
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
     * @see AliasMappings_clear_has_duplicate_alias_flag - Clears the flag
     * @see AliasMappings_get_alias_mapping_info - Access the flag for reading
     * @see alias_mapping_info_t::has_duplicate_alias - The flag field
     */
    extern void AliasMappings_set_has_duplicate_alias_flag(void);

    /**
     * @brief Clears the duplicate alias flag
     *
     * @details Clears the has_duplicate_alias flag in the mapping info structure to false.
     * This should be called by the main loop after handling an alias conflict and
     * successfully reallocating a new alias. Provides proper encapsulation instead of
     * requiring direct structure access.
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
     * @see AliasMappings_set_has_duplicate_alias_flag - Sets the flag
     * @see AliasMappings_get_alias_mapping_info - Access the flag for reading
     */
    extern void AliasMappings_clear_has_duplicate_alias_flag(void);

    /**
     * @brief Registers a new Alias/NodeID pair
     *
     * @details Stores an alias and Node ID mapping in the buffer. The function searches
     * for an available slot or an existing entry with the same Node ID. If the Node ID
     * already exists, the old alias is replaced with the new one (alias update). If an
     * empty slot is found, the new pair is stored there. Returns NULL if buffer is full.
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
     * @warning Returns NULL if CAN Alias is outside valid 12-bit range (0 or > 0xFFF).
     *          OpenLCB CAN protocol requires aliases in range 0x001-0xFFF. Zero is
     *          reserved to mark empty buffer slots.
     *
     * @warning Returns NULL if OpenLCB Node ID exceeds 48-bit range (0 or > 0xFFFFFFFFFFFF).
     *          Valid OpenLCB Node IDs are 48-bit values from 0x000000000001-0xFFFFFFFFFFFF.
     *          Zero is reserved as "no valid Node ID assigned".
     *
     * @warning If an OpenLCB Node ID already exists in the buffer, its previously registered
     *          CAN Alias is silently replaced with the new one. This is correct behavior for
     *          alias updates after conflict resolution but could mask programming errors.
     *
     * @attention Buffer capacity is ALIAS_MAPPING_BUFFER_DEPTH entries. Plan node count
     *            accordingly or handle registration failures gracefully.
     *
     * @attention The returned pointer remains valid until the entry is unregistered or
     *            the buffer is flushed. Do not cache pointers across these operations.
     *
     * @note Registering with CAN Alias = 0 will create an entry but it won't be findable by
     *       AliasMappings_find_mapping_by_alias() since 0 is reserved to mark empty slots.
     *       Per OpenLCB spec, aliases must be in range 0x001-0xFFF.
     *
     * @see AliasMappings_unregister - Removes a mapping
     * @see AliasMappings_find_mapping_by_alias - Finds existing mapping by alias
     * @see AliasMappings_find_mapping_by_node_id - Finds existing mapping by Node ID
     */
    extern alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id);

    /**
     * @brief Deregisters an existing Alias/NodeID pair
     *
     * @details Removes an alias mapping from the buffer by searching for the specified
     * alias and clearing its entry. All fields (alias, node_id, is_duplicate, is_permitted)
     * are reset to their default values. If the alias is not found, the function does
     * nothing (safe to call with non-existent aliases).
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
     * @see AliasMappings_register - Adds a mapping
     * @see AliasMappings_flush - Removes all mappings
     */
    extern void AliasMappings_unregister(uint16_t alias);

    /**
     * @brief Finds an Alias/NodeID pair that matches the Alias passed
     *
     * @details Performs a linear search through the buffer looking for an entry where
     * the alias field matches the provided value. Returns a pointer to the matching
     * entry or NULL if not found. This is used to resolve an alias to its corresponding
     * Node ID during message processing.
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
     * @attention CAN Alias 0 is reserved to mark empty buffer slots and will never match.
     *            Per OpenLCB CAN protocol, valid aliases are 0x001-0xFFF. Entries with
     *            CAN Alias = 0 are skipped during search.
     *
     * @note Linear search through all entries. For typical buffer sizes (8-16 entries),
     *       this is acceptably fast.
     *
     * @see AliasMappings_find_mapping_by_node_id - Reverse lookup by Node ID
     * @see AliasMappings_register - Adds a mapping
     */
    extern alias_mapping_t *AliasMappings_find_mapping_by_alias(uint16_t alias);

    /**
     * @brief Finds an Alias/NodeID pair that matches the NodeID passed
     *
     * @details Performs a linear search through the buffer looking for an entry where
     * the node_id field matches the provided value. Returns a pointer to the matching
     * entry or NULL if not found. This is used to find what alias is currently assigned
     * to a specific Node ID.
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
     * @attention OpenLCB Node ID 0 is reserved as "no valid Node ID assigned" and will
     *            never match. Entries with Node ID = 0 are considered empty buffer slots
     *            and are skipped during search.
     *
     * @note Linear search through all entries. For typical buffer sizes (8-16 entries),
     *       this is acceptably fast.
     *
     * @see AliasMappings_find_mapping_by_alias - Reverse lookup by alias
     * @see AliasMappings_register - Adds a mapping
     */
    extern alias_mapping_t *AliasMappings_find_mapping_by_node_id(node_id_t node_id);

    /**
     * @brief Releases all stored Alias Mapping pairs
     *
     * @details Clears all entries in the alias mapping buffer by calling the internal
     * reset function. All aliases, Node IDs, and flags are reset to their default values.
     * The duplicate alias flag is also cleared. This is typically used during system
     * reset or when all nodes need to be logged out.
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
     * @see AliasMappings_initialize - Initial setup
     * @see AliasMappings_unregister - Removes a single mapping
     */
    extern void AliasMappings_flush(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_ALIAS_MAPPINGS__ */
