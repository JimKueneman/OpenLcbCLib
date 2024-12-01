/*
 * File:   _100ms_timer_statemachine.c
 * Author: jimkueneman
 *
 * Created on November 16, 2024, 5:07 AM
 */


#include "xc.h"
#include "openlcb_node.h"
#include "protocol_datagram.h"
#include "../drivers/driver_100ms_clock.h"


void _100msTimeTickNode() {
   
    Node_100ms_timer_tick();
    DatagramProtocol_100ms_time_tick();

}

void ClockDistribution_initialize() {
    
    Driver100msClock_Initialization(&_100msTimeTickNode);
    
    
}

