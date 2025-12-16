
#include "debug_tools.h"

#include "ti_msp_dl_config.h"

#include "stdio.h"

#include "./src/drivers/canbus/can_types.h"
#include "./src/openlcb/openlcb_defines.h"
#include "./src/openlcb/openlcb_types.h"

void print_interrupt_flags(uint32_t interrupt_flags)
{

    if ((interrupt_flags & DL_MCAN_INTERRUPT_ARA) == DL_MCAN_INTERRUPT_ARA)
    {

        printf("\tDL_MCAN_INTERRUPT_ARA (Access to Reserved Address interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_ARA);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_PED) == DL_MCAN_INTERRUPT_PED)
    {

        printf("\tDL_MCAN_INTERRUPT_PED (Protocol Error in Data Phase interrupt_): 0x%08X\n", DL_MCAN_INTERRUPT_PED);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_PEA) == DL_MCAN_INTERRUPT_PEA)
    {

        printf("\tDL_MCAN_INTERRUPT_PEA (Protocol Error in Arbitration Phase interrupt_): 0x%08X\n", DL_MCAN_INTERRUPT_PEA);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_WDI) == DL_MCAN_INTERRUPT_WDI)
    {

        printf("\tDL_MCAN_INTERRUPT_WDI (Watchdog interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_WDI);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_BO) == DL_MCAN_INTERRUPT_BO)
    {

        printf("\tDL_MCAN_INTERRUPT_BO (Bus_Off Status interrupt): 0x%08X", DL_MCAN_INTERRUPT_BO);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_EW) == DL_MCAN_INTERRUPT_EW)
    {

        printf("\tDL_MCAN_INTERRUPT_EW (Warning Status interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_EW);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_EP) == DL_MCAN_INTERRUPT_EP)
    {

        printf("\tDL_MCAN_INTERRUPT_EP (Error Passive interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_EP);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_ELO) == DL_MCAN_INTERRUPT_ELO)
    {

        printf("\tDL_MCAN_INTERRUPT_ELO (Error Logging Overflow interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_ELO);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_BEU) == DL_MCAN_INTERRUPT_BEU)
    {

        printf("\tDL_MCAN_INTERRUPT_BEU (Bit Error Uncorrected. Message RAM bit error detected, uncorrected interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_BEU);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_DRX) == DL_MCAN_INTERRUPT_DRX)
    {

        printf("\tDL_MCAN_INTERRUPT_DRX (Message Stored to Dedicated Rx Buffer interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_DRX);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TOO) == DL_MCAN_INTERRUPT_TOO)
    {

        printf("\tDL_MCAN_INTERRUPT_TOO (Timeout Occurred interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_TOO);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_MRAF) == DL_MCAN_INTERRUPT_MRAF)
    {

        printf("\tDL_MCAN_INTERRUPT_MRAF (Message RAM Access Failure interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_MRAF);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TSW) == DL_MCAN_INTERRUPT_TSW)
    {

        printf("\tDL_MCAN_INTERRUPT_TSW(Timestamp Wraparound interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_TSW);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TEFL) == DL_MCAN_INTERRUPT_TEFL)
    {

        printf("\tDL_MCAN_INTERRUPT_TEFL (Tx Event FIFO Element Lost interruptA): 0x%08X\n", DL_MCAN_INTERRUPT_TEFL);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TEFF) == DL_MCAN_INTERRUPT_TEFF)
    {

        printf("\tDL_MCAN_INTERRUPT_TEFF (Tx Event FIFO Full interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_TEFF);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TEFW) == DL_MCAN_INTERRUPT_TEFW)
    {

        printf("\tDL_MCAN_INTERRUPT_TEFW (Tx Event FIFO Watermark Reached interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_TEFW);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TEFN) == DL_MCAN_INTERRUPT_TEFN)
    {

        printf("\tDL_MCAN_INTERRUPT_TEFN (Tx Event FIFO New Entry interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_TEFN);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TFE) == DL_MCAN_INTERRUPT_TFE)
    {

        printf("\tDL_MCAN_INTERRUPT_TFE (Tx FIFO Empty interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_TFE);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TCF) == DL_MCAN_INTERRUPT_TCF)
    {

        printf("\tDL_MCAN_INTERRUPT_TCF (Transmission Cancellation Finished interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_TCF);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_TC) == DL_MCAN_INTERRUPT_TC)
    {

        printf("\tDL_MCAN_INTERRUPT_TC (Transmission Completed interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_TC);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_HPM) == DL_MCAN_INTERRUPT_HPM)
    {

        printf("\t (High Priority Message interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_HPM);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_RF1L) == DL_MCAN_INTERRUPT_RF1L)
    {

        printf("\tDL_MCAN_INTERRUPT_RF1L (Rx FIFO 1 Message Lost interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_RF1L);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_RF1F) == DL_MCAN_INTERRUPT_RF1F)
    {

        printf("\t (Rx FIFO 1 Full interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_RF1F);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_RF1W) == DL_MCAN_INTERRUPT_RF1W)
    {

        printf("\t(Rx FIFO 1 Watermark Reached interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_RF1W);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_RF1N) == DL_MCAN_INTERRUPT_RF1N)
    {

        printf("\tDL_MCAN_INTERRUPT_RF1N (Rx FIFO 1 New Message interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_RF1N);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_RF0L) == DL_MCAN_INTERRUPT_RF0L)
    {

        printf("\tDL_MCAN_INTERRUPT_RF0L (Rx FIFO 0 Message Lost interrupt): 0x%08X", DL_MCAN_INTERRUPT_RF0L);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_RF0F) == DL_MCAN_INTERRUPT_RF0F)
    {

        printf("\tDL_MCAN_INTERRUPT_RF0F (Rx FIFO 0 Full interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_RF0F);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_RF0W) == DL_MCAN_INTERRUPT_RF0W)
    {

        printf("\tDL_MCAN_INTERRUPT_RF0W (Rx FIFO 0 Watermark interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_RF0W);
    }

    if ((interrupt_flags & DL_MCAN_INTERRUPT_RF0N) == DL_MCAN_INTERRUPT_RF0N)
    {

        printf("\tDL_MCAN_INTERRUPT_RF0N (Rx FIFO 0 New Message interrupt): 0x%08X\n", DL_MCAN_INTERRUPT_RF0N);
    }

    // MSP flags are different as the values overlap with the previous non MSP defines

    //  if ((interrupt_flags & DL_MCAN_MSP_INTERRUPT_WAKEUP) == DL_MCAN_MSP_INTERRUPT_WAKEUP) {

    //     printf("\tDL_MCAN_MSP_INTERRUPT_WAKEUP (Clock Stop Wake Up interrupt): 0x%08X\n", DL_MCAN_MSP_INTERRUPT_WAKEUP);
    //  }

    //  if ((interrupt_flags & DL_MCAN_MSP_INTERRUPT_TIMESTAMP_OVERFLOW) == DL_MCAN_MSP_INTERRUPT_TIMESTAMP_OVERFLOW) {

    //     printf("\tDL_MCAN_MSP_INTERRUPT_TIMESTAMP_OVERFLOW (External Timestamp Counter Overflow interrupt): 0x%08X\n", DL_MCAN_MSP_INTERRUPT_TIMESTAMP_OVERFLOW);
    //  }

    //  if ((interrupt_flags & DL_MCAN_MSP_INTERRUPT_DOUBLE_ERROR_DETECTION) == DL_MCAN_MSP_INTERRUPT_DOUBLE_ERROR_DETECTION) {

    //     printf("\tDL_MCAN_MSP_INTERRUPT_DOUBLE_ERROR_DETECTION (Message RAM Double Error Detection interrupt): 0x%08X\n", DL_MCAN_MSP_INTERRUPT_DOUBLE_ERROR_DETECTION);
    //  }

    //  if ((interrupt_flags & DL_MCAN_MSP_INTERRUPT_SINGLE_ERROR_CORRECTION) == DL_MCAN_MSP_INTERRUPT_SINGLE_ERROR_CORRECTION) {

    //     printf("\tDL_MCAN_MSP_INTERRUPT_SINGLE_ERROR_CORRECTION (Message RAM Single Error Correction interrupt): 0x%08X\n", DL_MCAN_MSP_INTERRUPT_SINGLE_ERROR_CORRECTION);
    //  }

    //  if ((interrupt_flags & DL_MCAN_MSP_INTERRUPT_LINE1) == DL_MCAN_MSP_INTERRUPT_LINE1) {

    //     printf("\tDL_MCAN_MSP_INTERRUPT_LINE1 (MCAN Interrupt Line 1): 0x%08X\n", DL_MCAN_MSP_INTERRUPT_LINE1);
    //  }

    //  if ((interrupt_flags & DL_MCAN_MSP_INTERRUPT_LINE0) == DL_MCAN_MSP_INTERRUPT_LINE0) {

    //     printf("\tDL_MCAN_MSP_INTERRUPT_LINE0 (MCAN Interrupt Line 0): 0x%08X\n", DL_MCAN_MSP_INTERRUPT_LINE0);
    //  }
}

void print_interrupt(DL_MCAN_IIDX pending_interrupts)
{

    switch (pending_interrupts)
    {

    case DL_MCAN_IIDX_WAKEUP:
        printf("DL_MCAN_IIDX_WAKEUP: ");
        break;
    case DL_MCAN_IIDX_TIMESTAMP_OVERFLOW:
        printf("DL_MCAN_IIDX_TIMESTAMP_OVERFLOW: ");
        break;
    case DL_MCAN_IIDX_DOUBLE_ERROR_DETECTION:
        printf("DL_MCAN_IIDX_DOUBLE_ERROR_DETECTION: ");
        break;
    case DL_MCAN_IIDX_SINGLE_ERROR_CORRECTION:
        printf("DL_MCAN_IIDX_SINGLE_ERROR_CORRECTION: ");
        break;
    case DL_MCAN_IIDX_LINE1:
        printf("DL_MCAN_IIDX_LINE1: ");
        break;
    case DL_MCAN_IIDX_LINE0:
        printf("DL_MCAN_IIDX_LINE0: ");
        break;
    }
}

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

void PrintMtiName(uint16_t mti)
{
    switch (mti)
    {
    case MTI_INITIALIZATION_COMPLETE:
        printf("MTI_INITIALIZATION_COMPLETE");
        break;
    case MTI_INITIALIZATION_COMPLETE_SIMPLE:
        printf("MTI_INITIALIZATION_COMPLETE_SIMPLE");
        break;
    case MTI_VERIFY_NODE_ID_ADDRESSED:
        printf("MTI_VERIFY_NODE_ID_ADDRESSED");
        break;
    case MTI_VERIFY_NODE_ID_GLOBAL:
        printf("MTI_VERIFY_NODE_ID_GLOBAL");
        break;
    case MTI_VERIFIED_NODE_ID:
        printf("MTI_VERIFIED_NODE_ID");
        break;
    case MTI_VERIFIED_NODE_ID_SIMPLE:
        printf("MTI_VERIFIED_NODE_ID_SIMPLE");
        break;
    case MTI_OPTIONAL_INTERACTION_REJECTED:
        printf("MTI_OPTIONAL_INTERACTION_REJECTED");
        break;
    case MTI_TERMINATE_DO_TO_ERROR:
        printf("MTI_TERMINATE_DO_TO_ERROR");
        break;
    case MTI_PROTOCOL_SUPPORT_INQUIRY:
        printf("MTI_PROTOCOL_SUPPORT_INQUIRY");
        break;
    case MTI_PROTOCOL_SUPPORT_REPLY:
        printf("MTI_PROTOCOL_SUPPORT_REPLY");
        break;
    case MTI_CONSUMER_IDENTIFY:
        printf("MTI_CONSUMER_IDENTIFY");
        break;
    case MTI_CONSUMER_RANGE_IDENTIFIED:
        printf("MTI_CONSUMER_RANGE_IDENTIFIED");
        break;
    case MTI_CONSUMER_IDENTIFIED_UNKNOWN:
        printf("MTI_CONSUMER_IDENTIFIED_UNKNOWN");
        break;
    case MTI_CONSUMER_IDENTIFIED_SET:
        printf("MTI_CONSUMER_IDENTIFIED_SET");
        break;
    case MTI_CONSUMER_IDENTIFIED_CLEAR:
        printf("MTI_CONSUMER_IDENTIFIED_CLEAR");
        break;
    case MTI_CONSUMER_IDENTIFIED_RESERVED:
        printf("MTI_CONSUMER_IDENTIFIED_RESERVED");
        break;
    case MTI_PRODUCER_IDENTIFY:
        printf("MTI_PRODUCER_IDENTIFY");
        break;
    case MTI_PRODUCER_RANGE_IDENTIFIED:
        printf("MTI_PRODUCER_RANGE_IDENTIFIED");
        break;
    case MTI_PRODUCER_IDENTIFIED_UNKNOWN:
        printf("MTI_PRODUCER_IDENTIFIED_UNKNOWN");
        break;
    case MTI_PRODUCER_IDENTIFIED_SET:
        printf("MTI_PRODUCER_IDENTIFIED_SET");
        break;
    case MTI_PRODUCER_IDENTIFIED_CLEAR:
        printf("MTI_PRODUCER_IDENTIFIED_CLEAR");
        break;
    case MTI_PRODUCER_IDENTIFIED_RESERVED:
        printf("MTI_PRODUCER_IDENTIFIED_RESERVED");
        break;
    case MTI_EVENTS_IDENTIFY_DEST:
        printf("MTI_EVENTS_IDENTIFY_DEST");
        break;
    case MTI_EVENTS_IDENTIFY:
        printf("MTI_EVENTS_IDENTIFY");
        break;
    case MTI_EVENT_LEARN:
        printf("MTI_EVENT_LEARN");
        break;
    case MTI_PC_EVENT_REPORT:
        printf("MTI_PC_EVENT_REPORT");
        break;
    case MTI_SIMPLE_NODE_INFO_REQUEST:
        printf("MTI_SIMPLE_NODE_INFO_REQUEST");
        break;
    case MTI_SIMPLE_NODE_INFO_REPLY:
        printf("MTI_SIMPLE_NODE_INFO_REPLY");
        break;
    case MTI_SIMPLE_TRAIN_INFO_REQUEST:
        printf("MTI_SIMPLE_TRAIN_INFO_REQUEST");
        break;
    case MTI_SIMPLE_TRAIN_INFO_REPLY:
        printf("MTI_SIMPLE_TRAIN_INFO_REPLY");
        break;
    case MTI_TRACTION_PROTOCOL:
        printf("MTI_TRACTION_PROTOCOL");
        break;
    case MTI_TRACTION_REPLY:
        printf("MTI_TRACTION_REPLY ");
        break;
    case MTI_STREAM_INIT_REQUEST:
        printf("MTI_STREAM_INIT_REQUEST");
        break;
    case MTI_STREAM_INIT_REPLY:
        printf("MTI_STREAM_INIT_REPLY");
        break;
    case MTI_FRAME_TYPE_CAN_STREAM_SEND:
        printf("MTI_FRAME_TYPE_CAN_STREAM_SEND");
        break;
    case MTI_STREAM_PROCEED:
        printf("MTI_STREAM_PROCEED");
        break;
    case MTI_STREAM_COMPLETE:
        printf("MTI_STREAM_COMPLETE");
        break;
    case MTI_DATAGRAM:
        printf("MTI_DATAGRAM");
        break;
    case MTI_DATAGRAM_OK_REPLY:
        printf("MTI_DATAGRAM_OK_REPLY");
        break;
    case MTI_DATAGRAM_REJECTED_REPLY:
        printf("MTI_DATAGRAM_REJECTED_REPLY");
        break;
    default:
        printf("[UNKNOWN MTI]");
        break;
    };

    printf("\n");
};

void PrintOpenLcbMsg(openlcb_msg_t *openlcb_msg)
{

    if (openlcb_msg)
    {

        printf("Source : ");
        PrintAliasAndNodeID(openlcb_msg->source_alias, openlcb_msg->source_id);
        printf("Dest : ");
        PrintAliasAndNodeID(openlcb_msg->dest_alias, openlcb_msg->dest_id);
        printf("mti : %04X\n", openlcb_msg->mti);
        PrintMtiName(openlcb_msg->mti);
        printf("Payload Count: %d = ", openlcb_msg->payload_count);
        printf("0x");
        for (int i = 0; i < openlcb_msg->payload_count; i++)
            printf("%02X", ((uint8_t *)openlcb_msg->payload)[i]);
        printf("\n");
        if (openlcb_msg->state.allocated)
            printf("Allocated: True\n");
        else
            printf("Allocated: False\n");
    }
}

void PrintCanMsg(can_msg_t *can_msg)
{

    printf("Identifier: ");

    printf("0x%04X", (uint16_t)(can_msg->identifier >> 16));
    printf("%04X", (uint16_t)(can_msg->identifier));

    printf(";  Buffer Count: %d  ", can_msg->payload_count);

    printf("[ ");
    for (int i = 0; i < can_msg->payload_count; i++)
    {

        if (i < (can_msg->payload_count - 1))
        {

            printf("%02X.", can_msg->payload[i]);
        }
        else
        {

            printf("%02X", can_msg->payload[i]);
        }
    }
    printf(" ]");
}

void PrintNode(openlcb_node_t *node)
{

    printf("State Info\n");
    printf("  allocated = 0x%02X\n", node->state.allocated);
    printf("  permitted = 0x%02X\n", node->state.permitted);
    printf("  initialized = 0x%02X\n", node->state.initialized);
    printf("  duplicate_id_detected = 0x%02X\n", node->state.duplicate_id_detected);
    printf("  openlcb_datagram_ack_sent = 0x%02X\n", node->state.openlcb_datagram_ack_sent);
    printf("  resend_datagram = 0x%02X\n", node->state.resend_datagram);
    printf("  State = %d\n", node->state.run_state);

    printf("ID: ");
    PrintInt64(node->id);
    printf("Alias: %04X\n", node->alias);
    printf("Parameters: 0x%p\n", node->parameters);
    printf("Sent Datagrams: 0x%p\n", &node->last_received_datagram);
    if (node->last_received_datagram)
        PrintOpenLcbMsg(node->last_received_datagram);
    else
        printf("  null\n");
    printf("NodeLock ID: ");
    PrintInt64(node->owner_node);
    printf("Timer Ticks: %u", node->timerticks);
}

void PrintEventID(event_id_t event_id)
{

    printf("EventID: 0x%04X", (uint16_t)(event_id >> 48));
    printf("%04X", (uint16_t)(event_id >> 32));
    printf("%04X", (uint16_t)(event_id >> 16));
    printf("%04X\n", (uint16_t)(event_id >> 0));
}

void PrintCanFrameIdentifierName(uint32_t identifier)
{

    if (identifier & 0xFF000000 & ~RESERVED_TOP_BIT)
    {

        switch (identifier & 0xFF000000 & ~RESERVED_TOP_BIT)
        {

        case CAN_CONTROL_FRAME_CID1:
            printf("CAN_CONTROL_FRAME_CID1\n");
            return;
        case CAN_CONTROL_FRAME_CID2:
            printf("CAN_CONTROL_FRAME_CID2\n");
            return;
        case CAN_CONTROL_FRAME_CID3:
            printf("CAN_CONTROL_FRAME_CID3\n");
            return;
        case CAN_CONTROL_FRAME_CID4:
            printf("CAN_CONTROL_FRAME_CID4\n");
            return;
        case CAN_CONTROL_FRAME_CID5:
            printf("CAN_CONTROL_FRAME_CID5\n");
            return;
        case CAN_CONTROL_FRAME_CID6:
            printf("CAN_CONTROL_FRAME_CID6\n");
            return;
        case CAN_CONTROL_FRAME_CID7:
            printf("CAN_CONTROL_FRAME_CID7\n");
            return;
        default:
            printf("[UNKNOWN]\n");
            return;
        }
    }

    switch (identifier & 0xFFFFF000 & ~RESERVED_TOP_BIT)
    {

    case CAN_CONTROL_FRAME_AMD:
        printf("CAN_CONTROL_FRAME_AMD\n");
        return;
    case CAN_CONTROL_FRAME_AME:
        printf("CAN_CONTROL_FRAME_AME\n");
        return;
    case CAN_CONTROL_FRAME_AMR:
        printf("CAN_CONTROL_FRAME_AMR\n");
        return;
    case CAN_CONTROL_FRAME_RID:
        printf("CAN_CONTROL_FRAME_RID\n");
        return;
    case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0:
        printf("CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0\n");
        return;
    case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1:
        printf("CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1\n");
        return;
    case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2:
        printf("CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2\n");
        return;
    case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3:
        printf("CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3\n");
        return;
    default:
        printf("[UNKNOWN]\n");
        return;
    }
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

void PrintRxFIFOStatus(DL_MCAN_RxFIFOStatus *fifo_Status)
{

    printf("Rx Fifo Status: num = %d, fillLvl = %d, getIdx = %d, putIdx = %d, fifoFull = %d, msgLost = %d\n",
           fifo_Status->num,
           fifo_Status->fillLvl,
           fifo_Status->getIdx,
           fifo_Status->putIdx,
           fifo_Status->fifoFull,
           fifo_Status->msgLost);
}

void PrintfTxFIFOStatus(DL_MCAN_TxFIFOStatus *fifo_Status)
{

    printf("Tx Fifo Status: freeLvl = %d, getIdx = %d, putIdx = %d, fifoFull = %d",
           fifo_Status->freeLvl,
           fifo_Status->getIdx,
           fifo_Status->putIdx,
           fifo_Status->fifoFull);
}
