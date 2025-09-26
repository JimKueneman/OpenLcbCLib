/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file can_main_statemachine_hander.c
 *
 *
 * @author Jim Kueneman
 * @date 24 Sept 2025
 */

#include "can_main_statemachine_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_utilities.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"

#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"

static interface_can_main_statemachine_handler_t *_interface;

void CanMainStatemachineHandler_initialize(const interface_can_main_statemachine_handler_t *interface_can_main_statemachine_handler) {

    _interface = (interface_can_main_statemachine_handler_t*) interface_can_main_statemachine_handler;

}

static void _load_amr_message(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_statemachine_info->openlcb_node->alias;
    can_statemachine_info->outgoing_can_msg->payload_count = 6;
    CanUtilities_copy_node_id_to_can_payload_buffer(can_statemachine_info->openlcb_node->id, &can_statemachine_info->outgoing_can_msg->payload);

    can_statemachine_info->outgoing_can_msg_valid = true;

}

static void _reset_node(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->state.permitted = false;
    can_statemachine_info->openlcb_node->state.initalized = false;
    can_statemachine_info->openlcb_node->state.duplicate_id_detected = false;
    can_statemachine_info->openlcb_node->state.firmware_upgrade_active = false;
    can_statemachine_info->openlcb_node->state.resend_datagram = false;
    can_statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false;
    if (can_statemachine_info->openlcb_node->last_received_datagram) {

        OpenLcbBufferStore_free_buffer(can_statemachine_info->openlcb_node->last_received_datagram);
        can_statemachine_info->openlcb_node->last_received_datagram = NULL;

    }

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_GENERATE_SEED; // Re-log in with a new generated Alias  

}

static void _handle_duplicate_alias_detected(can_statemachine_info_t *can_statemachine_info) {

    _reset_node(can_statemachine_info);
    _load_amr_message(can_statemachine_info);

}

static bool _is_duplicate_alias(can_statemachine_info_t *can_statemachine_info) {

    // Check for duplicate Alias 
    return (_interface->find_by_alias(CanUtilities_extract_source_alias_from_can_identifier(can_statemachine_info->incoming_msg)));

}

void CanMainStatemachineHandler_cid(can_statemachine_info_t *can_statemachine_info) {

    // Check for duplicate Alias 
    openlcb_node_t *openlcb_node = _interface->find_by_alias(CanUtilities_extract_source_alias_from_can_identifier(can_statemachine_info->incoming_msg));

    if (openlcb_node) {

        can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | openlcb_node->alias;
        can_statemachine_info->outgoing_can_msg->payload_count = 0;
        can_statemachine_info->outgoing_can_msg_valid = true;

    }

}

void CanMainStatemachineHandler_rid(can_statemachine_info_t *can_statemachine_info) {

    if (_is_duplicate_alias(can_statemachine_info)) {

        _handle_duplicate_alias_detected(can_statemachine_info);

    }

}

void CanMainStatemachineHandler_amd(can_statemachine_info_t *can_statemachine_info) {

    if (_is_duplicate_alias(can_statemachine_info)) {

        _handle_duplicate_alias_detected(can_statemachine_info);

    }

}

void CanMainStatemachineHandler_ame(can_statemachine_info_t *can_statemachine_info) {

    if (_is_duplicate_alias(can_statemachine_info)) {

        _handle_duplicate_alias_detected(can_statemachine_info);

        return;

    }


    openlcb_node_t *openlcb_node = NULL;

    if (can_statemachine_info->incoming_msg->payload_count > 0) {

        openlcb_node = _interface->find_by_node_id(CanUtilities_extract_can_payload_as_node_id(can_statemachine_info->incoming_msg));

        if (openlcb_node) {

            can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | openlcb_node->alias;
            can_statemachine_info->outgoing_can_msg->payload_count = 6;
            CanUtilities_copy_node_id_to_can_payload_buffer(openlcb_node->id, &can_statemachine_info->outgoing_can_msg->payload);
            can_statemachine_info->outgoing_can_msg_valid = true;

            return;

        }

        return;

    }

    openlcb_node = _interface->get_first(100);

    while (openlcb_node) {

        can_statemachine_info->outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | openlcb_node->alias;
        can_statemachine_info->outgoing_can_msg->payload_count = 6;
        CanUtilities_copy_node_id_to_can_payload_buffer(openlcb_node->id, &can_statemachine_info->outgoing_can_msg->payload);
        can_statemachine_info->outgoing_can_msg_valid = true;

        openlcb_node = _interface->get_next(100);

    }

}

void CanMainStatemachineHandler_amr(can_statemachine_info_t *can_statemachine_info) {

    if (_is_duplicate_alias(can_statemachine_info)) {

        _handle_duplicate_alias_detected(can_statemachine_info);

    }


}

void CanMainStatemachineHandler_error_information_report(can_statemachine_info_t *can_statemachine_info) {

    if (_is_duplicate_alias(can_statemachine_info)) {

        _handle_duplicate_alias_detected(can_statemachine_info);

    }

}
