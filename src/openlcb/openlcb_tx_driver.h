
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_TX_DRIVER__
#define	__OPENLCB_TX_DRIVER__

#include "openlcb_types.h"
#include "openlcb_node.h"

extern uint8_t OpenLcbTxDriver_try_transmit(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

#include <xc.h> // include processor files - each processor file is guarded.  

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_TX_DRIVER__ */

