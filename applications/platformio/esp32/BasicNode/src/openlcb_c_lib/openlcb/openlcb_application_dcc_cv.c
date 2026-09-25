/** \copyright
 * Copyright (c) 2026, Bob Gamble
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
 * @file openlcb_application_dcc_cv.c
 * @brief DCC CV memory space (0xF8) with deferred replies.
 *
 * @details See openlcb_application_dcc_cv.h for the contract.  This file holds
 * the small table of requests that are waiting for the application, the request
 * handlers that fill it, and the run function that sends the replies.
 *
 * A request goes through these states:
 *
 *   FREE --request, hook says PENDING--> WAITING --complete() or timeout--> REPLY --sent--> FREE
 *
 * A request the hook answers at once never enters the table for longer than the
 * hook call.
 *
 */

#include "openlcb_application_dcc_cv.h"

#ifdef OPENLCB_COMPILE_DCC_CV

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"

    /** @brief Slot life cycle. */
typedef enum {

    DCC_CV_SLOT_FREE = 0,   /**< Unused. */
    DCC_CV_SLOT_WAITING,    /**< Hook answered PENDING; waiting for OpenLcbApplicationDccCv_complete() or the timeout. */
    DCC_CV_SLOT_REPLY       /**< Result known; the reply datagram is waiting to go out. */

} dcc_cv_slot_state_enum;

    /** @brief One request that is waiting for the application, or for the bus. */
typedef struct {

    uint8_t state;                  /**< @ref dcc_cv_slot_state_enum. */
    uint8_t sequence;               /**< Generation, 1..255; lets a stale handle be recognised. */
    uint8_t tick;                   /**< Global tick when the slot entered its current state. */
    bool is_write;                  /**< true = write request. */
    uint8_t command;                /**< payload[1] of the request; the reply command is this plus the OK/FAIL offset. */
    uint8_t value;                  /**< Write: the byte to write.  Read: the byte read, once the slot is in REPLY. */
    uint16_t result;                /**< S_OK or an OpenLCB error code, once the slot is in REPLY. */
    uint16_t requester_alias;       /**< Where the reply goes. */
    node_id_t requester_id;
    uint32_t address;               /**< CV number minus one, echoed in the reply. */
    openlcb_node_t *node;           /**< The node that was addressed and will answer. */

} dcc_cv_pending_t;

static const interface_openlcb_application_dcc_cv_t *_interface;
static dcc_cv_pending_t _pending[USER_DEFINED_DCC_CV_PENDING_COUNT];
static uint8_t _sequence;
static uint8_t _tick;

    /**
     * @brief Stores the interface and clears the pending table.
     *
     * @verbatim
     * @param interface  Populated table; must remain valid for the life of the application.
     * @endverbatim
     */
void OpenLcbApplicationDccCv_initialize(const interface_openlcb_application_dcc_cv_t *interface) {

    _interface = interface;

    memset(_pending, 0, sizeof(_pending));

    _sequence = 0;
    _tick = 0;

}

    /** @brief Returns the slot a handle refers to, or NULL if it is unknown or stale. */
static dcc_cv_pending_t *_slot_from_handle(uint16_t handle) {

    uint8_t index = (uint8_t) (handle & 0xFF);

    if (index >= USER_DEFINED_DCC_CV_PENDING_COUNT) {

        return NULL;

    }

    dcc_cv_pending_t *slot = &_pending[index];

    if (slot->state == DCC_CV_SLOT_FREE || slot->sequence != (uint8_t) (handle >> 8)) {

        return NULL;

    }

    return slot;

}

    /**
     * @brief True if an identical request from the same requester is already outstanding.
     *
     * @details A requester that has waited about three seconds sends the request again.
     * That must not start a second operation on the track; the reply to the first
     * one answers both.
     */
static bool _is_duplicate(openlcb_node_t *node, openlcb_msg_t *request, bool is_write, uint32_t address, uint8_t value) {

    for (uint8_t i = 0; i < USER_DEFINED_DCC_CV_PENDING_COUNT; i++) {

        const dcc_cv_pending_t *slot = &_pending[i];

        if (slot->state != DCC_CV_SLOT_FREE &&
                slot->node == node &&
                slot->requester_alias == request->source_alias &&
                slot->is_write == is_write &&
                slot->address == address &&
                (!is_write || slot->value == value)) {

            return true;

        }

    }

    return false;

}

    /**
     * @brief Takes a free slot for a request and returns its handle in *handle.
     *
     * @return The slot, in WAITING, or NULL if the table is full.
     */
static dcc_cv_pending_t *_accept(openlcb_statemachine_info_t *statemachine_info, bool is_write, uint32_t address, uint8_t value, uint16_t *handle) {

    openlcb_msg_t *request = statemachine_info->incoming_msg_info.msg_ptr;

    for (uint8_t i = 0; i < USER_DEFINED_DCC_CV_PENDING_COUNT; i++) {

        dcc_cv_pending_t *slot = &_pending[i];

        if (slot->state != DCC_CV_SLOT_FREE) {

            continue;

        }

        _sequence++;

        if (_sequence == 0) {

            _sequence = 1;  // 0 is never issued, so a handle of 0 is always stale

        }

        slot->state = DCC_CV_SLOT_WAITING;
        slot->sequence = _sequence;
        slot->tick = statemachine_info->current_tick;
        slot->is_write = is_write;
        slot->command = *request->payload[1];
        slot->value = value;
        slot->result = S_OK;
        slot->requester_alias = request->source_alias;
        slot->requester_id = request->source_id;
        slot->address = address;
        slot->node = statemachine_info->openlcb_node;

        *handle = (uint16_t) (((uint16_t) slot->sequence << 8) | i);

        return slot;

    }

    return NULL;

}

    /** @brief Loads a Read Reply OK into the state machine's outgoing message. */
static void _load_read_ok(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info, uint8_t value) {

    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(statemachine_info, config_mem_read_request_info);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            value,
            config_mem_read_request_info->data_start);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /** @brief Loads a Read Reply Fail into the state machine's outgoing message. */
static void _load_read_fail(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info, uint16_t error_code) {

    OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, error_code);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Read request function for space 0xF8.
     *
     * @details Algorithm:
     * -# Reject anything but a one-byte read
     * -# Absorb a request identical to one already outstanding
     * -# Take a slot (none free: temporary error, the requester will retry)
     * -# Call the application hook
     * -# PENDING: keep the slot and send no reply now.  Otherwise free the slot and
     *    load the reply.
     */
void OpenLcbApplicationDccCv_handle_read_request(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    uint8_t value = 0;
    uint16_t handle = 0;
    uint16_t result;
    dcc_cv_pending_t *slot;

    if (config_mem_read_request_info->bytes != 1) {

        _load_read_fail(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

        return;

    }

    if (_is_duplicate(statemachine_info->openlcb_node, statemachine_info->incoming_msg_info.msg_ptr, false, config_mem_read_request_info->address, 0)) {

        return;

    }

    slot = _accept(statemachine_info, false, config_mem_read_request_info->address, 0, &handle);

    if (!slot) {

        _load_read_fail(statemachine_info, config_mem_read_request_info, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

        return;

    }

    result = _interface->dcc_cv_read(statemachine_info->openlcb_node, config_mem_read_request_info->address, &value, handle);

    if (result == OPENLCB_DCC_CV_RESULT_PENDING) {

        return;  // the hook may already have completed it; either way run() takes it from here

    }

    slot->state = DCC_CV_SLOT_FREE;

    if (result == S_OK) {

        _load_read_ok(statemachine_info, config_mem_read_request_info, value);

    } else {

        _load_read_fail(statemachine_info, config_mem_read_request_info, result);

    }

}

    /** @brief Loads a Write Reply OK into the state machine's outgoing message. */
static void _load_write_ok(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(statemachine_info, config_mem_write_request_info);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /** @brief Loads a Write Reply Fail into the state machine's outgoing message. */
static void _load_write_fail(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, uint16_t error_code) {

    OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(statemachine_info, config_mem_write_request_info, error_code);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
     * @brief Write request function for space 0xF8.  Same shape as the read request.
     */
void OpenLcbApplicationDccCv_handle_write_request(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    uint8_t value;
    uint16_t handle = 0;
    uint16_t result;
    dcc_cv_pending_t *slot;

    if (config_mem_write_request_info->bytes != 1) {

        _load_write_fail(statemachine_info, config_mem_write_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

        return;

    }

    value = (*config_mem_write_request_info->write_buffer)[0];

    if (_is_duplicate(statemachine_info->openlcb_node, statemachine_info->incoming_msg_info.msg_ptr, true, config_mem_write_request_info->address, value)) {

        return;

    }

    slot = _accept(statemachine_info, true, config_mem_write_request_info->address, value, &handle);

    if (!slot) {

        _load_write_fail(statemachine_info, config_mem_write_request_info, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

        return;

    }

    result = _interface->dcc_cv_write(statemachine_info->openlcb_node, config_mem_write_request_info->address, value, handle);

    if (result == OPENLCB_DCC_CV_RESULT_PENDING) {

        return;

    }

    slot->state = DCC_CV_SLOT_FREE;

    if (result == S_OK) {

        _load_write_ok(statemachine_info, config_mem_write_request_info);

    } else {

        _load_write_fail(statemachine_info, config_mem_write_request_info, result);

    }

}

    /**
     * @brief Reply time for a read: the 0xF8 default, or the application's answer.
     *
     * @verbatim
     * @param statemachine_info             Context.
     * @param config_mem_read_request_info  Parsed request.
     * @endverbatim
     *
     * @return Seconds, or 0 for no estimate.
     */
uint16_t OpenLcbApplicationDccCv_read_delayed_reply_time(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    if (config_mem_read_request_info->space_info->address_space == CONFIG_MEM_SPACE_DCC_CV) {

        return USER_DEFINED_DCC_CV_REPLY_TIME_SECONDS;

    }

    if (_interface->config_mem_read_delayed_reply_time) {

        return _interface->config_mem_read_delayed_reply_time(statemachine_info, config_mem_read_request_info);

    }

    return 0;

}

    /**
     * @brief Reply time for a write: the 0xF8 default, or the application's answer.
     *
     * @verbatim
     * @param statemachine_info              Context.
     * @param config_mem_write_request_info  Parsed request.
     * @endverbatim
     *
     * @return Seconds, or 0 for no estimate.
     */
uint16_t OpenLcbApplicationDccCv_write_delayed_reply_time(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    if (config_mem_write_request_info->space_info->address_space == CONFIG_MEM_SPACE_DCC_CV) {

        return USER_DEFINED_DCC_CV_REPLY_TIME_SECONDS;

    }

    if (_interface->config_mem_write_delayed_reply_time) {

        return _interface->config_mem_write_delayed_reply_time(statemachine_info, config_mem_write_request_info);

    }

    return 0;

}

    /**
     * @brief Records the result of a deferred request; run() sends the reply.
     *
     * @verbatim
     * @param handle  Handle the hook was given.
     * @param result  S_OK or an OpenLCB error code.
     * @param value   The byte read, for a successful read.
     * @endverbatim
     *
     * @return true if recorded, false if the handle is stale or unknown.
     */
bool OpenLcbApplicationDccCv_complete(uint16_t handle, uint16_t result, uint8_t value) {

    dcc_cv_pending_t *slot = _slot_from_handle(handle);

    if (!slot || slot->state != DCC_CV_SLOT_WAITING || result == OPENLCB_DCC_CV_RESULT_PENDING) {

        return false;

    }

    slot->result = result;

    if (!slot->is_write) {

        slot->value = value;

    }

    slot->tick = _tick;
    slot->state = DCC_CV_SLOT_REPLY;

    return true;

}

    /**
     * @brief Builds the reply datagram for a slot and hands it to the bus.
     *
     * @details The reply mirrors what the datagram handler would have built:
     * command = request command + 0x10 (OK) or + 0x18 (fail), the address, the
     * space byte, then the data byte (read OK) or the error code (fail).  The
     * request always used the space-in-byte-6 form, because 0xF8 cannot be
     * expressed in the command byte.
     *
     * @return true if the message was accepted for transmission.
     */
static bool _send_reply(const dcc_cv_pending_t *slot) {

    openlcb_msg_t msg = {0};
    payload_datagram_t payload;
    bool ok = (slot->result == S_OK);

    msg.payload = (openlcb_payload_t *) &payload;
    msg.payload_type = DATAGRAM;

    OpenLcbUtilities_load_openlcb_message(
            &msg,
            slot->node->alias,
            slot->node->id,
            slot->requester_alias,
            slot->requester_id,
            MTI_DATAGRAM);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, CONFIG_MEM_CONFIGURATION, 0);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            &msg,
            (uint8_t) (slot->command + (ok ? CONFIG_MEM_REPLY_OK_OFFSET : CONFIG_MEM_REPLY_FAIL_OFFSET)),
            1);

    OpenLcbUtilities_copy_dword_to_openlcb_payload(&msg, slot->address, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, CONFIG_MEM_SPACE_DCC_CV, 6);

    if (!ok) {

        OpenLcbUtilities_copy_word_to_openlcb_payload(&msg, slot->result, 7);

    } else if (!slot->is_write) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(&msg, slot->value, 7);

    }

    return _interface->send_openlcb_msg(&msg);

}

    /**
     * @brief Sends finished replies and times out requests the application never completed.
     *
     * @details Algorithm, per slot:
     * -# WAITING for longer than USER_DEFINED_DCC_CV_TIMEOUT_TICKS: result becomes
     *    ERROR_TEMPORARY_TIME_OUT and the slot moves to REPLY
     * -# REPLY: try to send; free the slot once sent, or once it has been stuck for
     *    longer than the timeout (the bus is not taking traffic and the requester has
     *    given up by now)
     *
     * @verbatim
     * @param current_tick  Global 100 ms tick.
     * @endverbatim
     */
void OpenLcbApplicationDccCv_run(uint8_t current_tick) {

    _tick = current_tick;

    for (uint8_t i = 0; i < USER_DEFINED_DCC_CV_PENDING_COUNT; i++) {

        dcc_cv_pending_t *slot = &_pending[i];
        uint8_t elapsed = (uint8_t) (current_tick - slot->tick);

        if (slot->state == DCC_CV_SLOT_WAITING && elapsed >= USER_DEFINED_DCC_CV_TIMEOUT_TICKS) {

            slot->result = ERROR_TEMPORARY_TIME_OUT;
            slot->tick = current_tick;
            slot->state = DCC_CV_SLOT_REPLY;
            elapsed = 0;

        }

        if (slot->state == DCC_CV_SLOT_REPLY) {

            if (_send_reply(slot) || elapsed >= USER_DEFINED_DCC_CV_TIMEOUT_TICKS) {

                slot->state = DCC_CV_SLOT_FREE;

            }

        }

    }

}

#endif /* OPENLCB_COMPILE_DCC_CV */
