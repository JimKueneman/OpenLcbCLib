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
 * @file bootloader_protocol.h
 *
 * Protocol message handlers for the standalone bootloader.
 * These are callbacks invoked by the transport layer when complete
 * messages arrive, and are called directly (not via extern/DI).
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_PROTOCOL__
#define __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_PROTOCOL__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     *     Called by the transport layer when a complete datagram is received.
     *
     *     @param src_alias source node alias
     *     @param data datagram payload bytes
     *     @param len payload length (1-72)
     */
    void BootloaderProtocol_on_datagram_received(
            uint16_t src_alias,
            const uint8_t *data,
            uint8_t len);

    /**
     *     Called by the transport layer when an addressed message is received.
     *
     *     @param mti message type indicator
     *     @param src_alias source node alias
     *     @param data payload bytes after the address bytes (may be NULL)
     *     @param len payload length
     */
    void BootloaderProtocol_on_addressed_message(
            uint16_t mti,
            uint16_t src_alias,
            const uint8_t *data,
            uint8_t len);

    /**
     *     Called by the transport layer when a global message is received.
     *
     *     @param mti message type indicator
     *     @param data payload bytes (may be NULL)
     *     @param len payload length
     */
    void BootloaderProtocol_on_global_message(
            uint16_t mti,
            const uint8_t *data,
            uint8_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_PROTOCOL__ */
