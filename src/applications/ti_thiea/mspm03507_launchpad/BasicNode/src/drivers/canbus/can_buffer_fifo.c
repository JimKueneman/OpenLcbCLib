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
 * @brief Circular FIFO queue for @ref can_msg_t pointers.
 *
 * @details Uses one extra slot (USER_DEFINED_CAN_MSG_BUFFER_DEPTH + 1) so that
 * head == tail always means empty without needing a separate counter.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "can_buffer_fifo.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_buffer_store.h"

/** @brief Internal circular buffer for queuing @ref can_msg_t pointers. */
typedef struct {
    can_msg_t *list[LEN_CAN_FIFO_BUFFER];  /**< @brief Message pointer slots. */
    uint8_t head;                           /**< @brief Next write position. */
    uint8_t tail;                           /**< @brief Next read position. */
} can_fifo_t;

/** @brief Single global FIFO instance. */
static can_fifo_t can_msg_buffer_fifo;

    /**
     * @brief Clears all FIFO slots and resets head and tail to zero.
     *
     * @details Algorithm:
     * -# Set all LEN_CAN_FIFO_BUFFER pointer slots to NULL.
     * -# Reset head and tail indices to zero.
     */
void CanBufferFifo_initialize(void) {

    for (int i = 0; i < LEN_CAN_FIFO_BUFFER; i++) {

        can_msg_buffer_fifo.list[i] = NULL;

    }

    can_msg_buffer_fifo.head = 0;
    can_msg_buffer_fifo.tail = 0;

}

    /**
     * @brief Pushes a @ref can_msg_t pointer onto the tail of the FIFO.
     *
     * @details Algorithm:
     * -# Compute next head position with wraparound.
     * -# If next != tail (not full): store pointer, advance head, return true.
     * -# Otherwise return false (FIFO full).
     *
     * @verbatim
     * @param new_msg Pointer to an allocated @ref can_msg_t.
     * @endverbatim
     *
     * @return true on success, false if the FIFO is full.
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
     * @brief Removes and returns the oldest @ref can_msg_t from the FIFO.
     *
     * @details Algorithm:
     * -# If head != tail (not empty): save pointer at tail, clear slot, advance tail
     *    with wraparound, return saved pointer.
     * -# Otherwise return NULL (FIFO empty).
     *
     * @return Pointer to the oldest @ref can_msg_t, or NULL if the FIFO is empty.
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

    /** @brief Returns non-zero if the FIFO is empty, zero if messages are present. */
uint8_t CanBufferFifo_is_empty(void) {

    return can_msg_buffer_fifo.head == can_msg_buffer_fifo.tail;

}

    /**
     * @brief Returns the number of @ref can_msg_t pointers currently in the FIFO.
     *
     * @details Handles the wraparound case where tail > head.
     *
     * @return Current FIFO occupancy count.
     */
uint16_t CanBufferFifo_get_allocated_count(void) {

    if (can_msg_buffer_fifo.tail > can_msg_buffer_fifo.head) {

        return (can_msg_buffer_fifo.head + (LEN_CAN_FIFO_BUFFER - can_msg_buffer_fifo.tail));

    } else {

        return (can_msg_buffer_fifo.head - can_msg_buffer_fifo.tail);

    }

}
