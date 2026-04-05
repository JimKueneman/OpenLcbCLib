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
 * @file tcp_login_statemachine.c
 * @brief Implementation of the TCP/IP login state machine.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_login_statemachine.h"
#include "../../openlcb/openlcb_defines.h"

#include <string.h>

// =========================================================================
// Module state
// =========================================================================

static const interface_tcp_login_statemachine_t *_interface;

static tcp_login_state_enum _login_state;

// =========================================================================
// Public API
// =========================================================================

void TcpLoginStatemachine_initialize(const interface_tcp_login_statemachine_t *interface) {

    _interface = interface;
    _login_state = TCP_LOGIN_IDLE;
}

void TcpLoginStatemachine_link_up(void) {

    _login_state = TCP_LOGIN_SEND_VERIFY_GLOBAL;

    // The protocol layer's OpenLCB login statemachine handles the rest:
    // - Initialization Complete for each local node
    // - Producer/Consumer Identified for declared events
    // - Transition to RUNSTATE_RUN
    //
    // For TCP, nodes enter the login statemachine immediately (no alias
    // negotiation delay like CAN).
    //
    // The actual Verify Node ID Global send happens in
    // TcpLoginStatemachine_run(), which retries until the transport accepts it.
}

bool TcpLoginStatemachine_run(void) {

    if (_login_state != TCP_LOGIN_SEND_VERIFY_GLOBAL) {

        return false;

    }

    // Send Verify Node ID Global to discover remote nodes
    _interface->lock_shared_resources();
    openlcb_msg_t *msg = _interface->allocate_buffer(BASIC);
    _interface->unlock_shared_resources();

    if (!msg) {

        return true; // allocation failed, retry next cycle

    }

    msg->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    msg->source_id = 0;
    msg->dest_id = 0;
    msg->source_alias = 0;
    msg->dest_alias = 0;
    msg->payload_count = 0;

    bool sent = _interface->send_openlcb_msg(msg);

    _interface->lock_shared_resources();
    _interface->free_buffer(msg);
    _interface->unlock_shared_resources();

    if (sent) {

        _login_state = TCP_LOGIN_COMPLETE;
        return false; // done

    }

    return true; // send failed, retry next cycle

}

tcp_login_state_enum TcpLoginStatemachine_get_state(void) {

    return _login_state;
}
