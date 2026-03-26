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
 * @file bootloader_can_statemachine.c
 *
 * CAN state machine for the standalone bootloader.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "bootloader_can_statemachine.h"
#include "bootloader_rx.h"
#include "bootloader_tx.h"
#include "../../openlcb/bootloader_openlcb_statemachine.h"

/* ====================================================================== */
/* Init state machine enum                                                 */
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
/* Internal state                                                          */
/* ====================================================================== */

typedef struct {

    const bootloader_can_driver_t *can_driver;
    const bootloader_openlcb_driver_t *openlcb_driver;

    bootloader_can_frame_t pending_frame;
    unsigned pending_frame_full : 1;

    uint16_t alias;
    uint64_t seed;
    _init_state_enum init_state;
    uint8_t wait_tick;
    uint8_t tx_timeout_tick;

} _can_sm_state_t;

static _can_sm_state_t _sm;

/* ====================================================================== */
/* Alias generation                                                        */
/* ====================================================================== */

static uint64_t _generate_seed(uint64_t start_seed) {

    uint32_t lfsr1 = (uint32_t) ((start_seed >> 24) & 0xFFFFFF);
    uint32_t lfsr2 = (uint32_t) (start_seed & 0xFFFFFF);

    lfsr1 = ((lfsr1 << 1) | ((lfsr1 >> 22) & 1)) ^ ((lfsr1 >> 1) & 1);
    lfsr1 &= 0xFFFFFF;
    lfsr1 ^= 0x1B0CA3L;

    lfsr2 = ((lfsr2 << 1) | ((lfsr2 >> 22) & 1)) ^ ((lfsr2 >> 1) & 1);
    lfsr2 &= 0xFFFFFF;
    lfsr2 ^= 0x7A4BA9L;

    return ((uint64_t) lfsr1 << 24) | (uint64_t) lfsr2;

}

static uint16_t _generate_alias(uint64_t seed) {

    uint32_t lfsr1 = (uint32_t) ((seed >> 24) & 0xFFFFFF);
    uint32_t lfsr2 = (uint32_t) (seed & 0xFFFFFF);

    return (uint16_t) ((lfsr1 ^ lfsr2 ^ (lfsr1 >> 12) ^ (lfsr2 >> 12)) & 0x0FFF);

}

/* ====================================================================== */
/* Frame construction helpers                                              */
/* ====================================================================== */

static void _set_node_id_payload(bootloader_can_frame_t *frame) {

    uint64_t node_id = _sm.openlcb_driver->get_persistent_node_id();

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

    uint64_t node_id = _sm.openlcb_driver->get_persistent_node_id();
    uint64_t received =
            ((uint64_t) frame->data[0] << 40) |
            ((uint64_t) frame->data[1] << 32) |
            ((uint64_t) frame->data[2] << 24) |
            ((uint64_t) frame->data[3] << 16) |
            ((uint64_t) frame->data[4] << 8) |
            (uint64_t) frame->data[5];

    return received == node_id;

}

static void _build_pending_control_frame(uint32_t control_field) {

    memset(&_sm.pending_frame, 0, sizeof(_sm.pending_frame));
    _sm.pending_frame.can_id = BOOTLOADER_CAN_EFF_FLAG |
            RESERVED_TOP_BIT | control_field | _sm.alias;
    _sm.pending_frame_full = 1;
    _sm.tx_timeout_tick = _sm.openlcb_driver->get_100ms_timer_tick();

}

static void _build_pending_global_frame(uint16_t mti) {

    memset(&_sm.pending_frame, 0, sizeof(_sm.pending_frame));
    _sm.pending_frame.can_id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_OPENLCB_MSG | OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t)(mti & 0xFFF) << 12) | _sm.alias;
    _sm.pending_frame_full = 1;
    _sm.tx_timeout_tick = _sm.openlcb_driver->get_100ms_timer_tick();

}


/* ====================================================================== */
/* Init state machine                                                      */
/* ====================================================================== */

static void _handle_init(void) {

    uint64_t node_id = _sm.openlcb_driver->get_persistent_node_id();

    switch (_sm.init_state) {

        case INIT_PICK_ALIAS:
        {

            uint16_t saved = _sm.can_driver->get_cached_alias();

            if (saved != 0) {

                /* The application was already logged in with this alias.
                 * Skip the full CID negotiation, 200ms wait, and AMD —
                 * the alias is still valid and mapped on the bus.  The CT
                 * that sent Freeze is waiting for Init Complete, so jump
                 * straight there.  (Sending AMD would confuse the CT.) */
                _sm.alias = saved;
                _sm.init_state = INIT_SEND_INIT_COMPLETE;

            } else {

                _sm.seed = node_id;

                do {

                    _sm.alias = _generate_alias(_sm.seed);
                    _sm.seed = _generate_seed(_sm.seed);

                } while (_sm.alias == 0);

                _sm.init_state = INIT_SEND_CID_7;

            }

            break;

        }

        case INIT_SEND_CID_7:

            _build_pending_control_frame(CAN_CONTROL_FRAME_CID7 | (((uint32_t)(node_id >> 24) & 0xFFF) << 12));
            _sm.init_state = INIT_SEND_CID_6;
            break;

        case INIT_SEND_CID_6:

            _build_pending_control_frame(CAN_CONTROL_FRAME_CID6 | (((uint32_t)(node_id >> 12) & 0xFFF) << 12));
            _sm.init_state = INIT_SEND_CID_5;
            break;

        case INIT_SEND_CID_5:

            _build_pending_control_frame(CAN_CONTROL_FRAME_CID5 | (((uint32_t) node_id & 0xFFF) << 12));
            _sm.init_state = INIT_SEND_CID_4;
            break;

        case INIT_SEND_CID_4:

            _build_pending_control_frame(CAN_CONTROL_FRAME_CID4 | (((uint32_t)(node_id << 12) & 0xFFF000)));
            _sm.wait_tick = _sm.openlcb_driver->get_100ms_timer_tick();
            _sm.init_state = INIT_WAIT_RID;
            break;

        case INIT_WAIT_RID:
        {

            uint8_t elapsed = (uint8_t) (_sm.openlcb_driver->get_100ms_timer_tick() - _sm.wait_tick);

            if (elapsed > 2) {

                _sm.init_state = INIT_SEND_RID;

            }

            break;

        }

        case INIT_SEND_RID:

            _build_pending_control_frame(CAN_CONTROL_FRAME_RID);
            _sm.init_state = INIT_SEND_AMD;
            break;

        case INIT_SEND_AMD:

            _build_pending_control_frame(CAN_CONTROL_FRAME_AMD);
            _set_node_id_payload(&_sm.pending_frame);
            _sm.init_state = INIT_SEND_INIT_COMPLETE;
            break;

        case INIT_SEND_INIT_COMPLETE:

            _build_pending_global_frame(MTI_INITIALIZATION_COMPLETE);
            _set_node_id_payload(&_sm.pending_frame);
            _sm.init_state = INIT_INITIALIZED;
            break;

        case INIT_INITIALIZED:

            break;

    }

}

/* ====================================================================== */
/* Frame dispatch                                                          */
/* ====================================================================== */

static void _dispatch_frame(const bootloader_can_frame_t *frame) {

    uint32_t can_id = frame->can_id & ~BOOTLOADER_CAN_EFF_FLAG;
    uint32_t frame_type = can_id & MASK_CAN_FRAME_TYPE;

    /* AME control frame — respond with AMD if node ID matches or no data. */
    if (!(can_id & CAN_OPENLCB_MSG)) {

        uint32_t control_field = can_id & MASK_CAN_VARIABLE_FIELD;

        if (control_field == CAN_CONTROL_FRAME_AME &&
                _sm.init_state == INIT_INITIALIZED &&
                (frame->can_dlc == 0 || _is_node_id_match(frame))) {

            _build_pending_control_frame(CAN_CONTROL_FRAME_AMD);
            _set_node_id_payload(&_sm.pending_frame);

        }

        return;

    }

    /* Datagram frames are assembled by the RX engine — they never
     * appear in the frame FIFO, so nothing to handle here. */

    /* Standard OpenLCB messages. */
    if (frame_type == OPENLCB_MESSAGE_STANDARD_FRAME_TYPE) {

        uint16_t mti_raw = (can_id >> 12) & 0xFFF;
        uint16_t src = can_id & 0xFFF;
        bool is_addressed = (mti_raw & 0x0008) != 0;

        if (is_addressed) {

            BootloaderOpenlcbSM_on_addressed_message(mti_raw, src, 0, frame->data + 2, (frame->can_dlc > 2) ? frame->can_dlc - 2 : 0);

        } else {

            BootloaderOpenlcbSM_on_global_message(mti_raw, frame->data, frame->can_dlc);

        }

    }

}

/* ====================================================================== */
/* Collision handling                                                       */
/* ====================================================================== */

static void _handle_collisions(void) {

    if (BootloaderRx_collision_cid()) {

        if (_sm.init_state == INIT_INITIALIZED) {

            /* CID for our alias — reply with RID. */
            _build_pending_control_frame(CAN_CONTROL_FRAME_RID);

        } else {

            /* Inhibited — restart alias reservation. */
            _sm.pending_frame_full = 0;
            _sm.seed = _generate_seed(_sm.seed);
            _sm.init_state = INIT_PICK_ALIAS;

        }

    }

    if (BootloaderRx_collision_non_cid()) {

        if (_sm.init_state == INIT_INITIALIZED) {

            /* Hard conflict — send AMR, then restart. */
            _build_pending_control_frame(CAN_CONTROL_FRAME_AMR);
            _set_node_id_payload(&_sm.pending_frame);
            _sm.init_state = INIT_PICK_ALIAS;

        } else {

            /* Inhibited — restart. */
            _sm.pending_frame_full = 0;
            _sm.seed = _generate_seed(_sm.seed);
            _sm.init_state = INIT_PICK_ALIAS;

        }

    }

    BootloaderRx_clear_collision_flags();

}

/* ====================================================================== */
/* Public API                                                              */
/* ====================================================================== */

void BootloaderCanSM_init(const bootloader_can_driver_t *can_driver, const bootloader_openlcb_driver_t *openlcb_driver) {

    memset(&_sm, 0, sizeof(_sm));
    _sm.can_driver = can_driver;
    _sm.openlcb_driver = openlcb_driver;

    BootloaderRx_init(can_driver);
    BootloaderTx_init(can_driver);

}

void BootloaderCanSM_loop(void) {

    /* Poll RX — assembles datagrams, flags errors, buffers other frames. */
    BootloaderRx_poll(_sm.alias, _sm.openlcb_driver->get_100ms_timer_tick());

    /* Handle alias collisions. */
    _handle_collisions();

    /* Try to send pending frame. */
    if (_sm.pending_frame_full) {

        if (BootloaderTx_send_frame(&_sm.pending_frame)) {

            _sm.pending_frame_full = 0;

        } else if (_sm.init_state != INIT_INITIALIZED) {

            uint8_t elapsed = (uint8_t) (_sm.openlcb_driver->get_100ms_timer_tick() - _sm.tx_timeout_tick);

            if (elapsed > 30) {

                _sm.pending_frame_full = 0;
                _sm.init_state = INIT_PICK_ALIAS;

            }

        }

    }

    /* Run init state machine. */
    if (_sm.init_state != INIT_INITIALIZED && !_sm.pending_frame_full) {

        _handle_init();

    }

    if (_sm.init_state != INIT_INITIALIZED) { return; }

    /* Check for datagram assembly errors — send rejection. */
    if (BootloaderRx_has_error()) {

        bootloader_rx_error_t err = BootloaderRx_get_error();
        BootloaderCanSM_send_datagram_rejected(err.error_src_alias, 0, err.error_code);

    }

    /* Check for completed datagram. */
    if (BootloaderRx_has_datagram()) {

        uint16_t src;
        uint8_t buf[72];
        uint8_t len;

        if (BootloaderRx_get_datagram(&src, buf, &len)) {

            BootloaderOpenlcbSM_on_datagram_received(src, 0, buf, len);

        }

    }

    /* Dispatch non-datagram frames. */
    bootloader_can_frame_t frame;

    while (BootloaderRx_get_frame(&frame)) {

        _dispatch_frame(&frame);

    }

}

bool BootloaderCanSM_is_initialized(void) {

    return _sm.init_state == INIT_INITIALIZED;

}

uint16_t BootloaderCanSM_alias(void) {

    return _sm.alias;

}

/* ====================================================================== */
/* Send helpers                                                            */
/* ====================================================================== */

bool BootloaderCanSM_send_addressed(uint16_t mti, uint16_t dest_alias, uint64_t dest_node_id, const uint8_t *data, uint8_t len) {

    (void) dest_node_id;

    return BootloaderTx_send_multiframe(mti, _sm.alias, dest_alias, _sm.alias, _sm.openlcb_driver->get_100ms_timer_tick(), data, len);

}

bool BootloaderCanSM_send_global(uint16_t mti, const uint8_t *data, uint8_t len) {

    return BootloaderTx_send_global(mti, _sm.alias, _sm.alias, _sm.openlcb_driver->get_100ms_timer_tick(), data, len);

}

bool BootloaderCanSM_send_datagram_ok(uint16_t dest_alias, uint64_t dest_node_id) {

    (void) dest_node_id;

    return BootloaderTx_send_datagram_ok(_sm.alias, dest_alias, _sm.alias, _sm.openlcb_driver->get_100ms_timer_tick(), DATAGRAM_OK_REPLY_PENDING);

}

bool BootloaderCanSM_send_datagram_rejected(uint16_t dest_alias, uint64_t dest_node_id, uint16_t error_code) {

    (void) dest_node_id;

    return BootloaderTx_send_datagram_rejected(_sm.alias, dest_alias, _sm.alias, _sm.openlcb_driver->get_100ms_timer_tick(), error_code);

}

bool BootloaderCanSM_send_datagram(uint16_t dest_alias, uint64_t dest_node_id, const uint8_t *data, uint8_t len) {

    (void) dest_node_id;

    return BootloaderTx_send_datagram(_sm.alias, dest_alias, _sm.alias, _sm.openlcb_driver->get_100ms_timer_tick(), data, len);

}
