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
 * @file can_buffer_store.h
 * @brief Pre-allocated pool of @ref can_msg_t buffers (8-byte payload each).
 *
 * @details Pool size is set at compile time via USER_DEFINED_CAN_MSG_BUFFER_DEPTH.
 * All memory is allocated at startup - no dynamic allocation at runtime.
 * Telemetry counters support pool-size tuning during development.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_BUFFER_STORE__
#define __DRIVERS_CANBUS_CAN_BUFFER_STORE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
         * @brief Clears all buffers and resets telemetry counters.
         *
         * @warning Must be called once at startup before any buffer operations.
         * @warning NOT thread-safe.
         *
         * @see CanBufferStore_allocate_buffer
         */
    extern void CanBufferStore_initialize(void);

        /**
         * @brief Allocates one @ref can_msg_t buffer from the pool.
         *
         * @details Finds the first free slot, clears it, marks it allocated, and
         * updates the peak telemetry counter.
         *
         * @return Pointer to the allocated @ref can_msg_t, or NULL if the pool is exhausted.
         *
         * @warning Returns NULL when pool is full - caller MUST check before use.
         * @warning NOT thread-safe.
         *
         * @see CanBufferStore_free_buffer
         */
    extern can_msg_t *CanBufferStore_allocate_buffer(void);

        /**
         * @brief Returns a @ref can_msg_t buffer to the pool.
         *
         * @param msg Pointer to the buffer to free. NULL is safely ignored.
         *
         * @warning Do not access the buffer after freeing.
         * @warning NOT thread-safe - use shared resource locking.
         *
         * @see CanBufferStore_allocate_buffer
         */
    extern void CanBufferStore_free_buffer(can_msg_t *msg);

        /** @brief Returns the number of @ref can_msg_t buffers currently allocated. */
    extern uint16_t CanBufferStore_messages_allocated(void);

        /** @brief Returns the peak allocation count since last reset. */
    extern uint16_t CanBufferStore_messages_max_allocated(void);

        /** @brief Resets the peak counter without affecting current allocations. */
    extern void CanBufferStore_clear_max_allocated(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_BUFFER_STORE__ */
