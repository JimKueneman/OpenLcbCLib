
/* 
 * File: can_outgoing_statemachine.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_TX_STATEMACHINE__
#define	__CAN_TX_STATEMACHINE__

#include <xc.h> // include processor files - each processor file is guarded.  


extern void CanTxStatemachine_initialize();

// returns the number of bytes that were written to the CAN bus for this iteration (openlcb_start_index + 8 max)
extern uint16_t CanTxStatemachine_try_transmit_openlcb_message(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t openlcb_start_index);

// returns the number of bytes that were written to the CAN bus
extern uint8_t CanTxStatemachine_try_transmit_can_message(can_msg_t* can_msg);
 

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */


#endif	/* __CAN_TX_STATEMACHINE__ */

