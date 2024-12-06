
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
 * \file can_buffer_fifo.c
 *
 * Implements a FIFO of core buffers for CAN messages that hold CAN only frame messages 
 * that need to be passed on to the nodes.  This buffer is accessed in the CAN Rx
 * statemachine and the main loop so using Pause and Resume to stop the Rx and 100ms
 * timer when accessing it is critical.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


#include "can_types.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "can_buffer_store.h"


#define LEN_CAN_FIFO_BUFFER  LEN_CAN_BUFFER + 1  // add one slot to the fifo so it can be full without head == tail

typedef struct {
    can_msg_t* list[LEN_CAN_FIFO_BUFFER];  
    uint8_t head;
    uint8_t tail;
} can_fifo_t;


can_fifo_t can_msg_buffer_fifo;


void CanBufferFifo_initialiaze() {
    
    for (int i = 0; i < LEN_CAN_FIFO_BUFFER; i++) {

        can_msg_buffer_fifo.list[i] = (void*) 0;
        
    }

    can_msg_buffer_fifo.head = 0;
    can_msg_buffer_fifo.tail = 0;

}

can_msg_t* CanBufferFifo_push() {

    uint16_t next = can_msg_buffer_fifo.head + 1;
    if (next >= LEN_CAN_FIFO_BUFFER)
        next = 0;

    if (next != can_msg_buffer_fifo.tail) {
        
        can_msg_t* new_msg = CanBufferStore_allocateBuffer();
        
        if (!new_msg)
            return (void*) 0;

        can_msg_buffer_fifo.list[can_msg_buffer_fifo.head] = new_msg;

        can_msg_buffer_fifo.head = next;
        
        return new_msg;

    }

    return (void*) 0;

}

can_msg_t* CanBufferFifo_pop() {

    // Is there something here?
    if (can_msg_buffer_fifo.head != can_msg_buffer_fifo.tail) {

        can_msg_t* msg = can_msg_buffer_fifo.list[can_msg_buffer_fifo.tail];
        
        can_msg_buffer_fifo.list[can_msg_buffer_fifo.tail] = (void*) 0;

        can_msg_buffer_fifo.tail = can_msg_buffer_fifo.tail + 1;

        if (can_msg_buffer_fifo.tail >= LEN_CAN_FIFO_BUFFER)
            can_msg_buffer_fifo.tail = 0;

        return msg;
        
    }

    return (void*) 0;

}

uint8_t CanBufferFifo_is_empty() {

    return can_msg_buffer_fifo.head == can_msg_buffer_fifo.tail;

}

uint16_t CanBufferFifo_get_allocated_count() {

    if (can_msg_buffer_fifo.tail > can_msg_buffer_fifo.head) {

        return (can_msg_buffer_fifo.head + (LEN_CAN_FIFO_BUFFER - can_msg_buffer_fifo.tail));

    } else {

        return (can_msg_buffer_fifo.head - can_msg_buffer_fifo.tail);

    }

}