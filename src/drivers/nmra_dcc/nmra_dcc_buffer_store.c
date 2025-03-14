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

#include "nmra_dcc_buffer_store.h"

#include "stdio.h"  // printf
#include "../../openlcb/openlcb_types.h"
#include "nmra_dcc_types.h"

nmra_dcc_message_t _nmra_dcc_buffer[LEN_NMRA_DCC_BUFFER];
uint16_olcb_t  _nrma_dcc_buffer_store_message_allocated = 0;
uint16_olcb_t _nrma_dcc_buffer_store_message_max_allocated = 0;

void _clear_nmra_dcc_message(nmra_dcc_message_t* nmra_dcc_msg) {

    nmra_dcc_msg->reference_count = 0;
    nmra_dcc_msg->allocated = FALSE;
    nmra_dcc_msg->buffer_byte_bit_index = 0;
    nmra_dcc_msg->buffer_byte_index = 0;
    nmra_dcc_msg->preamble_size = 12;
    nmra_dcc_msg->preamble_position = 0;
    nmra_dcc_msg->xor_byte = 0;
    nmra_dcc_msg->next_callback = (void *) 0;
    nmra_dcc_msg->buffer_size = 0;
    nmra_dcc_msg->next_callback_parameter = 0;
    nmra_dcc_msg->state = STATE_NMRA_DCC_PREAMBLE;

}

void NmraDccBufferStore_initialize(void) {

    for (int i = 0; i < LEN_NMRA_DCC_BUFFER; i++) {

        _clear_nmra_dcc_message(&_nmra_dcc_buffer[i]);

    }
}

nmra_dcc_message_t* NmraDccBufferStore_allocateBuffer() {

    for (int i = 0; i < LEN_NMRA_DCC_BUFFER; i++) {

        if (!_nmra_dcc_buffer[i].allocated) {

            _nrma_dcc_buffer_store_message_allocated = _nrma_dcc_buffer_store_message_allocated + 1;

            if (_nrma_dcc_buffer_store_message_allocated > _nrma_dcc_buffer_store_message_max_allocated)
                _nrma_dcc_buffer_store_message_max_allocated = _nrma_dcc_buffer_store_message_allocated;

            _clear_nmra_dcc_message(&_nmra_dcc_buffer[i]);

            _nmra_dcc_buffer[i].reference_count = 1;

            _nmra_dcc_buffer[i].allocated = TRUE;

            return &_nmra_dcc_buffer[i];

        }

    }

    return (void*) 0;

}

void NmraDccBufferStore_freeBuffer(nmra_dcc_message_t* nmra_dcc_msg) {

    if (!nmra_dcc_msg)
        return;

    nmra_dcc_msg->reference_count = nmra_dcc_msg->reference_count - 1;

    if (nmra_dcc_msg->reference_count > 0)
        return;

    _nrma_dcc_buffer_store_message_allocated = _nrma_dcc_buffer_store_message_allocated - 1;
    nmra_dcc_msg->reference_count = 0;

    nmra_dcc_msg->allocated = FALSE;

}

uint16_olcb_t NmraDccBufferStore_messages_allocated(void) {

    return _nrma_dcc_buffer_store_message_allocated;

}

uint16_olcb_t NmraDccBufferStore_messages_max_allocated(void) {

    return _nrma_dcc_buffer_store_message_max_allocated;

}

extern void NmraDccBufferStore_inc_reference_count(nmra_dcc_message_t* nmra_dcc_msg) {

    nmra_dcc_msg->reference_count = nmra_dcc_msg->reference_count + 1;

}


