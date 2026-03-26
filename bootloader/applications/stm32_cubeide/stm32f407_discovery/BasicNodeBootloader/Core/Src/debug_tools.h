#ifndef __DELAY_TOGGLE__
#define __DELAY_TOGGLE__

#include <stdint.h>

/* openlcb_c_lib includes removed — not present in the bootloader project */

extern void delay_pin_toggle(void);

extern void PrintInt64(uint64_t n);
extern void PrintAlias(uint16_t alias);
extern void PrintNodeID(uint64_t node_id);
extern void PrintAliasAndNodeID(uint16_t alias, uint64_t node_id);
extern void PrintCanIdentifier(uint32_t identifier);
extern void PrintDWord(uint32_t dword);

#endif //__DELAY_TOGGLE__
