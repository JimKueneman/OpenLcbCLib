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
 * @file can_buffer_store.c
 * @brief Implementation of the pre-allocated CAN message buffer pool.
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "can_buffer_store.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf
#include <string.h>

#include "can_types.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_types.h"


/** @brief Pre-allocated pool of @ref can_msg_t buffers, size USER_DEFINED_CAN_MSG_BUFFER_DEPTH. */
static can_msg_array_t _can_buffer_store;

/** @brief Current number of allocated @ref can_msg_t buffers. */
static uint16_t _can_buffer_store_message_allocated;

/** @brief Peak allocation count since last reset. */
static uint16_t _can_buffer_store_message_max_allocated;

    /**
     * @brief Clears all buffers and resets telemetry counters.
     *
     * @details Algorithm:
     * -# Iterate through all USER_DEFINED_CAN_MSG_BUFFER_DEPTH entries.
     * -# Set allocated flag to false, clear identifier, payload_count, and all payload bytes.
     * -# Reset both allocation counters to zero.
     */
void CanBufferStore_initialize(void) {

    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++) {

        _can_buffer_store[i].state.allocated = false;
        _can_buffer_store[i].identifier = 0;
        _can_buffer_store[i].payload_count = 0;

        for (int j = 0; j < LEN_CAN_BYTE_ARRAY; j++) {

            _can_buffer_store[i].payload[j] = 0;

        }

    }

    _can_buffer_store_message_allocated = 0;
    _can_buffer_store_message_max_allocated = 0;

}

    /**
     * @brief Allocates one @ref can_msg_t buffer from the pool.
     *
     * @details Algorithm:
     * -# Iterate through pool looking for first unallocated buffer.
     * -# When found: increment allocation counter, update peak counter if needed,
     *    clear the buffer, mark it allocated, and return pointer.
     * -# If no free buffer found, return NULL.
     *
     * @return Pointer to the allocated @ref can_msg_t, or NULL if the pool is exhausted.
     */
can_msg_t *CanBufferStore_allocate_buffer(void) {

    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++) {

        if (!_can_buffer_store[i].state.allocated) {

            _can_buffer_store_message_allocated = _can_buffer_store_message_allocated + 1;

            if (_can_buffer_store_message_allocated > _can_buffer_store_message_max_allocated) {

                _can_buffer_store_message_max_allocated = _can_buffer_store_message_allocated;

            }

            CanUtilities_clear_can_message(&_can_buffer_store[i]);

             _can_buffer_store[i].state.allocated = true;

            return &_can_buffer_store[i];

        }

    }

    return NULL;

}

    /**
     * @brief Returns a @ref can_msg_t buffer to the pool.
     *
     * @details Algorithm:
     * -# If NULL pointer passed, return immediately.
     * -# Decrement allocation counter and clear the allocated flag.
     *
     * @verbatim
     * @param msg Pointer to the buffer to free. NULL is safely ignored.
     * @endverbatim
     */
void CanBufferStore_free_buffer(can_msg_t *msg) {

    if (!msg) {

        return;

    }

    _can_buffer_store_message_allocated = _can_buffer_store_message_allocated - 1;
    msg->state.allocated = false;

}

    /** @brief Returns the number of @ref can_msg_t buffers currently allocated. */
uint16_t CanBufferStore_messages_allocated(void) {

    return _can_buffer_store_message_allocated;

}

    /** @brief Returns the peak allocation count since last reset. */
uint16_t CanBufferStore_messages_max_allocated(void) {

    return _can_buffer_store_message_max_allocated;

}

    /** @brief Resets the peak counter without affecting current allocations. */
void CanBufferStore_clear_max_allocated(void) {

    _can_buffer_store_message_max_allocated = 0;

}
