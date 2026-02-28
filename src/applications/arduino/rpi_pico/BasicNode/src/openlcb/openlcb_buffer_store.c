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
 * @file openlcb_buffer_store.c
 * @brief Pre-allocated message pool for OpenLCB buffer management.
 *
 * @details Uses segregated pools for BASIC, DATAGRAM, SNIP, and STREAM payload
 * sizes.  Reference counting supports shared buffers across multiple queues.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "openlcb_buffer_store.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"

/** @brief Main buffer pool containing all message structures and payload buffers */
static message_buffer_t _message_buffer;

/** @brief Current number of allocated BASIC messages */
static uint16_t _buffer_store_basic_messages_allocated = 0;

/** @brief Current number of allocated DATAGRAM messages */
static uint16_t _buffer_store_datagram_messages_allocated = 0;

/** @brief Current number of allocated SNIP messages */
static uint16_t _buffer_store_snip_messages_allocated = 0;

/** @brief Current number of allocated STREAM messages */
static uint16_t _buffer_store_stream_messages_allocated = 0;

/** @brief Peak number of BASIC messages allocated simultaneously */
static uint16_t _buffer_store_basic_max_messages_allocated = 0;

/** @brief Peak number of DATAGRAM messages allocated simultaneously */
static uint16_t _buffer_store_datagram_max_messages_allocated = 0;

/** @brief Peak number of SNIP messages allocated simultaneously */
static uint16_t _buffer_store_snip_max_messages_allocated = 0;

/** @brief Peak number of STREAM messages allocated simultaneously */
static uint16_t _buffer_store_stream_max_messages_allocated = 0;


    /**
    * @brief Initializes the buffer store.
    *
    * @details Algorithm:
    * -# Clear each message structure
    * -# Link each slot to its payload buffer based on pool segment (BASIC, DATAGRAM, SNIP, STREAM)
    * -# Reset all allocation and peak counters to zero
    */
void OpenLcbBufferStore_initialize(void) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        OpenLcbUtilities_clear_openlcb_message(&_message_buffer.messages[i]);

        if (i < USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_type = BASIC;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) &_message_buffer.basic[i];

        } else if (i < USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_type = DATAGRAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) &_message_buffer.datagram[i - USER_DEFINED_BASIC_BUFFER_DEPTH];

        } else if (i < USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_type = SNIP;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) &_message_buffer.snip[i - (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH)];

        } else {

            _message_buffer.messages[i].payload_type = STREAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) &_message_buffer.stream[i - (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH)];

        }
    }

    _buffer_store_basic_messages_allocated = 0;
    _buffer_store_datagram_messages_allocated = 0;
    _buffer_store_snip_messages_allocated = 0;
    _buffer_store_stream_messages_allocated = 0;

    _buffer_store_basic_max_messages_allocated = 0;
    _buffer_store_datagram_max_messages_allocated = 0;
    _buffer_store_snip_max_messages_allocated = 0;
    _buffer_store_stream_max_messages_allocated = 0;

}

    /** @brief Increments the current and peak allocation counters for the given pool type. */
static void _update_buffer_telemetry(payload_type_enum payload_type) {

    switch (payload_type) {

        case BASIC:

            _buffer_store_basic_messages_allocated++;
            if (_buffer_store_basic_messages_allocated > _buffer_store_basic_max_messages_allocated) {

                _buffer_store_basic_max_messages_allocated = _buffer_store_basic_messages_allocated;

                break;
            }

            break;

        case DATAGRAM:

            _buffer_store_datagram_messages_allocated++;
            if (_buffer_store_datagram_messages_allocated > _buffer_store_datagram_max_messages_allocated) {

                _buffer_store_datagram_max_messages_allocated = _buffer_store_datagram_messages_allocated;

                break;
            }

            break;

        case SNIP:

            _buffer_store_snip_messages_allocated++;
            if (_buffer_store_snip_messages_allocated > _buffer_store_snip_max_messages_allocated) {

                _buffer_store_snip_max_messages_allocated = _buffer_store_snip_messages_allocated;

                break;
            }

            break;

        case STREAM:

            _buffer_store_stream_messages_allocated++;
            if (_buffer_store_stream_messages_allocated > _buffer_store_stream_max_messages_allocated) {

                _buffer_store_stream_max_messages_allocated = _buffer_store_stream_messages_allocated;

                break;
            }

            break;

        default:

            break;

    }
}

    /**
    * @brief Allocates a buffer from the specified pool.
    *
    * @details Algorithm:
    * -# Determine pool segment range from payload_type
    * -# Search for first unallocated slot in that range
    * -# Clear the message, set reference_count to 1, mark allocated
    * -# Update telemetry and return the pointer (or NULL if exhausted)
    *
    * @verbatim
    * @param payload_type Type of buffer requested (BASIC, DATAGRAM, SNIP, or STREAM)
    * @endverbatim
    *
    * @return Pointer to allocated @ref openlcb_msg_t, or NULL if pool exhausted
    */
openlcb_msg_t *OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type) {

    uint8_t offset_start = 0;
    uint8_t offset_end = 0;

    switch (payload_type) {

        case BASIC:

            offset_start = 0;
            offset_end = USER_DEFINED_BASIC_BUFFER_DEPTH;

            break;

        case DATAGRAM:

            offset_start = USER_DEFINED_BASIC_BUFFER_DEPTH;
            offset_end = offset_start + USER_DEFINED_DATAGRAM_BUFFER_DEPTH;

            break;

        case SNIP:

            offset_start = USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH;
            offset_end = offset_start + USER_DEFINED_SNIP_BUFFER_DEPTH;

            break;

        case STREAM:

            offset_start = USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH;
            offset_end = offset_start + USER_DEFINED_STREAM_BUFFER_DEPTH;

            break;

        default:

            return NULL;

    }

    for (int i = offset_start; i < offset_end; i++) {

        if (!_message_buffer.messages[i].state.allocated) {

            OpenLcbUtilities_clear_openlcb_message(&_message_buffer.messages[i]);
            _message_buffer.messages[i].reference_count = 1;
            _message_buffer.messages[i].state.allocated = true;
            _update_buffer_telemetry(_message_buffer.messages[i].payload_type);

            return &_message_buffer.messages[i];
        }
    }

    return NULL;

}

    /**
    * @brief Decrements the reference count; frees the buffer when it reaches zero.
    *
    * @details Algorithm:
    * -# If NULL, return immediately
    * -# Decrement reference_count; if still positive, return
    * -# Decrement the pool allocation counter and mark the slot as free
    *
    * @verbatim
    * @param msg Pointer to @ref openlcb_msg_t to release (NULL is safe)
    * @endverbatim
    */
void OpenLcbBufferStore_free_buffer(openlcb_msg_t *msg) {

    if (!msg) {

        return;

    }

    msg->reference_count = msg->reference_count - 1;

    if (msg->reference_count > 0) {

        return;

    }

    switch (msg->payload_type) {

        case BASIC:

            _buffer_store_basic_messages_allocated = _buffer_store_basic_messages_allocated - 1;

            break;

        case DATAGRAM:

            _buffer_store_datagram_messages_allocated = _buffer_store_datagram_messages_allocated - 1;

            break;

        case SNIP:

            _buffer_store_snip_messages_allocated = _buffer_store_snip_messages_allocated - 1;

            break;

        case STREAM:

            _buffer_store_stream_messages_allocated = _buffer_store_stream_messages_allocated - 1;

            break;

    }

    msg->reference_count = 0;

    msg->state.allocated = false;

}

    /** @brief Returns the number of BASIC messages currently allocated. */
uint16_t OpenLcbBufferStore_basic_messages_allocated(void) {

    return (_buffer_store_basic_messages_allocated);

}

    /** @brief Returns the peak number of BASIC messages allocated simultaneously. */
uint16_t OpenLcbBufferStore_basic_messages_max_allocated(void) {

    return (_buffer_store_basic_max_messages_allocated);

}

    /** @brief Returns the number of DATAGRAM messages currently allocated. */
uint16_t OpenLcbBufferStore_datagram_messages_allocated(void) {

    return (_buffer_store_datagram_messages_allocated);

}

    /** @brief Returns the peak number of DATAGRAM messages allocated simultaneously. */
uint16_t OpenLcbBufferStore_datagram_messages_max_allocated(void) {

    return (_buffer_store_datagram_max_messages_allocated);

}

    /** @brief Returns the number of SNIP messages currently allocated. */
uint16_t OpenLcbBufferStore_snip_messages_allocated(void) {

    return (_buffer_store_snip_messages_allocated);

}

    /** @brief Returns the peak number of SNIP messages allocated simultaneously. */
uint16_t OpenLcbBufferStore_snip_messages_max_allocated(void) {

    return (_buffer_store_snip_max_messages_allocated);

}

    /** @brief Returns the number of STREAM messages currently allocated. */
uint16_t OpenLcbBufferStore_stream_messages_allocated(void) {

    return (_buffer_store_stream_messages_allocated);

}

    /** @brief Returns the peak number of STREAM messages allocated simultaneously. */
uint16_t OpenLcbBufferStore_stream_messages_max_allocated(void) {

    return (_buffer_store_stream_max_messages_allocated);

}

    /** @brief Increments the reference count on an allocated buffer. */
void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg) {

    msg->reference_count = msg->reference_count + 1;

}

    /** @brief Resets all peak allocation counters to zero. */
void OpenLcbBufferStore_clear_max_allocated(void) {

    _buffer_store_basic_max_messages_allocated = 0;
    _buffer_store_datagram_max_messages_allocated = 0;
    _buffer_store_snip_max_messages_allocated = 0;
    _buffer_store_stream_max_messages_allocated = 0;

}
