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


#include "xc.h"
#include "stdio.h"  // printf
#include "openlcb_types.h"

message_buffer_t _message_buffer;
uint16_t _buffer_store_message_allocated = 0;
uint16_t _buffer_store_message_max_allocated = 0;

void _clear_openlcb_message(openlcb_msg_t* openlcb_msg) {


    openlcb_msg->dest_alias = 0;
    openlcb_msg->dest_id = 0;
    openlcb_msg->source_alias = 0;
    openlcb_msg->source_id = 0;
    openlcb_msg->mti = 0;
    openlcb_msg->payload_count = 0;
    openlcb_msg->timerticks = 0;
    openlcb_msg->retry_count = 0;
    openlcb_msg->reference_count = 0;
    openlcb_msg->state.allocated = FALSE;
    openlcb_msg->state.inprocess = FALSE;

}

void BufferStore_initialize() {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        _clear_openlcb_message(&_message_buffer.messages[i]);

        if (i < LEN_BASIC_BUFFER) {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_BASIC;
            _message_buffer.messages[i].payload = (openlcb_payload_t*) & _message_buffer.basic[i];

        } else if (i < LEN_DATAGRAM_BUFFER + LEN_BASIC_BUFFER) {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_DATAGRAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t*) & _message_buffer.datagram[i - LEN_BASIC_BUFFER];

        } else if (i < LEN_SNIP_BUFFER + LEN_DATAGRAM_BUFFER + LEN_BASIC_BUFFER) {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_SNIP;
            _message_buffer.messages[i].payload = (openlcb_payload_t*) & _message_buffer.snip[i - (LEN_BASIC_BUFFER + LEN_DATAGRAM_BUFFER)];

        } else {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_STREAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t*) & _message_buffer.stream[i - (LEN_BASIC_BUFFER + LEN_DATAGRAM_BUFFER + LEN_SNIP_BUFFER)];

        }

    }
}

openlcb_msg_t* BufferStore_allocateBuffer(uint16_t buffer_size) {

    uint16_t offset_start = 0;
    uint16_t offset_end = 0;

    if (buffer_size <= LEN_MESSAGE_BYTES_BASIC) {
        
        buffer_size = LEN_MESSAGE_BYTES_BASIC;
    
    } else if (buffer_size <= LEN_MESSAGE_BYTES_DATAGRAM) {
        
        buffer_size = LEN_MESSAGE_BYTES_DATAGRAM;
    
    } else if (buffer_size <= LEN_MESSAGE_BYTES_SNIP) {
        
        buffer_size = LEN_MESSAGE_BYTES_SNIP;
    
    } else 
        buffer_size = LEN_MESSAGE_BYTES_STREAM;


    switch (buffer_size) {

        case LEN_MESSAGE_BYTES_BASIC:
            offset_start = 0;
            offset_end = LEN_BASIC_BUFFER;
            break;
        case LEN_MESSAGE_BYTES_DATAGRAM:
            offset_start = LEN_BASIC_BUFFER;
            offset_end = offset_start + LEN_DATAGRAM_BUFFER;
            break;
        case LEN_MESSAGE_BYTES_SNIP:
            offset_start = LEN_BASIC_BUFFER + LEN_DATAGRAM_BUFFER;
            offset_end = offset_start + LEN_SNIP_BUFFER;
            break;
        case LEN_MESSAGE_BYTES_STREAM:
            offset_start = LEN_BASIC_BUFFER + LEN_DATAGRAM_BUFFER + LEN_SNIP_BUFFER;
            offset_end = offset_start + LEN_STREAM_BUFFER;
            break;

    }

    for (int i = offset_start; i < offset_end; i++) {

        if (!_message_buffer.messages[i].state.allocated) {

            _buffer_store_message_allocated = _buffer_store_message_allocated + 1;

            if (_buffer_store_message_allocated > _buffer_store_message_max_allocated)
                _buffer_store_message_max_allocated = _buffer_store_message_allocated;
            
            _clear_openlcb_message(&_message_buffer.messages[i]);
            
            _message_buffer.messages[i].reference_count = 1;

            _message_buffer.messages[i].state.allocated = TRUE;

            return &_message_buffer.messages[i];

        }

    }

    return (void*) 0;

}

void BufferStore_freeBuffer(openlcb_msg_t* openlcb_msg) {

    if (!openlcb_msg)
        return;
    
    openlcb_msg->reference_count = openlcb_msg->reference_count - 1;
    
    if (openlcb_msg->reference_count > 0)
        return;

    _buffer_store_message_allocated = _buffer_store_message_allocated - 1;
    openlcb_msg->reference_count = 0;

    openlcb_msg->state.allocated = FALSE;

}

uint16_t BufferStore_messages_allocated() {

    return _buffer_store_message_allocated;

}

uint16_t BufferStore_messages_max_allocated() {

    return _buffer_store_message_max_allocated;

}

extern void BufferStore_inc_reference_count(openlcb_msg_t* openlcb_msg) {
    
    openlcb_msg->reference_count = openlcb_msg->reference_count + 1;
    
}


