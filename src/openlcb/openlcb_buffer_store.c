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
 * \file openlcb_buffer_store.c
 *
 * Implements the core buffers for normal, snip, datagram, and stream length buffers.
 * The FIFO and List buffers are arrays of pointers to these core buffers that are
 * allocated and freed through access.  The CAN Rx and 100ms timer access these buffers
 * so care must be taken to Pause and Resume those calls if the main loop needs to
 * access the buffers.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "openlcb_buffer_store.h"

#include "stdio.h" // printf
#include "openlcb_types.h"

message_buffer_t _message_buffer;

uint16_olcb_t _buffer_store_basic_messages_allocated = 0;
uint16_olcb_t _buffer_store_datagram_messages_allocated = 0;
uint16_olcb_t _buffer_store_snip_messages_allocated = 0;
uint16_olcb_t _buffer_store_stream_messages_allocated = 0;

uint16_olcb_t _buffer_store_basic_max_messages_allocated = 0;
uint16_olcb_t _buffer_store_datagram_max_messages_allocated = 0;
uint16_olcb_t _buffer_store_snip_max_messages_allocated = 0;
uint16_olcb_t _buffer_store_stream_max_messages_allocated = 0;

void _clear_openlcb_message(openlcb_msg_t *openlcb_msg) {

    openlcb_msg->dest_alias = 0;
    openlcb_msg->dest_id = 0;
    openlcb_msg->source_alias = 0;
    openlcb_msg->source_id = 0;
    openlcb_msg->mti = 0;
    openlcb_msg->payload_count = 0;
    openlcb_msg->timerticks = 0;
    openlcb_msg->reference_count = 0;
    openlcb_msg->state.allocated = FALSE;
    openlcb_msg->state.inprocess = FALSE;
}

void BufferStore_initialize(void) {

    for (olcb_int_t i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        _clear_openlcb_message(&_message_buffer.messages[i]);

        if (i < USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_BASIC;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) & _message_buffer.basic[i];
        } else if (i < USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_DATAGRAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) & _message_buffer.datagram[i - USER_DEFINED_BASIC_BUFFER_DEPTH];
        } else if (i < USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_SNIP;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) & _message_buffer.snip[i - (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH)];
        } else {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_STREAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) & _message_buffer.stream[i - (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH)];
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

void _update_buffer_telemetry(payload_type_enum_t payload_type) {

    switch (payload_type) {

        case BASIC:

            _buffer_store_basic_messages_allocated++;
            if (_buffer_store_basic_messages_allocated > _buffer_store_basic_max_messages_allocated) {

                _buffer_store_basic_max_messages_allocated++;
                break;
            } else {
                break;  // GoogleTest likes this better for coverage metrics
            }
        case DATAGRAM:

            _buffer_store_datagram_messages_allocated++;
            if (_buffer_store_datagram_messages_allocated > _buffer_store_datagram_max_messages_allocated) {

                _buffer_store_datagram_max_messages_allocated++;
                break;
            } else {
                break; // GoogleTest likes this better for coverage metrics
            }
        case SNIP:

            _buffer_store_snip_messages_allocated++;
            if (_buffer_store_snip_messages_allocated > _buffer_store_snip_max_messages_allocated) {

                _buffer_store_snip_max_messages_allocated++;
                break;
            } else {
                break; // GoogleTest likes this better for coverage metrics
            }
        case STREAM:

            _buffer_store_stream_messages_allocated++;
            if (_buffer_store_stream_messages_allocated > _buffer_store_stream_max_messages_allocated) {

                _buffer_store_stream_max_messages_allocated++;
                break;
            } else {
                break; // GoogleTest likes this better for coverage metrics
            }
        default:
            break;
    }
}

openlcb_msg_t *BufferStore_allocate_buffer(payload_type_enum_t payload_type) {

    uint8_olcb_t offset_start = 0;
    uint8_olcb_t offset_end = 0;

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
            return (void *) 0;
    }

    for (olcb_int_t i = offset_start; i < offset_end; i++) {

        if (!_message_buffer.messages[i].state.allocated) {

            _clear_openlcb_message(&_message_buffer.messages[i]);

            _message_buffer.messages[i].reference_count = 1;

            _message_buffer.messages[i].state.allocated = TRUE;

            _update_buffer_telemetry(_message_buffer.messages[i].payload_size);

            return &_message_buffer.messages[i];
        }
    }

    return (void *) 0;
}

void BufferStore_free_buffer(openlcb_msg_t *openlcb_msg) {

    if (!openlcb_msg)
        return;

    openlcb_msg->reference_count = openlcb_msg->reference_count - 1;

    if (openlcb_msg->reference_count > 0)
        return;

    switch (openlcb_msg->payload_size) {

        case LEN_MESSAGE_BYTES_BASIC:
            _buffer_store_basic_messages_allocated = _buffer_store_basic_messages_allocated - 1;

            break;
        case LEN_MESSAGE_BYTES_DATAGRAM:
            _buffer_store_datagram_messages_allocated = _buffer_store_datagram_messages_allocated - 1;

            break;
        case LEN_MESSAGE_BYTES_SNIP:
            _buffer_store_snip_messages_allocated = _buffer_store_snip_messages_allocated - 1;

            break;
        case LEN_MESSAGE_BYTES_STREAM:
            _buffer_store_stream_messages_allocated = _buffer_store_stream_messages_allocated - 1;

            break;
    }

    openlcb_msg->reference_count = 0;

    openlcb_msg->state.allocated = FALSE;
}

uint16_olcb_t BufferStore_basic_messages_allocated(void) {

    return (_buffer_store_basic_messages_allocated);
}

uint16_olcb_t BufferStore_basic_messages_max_allocated(void) {

    return (_buffer_store_basic_max_messages_allocated);
}

uint16_olcb_t BufferStore_datagram_messages_allocated(void) {

    return (_buffer_store_datagram_messages_allocated);
}

uint16_olcb_t BufferStore_datagram_messages_max_allocated(void) {

    return (_buffer_store_datagram_max_messages_allocated);
}

uint16_olcb_t BufferStore_snip_messages_allocated(void) {

    return (_buffer_store_snip_messages_allocated);
}

uint16_olcb_t BufferStore_snip_messages_max_allocated(void) {

    return (_buffer_store_snip_max_messages_allocated);
}

uint16_olcb_t BufferStore_stream_messages_allocated(void) {

    return (_buffer_store_stream_messages_allocated);
}

uint16_olcb_t BufferStore_stream_messages_max_allocated(void) {

    return (_buffer_store_stream_max_messages_allocated);
}

void BufferStore_inc_reference_count(openlcb_msg_t *openlcb_msg) {

    openlcb_msg->reference_count = openlcb_msg->reference_count + 1;
}

void BufferStore_clear_max_allocated(void) {

    _buffer_store_basic_max_messages_allocated = 0;
    _buffer_store_datagram_max_messages_allocated = 0;
    _buffer_store_snip_max_messages_allocated = 0;
    _buffer_store_stream_max_messages_allocated = 0;
}
