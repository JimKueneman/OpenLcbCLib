
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_LIST__
#define	__OPENLCB_LIST__

#include <xc.h> // include processor files - each processor file is guarded.  
#include "openlcb_types.h"

extern void BufferList_initialiaze();

extern openlcb_msg_t* BufferList_allocate(uint16_t data_len);  

extern openlcb_msg_t* BufferList_find(uint16_t source_alias, uint16_t dest_alias, uint16_t mti); 

extern void BufferList_free(openlcb_msg_t* msg);

extern void BufferList_release(openlcb_msg_t* msg); 

extern openlcb_msg_t* BufferList_index_of(uint16_t index);

extern uint8_t BufferList_is_empty();



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_LIST__ */

