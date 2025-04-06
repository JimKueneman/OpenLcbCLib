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
 * \file i2c1.h
 *
 * @author Jim Kueneman
 * @date 5 April 2025
 */

#include "i2c1.h"

#include "xc.h"
#include "stdio.h"  // printf


#include "../../../openlcb/openlcb_types.h"


i2c_message_t* target_msg = (void*) 0;
uint8_olcb_t _verbose = FALSE;

uint8_olcb_t _send_start_bit(uint8_olcb_t waitfor) {


    if (I2C1STATbits.P) { // Sent a Start/Stop sequence on initialization to make sure this bit is set

        I2C1CONbits.SEN = 1; // Initiate a start   

        if (waitfor) {

            while (I2C1CONbits.SEN) {
            }

        }

        return TRUE;

    }

    return FALSE;

}

uint8_olcb_t _send_stop_bit(uint8_olcb_t waitfor) {


    if (I2C1STATbits.S) { // if we have already sent a start then we can send a stop

        I2C1CONbits.PEN = 1; // Initiate a stop

        if (waitfor) {

            while (I2C1CONbits.PEN) {
            }

        }

        return TRUE;

    }

    return FALSE;

}

uint8_olcb_t _ready_for_next_state(void) {

    // Start Enable/Repeated Start Enable/Stop Enable/Receive Enable/Ack Enable must all be 0 before moving on

    return ((I2C1CON & 0b11111) == 0);

}

uint8_olcb_t _load_address_to_read(uint8_olcb_t address) {

    address = address << 1;
    address = address | 0b00000001; // make sure the low bit is set  

    return (address);

}

uint8_olcb_t _load_address_to_write(uint8_olcb_t address) {

    address = address << 1;

    return (address);

}

uint8_olcb_t _transmit_byte(uint8_olcb_t waitfor, uint8_olcb_t data) {

    if (!I2C1STATbits.TRSTAT) {
        
        I2C1TRN = data; // Register

        if (waitfor) {

            while (I2C1STATbits.TRSTAT) {
            } // Transmit + ACK time

            return TRUE;

        }

    }

    return FALSE;
}

void I2C1_load_message(i2c_message_t * msg) {

    target_msg = msg;

}

void I2C1_start_message_transmission(void) {

    if (!target_msg) {

        return;

    }

    target_msg->state = STATE_I2C_START_BIT;

}

void I2C1_run(void) {

    if (!target_msg) {

        return;

    }

    switch (target_msg->state) {

        case STATE_I2C_IDLE:
        {

            break;
        }

        case STATE_I2C_START_BIT:
        {

            if (_verbose)
                printf("Start\n");

            target_msg->sent = 0; // reset

            if (_send_start_bit(TRUE)) {

                target_msg->state = STATE_I2C_SEND_ADDRESS;

            }

            break;

        }

        case STATE_I2C_SEND_ADDRESS:
        {

            if (_verbose)
                printf("Send Address\n");

            if (target_msg->write) {

                _transmit_byte(TRUE, _load_address_to_write(target_msg->address));

            } else {

                _transmit_byte(TRUE, _load_address_to_read(target_msg->address));

            }

            target_msg->state = STATE_I2C_READ_ADDRESS_ACK;

            break;

        }
        
        case STATE_I2C_READ_ADDRESS_ACK: {
            
            if (_verbose)
                printf("Address Ack\n");

            // Could do something here if it was a NACK but we just keep updating so it doesn't really matter
            target_msg->state = STATE_I2C_SEND_REGISTER;
            
            break;
        }
        
        case STATE_I2C_SEND_REGISTER: {
            
            if (_transmit_byte(TRUE, target_msg->reg)) {
                
                 target_msg->state = STATE_I2C_READ_ACK;
                 
            }
            
            break;
        }

        case STATE_I2C_READ_ACK:
        {

            if (_verbose)
                printf("Ack\n");

            // Could do something here if it was a NACK but we just keep updating so it doesn't really matter
            target_msg->state = STATE_I2C_SEND_BYTE;

            break;

        }

        case STATE_I2C_SEND_BYTE:
        {

            if (_verbose) {
                printf("Send (sent): %d\n", target_msg->sent);
                printf("Send (length): %d\n", target_msg->length);
            }

            if (target_msg->sent == target_msg->length) {

                target_msg->state = STATE_I2C_SEND_STOP;

                break;

            }

            while (target_msg->sent < target_msg->length) {

                if (_transmit_byte(TRUE, target_msg->data[target_msg->sent])) {
                    
                  target_msg->sent = target_msg->sent + 1;
                  
                }

            }

            target_msg->state = STATE_I2C_READ_ACK;

            break;
        }

        case STATE_I2C_READ_BYTE:
        {

            // TODO:  this will likely need a new field in the message structure to tell the statemachine where to actually do a read in the sequence

            break;
        }

        case STATE_I2C_SEND_STOP:
        {

            if (_verbose)
                printf("Stop\n");

            if (_send_stop_bit(TRUE)) {

                target_msg->state = STATE_I2C_IDLE;

                target_msg = (void*) 0;

            }

            break;

        }

        default:
        {

            target_msg->state = STATE_I2C_IDLE;
            break;

        }

    }

}

uint8_olcb_t I2C1_is_message_complete(i2c_message_t * msg) {

    if (target_msg) {

        return (target_msg->state == STATE_I2C_IDLE);

    }

    return (TRUE);

}

void I2C1_initialize_message(i2c_message_t* msg) {

    if (msg) {

        msg->address = 0;
        for (int i = 0; i < MAX_LEN_I2C_DATA; i++) {

            msg->data[i] = 0;

        }

        msg->length = 0;
        msg->sent = 0;
        msg->state = 0;
        msg->write = FALSE;

    }
}

void I2C1_start_transmission_and_run_to_completion(i2c_message_t* msg) {
    
    if (!msg) {
        
        return;
        
    }
    
    I2C1_load_message(msg);
    I2C1_start_message_transmission();
    
    while (target_msg) {
        
        I2C1_run();
        
    }
}