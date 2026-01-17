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
 * @brief Core buffer store for OpenLcb/LCC message buffers with allocation and management
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_BUFFER_STORE__
#define __OPENLCB_OPENLCB_BUFFER_STORE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLcb Buffer Store
     * @param None
     * @return None
     * @attention This must always be called during application initialization
     */
    extern void OpenLcbBufferStore_initialize(void);

    /**
     * @brief Allocates a new buffer of the type passed
     * @param Type of buffer that is requested to be allocated (Basic, SNIP, Datagram, Stream)
     * @return Pointer to the message buffer or NULL if it fails
     */
    extern openlcb_msg_t *OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type);

    /**
     * @brief Frees the buffer so it can be reused
     * @param msg Pointer to a message to be freed
     * @return None
     * @note The buffers are reference counted
     * (see \ref OpenLcbBufferStore_inc_reference_count) so this call may or may not actually
     * release that buffer slot for reuse.  If the count drops to zero then it will be released
     * for reuse.
     */
    extern void OpenLcbBufferStore_free_buffer(openlcb_msg_t *msg);

    /**
     * @brief The number of Basic messages currently allocated in the buffer store
     * @param None
     * @return Number of Basic sized messages currently allocated
     */
    extern uint16_t OpenLcbBufferStore_basic_messages_allocated(void);

    /**
     * @brief The maximum number of Basic messages that have been allocated in the buffer store
     * Useful for understanding how deep the buffer store needs to be during stress testing.
     * @param None
     * @return Maximum number of Basic sized messages that have been allocated currently
     */
    extern uint16_t OpenLcbBufferStore_basic_messages_max_allocated(void);

    /**
     * @brief The number of Datagram messages currently allocated in the buffer store
     * @param None
     * @return Number of Datagram sized messages currently allocated
     */
    extern uint16_t OpenLcbBufferStore_datagram_messages_allocated(void);

    /**
     * @brief The maximum number of Datagram messages that have been allocated in the buffer store
     * Useful for understanding how deep the buffer store needs to be during stress testing.
     * @param None
     * @return Maximum number of Datagram sized messages that have been allocated currently
     */
    extern uint16_t OpenLcbBufferStore_datagram_messages_max_allocated(void);

    /**
     * @brief The number of SNIP message currently allocated in the buffer store
     * @param None
     * @return  Number of Basic sized messages currently allocated
     */
    extern uint16_t OpenLcbBufferStore_snip_messages_allocated(void);

    /**
     * @brief The maximum number of SNIP messages that have been allocated in the buffer store
     * Useful for understanding how deep the buffer store needs to be during stress testing.
     * @param None
     * @return Maximum number of SNIP sized messages that have been allocated currently
     */
    extern uint16_t OpenLcbBufferStore_snip_messages_max_allocated(void);

    /**
     * @brief The number of Stream message buffers currently allocated in the buffer store
     * @param None
     * @return Number of Stream sized messages currently allocated
     */
    extern uint16_t OpenLcbBufferStore_stream_messages_allocated(void);

    /**
     * @brief The maximum number of Stream messages that have been allocated in the buffer store
     * Useful for understanding how deep the buffer store needs to be during stress testing.
     * @param None
     * @return Maximum number of Stream sized messages that have been allocated currently
     */
    extern uint16_t OpenLcbBufferStore_stream_messages_max_allocated(void);

    /**
     * @brief Increases the reference count on the allocated buffer
     * @param msg Pointer to a message to increase the reference count
     * @return None
     * @note When \ref OpenLcbBufferStore_free_buffer(openlcb_msg_t *msg) is called it is only
     * freed when the reference count reaches zero.
     */
    extern void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg);

    /**
     * @brief Resets the running count of the Maximum number of buffer of each type have been allocated at a one time
     * @param None
     * @return None
     */
    void OpenLcbBufferStore_clear_max_allocated(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_STORE__ */
