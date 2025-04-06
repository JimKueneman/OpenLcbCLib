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
 * \file i2c1.c
 *
 * @author Jim Kueneman
 * @date 5 April 2025
 */

#ifndef _I2C1_H
#define _I2C1_H

#include "../../../openlcb/openlcb_types.h"

#define MAX_LEN_I2C_DATA 64 // allow 16 LEDs * 4 bytes per

#define STATE_I2C_IDLE 0
#define STATE_I2C_START_BIT 1
#define STATE_I2C_SEND_ADDRESS 2
#define STATE_I2C_READ_ADDRESS_ACK 3
#define STATE_I2C_SEND_REGISTER 4
#define STATE_I2C_READ_ACK 5
#define STATE_I2C_SEND_BYTE 6
#define STATE_I2C_READ_BYTE 7
#define STATE_I2C_SEND_STOP 8


typedef struct {
    
    uint8_olcb_t write;  // true if this is a write message
    uint8_olcb_t length; // number of bytes to send not including the address
    uint8_olcb_t address;  // 7 bit address to send to
    uint8_olcb_t reg; // register to access
    uint8_olcb_t data[MAX_LEN_I2C_DATA];  // databytes to send
    uint8_olcb_t state; // what state are we in for sending the message STATE_I2C_XXXX 
    uint8_olcb_t sent;  // internal counter for how many have been sent from the data array
    
} i2c_message_t;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
    extern void I2C1_initialize_message(i2c_message_t* msg);

    extern void I2C1_load_message(i2c_message_t* msg);
    
    extern void I2C1_start_message_transmission(void);
    
    extern void I2C1_run(void);
    
    extern void I2C1_start_transmission_and_run_to_completion(i2c_message_t* msg);
    
    extern uint8_olcb_t I2C1_is_message_complete(i2c_message_t* msg);
    
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* _I2C1_H */
