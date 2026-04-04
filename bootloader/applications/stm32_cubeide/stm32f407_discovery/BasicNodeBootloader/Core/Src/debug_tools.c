
#include "debug_tools.h"

#include "stdio.h"

/* openlcb_c_lib includes removed — not present in the bootloader project.
   Functions that depend on OpenLCB types (PrintMtiName, PrintOpenLcbMsg,
   PrintCanMsg, PrintNode, PrintEventID, PrintCanFrameIdentifierName)
   are commented out below until the bootloader is fully ported. */

void delay_pin_toggle(void)
{

    asm(" NOP");
    asm(" NOP");
    asm(" NOP");
    asm(" NOP");
    asm(" NOP");
    asm(" NOP");
    asm(" NOP");
    asm(" NOP");
    asm(" NOP");
    asm(" NOP");

}

void PrintInt64(uint64_t n)
{

    printf("0x%04X", (uint16_t)((n >> 48) & 0xFFFF));
    printf("%04X", (uint16_t)((n >> 32) & 0xFFFF));
    printf("%04X", (uint16_t)((n >> 16) & 0xFFFF));
    printf("%04X\n", (uint16_t)((n >> 0) & 0xFFFF));
}

void PrintAlias(uint16_t alias)
{

    printf("Alias: %04X\n", alias);
}

void PrintNodeID(uint64_t node_id)
{

    printf("NodeID: 0x%04X", (uint16_t)(node_id >> 32));
    printf("%04X", (uint16_t)(node_id >> 16));
    printf("%04X\n", (uint16_t)(node_id >> 0));
}

void PrintAliasAndNodeID(uint16_t alias, uint64_t node_id)
{

    PrintAlias(alias);
    PrintNodeID(node_id);
}

void PrintCanIdentifier(uint32_t identifier)
{

    printf("0x%04X", (uint16_t)(identifier >> 16));
    printf("%04X\n", (uint16_t)(identifier));
}

void PrintDWord(uint32_t dword)
{

    PrintCanIdentifier(dword);
}
