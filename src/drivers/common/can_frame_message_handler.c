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



#define _OPENLCB_GLOBAL_ADDRESSED (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED)

#define _DATAGRAM_REJECT_REPLY (_OPENLCB_GLOBAL_ADDRESSED | ((uint32_t) (MTI_DATAGRAM_REJECTED_REPLY & 0x0FFF) << 12))
#define _OPTIONAL_INTERACTION_REJECT_REPLY (_OPENLCB_GLOBAL_ADDRESSED | ((uint32_t) (MTI_OPTIONAL_INTERACTION_REJECTED & 0x0FFF) << 12))


static interface_can_frame_message_handler_t *_interface;

void CanFrameMessageHandler_initialize(const interface_can_frame_message_handler_t *interface) {

    _interface = (interface_can_frame_message_handler_t*) interface;

}

static void _allocate_and_push(uint32_t identifier, uint8_t buffer_count, payload_bytes_can_t *buffer) {

    can_msg_t* new_msg = CanBufferStore_allocate_buffer();

    if (new_msg) {

        new_msg->payload_count = buffer_count;
        new_msg->identifier = identifier;

        for (int i = 0; i < buffer_count; i++) {

            new_msg->payload[i] = (*buffer)[i];

        }

        // Can not fail Fifo is as large as the number of buffers
        CanBufferFifo_push(new_msg);

    }

}

static void _send_reject(uint16_t source_alias, uint16_t dest_alias, uint16_t mti, uint16_t error_code) {

    payload_bytes_can_t buffer;

    buffer[0] = (uint8_t) (dest_alias >> 8) & 0x00FF;
    buffer[1] = (uint8_t) dest_alias & 0x00FF;
    buffer[2] = (uint8_t) (error_code >> 8) & 0x00FF;
    buffer[3] = (uint8_t) error_code & 0x00FF;

    if (mti == MTI_DATAGRAM) {

        _allocate_and_push((_DATAGRAM_REJECT_REPLY | source_alias), 4, &buffer);

    } else {

        _allocate_and_push((_OPTIONAL_INTERACTION_REJECT_REPLY | source_alias), 4, &buffer);

    }

}

bool _test_for_duplicate_alias_then_send_amr_and_set_duplicate_alias_detected_flag(can_msg_t* can_msg) {

    payload_bytes_can_t buffer;

    // Check for duplicate Alias 
    alias_mapping_t *mapping = _interface->find_alias_mapping(0, CanUtilities_extract_source_alias_from_can_identifier(can_msg));

    if (mapping) {

        CanUtilities_copy_node_id_to_can_payload_buffer(mapping->node_id, &buffer);
        _allocate_and_push(RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | mapping->alias, 6, &buffer);
        _interface->set_mapping_duplicate_alias_detected(mapping->alias);
        
        return true;

    }
    
    return false;

}

void CanFrameMessageHandler_cid(can_msg_t* can_msg) {

    // Check for duplicate Alias 
    alias_mapping_t *mapping = _interface->find_alias_mapping(0, CanUtilities_extract_source_alias_from_can_identifier(can_msg));

    if (mapping) {

        _allocate_and_push(RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | mapping->alias, 0, NULL);

    }

}

void CanFrameMessageHandler_rid(can_msg_t* can_msg) {

    // Check for duplicate Alias 
    _test_for_duplicate_alias_then_send_amr_and_set_duplicate_alias_detected_flag(can_msg);

}

void CanFrameMessageHandler_amd(can_msg_t* can_msg) {

    // Check for duplicate Alias 
    _test_for_duplicate_alias_then_send_amr_and_set_duplicate_alias_detected_flag(can_msg);
    
}

void CanFrameMessageHandler_ame(can_msg_t* can_msg) {

    payload_bytes_can_t buffer;
    alias_mapping_t *mapping = NULL;

    // Check for duplicate Alias  
    if (_test_for_duplicate_alias_then_send_amr_and_set_duplicate_alias_detected_flag(can_msg)) {

        return;

    }

    if (can_msg->payload_count > 0) {

        mapping = _interface->find_alias_mapping(CanUtilities_extract_can_payload_as_node_id(can_msg), 0);

        if (mapping) {

            CanUtilities_copy_node_id_to_can_payload_buffer(mapping->node_id, &buffer);
            _allocate_and_push(RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | mapping->alias, 6, &buffer);

            return;

        }

    }

    uint16_t node_count = _interface->mapping_count();

    for (uint16_t i = 0; i < node_count; i++) {

        mapping = _interface->alias_mapping(i);

        CanUtilities_copy_node_id_to_can_payload_buffer(mapping->node_id, &buffer);
        _allocate_and_push(RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | mapping->alias, 6, &buffer);

    }

}

void CanFrameMessageHandler_amr(can_msg_t* can_msg) {

    _test_for_duplicate_alias_then_send_amr_and_set_duplicate_alias_detected_flag(can_msg);

}

void CanFrameMessageHandler_error_information_report(can_msg_t* can_msg) {
    
    _test_for_duplicate_alias_then_send_amr_and_set_duplicate_alias_detected_flag(can_msg);
    
}

void CanFrameMessageHandler_handle_first_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum_t data_type) {
    
    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    
        // Check if the frame is for us or not
    if (dest_alias > 0) {
        
        if (!_interface->find_alias_mapping(0, dest_alias)) {
            
            return;
            
        }
        
    }

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);
    
    openlcb_msg_t* new_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (new_msg) {

        return _send_reject(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END);

    }
   
    new_msg = _interface->openlcb_buffer_store_allocate_buffer(data_type);

    if (!new_msg) {

        _send_reject(dest_alias, source_alias, mti, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

        return;

    }
   
    new_msg->mti = mti;
    new_msg->source_alias = source_alias;
    new_msg->dest_alias = dest_alias;
    new_msg->state.inprocess = true;

    CanUtilities_copy_can_payload_to_openlcb_payload(new_msg, can_msg, can_buffer_start_index);

    // Can not fail List is as large as the number of buffers
    OpenLcbBufferList_add(new_msg);
   
}

void CanFrameMessageHandler_handle_middle_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index) {
    
    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    
        // Check if the frame is for us or not
    if (dest_alias > 0) {
        
        if (!_interface->find_alias_mapping(0, dest_alias)) {
            
            return;
            
        }
        
    }

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* new_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!new_msg) {

        _send_reject(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

        return;

    }

    CanUtilities_append_can_payload_to_openlcb_payload(new_msg, can_msg, can_buffer_start_index);

    return;

}

void CanFrameMessageHandler_handle_last_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index) {
    
    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    
        // Check if the frame is for us or not
    if (dest_alias > 0) {
        
        if (!_interface->find_alias_mapping(0, dest_alias)) {
            
            return;
            
        }
        
    }

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t * new_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!new_msg) {

        _send_reject(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

        return;

    }

    CanUtilities_append_can_payload_to_openlcb_payload(new_msg, can_msg, can_buffer_start_index);
    new_msg->state.inprocess = false;

    OpenLcbBufferList_release(new_msg);
    OpenLcbBufferFifo_push_existing(new_msg);

}

void CanFrameMessageHandler_handle_single_frame(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum_t data_type) {
    

    uint16_t dest_alias =  CanUtilities_extract_dest_alias_from_can_message(can_msg);
 
    // Check if the frame is for us or not
    if (dest_alias > 0) {
        
        if (!_interface->find_alias_mapping(0, dest_alias)) {
            
            return;
            
        }
        
    }

    openlcb_msg_t* new_msg = _interface->openlcb_buffer_store_allocate_buffer(data_type);

    if (!new_msg) {

        return;

    }

    new_msg->source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    new_msg->dest_alias = dest_alias;
    new_msg->mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);
    new_msg->dest_id = 0;
    new_msg->source_id = 0;
    new_msg->payload_count = 0;
    CanUtilities_copy_can_payload_to_openlcb_payload(new_msg, can_msg, can_buffer_start_index);

    // Can not fail List is as large as the number of buffers
    OpenLcbBufferFifo_push(new_msg);
   
}

void CanFrameMessageHandler_handle_can_legacy_snip(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum_t data_type) {

    // Early implementations did not have the multi-frame bits to use... special case

    openlcb_msg_t* openlcb_msg_inprocess = OpenLcbBufferList_find(
            CanUtilities_extract_source_alias_from_can_identifier(can_msg),
            CanUtilities_extract_dest_alias_from_can_message(can_msg),
            CanUtilities_convert_can_mti_to_openlcb_mti(can_msg)
            );


    if (!openlcb_msg_inprocess) { // Do we have one in process?

        CanFrameMessageHandler_handle_first_frame(can_msg, can_buffer_start_index, data_type);

    } else { // Yes we have one in process   


        if (CanUtilities_count_nulls_in_payloads(openlcb_msg_inprocess, can_msg) < 6)

            CanFrameMessageHandler_handle_middle_frame(can_msg, can_buffer_start_index);

        else

            CanFrameMessageHandler_handle_last_frame(can_msg, can_buffer_start_index);

    };

}

void CanFrameMessageHandler_handle_stream(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum_t data_type) {
    
    
}
