
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __PROTOCOL_DATAGRAM__
#define	__PROTOCOL_DATAGRAM__

#include <xc.h> // include processor files - each processor file is guarded. 
#include "openlcb_types.h"

extern void ProtocolDatagram_initialize();

extern void ProtocolDatagram_handle_datagram(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);
   
extern void Protocol_Datagram_handle_datagram_ok_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

extern void ProtocolDatagram_handle_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    
extern void DatagramProtocol_100ms_time_tick();

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __PROTOCOL_DATAGRAM__ */

