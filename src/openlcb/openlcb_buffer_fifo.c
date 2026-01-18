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
 * @file openlcb_buffer_fifo.c
 * @brief Implementation of the FIFO buffer for OpenLcb messages
 *
 * @details Implements a circular buffer (ring buffer) FIFO queue for OpenLCB message
 * pointers. The implementation uses the classic "one slot wasted" approach where the
 * buffer size is (capacity + 1) to allow simple full/empty detection without requiring
 * additional state variables.
 *
 * Algorithm details:
 * - Circular buffer with power-of-two friendly wraparound
 * - Head points to next insertion position
 * - Tail points to next removal position
 * - Empty condition: head == tail
 * - Full condition: (head + 1) % buffer_size == tail
 * - Count calculation handles wraparound correctly
 *
 * Memory characteristics:
 * - Fixed size allocation at compile time
 * - No dynamic memory allocation during runtime
 * - Pointer storage only (8 bytes per message on 64-bit, 4 bytes on 32-bit)
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "openlcb_buffer_fifo.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_buffer_store.h"



/** @brief FIFO buffer size (one extra slot for full detection without additional state) */
#define LEN_MESSAGE_FIFO_BUFFER (LEN_MESSAGE_BUFFER + 1)

/**
 * @brief FIFO structure containing circular buffer and pointers
 *
 * @details Contains the message pointer array and head/tail indices for FIFO management.
 * The extra buffer slot (LEN_MESSAGE_BUFFER + 1) allows distinguishing between full
 * and empty states using only the head and tail pointers.
 */
typedef struct {

    openlcb_msg_t *list[LEN_MESSAGE_FIFO_BUFFER];  ///< Circular buffer of message pointers
    uint8_t head;                                   ///< Next insertion position
    uint8_t tail;                                   ///< Next removal position

} openlcb_msg_fifo_t;

/** @brief Static FIFO instance (single global queue) */
openlcb_msg_fifo_t openlcb_msg_buffer_fifo;

    /**
     * @brief Initializes the OpenLcb Message Buffer FIFO
     *
     * @details Algorithm:
     * Sets up an empty FIFO by clearing all pointers and resetting indices.
     * -# Iterate through all LEN_MESSAGE_FIFO_BUFFER slots
     * -# Set each slot to NULL
     * -# Set head = 0 (next insertion at position 0)
     * -# Set tail = 0 (next removal from position 0)
     * -# Result: Empty FIFO (head == tail)
     *
     * Use cases:
     * - Called once during application startup
     * - Required before any FIFO operations
     * - Must be called after OpenLcbBufferStore_initialize()
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call after OpenLcbBufferStore_initialize()
     *
     * @see OpenLcbBufferStore_initialize - Must be called first
     */
void OpenLcbBufferFifo_initialize(void) {

    for (int i = 0; i < LEN_MESSAGE_FIFO_BUFFER; i++) {

        openlcb_msg_buffer_fifo.list[i] = NULL;

    }

    openlcb_msg_buffer_fifo.head = 0;
    openlcb_msg_buffer_fifo.tail = 0;

}

    /**
     * @brief Pushes a new OpenLcb message into the FIFO buffer
     *
     * @details Algorithm:
     * Implements circular buffer insertion with full detection.
     * -# Calculate next head position: next = (head + 1) % buffer_size
     * -# If next >= buffer_size, wrap to 0
     * -# Check if FIFO is full: if next == tail, return NULL
     * -# If not full:
     *    - Store new_msg pointer at list[head]
     *    - Update head = next
     *    - Return new_msg (success)
     * -# If full, return NULL (failure)
     *
     * Use cases:
     * - Queuing incoming messages
     * - Buffering outgoing messages
     * - Managing message backlog
     *
     * @verbatim
     * @param new_msg Pointer to message allocated from buffer store (must NOT be NULL)
     * @endverbatim
     *
     * @return Pointer to queued message on success, NULL if FIFO is full
     *
     * @warning Returns NULL when FIFO is full
     * @warning Passing NULL will store NULL in FIFO - no validation performed
     * @warning NOT thread-safe
     *
     * @attention Caller retains ownership of message
     * @attention Check return value for NULL before assuming success
     *
     * @remark The "one slot wasted" approach means maximum capacity is LEN_MESSAGE_BUFFER,
     *         not LEN_MESSAGE_FIFO_BUFFER
     *
     * @see OpenLcbBufferStore_allocate_buffer - Allocate message before pushing
     * @see OpenLcbBufferFifo_pop - Remove messages from FIFO
     */
openlcb_msg_t *OpenLcbBufferFifo_push(openlcb_msg_t *new_msg) {

    uint8_t next = openlcb_msg_buffer_fifo.head + 1;
    if (next >= LEN_MESSAGE_FIFO_BUFFER) {

        next = 0;

    }

    if (next != openlcb_msg_buffer_fifo.tail) {

        openlcb_msg_buffer_fifo.list[openlcb_msg_buffer_fifo.head] = new_msg;
        openlcb_msg_buffer_fifo.head = next;

        return new_msg;

    }

    return NULL;

}

    /**
     * @brief Pops an OpenLcb message off the FIFO buffer
     *
     * @details Algorithm:
     * Implements circular buffer removal with empty detection.
     * -# Initialize result = NULL
     * -# Check if FIFO is empty: if head == tail, return NULL
     * -# If not empty:
     *    - Get message pointer from list[tail]
     *    - Increment tail = tail + 1
     *    - If tail >= buffer_size, wrap tail to 0
     *    - Return the retrieved message pointer
     * -# If empty, return NULL
     *
     * Use cases:
     * - Processing queued messages in order
     * - Transmitting queued messages
     * - Draining FIFO
     *
     * @return Pointer to oldest message, or NULL if FIFO is empty
     *
     * @warning Returns NULL when empty
     * @warning Caller must free returned message with OpenLcbBufferStore_free_buffer()
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL
     * @attention Caller becomes responsible for freeing message
     *
     * @see OpenLcbBufferFifo_push - Add messages to FIFO
     * @see OpenLcbBufferStore_free_buffer - Free popped message when done
     * @see OpenLcbBufferFifo_is_empty - Check before popping
     */
openlcb_msg_t *OpenLcbBufferFifo_pop(void) {

    openlcb_msg_t *result = NULL;

    if (openlcb_msg_buffer_fifo.head != openlcb_msg_buffer_fifo.tail) {

        result = openlcb_msg_buffer_fifo.list[openlcb_msg_buffer_fifo.tail];

        openlcb_msg_buffer_fifo.tail = openlcb_msg_buffer_fifo.tail + 1;

        if (openlcb_msg_buffer_fifo.tail >= LEN_MESSAGE_FIFO_BUFFER) {

            openlcb_msg_buffer_fifo.tail = 0;

        }

    }

    return result;

}

    /**
     * @brief Tests if there is a message in the FIFO buffer
     *
     * @details Algorithm:
     * Simple pointer comparison to determine empty state.
     * -# Compare head pointer with tail pointer
     * -# Return (head == tail)
     *
     * Use cases:
     * - Check before popping
     * - Polling for messages
     * - Idle detection
     *
     * @return True if FIFO is empty (no messages), false if at least one message present
     *
     * @note Non-destructive operation
     *
     * @see OpenLcbBufferFifo_pop - Use after checking not empty
     * @see OpenLcbBufferFifo_get_allocated_count - Get exact count
     */
bool OpenLcbBufferFifo_is_empty(void) {

    return (openlcb_msg_buffer_fifo.head == openlcb_msg_buffer_fifo.tail);

}

    /**
     * @brief Returns the number of messages currently in the FIFO buffer
     *
     * @details Algorithm:
     * Calculates occupancy handling circular buffer wraparound.
     * -# If tail > head (wraparound case):
     *    - count = head + (buffer_size - tail)
     *    - This accounts for messages from tail to end, plus messages from start to head
     * -# Else (normal case):
     *    - count = head - tail
     *    - Simple difference when head is ahead of tail
     * -# Return calculated count
     *
     * Examples:
     * - head=5, tail=2, size=10: count = 5-2 = 3
     * - head=2, tail=8, size=10: count = 2+(10-8) = 4
     *
     * Use cases:
     * - Monitoring FIFO utilization
     * - Load balancing
     * - Flow control
     *
     * @return Number of messages in buffer (0 to LEN_MESSAGE_BUFFER)
     *
     * @note Maximum return value is LEN_MESSAGE_BUFFER (one slot reserved)
     *
     * @see OpenLcbBufferFifo_is_empty - Simpler zero check
     * @see OpenLcbBufferFifo_push - Increases count
     * @see OpenLcbBufferFifo_pop - Decreases count
     */
uint16_t OpenLcbBufferFifo_get_allocated_count(void) {

    if (openlcb_msg_buffer_fifo.tail > openlcb_msg_buffer_fifo.head) {

        return (openlcb_msg_buffer_fifo.head + (LEN_MESSAGE_FIFO_BUFFER - openlcb_msg_buffer_fifo.tail));

    } else {

        return (openlcb_msg_buffer_fifo.head - openlcb_msg_buffer_fifo.tail);

    }

}
