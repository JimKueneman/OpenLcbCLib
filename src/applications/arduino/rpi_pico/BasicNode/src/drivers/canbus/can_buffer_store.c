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
 * @brief Implementation of the core buffer store for CAN frames
 * @author Jim Kueneman
 * @date 17 Jan 2026
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


    /**
     * @brief Pre-allocated array of CAN message buffers
     *
     * @details Static storage for all CAN message buffers. Size is determined by
     * USER_DEFINED_CAN_MSG_BUFFER_DEPTH. Each buffer contains an 8-byte payload
     * plus metadata (identifier, payload_count, state flags).
     */
static can_msg_array_t _can_buffer_store;

    /**
     * @brief Current number of allocated buffers
     *
     * @details Tracks how many buffers are currently allocated. Incremented by
     * CanBufferStore_allocate_buffer() and decremented by CanBufferStore_free_buffer().
     * Used for monitoring buffer usage and detecting leaks.
     */
static uint16_t _can_buffer_store_message_allocated;

    /**
     * @brief Peak number of allocated buffers
     *
     * @details Records the maximum number of buffers allocated simultaneously since
     * initialization or last reset. Used for sizing analysis during stress testing.
     * Reset by CanBufferStore_clear_max_allocated().
     */
static uint16_t _can_buffer_store_message_max_allocated;

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

void CanBufferStore_free_buffer(can_msg_t *msg) {

    if (!msg) {

        return;

    }

    _can_buffer_store_message_allocated = _can_buffer_store_message_allocated - 1;
    msg->state.allocated = false;

}

uint16_t CanBufferStore_messages_allocated(void) {

    return _can_buffer_store_message_allocated;

}

uint16_t CanBufferStore_messages_max_allocated(void) {

    return _can_buffer_store_message_max_allocated;

}

void CanBufferStore_clear_max_allocated(void) {

    _can_buffer_store_message_max_allocated = 0;

}
