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
 * @file openlcb_buffer_list.h
 * @brief Random-access list of OpenLCB message pointers.
 *
 * @details Provides a fixed-size array of @ref openlcb_msg_t pointers that supports
 * both indexed access and attribute-based search (source alias, destination alias,
 * and MTI).  Primarily used for multi-frame message assembly where frames must be
 * matched to an in-progress message by sender and type.  Must be initialized after
 * OpenLcbBufferStore_initialize().
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_BUFFER_LIST__
#define __OPENLCB_OPENLCB_BUFFER_LIST__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
         * @brief Initializes the buffer list to an empty state.
         *
         * @details Clears all slots to NULL. Must be called once during startup
         * after OpenLcbBufferStore_initialize().
         */
    extern void OpenLcbBufferList_initialize(void);

        /**
         * @brief Inserts a message pointer into the first available slot.
         *
         * @param new_msg  Pointer to an @ref openlcb_msg_t allocated from OpenLcbBufferStore.
         *
         * @return The stored pointer on success, or NULL if the list is full.
         */
    extern openlcb_msg_t *OpenLcbBufferList_add(openlcb_msg_t *new_msg);

        /**
         * @brief Finds a message matching source alias, dest alias, and MTI.
         *
         * @param source_alias  12-bit CAN alias of the originating node.
         * @param dest_alias    12-bit CAN alias of the destination node.
         * @param mti           Message Type Indicator to match.
         *
         * @return Pointer to matching @ref openlcb_msg_t, or NULL if not found.
         */
    extern openlcb_msg_t *OpenLcbBufferList_find(uint16_t source_alias, uint16_t dest_alias, uint16_t mti);

        /**
         * @brief Removes a message from the list without freeing it.
         *
         * @param msg  Pointer to the @ref openlcb_msg_t to remove (NULL is safe).
         *
         * @return The removed pointer, or NULL if not found.
         */
    extern openlcb_msg_t *OpenLcbBufferList_release(openlcb_msg_t *msg);

        /**
         * @brief Returns the message pointer at the given index.
         *
         * @param index  Zero-based slot index (0 to LEN_MESSAGE_BUFFER - 1).
         *
         * @return Pointer to @ref openlcb_msg_t, or NULL if empty or out of range.
         */
    extern openlcb_msg_t *OpenLcbBufferList_index_of(uint16_t index);

        /** @brief Returns true if the list contains no messages. */
    extern bool OpenLcbBufferList_is_empty(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_LIST__ */
