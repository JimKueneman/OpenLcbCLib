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
 * \file openlcb_buffer_list.h
 *
 * A linear search list that the incoming CAN Rx module uses to hold messages that
 * are being collected into a single OpenLcb message on the CAN bus.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_LIST__
#define __OPENLCB_LIST__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void BufferList_initialize(void);

    extern openlcb_msg_t *BufferList_add(openlcb_msg_t *new_msg);

    extern openlcb_msg_t *BufferList_find(uint16_t source_alias, uint16_t dest_alias, uint16_t mti);

    extern uint8_t BufferList_free(openlcb_msg_t *msg);

    extern openlcb_msg_t *BufferList_release(openlcb_msg_t *msg);

    extern openlcb_msg_t *BufferList_index_of(uint16_t index);

    extern uint8_t BufferList_is_empty(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_LIST__ */
