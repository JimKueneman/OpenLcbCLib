/** \copyright
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
 * @file tcp_link_control.c
 * @brief Implementation of the TCP/IP link control message handler.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_link_control.h"
#include "tcp_utilities.h"

// =========================================================================
// Module state
// =========================================================================

static const interface_tcp_link_control_t *_interface;

// =========================================================================
// Internal helpers
// =========================================================================

/**
 * @brief Extracts the 16-bit link control type from the body.
 */
static uint16_t _decode_link_control_type(const uint8_t *data, uint16_t len) {

    if (len < 2)
        return 0xFFFF;

    return ((uint16_t) data[0] << 8) | (uint16_t) data[1];
}

/**
 * @brief Sets the pending Status Reply flag in the statemachine info.
 */
static void _handle_status_request(void) {

    tcp_statemachine_info_t *info = _interface->get_statemachine_info();

    if (info) {

        info->pending_status_reply = 1;

    }

}

/**
 * @brief Sets the pending Drop Link Reply flag in the statemachine info.
 *
 * @details The on_link_drop_requested callback is NOT fired here.
 * It fires in TcpLinkControl_run() after the reply has been sent.
 */
static void _handle_drop_link_request(void) {

    tcp_statemachine_info_t *info = _interface->get_statemachine_info();

    if (info) {

        info->pending_drop_reply = 1;

    }

}

// =========================================================================
// Public API
// =========================================================================

void TcpLinkControl_initialize(const interface_tcp_link_control_t *interface) {

    _interface = interface;
}

void TcpLinkControl_handle(uint16_t flags, const uint8_t *data, uint16_t len) {

    (void) flags;

    uint16_t type = _decode_link_control_type(data, len);

    switch (type) {

        case TCP_LINK_CONTROL_STATUS_REQUEST:
            _handle_status_request();
            break;

        case TCP_LINK_CONTROL_STATUS_REPLY:
            if (_interface->on_link_status_changed)
                _interface->on_link_status_changed(true);
            break;

        case TCP_LINK_CONTROL_DROP_REQUEST:
            _handle_drop_link_request();
            break;

        case TCP_LINK_CONTROL_DROP_REPLY:
            if (_interface->on_link_status_changed)
                _interface->on_link_status_changed(false);
            break;

        default:
            break;
    }
}

bool TcpLinkControl_run(tcp_statemachine_info_t *info) {

    if (!info) {

        return false;

    }

    if (info->pending_status_reply) {

        uint8_t reply_body[2];
        TcpUtilities_encode_uint16(reply_body, TCP_LINK_CONTROL_STATUS_REPLY);

        if (_interface->send_link_control(0x0000, reply_body, 2)) {

            info->pending_status_reply = 0;

        }

    }

    if (info->pending_drop_reply) {

        uint8_t reply_body[2];
        TcpUtilities_encode_uint16(reply_body, TCP_LINK_CONTROL_DROP_REPLY);

        if (_interface->send_link_control(0x0000, reply_body, 2)) {

            info->pending_drop_reply = 0;

            if (_interface->on_link_drop_requested) {

                _interface->on_link_drop_requested();

            }

        }

    }

    return info->pending_status_reply || info->pending_drop_reply;

}

bool TcpLinkControl_send_status_request(void) {

    uint8_t body[2];

    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_STATUS_REQUEST);

    return _interface->send_link_control(0x0000, body, 2);
}

bool TcpLinkControl_send_drop_link_request(void) {

    uint8_t body[2];

    TcpUtilities_encode_uint16(body, TCP_LINK_CONTROL_DROP_REQUEST);

    return _interface->send_link_control(0x0000, body, 2);
}
