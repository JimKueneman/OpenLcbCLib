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
 * \file openlcb_buffer_store.h
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

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_BUFFER_STORE__
#define __OPENLCB_BUFFER_STORE__

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void BufferStore_initialize(void);

    extern openlcb_msg_t *BufferStore_allocate_buffer(payload_type_enum_t payload_type);

    extern void BufferStore_free_buffer(openlcb_msg_t *openlcb_msg);

    extern void BufferStore_clear_openlcb_message(openlcb_msg_t *openlcb_msg);

    extern uint16_olcb_t BufferStore_basic_messages_allocated(void);

    extern uint16_olcb_t BufferStore_basic_messages_max_allocated(void);

    extern uint16_olcb_t BufferStore_datagram_messages_allocated(void);

    extern uint16_olcb_t BufferStore_datagram_messages_max_allocated(void);

    extern uint16_olcb_t BufferStore_snip_messages_allocated(void);

    extern uint16_olcb_t BufferStore_snip_messages_max_allocated(void);

    extern uint16_olcb_t BufferStore_stream_messages_allocated(void);

    extern uint16_olcb_t BufferStore_stream_messages_max_allocated(void);

    extern void BufferStore_inc_reference_count(openlcb_msg_t *openlcb_msg);

    void BufferStore_clear_max_allocated(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_BUFFER_STORE__ */
