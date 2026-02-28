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
 * @brief FIFO queue for OpenLCB message pointers.
 *
 * @details Provides a fixed-capacity First-In-First-Out queue that holds pointers
 * to @ref openlcb_msg_t buffers allocated from @ref openlcb_buffer_store.h.  Messages
 * are delivered in the order they were pushed.  Must be initialized after
 * OpenLcbBufferStore_initialize() and before any push or pop operations.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
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
         * @brief Initializes the FIFO to an empty state.
         *
         * @details Clears all slots and resets head/tail. Must be called once
         * during startup after OpenLcbBufferStore_initialize().
         */
    extern void OpenLcbBufferFifo_initialize(void);

        /**
         * @brief Adds a message pointer to the tail of the FIFO.
         *
         * @param new_msg  Pointer to an @ref openlcb_msg_t allocated from OpenLcbBufferStore.
         *
         * @return The queued message pointer on success, or NULL if the FIFO is full.
         */
    extern openlcb_msg_t *OpenLcbBufferFifo_push(openlcb_msg_t *new_msg);

        /**
         * @brief Removes and returns the oldest message from the FIFO.
         *
         * @return Pointer to the oldest @ref openlcb_msg_t, or NULL if empty.
         */
    extern openlcb_msg_t *OpenLcbBufferFifo_pop(void);

        /** @brief Returns true if the FIFO contains no messages. */
    extern bool OpenLcbBufferFifo_is_empty(void);

        /** @brief Returns the number of messages currently held in the FIFO. */
    extern uint16_t OpenLcbBufferFifo_get_allocated_count(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_FIFO__ */
