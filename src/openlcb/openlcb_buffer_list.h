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
 * @file openlcb_buffer_list.h
 * @brief Linear search buffer list for OpenLcb/LCC message structures
 *
 * @details This module provides a linear array-based list for storing and searching
 * OpenLCB message pointers. Unlike the FIFO, this list supports random access by
 * index and searching by message attributes (source alias, destination alias, and MTI).
 *
 * Key features:
 * - Linear array with NULL slots indicating free positions
 * - Stores pointers only (messages allocated from buffer store)
 *
 * Primary use cases:
 * - Multi-frame message assembly (tracking partial messages)
 * - Finding in-progress messages by source/dest/MTI combination
 * - Holding messages that don't fit pure FIFO semantics
 * - Managing messages that need lookup by attributes rather than just FIFO order
 *
 * Typical workflow:
 * 1. Receive first frame of multi-frame message
 * 2. Allocate buffer from store
 * 3. Add to list
 * 4. On subsequent frames, find by source/dest/MTI
 * 5. Complete assembly
 * 6. Release from list
 * 7. Free buffer when done
 *
 * This list must be initialized after OpenLcbBufferStore_initialize() but can be
 * used independently of OpenLcbBufferFifo.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_BUFFER_LIST__
#define __OPENLCB_OPENLCB_BUFFER_LIST__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLcb Message Buffer List
     *
     * @details Prepares the list for use by setting all array positions to NULL,
     * indicating all slots are free and available for message storage.
     *
     * The list uses NULL pointers to indicate free slots, so initialization simply
     * clears the entire array. Messages can then be added to any NULL slot.
     *
     * Use cases:
     * - Called once during application startup
     * - Required before any list add/find/release operations
     * - Must be called after OpenLcbBufferStore_initialize()
     *
     * @warning MUST be called exactly once during application initialization before
     *          any list operations. Calling multiple times will discard all stored messages.
     * @warning NOT thread-safe - must be called during single-threaded initialization
     *
     * @attention Call this function after OpenLcbBufferStore_initialize()
     * @attention Call before using OpenLcbBufferList_add(), find(), release(), or index_of()
     *
     * @see OpenLcbBufferStore_initialize - Must be called first
     * @see OpenLcbBufferList_add - Adds messages to initialized list
     * @see OpenLcbBufferList_find - Searches initialized list
     */
    extern void OpenLcbBufferList_initialize(void);

    /**
     * @brief Inserts a new OpenLcb message into the buffer at the first open slot
     *
     * @details Performs a linear search through the list to find the first NULL slot
     * and stores the message pointer there. This allows the message to be later
     * retrieved by its attributes (source/dest/MTI) or by its index.
     *
     * Use cases:
     * - Storing first frame of multi-frame message for assembly
     * - Holding messages that need attribute-based lookup
     * - Managing messages in non-FIFO patterns
     * - Tracking in-progress operations
     *
     * @param new_msg Pointer to a message allocated from OpenLcbBufferStore (must NOT be NULL)
     * @return Pointer to the message on success, or NULL if list is full
     *
     * @warning Passing NULL will store NULL in list (though function will store it if passed)
     * @warning Returns NULL when list is full - caller MUST check return value
     * @warning List full means all LEN_MESSAGE_BUFFER slots occupied
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL before assuming success
     * @attention Caller retains ownership - must call release() and free() later
     * @attention List stores pointers only - does not copy message data
     *
     * @remark For frequently-full lists, consider increasing LEN_MESSAGE_BUFFER
     *
     * @see OpenLcbBufferStore_allocate_buffer - Allocate message before adding
     * @see OpenLcbBufferList_find - Find message by attributes
     * @see OpenLcbBufferList_release - Remove message from list
     * @see OpenLcbBufferStore_free_buffer - Free message after releasing
     */
    extern openlcb_msg_t *OpenLcbBufferList_add(openlcb_msg_t *new_msg);

    /**
     * @brief Searches the buffer for a message matching the passed parameters
     *
     * @details Performs a linear search through the list looking for a message
     * that matches all three criteria: source_alias, dest_alias, and mti. This is
     * typically used to find the partial message for a specific multi-frame transfer.
     *
     * Use cases:
     * - Finding partial multi-frame message to append additional frames
     * - Looking up message by source/destination/type combination
     * - Verifying if a message with specific attributes exists
     *
     * @param source_alias The CAN alias of the node sending the message frame
     * @param dest_alias The CAN alias of the node receiving the message frame
     * @param mti The Message Type Indicator (MTI) code of the message frame type
     * @return Pointer to first matching message, or NULL if no match found
     *
     * @warning Returns NULL if no match found - caller MUST check for NULL
     * @warning Does NOT remove message from list - use release() for that
     * @warning NOT thread-safe
     *
     * @attention All three parameters must match for a message to be returned
     * @attention Returns first match only (if duplicates exist, others not returned)
     * @attention Returned pointer is still in the list until release() is called
     *
     * @note Skips NULL entries during search
     *
     * @remark Common pattern: find() to locate message, update it, then release() when complete
     *
     * @see OpenLcbBufferList_add - How messages are added to the list
     * @see OpenLcbBufferList_release - Remove message from list after finding
     * @see OpenLcbBufferList_index_of - Alternative lookup by index
     */
    extern openlcb_msg_t *OpenLcbBufferList_find(uint16_t source_alias, uint16_t dest_alias, uint16_t mti);

    /**
     * @brief Removes an OpenLcb message from the buffer list
     *
     * @details Searches the list for the specified message pointer and sets that
     * slot to NULL, effectively removing it from the list. The message itself is NOT
     * freed - the caller must call OpenLcbBufferStore_free_buffer() separately.
     *
     * Use cases:
     * - Removing completed multi-frame message from assembly tracking
     * - Cleaning up after message processing
     * - Removing message before freeing it
     *
     * @param msg Pointer to message to be removed from list (can be NULL)
     * @return Pointer to the message if found and removed, or NULL if msg was NULL or not in list
     *
     * @warning Message is NOT freed, only removed from list
     * @warning Caller must call OpenLcbBufferStore_free_buffer() to actually free the message
     * @warning Returns NULL if message not found in list
     * @warning NOT thread-safe
     *
     * @attention Safe to call with NULL (returns NULL)
     * @attention Caller responsible for freeing the returned message
     * @attention Does not free the message buffer, only removes from list
     *
     * @note Common pattern: release() then free() the message
     *
     * @see OpenLcbBufferList_add - How messages are added
     * @see OpenLcbBufferList_find - Find message before releasing
     * @see OpenLcbBufferStore_free_buffer - Free message after releasing
     */
    extern openlcb_msg_t *OpenLcbBufferList_release(openlcb_msg_t *msg);

    /**
     * @brief Returns a pointer to the message at the passed index
     *
     * @details Provides direct array access to the list by index. The returned pointer
     * may be NULL if that slot is empty, or a valid message pointer if occupied.
     *
     * Use cases:
     * - Iterating through all list entries
     * - Direct access when index is known
     * - Debugging and diagnostics
     *
     * @param index Index of the message requested (0 to LEN_MESSAGE_BUFFER-1)
     * @return Pointer to message at index, or NULL if:
     *         - index is out of bounds (>= LEN_MESSAGE_BUFFER)
     *         - slot at index is empty (NULL)
     *
     * @warning Returns NULL for out-of-bounds index
     * @warning Returns NULL for empty slots
     * @warning Caller cannot distinguish between out-of-bounds and empty slot
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL
     * @attention Valid index range is 0 to LEN_MESSAGE_BUFFER-1
     * @attention Returned pointer may be NULL even for valid index
     *
     * @note Does not remove message from list
     *
     * @remark Common pattern: loop from 0 to LEN_MESSAGE_BUFFER-1, check each index_of() result for NULL
     *
     * @see OpenLcbBufferList_is_empty - Check if entire list is empty
     * @see OpenLcbBufferList_find - Search by attributes instead of index
     */
    extern openlcb_msg_t *OpenLcbBufferList_index_of(uint16_t index);

    /**
     * @brief Tests if there are any messages stored in the buffer list
     *
     * @details Scans the entire list to determine if all slots are NULL (empty)
     * or if at least one message pointer exists.
     *
     * Use cases:
     * - Checking if any multi-frame messages are in progress
     * - Idle detection
     * - Shutdown validation (ensure list is empty before shutdown)
     * - Memory leak detection
     *
     * @return True if list is empty (all slots NULL), false if at least one message present
     *
     * @note Non-destructive operation - does not modify list
     * @note Early exit on first non-NULL entry found
     *
     * @see OpenLcbBufferList_add - Causes is_empty() to return false
     * @see OpenLcbBufferList_release - May cause is_empty() to return true
     * @see OpenLcbBufferList_index_of - Alternative to check individual slots
     */
    extern bool OpenLcbBufferList_is_empty(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_LIST__ */
