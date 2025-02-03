
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DEBUG__
#define	__DEBUG__


#ifdef PRINT_DEBUG 

#include "../../../openlcb/openlcb_defines.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../drivers/common/can_types.h"

#endif

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef PRINT_DEBUG 

    extern void PrintCAN1Registers(void);

    extern void PrintDMA0Registers(void);

    extern void PrintDMA1Registers(void);

    extern void PrintDMA2Registers(void);

    extern void PrintDMA3Registers(void);

    extern void PrintMtiName(uint16_olcb_t mti);

    extern void PrintBufferStats(void);

    extern void PrintAliasAndNodeID(uint16_olcb_t alias, uint64_olcb_t node_id);

    extern void PrintCanIdentifier(uint32_olcb_t identifier);

    extern void PrintCanFrameIdentifierName(uint32_olcb_t identifier);

    extern void PrintAlias(uint16_olcb_t alias);

    extern void PrintNodeID(uint64_olcb_t node_id);

    extern void PrintEventID(event_id_t event_id);

    extern void PrintOpenLcbMsg(openlcb_msg_t* openlcb_msg);

    extern void PrintInt64(uint64_olcb_t n);

    extern void PrintDWord(uint32_olcb_t dword);

    extern void PrintCanMsg(can_msg_t* can_msg);

    extern void PrintNode(openlcb_node_t* node);


    extern uint8_olcb_t print_msg;

#endif // PRINT_DEBUG

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DEBUG__ */

