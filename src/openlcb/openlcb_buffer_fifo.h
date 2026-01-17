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
 * @file openlcb_buffer_fifo.h
 * @brief FIFO buffer implementation for OpenLcb/LCC message structures
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_BUFFER_FIFO__
#define __OPENLCB_OPENLCB_BUFFER_FIFO__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLcb Message Buffer FIFO
     * @param None
     * @return None
     * @attention This must always be called during application initialization
     */
    extern void OpenLcbBufferFifo_initialize(void);


     /**
     * @brief Pushes a new OpenLcb message into the FIFO buffer that was allocated from the buffer store
     * @param new_msg Pointer to a message allocated from the OpenLcb Buffer Pool
     * @return Pointer to the message or NULL if it fails
     * @see OpenLcbBufferStore_allocate_buffer()
     */
    extern openlcb_msg_t *OpenLcbBufferFifo_push(openlcb_msg_t *new_msg);


    /**
     * @brief Pops an OpenLcb message off the FIFO buffer
     * @param None
     * @return Pointer to the message or NULL if the buffer is empty
     * @note The caller is responsible for freeing the message when done
     * @see OpenLcbBufferStore_free_buffer()
     */
    extern openlcb_msg_t *OpenLcbBufferFifo_pop(void);


    /**
     * @brief Tests if there is a message in the FIFO buffer
     * @param None
     * @return True if there is at least one message, false if the list is empty
     */
    extern bool OpenLcbBufferFifo_is_empty(void);


    /**
     * @brief Returns the number of messages in the FIFO buffer
     * @param None
     * @return The number of messages in the buffer
     */
    extern uint16_t OpenLcbBufferFifo_get_allocated_count(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_FIFO__ */
