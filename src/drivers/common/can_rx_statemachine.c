
/*
 * File:   can_incoming_statemachine.c
 * Author: Jim Kueneman
 *
 * Created on February 7, 2024, 6:03 AM
 * 
 */


#include <libpic30.h>

#include "xc.h"
#include "stdio.h" // printf
#include "../../drivers/driver_can.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_node.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "can_utilities.h"
#include "can_types.h"


#define OFFSET_DEST_ID_IN_PAYLOAD     2
#define OFFSET_DEST_ID_IN_IDENTIFIER  0
#define OFFSET_NO_DEST_ID             0

uint32_t _ack_reject_identifier(uint16_t source_alias) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED | ((uint32_t) (MTI_DATAGRAM_REJECTED_REPLY & 0x0FFF) << 12) | source_alias);

}

uint32_t _oir_identifier(uint16_t source_alias) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED | ((uint32_t) (MTI_OPTIONAL_INTERACTION_REJECTED & 0x0FFF) << 12) | source_alias);

}

openlcb_msg_t* _send_reject(uint16_t source_alias, uint16_t dest_alias, uint16_t mti, uint16_t error_code) {

    // Does not work... this is for incoming that need processed.  It will not get just sent out....
    can_msg_t* can_msg_error = CanBufferFifo_push();

    if (can_msg_error) {

        if (mti == MTI_DATAGRAM)
            can_msg_error->identifier = _ack_reject_identifier(source_alias);
        else
            can_msg_error->identifier = _oir_identifier(source_alias);

        can_msg_error->payload[0] = (uint8_t) (dest_alias >> 8) & 0x00FF;
        can_msg_error->payload[1] = (uint8_t) dest_alias & 0x00FF;
        can_msg_error->payload[2] = (uint8_t) (error_code >> 8) & 0x00FF;
        can_msg_error->payload[3] = (uint8_t) error_code & 0x00FF;
        
        can_msg_error->state.direct_tx = TRUE;
        
        can_msg_error->payload_count = 4;

    }
    
    return (void*) 0;

}

openlcb_msg_t* _handle_first_frame(can_msg_t* can_msg, uint16_t can_buffer_start_index, uint16_t data_size) {

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_message(can_msg);
    uint16_t dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
    uint16_t mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* result = BufferList_find(source_alias, dest_alias, mti);

    if (result) 
        return _send_reject(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END);

    result = BufferList_allocate(data_size);

    if (!result) 
        return _send_reject(dest_alias, source_alias, mti, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);


    result->mti = mti;
    result->source_alias = source_alias;
    result->dest_alias = dest_alias;
    result->state.inprocess = TRUE;

    CanUtilities_copy_can_payload_to_openlcb_payload(result, can_msg, can_buffer_start_index);

    return result;

}

openlcb_msg_t* _handle_middle_frame(can_msg_t* can_msg, uint16_t can_buffer_start_index) {

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_message(can_msg);
    uint16_t dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
    uint16_t mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* result = BufferList_find(source_alias, dest_alias, mti);

    if (!result) 
        return _send_reject(source_alias, dest_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

    CanUtilities_append_can_payload_to_openlcb_payload(result, can_msg, can_buffer_start_index);

    return result;

}

openlcb_msg_t* _handle_last_frame(can_msg_t* can_msg, uint16_t can_buffer_start_index) {

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_message(can_msg);
    uint16_t dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
    uint16_t mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t * result = BufferList_find(source_alias, dest_alias, mti);

    if (!result) 
        return _send_reject(source_alias, dest_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

    CanUtilities_append_can_payload_to_openlcb_payload(result, can_msg, can_buffer_start_index);
    result->state.inprocess = FALSE;

    BufferList_release(result);
    BufferFifo_push_existing(result);

    return result;

}

openlcb_msg_t* _handle_single_frame(can_msg_t* can_msg, uint16_t can_buffer_start_index, uint16_t data_size) {

    openlcb_msg_t* new_msg = BufferFifo_push(data_size);

    if (new_msg) {

        new_msg->source_alias = CanUtilities_extract_source_alias_from_can_message(can_msg);
        new_msg->dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
        new_msg->mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);
        new_msg->dest_id = 0;
        new_msg->source_id = 0;
        new_msg->payload_count = 0;
        CanUtilities_copy_can_payload_to_openlcb_payload(new_msg, can_msg, can_buffer_start_index);

        return new_msg;

    }

    return (void*) 0;

}

void _handle_can_legacy_snip(can_msg_t* can_msg, uint16_t can_buffer_start_index, uint16_t data_size) {

    // Early implementations did not have the multi-frame bits to use... special case

    openlcb_msg_t* openlcb_msg_inprocess = BufferList_find(
            CanUtilities_extract_source_alias_from_can_message(can_msg),
            CanUtilties_extract_dest_alias_from_can_message(can_msg),
            CanUtilties_convert_can_mti_to_openlcb_mti(can_msg)
            );


    if (!openlcb_msg_inprocess) { // Do we have one in process?

        _handle_first_frame(can_msg, can_buffer_start_index, data_size);

    } else { // Yes we have one in process   


        if (CanUtilities_count_nulls_in_payloads(openlcb_msg_inprocess, can_msg) < 6)

            _handle_middle_frame(can_msg, can_buffer_start_index);

        else

            _handle_last_frame(can_msg, can_buffer_start_index);

    };

    return;

}

void _handle_global_addressed_messages(can_msg_t* can_msg) {

    if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT) {

        uint16_t can_mti = CanUtilties_extract_can_mti_from_can_identifier(can_msg);

        switch (can_msg->payload[0] & 0xF0) { // Extract Framing Bits

            case MULTIFRAME_ONLY:

                if (can_mti == MTI_SIMPLE_NODE_INFO_REPLY) {

                    _handle_can_legacy_snip(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_SNIP);

                } else

                    _handle_single_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_BASIC);

                break;

            case MULTIFRAME_FIRST:

                if (can_mti == MTI_SIMPLE_NODE_INFO_REPLY)

                    _handle_first_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_SNIP);

                else

                    // TODO: This could be dangerous if a future message used more than 2 frames.... (larger than LEN_MESSAGE_BYTES_BASIC)
                    _handle_first_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_BASIC);

                break;

            case MULTIFRAME_MIDDLE:

                _handle_middle_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD);

                break;

            case MULTIFRAME_FINAL:

                _handle_last_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD);

                break;
        }
    } else { // No Destination Address

        _handle_single_frame(can_msg, OFFSET_NO_DEST_ID, LEN_MESSAGE_BYTES_BASIC);

    }

}

void _handle_rid_control_frame(can_msg_t* can_msg) {

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) 
        return;

    CanUtilities_copy_can_message(can_msg, new_can_msg);
}

void _handle_amd_control_frame(can_msg_t* can_msg) {

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) 
        return;

    CanUtilities_copy_can_message(can_msg, new_can_msg);
}

void _handle_ame_control_frame(can_msg_t* can_msg) {

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) 
        return;

    CanUtilities_copy_can_message(can_msg, new_can_msg);
}

void _handle_amr_control_frame(can_msg_t* can_msg) {

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) 
        return;

    CanUtilities_copy_can_message(can_msg, new_can_msg);

}

void _handle_incoming_can_variable_field(can_msg_t* can_msg) {

    switch (can_msg->identifier & MASK_CAN_VARIABLE_FIELD) {

        case CAN_CONTROL_FRAME_RID: // Reserve ID

            _handle_rid_control_frame(can_msg);
            break;

        case CAN_CONTROL_FRAME_AMD: // Alias Map Definition

            _handle_amd_control_frame(can_msg);
            break;

        case CAN_CONTROL_FRAME_AME:

            _handle_ame_control_frame(can_msg);
            break;

        case CAN_CONTROL_FRAME_AMR:

            _handle_amr_control_frame(can_msg);
            break;

        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0:
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1:
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2:
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3:
            // Advanced feature for gateways/routers/etc.
            break;

    }

}

void _handle_incoming_cid(can_msg_t* can_msg) {

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) 
        return;

    CanUtilities_copy_can_message(can_msg, new_can_msg);

}

void _handle_incoming_can_frame_sequence_number(can_msg_t* can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

        case CAN_CONTROL_FRAME_CID7:
        case CAN_CONTROL_FRAME_CID6:
        case CAN_CONTROL_FRAME_CID5:
        case CAN_CONTROL_FRAME_CID4:

            _handle_incoming_cid(can_msg);

            return;

        case CAN_CONTROL_FRAME_CID3:
        case CAN_CONTROL_FRAME_CID2:
        case CAN_CONTROL_FRAME_CID1:

            return;
    }

}

void _state_machine_incoming_can(uint8_t channel, can_msg_t* can_msg) {


    if (CanUtilities_is_openlcb_message(can_msg)) { // Only handle OpenLCB Messages

        switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

            case CAN_FRAME_TYPE_GLOBAL_ADDRESSED:

                _handle_global_addressed_messages(can_msg);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_ONLY:

                _handle_single_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, LEN_MESSAGE_BYTES_BASIC);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_FIRST:

                _handle_first_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, LEN_MESSAGE_BYTES_DATAGRAM);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:

                _handle_middle_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_FINAL:

                _handle_last_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER);
                break;

            case CAN_FRAME_TYPE_RESERVED:

                break;
            case CAN_FRAME_TYPE_STREAM:

                break;
        }

    } else { // CAN Control Messages

        if (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

            _handle_incoming_can_frame_sequence_number(can_msg);

        } else {

            _handle_incoming_can_variable_field(can_msg);

        }

    };

};

// Call on startup to initialize variables and callbacks

void CanRxStatemachine_initialize() {

    // The mcu_driver.h file exports a function pointer that is used to hook into the incoming CAN message stream
    // This allows the mcu_driver to call into this unit with incoming CAN frames.

    DriverCan_Initialization(&_state_machine_incoming_can);


}

// TODO: CREATE A TIMEOUT TO FLUSH OUT ABANDONDOED MESSAGES......





