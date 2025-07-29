
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
 * \file can_buffer_fifo.h
 *
 * Implements a FIFO of core buffers for CAN messages that hold CAN only frame messages
 * that need to be passed on to the nodes.  This buffer is accessed in the CAN Rx
 * statemachine and the main loop so using Pause and Resume to stop the Rx and 100ms
 * timer when accessing it is critical.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __CAN_BUFFERS__
#define __CAN_BUFFERS__

#include "can_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void CanBufferFifo_initialize(void);

    extern uint8_olcb_t *CanBufferFifo_push(can_msg_t* new_msg);

    extern can_msg_t *CanBufferFifo_pop(void);

    extern uint8_olcb_t CanBufferFifo_is_empty(void);

    extern uint16_olcb_t CanBufferFifo_get_allocated_count(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CAN_BUFFERS__ */
