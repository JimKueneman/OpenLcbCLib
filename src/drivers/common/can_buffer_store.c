/*
 * File:   can_buffer_store.c
 * Author: jimkueneman
 *
 * Created on November 18, 2024, 12:32 PM
 */


#include "xc.h"
#include "can_types.h"
#include "../../openlcb/openlcb_types.h"


can_buffer_store_t _can_buffer_store;
uint16_t _can_buffer_store_message_allocated = 0;

void CanBufferStore_initialize() {

    for (int i = 0; i < LEN_CAN_CONTROL_FRAME_FIFO_BUFFER; i++) {

        _can_buffer_store[i].allocated = FALSE;
        _can_buffer_store[i].identifier = 0;
        _can_buffer_store[i].payload_count = 0;
        for (int j = 0; j < LEN_CAN_BYTE_ARRAY; j++)
            _can_buffer_store[i].payload[j] = 0;

    }
}

void CanBufferStore_clear_can_message(can_msg_t* msg) {

    msg->identifier = 0;
    msg->payload_count = 0;
    
}

can_msg_t* CanBufferStore_allocateBuffer() {

    for (int i = 0; i < LEN_CAN_CONTROL_FRAME_FIFO_BUFFER; i++) {

        if (!_can_buffer_store[i].allocated) {

            _can_buffer_store_message_allocated = _can_buffer_store_message_allocated + 1;
            CanBufferStore_clear_can_message(&_can_buffer_store[i]);
            _can_buffer_store[i].allocated = TRUE;
            
            return &_can_buffer_store[i];

        }

    }
    
    return (void*) 0;

}

void CanBufferStore_freeBuffer(can_msg_t* msg) {

    if (!msg) return;

    _can_buffer_store_message_allocated = _can_buffer_store_message_allocated - 1;
    msg->allocated = FALSE;

}

uint16_t CanBufferStore_messages_allocated() {

    return _can_buffer_store_message_allocated;

}