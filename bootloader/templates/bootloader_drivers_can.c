/** @copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * @file bootloader_drivers_can.c
 *
 * dsPIC33EP512MC506 CAN physical-layer driver implementation for the
 * standalone bootloader.  Reads and writes CAN frames through the ECAN1
 * peripheral via the MCC-generated CAN1 driver (polling mode -- no ISR
 * required for bootloader operation).  Functions are public so
 * bootloader_main.c can wire them into the bootloader_can_driver_t DI struct.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include "bootloader_drivers_can.h"
#include "../../../shared/bootloader_shared_ram.h"
#include "../openlcb_bootloader_src/drivers/canbus/bootloader_can.h"
#include "../../mcc_generated_files/can1.h"

/* ====================================================================== */
/* CAN receive                                                             */
/* ====================================================================== */

bool BootloaderDriversCan_read_received_frame(bootloader_can_frame_t *frame) {

    if (CAN1_ReceivedMessageCountGet() == 0) {

        return false;

    }

    uint8_t rx_data[8] = {0};
    CAN_MSG_OBJ msg;
    msg.data = rx_data;

    if (!CAN1_Receive(&msg)) {

        return false;

    }

    if (msg.field.idType == CAN_FRAME_EXT) {

        frame->can_id = (msg.msgId & 0x1FFFFFFFU) | BOOTLOADER_CAN_EFF_FLAG;

    } else {

        frame->can_id = msg.msgId & 0x7FFU;

    }

    frame->can_dlc = (uint8_t) msg.field.dlc;

    uint8_t i;

    for (i = 0; i < frame->can_dlc; i++) {

        frame->data[i] = rx_data[i];

    }

    return true;

}

/* ====================================================================== */
/* CAN transmit                                                            */
/* ====================================================================== */

bool BootloaderDriversCan_try_send_frame(const bootloader_can_frame_t *frame) {

    uint8_t tx_data[8] = {0};
    uint8_t i;

    for (i = 0; i < frame->can_dlc && i < 8U; i++) {

        tx_data[i] = frame->data[i];

    }

    CAN_MSG_OBJ msg;
    msg.data             = tx_data;
    msg.field.frameType  = CAN_FRAME_DATA;
    msg.field.dlc        = (uint8_t) frame->can_dlc;
    msg.field.reserved   = 0;

    if (frame->can_id & BOOTLOADER_CAN_EFF_FLAG) {

        msg.msgId         = frame->can_id & 0x1FFFFFFFU;
        msg.field.idType  = CAN_FRAME_EXT;

    } else {

        msg.msgId         = frame->can_id & 0x7FFU;
        msg.field.idType  = CAN_FRAME_STD;

    }

    return (CAN1_Transmit(CAN_PRIORITY_HIGH, &msg) == CAN_TX_MSG_REQUEST_SUCCESS);

}

/* ====================================================================== */
/* Node alias                                                              */
/* ====================================================================== */

uint16_t BootloaderDriversCan_get_cached_alias_passed_from_application(void) {

    uint16_t alias = bootloader_cached_alias;
    bootloader_cached_alias = 0;

    return alias;

}
