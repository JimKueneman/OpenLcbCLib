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
 * @file openlcb_buffer_store.h
 * @brief Pre-allocated message pool for OpenLCB buffer management.
 *
 * @details Provides fixed-size pools for BASIC, DATAGRAM, SNIP, and STREAM message
 * types.  All memory is allocated at startup; there is no dynamic allocation at
 * runtime.  Reference counting lets the same buffer be held by multiple queues
 * simultaneously.  Must be initialized before any other OpenLCB module.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_BUFFER_STORE__
#define __OPENLCB_OPENLCB_BUFFER_STORE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef	__cplusplus
  extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Initializes the buffer store.
         *
         * @details Clears all message structures, links payloads, and resets counters.
         * Must be called once at startup before any allocation.
         */
    extern void OpenLcbBufferStore_initialize(void);

        /**
         * @brief Allocates a buffer from the specified pool.
         *
         * @param payload_type  @ref payload_type_enum (BASIC, DATAGRAM, SNIP, or STREAM).
         *
         * @return Pointer to the allocated @ref openlcb_msg_t, or NULL if the pool is exhausted.
         */
    extern openlcb_msg_t *OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type);

        /**
         * @brief Decrements the reference count; frees the buffer when it reaches zero.
         *
         * @param msg  Pointer to the @ref openlcb_msg_t to release (NULL is safe).
         */
    extern void OpenLcbBufferStore_free_buffer(openlcb_msg_t *msg);

        /**
         * @brief Increments the reference count on an allocated buffer.
         *
         * @param msg  Pointer to the @ref openlcb_msg_t to share.
         */
    extern void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg);

        /** @brief Returns the number of BASIC messages currently allocated. */
    extern uint16_t OpenLcbBufferStore_basic_messages_allocated(void);

        /** @brief Returns the peak number of BASIC messages allocated simultaneously. */
    extern uint16_t OpenLcbBufferStore_basic_messages_max_allocated(void);

        /** @brief Returns the number of DATAGRAM messages currently allocated. */
    extern uint16_t OpenLcbBufferStore_datagram_messages_allocated(void);

        /** @brief Returns the peak number of DATAGRAM messages allocated simultaneously. */
    extern uint16_t OpenLcbBufferStore_datagram_messages_max_allocated(void);

        /** @brief Returns the number of SNIP messages currently allocated. */
    extern uint16_t OpenLcbBufferStore_snip_messages_allocated(void);

        /** @brief Returns the peak number of SNIP messages allocated simultaneously. */
    extern uint16_t OpenLcbBufferStore_snip_messages_max_allocated(void);

        /** @brief Returns the number of STREAM messages currently allocated. */
    extern uint16_t OpenLcbBufferStore_stream_messages_allocated(void);

        /** @brief Returns the peak number of STREAM messages allocated simultaneously. */
    extern uint16_t OpenLcbBufferStore_stream_messages_max_allocated(void);

        /** @brief Resets all peak allocation counters to zero. */
    extern void OpenLcbBufferStore_clear_max_allocated(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_STORE__ */
