/*
 * File:   openlcb_fifo.c
 * Author: jimkueneman
 *
 * Created on November 17, 2024, 2:08 PM
 */


#include "xc.h"
#include "openlcb_buffer_store.h"
#include "openlcb_types.h"

#define LEN_MESSAGE_FIFO_BUFFER  LEN_MESSAGE_BUFFER + 1  // add one slot to the fifo so it can be full without head == tail

typedef struct {
    openlcb_msg_t* list[LEN_MESSAGE_FIFO_BUFFER];  // add one slot to the fifo so it can be full without head == tail
    uint16_t head;
    uint16_t tail;

} openlcb_msg_fifo_t;

openlcb_msg_fifo_t openlcb_msg_buffer_fifo;

void BufferFifo_initialiaze() {

    for (int i = 0; i < LEN_MESSAGE_FIFO_BUFFER; i++)
        openlcb_msg_buffer_fifo.list[i] = (void*) 0;

    openlcb_msg_buffer_fifo.head = 0;
    openlcb_msg_buffer_fifo.tail = 0;

}

openlcb_msg_t* BufferFifo_push(uint16_t data_len) {

    uint16_t next = openlcb_msg_buffer_fifo.head + 1;
    if (next >= LEN_MESSAGE_FIFO_BUFFER)
        next = 0;

    if (next != openlcb_msg_buffer_fifo.tail) {

        openlcb_msg_t* new_msg = BufferStore_allocateBuffer(data_len);

        if (!new_msg)
            return (void*) 0;

        new_msg->owner = &openlcb_msg_buffer_fifo;
        openlcb_msg_buffer_fifo.list[openlcb_msg_buffer_fifo.head] = new_msg;
        openlcb_msg_buffer_fifo.head = next;
        
        return new_msg;

    }

    return (void*) 0;

}

openlcb_msg_t* BufferFifo_push_existing(openlcb_msg_t* existing_msg) {

    uint16_t next = openlcb_msg_buffer_fifo.head + 1;
    if (next >= LEN_MESSAGE_FIFO_BUFFER)
        next = 0;

    if (next != openlcb_msg_buffer_fifo.tail) {

        openlcb_msg_buffer_fifo.list[openlcb_msg_buffer_fifo.head] = existing_msg;
        openlcb_msg_buffer_fifo.head = next;
        
        existing_msg->owner = &openlcb_msg_buffer_fifo;
        
        return existing_msg;

    }

    return (void*) 0;

}

openlcb_msg_t* BufferFifo_pop() {

    openlcb_msg_t* result = (void*) 0;

    if (openlcb_msg_buffer_fifo.head != openlcb_msg_buffer_fifo.tail) {

        result = openlcb_msg_buffer_fifo.list[openlcb_msg_buffer_fifo.tail];

        openlcb_msg_buffer_fifo.tail = openlcb_msg_buffer_fifo.tail + 1;

        if (openlcb_msg_buffer_fifo.tail >= LEN_MESSAGE_FIFO_BUFFER)
            openlcb_msg_buffer_fifo.tail = 0;
        
        result->owner = (void*) 0;

    }

    return result;

}

uint8_t BufferFifo_is_empty(openlcb_msg_fifo_t* fifo) {

    return (openlcb_msg_buffer_fifo.head == openlcb_msg_buffer_fifo.tail);

}

uint16_t BufferFifo_get_allocated_count() {

    if (openlcb_msg_buffer_fifo.tail > openlcb_msg_buffer_fifo.head) {

        return (openlcb_msg_buffer_fifo.head + (LEN_MESSAGE_FIFO_BUFFER - openlcb_msg_buffer_fifo.tail));

    } else {

        return (openlcb_msg_buffer_fifo.head - openlcb_msg_buffer_fifo.tail);

    }

}
