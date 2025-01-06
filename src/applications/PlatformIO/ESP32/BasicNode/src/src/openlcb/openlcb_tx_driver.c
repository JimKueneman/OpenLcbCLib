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
 * \file openlcb_tx_driver.c
 *
 * Implementation of the Openlcb message transmit engine.  It calls the CAN Tx Driver
 * to send the message and blocks until the entire message is sent (on CAN that could 
 * be many CAN frames.  If the transmitter is busy it skips by and lets the main loop 
 * run until it finds an open transmit channel. 
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "openlcb_tx_driver.h"

#include "stdio.h"  // printf
#include "openlcb_types.h"
#include "openlcb_node.h"

#include "../drivers/common/can_tx_statemachine.h"

uint8_olcb_t OpenLcbTxDriver_try_transmit(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {
  
    can_msg_t can_msg;
    
    uint16_olcb_t payload_index = 0;
    uint16_olcb_t bytes_transmitted = 0;
    
    // TODO:  I don't like this coupling into the CAN drivers here... need to come up with a better way so this file does not need to access
    //        the can driver files... maybe a callback function that connects this library to the desired TX driver... need to think about it.
    while (payload_index < openlcb_msg->payload_count) {
        
        bytes_transmitted = CanTxStatemachine_try_transmit_openlcb_message(&can_msg, openlcb_msg, payload_index);
        
        payload_index = payload_index + bytes_transmitted;
        
    }
    
    return TRUE;
    
    
}
