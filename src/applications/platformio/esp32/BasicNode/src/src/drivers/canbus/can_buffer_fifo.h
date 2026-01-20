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
 * @brief FIFO buffer implementation for CAN message structures
 *
 * @details Provides a circular queue for managing CAN message pointers. Messages
 * are allocated from the CAN Buffer Store and pushed into the FIFO for ordered
 * processing. The FIFO uses a circular buffer with one extra slot to distinguish
 * between empty and full states.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
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
     * @brief Initializes the CAN Message Buffer FIFO
     *
     * @details Clears all FIFO slots and resets head and tail pointers to zero.
     * Must be called once during application startup before any FIFO operations.
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
    extern void CanBufferFifo_initialize(void);

    /**
     * @brief Pushes a new CAN message into the FIFO buffer
     *
     * @details Adds a CAN message pointer to the FIFO queue. The message must be
     * allocated from the CAN Buffer Store before pushing. The FIFO stores pointers
     * only, not the message structures themselves.
     *
     * Use cases:
     * - Hardware CAN receive interrupt/callback
     * - Queueing incoming CAN frames for processing
     *
     * @param new_msg Pointer to allocated CAN message from CanBufferStore_allocate_buffer()
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
    extern bool CanBufferFifo_push(can_msg_t *new_msg);

    /**
     * @brief Pops a CAN message off the FIFO buffer
     *
     * @details Removes and returns the oldest message from the FIFO queue.
     * The caller is responsible for freeing the message when processing is complete.
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
    extern can_msg_t *CanBufferFifo_pop(void);

    /**
     * @brief Tests if the FIFO buffer is empty
     *
     * @details Checks whether any messages are currently in the FIFO queue.
     * Useful for polling-based processing loops.
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
    extern uint8_t CanBufferFifo_is_empty(void);

    /**
     * @brief Returns the number of messages currently in the FIFO buffer
     *
     * @details Calculates the current FIFO occupancy by comparing head and tail
     * pointers. Accounts for circular buffer wraparound.
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
    extern uint16_t CanBufferFifo_get_allocated_count(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_BUFFER_FIFO__ */
