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
 * \file can_rx_message_hander.c
 *
 * As CAN only frame messages come in they need to be processed by the node(s) to see
 * if there is a response required.  These are the handlers called by the CAN main
 * statemachine to accomplish that.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_rx_message_handler.h"

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
#include "../../openlcb/openlcb_utilities.h"


static interface_can_rx_message_handler_t *_interface;

void CanRxMessageHandler_initialize(const interface_can_rx_message_handler_t *interface_can_frame_message_handler) {

    _interface = (interface_can_rx_message_handler_t*) interface_can_frame_message_handler;

}

static void _queue_reject_message(uint16_t source_alias, uint16_t dest_alias, uint16_t mti, uint16_t error_code) {

    openlcb_msg_t * target_openlcb_msg = _interface->openlcb_buffer_store_allocate_buffer(BASIC);

    if (target_openlcb_msg) {

        if (mti == MTI_DATAGRAM) {

            mti = MTI_DATAGRAM_REJECTED_REPLY;
            

        } else {

            mti = MTI_OPTIONAL_INTERACTION_REJECTED;

        }

        // TODO: Probably Stream is a special case too

        OpenLcbUtilities_load_openlcb_message(
                target_openlcb_msg, 
                source_alias, 
                0, 
                dest_alias, 
                0, 
                mti, 
                4);
        OpenLcbUtilities_copy_word_to_openlcb_payload(
                target_openlcb_msg, 
                dest_alias, 
                0);
        OpenLcbUtilities_copy_word_to_openlcb_payload(
                target_openlcb_msg, 
                error_code, 
                2);

        OpenLcbBufferFifo_push(target_openlcb_msg);

    }

}

static bool _check_for_duplicate_alias(can_msg_t* can_msg) {

    // Check for duplicate Alias 
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(source_alias);

    if (!alias_mapping) {

        return false; // Done nothing to do
    }

    alias_mapping->is_duplicate = true; // flag for the main loop to handle
    _interface->alias_mapping_set_has_duplicate_alias_flag();

    if (alias_mapping->is_permitted) {
   
        can_msg_t *outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();
        outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | source_alias;
        outgoing_can_msg->payload_count = 6;
        CanUtilities_copy_node_id_to_can_payload_buffer(alias_mapping->node_id, &outgoing_can_msg->payload);

        CanBufferFifo_push(outgoing_can_msg);

    }

    return true;

}

void CanRxMessageHandler_first_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type) {

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    // See if there is a message already started for this.
    openlcb_msg_t* target_can_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (target_can_msg) {

        // If we find a message for this source/dest/mti then it is an error as it is out of order
        _queue_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END);

        return;

    }

    // Try to allocate an openlcb message buffer to start accumulating the frames into an openlcb message
    target_can_msg = _interface->openlcb_buffer_store_allocate_buffer(data_type);

    if (!target_can_msg) {
 
        _queue_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

        return;

    }

    target_can_msg->mti = mti;
    target_can_msg->source_alias = source_alias;
    target_can_msg->dest_alias = dest_alias;
    target_can_msg->state.inprocess = true;

    CanUtilities_copy_can_payload_to_openlcb_payload(target_can_msg, can_msg, can_buffer_start_index);

    OpenLcbBufferList_add(target_can_msg); // Can not fail List is as large as the number of buffers

}

void CanRxMessageHandler_middle_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index) {

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* target_can_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!target_can_msg) {

        _queue_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

        return;

    }

    CanUtilities_append_can_payload_to_openlcb_payload(target_can_msg, can_msg, can_buffer_start_index);

}

void CanRxMessageHandler_last_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index) {
    
    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    int16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t * target_can_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!target_can_msg) {
 
        _queue_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

        return;

    }

    CanUtilities_append_can_payload_to_openlcb_payload(target_can_msg, can_msg, can_buffer_start_index);
    
    target_can_msg->state.inprocess = false;

    OpenLcbBufferList_release(target_can_msg);
    OpenLcbBufferFifo_push_existing(target_can_msg);

}

void CanRxMessageHandler_single_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type) {

    openlcb_msg_t* target_openlcb_msg = _interface->openlcb_buffer_store_allocate_buffer(data_type);

    if (!target_openlcb_msg) {

        return;

    }

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    int16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);
    OpenLcbUtilities_load_openlcb_message(
            target_openlcb_msg, 
            source_alias, 
            0, 
            dest_alias, 
            0, 
            mti, 
            0);
    CanUtilities_copy_can_payload_to_openlcb_payload(
            target_openlcb_msg, 
            can_msg, 
            can_buffer_start_index);

    OpenLcbBufferFifo_push(target_openlcb_msg); // Can not fail List is as large as the number of buffers

}

void CanRxMessageHandler_can_legacy_snip(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type) {

    // Early implementations did not have the multi-frame bits to use... special case

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    int16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* openlcb_msg_inprocess = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!openlcb_msg_inprocess) { // Do we have one in process?

        CanRxMessageHandler_first_frame(can_msg, can_buffer_start_index, data_type);

    } else { // Yes we have one in process   


        if (CanUtilities_count_nulls_in_payloads(openlcb_msg_inprocess, can_msg) < 6)

            CanRxMessageHandler_middle_frame(can_msg, can_buffer_start_index);

        else

            CanRxMessageHandler_last_frame(can_msg, can_buffer_start_index);

    };

}

void CanRxMessageHandler_stream_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type) {


}

void CanRxMessageHandler_cid_frame(can_msg_t* can_msg) {

    // Check for duplicate Alias 
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(source_alias);

    if (alias_mapping) {

        can_msg_t *can_msg = _interface->can_buffer_store_allocate_buffer();

        can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | source_alias;
        can_msg->payload_count = 0;

        CanBufferFifo_push(can_msg);

    }

}

void CanRxMessageHandler_rid_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

void CanRxMessageHandler_amd_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

void CanRxMessageHandler_ame_frame(can_msg_t* can_msg) {

    if (_check_for_duplicate_alias(can_msg)) {

        return;

    }

    can_msg_t *outgoing_can_msg = NULL;
 //   uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);

    if (can_msg->payload_count > 0) {

        alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_node_id(CanUtilities_extract_can_payload_as_node_id(can_msg));

        if (alias_mapping) {
 
            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();
            outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping->alias;
            outgoing_can_msg->payload_count = 6;
            CanUtilities_copy_node_id_to_can_payload_buffer(alias_mapping->node_id, &outgoing_can_msg->payload);

            CanBufferFifo_push(outgoing_can_msg);

            return;

        }

        return;

    }

    alias_mapping_info_t *alias_mapping_info = _interface->alias_mapping_get_alias_mapping_info();

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping_info->list[i].alias != 0x00) {

            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();
            outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping_info->list[i].alias;
            outgoing_can_msg->payload_count = 6;
            CanUtilities_copy_node_id_to_can_payload_buffer(alias_mapping_info->list[i].node_id, &outgoing_can_msg->payload);

            CanBufferFifo_push(outgoing_can_msg);

        }

    }

}

void CanRxMessageHandler_amr_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

void CanRxMessageHandler_error_info_report_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}

