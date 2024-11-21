
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_MSG_HANDLER__
#define	__CAN_MSG_HANDLER__

#include <xc.h> // include processor files - each processor file is guarded.
#include "can_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_node.h"


extern uint8_t CanMessageHandler_process(openlcb_node_t* can_node, can_msg_t* msg);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_MSG_HANDLER__ */

