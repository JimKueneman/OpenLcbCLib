/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file nmra_dcc_types.h
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to write/read configuration memory.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 14 Mar 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __NMRA_DCC_TYPES__
#define	__NMRA_DCC_TYPES__

#include "../../openlcb/openlcb_types.h"

#define NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS 58
#define NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS 116

#define STATE_NMRA_DCC_PREAMBLE 0
#define STATE_NMRA_DCC_START_BIT 1
#define STATE_NMRA_DCC_DATA 2
#define STATE_NMRA_DCC_XOR_BYTE 3
#define STATE_NMRA_DCC_END_BIT 4


#define DCC_MESSAGE_LENGTH 8

#define LEN_NMRA_DCC_MESSAGE_BUFFER 16

#define LEN_NMRA_DCC_BUFFER (LEN_NMRA_DCC_MESSAGE_BUFFER + LEN_NMRA_DCC_MESSAGE_BUFFER + LEN_NMRA_DCC_MESSAGE_BUFFER + 3)

typedef void(*nmra_dcc_timing_callback_t) (uint16_olcb_t timing_in_us);

typedef struct {
    uint8_olcb_t allocated : 1;
    uint8_olcb_t state;
    
    nmra_dcc_timing_callback_t next_callback;
    uint16_olcb_t next_callback_parameter;

    uint16_olcb_t preamble_size : 4;
    uint16_olcb_t preamble_position : 4;
    uint16_olcb_t buffer_byte_index : 4;
    uint16_olcb_t buffer_byte_bit_index : 4;

    uint8_olcb_t buffer[DCC_MESSAGE_LENGTH];
    uint8_olcb_t buffer_size;

    uint8_olcb_t xor_byte;

    uint8_olcb_t reference_count;

} nmra_dcc_message_t;

typedef struct {
    nmra_dcc_message_t* buffer[LEN_NMRA_DCC_MESSAGE_BUFFER];
    uint8_olcb_t head;
    uint8_olcb_t tail;

} nmra_dcc_message_buffer_t;

typedef struct {
    nmra_dcc_message_buffer_t high;
    nmra_dcc_message_buffer_t medium;
    nmra_dcc_message_buffer_t low;

} nmra_dcc_priority_buffers_t;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */



#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __NMRA_DCC_TYPES__ */

