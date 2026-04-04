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
 * ESP32 CAN physical-layer driver implementation for the standalone bootloader.
 * Reads and writes CAN frames through the TWAI peripheral in polling mode --
 * no FreeRTOS receive task is created.  Functions are public so main.cpp can
 * wire them into the bootloader_can_driver_t DI struct.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include "bootloader_drivers_can.h"
#include "../../shared/bootloader_shared_ram.h"
#include "driver/twai.h"
#include "../openlcb_c_lib_bootloader/drivers/canbus/bootloader_can.h"

/* ====================================================================== */
/* CAN receive                                                             */
/* ====================================================================== */

bool BootloaderDriversCan_read_received_frame(bootloader_can_frame_t *frame) {

    twai_message_t message;

    /*
     * Zero timeout: return immediately if no frame is in the RX queue.
     * The bootloader runs a tight polling loop so blocking is not needed.
     */
    esp_err_t err = twai_receive(&message, 0);

    if (err != ESP_OK) {

        return false;

    }

    /*
     * OpenLCB over CAN uses 29-bit extended frames exclusively.
     * Discard any standard (11-bit) frames silently.
     */
    if (!message.extd) {

        return false;

    }

    frame->can_id  = (message.identifier & 0x1FFFFFFFU) | BOOTLOADER_CAN_EFF_FLAG;
    frame->can_dlc = (uint8_t) message.data_length_code;

    for (uint8_t byte_index = 0; byte_index < frame->can_dlc; byte_index++) {

        frame->data[byte_index] = message.data[byte_index];

    }

    return true;

}

/* ====================================================================== */
/* CAN transmit                                                            */
/* ====================================================================== */

bool BootloaderDriversCan_try_send_frame(const bootloader_can_frame_t *frame) {

    twai_message_t message = {0};

    message.identifier       = frame->can_id & 0x1FFFFFFFU;
    message.extd             = 1;   /* 29-bit extended frame */
    message.rtr              = 0;
    message.ss               = 0;   /* retry on error (not single-shot) */
    message.self             = 0;
    message.dlc_non_comp     = 0;
    message.data_length_code = frame->can_dlc;

    for (uint8_t byte_index = 0; byte_index < frame->can_dlc; byte_index++) {

        message.data[byte_index] = frame->data[byte_index];

    }

    /*
     * Zero timeout: return false immediately if the TX queue is full
     * rather than blocking.  The bootloader library retries on the next
     * poll cycle.
     */
    return (twai_transmit(&message, 0) == ESP_OK);

}

/* ====================================================================== */
/* Node alias                                                              */
/* ====================================================================== */

uint16_t BootloaderDriversCan_get_cached_alias_passed_from_application(void) {

    uint16_t alias      = bootloader_shared_ram.cached_alias;
    bootloader_shared_ram.cached_alias = 0;

    return alias;

}
