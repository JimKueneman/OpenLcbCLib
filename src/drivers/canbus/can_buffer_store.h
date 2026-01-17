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
 * @file can_buffer_store.h
 * @brief Core buffer store for CAN frame allocation and management
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
     * @param None
     * @return None
     * @attention This must always be called during application initialization
     */
    extern void CanBufferStore_initialize(void);

    /**
     * @brief Allocates a new CAN buffer
     * @param None
     * @return Pointer to the message buffer or NULL if it fails
     */
    extern can_msg_t *CanBufferStore_allocate_buffer(void);

    /**
     * @brief Frees the buffer so it can be reused
     * @param can_msg_t *msg [in] - Pointer to a message to be freed
     * @return None
     */
    extern void CanBufferStore_free_buffer(can_msg_t *msg);

    /**
     * @brief The number of CAN messages currently allocated in the buffer store
     * @param None
     * @return Number of CAN messages currently allocated
     */
    extern uint16_t CanBufferStore_messages_allocated(void);

    /**
     * @brief The maximum number of CAN messages that have been allocated in the buffer store
     * Useful for understanding how deep the buffer store needs to be during stress testing
     * @param None
     * @return Maximum number of CAN sized messages that have been allocated currently
     */
    extern uint16_t CanBufferStore_messages_max_allocated(void);

    /**
     * @brief Resets the running count of the Maximum number of CAN buffer that have been allocated at one time
     * @param None
     * @return None
     */
    extern void CanBufferStore_clear_max_allocated(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_BUFFER_STORE__ */
