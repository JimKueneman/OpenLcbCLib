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
 * MSPM0G3507 CAN physical-layer driver implementation for the standalone
 * bootloader.  Reads and writes CAN frames through the MCAN peripheral
 * registers.  Functions are public so main.c can wire them into the
 * bootloader_can_driver_t DI struct.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "../Debug/ti_msp_dl_config.h"
#include "bootloader_drivers_can.h"
#include "../shared/bootloader_shared_ram.h"

#include "../src/drivers/canbus/bootloader_can.h"

/* ====================================================================== */
/* CAN receive                                                             */
/* ====================================================================== */

bool BootloaderDriversCan_read_received_frame(bootloader_can_frame_t *frame) {

    uint32_t rxf1s = CANFD0->MCANSS.MCAN.MCAN_RXF1S;
    uint32_t fill_lvl = (rxf1s & MCAN_RXF1S_F1FL_MASK) >> MCAN_RXF1S_F1FL_OFS;

    if (fill_lvl == 0) {

        return false;

    }

    uint32_t get_idx = (rxf1s & MCAN_RXF1S_F1GI_MASK) >> MCAN_RXF1S_F1GI_OFS;

    DL_MCAN_RxBufElement rx_elem;
    DL_MCAN_readMsgRam(CANFD0, DL_MCAN_MEM_TYPE_FIFO, get_idx, DL_MCAN_RX_FIFO_NUM_1, &rx_elem);

    DL_MCAN_writeRxFIFOAck(CANFD0, DL_MCAN_RX_FIFO_NUM_1, get_idx);

    if (rx_elem.xtd) {

        frame->can_id = (rx_elem.id & 0x1FFFFFFFU) | BOOTLOADER_CAN_EFF_FLAG;

    } else {

        frame->can_id = (rx_elem.id >> 18) & 0x7FFU;

    }

    frame->can_dlc = rx_elem.dlc;

    if (frame->can_dlc > 8) {

        frame->can_dlc = 8;

    }

    memcpy(frame->data, rx_elem.data, frame->can_dlc);

    return true;

}

/* ====================================================================== */
/* CAN transmit                                                            */
/* ====================================================================== */

bool BootloaderDriversCan_try_send_frame(const bootloader_can_frame_t *frame) {

    uint32_t buf_idx = 0;
    uint32_t tx_req_pending = DL_MCAN_getTxBufReqPend(CANFD0);

    if (tx_req_pending & (1U << 0)) {

        if (tx_req_pending & (1U << 1)) {

            return false;

        }

        buf_idx = 1;

    }

    DL_MCAN_TxBufElement tx_elem;
    memset(&tx_elem, 0, sizeof(tx_elem));

    if (frame->can_id & BOOTLOADER_CAN_EFF_FLAG) {

        tx_elem.id = frame->can_id & 0x1FFFFFFFU;
        tx_elem.xtd = 1;

    } else {

        tx_elem.id = (frame->can_id & 0x7FFU) << 18;
        tx_elem.xtd = 0;

    }

    tx_elem.dlc = frame->can_dlc;
    tx_elem.fdf = 0;
    tx_elem.brs = 0;

    memcpy(tx_elem.data, frame->data, frame->can_dlc);

    DL_MCAN_writeMsgRam(CANFD0, DL_MCAN_MEM_TYPE_BUF, buf_idx, &tx_elem);
    DL_MCAN_TXBufAddReq(CANFD0, buf_idx);

    return true;

}

/* ====================================================================== */
/* Node alias                                                              */
/* ====================================================================== */

uint16_t BootloaderDriversCan_get_cached_alias_passed_from_application(void) {

    uint16_t alias = bootloader_cached_alias;
    bootloader_cached_alias = 0;

    return alias;

}
