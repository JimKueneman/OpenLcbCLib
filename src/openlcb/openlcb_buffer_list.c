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
 * @file openlcb_buffer_list.c
 * @brief Implementation of the linear search buffer list
 *
 * @details Implements a simple array-based list with linear search operations.
 * This data structure is optimized for small list sizes where the overhead of
 * more complex data structures (hash tables, trees) outweighs their benefits.
 *
 * Design rationale:
 * - Small expected list size (typically < 20 entries)
 * - Simple NULL-based free slot detection
 * - No memory allocation overhead
 * - Predictable worst-case behavior
 * - Easy to debug and verify
 *
 * Memory characteristics:
 * - Fixed size allocation at compile time
 * - No dynamic memory allocation
 * - Pointer storage only (4-8 bytes per slot)
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "openlcb_buffer_list.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_buffer_store.h"

/** @brief Static array of message pointers for the list */
static openlcb_msg_t *_openlcb_msg_buffer_list[LEN_MESSAGE_BUFFER];

    /**
     * @brief Initializes the OpenLcb Message Buffer List
     *
     * @details Algorithm:
     * Simple initialization by clearing all slots to NULL.
     * -# Iterate through all LEN_MESSAGE_BUFFER slots
     * -# Set each slot to NULL
     * -# Result: Empty list ready for use
     *
     * Use cases:
     * - Called once during application startup
     * - Required before any list operations
     * - Must be called after OpenLcbBufferStore_initialize()
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call after OpenLcbBufferStore_initialize()
     *
     * @see OpenLcbBufferStore_initialize - Must be called first
     */
void OpenLcbBufferList_initialize(void) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        _openlcb_msg_buffer_list[i] = NULL;

    }

}

    /**
     * @brief Inserts a new OpenLcb message into the buffer at the first open slot
     *
     * @details Algorithm:
     * Linear search for first free slot, then store pointer.
     * -# Iterate from index 0 to LEN_MESSAGE_BUFFER-1
     * -# Check if _openlcb_msg_buffer_list[i] is NULL (free slot)
     * -# If NULL found:
     *    - Store new_msg at that index
     *    - Return new_msg (success)
     * -# If loop completes without finding NULL:
     *    - Return NULL (list full)
     *
     * Use cases:
     * - Storing first frame of multi-frame message
     * - Tracking in-progress operations
     * - Managing messages needing attribute lookup
     *
     * @verbatim
     * @param new_msg Pointer to message from buffer store (should NOT be NULL)
     * @endverbatim
     *
     * @return Pointer to message on success, NULL if list full
     *
     * @warning Returns NULL when list full
     * @warning NOT thread-safe
     *
     * @attention Check return value for NULL
     * @attention Caller retains ownership
     *
     * @remark If frequently returning NULL, increase LEN_MESSAGE_BUFFER
     *
     * @see OpenLcbBufferStore_allocate_buffer - Allocate before adding
     * @see OpenLcbBufferList_release - Remove from list
     */
openlcb_msg_t *OpenLcbBufferList_add(openlcb_msg_t *new_msg) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (!_openlcb_msg_buffer_list[i]) {

            _openlcb_msg_buffer_list[i] = new_msg;

            return new_msg;

        }

    }

    return NULL;
}

    /**
     * @brief Searches the buffer for a message matching the passed parameters
     *
     * @details Algorithm:
     * Linear search with triple condition matching.
     * -# Iterate from index 0 to LEN_MESSAGE_BUFFER-1
     * -# Skip NULL slots (no message stored)
     * -# For each non-NULL slot, check if ALL three conditions match:
     *    - dest_alias matches
     *    - source_alias matches  
     *    - mti matches
     * -# If all match, return pointer immediately
     * -# If loop completes without match, return NULL
     *
     * Use cases:
     * - Finding partial multi-frame message by source/dest/type
     * - Looking up message for frame assembly
     * - Verifying message existence
     *
     * @verbatim
     * @param source_alias CAN alias of sending node
     * @param dest_alias CAN alias of receiving node
     * @param mti Message Type Indicator code
     * @endverbatim
     *
     * @return Pointer to first matching message, or NULL if not found
     *
     * @warning Returns NULL if no match
     * @warning Does NOT remove from list
     * @warning NOT thread-safe
     *
     * @attention All three parameters must match
     * @attention Returns first match only
     *
     * @note Early exit on first match
     *
     * @see OpenLcbBufferList_add - How messages get into list
     * @see OpenLcbBufferList_release - Remove after finding
     */
openlcb_msg_t *OpenLcbBufferList_find(uint16_t source_alias, uint16_t dest_alias, uint16_t mti) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (_openlcb_msg_buffer_list[i]) {

            if ((_openlcb_msg_buffer_list[i]->dest_alias == dest_alias) &&
                    (_openlcb_msg_buffer_list[i]->source_alias == source_alias) &&
                    (_openlcb_msg_buffer_list[i]->mti == mti)) {

                return _openlcb_msg_buffer_list[i];

            }

        }

    }

    return NULL;
}

    /**
     * @brief Removes an OpenLcb message from the buffer list
     *
     * @details Algorithm:
     * Linear search for matching pointer, then NULL that slot.
     * -# If msg is NULL, return NULL immediately
     * -# Iterate from index 0 to LEN_MESSAGE_BUFFER-1
     * -# Compare each slot's pointer with msg
     * -# If match found:
     *    - Set slot to NULL (remove from list)
     *    - Return msg pointer
     * -# If loop completes without match:
     *    - Return NULL (not found)
     *
     * Use cases:
     * - Removing completed multi-frame message
     * - Cleaning up before freeing message
     * - Removing message from tracking
     *
     * @verbatim
     * @param msg Pointer to message to remove (can be NULL)
     * @endverbatim
     *
     * @return Pointer to message if found and removed, NULL otherwise
     *
     * @warning Does NOT free message buffer
     * @warning Caller must call free_buffer() separately
     * @warning NOT thread-safe
     *
     * @attention Safe to call with NULL
     * @attention Message not freed, only removed from list
     *
     * @see OpenLcbBufferStore_free_buffer - Free after releasing
     * @see OpenLcbBufferList_find - Find before releasing
     */
openlcb_msg_t *OpenLcbBufferList_release(openlcb_msg_t *msg) {

    if (!msg) {

        return NULL;

    }

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (_openlcb_msg_buffer_list[i] == msg) {

            _openlcb_msg_buffer_list[i] = NULL;

            return msg;

        }

    }

    return NULL;
}

    /**
     * @brief Returns a pointer to the message at the passed index
     *
     * @details Algorithm:
     * Simple bounds checking followed by array access.
     * -# Check if index >= LEN_MESSAGE_BUFFER
     * -# If out of bounds, return NULL
     * -# Otherwise, return _openlcb_msg_buffer_list[index]
     *    (may be NULL if slot is empty)
     *
     * Use cases:
     * - Iterating through all entries
     * - Direct access by index
     * - Debugging and diagnostics
     *
     * @verbatim
     * @param index Index of message requested (0 to LEN_MESSAGE_BUFFER-1)
     * @endverbatim
     *
     * @return Pointer to message at index, or NULL if out of bounds or empty
     *
     * @warning Returns NULL for out-of-bounds
     * @warning Returns NULL for empty slots
     * @warning Cannot distinguish out-of-bounds from empty
     * @warning NOT thread-safe
     *
     * @attention Check return value for NULL
     * @attention Valid range: 0 to LEN_MESSAGE_BUFFER-1
     *
     * @note Does not remove from list
     *
     * @see OpenLcbBufferList_find - Search by attributes instead
     */
openlcb_msg_t *OpenLcbBufferList_index_of(uint16_t index) {

    if (index >= LEN_MESSAGE_BUFFER) {

        return NULL;

    }

    return _openlcb_msg_buffer_list[index];
}

    /**
     * @brief Tests if there are any messages stored in the buffer list
     *
     * @details Algorithm:
     * Linear scan with early exit optimization.
     * -# Iterate from index 0 to LEN_MESSAGE_BUFFER-1
     * -# If any slot is not NULL:
     *    - Return false immediately (not empty)
     * -# If loop completes (all slots NULL):
     *    - Return true (empty)
     *
     * Use cases:
     * - Checking for in-progress multi-frame messages
     * - Idle detection
     * - Shutdown validation
     * - Memory leak detection
     *
     * @return True if all slots NULL (empty), false if any message present
     *
     * @note Non-destructive
     * @note Early exit on first non-NULL
     *
     * @see OpenLcbBufferList_add - Causes to return false
     * @see OpenLcbBufferList_release - May cause to return true
     */
bool OpenLcbBufferList_is_empty(void) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (_openlcb_msg_buffer_list[i] != NULL) {

            return false;

        }

    }

    return true;
}
