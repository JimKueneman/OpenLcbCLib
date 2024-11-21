

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
#ifndef __PROTOCOL_EVENT_TRANSPORT__
#define	__PROTOCOL_EVENT_TRANSPORT__

#include <xc.h> // include processor files - each processor file is guarded. 
#include "openlcb_types.h"

extern void ProtocolEventTransport_handle_consumer_identify(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_consumer_identify_range(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_consumer_identified_set(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_consumer_identified_clear(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_producer_identify(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_producer_identify_range(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_producer_identified_unknown(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_producer_identified_set(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_producer_identified_clear(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_producer_identified_reserved(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_identify_dest(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_identify(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_event_learn(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_pc_event_report(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern uint16_t ProtocolEventTransport_extract_consumer_event_state_mti(openlcb_node_t* openlcb_node, uint8_t event_index);

extern uint16_t ProtocolEventTransport_extract_producer_event_state_mti(openlcb_node_t* openlcb_node, uint8_t event_index);


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __PROTOCOL_EVENT_TRANSPORT__ */

