
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __PROTOCOL_MESSAGE_NETWORK__
#define	__PROTOCOL_MESSAGE_NETWORK__

#include <xc.h> // include processor files - each processor file is guarded.  
#include "openlcb_types.h"


extern void ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);
extern void ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);
extern void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);
extern void ProtocolMessageNetwork_handle_verified_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */


#endif	/* __PROTOCOL_MESSAGE_NETWORK__ */

