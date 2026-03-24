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
 * @file bootloader_tx.c
 *
 * CAN transmit engine for the standalone bootloader.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "bootloader_tx.h"
#include "bootloader_rx.h"

/* ====================================================================== */
/* Internal state                                                          */
/* ====================================================================== */

typedef struct {

    const bootloader_can_driver_t *can_driver;

} _tx_state_t;

static _tx_state_t _tx;

/* ====================================================================== */
/* Helpers                                                                 */
/* ====================================================================== */

static void _build_addressed_frame(
        bootloader_can_frame_t *frame,
        uint16_t mti,
        uint16_t src_alias,
        uint16_t dest_alias,
        uint8_t frame_flag,
        const uint8_t *data,
        uint8_t data_len) {

    memset(frame, 0, sizeof(*frame));

    frame->can_id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT | CAN_OPENLCB_MSG |
            OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t)(mti & 0xFFF) << 12) | src_alias;

    frame->data[0] = frame_flag | ((dest_alias >> 8) & 0x0F);
    frame->data[1] = dest_alias & 0xFF;

    if (data && data_len > 0) {

        memcpy(frame->data + 2, data, data_len);

    }

    frame->can_dlc = 2 + data_len;

}

static void _block_send(const bootloader_can_frame_t *frame, uint16_t our_alias, uint8_t current_tick) {

    while (!_tx.can_driver->try_send_frame(frame)) {

        BootloaderRx_poll(our_alias, current_tick);

    }

}

/* ====================================================================== */
/* Public API                                                              */
/* ====================================================================== */

void BootloaderTx_init(const bootloader_can_driver_t *can_driver) {

    memset(&_tx, 0, sizeof(_tx));
    _tx.can_driver = can_driver;

}

bool BootloaderTx_send_frame(const bootloader_can_frame_t *frame) {

    return _tx.can_driver->try_send_frame(frame);

}

bool BootloaderTx_send_multiframe(
            uint16_t mti,
            uint16_t src_alias,
            uint16_t dest_alias,
            uint16_t our_alias,
            uint8_t current_tick,
            const uint8_t *data,
            uint8_t len) {

    bootloader_can_frame_t frame;

    /* Single frame — fits with the 2-byte address header. */
    if (len <= 6) {

        _build_addressed_frame(&frame, mti, src_alias, dest_alias, MULTIFRAME_ONLY, data, len);
        _block_send(&frame, our_alias, current_tick);
        return true;

    }

    /* First frame — 6 data bytes. */
    _build_addressed_frame(&frame, mti, src_alias, dest_alias, MULTIFRAME_FIRST, data, 6);
    _block_send(&frame, our_alias, current_tick);

    uint8_t offset = 6;

    /* Middle and last frames. */
    while (offset < len) {

        uint8_t remaining = len - offset;
        uint8_t chunk = (remaining > 6) ? 6 : remaining;
        uint8_t flag = (remaining > 6) ? MULTIFRAME_MIDDLE : MULTIFRAME_FINAL;

        _build_addressed_frame(&frame, mti, src_alias, dest_alias, flag, data + offset, chunk);
        _block_send(&frame, our_alias, current_tick);

        offset += chunk;

    }

    return true;

}

bool BootloaderTx_send_global(uint16_t mti, uint16_t src_alias, uint16_t our_alias, uint8_t current_tick, const uint8_t *data, uint8_t len) {

    bootloader_can_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    frame.can_id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_OPENLCB_MSG | OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t)(mti & 0xFFF) << 12) | src_alias;

    uint8_t copy_len = (len > 8) ? 8 : len;

    if (data && copy_len > 0) {

        memcpy(frame.data, data, copy_len);

    }

    frame.can_dlc = copy_len;

    _block_send(&frame, our_alias, current_tick);

    return true;

}

bool BootloaderTx_send_datagram(uint16_t src_alias, uint16_t dest_alias, uint16_t our_alias, uint8_t current_tick, const uint8_t *data, uint8_t len) {

    uint32_t base_id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_OPENLCB_MSG | ((uint32_t) dest_alias << 12) | src_alias;

    if (len <= 8) {

        bootloader_can_frame_t frame;
        memset(&frame, 0, sizeof(frame));

        frame.can_id = base_id | CAN_FRAME_TYPE_DATAGRAM_ONLY;
        frame.can_dlc = len;

        if (data && len > 0) {

            memcpy(frame.data, data, len);

        }

        _block_send(&frame, our_alias, current_tick);

    } else {

        uint8_t offset = 0;

        while (offset < len) {

            bootloader_can_frame_t frame;
            memset(&frame, 0, sizeof(frame));

            uint32_t frame_type;
            uint8_t remaining = len - offset;
            uint8_t chunk;

            if (offset == 0) {

                frame_type = CAN_FRAME_TYPE_DATAGRAM_FIRST;
                chunk = 8;

            } else if (remaining <= 8) {

                frame_type = CAN_FRAME_TYPE_DATAGRAM_FINAL;
                chunk = remaining;

            } else {

                frame_type = CAN_FRAME_TYPE_DATAGRAM_MIDDLE;
                chunk = 8;

            }

            frame.can_id = base_id | frame_type;
            frame.can_dlc = chunk;
            memcpy(frame.data, data + offset, chunk);

            _block_send(&frame, our_alias, current_tick);

            offset += chunk;

        }

    }

    return true;

}

bool BootloaderTx_send_datagram_ok(uint16_t src_alias, uint16_t dest_alias, uint16_t our_alias, uint8_t current_tick, uint8_t flags) {

    bootloader_can_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    frame.can_id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_OPENLCB_MSG | OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t)(MTI_DATAGRAM_OK_REPLY & 0xFFF) << 12) | src_alias;
    frame.data[0] = (dest_alias >> 8) & 0x0F;
    frame.data[1] = dest_alias & 0xFF;
    frame.data[2] = flags;
    frame.can_dlc = 3;

    _block_send(&frame, our_alias, current_tick);

    return true;

}

bool BootloaderTx_send_datagram_rejected(uint16_t src_alias, uint16_t dest_alias, uint16_t our_alias, uint8_t current_tick, uint16_t error_code) {

    bootloader_can_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    frame.can_id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_OPENLCB_MSG | OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t)(MTI_DATAGRAM_REJECTED_REPLY & 0xFFF) << 12) | src_alias;
    frame.data[0] = (dest_alias >> 8) & 0x0F;
    frame.data[1] = dest_alias & 0xFF;
    frame.data[2] = (error_code >> 8) & 0xFF;
    frame.data[3] = error_code & 0xFF;
    frame.can_dlc = 4;

    _block_send(&frame, our_alias, current_tick);

    return true;

}
