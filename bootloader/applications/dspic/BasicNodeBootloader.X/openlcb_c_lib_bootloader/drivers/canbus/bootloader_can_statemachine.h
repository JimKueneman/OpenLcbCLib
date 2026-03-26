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
 * @file bootloader_can_statemachine.h
 *
 * CAN state machine for the standalone bootloader. Handles alias reservation,
 * collision response, AME handling, datagram assembly, frame dispatch to the
 * OpenLCB state machine, and send helpers for outgoing messages.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN_STATEMACHINE__
#define __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN_STATEMACHINE__

#include <stdint.h>
#include <stdbool.h>

#include "../../openlcb/bootloader_types.h"
#include "bootloader_can.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     *     Initializes the CAN state machine, RX engine, and TX engine.
     *
     *     @param can_driver CAN hardware driver (forwarded to RX/TX)
     *     @param openlcb_driver hardware driver (for timer_tick, node_id)
     */
    void BootloaderCanSM_init(const bootloader_can_driver_t *can_driver, const bootloader_openlcb_driver_t *openlcb_driver);

    /**
     *     Runs one iteration: poll RX, handle collisions, run alias
     *     state machine, dispatch received frames to protocol layer.
     */
    void BootloaderCanSM_loop(void);

    /**
     *     @return true after alias negotiation is complete
     */
    bool BootloaderCanSM_is_initialized(void);

    /**
     *     @return the current 12-bit CAN alias
     */
    uint16_t BootloaderCanSM_alias(void);

    /**
     *     Sends an addressed message (single or multi-frame).
     *
     *     @param mti message type indicator
     *     @param dest_alias destination CAN alias
     *     @param dest_node_id destination node ID (0 on CAN)
     *     @param data payload bytes (may be NULL if len is 0)
     *     @param len number of payload bytes
     *     @return true if sent
     */
    bool BootloaderCanSM_send_addressed(uint16_t mti, uint16_t dest_alias, uint64_t dest_node_id, const uint8_t *data, uint8_t len);

    /**
     *     Sends a global (unaddressed) message.
     *
     *     @param mti message type indicator
     *     @param data payload bytes (may be NULL if len is 0)
     *     @param len number of payload bytes
     *     @return true if sent
     */
    bool BootloaderCanSM_send_global(uint16_t mti, const uint8_t *data, uint8_t len);

    /**
     *     Sends a Datagram Received OK reply.
     *
     *     @param dest_alias destination CAN alias
     *     @param dest_node_id destination node ID (0 on CAN)
     *     @return true if sent
     */
    bool BootloaderCanSM_send_datagram_ok(uint16_t dest_alias, uint64_t dest_node_id);

    /**
     *     Sends a Datagram Rejected reply.
     *
     *     @param dest_alias destination CAN alias
     *     @param dest_node_id destination node ID (0 on CAN)
     *     @param error_code OpenLCB error code
     *     @return true if sent
     */
    bool BootloaderCanSM_send_datagram_rejected(uint16_t dest_alias, uint64_t dest_node_id, uint16_t error_code);

    /**
     *     Sends a response datagram (Options reply, Address Space Info reply).
     *
     *     @param dest_alias destination CAN alias
     *     @param dest_node_id destination node ID (0 on CAN)
     *     @param data datagram payload
     *     @param len payload length (max 72)
     *     @return true if sent
     */
    bool BootloaderCanSM_send_datagram(uint16_t dest_alias, uint64_t dest_node_id, const uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN_STATEMACHINE__ */
