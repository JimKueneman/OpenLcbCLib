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
 * @brief State machine for receiving and decoding incoming CAN frames
 *
 * @details This module implements the CAN receive state machine that processes raw CAN
 * frames from the hardware driver and routes them to appropriate handlers. It decodes
 * the CAN frame format, extracts MTI and addressing information, and dispatches to
 * message handlers based on frame type.
 *
 * The state machine handles:
 * - CAN control frames (CID, RID, AMD, AME, AMR, error reports)
 * - OpenLCB message frames (global, addressed, datagram, stream)
 * - Multi-frame message assembly coordination
 * - Framing bit extraction and validation
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

/**
 * @brief Dependency injection interface for CAN RX state machine module
 *
 * @details OpenLcbCLib uses dependency injection to enable comprehensive testing
 * and reduce code size. Functions can be mocked in tests to validate all code paths.
 * Setting handlers to NULL strips out unused protocol code, useful for bootloaders
 * and resource-constrained applications.
 *
 * Use cases:
 * - Unit testing with mock implementations
 * - Reducing code size by excluding unused protocols
 * - Platform-specific customization
 * - Adding application-specific callbacks
 *
 * @warning All function pointers marked as Required must be initialized
 * @warning Optional callbacks may be NULL
 *
 * @see CanRxStatemachine_initialize - Stores this interface
 */
typedef struct
{

    /*@{*/

    // REQUIRED FUNCTIONS

    /**
     * @brief Pointer to function for handling legacy SNIP CAN frames
     *
     * @details Processes SNIP messages from early OpenLCB implementations without
     * standard framing bits. Uses NULL byte counting to detect message completion.
     *
     * @param can_msg Pointer to received CAN frame
     * @param can_buffer_start_index Offset where SNIP data begins in payload
     * @param data_type Buffer type needed (must be SNIP)
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_can_legacy_snip()
     *
     * @see CanRxMessageHandler_can_legacy_snip - Standard implementation
     */
    void (*handle_can_legacy_snip)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);


    /**
     * @brief Pointer to function for handling single-frame CAN messages
     *
     * @details Processes CAN frames containing complete OpenLCB messages that fit
     * in one frame. Allocates buffer, copies data, and pushes to OpenLCB FIFO.
     *
     * @param can_msg Pointer to received CAN frame
     * @param can_buffer_start_index Offset where OpenLCB data begins in payload
     * @param data_type Buffer type needed (typically BASIC)
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_single_frame()
     *
     * @see CanRxMessageHandler_single_frame - Standard implementation
     */
    void (*handle_single_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);


    /**
     * @brief Pointer to function for handling first frame of multi-frame messages
     *
     * @details Processes initial frame of multi-frame sequence. Allocates OpenLCB
     * buffer, initializes message header, copies first payload chunk, adds to buffer list.
     *
     * @param can_msg Pointer to received CAN frame
     * @param can_buffer_start_index Offset where OpenLCB data begins in payload
     * @param data_type Buffer type needed (BASIC, DATAGRAM, SNIP, STREAM)
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_first_frame()
     *
     * @see CanRxMessageHandler_first_frame - Standard implementation
     */
    void (*handle_first_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);


    /**
     * @brief Pointer to function for handling middle frames of multi-frame messages
     *
     * @details Processes continuation frames in multi-frame sequence. Finds in-progress
     * message in buffer list and appends payload data.
     *
     * @param can_msg Pointer to received CAN frame
     * @param can_buffer_start_index Offset where OpenLCB data begins in payload
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_middle_frame()
     *
     * @see CanRxMessageHandler_middle_frame - Standard implementation
     */
    void (*handle_middle_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index);

    /**
     * @brief Pointer to function for handling last frame of multi-frame messages
     *
     * @details Processes final frame of multi-frame sequence. Appends final payload,
     * marks message complete, removes from buffer list, pushes to OpenLCB FIFO.
     *
     * @param can_msg Pointer to received CAN frame
     * @param can_buffer_start_index Offset where OpenLCB data begins in payload
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_last_frame()
     *
     * @see CanRxMessageHandler_last_frame - Standard implementation
     */
    void (*handle_last_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index);

    /**
     * @brief Pointer to function for handling stream protocol CAN frames
     *
     * @details Processes stream frames used for continuous data transfer. Currently
     * placeholder for future stream protocol implementation.
     *
     * @param can_msg Pointer to received CAN frame
     * @param can_buffer_start_index Offset where stream data begins in payload
     * @param data_type Buffer type needed (must be STREAM)
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_stream_frame()
     *
     * @see CanRxMessageHandler_stream_frame - Standard implementation
     */
    void (*handle_stream_frame)(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);


    /**
     * @brief Pointer to function for handling RID CAN control frames
     *
     * @details Processes Reserve ID frames during alias allocation sequence. Checks
     * for alias conflicts and sends response if needed.
     *
     * @param can_msg Pointer to received RID frame
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_rid_frame()
     *
     * @see CanRxMessageHandler_rid_frame - Standard implementation
     */
    void (*handle_rid_frame)(can_msg_t *can_msg);


    /**
     * @brief Pointer to function for handling AMD CAN control frames
     *
     * @details Processes Alias Map Definition frames that announce alias/NodeID mappings.
     * Checks for duplicate aliases and updates mapping table.
     *
     * @param can_msg Pointer to received AMD frame containing NodeID
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_amd_frame()
     *
     * @see CanRxMessageHandler_amd_frame - Standard implementation
     */
    void (*handle_amd_frame)(can_msg_t *can_msg);

    /**
     * @brief Pointer to function for handling AME CAN control frames
     *
     * @details Processes Alias Map Enquiry frames requesting alias information. Responds
     * with AMD frames for our registered aliases.
     *
     * @param can_msg Pointer to received AME frame (may contain optional NodeID)
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_ame_frame()
     *
     * @see CanRxMessageHandler_ame_frame - Standard implementation
     */
    void (*handle_ame_frame)(can_msg_t *can_msg);


    /**
     * @brief Pointer to function for handling AMR CAN control frames
     *
     * @details Processes Alias Map Reset frames commanding alias release. Checks for
     * duplicate alias condition.
     *
     * @param can_msg Pointer to received AMR frame
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_amr_frame()
     *
     * @see CanRxMessageHandler_amr_frame - Standard implementation
     */
    void (*handle_amr_frame)(can_msg_t *can_msg);


    /**
     * @brief Pointer to function for handling Error Information Report CAN control frames
     *
     * @details Processes error report frames from other nodes indicating network problems
     * or protocol violations. Checks for duplicate alias condition.
     *
     * @param can_msg Pointer to received error report frame
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_error_info_report_frame()
     *
     * @see CanRxMessageHandler_error_info_report_frame - Standard implementation
     */
    void (*handle_error_info_report_frame)(can_msg_t *can_msg);

    /**
     * @brief Pointer to function for handling CID CAN control frames
     *
     * @details Processes Check ID frames during alias allocation sequence. Checks for
     * conflicts and sends RID response if we have the alias being checked.
     *
     * @param can_msg Pointer to received CID frame containing NodeID fragment
     *
     * @warning Required assignment - Defaults to CanRxMessageHandler_cid_frame()
     *
     * @see CanRxMessageHandler_cid_frame - Standard implementation
     */
    void (*handle_cid_frame)(can_msg_t *can_msg);

    /**
     * @brief Pointer to function for finding alias mappings by alias
     *
     * @details Looks up NodeID associated with a CAN alias. Used to validate that
     * addressed messages are for one of our nodes before processing.
     *
     * @param alias 12-bit CAN alias to search for
     *
     * @warning Required assignment - Defaults to AliasMappings_find_mapping_by_alias()
     *
     * @see AliasMappings_find_mapping_by_alias - Standard implementation
     */
    alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

    // OPTIONAL FUNCTION

    // CALLBACK FUNCTIONS

    /**
     * @brief Optional callback for application notification on CAN frame reception
     *
     * @details Called immediately when CAN frame is received, before any processing.
     * Useful for monitoring, logging, or statistics gathering.
     *
     * @param can_msg Pointer to received CAN frame
     *
     * @warning Optional callback - Defaults to NULL
     * @warning Called in interrupt/thread context - keep processing minimal
     * @warning Must not block execution
     * @warning Must complete quickly (microseconds, not milliseconds)
     *
     * @attention Do not perform lengthy operations in this callback
     * @attention Do not call blocking functions
     * @attention Consider setting flags for main loop processing instead
     *
     * @note Typical uses: incrementing counters, toggling LEDs, setting flags
     */
    void (*on_receive)(can_msg_t *can_msg);

    /*@}*/

} interface_can_rx_statemachine_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Receive State Machine module
     *
     * @details Stores the dependency injection interface for use by the state machine.
     * Must be called during application initialization before processing any CAN frames.
     *
     * Use cases:
     * - Application startup sequence
     * - System initialization
     *
     * @param interface_can_rx_statemachine Pointer to interface structure containing required function pointers
     *
     * @warning Must be called exactly once during initialization
     * @warning NOT thread-safe
     *
     * @attention Call before CanRxStatemachine_incoming_can_driver_callback is invoked
     * @attention All required function pointers in interface must be initialized
     *
     * @see interface_can_rx_statemachine_t - Interface structure definition
     */
    extern void CanRxStatemachine_initialize(const interface_can_rx_statemachine_t *interface_can_rx_statemachine);


    /**
     * @brief Entry point for incoming CAN frames from hardware driver
     *
     * @details Called by the application's CAN hardware driver when a frame is received.
     * Decodes the CAN frame format, routes to appropriate handler based on frame type,
     * and initiates multi-frame message assembly if needed. This is the primary entry
     * point from the hardware layer into the OpenLCB library.
     *
     * Use cases:
     * - CAN receive interrupt handler
     * - CAN receive thread/task
     * - Polled CAN reception
     *
     * @param can_msg Pointer to received CAN frame buffer from hardware
     *
     * @warning Must not be called when shared resources are locked
     * @warning NOT thread-safe with main state machine
     * @warning Frame buffer must remain valid until processing completes
     *
     * @attention Application must respect lock_shared_resources/unlock_shared_resources
     * @attention If using interrupts, disable CAN RX interrupt during resource lock
     * @attention If using threads, either suspend thread or queue frame for later processing
     * @attention Lock duration is minimal (microseconds) - queuing should not overflow
     *
     * @note When lock_shared_resources is active, either:
     * @note - Disable CAN interrupts until unlock_shared_resources
     * @note - Queue incoming frames and call this function after unlock
     * @note - Suspend receiving thread until unlock (RTOS only)
     *
     * @see CanMainStatemachine_run - Main state machine that may lock resources
     * @see interface_can_rx_statemachine_t::on_receive - Optional receive callback
     */
    extern void CanRxStatemachine_incoming_can_driver_callback(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__ */
