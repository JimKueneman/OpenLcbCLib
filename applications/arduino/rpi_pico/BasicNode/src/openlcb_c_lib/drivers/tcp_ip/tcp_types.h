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
 * @file tcp_types.h
 * @brief Type definitions and constants for the TCP/IP transport layer.
 *
 * @details Defines @ref tcp_statemachine_info_t, all preamble flag constants,
 * link control message types, and compile-time configuration constants shared
 * across the TCP driver modules.  No executable code — types and macros only.
 *
 * Analogous to can_types.h for the CAN transport layer.
 *
 * @author Jim Kueneman
 * @date 4 Apr 2026
 */

#ifndef __DRIVERS_TCP_IP_TCP_TYPES__
#define __DRIVERS_TCP_IP_TCP_TYPES__

#include <stdbool.h>
#include <stdint.h>

#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __has_include
  #if __has_include("tcp_user_config.h")
  #include "tcp_user_config.h"
  #elif __has_include("../../tcp_user_config.h")
  #include "../../tcp_user_config.h"
  #elif __has_include("../../../tcp_user_config.h")
  #include "../../../tcp_user_config.h"
  #elif __has_include("../../../../tcp_user_config.h")
  #include "../../../../tcp_user_config.h"
  #else
  #error "tcp_user_config.h not found. Copy templates/tcp_ip/tcp_user_config.h to your project include path."
  #endif
#else
  #include "tcp_user_config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // =========================================================================
    // User-configurable constants (defaults if not overridden in tcp_user_config.h)
    // =========================================================================

    /**
     * @brief Maximum bytes in the RX accumulation buffer.
     *
     * @details Override at compile time: -D USER_DEFINED_TCP_RX_ACCUMULATION_BUFFER_LEN=2048
     */
#ifndef USER_DEFINED_TCP_RX_ACCUMULATION_BUFFER_LEN
#define USER_DEFINED_TCP_RX_ACCUMULATION_BUFFER_LEN    1024
#endif

#if USER_DEFINED_TCP_RX_ACCUMULATION_BUFFER_LEN < 25
#error "USER_DEFINED_TCP_RX_ACCUMULATION_BUFFER_LEN must be >= 25 (17 preamble + 2 MTI + 6 source)"
#endif

    /**
     * @brief Maximum bytes in the TX scratch buffer.
     *
     * @details Override at compile time: -D USER_DEFINED_TCP_TX_BUFFER_LEN=2048
     */
#ifndef USER_DEFINED_TCP_TX_BUFFER_LEN
#define USER_DEFINED_TCP_TX_BUFFER_LEN                 1024
#endif

#if USER_DEFINED_TCP_TX_BUFFER_LEN < 25
#error "USER_DEFINED_TCP_TX_BUFFER_LEN must be >= 25 (17 preamble + 2 MTI + 6 source)"
#endif

    /**
     * @brief Maximum concurrent multi-part message reassemblies.
     *
     * @details Override at compile time: -D USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES=4
     */
#ifndef USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES
#define USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES      2
#endif

#if USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES < 1
#error "USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES must be >= 1"
#endif

    // =========================================================================
    // TCP/IP OpenLCB Preamble Constants
    // =========================================================================

    /** @brief Total preamble length: 2 flags + 3 length + 6 originating NodeID + 6 capture time. */
#define TCP_PREAMBLE_LEN                               17

    /** @brief Byte offset of the flags field in the preamble. */
#define TCP_PREAMBLE_OFFSET_FLAGS                      0

    /** @brief Byte offset of the length field in the preamble (3 bytes, big-endian). */
#define TCP_PREAMBLE_OFFSET_LENGTH                     2

    /** @brief Byte offset of the originating Node ID in the preamble (6 bytes). */
#define TCP_PREAMBLE_OFFSET_ORIGINATING_NODE_ID        5

    /** @brief Byte offset of the capture time in the preamble (6 bytes). */
#define TCP_PREAMBLE_OFFSET_CAPTURE_TIME               11

    // =========================================================================
    // Preamble Flag Bits (16-bit flags field, bytes 0-1)
    // =========================================================================

    /** @brief Bit 15: 1 = OpenLCB message, 0 = link control message. */
#define TCP_FLAGS_MESSAGE                              0x8000

    /** @brief Bit 14: 1 = another preamble follows (chaining). */
#define TCP_FLAGS_CHAINING                             0x4000

    /** @brief Mask for multi-part indicator bits (bits 11-10). */
#define TCP_FLAGS_MULTIPART_MASK                       0x00C0

    /** @brief Multi-part: single complete message (bits 11-10 = 00). */
#define TCP_FLAGS_MULTIPART_SINGLE                     0x0000

    /** @brief Multi-part: first part of message (bits 11-10 = 01). */
#define TCP_FLAGS_MULTIPART_FIRST                      0x0040

    /** @brief Multi-part: middle part of message (bits 11-10 = 11). */
#define TCP_FLAGS_MULTIPART_MIDDLE                     0x00C0

    /** @brief Multi-part: last part of message (bits 11-10 = 10). */
#define TCP_FLAGS_MULTIPART_LAST                       0x0080

    // =========================================================================
    // Message Body Offsets (relative to start of body, after preamble)
    // =========================================================================

    /** @brief Offset of MTI in the message body (2 bytes, big-endian). */
#define TCP_BODY_OFFSET_MTI                            0

    /** @brief Offset of source Node ID in the message body (6 bytes). */
#define TCP_BODY_OFFSET_SOURCE_NODE_ID                 2

    /** @brief Offset of destination Node ID when present (6 bytes). */
#define TCP_BODY_OFFSET_DEST_NODE_ID                   8

    /** @brief Size of the MTI field in the message body. */
#define TCP_BODY_MTI_LEN                               2

    /** @brief Size of a Node ID field in the message body. */
#define TCP_BODY_NODE_ID_LEN                           6

    // =========================================================================
    // Link Control Message Types
    // =========================================================================
    // Link control messages have TCP_FLAGS_MESSAGE bit clear (bit 15 = 0).
    // The type is encoded in the first 2 bytes of the message body.

    /** @brief Link control: request status from peer. */
#define TCP_LINK_CONTROL_STATUS_REQUEST                0x0000

    /** @brief Link control: reply with status. */
#define TCP_LINK_CONTROL_STATUS_REPLY                  0x0001

    /** @brief Link control: request graceful link drop. */
#define TCP_LINK_CONTROL_DROP_REQUEST                  0x0002

    /** @brief Link control: acknowledge link drop. */
#define TCP_LINK_CONTROL_DROP_REPLY                    0x0003

    // =========================================================================
    // Login State Machine States
    // =========================================================================

    /** @brief TCP login states. */
    typedef enum {
        TCP_LOGIN_IDLE = 0,              /**< @brief Not connected / not started. */
        TCP_LOGIN_SEND_VERIFY_GLOBAL,    /**< @brief Send Verify Node ID Global (retries until transport accepts). */
        TCP_LOGIN_SEND_INIT_COMPLETE,    /**< @brief Send Initialization Complete for each local node. */
        TCP_LOGIN_SEND_EVENTS,           /**< @brief Send Producer/Consumer Identified for declared events. */
        TCP_LOGIN_COMPLETE               /**< @brief Login finished, all nodes in RUN state. */
    } tcp_login_state_enum;

    // =========================================================================
    // Link State
    // =========================================================================

    /** @brief TCP link states. */
    typedef enum {
        TCP_LINK_STATE_DISCONNECTED = 0, /**< @brief No active TCP connection. */
        TCP_LINK_STATE_CONNECTING,       /**< @brief TCP connection being established. */
        TCP_LINK_STATE_LOGGING_IN,       /**< @brief Connected, running login sequence. */
        TCP_LINK_STATE_RUNNING,          /**< @brief Fully operational. */
        TCP_LINK_STATE_DROPPING          /**< @brief Graceful disconnect in progress. */
    } tcp_link_state_enum;

    // =========================================================================
    // State Machine Context
    // =========================================================================

    /**
     * @typedef tcp_statemachine_info_t
     * @brief Context block passed through the TCP state machine on every iteration.
     *
     * @details Carries the current node pointer and control flags.
     * Analogous to @ref can_statemachine_info_t for the CAN driver.
     *
     * @see tcp_main_statemachine.h
     * @see tcp_login_statemachine.h
     */
    typedef struct tcp_statemachine_info_struct {
        openlcb_node_t *openlcb_node;       /**< @brief Node currently being processed. */
        tcp_login_state_enum login_state;    /**< @brief Current login state for this node. */
        tcp_link_state_enum link_state;      /**< @brief Current link state. */
        uint8_t current_tick;                /**< @brief Snapshot of the global 100ms tick. */
        uint8_t pending_status_reply : 1;    /**< @brief Status Reply needs retry. */
        uint8_t pending_drop_reply : 1;      /**< @brief Drop Link Reply needs retry. */
    } tcp_statemachine_info_t;

    // =========================================================================
    // Multi-part Message Accumulation Entry
    // =========================================================================

    /**
     * @typedef tcp_multipart_entry_t
     * @brief One slot in the multi-part message reassembly table.
     *
     * @details Keyed by originating Node ID from the preamble.  Accumulates
     * payload bytes across first/middle/last parts until a complete OpenLCB
     * message can be forwarded to the protocol layer.
     */
    typedef struct tcp_multipart_entry_struct {
        node_id_t originating_node_id;   /**< @brief Key: originating Node ID from preamble. 0 = unused. */
        openlcb_msg_t *openlcb_msg;      /**< @brief Allocated message buffer being assembled. */
    } tcp_multipart_entry_t;

    /**
     * @typedef tcp_multipart_table_t
     * @brief Table of all multi-part reassembly slots.
     */
    typedef tcp_multipart_entry_t tcp_multipart_table_t[USER_DEFINED_TCP_MAX_MULTIPART_ASSEMBLIES];

    // =========================================================================
    // RX Accumulation Buffer Type
    // =========================================================================

    /**
     * @typedef tcp_rx_accumulation_buffer_t
     * @brief Static byte buffer for accumulating incoming TCP data.
     */
    typedef uint8_t tcp_rx_accumulation_buffer_t[USER_DEFINED_TCP_RX_ACCUMULATION_BUFFER_LEN];

    // =========================================================================
    // TX Scratch Buffer Type
    // =========================================================================

    /**
     * @typedef tcp_tx_buffer_t
     * @brief Static byte buffer for building outgoing TCP messages.
     */
    typedef uint8_t tcp_tx_buffer_t[USER_DEFINED_TCP_TX_BUFFER_LEN];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_TCP_IP_TCP_TYPES__ */
