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
 * @file openlcb_buffer_fifo.h
 * @brief FIFO buffer implementation for OpenLcb/LCC message structures
 *
 * @details This module provides a First-In-First-Out (FIFO) queue for OpenLCB message
 * pointers using a circular buffer algorithm. Messages are allocated from the
 * OpenLcbBufferStore and their pointers are queued in this FIFO for ordered processing.
 *
 * Key features:
 * - Circular buffer with head and tail pointers
 * - Fixed size buffer (LEN_MESSAGE_BUFFER + 1 to allow full detection)
 * - Stores pointers only (actual messages allocated from buffer store)
 * - Full/empty detection without requiring additional state flags
 *
 * FIFO state detection:
 * - Empty when: head == tail
 * - Full when: (head + 1) % buffer_size == tail
 * - Extra slot (+1) allows distinguishing full from empty
 *
 * Typical use cases:
 * - Incoming message queue for sequential processing
 * - Outgoing message queue for transmission scheduling
 * - Event buffering during burst activity
 *
 * This FIFO must be initialized after OpenLcbBufferStore_initialize() but can be
 * used independently of OpenLcbBufferList.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_BUFFER_FIFO__
#define __OPENLCB_OPENLCB_BUFFER_FIFO__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLcb Message Buffer FIFO
     *
     * @details Prepares the FIFO for use by clearing all slots and resetting pointers.
     * The FIFO uses a circular buffer algorithm where head points to the next
     * insertion position and tail points to the next removal position. The buffer
     * includes one extra slot beyond LEN_MESSAGE_BUFFER to allow detection of the
     * full condition without requiring an additional counter or flag.
     *
     * Use cases:
     * - Called once during application startup
     * - Required before any FIFO push/pop operations
     * - Must be called after OpenLcbBufferStore_initialize()
     *
     *
     * @warning MUST be called exactly once during application initialization before
     *          any FIFO operations. Calling multiple times will discard any queued messages.
     *
     * @warning NOT thread-safe - must be called during single-threaded initialization
     *
     * @attention Call this function after OpenLcbBufferStore_initialize()
     * @attention Call before using OpenLcbBufferFifo_push() or OpenLcbBufferFifo_pop()
     *
     * @see OpenLcbBufferStore_initialize - Must be called first
     * @see OpenLcbBufferFifo_push - Adds messages to initialized FIFO
     * @see OpenLcbBufferFifo_pop - Removes messages from initialized FIFO
     */
    extern void OpenLcbBufferFifo_initialize(void);


     /**
     * @brief Pushes a new OpenLcb message into the FIFO buffer
     *
     * @details Adds a message pointer to the tail of the queue if space is available.
     * The message itself must have been previously allocated from the OpenLcb Buffer
     * Store using OpenLcbBufferStore_allocate_buffer(). Returns the message pointer
     * on success, or NULL if the FIFO is full.
     *
     * Use cases:
     * - Queuing incoming messages for processing
     * - Buffering outgoing messages for transmission
     * - Managing message backlog during burst activity
     *
     * @param new_msg Pointer to a message allocated from OpenLcbBufferStore (must NOT be NULL)
     * @return Pointer to the queued message on success, or NULL if FIFO is full
     *
     * @warning new_msg must NOT be NULL - passing NULL will store NULL in FIFO
     * @warning Returns NULL when FIFO is full - caller MUST check return value
     * @warning FIFO full condition means buffer is exhausted - messages may be dropped
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL before assuming success
     * @attention Caller retains ownership - must free message with OpenLcbBufferStore_free_buffer() later
     * @attention FIFO stores pointers only - does not copy message data
     *
     *
     * @see OpenLcbBufferStore_allocate_buffer - Allocates messages before pushing
     * @see OpenLcbBufferFifo_pop - Removes messages from FIFO
     * @see OpenLcbBufferFifo_is_empty - Check before popping
     * @see OpenLcbBufferFifo_get_allocated_count - Check FIFO occupancy
     */
    extern openlcb_msg_t *OpenLcbBufferFifo_push(openlcb_msg_t *new_msg);


    /**
     * @brief Pops an OpenLcb message off the FIFO buffer
     *
     * @details Removes and returns the message pointer from the head of the queue.
     * Uses FIFO ordering (First-In-First-Out) so the oldest message is returned first.
     * Returns NULL if the FIFO is empty.
     *
     * Use cases:
     * - Processing queued incoming messages in order
     * - Transmitting queued outgoing messages sequentially
     * - Draining FIFO during shutdown or error recovery
     *
     * @return Pointer to the oldest message in FIFO, or NULL if FIFO is empty
     *
     * @warning Returns NULL when FIFO is empty - caller MUST check for NULL
     * @warning Caller becomes responsible for freeing the returned message
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL before dereferencing
     * @attention Caller must call OpenLcbBufferStore_free_buffer() when done with message
     * @attention Popped message is removed from FIFO but not freed from buffer store
     *
     * @note Returns NULL if empty - check with OpenLcbBufferFifo_is_empty() first
     *
     * @see OpenLcbBufferFifo_push - Adds messages to FIFO
     * @see OpenLcbBufferFifo_is_empty - Check before popping
     * @see OpenLcbBufferStore_free_buffer - Free the message when done
     * @see OpenLcbBufferFifo_get_allocated_count - Check how many messages available
     */
    extern openlcb_msg_t *OpenLcbBufferFifo_pop(void);


    /**
     * @brief Tests if there is a message in the FIFO buffer
     *
     * @details Checks whether the FIFO contains any messages by comparing head and
     * tail pointers. This is a non-destructive peek operation - it does not modify
     * the FIFO state. Returns true if empty, false otherwise.
     *
     * Use cases:
     * - Checking before attempting to pop
     * - Polling for message availability
     * - Conditional processing loops
     * - Idle detection
     *
     * @return True if FIFO is empty, false if at least one message is present
     *
     * @note Non-destructive - does not modify FIFO state
     * @note Safe to call at any time
     *
     * @see OpenLcbBufferFifo_pop - Use this after checking is_empty() returns false
     * @see OpenLcbBufferFifo_get_allocated_count - Get exact count instead of just empty/not empty
     */
    extern bool OpenLcbBufferFifo_is_empty(void);


    /**
     * @brief Returns the number of messages currently in the FIFO buffer
     *
     * @details Calculates the current occupancy of the FIFO by analyzing head and
     * tail pointer positions. Handles the circular buffer wraparound case correctly.
     *
     * Use cases:
     * - Monitoring FIFO utilization
     * - Load balancing decisions
     * - Flow control (backpressure)
     * - Debug and diagnostics
     *
     * @return The number of messages currently in the buffer (0 to LEN_MESSAGE_BUFFER)
     *
     * @attention Return value can range from 0 (empty) to LEN_MESSAGE_BUFFER (full)
     * @attention Does not count available slots, counts occupied slots
     *
     * @note Maximum count is LEN_MESSAGE_BUFFER (one slot reserved for full detection)
     *
     * @see OpenLcbBufferFifo_is_empty - Simpler check for zero count
     * @see OpenLcbBufferFifo_push - Adding messages increases this count
     * @see OpenLcbBufferFifo_pop - Removing messages decreases this count
     */
    extern uint16_t OpenLcbBufferFifo_get_allocated_count(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_FIFO__ */
