
/* 
 * File: can_incoming_statemachine.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_RX_STATEMACHINE__
#define	__CAN_RX_STATEMACHINE__

#include <xc.h> // include processor files - each processor file is guarded.  


extern void CanRxStatemachine_initialize();


// TODO: THIS IS TEMPORARY TO ALLOW SIMULATOR TESTING OF THE RX CHANNEL
extern void _state_machine_incoming_can(uint8_t channel, can_msg_t* msg); 


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_RX_STATEMACHINE__ */

