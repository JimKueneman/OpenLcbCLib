
/*
 * File:   can_buffers.c
 * Author: Jim Kueneman
 *
 * Created on November 7, 2024, 6:09 AM
 */


#include "xc.h"
#include "can_types.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "can_buffer_store.h"


#define LEN_CAN_FIFO_BUFFER  LEN_CAN_CONTROL_FRAME_FIFO_BUFFER + 1  // add one slot to the fifo so it can be full without head == tail

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