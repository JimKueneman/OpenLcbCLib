
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_BUFFER_STORE__
#define	__CAN_BUFFER_STORE__

#include <xc.h> // include processor files - each processor file is guarded. 
#include "can_types.h"

extern void CanBufferStore_initialize();

extern can_msg_t* CanBufferStore_allocateBuffer();

extern void CanBufferStore_freeBuffer(can_msg_t* msg);

extern void CanBufferStore_clear_can_message(can_msg_t* msg);

extern uint16_t CanBufferStore_messages_allocated();

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_BUFFER_STORE__ */

