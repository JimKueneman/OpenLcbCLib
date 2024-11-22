/*
 * File:   openlcb_tx_driver.c
 * Author: jimkueneman
 *
 * Created on November 21, 2024, 6:21 PM
 */


#include "xc.h"
#include "stdio.h"  // printf
#include "openlcb_types.h"
#include "openlcb_node.h"

#include "../drivers/common/can_tx_statemachine.h"

#include "../applications/test/main_statemachine/main_statemachine.X/debug.h"

uint8_t OpenLcbTxDriver_try_transmit(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {
  
    can_msg_t can_msg;
    
    uint16_t payload_index = 0;
    uint16_t bytes_transmitted = 0;
    
    // TODO:  I don't like this coupling into the CAN drivers here... need to come up with a better way so this file does not need to access
    //        the can driver files... maybe a callback function that connects this library to the desired TX driver... need to think about it.
    while (payload_index < openlcb_msg->payload_count) {
        
        bytes_transmitted = CanTxStatemachine_try_transmit_openlcb_message(openlcb_msg, &can_msg, payload_index);
        
        payload_index = payload_index + bytes_transmitted;
        
    }
    return TRUE;
    
    
}
