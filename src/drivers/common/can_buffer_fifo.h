
/* 
 * File: can_buffers.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_BUFFERS__
#define	__CAN_BUFFERS__

#include <xc.h> // include processor files - each processor file is guarded.  
#include "can_types.h"

extern void CanBufferFifo_initialiaze();

extern can_msg_t* CanBufferFifo_push(); 

extern can_msg_t* CanBufferFifo_pop();

extern uint8_t CanBufferFifo_is_empty();

extern uint16_t CanBufferFifo_get_allocated_count();


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */


#endif	/* __CAN_BUFFERS__ */

