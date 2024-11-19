
/* 
 * File: can_incoming_statemachine.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __ENGINE_CAN_MESSAGE__
#define	__ENGINE_CAN_MESSAGE__

#include <xc.h> // include processor files - each processor file is guarded.  


extern void CanRxStatemachine_initialize();

void _StatemachineIncomingCan(uint8_t channel, can_msg_t* msg); 


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __ENGINE_CAN_MESSAGE__ */

