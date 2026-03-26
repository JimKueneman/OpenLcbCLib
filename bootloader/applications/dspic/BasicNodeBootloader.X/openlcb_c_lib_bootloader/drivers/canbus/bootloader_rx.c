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
 * @file bootloader_rx.c
 *
 * CAN receive engine for the standalone bootloader. Polls hardware, filters
 * frames, assembles multi-frame datagrams, flags assembly errors, and buffers
 * non-datagram frames in a circular FIFO.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "bootloader_rx.h"

/* ====================================================================== */
/* Constants                                                               */
/* ====================================================================== */

/** Assembly timeout: 30 ticks x 100ms = 3 seconds. */
#define BOOTLOADER_RX_ASSEMBLY_TIMEOUT_TICKS 30

/* ====================================================================== */
/* Internal state                                                          */
/* ====================================================================== */

typedef struct {

    const bootloader_can_driver_t *can_driver;

    /* Non-datagram frame FIFO. */
    bootloader_can_frame_t buffer[BOOTLOADER_RX_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;

    /* Collision flags. */
    unsigned collision_cid : 1;
    unsigned collision_non_cid : 1;

    /* Datagram assembly. */
    uint8_t datagram_buffer[72];
    uint8_t datagram_len;
    uint16_t datagram_src;
    unsigned datagram_in_progress : 1;
    unsigned datagram_complete : 1;
    uint8_t datagram_last_tick;

    /* Error flagging. */
    bootloader_rx_error_t pending_error;

} _rx_state_t;

static _rx_state_t _rx;

/* ====================================================================== */
/* Circular buffer helpers                                                 */
/* ====================================================================== */

static bool _buffer_full(void) {

    return _rx.count >= BOOTLOADER_RX_BUFFER_SIZE;

}

static void _buffer_push(const bootloader_can_frame_t *frame) {

    if (_buffer_full()) { return; }

    _rx.buffer[_rx.head] = *frame;
    _rx.head = (_rx.head + 1) % BOOTLOADER_RX_BUFFER_SIZE;
    _rx.count++;

}

/* ====================================================================== */
/* Frame classification helpers                                            */
/* ====================================================================== */

static bool _is_cid_frame(uint32_t can_id) {

    uint32_t frame_type = can_id & MASK_CAN_FRAME_TYPE;

    return (!(can_id & CAN_OPENLCB_MSG)) &&
            (frame_type >= CAN_CONTROL_FRAME_CID4 &&
             frame_type <= CAN_CONTROL_FRAME_CID7);

}

static bool _is_alias_collision(uint32_t can_id, uint16_t our_alias) {

    return our_alias != 0 && (can_id & 0xFFF) == our_alias;

}

static bool _is_control_frame(uint32_t can_id) {

    return !(can_id & CAN_OPENLCB_MSG);

}

static bool _is_ame_frame(uint32_t can_id) {

    return _is_control_frame(can_id) &&
            (can_id & MASK_CAN_VARIABLE_FIELD) == CAN_CONTROL_FRAME_AME;

}

/**
 *     Returns the datagram frame type if the frame is a datagram addressed
 *     to us, or 0 if it is not a datagram for us.
 *
 *     @param can_id    CAN ID with EFF flag stripped
 *     @param our_alias our current alias
 *     @return CAN_FRAME_TYPE_DATAGRAM_* value, or 0
 */
static uint32_t _datagram_type_for_us(uint32_t can_id, uint16_t our_alias) {

    uint32_t frame_type = can_id & MASK_CAN_FRAME_TYPE;

    if (frame_type < CAN_FRAME_TYPE_DATAGRAM_ONLY ||
            frame_type > CAN_FRAME_TYPE_DATAGRAM_FINAL) {

        return 0;

    }

    uint16_t dest = (can_id >> 12) & 0xFFF;

    if (dest != our_alias) { return 0; }

    return frame_type;

}

static bool _is_addressed_for_us(const bootloader_can_frame_t *frame, uint16_t our_alias) {

    uint32_t can_id = frame->can_id;
    uint32_t frame_type = can_id & MASK_CAN_FRAME_TYPE;

    if (frame_type != OPENLCB_MESSAGE_STANDARD_FRAME_TYPE) { return false; }

    uint16_t mti_raw = (can_id >> 12) & 0xFFF;
    bool is_addressed = (mti_raw & 0x0008) != 0;

    if (!is_addressed) { return false; }

    if (frame->can_dlc < 2) { return false; }

    uint16_t dest_alias = ((frame->data[0] & 0x0F) << 8) | frame->data[1];
    return dest_alias == our_alias;

}

static bool _is_verify_node_id_global(const bootloader_can_frame_t *frame) {

    uint32_t can_id = frame->can_id;
    uint32_t frame_type = can_id & MASK_CAN_FRAME_TYPE;

    if (frame_type != OPENLCB_MESSAGE_STANDARD_FRAME_TYPE) { return false; }

    uint16_t mti_raw = (can_id >> 12) & 0xFFF;
    return mti_raw == (MTI_VERIFY_NODE_ID_GLOBAL & 0xFFF);

}

/* ====================================================================== */
/* Datagram assembly helpers                                               */
/* ====================================================================== */

static void _flag_error(uint16_t error_code, uint16_t src_alias) {

    _rx.pending_error.has_error = 1;
    _rx.pending_error.error_code = error_code;
    _rx.pending_error.error_src_alias = src_alias;
    _rx.datagram_in_progress = 0;
    _rx.datagram_len = 0;

}

static void _append_datagram_data(const uint8_t *data, uint8_t dlc) {

    uint8_t copy_len = dlc;

    if (_rx.datagram_len + copy_len > 72) {

        copy_len = 72 - _rx.datagram_len;

    }

    memcpy(_rx.datagram_buffer + _rx.datagram_len, data, copy_len);
    _rx.datagram_len += copy_len;

}

static void _handle_datagram_frame(uint32_t frame_type, uint16_t src_alias, const uint8_t *data, uint8_t dlc, uint8_t current_tick) {

    if (frame_type == CAN_FRAME_TYPE_DATAGRAM_ONLY) {

        if (_rx.datagram_in_progress) {

            _flag_error(ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END, _rx.datagram_src);

        }

        _rx.datagram_len = 0;
        _rx.datagram_src = src_alias;
        _append_datagram_data(data, dlc);
        _rx.datagram_complete = 1;
        _rx.datagram_in_progress = 0;
        return;

    }

    if (frame_type == CAN_FRAME_TYPE_DATAGRAM_FIRST) {

        if (_rx.datagram_in_progress) {

            _flag_error(ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END, _rx.datagram_src);

        }

        _rx.datagram_len = 0;
        _rx.datagram_src = src_alias;
        _rx.datagram_in_progress = 1;
        _rx.datagram_last_tick = current_tick;
        _append_datagram_data(data, dlc);
        return;

    }

    /* MIDDLE or FINAL — must have a matching in-progress assembly. */
    if (!_rx.datagram_in_progress || src_alias != _rx.datagram_src) {

        _flag_error(ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START, src_alias);
        return;

    }

    /* Check timeout. */
    uint8_t elapsed = (uint8_t) (current_tick - _rx.datagram_last_tick);

    if (elapsed >= BOOTLOADER_RX_ASSEMBLY_TIMEOUT_TICKS) {

        _flag_error(ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START, src_alias);
        return;

    }

    _rx.datagram_last_tick = current_tick;
    _append_datagram_data(data, dlc);

    if (frame_type == CAN_FRAME_TYPE_DATAGRAM_FINAL) {

        _rx.datagram_complete = 1;
        _rx.datagram_in_progress = 0;

    }

}

/* ====================================================================== */
/* Public API                                                              */
/* ====================================================================== */

void BootloaderRx_init(const bootloader_can_driver_t *can_driver) {

    memset(&_rx, 0, sizeof(_rx));
    _rx.can_driver = can_driver;

}

void BootloaderRx_poll(uint16_t our_alias, uint8_t current_tick) {

    bootloader_can_frame_t frame;

    while (_rx.can_driver->read_received_frame(&frame)) {

        uint32_t can_id = frame.can_id;

        /* Strip EFF flag for field extraction. */
        uint32_t id = can_id & ~BOOTLOADER_CAN_EFF_FLAG;

        /* Discard non-EFF frames. */
        if (!(can_id & BOOTLOADER_CAN_EFF_FLAG)) { continue; }

        /* Discard frames without reserved top bit. */
        if (!(id & RESERVED_TOP_BIT)) { continue; }

        /* Check alias collision — set flags, do not buffer. */
        if (_is_alias_collision(id, our_alias)) {

            if (_is_cid_frame(id)) {

                _rx.collision_cid = 1;

            } else {

                _rx.collision_non_cid = 1;

            }

            continue;

        }

        /* CAN control frames — only buffer AME. */
        if (_is_control_frame(id)) {

            if (_is_ame_frame(id)) {

                _buffer_push(&frame);

            }

            continue;

        }

        /* Datagram frames — assemble directly, do not enter FIFO. */
        uint32_t dg_type = _datagram_type_for_us(id, our_alias);

        if (dg_type != 0) {

            uint16_t src = id & 0xFFF;
            _handle_datagram_frame(dg_type, src, frame.data, frame.can_dlc, current_tick);
            continue;

        }

        /* Standard OpenLCB messages. */
        if (_is_addressed_for_us(&frame, our_alias)) {

            _buffer_push(&frame);
            continue;

        }

        if (_is_verify_node_id_global(&frame)) {

            _buffer_push(&frame);
            continue;

        }

        /* Everything else discarded. */

    }

}

/* ====================================================================== */
/* Non-datagram frame FIFO                                                 */
/* ====================================================================== */

bool BootloaderRx_has_frame(void) {

    return _rx.count > 0;

}

bool BootloaderRx_get_frame(bootloader_can_frame_t *frame) {

    if (_rx.count == 0) { return false; }

    *frame = _rx.buffer[_rx.tail];
    _rx.tail = (_rx.tail + 1) % BOOTLOADER_RX_BUFFER_SIZE;
    _rx.count--;

    return true;

}

/* ====================================================================== */
/* Datagram assembly                                                       */
/* ====================================================================== */

bool BootloaderRx_has_datagram(void) {

    return _rx.datagram_complete;

}

bool BootloaderRx_get_datagram(uint16_t *src_alias, uint8_t *buffer, uint8_t *len) {

    if (!_rx.datagram_complete) { return false; }

    *src_alias = _rx.datagram_src;
    memcpy(buffer, _rx.datagram_buffer, _rx.datagram_len);
    *len = _rx.datagram_len;

    _rx.datagram_complete = 0;
    _rx.datagram_len = 0;

    return true;

}

/* ====================================================================== */
/* Assembly error reporting                                                */
/* ====================================================================== */

bool BootloaderRx_has_error(void) {

    return _rx.pending_error.has_error;

}

bootloader_rx_error_t BootloaderRx_get_error(void) {

    bootloader_rx_error_t err = _rx.pending_error;
    _rx.pending_error.has_error = 0;
    return err;

}

/* ====================================================================== */
/* Collision flags                                                         */
/* ====================================================================== */

bool BootloaderRx_collision_cid(void) {

    return _rx.collision_cid;

}

bool BootloaderRx_collision_non_cid(void) {

    return _rx.collision_non_cid;

}

void BootloaderRx_clear_collision_flags(void) {

    _rx.collision_cid = 0;
    _rx.collision_non_cid = 0;

}
