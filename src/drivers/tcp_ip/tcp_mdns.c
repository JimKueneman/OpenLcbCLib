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
 * @file tcp_mdns.c
 * @brief Implementation of the mDNS discovery module for OpenLCB TCP/IP.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#include "tcp_mdns.h"

// =========================================================================
// Module state
// =========================================================================

static const tcp_mdns_config_t *_config;

// =========================================================================
// Public API
// =========================================================================

void TcpMdns_initialize(const tcp_mdns_config_t *config) {

    _config = config;
}

bool TcpMdns_discover_hub(tcp_mdns_result_callback_t callback) {

    if (!_config)
        return false;

    return _config->mdns_query(TCP_MDNS_HUB_SERVICE_TYPE, callback);
}

bool TcpMdns_advertise_hub(const char *name, uint16_t port) {

    if (!_config)
        return false;

    return _config->mdns_register(TCP_MDNS_HUB_SERVICE_TYPE, name, port);
}

bool TcpMdns_advertise_node(const char *name, uint16_t port) {

    if (!_config)
        return false;

    return _config->mdns_register(TCP_MDNS_NODE_SERVICE_TYPE, name, port);
}

void TcpMdns_stop_advertising(const char *name) {

    if (!_config)
        return;

    _config->mdns_unregister(name);
}
