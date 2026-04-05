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
 * @file tcp_mdns.h
 * @brief mDNS/Bonjour service discovery for OpenLCB TCP/IP transport.
 *
 * @details Discovers OpenLCB hubs via "_openlcb-hub._tcp.local." and
 * advertises this node as either a hub or spoke.  Uses a platform-specific
 * DI interface for the actual mDNS backend (ESP32 mDNS, Avahi, Bonjour, etc.).
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_MDNS__
#define __DRIVERS_TCP_IP_TCP_MDNS__

#include <stdbool.h>
#include <stdint.h>

#include "tcp_types.h"

/** @brief OpenLCB hub service type for mDNS discovery. */
#define TCP_MDNS_HUB_SERVICE_TYPE     "_openlcb-hub._tcp.local."

/** @brief OpenLCB node service type for mDNS discovery. */
#define TCP_MDNS_NODE_SERVICE_TYPE    "_openlcb-node._tcp.local."

/**
 * @brief mDNS discovery result.
 */
typedef struct {

    char hostname[64];   /**< @brief Resolved hostname or IP address. */
    uint16_t port;       /**< @brief Resolved port number. */
    node_id_t node_id;   /**< @brief Node ID extracted from service name (0 if not available). */

} tcp_mdns_result_t;

/**
 * @brief Callback for mDNS discovery results.
 *
 * @param result  Pointer to the discovery result.  Valid only during the callback.
 */
typedef void (*tcp_mdns_result_callback_t)(const tcp_mdns_result_t *result);

/**
 * @brief Platform-specific mDNS backend interface.
 *
 * @details Users provide their platform's mDNS implementation via these
 * function pointers.  All are REQUIRED.
 *
 * Example implementations:
 * - ESP32: esp_mdns_query_ptr / mdns_service_add
 * - Linux: avahi_client / avahi_service_browser
 * - macOS: DNSServiceBrowse / DNSServiceRegister
 */
typedef struct {

    /** @brief Start an mDNS query for the given service type.
     *  Results are delivered via the callback. REQUIRED. */
    bool (*mdns_query)(const char *service_type, tcp_mdns_result_callback_t callback);

    /** @brief Register an mDNS service. REQUIRED. */
    bool (*mdns_register)(const char *service_type, const char *name, uint16_t port);

    /** @brief Unregister a previously registered mDNS service. REQUIRED. */
    void (*mdns_unregister)(const char *name);

} tcp_mdns_config_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the mDNS discovery module.
     *
     * @param config  Pointer to platform-specific mDNS backend.
     *                Must remain valid for the lifetime of the application.
     */
    extern void TcpMdns_initialize(const tcp_mdns_config_t *config);

    /**
     * @brief Queries for OpenLCB hub services on the local network.
     *
     * @details Starts an mDNS query for "_openlcb-hub._tcp.local.".
     * Results are delivered asynchronously via the callback.
     *
     * @param callback  Function called for each discovered hub.
     *
     * @return true if the query was started, false on error.
     */
    extern bool TcpMdns_discover_hub(tcp_mdns_result_callback_t callback);

    /**
     * @brief Advertises this device as an OpenLCB hub.
     *
     * @param name  Service instance name (e.g. "MyHub").
     * @param port  TCP port number to advertise.
     *
     * @return true on success, false on error.
     */
    extern bool TcpMdns_advertise_hub(const char *name, uint16_t port);

    /**
     * @brief Advertises this device as an OpenLCB node.
     *
     * @param name  Service instance name (e.g. "MyNode").
     * @param port  TCP port number to advertise.
     *
     * @return true on success, false on error.
     */
    extern bool TcpMdns_advertise_node(const char *name, uint16_t port);

    /**
     * @brief Stops advertising a previously registered service.
     *
     * @param name  Service instance name to unregister.
     */
    extern void TcpMdns_stop_advertising(const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_MDNS__ */
