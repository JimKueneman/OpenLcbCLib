
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

/*
 * NOTE:  All Function for all Protocols expect that the incoming CAN messages have been
 *        blocked so there is not a race on the incoming message buffer.
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __PROTOCOL_SNIP__
#define	__PROTOCOL_SNIP__

#include <xc.h> // include processor files - each processor file is guarded.
#include "openlcb_types.h"

extern void ProtocolSnip_handle_simple_node_info_request(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __PROTOCOL_SNIP__ */

