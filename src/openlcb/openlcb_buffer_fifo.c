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
 * @brief FIFO queue for OpenLCB message pointers.
 *
 * @details Circular buffer with one wasted slot for full/empty detection.
 * Head = next insertion, tail = next removal.  Empty when head == tail.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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

/** @brief Circular buffer of message pointers with head/tail indices. */
typedef struct {

    openlcb_msg_t *list[LEN_MESSAGE_FIFO_BUFFER];  ///< Circular buffer of message pointers
    uint8_t head;                                   ///< Next insertion position
    uint8_t tail;                                   ///< Next removal position

} openlcb_msg_fifo_t;

/** @brief Static FIFO instance (single global queue) */
static openlcb_msg_fifo_t _openlcb_msg_buffer_fifo;

    /**
    * @brief Initializes the FIFO.
    *
    * @details Algorithm:
    * -# Clear all slots to NULL
    * -# Reset head and tail to 0
    */
void OpenLcbBufferFifo_initialize(void) {

    for (int i = 0; i < LEN_MESSAGE_FIFO_BUFFER; i++) {

        _openlcb_msg_buffer_fifo.list[i] = NULL;

    }

    _openlcb_msg_buffer_fifo.head = 0;
    _openlcb_msg_buffer_fifo.tail = 0;

}

    /**
    * @brief Adds a message pointer to the tail of the FIFO.
    *
    * @details Algorithm:
    * -# Compute next head position with wraparound
    * -# If next == tail the FIFO is full, return NULL
    * -# Store pointer at head, advance head, return the pointer
    *
    * @verbatim
    * @param new_msg Pointer to @ref openlcb_msg_t allocated from OpenLcbBufferStore
    * @endverbatim
    *
    * @return The queued pointer on success, or NULL if the FIFO is full
    */
openlcb_msg_t *OpenLcbBufferFifo_push(openlcb_msg_t *new_msg) {

    uint8_t next = _openlcb_msg_buffer_fifo.head + 1;
    if (next >= LEN_MESSAGE_FIFO_BUFFER) {

        next = 0;

    }

    if (next != _openlcb_msg_buffer_fifo.tail) {

        _openlcb_msg_buffer_fifo.list[_openlcb_msg_buffer_fifo.head] = new_msg;
        _openlcb_msg_buffer_fifo.head = next;

        return new_msg;

    }

    return NULL;

}

    /**
    * @brief Removes and returns the oldest message from the FIFO.
    *
    * @details Algorithm:
    * -# If head == tail the FIFO is empty, return NULL
    * -# Retrieve pointer at tail, advance tail with wraparound
    * -# Return the pointer
    *
    * @return Pointer to the oldest @ref openlcb_msg_t, or NULL if the FIFO is empty
    */
openlcb_msg_t *OpenLcbBufferFifo_pop(void) {

    openlcb_msg_t *result = NULL;

    if (_openlcb_msg_buffer_fifo.head != _openlcb_msg_buffer_fifo.tail) {

        result = _openlcb_msg_buffer_fifo.list[_openlcb_msg_buffer_fifo.tail];

        _openlcb_msg_buffer_fifo.tail = _openlcb_msg_buffer_fifo.tail + 1;

        if (_openlcb_msg_buffer_fifo.tail >= LEN_MESSAGE_FIFO_BUFFER) {

            _openlcb_msg_buffer_fifo.tail = 0;

        }

    }

    return result;

}

    /** @brief Returns true if the FIFO contains no messages. */
bool OpenLcbBufferFifo_is_empty(void) {

    return (_openlcb_msg_buffer_fifo.head == _openlcb_msg_buffer_fifo.tail);

}

    /** @brief Returns the number of messages currently held in the FIFO. */
uint16_t OpenLcbBufferFifo_get_allocated_count(void) {

    if (_openlcb_msg_buffer_fifo.tail > _openlcb_msg_buffer_fifo.head) {

        return (_openlcb_msg_buffer_fifo.head + (LEN_MESSAGE_FIFO_BUFFER - _openlcb_msg_buffer_fifo.tail));

    } else {

        return (_openlcb_msg_buffer_fifo.head - _openlcb_msg_buffer_fifo.tail);

    }

}
