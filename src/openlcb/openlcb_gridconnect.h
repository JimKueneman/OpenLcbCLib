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

#ifndef __OPENLCB_GRIDCONNECT__
#define	__OPENLCB_GRIDCONNECT__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"
#include "../drivers/common/can_types.h"

#define GRIDCONNECT_STATE_SYNC_START 0
#define GRIDCONNECT_STATE_SYNC_FIND_HEADER 2
#define GRIDCONNECT_STATE_SYNC_FIND_DATA 4

// :X19170640N0501010107015555;#0  Example.....
// ^         ^                  ^
// 0         10                28
#define MAX_GRID_CONNECT_LEN 29

typedef uint8_olcb_t gridconnect_buffer_t[MAX_GRID_CONNECT_LEN];


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern uint8_olcb_t OpenLcbGridConnect_copy_out_gridconnect_when_done(uint8_olcb_t next_byte, gridconnect_buffer_t* buffer);
    
    extern void OpenLcbGridConnect_to_can_msg(gridconnect_buffer_t *gridconnect, can_msg_t *can_msg);

    extern void OpenLcbGridConnect_from_can_msg(gridconnect_buffer_t *gridconnect, can_msg_t *can_msg);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_BUFFER_STORE__ */