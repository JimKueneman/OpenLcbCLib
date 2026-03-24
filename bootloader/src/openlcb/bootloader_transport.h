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
 * @file bootloader_transport.h
 *
 * Transport layer interface for the standalone bootloader.
 * These extern functions are implemented by the active transport layer
 * (e.g., drivers/canbus/bootloader_can.c). The protocol layer calls these
 * to send messages without knowing the underlying transport.
 *
 * To swap transports, compile a different .c file that implements these
 * functions (e.g., a TCP transport, serial transport, etc.).
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_TRANSPORT__
#define __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_TRANSPORT__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     *     Initializes the transport layer.
     *     For CAN: begins alias negotiation.
     */
    extern void BootloaderTransport_init(void);

    /**
     *     Transport layer tick. Called once per main loop iteration.
     *     For CAN: polls RX, tries to send pending TX, runs alias state machine.
     *
     *     During this call the transport may invoke protocol-layer callbacks:
     *       - BootloaderProtocol_on_datagram_received()
     *       - BootloaderProtocol_on_addressed_message()
     *       - BootloaderProtocol_on_global_message()
     */
    extern void BootloaderTransport_loop(void);

    /**
     *     Checks if the transport is initialized and ready to send/receive.
     *     For CAN: returns true after alias negotiation is complete.
     *
     *     @return true if transport is ready
     */
    extern bool BootloaderTransport_is_initialized(void);

    /**
     *     Sends an addressed message.
     *
     *     @param mti message type indicator
     *     @param dest destination alias
     *     @param data payload bytes (may be NULL if len is 0)
     *     @param len number of payload bytes
     *     @return true if the message was queued/sent, false if busy
     */
    extern bool BootloaderTransport_send_addressed(
            uint16_t mti,
            uint16_t dest,
            const uint8_t *data,
            uint8_t len);

    /**
     *     Sends a global (unaddressed) message.
     *
     *     @param mti message type indicator
     *     @param data payload bytes (may be NULL if len is 0)
     *     @param len number of payload bytes
     *     @return true if the message was queued/sent, false if busy
     */
    extern bool BootloaderTransport_send_global(
            uint16_t mti,
            const uint8_t *data,
            uint8_t len);

    /**
     *     Sends a Datagram Received OK reply.
     *
     *     @param dest destination alias of the datagram sender
     *     @return true if sent, false if busy
     */
    extern bool BootloaderTransport_send_datagram_ok(uint16_t dest);

    /**
     *     Sends a Datagram Rejected reply with an error code.
     *
     *     @param dest destination alias of the datagram sender
     *     @param error_code OpenLCB error code
     *     @return true if sent, false if busy
     */
    extern bool BootloaderTransport_send_datagram_rejected(
            uint16_t dest,
            uint16_t error_code);

    /**
     *     Sends a response datagram (e.g., Get Options reply, Address Space Info reply).
     *
     *     @param dest destination alias
     *     @param data datagram payload
     *     @param len payload length (max 72)
     *     @return true if queued, false if busy
     */
    extern bool BootloaderTransport_send_datagram(
            uint16_t dest,
            const uint8_t *data,
            uint8_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_TRANSPORT__ */
