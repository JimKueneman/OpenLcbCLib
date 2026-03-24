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
 * @file bootloader_can.c
 *
 * CAN transport layer for the standalone bootloader.
 * Implements BootloaderTransport_* functions using CAN bus.
 * Handles alias negotiation, CAN frame construction/parsing,
 * and datagram assembly from multi-frame CAN messages.
 *
 * Alias generation algorithm matches the main OpenLcbCLib library
 * (can_login_message_handler.c).
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "bootloader_can.h"
#include "bootloader_can_driver.h"
#include "../../openlcb/bootloader_transport.h"
#include "../../openlcb/bootloader_driver.h"
#include "../../openlcb/bootloader_protocol.h"
#include "../../openlcb/bootloader_types.h"

/* ====================================================================== */
/* Init state machine                                                      */
/* ====================================================================== */

typedef enum {

    INIT_PICK_ALIAS = 0,
    INIT_SEND_CID_7,
    INIT_SEND_CID_6,
    INIT_SEND_CID_5,
    INIT_SEND_CID_4,
    INIT_WAIT_RID,
    INIT_SEND_RID,
    INIT_SEND_AMD,
    INIT_SEND_INIT_COMPLETE,
    INIT_INITIALIZED

} _init_state_enum;

/* ====================================================================== */
/* Internal CAN transport state                                            */
/* ====================================================================== */

typedef struct {

    bootloader_can_frame_t input_frame;
    bootloader_can_frame_t output_frame;

    unsigned input_frame_full : 1;
    unsigned output_frame_full : 1;
    unsigned datagram_output_pending : 1;
    unsigned datagram_reply_waiting : 1;
    unsigned incoming_datagram_pending : 1;

    uint16_t alias;
    uint64_t seed;
    _init_state_enum init_state;
    uint8_t wait_tick;
    uint8_t tx_timeout_tick;

    /** Outgoing datagram state. */
    uint16_t datagram_dst;
    uint8_t datagram_payload[72];
    uint8_t datagram_dlc;
    uint8_t datagram_offset;

    /** Incoming multi-frame datagram assembly. */
    uint8_t datagram_rx_buffer[72];
    uint8_t datagram_rx_len;
    uint16_t datagram_rx_src;

} _can_state_t;

static _can_state_t _can;

/* ====================================================================== */
/* Alias generation (matches main library can_login_message_handler.c)     */
/* ====================================================================== */

static uint64_t _generate_seed(uint64_t start_seed) {

    uint32_t lfsr2 = start_seed & 0xFFFFFF;
    uint32_t lfsr1 = (start_seed >> 24) & 0xFFFFFF;

    uint32_t temp1 = ((lfsr1 << 9) | ((lfsr2 >> 15) & 0x1FF)) & 0xFFFFFF;
    uint32_t temp2 = (lfsr2 << 9) & 0xFFFFFF;

    lfsr1 = lfsr1 + temp1 + 0x1B0CA3L;
    lfsr2 = lfsr2 + temp2 + 0x7A4BA9L;

    lfsr1 = (lfsr1 & 0xFFFFFF) + ((lfsr2 & 0xFF000000) >> 24);
    lfsr2 = lfsr2 & 0xFFFFFF;

    return ((uint64_t) lfsr1 << 24) | lfsr2;

}

static uint16_t _generate_alias(uint64_t seed) {

    uint32_t lfsr2 = seed & 0xFFFFFF;
    uint32_t lfsr1 = (seed >> 24) & 0xFFFFFF;

    return (lfsr1 ^ lfsr2 ^ (lfsr1 >> 12) ^ (lfsr2 >> 12)) & 0x0FFF;

}

/* ====================================================================== */
/* CAN frame helpers                                                       */
/* ====================================================================== */

static void _setup_output_frame(void) {

    memset(&_can.output_frame, 0, sizeof(_can.output_frame));
    _can.output_frame_full = 1;
    _can.tx_timeout_tick = BootloaderDriver_timer_tick();

}

static void _set_node_id_payload(bootloader_can_frame_t *frame) {

    uint64_t node_id = BootloaderDriver_node_id();

    frame->data[0] = (node_id >> 40) & 0xFF;
    frame->data[1] = (node_id >> 32) & 0xFF;
    frame->data[2] = (node_id >> 24) & 0xFF;
    frame->data[3] = (node_id >> 16) & 0xFF;
    frame->data[4] = (node_id >> 8) & 0xFF;
    frame->data[5] = node_id & 0xFF;
    frame->can_dlc = 6;

}

static bool _is_node_id_match(const bootloader_can_frame_t *frame) {

    if (frame->can_dlc != 6) { return false; }

    uint64_t node_id = BootloaderDriver_node_id();

    for (int i = 5; i >= 0; i--) {

        if (frame->data[i] != (node_id & 0xFF)) { return false; }
        node_id >>= 8;

    }

    return true;

}

static void _set_global_frame(uint16_t mti) {

    _setup_output_frame();
    uint32_t id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT | CAN_OPENLCB_MSG |
            OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t) (mti & 0xFFF) << 12) | _can.alias;
    _can.output_frame.can_id = id;

}

static void _set_addressed_frame(uint16_t mti, uint16_t dest) {

    _set_global_frame(mti);
    _can.output_frame.data[0] = (dest >> 8) & 0x0F;
    _can.output_frame.data[1] = dest & 0xFF;
    _can.output_frame.can_dlc = 2;

}

static uint16_t _get_src_alias(uint32_t can_id) {

    return can_id & 0xFFF;

}

/* ====================================================================== */
/* Init state machine                                                      */
/* ====================================================================== */

static void _handle_init(void) {

    uint64_t node_id = BootloaderDriver_node_id();

    switch (_can.init_state) {

        case INIT_PICK_ALIAS:

            if (!_can.alias) {

                _can.alias = BootloaderCanDriver_node_alias();

                if (!_can.alias) {

                    /* Initial seed = node ID, generate alias from it.
                     * Reject alias 0 and advance seed until valid. */
                    _can.seed = node_id;
                    _can.alias = _generate_alias(_can.seed);

                    while (!_can.alias) {

                        _can.seed = _generate_seed(_can.seed);
                        _can.alias = _generate_alias(_can.seed);

                    }

                }

            } else {

                /* Conflict retry — advance seed and try again. */
                do {

                    _can.seed = _generate_seed(_can.seed);
                    _can.alias = _generate_alias(_can.seed);

                } while (!_can.alias);

            }

            break;

        case INIT_SEND_CID_7:

            _setup_output_frame();
            _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                    RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 |
                    (((node_id >> 24) & 0xFFF000) | _can.alias);
            break;

        case INIT_SEND_CID_6:

            _setup_output_frame();
            _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                    RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 |
                    (((node_id >> 12) & 0xFFF000) | _can.alias);
            break;

        case INIT_SEND_CID_5:

            _setup_output_frame();
            _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                    RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 |
                    ((node_id & 0xFFF000) | _can.alias);
            break;

        case INIT_SEND_CID_4:

            _setup_output_frame();
            _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                    RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 |
                    (((node_id << 12) & 0xFFF000) | _can.alias);
            _can.wait_tick = BootloaderDriver_timer_tick();
            break;

        case INIT_WAIT_RID: {

            /* Spec §6.2.1: Wait at least 200 ms after CID4.
             * SysTick fires every 100 ms so elapsed > 2 means > 200 ms. */
            uint8_t elapsed = (uint8_t) (BootloaderDriver_timer_tick()
                    - _can.wait_tick);

            if (elapsed <= 2) { return; }

            break;

        }

        case INIT_SEND_RID:

            _setup_output_frame();
            _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                    RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | _can.alias;
            break;

        case INIT_SEND_AMD:

            _setup_output_frame();
            _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                    RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | _can.alias;
            _set_node_id_payload(&_can.output_frame);
            break;

        case INIT_SEND_INIT_COMPLETE:

            _set_global_frame(MTI_INITIALIZATION_COMPLETE);
            _set_node_id_payload(&_can.output_frame);
            break;

        case INIT_INITIALIZED:

            return;

    }

    _can.init_state = (_init_state_enum) (_can.init_state + 1);

}

/* ====================================================================== */
/* Incoming frame dispatch                                                 */
/* ====================================================================== */

static void _handle_input_frame(void) {

    uint32_t can_id = _can.input_frame.can_id & ~BOOTLOADER_CAN_EFF_FLAG;

    if (!(_can.input_frame.can_id & BOOTLOADER_CAN_EFF_FLAG)) {

        _can.input_frame_full = 0;
        return;

    }

    /* Check for RESERVED_TOP_BIT (bit 28). Per spec, ignore frames without it. */
    if (!(can_id & RESERVED_TOP_BIT)) {

        _can.input_frame_full = 0;
        return;

    }

    /* Alias conflict check — applies in ALL states, not just Permitted.
     * Spec §6.2.1: restart if any frame with our tentative alias arrives
     * before the reservation completes (during CID or 200 ms wait). */
    if ((can_id & 0xFFF) == _can.alias && _can.alias != 0) {

        uint32_t frame_type_field = can_id & MASK_CAN_FRAME_TYPE;
        bool is_cid = (!(can_id & CAN_OPENLCB_MSG)) &&
                (frame_type_field >= CAN_CONTROL_FRAME_CID4 &&
                 frame_type_field <= CAN_CONTROL_FRAME_CID7);

        if (_can.init_state == INIT_INITIALIZED) {

            /* Permitted state (§6.2.5). */
            if (is_cid) {

                /* CID for our alias — reply with RID. */
                if (_can.output_frame_full) { return; }
                _setup_output_frame();
                _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                        RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | _can.alias;

            } else {

                /* Hard conflict — send AMR, then re-init (§6.2.5). */
                if (_can.output_frame_full) { return; }
                _setup_output_frame();
                _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                        RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | _can.alias;
                _set_node_id_payload(&_can.output_frame);
                _can.init_state = INIT_PICK_ALIAS;

            }

        } else {

            /* Inhibited state — restart alias reservation (§6.2.1 line 77-79). */
            _can.init_state = INIT_PICK_ALIAS;

        }

        _can.input_frame_full = 0;
        return;

    }

    /* Control frames (bit 27 = 0). */
    if (!(can_id & CAN_OPENLCB_MSG)) {

        uint32_t control_field = can_id & MASK_CAN_VARIABLE_FIELD;

        /* Spec §6.2.3: A node in Inhibited state shall not reply to AME. */
        if (control_field == CAN_CONTROL_FRAME_AME &&
                _can.init_state == INIT_INITIALIZED &&
                (_can.input_frame.can_dlc == 0 || _is_node_id_match(&_can.input_frame))) {

            if (_can.output_frame_full) { return; }
            _setup_output_frame();
            _can.output_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
                    RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | _can.alias;
            _set_node_id_payload(&_can.output_frame);

        }

        _can.input_frame_full = 0;
        return;

    }

    /* OpenLCB messages (bit 27 = 1). */
    uint32_t frame_type = can_id & MASK_CAN_FRAME_TYPE;

    /* Datagram frames. */
    if (frame_type == CAN_FRAME_TYPE_DATAGRAM_ONLY ||
            frame_type == CAN_FRAME_TYPE_DATAGRAM_FIRST) {

        uint16_t dest = ((can_id >> 12) & 0xFFF);

        if (dest != _can.alias) {

            _can.input_frame_full = 0;
            return;

        }

        uint16_t src = _get_src_alias(can_id);

        _can.datagram_rx_src = src;
        _can.datagram_rx_len = 0;

        memcpy(_can.datagram_rx_buffer, _can.input_frame.data, _can.input_frame.can_dlc);
        _can.datagram_rx_len = _can.input_frame.can_dlc;

        if (frame_type == CAN_FRAME_TYPE_DATAGRAM_ONLY) {

            _can.input_frame_full = 0;
            BootloaderProtocol_on_datagram_received(
                    _can.datagram_rx_src,
                    _can.datagram_rx_buffer,
                    _can.datagram_rx_len);
            return;

        }

        _can.incoming_datagram_pending = 1;
        _can.input_frame_full = 0;
        return;

    }

    if (frame_type == CAN_FRAME_TYPE_DATAGRAM_MIDDLE ||
            frame_type == CAN_FRAME_TYPE_DATAGRAM_FINAL) {

        uint16_t dest = ((can_id >> 12) & 0xFFF);

        if (dest != _can.alias) {

            _can.input_frame_full = 0;
            return;

        }

        if (!_can.incoming_datagram_pending ||
                _get_src_alias(can_id) != _can.datagram_rx_src) {

            _can.input_frame_full = 0;
            return;

        }

        uint8_t copy_len = _can.input_frame.can_dlc;

        if (_can.datagram_rx_len + copy_len > 72) {

            copy_len = 72 - _can.datagram_rx_len;

        }

        memcpy(_can.datagram_rx_buffer + _can.datagram_rx_len,
                _can.input_frame.data, copy_len);
        _can.datagram_rx_len += copy_len;

        if (frame_type == CAN_FRAME_TYPE_DATAGRAM_FINAL) {

            _can.incoming_datagram_pending = 0;
            _can.input_frame_full = 0;
            BootloaderProtocol_on_datagram_received(
                    _can.datagram_rx_src,
                    _can.datagram_rx_buffer,
                    _can.datagram_rx_len);
            return;

        }

        _can.input_frame_full = 0;
        return;

    }

    /* Standard OpenLCB message (global or addressed). */
    if (frame_type == OPENLCB_MESSAGE_STANDARD_FRAME_TYPE) {

        uint16_t mti_raw = (can_id >> 12) & 0xFFF;
        uint16_t src = _get_src_alias(can_id);
        bool is_addressed = (mti_raw & 0x0008) != 0;

        if (is_addressed) {

            uint16_t dest_alias = ((_can.input_frame.data[0] & 0x0F) << 8) |
                    _can.input_frame.data[1];

            if (dest_alias != _can.alias) {

                _can.input_frame_full = 0;
                return;

            }

            BootloaderProtocol_on_addressed_message(
                    mti_raw, src,
                    _can.input_frame.data + 2,
                    (_can.input_frame.can_dlc > 2) ? _can.input_frame.can_dlc - 2 : 0);

        } else {

            BootloaderProtocol_on_global_message(
                    mti_raw, _can.input_frame.data, _can.input_frame.can_dlc);

        }

    }

    _can.input_frame_full = 0;

}

/* ====================================================================== */
/* Outgoing datagram sender                                                */
/* ====================================================================== */

static void _handle_send_datagram(void) {

    _setup_output_frame();

    uint32_t frame_type;

    if (!_can.datagram_offset) {

        if (_can.datagram_dlc <= 8) {

            frame_type = CAN_FRAME_TYPE_DATAGRAM_ONLY;

        } else {

            frame_type = CAN_FRAME_TYPE_DATAGRAM_FIRST;

        }

    } else if (_can.datagram_dlc - _can.datagram_offset <= 8) {

        frame_type = CAN_FRAME_TYPE_DATAGRAM_FINAL;

    } else {

        frame_type = CAN_FRAME_TYPE_DATAGRAM_MIDDLE;

    }

    uint32_t id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT | CAN_OPENLCB_MSG |
            frame_type |
            ((uint32_t) _can.datagram_dst << 12) |
            _can.alias;
    _can.output_frame.can_id = id;

    uint8_t len = _can.datagram_dlc - _can.datagram_offset;

    if (len > 8) { len = 8; }

    memcpy(_can.output_frame.data, &_can.datagram_payload[_can.datagram_offset], len);
    _can.datagram_offset += len;
    _can.output_frame.can_dlc = len;

    if (_can.datagram_offset >= _can.datagram_dlc) {

        _can.datagram_reply_waiting = 1;

    }

}

/* ====================================================================== */
/* Transport interface implementation                                      */
/* ====================================================================== */

uint16_t BootloaderCan_get_alias(void) {

    return _can.alias;

}

void BootloaderTransport_init(void) {

    memset(&_can, 0, sizeof(_can));

}

bool BootloaderTransport_is_initialized(void) {

    return _can.init_state == INIT_INITIALIZED;

}

void BootloaderTransport_loop(void) {

    /* Poll CAN RX. */
    if (!_can.input_frame_full &&
            BootloaderCanDriver_read_can_frame(&_can.input_frame)) {

        _can.input_frame_full = 1;

    }

    /* Try to send pending output frame.
     * Spec §6.2.1 line 79-80: restart if TX error during reservation.
     * Use a 3-second timeout — if frame can't be sent, bus is stuck. */
    if (_can.output_frame_full) {

        if (BootloaderCanDriver_try_send_can_frame(&_can.output_frame)) {

            _can.output_frame_full = 0;

        } else if (_can.init_state != INIT_INITIALIZED) {

            uint8_t elapsed = (uint8_t) (BootloaderDriver_timer_tick()
                    - _can.tx_timeout_tick);

            if (elapsed > 30) {

                /* 3 seconds with no TX — restart alias reservation. */
                _can.output_frame_full = 0;
                _can.init_state = INIT_PICK_ALIAS;

            }

        }

    }

    /* Process incoming frame. */
    if (_can.input_frame_full) {

        _handle_input_frame();

    }

    /* Run init state machine. */
    if (_can.init_state != INIT_INITIALIZED && !_can.output_frame_full) {

        _handle_init();

    }

    /* Send outgoing datagram frames. */
    if (_can.datagram_output_pending && !_can.datagram_reply_waiting &&
            !_can.output_frame_full) {

        _handle_send_datagram();

    }

}

bool BootloaderTransport_send_addressed(
        uint16_t mti,
        uint16_t dest,
        const uint8_t *data,
        uint8_t len) {

    if (_can.output_frame_full) { return false; }

    _set_addressed_frame(mti, dest);

    /* Single frame — fits in one CAN frame. */
    if (len <= 6) {

        _can.output_frame.data[0] |= MULTIFRAME_ONLY;

        if (data && len > 0) {

            memcpy(_can.output_frame.data + 2, data, len);

        }

        _can.output_frame.can_dlc = 2 + len;
        return true;

    }

    /* First frame. */
    _can.output_frame.data[0] |= MULTIFRAME_FIRST;
    memcpy(_can.output_frame.data + 2, data, 6);
    _can.output_frame.can_dlc = 8;

    uint8_t offset = 6;

    /* Block-send first frame. */
    while (!BootloaderCanDriver_try_send_can_frame(&_can.output_frame)) {}

    /* Middle and last frames. */
    while (offset < len) {

        _set_addressed_frame(mti, dest);

        uint8_t remaining = len - offset;
        uint8_t chunk = (remaining > 6) ? 6 : remaining;

        _can.output_frame.data[0] |= (remaining > 6)
                ? MULTIFRAME_MIDDLE : MULTIFRAME_FINAL;

        memcpy(_can.output_frame.data + 2, data + offset, chunk);
        _can.output_frame.can_dlc = 2 + chunk;
        offset += chunk;

        /* Block-send all but the last frame; last goes through normal TX path. */
        if (offset < len) {

            while (!BootloaderCanDriver_try_send_can_frame(&_can.output_frame)) {}

        }

    }

    return true;

}

bool BootloaderTransport_send_global(
        uint16_t mti,
        const uint8_t *data,
        uint8_t len) {

    if (_can.output_frame_full) { return false; }

    _set_global_frame(mti);

    uint8_t copy_len = len;

    if (copy_len > 8) { copy_len = 8; }

    if (data && copy_len > 0) {

        memcpy(_can.output_frame.data, data, copy_len);
        _can.output_frame.can_dlc = copy_len;

    }

    return true;

}

bool BootloaderTransport_send_datagram_ok(uint16_t dest) {

    if (_can.output_frame_full) { return false; }

    _set_addressed_frame(MTI_DATAGRAM_OK_REPLY, dest);
    _can.output_frame.data[2] = DATAGRAM_OK_REPLY_PENDING;
    _can.output_frame.can_dlc = 3;

    return true;

}

bool BootloaderTransport_send_datagram_rejected(uint16_t dest, uint16_t error_code) {

    if (_can.output_frame_full) { return false; }

    _set_addressed_frame(MTI_DATAGRAM_REJECTED_REPLY, dest);
    _can.output_frame.data[2] = (error_code >> 8) & 0xFF;
    _can.output_frame.data[3] = error_code & 0xFF;
    _can.output_frame.can_dlc = 4;

    return true;

}

bool BootloaderTransport_send_datagram(
        uint16_t dest,
        const uint8_t *data,
        uint8_t len) {

    if (_can.datagram_output_pending) { return false; }

    _can.datagram_dst = dest;
    _can.datagram_dlc = len;
    _can.datagram_offset = 0;
    _can.datagram_reply_waiting = 0;
    memcpy(_can.datagram_payload, data, len);
    _can.datagram_output_pending = 1;

    return true;

}
