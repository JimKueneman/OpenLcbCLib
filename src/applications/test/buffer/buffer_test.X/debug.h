
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __Debug__
#define	__Debug__

#include <xc.h> // include processor files - each processor file is guarded.  

extern void PrintCAN1Registers(void);

extern void PrintDMA0Registers(void);

extern void PrintDMA1Registers(void);

extern void PrintDMA2Registers(void);

extern void PrintDMA3Registers(void);

extern void PrintMtiName(uint16_t mti);

extern void PrintBufferStats(void);

//extern void ForceFlushAndFreeFIFO(openlcb_msg_buffer_t* fifo);
//extern void PrintContentsFIFO(openlcb_msg_buffer_t* fifo);

extern void PrintAliasAndNodeID(uint16_t alias, uint64_t node_id);

extern void PrintCanIdentifier(uint32_t identifier);

extern void PrintAlias(uint16_t alias);

extern void PrintNodeID(uint64_t node_id);

extern void PrintNodeEventID(event_id_t event_id) ;

extern void PrintOpenLcbMsg(openlcb_msg_t* openlcb_msg);

extern void PrintBufferTypeNameMsg(openlcb_msg_t* openlcb_msg);

extern void PrintBufferTypeName(uint16_t type_id);

extern void PrintInt64(uint64_t n);
    

extern uint8_t print_msg;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __Debug__ */

