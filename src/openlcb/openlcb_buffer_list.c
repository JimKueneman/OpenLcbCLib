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
 * \file openlcb_buffer_list.c
 *
 * A linear search list that the incoming CAN Rx module uses to hold messages that
 * are being collected into a single OpenLcb message on the CAN bus.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "openlcb_buffer_list.h"

#include "openlcb_types.h"
#include "openlcb_buffer_store.h"

openlcb_msg_t *openlcb_msg_buffer_list[LEN_MESSAGE_BUFFER];

void BufferList_initialize(void)
{

    for (olcb_int_t i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {

        openlcb_msg_buffer_list[i] = (void *)0;
    }
}

openlcb_msg_t *BufferList_add(openlcb_msg_t *new_msg)
{

    if (!new_msg)
    {

        return (void *)0;
    }

    for (olcb_int_t i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {

        if (!openlcb_msg_buffer_list[i])
        {

            openlcb_msg_buffer_list[i] = new_msg;

            return new_msg;
        }
    }

    return (void *)0;
}

openlcb_msg_t *BufferList_find(uint16_olcb_t source_alias, uint16_olcb_t dest_alias, uint16_olcb_t mti)
{

    for (olcb_int_t i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {

        if (openlcb_msg_buffer_list[i])
        {

            if ((openlcb_msg_buffer_list[i]->dest_alias == dest_alias) &&
                (openlcb_msg_buffer_list[i]->source_alias == source_alias) &&
                (openlcb_msg_buffer_list[i]->mti == mti))

                return openlcb_msg_buffer_list[i];
        }
    }

    return (void *)0;
}

uint8_olcb_t BufferList_free(openlcb_msg_t *msg)
{

    if (!msg)
        return FALSE;

    for (olcb_int_t i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {

        if (openlcb_msg_buffer_list[i] == msg)
        {

            openlcb_msg_buffer_list[i] = (void *)0;

            BufferStore_free_buffer(msg);

            return TRUE;
        }
    }

    return FALSE;
}

openlcb_msg_t *BufferList_release(openlcb_msg_t *msg)
{

    if (!msg)
        return (void *)0;

    for (olcb_int_t i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {

        if (openlcb_msg_buffer_list[i] == msg)
        {

            openlcb_msg_buffer_list[i] = (void *)0;

            return msg;
        }
    }

    return (void *)0;
}

openlcb_msg_t *BufferList_index_of(uint16_olcb_t index)
{

    if (index >= LEN_MESSAGE_BUFFER)
        return (void *)0;

    return openlcb_msg_buffer_list[index];
}

uint8_olcb_t BufferList_is_empty(void)
{

    for (olcb_int_t i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {

        if (openlcb_msg_buffer_list[i] != (void *)0)

            return FALSE;
    }

    return TRUE;
}
