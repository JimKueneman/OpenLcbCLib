#ifndef __DELAY_TOGGLE__
#define __DELAY_TOGGLE__

#include "ti_msp_dl_config.h"

#include "./src/drivers/common/can_types.h"
#include "./src/openlcb/openlcb_defines.h"
#include "./src/openlcb/openlcb_types.h"

extern void delay_pin_toggle(void);

extern void print_interrupt(DL_MCAN_IIDX pending_interrupts);

extern void print_interrupt_flags(uint32_t interrupt_flags);

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

extern void PrintFIFOStatus(DL_MCAN_RxFIFOStatus *fifo_Status);
    

extern uint8_t print_msg;






#endif  //__DELAY_TOGGLE__
