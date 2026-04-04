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
#include "../../shared/bootloader_shared_ram.h"
#include "../openlcb_c_lib_bootloader/drivers/canbus/bootloader_can.h"
#include "../mcc_generated_files/can1.h"

/* ====================================================================== */
/* Init                                                                    */
/* ====================================================================== */

void BootloaderDriversCan_initialize(void) {

    CAN1_TransmitEnable();
    CAN1_ReceiveEnable();

}

/* ====================================================================== */
/* CAN receive                                                             */
/* ====================================================================== */

bool BootloaderDriversCan_read_received_frame(bootloader_can_frame_t *frame) {

    if (CAN1_ReceivedMessageCountGet() == 0) {

        return false;

    }

    uint8_t rx_data[8] = {0};
    CAN_MSG_OBJ rx_msg;
    rx_msg.data = rx_data;

    if (!CAN1_Receive(&rx_msg)) {

        return false;

    }

    if (rx_msg.field.idType == CAN_FRAME_EXT) {

        frame->can_id = (rx_msg.msgId & 0x1FFFFFFFU) | BOOTLOADER_CAN_EFF_FLAG;

    } else {

        frame->can_id = rx_msg.msgId & 0x7FFU;

    }

    frame->can_dlc = (uint8_t) rx_msg.field.dlc;

    uint8_t byte_index;

    for (byte_index = 0; byte_index < frame->can_dlc; byte_index++) {

        frame->data[byte_index] = rx_data[byte_index];

    }

    return true;

}

/* ====================================================================== */
/* CAN transmit                                                            */
/* ====================================================================== */

bool BootloaderDriversCan_try_send_frame(const bootloader_can_frame_t *frame) {

    uint8_t tx_data[8] = {0};
    uint8_t byte_index;

    for (byte_index = 0; byte_index < frame->can_dlc && byte_index < 8U; byte_index++) {

        tx_data[byte_index] = frame->data[byte_index];

    }

    CAN_MSG_OBJ tx_msg;
    tx_msg.data             = tx_data;
    tx_msg.field.frameType  = CAN_FRAME_DATA;
    tx_msg.field.dlc        = (uint8_t) frame->can_dlc;
    tx_msg.field.reserved   = 0;

    if (frame->can_id & BOOTLOADER_CAN_EFF_FLAG) {

        tx_msg.msgId         = frame->can_id & 0x1FFFFFFFU;
        tx_msg.field.idType  = CAN_FRAME_EXT;

    } else {

        tx_msg.msgId         = frame->can_id & 0x7FFU;
        tx_msg.field.idType  = CAN_FRAME_STD;

    }

    return (CAN1_Transmit(CAN_PRIORITY_HIGH, &tx_msg) == CAN_TX_MSG_REQUEST_SUCCESS);

}

/* ====================================================================== */
/* Node alias                                                              */
/* ====================================================================== */

uint16_t BootloaderDriversCan_get_cached_alias_passed_from_application(void) {

    uint16_t alias = bootloader_cached_alias;
    bootloader_cached_alias = 0;

    return alias;

}
