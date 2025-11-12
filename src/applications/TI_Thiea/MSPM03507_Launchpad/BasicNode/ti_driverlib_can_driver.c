/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \file ti_driverlib_can_driver.c
 *
 * Definition of the node at the application level.  
 *
 * @author Jim Kueneman
 * @date 11 Nov 2024
 */

 #include "ti_driverlib_can_driver.h"

#include <ti/driverlib/m0p/dl_interrupt.h>
#include "ti_msp_dl_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "debug_tools.h"

#include "src/openlcb/openlcb_types.h"
#include "src/drivers/common/can_types.h"


 void TI_DriverLibCanDriver_initialize(void) {

    // Enabled MCAN0 interrupts
    NVIC_EnableIRQ(MCAN0_INST_INT_IRQN);

    // Is this needed?
    DL_MCAN_TXBufTransIntrEnable(MCAN0_INST, 1U, 1U);

 }

 bool TI_DriverLibCanDriver_is_can_tx_buffer_clear(void) {

    return true;

 }

 void TI_DriverLibCanDriver_pause_can_rx(void) {

    NVIC_DisableIRQ(MCAN0_INST_INT_IRQN);
 }

 void TI_DriverLibCanDriver_resume_can_rx(void) {

    NVIC_EnableIRQ(MCAN0_INST_INT_IRQN);

 }

 bool TI_DriverLibCanDriver_transmit_can_frame(can_msg_t *msg) {

    DL_MCAN_TxBufElement txMsg;


   /* Initialize message to transmit. */

    
    txMsg.id = msg->identifier; //  Identifier Value
    txMsg.rtr = 0U;                 // Transmit data frame - normal frame
    txMsg.xtd = 1U;                 // 11-bit standard identifier - false it is a 19 bit frame
    txMsg.esi = 0U;                 // ESI bit in CAN FD format depends only on gError passive flag - not a FD frame
    txMsg.dlc = msg->payload_count; // Transmitting N bytes
    txMsg.brs = 0U;                 // CAN FD frames transmitted with bit rate switching - not a FD frame
    txMsg.fdf = 0U;  // Frame transmitted in CAN FD format - not a FD frame
    txMsg.efc = 1U; // Store Tx events - do throw TX events
    txMsg.mm = 0x00U;  // Message Marker - not sure what this does but seems like it is an ID key value
    for (int i = 0; i < msg->payload_count; i++) {  // Copy the data

        txMsg.data[i] = msg->payload[i];

    } 

    // Write Tx Message to the Message RAM 
    DL_MCAN_writeMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_BUF, 1U, &txMsg);
    // Add request for transmission
    DL_MCAN_TXBufAddReq(MCAN0_INST, 1U);

    return true;

 }

 void MCAN0_INST_IRQHandler(void) {

    uint32_t interrupt_flags = 0;
    DL_MCAN_RxFIFOStatus fifo_Status;
    DL_MCAN_RxBufElement rxMsg;
    can_msg_t can_msg;

    // Pull out the interrupt "group" that is triggered from the CAN0 instance
    DL_MCAN_IIDX pending_interrupt_index = DL_MCAN_getPendingInterrupt(MCAN0_INST);

    switch (pending_interrupt_index) {

        case DL_MCAN_IIDX_LINE0:

            // We have no interrupts set up to use Line 0 so this is not used 

        break;

        case DL_MCAN_IIDX_LINE1:
            
            // Now we need to look into Line 1 interrupt and see what flags are set under this index, 
            // including strip off any bits that are of unknown flags so when we clear them we don't 
            // clear some unknown flag bit that could cause problems
            interrupt_flags = DL_MCAN_getIntrStatus(MCAN0_INST) & MCAN0_INST_MCAN_INTERRUPTS;
            // We have a copy of the flags set so now clear them
            DL_MCAN_clearIntrStatus(MCAN0_INST, interrupt_flags, DL_MCAN_INTR_SRC_MCAN_LINE_1);

            // Is the Rx FIFO 1 New Message interrupt (0x0010) flag set?
            if ((interrupt_flags & DL_MCAN_INTERRUPT_RF1N) == DL_MCAN_INTERRUPT_RF1N) {

                // We are setup for passing the incoming messages into the FIFO memory and not the Buffer memory so we use
                // the Rx FIFO DL_xxxx macros

                // clear the fifo status buffer, BUT what is not obvious is the num parameter is [IN/OUT] and selects what FIFO bank you want
                // Note we have the .sysconf set so only FIFO 1 is used
                memset(&fifo_Status, 0x00, sizeof(fifo_Status));
                fifo_Status.num = DL_MCAN_RX_FIFO_NUM_1;

                // Run through the number avialable
                DL_MCAN_getRxFIFOStatus(MCAN0_INST, &fifo_Status);

                while (fifo_Status.fillLvl > 0) {
         
                    // Reading RAM into the rxMsg structure in the FIFO memory (DL_MCAN_MEM_TYPE_FIFO), in Bank 1 (DL_MCAN_RX_FIFO_NUM_1)
                    // parameter 3 (0U) is not used in FIFO mode
                    DL_MCAN_readMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_FIFO, 0U, DL_MCAN_RX_FIFO_NUM_1, &rxMsg);
                    // Now update the FIFO parameters to increment the getIndex to the next position
                    DL_MCAN_writeRxFIFOAck(MCAN0_INST, DL_MCAN_RX_FIFO_NUM_1, fifo_Status.getIdx);
                   
                    can_msg.identifier = rxMsg.id;
                    can_msg.state.allocated = true;
                    can_msg.payload_count = rxMsg.dlc;
                    for (int iData = 0; iData < rxMsg.dlc; iData++) {

                       can_msg.payload[iData] = rxMsg.data[iData];

                    }

                    PrintCanMsg(&can_msg);
                    printf("\n");
                
                // TODO Uncomment to make work

          //         CanRxStatemachine_incoming_can_driver_callback(&can_msg);

                    DL_MCAN_getRxFIFOStatus(MCAN0_INST, &fifo_Status);

                }

            }

            // Is the Transmission Completed interrupt (0x0200) flag set?
            if ((interrupt_flags & DL_MCAN_INTERRUPT_TC) == DL_MCAN_INTERRUPT_TC) {

            }

            // We have other interrupts enabled in the .sysconf file for errors that can be handled here if desired, DL_MCAN_INTERRUPT_xxxxx

        break;


        case DL_MCAN_IIDX_WAKEUP:

            // We don't use low power mode so this is not used

        break;

        case DL_MCAN_IIDX_TIMESTAMP_OVERFLOW:

            // Need to do more research as to what this actually does and if we care about it

        break;

        case DL_MCAN_IIDX_DOUBLE_ERROR_DETECTION:

            // Need to do more research as to what this actually does and if we care about it

        break;

        case DL_MCAN_IIDX_SINGLE_ERROR_CORRECTION:
        
            // Need to do more research as to what this actually does and if we care about it

        break;


        default:

           // Unknown interrupt index

        break;

    }

}


