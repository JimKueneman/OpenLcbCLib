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
 * \file openlcb_buffer_fifo.c
 *
 * Implementation of the FIFO where the OpenLcb messages are placed by the receive
 * module.  The main loop pulls them out one at a time and dispatched them to the handlers.
 * Caution is required as the receive module may be putting new messages in within the 
 * context of an interrupt or thread so when the main loop access this buffer it must 
 * use the lock the Can Rx and 100ms timer modules with the Pause and Resume functions 
 * exported by them respectively.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "openlcb_buffer_fifo.h"

#include "openlcb_buffer_store.h"
#include "openlcb_types.h"

#define LEN_MESSAGE_FIFO_BUFFER  LEN_MESSAGE_BUFFER + 1  // add one slot to the fifo so it can be full without head == tail

typedef struct {
    openlcb_msg_t* list[LEN_MESSAGE_FIFO_BUFFER];  // add one slot to the fifo so it can be full without head == tail
    uint16_olcb_t head;
    uint16_olcb_t tail;

} openlcb_msg_fifo_t;

openlcb_msg_fifo_t openlcb_msg_buffer_fifo;

void BufferFifo_initialiaze(void) {

    for (int i = 0; i < LEN_MESSAGE_FIFO_BUFFER; i++)
        openlcb_msg_buffer_fifo.list[i] = (void*) 0;

    openlcb_msg_buffer_fifo.head = 0;
    openlcb_msg_buffer_fifo.tail = 0;

}

openlcb_msg_t* BufferFifo_push(uint16_olcb_t data_len) {

    uint16_olcb_t next = openlcb_msg_buffer_fifo.head + 1;
    if (next >= LEN_MESSAGE_FIFO_BUFFER)
        next = 0;

    if (next != openlcb_msg_buffer_fifo.tail) {

        openlcb_msg_t* new_msg = BufferStore_allocateBuffer(data_len);

        if (!new_msg)
            return (void*) 0;
        
        openlcb_msg_buffer_fifo.list[openlcb_msg_buffer_fifo.head] = new_msg;
        openlcb_msg_buffer_fifo.head = next;
        
        return new_msg;

    }

    return (void*) 0;

}

openlcb_msg_t* BufferFifo_push_existing(openlcb_msg_t* existing_msg) {

    uint16_olcb_t next = openlcb_msg_buffer_fifo.head + 1;
    if (next >= LEN_MESSAGE_FIFO_BUFFER)
        next = 0;

    if (next != openlcb_msg_buffer_fifo.tail) {

        openlcb_msg_buffer_fifo.list[openlcb_msg_buffer_fifo.head] = existing_msg;
        openlcb_msg_buffer_fifo.head = next;
        
        return existing_msg;

    }

    return (void*) 0;

}

openlcb_msg_t* BufferFifo_pop(void) {

    openlcb_msg_t* result = (void*) 0;

    if (openlcb_msg_buffer_fifo.head != openlcb_msg_buffer_fifo.tail) {

        result = openlcb_msg_buffer_fifo.list[openlcb_msg_buffer_fifo.tail];

        openlcb_msg_buffer_fifo.tail = openlcb_msg_buffer_fifo.tail + 1;

        if (openlcb_msg_buffer_fifo.tail >= LEN_MESSAGE_FIFO_BUFFER)
            openlcb_msg_buffer_fifo.tail = 0;
       
    }

    return result;

}

uint8_olcb_t BufferFifo_is_empty(void) {

    return (openlcb_msg_buffer_fifo.head == openlcb_msg_buffer_fifo.tail);

}

uint16_olcb_t BufferFifo_get_allocated_count(void) {

    if (openlcb_msg_buffer_fifo.tail > openlcb_msg_buffer_fifo.head) {

        return (openlcb_msg_buffer_fifo.head + (LEN_MESSAGE_FIFO_BUFFER - openlcb_msg_buffer_fifo.tail));

    } else {

        return (openlcb_msg_buffer_fifo.head - openlcb_msg_buffer_fifo.tail);

    }

}
