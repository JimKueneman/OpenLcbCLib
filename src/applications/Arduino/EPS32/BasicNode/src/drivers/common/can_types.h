
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
 * \file can_types.h
 *
 * Typedef and constants that define CAN specific datatypes.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_TYPES__
#define	__CAN_TYPES__
  
#include "../../openlcb/openlcb_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


    // ************************ USER DEFINED VARIABLES *****************************

#ifndef USER_DEFINED_CAN_MSG_BUFFER_DEPTH
#define USER_DEFINED_CAN_MSG_BUFFER_DEPTH                    10   // USER DEFINED 
#endif

    // *********************END USER DEFINED VARIABLES *****************************

#define TX_CHANNEL_CAN_CONTROL 0
#define TX_CHANNEL_OPENLCB_MSG 0

#define LEN_CAN_BYTE_ARRAY   8

#define OFFSET_CAN_WITHOUT_DEST_ADDRESS   0
#define OFFSET_CAN_WITH_DEST_ADDRESS      2


// Structure for a basic CAN payload
typedef uint8_olcb_t payload_bytes_can_t[LEN_CAN_BYTE_ARRAY];

typedef struct {
    uint8_olcb_t allocated: 1;
    uint8_olcb_t direct_tx: 1;    // If set the CAN statemachine will simply directly send it assuming all the source/dest/mti/data is all set up.  Mainly for sending error found during Can frame reception to allow for the rx thread/interrupt to not have to reach across boundries to send it.
} can_msg_state_t;

typedef struct {
    can_msg_state_t state;
    uint32_olcb_t identifier;              // CAN 29 bit identifier (extended)
    uint8_olcb_t payload_count;            // How many bytes are valid
    payload_bytes_can_t payload;      // Payload bytes
} can_msg_t;


typedef can_msg_t can_buffer_store_t[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];

typedef struct {
    openlcb_statemachine_worker_t* openlcb_worker;
    can_msg_t can_worker;
    can_msg_t* active_msg;
} can_main_statemachine_t;

// Assign the function pointer to where the incoming CAN messages should be dispatched to.
// WARNING: Is in the context of the interrupt, be careful
// void func(uint8_olcb_t channel, can_msg_t* can_msg)
typedef void (*can_rx_callback_func_t) (uint8_olcb_t, can_msg_t*);

typedef uint8_olcb_t (*transmit_raw_can_frame_func_t) (uint8_olcb_t, can_msg_t*);

typedef uint8_olcb_t (*is_can_tx_buffer_clear_func_t) (uint16_olcb_t); 

typedef void(*can_rx_driver_callback_t) (can_rx_callback_func_t);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_TYPES__ */

