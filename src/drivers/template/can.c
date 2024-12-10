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
 * \file can.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the 
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


// Add any compiler specific includes

#include "../common/can_types.h"


// How full the chips CAN fifo has gotten if supported
uint8_t DriverCan_max_can_fifo_depth = 0;


uint8_t DriverCan_is_can_tx_buffer_clear(uint16_t Channel) {
    
    // Implement a way to return if the can transmitter is clear to take another frame to send
    
    return 0;

}


uint8_t DriverCan_transmit_raw_can_frame(uint8_t channel, can_msg_t* msg) {

    // Implement a way to send a can frame with the passed message structure
    
    return 0;
    
 }


// This must be here and used to callback when a can frame is received
can_rx_callback_func_t internal_can_rx_callback_func;

// Implement a way to receive can frame and call the callback when a CAN frame arrives

//  Example:

//if (internal_can_rx_callback_func)
//    internal_can_rx_callback_func(buffer_tail, &ecan_msg);
//
//    
//}

// DMA2 Interrupt = CAN Receive



void DriverCan_pause_can_rx() {
    
// Implement a way to stop calling the rx callback function here
    
    
// Example:
    
//  C1INTEbits.RBIE = 0; // Enable CAN1 RX 

};

void DriverCan_resume_can_rx() {
    
// Implement a way to restart calling the rx callback function here
            
// Example:
    
// C1INTEbits.RBIE = 1; // Enable CAN1 RX

};

void DriverCan_Initialization(can_rx_callback_func_t can_rx_callback_func) {
    
    // This must be here
    internal_can_rx_callback_func = can_rx_callback_func;
    
    // Do any other initialization here

   
}