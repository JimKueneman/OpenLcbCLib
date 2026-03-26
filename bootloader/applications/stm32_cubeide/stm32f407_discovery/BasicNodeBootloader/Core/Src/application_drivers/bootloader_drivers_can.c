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
 * STM32F407 CAN physical-layer driver implementation for the standalone
 * bootloader.  Reads and writes CAN frames through the bxCAN peripheral
 * via STM32 HAL.  Functions are public so main.c can wire them into the
 * bootloader_can_driver_t DI struct.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "stm32f4xx_hal.h"
#include "bootloader_drivers_can.h"
#include "../../../../shared/bootloader_shared_ram.h"
#include "../openlcb_c_lib_bootloader/drivers/canbus/bootloader_can.h"

extern CAN_HandleTypeDef hcan1;

/* ====================================================================== */
/* CAN receive                                                             */
/* ====================================================================== */

bool BootloaderDriversCan_read_received_frame(bootloader_can_frame_t *frame) {

    static bool _can_started = false;

    if (!_can_started) {

        CAN_FilterTypeDef f = {0};
        f.FilterBank           = 0;
        f.FilterMode           = CAN_FILTERMODE_IDMASK;
        f.FilterScale          = CAN_FILTERSCALE_32BIT;
        f.FilterIdHigh         = 0x0000;
        f.FilterIdLow          = 0x0000;
        f.FilterMaskIdHigh     = 0x0000;
        f.FilterMaskIdLow      = 0x0000;
        f.FilterFIFOAssignment = CAN_RX_FIFO0;
        f.FilterActivation     = ENABLE;
        HAL_CAN_ConfigFilter(&hcan1, &f);
        HAL_CAN_Start(&hcan1);
        _can_started = true;

    }

    if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0) {

        return false;

    }

    CAN_RxHeaderTypeDef rx_hdr;
    uint8_t rx_data[8];

    if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_hdr, rx_data) != HAL_OK) {

        return false;

    }

    frame->can_id = (rx_hdr.IDE == CAN_ID_EXT)
                  ? (rx_hdr.ExtId | BOOTLOADER_CAN_EFF_FLAG)
                  : rx_hdr.StdId;

    frame->can_dlc = (rx_hdr.DLC > 8) ? 8 : (uint8_t) rx_hdr.DLC;

    memcpy(frame->data, rx_data, frame->can_dlc);

    return true;

}

/* ====================================================================== */
/* CAN transmit                                                            */
/* ====================================================================== */

bool BootloaderDriversCan_try_send_frame(const bootloader_can_frame_t *frame) {

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3) {

        return false;

    }

    CAN_TxHeaderTypeDef tx_hdr = {0};
    uint32_t mailbox;

    if (frame->can_id & BOOTLOADER_CAN_EFF_FLAG) {

        tx_hdr.ExtId = frame->can_id & 0x1FFFFFFFU;
        tx_hdr.IDE   = CAN_ID_EXT;

    } else {

        tx_hdr.StdId = frame->can_id & 0x7FFU;
        tx_hdr.IDE   = CAN_ID_STD;

    }

    tx_hdr.RTR               = CAN_RTR_DATA;
    tx_hdr.DLC               = frame->can_dlc;
    tx_hdr.TransmitGlobalTime = DISABLE;

    return (HAL_CAN_AddTxMessage(&hcan1, &tx_hdr,
            (uint8_t *) frame->data, &mailbox) == HAL_OK);

}

/* ====================================================================== */
/* Node alias                                                              */
/* ====================================================================== */

uint16_t BootloaderDriversCan_get_cached_alias_passed_from_application(void) {

    uint16_t alias = bootloader_cached_alias;
    bootloader_cached_alias = 0;

    return alias;

}
