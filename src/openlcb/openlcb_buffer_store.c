/*
 * File:   openlcb_buffer_store.c
 * Author: jimkueneman
 *
 * Created on November 17, 2024, 2:17 PM
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
            _message_buffer.messages[i].payload = (openlcb_payload_t*) &_message_buffer.basic[i];

        } else if (i < LEN_DATAGRAM_BUFFER + LEN_BASIC_BUFFER) {
            
            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_DATAGRAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t*) &_message_buffer.datagram[i - LEN_BASIC_BUFFER];

        } else if (i < LEN_SNIP_BUFFER + LEN_DATAGRAM_BUFFER + LEN_BASIC_BUFFER) {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_SNIP;
            _message_buffer.messages[i].payload = (openlcb_payload_t*) &_message_buffer.snip[i - (LEN_BASIC_BUFFER + LEN_DATAGRAM_BUFFER)];

        } else {

            _message_buffer.messages[i].payload_size = LEN_MESSAGE_BYTES_STREAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t*) &_message_buffer.stream[i - (LEN_BASIC_BUFFER + LEN_DATAGRAM_BUFFER + LEN_SNIP_BUFFER)];

        }

    }
}

openlcb_msg_t* BufferStore_allocateBuffer(uint16_t buffer_size) {

    uint16_t offset_start = 0;
    uint16_t offset_end = 0;

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


