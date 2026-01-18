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
 * @brief Core buffer store for CAN frame allocation and management
 *
 * @details Manages a pre-allocated pool of CAN message buffers (8-byte payload).
 * Buffers are allocated on demand and freed when no longer needed. Pool size is
 * configured at compile time via USER_DEFINED_CAN_MSG_BUFFER_DEPTH. Provides
 * telemetry for monitoring allocation patterns and optimizing pool size.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
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
     * @brief Initializes the CAN Buffer Store
     *
     * @details Clears all message structures in the buffer pool, resets allocation
     * flags, and zeros all telemetry counters. Must be called once during application
     * startup before any buffer operations.
     *
     * Use cases:
     * - Called during system initialization
     * - Required before any buffer allocation
     *
     * @warning MUST be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call before CanBufferFifo_initialize()
     * @attention Call before any CanBufferStore_allocate_buffer() operations
     *
     * @see CanBufferStore_allocate_buffer - Allocates from initialized pool
     */
    extern void CanBufferStore_initialize(void);

    /**
     * @brief Allocates a new CAN buffer from the pool
     *
     * @details Searches the buffer pool for an available buffer and returns a
     * pointer to it. The buffer is cleared and marked as allocated. Allocation
     * telemetry is updated to track current and peak usage.
     *
     * Use cases:
     * - Hardware CAN receive interrupt/callback
     * - Creating outgoing CAN frames
     * - Assembling multi-frame messages
     *
     * @return Pointer to allocated CAN message buffer, or NULL if pool is exhausted
     *
     * @warning Returns NULL when pool exhausted - caller MUST check for NULL
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL before use
     * @attention Caller must free buffer with CanBufferStore_free_buffer() when done
     *
     * @see CanBufferStore_free_buffer - Frees allocated buffer
     * @see CanBufferStore_messages_allocated - Current allocation count
     */
    extern can_msg_t *CanBufferStore_allocate_buffer(void);

    /**
     * @brief Frees a CAN buffer back to the pool
     *
     * @details Returns an allocated buffer to the pool for reuse. Marks the buffer
     * as unallocated and decrements the allocation counter. NULL pointers are
     * safely ignored.
     *
     * Use cases:
     * - After processing received CAN frame
     * - After transmitting CAN frame
     * - When discarding invalid message
     *
     * @param msg Pointer to CAN message buffer to free (NULL is safely ignored)
     *
     * @warning NOT thread-safe - use shared resource locking
     *
     * @attention Do not use buffer after freeing
     * @attention Safe to call with NULL pointer
     *
     * @see CanBufferStore_allocate_buffer - Allocates buffer
     * @see CanBufferFifo_pop - Returns buffer that must be freed
     */
    extern void CanBufferStore_free_buffer(can_msg_t *msg);

    /**
     * @brief Returns the number of CAN messages currently allocated
     *
     * @details Provides real-time count of allocated CAN message buffers. This
     * is a live count that changes as buffers are allocated and freed.
     *
     * Use cases:
     * - Runtime monitoring of buffer usage
     * - Detecting buffer leaks
     * - Load balancing decisions
     *
     * @return Number of CAN messages currently allocated (0 to USER_DEFINED_CAN_MSG_BUFFER_DEPTH)
     *
     * @note This is a live count that changes dynamically
     * @note Compare with messages_max_allocated for sizing analysis
     *
     * @see CanBufferStore_messages_max_allocated - Peak usage counter
     * @see CanBufferStore_allocate_buffer - Increments count
     * @see CanBufferStore_free_buffer - Decrements count
     */
    extern uint16_t CanBufferStore_messages_allocated(void);

    /**
     * @brief Returns the maximum number of CAN messages that have been allocated
     *
     * @details Tracks peak allocation count since initialization or last reset.
     * Useful for stress testing to determine optimal buffer pool size.
     *
     * Use cases:
     * - Sizing buffer pool during development
     * - Stress testing and load analysis
     * - Detecting memory usage spikes
     *
     * @return Maximum number of CAN messages allocated simultaneously since last reset
     *
     * @note Peak value persists until CanBufferStore_clear_max_allocated() is called
     * @note Use during stress testing to optimize USER_DEFINED_CAN_MSG_BUFFER_DEPTH
     *
     * @see CanBufferStore_messages_allocated - Current allocation count
     * @see CanBufferStore_clear_max_allocated - Resets peak counter
     */
    extern uint16_t CanBufferStore_messages_max_allocated(void);

    /**
     * @brief Resets the peak allocation counter to zero
     *
     * @details Clears the maximum allocation counter without affecting current
     * allocations. Useful for starting fresh stress tests or monitoring sessions.
     *
     * Use cases:
     * - Starting a new stress test run
     * - Monitoring specific test scenarios
     * - Resetting telemetry after configuration changes
     *
     * @warning Does NOT affect currently allocated buffers
     *
     * @attention Current allocations remain unchanged
     *
     * @see CanBufferStore_messages_max_allocated - Returns peak value
     */
    extern void CanBufferStore_clear_max_allocated(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_BUFFER_STORE__ */
