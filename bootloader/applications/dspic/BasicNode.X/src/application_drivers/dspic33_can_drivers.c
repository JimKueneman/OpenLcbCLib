/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * @file dspic33_can_drivers.c
 *
 * CAN hardware interface for dsPIC33 ECAN module.  Thin wrapper over MCC-
 * generated CAN1 APIs, translating between the library's can_msg_t and
 * MCC's CAN_MSG_OBJ.
 *
 * @author Jim Kueneman
 * @date 19 Mar 2026
 */

#include "dspic33_can_drivers.h"

#include "xc.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../../mcc_generated_files/can1.h"
#include "../openlcb_c_lib/drivers/canbus/can_rx_statemachine.h"

void Dspic33CanDriver_initialize(void) {

    /* CAN1_Initialize() already ran via SYSTEM_Initialize().
     * Enable the DMA TX/RX buffers so CAN1_Transmit/Receive work. */
    CAN1_TransmitEnable();
    CAN1_ReceiveEnable();

    /* CAN interrupt was disabled in MCC (bootloader uses polling).
     * The application needs interrupt-driven RX. */
    C1INTEbits.RBIE = 1;
    IEC2bits.C1IE = 1;

}

bool Dspic33CanDriver_is_can_tx_buffer_clear(void) {

    return (C1TR01CONbits.TXREQ0 == 0);

}

bool Dspic33CanDriver_transmit_can_frame(can_msg_t *msg) {

    CAN_MSG_OBJ mcc_msg;
    uint8_t data[8];

    memcpy(data, msg->payload, msg->payload_count);

    mcc_msg.msgId = msg->identifier;
    mcc_msg.field.idType = CAN_FRAME_EXT;
    mcc_msg.field.frameType = CAN_FRAME_DATA;
    mcc_msg.field.dlc = msg->payload_count;
    mcc_msg.data = data;

    return (CAN1_Transmit(CAN_PRIORITY_HIGH, &mcc_msg) == CAN_TX_MSG_REQUEST_SUCCESS);

}

void Dspic33CanDriver_pause_can_rx(void) {

    C1INTEbits.RBIE = 0;

}

void Dspic33CanDriver_resume_can_rx(void) {

    C1INTEbits.RBIE = 1;

}

/* CAN RX interrupt handler -- called two different ways:
 *
 *   Standalone:  _C1Interrupt (in mcc_generated_files/can1.c, hand-edited)
 *                calls this function directly.
 *
 *   Bootloaded:  The bootloader owns the hardware IVT.  Its _C1Interrupt
 *                stub reads bootloader_vivt_jumptable.can1_handler and calls
 *                through that pointer.  main() registers this function in
 *                the VIVT before enabling interrupts. */
void Dspic33CanDriver_c1_interrupt_handler(void) {

    IFS2bits.C1IF = 0;

    CAN_MSG_OBJ mcc_msg;
    uint8_t data[8];
    can_msg_t ecan_msg;

    mcc_msg.data = data;

    while (CAN1_ReceivedMessageCountGet() > 0) {

        if (CAN1_Receive(&mcc_msg)) {

            if (mcc_msg.field.idType == CAN_FRAME_EXT) {

                ecan_msg.identifier = mcc_msg.msgId;
                ecan_msg.payload_count = mcc_msg.field.dlc;
                memcpy(ecan_msg.payload, data, mcc_msg.field.dlc);

                CanRxStatemachine_incoming_can_driver_callback(&ecan_msg);

            }

        }

    }

}

