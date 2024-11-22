/*
 * File:   openlcb_list.c
 * Author: jimkueneman
 *
 * Created on November 17, 2024, 2:10 PM
 */


#include "xc.h"
#include "openlcb_types.h"
#include "openlcb_buffer_store.h"



openlcb_msg_t* openlcb_msg_buffer_list[LEN_MESSAGE_BUFFER];

void BufferList_initialiaze() {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
        openlcb_msg_buffer_list[i] = (void*) 0;

}

openlcb_msg_t* BufferList_allocate(uint16_t data_len) {

    openlcb_msg_t* new_msg;

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (!openlcb_msg_buffer_list[i]) {

            new_msg = BufferStore_allocateBuffer(data_len);

            if (!new_msg)

                return (void*) 0;


            openlcb_msg_buffer_list[i] = new_msg;
            new_msg->owner = &openlcb_msg_buffer_list;

            return new_msg;

        }

    }

    return (void*) 0;

}

openlcb_msg_t* BufferList_find(uint16_t source_alias, uint16_t dest_alias, uint16_t mti) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (openlcb_msg_buffer_list[i]) {

            if ((openlcb_msg_buffer_list[i]->dest_alias == dest_alias) &&
                    (openlcb_msg_buffer_list[i]->source_alias == source_alias) &&
                    (openlcb_msg_buffer_list[i]->mti == mti))

                return openlcb_msg_buffer_list[i];

        }

    }

    return (void*) 0;

}

void BufferList_free(openlcb_msg_t * msg) {

    if (!msg)
        return;

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (openlcb_msg_buffer_list[i] == msg) {

            openlcb_msg_buffer_list[i] = (void*) 0;
            
            BufferStore_freeBuffer(msg);

            return;

        }

    }

}

void BufferList_release(openlcb_msg_t* msg) {

    if (!msg)
        return;

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        if (openlcb_msg_buffer_list[i] == msg) {

            openlcb_msg_buffer_list[i] = (void*) 0;

            return;
        }

        return;

    }

}

openlcb_msg_t* BufferList_index_of(uint16_t index) {
    
    if (index >= LEN_MESSAGE_BUFFER)
        return (void*) 0;
    
    return  openlcb_msg_buffer_list[index];

}

uint8_t BufferList_is_empty() {
    
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {
        
        if (openlcb_msg_buffer_list[i] != (void*) 0)
            
            return 1;
        
    }
    
    return 0;
    
}