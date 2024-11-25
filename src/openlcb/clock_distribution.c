/*
 * File:   _100ms_timer_statemachine.c
 * Author: jimkueneman
 *
 * Created on November 16, 2024, 5:07 AM
 */


#include "xc.h"
#include "openlcb_node.h"
#include "../drivers/mcu_driver.h"
#include "protocol_datagram.h"


void _100msTimeTickNode() {
   
    Node_100ms_timer_tick();
    DatagramProtocol_100ms_time_tick();

}

void ClockDistribution_initialize() {
    
    // Hook into the mcu driver to get the clock
     McuDriver_100ms_timer_callback_func = &_100msTimeTickNode;
    
}

