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
 * @file can_types.h
 * @brief Type definitions and constants for the CAN transport layer.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#ifndef __DRIVERS_CANBUS_CAN_TYPES__
#define __DRIVERS_CANBUS_CAN_TYPES__

#include <stdbool.h>
#include <stdint.h>

#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // ************************ USER DEFINED VARIABLES *****************************

    /**
     * @brief Number of @ref can_msg_t buffers in the pool (can_buffer_store.h).
     *
     * @details Override at compile time: -D USER_DEFINED_CAN_MSG_BUFFER_DEPTH=20
     *
     * @warning Maximum value is 254 (0xFE).
     */
#ifndef USER_DEFINED_CAN_MSG_BUFFER_DEPTH
#define USER_DEFINED_CAN_MSG_BUFFER_DEPTH 10
#endif

    // *********************END USER DEFINED VARIABLES *****************************

    /** @brief Number of @ref alias_mapping_t slots. Defaults to USER_DEFINED_NODE_BUFFER_DEPTH. */
#ifndef ALIAS_MAPPING_BUFFER_DEPTH
#define ALIAS_MAPPING_BUFFER_DEPTH USER_DEFINED_NODE_BUFFER_DEPTH
#endif

    /** @brief FIFO slot count — one extra slot so head==tail always means empty. */
#define LEN_CAN_FIFO_BUFFER (USER_DEFINED_CAN_MSG_BUFFER_DEPTH + 1)

    /** @brief Number of data bytes in a CAN 2.0 frame. */
#define LEN_CAN_BYTE_ARRAY 8

    /** @brief Pass to functions when the CAN payload starts at byte 0 (no destination alias). */
#define OFFSET_CAN_WITHOUT_DEST_ADDRESS 0

    /** @brief Pass to functions when bytes 0-1 carry a destination alias; data starts at byte 2. */
#define OFFSET_CAN_WITH_DEST_ADDRESS 2

    /** @brief Bit mask for building global/addressed OpenLCB CAN identifiers. */
#define _OPENLCB_GLOBAL_ADDRESSED (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED)

    /** @brief Pre-built upper bits for a Datagram Rejected reply CAN identifier. */
#define _DATAGRAM_REJECT_REPLY (_OPENLCB_GLOBAL_ADDRESSED | ((uint32_t) (MTI_DATAGRAM_REJECTED_REPLY & 0x0FFF) << 12))

    /** @brief Pre-built upper bits for an Optional Interaction Rejected reply CAN identifier. */
#define _OPTIONAL_INTERACTION_REJECT_REPLY (_OPENLCB_GLOBAL_ADDRESSED | ((uint32_t) (MTI_OPTIONAL_INTERACTION_REJECTED & 0x0FFF) << 12))


    /**
     * @typedef payload_bytes_can_t
     * @brief Fixed 8-byte array holding a CAN frame payload.
     *
     * @see LEN_CAN_BYTE_ARRAY
     * @see can_msg_t
     */
    typedef uint8_t payload_bytes_can_t[LEN_CAN_BYTE_ARRAY];

    /**
     * @typedef can_msg_state_t
     * @brief Bit-field tracking allocation status of a @ref can_msg_t buffer.
     *
     * @see CanBufferStore_allocate_buffer
     * @see CanBufferStore_free_buffer
     */
    typedef struct can_msg_state_struct {
        uint8_t allocated : 1; /**< @brief Set while the buffer is owned by the pool allocator. */
    } can_msg_state_t;

    /**
     * @typedef can_msg_t
     * @brief Complete CAN 2.0B extended frame: 29-bit identifier + up to 8 data bytes.
     *
     * @warning Maximum payload is 8 bytes.
     *
     * @see can_buffer_store.h
     * @see can_buffer_fifo.h
     */
    typedef struct can_msg_struct {
        can_msg_state_t state;        /**< @brief Allocation flags. */
        uint32_t identifier;          /**< @brief 29-bit extended CAN identifier. */
        uint8_t payload_count;        /**< @brief Number of valid bytes in payload (0-8). */
        payload_bytes_can_t payload;  /**< @brief Data bytes of the frame. */
    } can_msg_t;

    /**
     * @typedef can_msg_array_t
     * @brief Pre-allocated array of @ref can_msg_t buffers, sized USER_DEFINED_CAN_MSG_BUFFER_DEPTH.
     *
     * @see can_buffer_store.c
     */
    typedef can_msg_t can_msg_array_t[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];

    /**
     * @typedef can_main_statemachine_t
     * @brief Working context for the CAN main state machine.
     */
    typedef struct can_main_statemachine_struct {
        openlcb_statemachine_worker_t *openlcb_worker; /**< @brief OpenLCB layer worker thread context. */
    } can_main_statemachine_t;

    /**
     * @typedef can_statemachine_info_t
     * @brief Context block passed through the CAN state machine on every iteration.
     *
     * @details Carries the current node pointer, a statically-allocated login frame buffer,
     * a pointer to an outgoing frame from the pool, and control flags.  The login buffer is
     * always valid; outgoing_can_msg is NULL when no message is pending.
     *
     * @warning login_outgoing_can_msg points to a static variable in can_main_statemachine.c — do not free it.
     * @warning outgoing_can_msg must be freed after successful transmission.
     *
     * @see can_main_statemachine.h
     * @see can_login_statemachine.h
     */
    typedef struct can_statemachine_info_struct {
        openlcb_node_t *openlcb_node;            /**< @brief Node currently being processed. */
        can_msg_t *login_outgoing_can_msg;        /**< @brief Statically-allocated login frame (CID/RID/AMD). */
        uint8_t login_outgoing_can_msg_valid : 1; /**< @brief Set when login_outgoing_can_msg needs transmitting. */
        can_msg_t *outgoing_can_msg;              /**< @brief Pool-allocated reply frame; freed after TX. */
        uint8_t enumerating : 1;                  /**< @brief Set when the handler will produce N reply frames. */
    } can_statemachine_info_t;

    /**
     * @typedef alias_mapping_t
     * @brief One entry in the alias mapping table: a @ref node_id_t / 12-bit alias pair.
     *
     * @details Flags may be set from interrupt context and read from the main loop.
     * Use lock/unlock around accesses shared between contexts.
     *
     * @see alias_mappings.h
     */
    typedef struct alias_mapping_struct {
        node_id_t node_id;       /**< @brief Permanent 48-bit Node ID. */
        uint16_t alias;          /**< @brief Temporary 12-bit CAN alias (0x001-0xFFF). */
        uint8_t is_duplicate : 1; /**< @brief Set by ISR when another node claims this alias. */
        uint8_t is_permitted : 1; /**< @brief Set after successful login (AMD transmitted). */
    } alias_mapping_t;

    /**
     * @typedef alias_mapping_info_t
     * @brief Container for all @ref alias_mapping_t entries plus a global duplicate flag.
     *
     * @details has_duplicate_alias is a fast-check flag: when set, at least one entry
     * has is_duplicate set and the main loop should scan the list to find and resolve it.
     *
     * @see alias_mappings.h
     * @see AliasMappings_set_has_duplicate_alias_flag
     */
    typedef struct alias_mapping_info_struct {
        alias_mapping_t list[ALIAS_MAPPING_BUFFER_DEPTH]; /**< @brief All registered mappings. */
        bool has_duplicate_alias;                          /**< @brief True if any entry has is_duplicate set. */
    } alias_mapping_info_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_TYPES__ */
