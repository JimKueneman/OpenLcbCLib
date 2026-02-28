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
 * @file can_buffer_fifo.h
 * @brief Circular FIFO queue for @ref can_msg_t pointers.
 *
 * @details Messages are allocated from CanBufferStore and pushed here for
 * ordered processing. Uses one extra slot so head == tail always means empty.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_BUFFER_FIFO__
#define __DRIVERS_CANBUS_CAN_BUFFER_FIFO__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
         * @brief Clears all FIFO slots and resets head and tail to zero.
         *
         * @warning Must be called once at startup before any push or pop.
         * @warning NOT thread-safe.
         *
         * @see CanBufferStore_initialize - must be called first
         */
    extern void CanBufferFifo_initialize(void);

        /**
         * @brief Pushes a @ref can_msg_t pointer onto the tail of the FIFO.
         *
         * @param new_msg Pointer to an allocated @ref can_msg_t.
         *
         * @return true on success, false if the FIFO is full.
         *
         * @warning Returns false when full - dropped messages are not recoverable.
         * @warning NOT thread-safe - use shared resource locking.
         *
         * @see CanBufferFifo_pop
         */
    extern bool CanBufferFifo_push(can_msg_t *new_msg);

        /**
         * @brief Removes and returns the oldest @ref can_msg_t from the FIFO.
         *
         * @details Caller is responsible for freeing the returned buffer with
         * CanBufferStore_free_buffer() when processing is complete.
         *
         * @return Pointer to the oldest @ref can_msg_t, or NULL if the FIFO is empty.
         *
         * @warning Returns NULL when empty - caller MUST check before use.
         * @warning Caller MUST free the buffer after processing.
         * @warning NOT thread-safe - use shared resource locking.
         *
         * @see CanBufferStore_free_buffer
         */
    extern can_msg_t *CanBufferFifo_pop(void);

        /** @brief Returns non-zero if the FIFO is empty, zero if messages are present. */
    extern uint8_t CanBufferFifo_is_empty(void);

        /** @brief Returns the number of @ref can_msg_t pointers currently in the FIFO. */
    extern uint16_t CanBufferFifo_get_allocated_count(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_BUFFER_FIFO__ */
