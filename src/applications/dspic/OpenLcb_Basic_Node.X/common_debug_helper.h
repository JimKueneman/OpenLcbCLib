
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __COMMON_DEBUG_HELPER__
#define	__COMMON_DEBUG_HELPER__

#include "../../../openlcb/openlcb_defines.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../drivers/common/can_types.h"

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

extern void PrintCanFrameIdentifierName(uint32_t identifier);

extern void PrintAlias(uint16_t alias);

extern void PrintNodeID(uint64_t node_id);

extern void PrintEventID(event_id_t event_id) ;

extern void PrintOpenLcbMsg(openlcb_msg_t* openlcb_msg);

extern void PrintInt64(uint64_t n);

extern void PrintDWord(uint32_t dword);

extern void PrintCanMsg(can_msg_t* can_msg);

extern void PrintNode(openlcb_node_t* node);
    

extern uint8_t print_msg;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __COMMON_DEBUG_HELPER__ */

