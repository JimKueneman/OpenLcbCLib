
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_LOGIN_MESSAGE_HANDLER__
#define	__CAN_LOGIN_MESSAGE_HANDLER__

#include <xc.h> // include processor files - each processor file is guarded.  
#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

extern void CanLoginMessageHandler_init(openlcb_node_t* next_node);

extern void CanFrameMessageHandler_generate_seed(openlcb_node_t* next_node);

extern void CanFrameMessageHandler_generate_alias(openlcb_node_t* next_node);

extern void CanFrameMessageHandler_transmit_cid07(openlcb_node_t* next_node, can_msg_t* worker_msg);

extern void CanFrameMessageHandler_transmit_cid06(openlcb_node_t* next_node, can_msg_t* worker_msg);

extern void CanFrameMessageHandler_transmit_cid05(openlcb_node_t* next_node, can_msg_t* worker_msg);

extern void CanFrameMessageHandler_transmit_cid04(openlcb_node_t* next_node, can_msg_t* worker_msg);

extern void CanFrameMessageHandler_wait_200ms(openlcb_node_t* next_node);

extern void CanFrameMessageHandler_transmit_rid(openlcb_node_t* next_node, can_msg_t* worker_msg);

extern void CanFrameMessageHandler_transmit_amd(openlcb_node_t* next_node, can_msg_t* worker_msg);

extern void CanFrameMessageHandler_transmit_initialization_complete(openlcb_node_t* next_node, can_msg_t* can_worker, openlcb_msg_t* openlcb_worker);

extern void CanFrameMessageHandler_transmit_producer_events(openlcb_node_t* next_node, can_msg_t* can_worker, openlcb_msg_t* openlcb_worker);

extern void CanFrameMessageHandler_transmit_consumer_events(openlcb_node_t* next_node, can_msg_t* can_worker, openlcb_msg_t* openlcb_worker);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_LOGIN_MESSAGE_HANDLER__ */

