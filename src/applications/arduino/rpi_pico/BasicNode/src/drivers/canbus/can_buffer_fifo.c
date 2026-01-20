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
 * @file can_buffer_fifo.c
 * @brief Implementation of the FIFO buffer for CAN messages
 *
 * @details Implements a circular queue using an array of message pointers. The
 * FIFO uses one extra slot (USER_DEFINED_CAN_MSG_BUFFER_DEPTH + 1) to distinguish
 * between empty and full states without requiring a separate counter. Head points
 * to the next insertion position, tail points to the next removal position.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "can_buffer_fifo.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_buffer_store.h"

    /**
     * @brief Internal FIFO structure for circular queue management
     *
     * @details Implements a circular buffer using array indices. The buffer size is
     * LEN_CAN_FIFO_BUFFER which is (USER_DEFINED_CAN_MSG_BUFFER_DEPTH + 1). The
     * extra slot allows distinction between empty (head == tail) and full 
     * (head + 1 == tail) conditions without needing a separate count variable.
     */
typedef struct {
    can_msg_t *list[LEN_CAN_FIFO_BUFFER];  ///< Array of message pointers (circular buffer)
    uint8_t head;                          ///< Next insertion position (points to empty slot)
    uint8_t tail;                          ///< Next removal position (points to oldest message)
} can_fifo_t;

    /**
     * @brief Static instance of the FIFO buffer
     *
     * @details Single global FIFO instance for CAN message queue. All incoming CAN
     * messages are pushed here for sequential processing. Initialized by 
     * CanBufferFifo_initialize().
     */
static can_fifo_t can_msg_buffer_fifo;

    /**
     * @brief Initializes the CAN Message Buffer FIFO
     *
     * @details Algorithm:
     * -# Iterate through entire FIFO array
     * -# Set each slot to NULL (no message present)
     * -# Reset head pointer to 0 (next insertion position)
     * -# Reset tail pointer to 0 (next removal position)
     * -# FIFO is now empty (head == tail)
     *
     * Use cases:
     * - Called during system initialization
     * - Required before any push or pop operations
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call before CanBufferFifo_push() or CanBufferFifo_pop()
     *
     * @see CanBufferFifo_push - Adds message to FIFO
     * @see CanBufferStore_initialize - Must be called before this function
     */
void CanBufferFifo_initialize(void) {

    for (int i = 0; i < LEN_CAN_FIFO_BUFFER; i++) {

        can_msg_buffer_fifo.list[i] = NULL;

    }

    can_msg_buffer_fifo.head = 0;
    can_msg_buffer_fifo.tail = 0;

}

    /**
     * @brief Pushes a new CAN message into the FIFO buffer
     *
     * @details Algorithm:
     * -# Calculate next head position (head + 1)
     * -# Apply wraparound if next >= buffer length (circular buffer)
     * -# Check if FIFO is full (next == tail)
     * -# If full, return false (cannot add)
     * -# If space available:
     *    - Store message pointer at current head position
     *    - Update head to next position
     *    - Return true (success)
     *
     * Use cases:
     * - Hardware CAN receive interrupt/callback
     * - Queueing incoming CAN frames for processing
     *
     * @verbatim
     * @param new_msg Pointer to allocated CAN message from CanBufferStore_allocate_buffer()
     * @endverbatim
     *
     * @return true if message successfully added to FIFO, false if FIFO is full
     *
     * @warning FIFO full condition returns false - caller must handle this error
     * @warning NOT thread-safe - use shared resource locking
     *
     * @attention Always check return value - dropped messages when FIFO full
     * @attention Message must be allocated before pushing
     *
     * @see CanBufferStore_allocate_buffer - Allocates message before push
     * @see CanBufferFifo_pop - Removes message from FIFO
     */
bool CanBufferFifo_push(can_msg_t *new_msg) {

    uint8_t next = can_msg_buffer_fifo.head + 1;

    if (next >= LEN_CAN_FIFO_BUFFER) {

        next = 0;

    }

    if (next != can_msg_buffer_fifo.tail) {

        can_msg_buffer_fifo.list[can_msg_buffer_fifo.head] = new_msg;
        can_msg_buffer_fifo.head = next;

        return true;

    }

    return false;

}

    /**
     * @brief Pops a CAN message off the FIFO buffer
     *
     * @details Algorithm:
     * -# Check if FIFO is empty (head == tail)
     * -# If empty, return NULL
     * -# If not empty:
     *    - Retrieve message pointer from tail position
     *    - Clear slot to NULL (optional cleanup)
     *    - Increment tail pointer
     *    - Apply wraparound if tail >= buffer length
     *    - Return retrieved message pointer
     *
     * Use cases:
     * - Main processing loop
     * - CAN receive state machine
     *
     * @return Pointer to the oldest CAN message, or NULL if FIFO is empty
     *
     * @warning Returns NULL when FIFO empty - caller MUST check for NULL
     * @warning Caller MUST free message with CanBufferStore_free_buffer() when done
     * @warning NOT thread-safe - use shared resource locking
     *
     * @attention Always check return value for NULL before use
     * @attention Remember to free message after processing
     *
     * @see CanBufferStore_free_buffer - Must call when done with message
     * @see CanBufferFifo_is_empty - Check before popping
     */
can_msg_t *CanBufferFifo_pop(void) {

    if (can_msg_buffer_fifo.head != can_msg_buffer_fifo.tail) {

        can_msg_t *msg = can_msg_buffer_fifo.list[can_msg_buffer_fifo.tail];
        can_msg_buffer_fifo.list[can_msg_buffer_fifo.tail] = NULL;
        can_msg_buffer_fifo.tail++;

        if (can_msg_buffer_fifo.tail >= LEN_CAN_FIFO_BUFFER) {

            can_msg_buffer_fifo.tail = 0;

        }

        return msg;

    }

    return NULL;

}

    /**
     * @brief Tests if the FIFO buffer is empty
     *
     * @details Algorithm:
     * -# Compare head and tail pointers
     * -# If head == tail, FIFO is empty (return true)
     * -# If head != tail, FIFO has messages (return false)
     *
     * Use cases:
     * - Checking before calling pop to avoid NULL
     * - Polling for incoming messages
     *
     * @return Non-zero (true) if FIFO is empty, zero (false) if messages present
     *
     * @note Return value is non-zero for true, zero for false
     *
     * @see CanBufferFifo_pop - Removes message from FIFO
     * @see CanBufferFifo_get_allocated_count - Get exact count
     */
uint8_t CanBufferFifo_is_empty(void) {

    return can_msg_buffer_fifo.head == can_msg_buffer_fifo.tail;

}

    /**
     * @brief Returns the number of messages currently in the FIFO buffer
     *
     * @details Algorithm:
     * -# Compare tail and head positions
     * -# If tail > head:
     *    - Buffer has wrapped around
     *    - Count = head + (buffer_length - tail)
     * -# If tail <= head:
     *    - Normal case, no wraparound
     *    - Count = head - tail
     * -# Return calculated count
     *
     * Use cases:
     * - Monitoring FIFO usage
     * - Detecting near-full conditions
     * - Performance analysis
     *
     * @return Number of messages currently in FIFO (0 to USER_DEFINED_CAN_MSG_BUFFER_DEPTH)
     *
     * @note Count changes dynamically as messages are pushed and popped
     *
     * @see CanBufferFifo_is_empty - Quick empty check
     * @see CanBufferFifo_push - Adds to count
     * @see CanBufferFifo_pop - Decrements count
     */
uint16_t CanBufferFifo_get_allocated_count(void) {

    if (can_msg_buffer_fifo.tail > can_msg_buffer_fifo.head) {

        return (can_msg_buffer_fifo.head + (LEN_CAN_FIFO_BUFFER - can_msg_buffer_fifo.tail));

    } else {

        return (can_msg_buffer_fifo.head - can_msg_buffer_fifo.tail);

    }

}
