/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * \file can_frame_message_hander.c
 *
 * As CAN only frame messages come in they need to be processed by the node(s) to see
 * if there is a response required.  These are the handlers called by the CAN main
 * statemachine to accomplish that.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_frame_message_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_utilities.h"
#include "can_tx_statemachine.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"

#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"

// Required external function calls:
//
// CanUtilities_extract_source_alias_from_can_identifier();
// CanUtilities_copy_node_id_to_payload();
// CanTxStatemachine_try_transmit_can_message();
//
//

const uint32_t OPENLCB_GLOBAL_ADDRESSED = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED;


static void _flush_alias_node_id_mappings(void) {

    int i = 0;

    i = i + 1;

    //   TODO:  

}

static bool _check_for_hard_alias_conflict(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (can_node->alias == CanUtilities_extract_source_alias_from_can_identifier(can_msg)) {

        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | can_node->alias;
        CanUtilities_copy_node_id_to_payload(worker_msg, can_node->id, 0);

        if (CanTxStatemachine_try_transmit_can_message(worker_msg)) {

            can_node->state.can_msg_handled = true;
            can_node->state.permitted = 0;
            can_node->state.initalized = 0;
            can_node->state.run_state = RUNSTATE_GENERATE_SEED;

        }

        return true;

    } else

        return false;

}

static bool _check_for_soft_alias_conflict(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (can_node->alias == CanUtilities_extract_source_alias_from_can_identifier(can_msg)) {

        worker_msg->payload_count = 0;
        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_node->alias;

        if (CanTxStatemachine_try_transmit_can_message(worker_msg))

            can_node->state.can_msg_handled = true;

        return true;

    }

    return false;

}

static uint32_t _ack_reject_identifier(uint16_t source_alias) {

    return (OPENLCB_GLOBAL_ADDRESSED | ((uint32_t) (MTI_DATAGRAM_REJECTED_REPLY & 0x0FFF) << 12) | source_alias);

}

static uint32_t _oir_identifier(uint16_t source_alias) {

    return (OPENLCB_GLOBAL_ADDRESSED | ((uint32_t) (MTI_OPTIONAL_INTERACTION_REJECTED & 0x0FFF) << 12) | source_alias);

}

static openlcb_msg_t* _send_reject(uint16_t source_alias, uint16_t dest_alias, uint16_t mti, uint16_t error_code) {

    can_msg_t* can_msg_error = CanBufferStore_allocate_buffer();

    if (can_msg_error) {

        if (mti == MTI_DATAGRAM) {

            can_msg_error->identifier = _ack_reject_identifier(source_alias);

        } else {

            can_msg_error->identifier = _oir_identifier(source_alias);

        }

        can_msg_error->payload[0] = (uint8_t) (dest_alias >> 8) & 0x00FF;
        can_msg_error->payload[1] = (uint8_t) dest_alias & 0x00FF;
        can_msg_error->payload[2] = (uint8_t) (error_code >> 8) & 0x00FF;
        can_msg_error->payload[3] = (uint8_t) error_code & 0x00FF;

        // Flag so it is directly sent out and not processed by nodes
        can_msg_error->state.addressed_direct_tx = true;

        can_msg_error->payload_count = 4;

        if (!CanBufferFifo_push(can_msg_error)) {

            CanBufferStore_free_buffer(can_msg_error);

            return NULL;

        };

    }

    return NULL;

}

void CanFrameMessageHandler_cid(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_soft_alias_conflict(can_node, can_msg, worker_msg)) {

        can_node->state.can_msg_handled = true;

    }

}

void CanFrameMessageHandler_rid(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_hard_alias_conflict(can_node, can_msg, worker_msg)) {

        can_node->state.can_msg_handled = true;

    }

}

void CanFrameMessageHandler_amd(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_hard_alias_conflict(can_node, can_msg, worker_msg)) {

        can_node->state.can_msg_handled = true;

    }

}

void CanFrameMessageHandler_ame(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {


    // Someone is requesting we reply with Alias Mapping Definitions for our Node(s)

    if (can_msg->payload_count == 0) {

        _flush_alias_node_id_mappings();

    }

    if (_check_for_hard_alias_conflict(can_node, can_msg, worker_msg)) {

        return;

    }

    if ((can_msg->payload_count == 0) || (can_node->id == CanUtilities_extract_can_payload_as_node_id(can_msg))) {

        CanUtilities_copy_node_id_to_payload(worker_msg, can_node->id, 0);
        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_node->alias;

        if (CanTxStatemachine_try_transmit_can_message(worker_msg)) {

            can_node->state.can_msg_handled = true;

        }

    } else {

        can_node->state.can_msg_handled = true;

    }

}

void CanFrameMessageHandler_amr(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    // printf("AMR\n");

    can_node->state.can_msg_handled = true;

}

openlcb_msg_t* CanFrameMessageHandler_handle_first_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum_t data_type) {

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
    uint16_t mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* result = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (result) {

        return _send_reject(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END);

    }

    result = OpenLcbBufferStore_allocate_buffer(data_type);

    if (!result) {

        return _send_reject(dest_alias, source_alias, mti, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

    }

    result->mti = mti;
    result->source_alias = source_alias;
    result->dest_alias = dest_alias;
    result->state.inprocess = true;

    CanUtilities_copy_can_payload_to_openlcb_payload(result, can_msg, can_buffer_start_index);

    if (!OpenLcbBufferList_add(result)) {

        OpenLcbBufferStore_free_buffer(result);

        return NULL;

    };

    return result;

}

openlcb_msg_t* CanFrameMessageHandler_handle_middle_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index) {

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
    uint16_t mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* result = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!result) {

        return _send_reject(source_alias, dest_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

    }


    CanUtilities_append_can_payload_to_openlcb_payload(result, can_msg, can_buffer_start_index);

    return result;

}

openlcb_msg_t* CanFrameMessageHandler_handle_last_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index) {

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
    uint16_t mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t * result = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!result) {

        return _send_reject(source_alias, dest_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

    }

    CanUtilities_append_can_payload_to_openlcb_payload(result, can_msg, can_buffer_start_index);
    result->state.inprocess = false;

    OpenLcbBufferList_release(result);
    OpenLcbBufferFifo_push_existing(result);

    return result;

}

openlcb_msg_t* CanFrameMessageHandler_handle_single_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum_t data_type) {

    openlcb_msg_t* new_msg = OpenLcbBufferStore_allocate_buffer(data_type);

    if (!new_msg) {

        return NULL;

    }

    new_msg->source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    new_msg->dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
    new_msg->mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);
    new_msg->dest_id = 0;
    new_msg->source_id = 0;
    new_msg->payload_count = 0;
    CanUtilities_copy_can_payload_to_openlcb_payload(new_msg, can_msg, can_buffer_start_index);

    if (OpenLcbBufferFifo_push(new_msg)) {

        return new_msg;

    }

    return NULL;

}

void CanFrameMessageHandler_handle_can_legacy_snip(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum_t data_type) {

    // Early implementations did not have the multi-frame bits to use... special case

    openlcb_msg_t* openlcb_msg_inprocess = OpenLcbBufferList_find(
            CanUtilities_extract_source_alias_from_can_identifier(can_msg),
            CanUtilties_extract_dest_alias_from_can_message(can_msg),
            CanUtilties_convert_can_mti_to_openlcb_mti(can_msg)
            );


    if (!openlcb_msg_inprocess) { // Do we have one in process?

        CanFrameMessageHandler_handle_first_frame(can_msg, can_buffer_start_index, data_type);

    } else { // Yes we have one in process   


        if (CanUtilities_count_nulls_in_payloads(openlcb_msg_inprocess, can_msg) < 6)

            CanFrameMessageHandler_handle_middle_frame(can_msg, can_buffer_start_index);

        else

            CanFrameMessageHandler_handle_last_frame(can_msg, can_buffer_start_index);

    };

    return;


}