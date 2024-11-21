
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_BUFFER_STORE__
#define	__OPENLCB_BUFFER_STORE__

#include <xc.h> // include processor files - each processor file is guarded. 
#include "openlcb_types.h"

extern void BufferStore_initialize();

extern openlcb_msg_t* BufferStore_allocateBuffer(uint16_t buffer_size);

extern void BufferStore_freeBuffer(openlcb_msg_t* msg);

extern void BufferStore_clear_openlcb_message(openlcb_msg_t* msg);

extern uint16_t BufferStore_messages_allocated();

extern uint16_t BufferStore_messages_max_allocated();


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_BUFFER_STORE__ */

