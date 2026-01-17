/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file can_buffer_fifo.h
 * @brief FIFO buffer implementation for CAN message structures
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_BUFFERS__
#define __DRIVERS_CANBUS_CAN_BUFFERS__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Message Buffer FIFO
     * @param None
     * @return None
     * @attention This must always be called during application initialization
     */
    extern void CanBufferFifo_initialize(void);

    /**
     * @brief Pushes a new CAN message into the FIFO buffer that was allocated
     *   see \ref CanBufferStore_allocate_buffer().
     * @param can_msg_t *new_msg [in] - Pointer to a message allocated from the CAN Buffer Pool
     * @return Pointer to the message or NULL if it fails
     */
    extern bool CanBufferFifo_push(can_msg_t *new_msg);

    /**
     * @brief Pops a CAN message off the FIFO buffer.  The caller is responsible to free the message when done
     *   see \ref CanBufferStore_free_buffer(can_msg_t *msg).
     * @param None
     * @return Pointer to the message or NULL if the buffer is empty
     */
    extern can_msg_t *CanBufferFifo_pop(void);

    /**
     * @brief Tests if there is a message in the FIFO buffer
     * @param None
     * @return true if there is at least one message, false if the list is empty
     */
    extern uint8_t CanBufferFifo_is_empty(void);

    /**
     * @brief Returns the number of messages in the FIFO buffer
     * @param None
     * @return The number of messages in the buffer
     */
    extern uint16_t CanBufferFifo_get_allocated_count(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_COMMON_CAN_BUFFERS__ */
