
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_FIFO__
#define	__OPENLCB_FIFO__

#include <xc.h> // include processor files - each processor file is guarded.  
#include "openlcb_types.h"

extern void BufferFifo_initialiaze();

extern openlcb_msg_t* BufferFifo_push(uint16_t data_len); 

extern openlcb_msg_t* BufferFifo_push_existing(openlcb_msg_t* existing_msg);

extern openlcb_msg_t* BufferFifo_pop();

extern uint8_t BufferFifo_is_empty();

extern uint16_t BufferFifo_get_free_count();


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_FIFO__ */

