/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file can_rx_statemachine.h
 * @brief State machine that decodes and routes incoming CAN frames.
 *
 * @details Identifies frame category (OpenLCB message vs CAN control frame),
 * validates destination aliases, extracts framing bits, and dispatches to the
 * appropriate handler in @ref interface_can_rx_statemachine_t.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

    /**
     * @brief Dependency-injection interface for the CAN receive state machine.
     *
     * @details Provides 12 REQUIRED frame-handler callbacks plus 1 REQUIRED alias lookup
     * and 1 OPTIONAL receive-notification callback.  All REQUIRED pointers must be non-NULL.
     *
     * Frame dispatch rules:
     * - CAN control frames  → handle_cid/rid/amd/ame/amr/error_info_report
     * - OpenLCB single      → handle_single_frame  (framing bits = ONLY or absent)
     * - OpenLCB first       → handle_first_frame   (framing bits = FIRST)
     * - OpenLCB middle      → handle_middle_frame  (framing bits = MIDDLE)
     * - OpenLCB last        → handle_last_frame    (framing bits = LAST)
     * - Legacy SNIP         → handle_can_legacy_snip (MTI_SNIP_REPLY, no framing bits)
     * - Stream              → handle_stream_frame
     *
     * @see CanRxStatemachine_initialize
     */
typedef struct {

    /** @brief REQUIRED. Legacy SNIP handler (NULL-count based completion). Typical: CanRxMessageHandler_can_legacy_snip. */
    void (*handle_can_legacy_snip)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

    /** @brief REQUIRED. Single-frame OpenLCB message handler. Typical: CanRxMessageHandler_single_frame. */
    void (*handle_single_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

    /** @brief REQUIRED. First frame of a multi-frame message. Typical: CanRxMessageHandler_first_frame. */
    void (*handle_first_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

    /** @brief REQUIRED. Middle frame of a multi-frame message. Typical: CanRxMessageHandler_middle_frame. */
    void (*handle_middle_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index);

    /** @brief REQUIRED. Last frame of a multi-frame message. Typical: CanRxMessageHandler_last_frame. */
    void (*handle_last_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index);

    /** @brief REQUIRED. Stream frame handler (placeholder). Typical: CanRxMessageHandler_stream_frame. */
    void (*handle_stream_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

    /** @brief REQUIRED. RID (Reserve ID) control frame. Typical: CanRxMessageHandler_rid_frame. */
    void (*handle_rid_frame)(can_msg_t *can_msg);

    /** @brief REQUIRED. AMD (Alias Map Definition) control frame. Typical: CanRxMessageHandler_amd_frame. */
    void (*handle_amd_frame)(can_msg_t *can_msg);

    /** @brief REQUIRED. AME (Alias Map Enquiry) control frame. Typical: CanRxMessageHandler_ame_frame. */
    void (*handle_ame_frame)(can_msg_t *can_msg);

    /** @brief REQUIRED. AMR (Alias Map Reset) control frame. Typical: CanRxMessageHandler_amr_frame. */
    void (*handle_amr_frame)(can_msg_t *can_msg);

    /** @brief REQUIRED. Error Information Report control frame. Typical: CanRxMessageHandler_error_info_report_frame. */
    void (*handle_error_info_report_frame)(can_msg_t *can_msg);

    /** @brief REQUIRED. CID (Check ID) control frame. Typical: CanRxMessageHandler_cid_frame. */
    void (*handle_cid_frame)(can_msg_t *can_msg);

    /** @brief REQUIRED. Find alias mapping by 12-bit alias (validates addressed-message destination). Typical: AliasMappings_find_mapping_by_alias. */
    alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

    /** @brief OPTIONAL. Called immediately when a frame arrives, before any routing. Good for counters/LEDs. May be NULL. */
    void (*on_receive)(can_msg_t *can_msg);

} interface_can_rx_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Registers the dependency-injection interface for this module.
     *
     * @param interface_can_rx_statemachine Pointer to a populated
     *        @ref interface_can_rx_statemachine_t. Must remain valid for the
     *        lifetime of the application. All REQUIRED pointers must be non-NULL.
     *
     * @warning NOT thread-safe - call during single-threaded initialization only.
     * @warning Must be called before any CAN frames arrive.
     *
     * @see CanRxMessageHandler_initialize - initialize first
     * @see CanRxStatemachine_incoming_can_driver_callback
     */
    extern void CanRxStatemachine_initialize(const interface_can_rx_statemachine_t *interface_can_rx_statemachine);

    /**
     * @brief Primary entry point called by the hardware CAN driver on frame reception.
     *
     * @details Invokes the optional on_receive callback, then classifies the frame as
     * an OpenLCB message or a CAN control frame and dispatches accordingly.
     *
     * This function is typically called from an interrupt or receive thread and accesses
     * shared resources (FIFOs, buffer lists).  It must NOT be called while the main state
     * machine has resources locked.  Recommended approaches:
     * - Interrupt: disable CAN Rx interrupt during lock_shared_resources / unlock_shared_resources.
     * - Thread: suspend Rx thread or queue frames during the lock window.
     *
     * @param can_msg  Pointer to the received CAN frame. Must not be NULL. Must remain
     *                 valid until this function returns.
     *
     * @warning NOT thread-safe with the main state machine resource lock.
     *
     * @see CanMainStatemachine_run - holds the resource lock briefly
     * @see interface_can_rx_statemachine_t::on_receive
     */
    extern void CanRxStatemachine_incoming_can_driver_callback(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__ */
