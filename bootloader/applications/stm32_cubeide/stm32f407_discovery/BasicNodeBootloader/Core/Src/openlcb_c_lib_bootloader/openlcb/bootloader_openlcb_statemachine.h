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
 * @file bootloader_openlcb_statemachine.h
 *
 * OpenLCB protocol state machine for the standalone bootloader. Handles
 * PIP, SNIP, Verify Node ID, OIR, and Memory Configuration commands.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_OPENLCB_STATEMACHINE__
#define __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_OPENLCB_STATEMACHINE__

#include <stdint.h>
#include <stdbool.h>

#include "bootloader_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         *     Initializes the OpenLCB protocol state machine.
         *
         *     @param openlcb_driver hardware driver for flash ops, LED, node_id
         */
    void BootloaderOpenlcbSM_init(const bootloader_openlcb_driver_t *openlcb_driver);

        /**
         *     Called when a complete datagram is received.
         *
         *     @param src_alias      source node alias (0 on non-CAN)
         *     @param src_node_id    source node ID (0 on CAN)
         *     @param data           datagram payload bytes
         *     @param payload_length payload length (1-72)
         */
    void BootloaderOpenlcbSM_on_datagram_received(uint16_t src_alias, uint64_t src_node_id, const uint8_t *data, uint8_t payload_length);

        /**
         *     Called when an addressed message is received.
         *
         *     @param mti            message type indicator
         *     @param src_alias      source node alias (0 on non-CAN)
         *     @param src_node_id    source node ID (0 on CAN)
         *     @param data           payload bytes after address bytes
         *     @param payload_length payload length
         */
    void BootloaderOpenlcbSM_on_addressed_message(uint16_t mti, uint16_t src_alias, uint64_t src_node_id, const uint8_t *data, uint8_t payload_length);

        /**
         *     Called when a global message is received.
         *
         *     @param mti            message type indicator
         *     @param data           payload bytes
         *     @param payload_length payload length
         */
    void BootloaderOpenlcbSM_on_global_message(uint16_t mti, const uint8_t *data, uint8_t payload_length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_OPENLCB_STATEMACHINE__ */
