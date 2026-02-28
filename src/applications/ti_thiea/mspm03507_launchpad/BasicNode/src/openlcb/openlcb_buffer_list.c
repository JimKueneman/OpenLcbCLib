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
 * @file openlcb_buffer_list.c
 * @brief Random-access list of OpenLCB message pointers.
 *
 * @details Fixed-size array with linear search.  NULL slots are free.
 * Supports indexed access and attribute-based search (alias + MTI).
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "openlcb_buffer_list.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_buffer_store.h"

/** @brief Static array of message pointers for the list */
static openlcb_msg_t *_openlcb_msg_buffer_list[LEN_MESSAGE_BUFFER];

    /**
    * @brief Initializes the buffer list.
    *
    * @details Algorithm:
    * -# Clear all slots to NULL
    */
void OpenLcbBufferList_initialize(void) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        _openlcb_msg_buffer_list[i] = NULL;

    }

}

    /**
    * @brief Inserts a message pointer into the first available slot.
    *
    * @details Algorithm:
    * -# Search for first NULL slot
    * -# Store the pointer and return it, or return NULL if full
    *
    * @verbatim
    * @param new_msg Pointer to @ref openlcb_msg_t from the buffer store
    * @endverbatim
    *
    * @return The stored pointer on success, or NULL if the list is full
    */
openlcb_msg_t *OpenLcbBufferList_add(openlcb_msg_t *new_msg) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (!_openlcb_msg_buffer_list[i]) {

            _openlcb_msg_buffer_list[i] = new_msg;

            return new_msg;

        }

    }

    return NULL;
}

    /**
    * @brief Finds a message matching source alias, dest alias, and MTI.
    *
    * @details Algorithm:
    * -# Search all non-NULL slots for a triple match
    * -# Return the first match, or NULL if none found
    *
    * @verbatim
    * @param source_alias CAN alias of sending node
    * @param dest_alias CAN alias of receiving node
    * @param mti Message Type Indicator code
    * @endverbatim
    *
    * @return Pointer to matching @ref openlcb_msg_t, or NULL if not found
    */
openlcb_msg_t *OpenLcbBufferList_find(uint16_t source_alias, uint16_t dest_alias, uint16_t mti) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (_openlcb_msg_buffer_list[i]) {

            if ((_openlcb_msg_buffer_list[i]->dest_alias == dest_alias) &&
                    (_openlcb_msg_buffer_list[i]->source_alias == source_alias) &&
                    (_openlcb_msg_buffer_list[i]->mti == mti)) {

                return _openlcb_msg_buffer_list[i];

            }

        }

    }

    return NULL;
}

    /**
    * @brief Removes a message from the list without freeing it.
    *
    * @details Algorithm:
    * -# If NULL, return NULL
    * -# Search for the pointer, clear the slot, return the pointer
    * -# Return NULL if not found
    *
    * @verbatim
    * @param msg Pointer to @ref openlcb_msg_t to remove (NULL is safe)
    * @endverbatim
    *
    * @return The removed pointer, or NULL if not found
    */
openlcb_msg_t *OpenLcbBufferList_release(openlcb_msg_t *msg) {

    if (!msg) {

        return NULL;

    }

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (_openlcb_msg_buffer_list[i] == msg) {

            _openlcb_msg_buffer_list[i] = NULL;

            return msg;

        }

    }

    return NULL;
}

    /** @brief Returns the message pointer at the given index, or NULL if empty or out of range. */
openlcb_msg_t *OpenLcbBufferList_index_of(uint16_t index) {

    if (index >= LEN_MESSAGE_BUFFER) {

        return NULL;

    }

    return _openlcb_msg_buffer_list[index];
}

    /** @brief Returns true if the list contains no messages. */
bool OpenLcbBufferList_is_empty(void) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (_openlcb_msg_buffer_list[i] != NULL) {

            return false;

        }

    }

    return true;
}
