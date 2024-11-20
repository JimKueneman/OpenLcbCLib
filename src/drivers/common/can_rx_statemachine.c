
/*
 * File:   can_incoming_statemachine.c
 * Author: Jim Kueneman
 *
 * Created on February 7, 2024, 6:03 AM
 * 
 */

// BIG TODO:  Decide how to cross the boundry from and possible interrupt/thread driven incoming can message stream and if we reach into the node
//            list from here to quickly reply to important messages or just put them in the buffer and let the main loop deal with it when
//            it get a chance.....


#include <libpic30.h>

#include "xc.h"
#include "stdio.h" // printf
#include "../../drivers/mcu_driver.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_node_buffer.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "can_utilities.h"
#include "can_types.h"


#define OFFSET_DEST_ID_IN_PAYLOAD     2
#define OFFSET_DEST_ID_IN_IDENTIFIER  0
#define OFFSET_NO_DEST_ID             0


openlcb_msg_t* _handle_first_frame(can_msg_t* can_msg, uint16_t can_buffer_start_index, uint16_t data_size, openlcb_msg_t* prefetched_find) {

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_message(can_msg);
    uint16_t dest_alias = CanUtilties_extract_dest_alias_from_can_message(can_msg);
    uint16_t mti = CanUtilties_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* result = prefetched_find;

    if (!prefetched_find)
        result = BufferList_find(source_alias, dest_alias, mti);

    if (result) {

        // TODO: Send error, can't interleave the same message from the same node.  No way to resolve them (Streams you can as it has an ID)
        printf("Error: HandleIncoming_CAN_Datagram_FirstFrame found an existing queued message\n");

        result = (void*) 0;

    }

    result = BufferList_allocate(data_size);

    if (!result) {

        // TODO Send Error No Buffer
        printf("Error: HandleIncoming_CAN_Datagram_FirstFrame failed to find a free buffer\n");

        result = (void*) 0;

    }

    result->mti = mti;
    result->source_alias = source_alias;
    result->dest_alias = dest_alias;
    result->state.inprocess = TRUE;

    CanUtilities_copy_can_payload_to_openlcb_payload(result, can_msg, can_buffer_start_index);


    return result;

}

openlcb_msg_t* _handle_middle_frame(can_msg_t* can_msg, uint16_t can_buffer_start_index, openlcb_msg_t* prefetched_find) {

    openlcb_msg_t* result = prefetched_find;

    if (!prefetched_find)
        result = BufferList_find(
            CanUtilities_extract_source_alias_from_can_message(can_msg),
            CanUtilties_extract_dest_alias_from_can_message(can_msg),
            CanUtilties_convert_can_mti_to_openlcb_mti(can_msg)
            );

    if (!result) {

        // TODO: Send error, can't interleave the same message from the same node.  No way to resolve them (Streams you can as it has an ID)
        printf("Error: HandleIncoming_CAN_MiddleFrame got a middle frame but did not get a first\n");

    }

    CanUtilities_append_can_payload_to_openlcb_payload(result, can_msg, can_buffer_start_index);

    return result;

}

openlcb_msg_t* _handle_last_frame(can_msg_t* can_msg, uint16_t can_buffer_start_index, openlcb_msg_t* prefetched_find) {

    openlcb_msg_t * result = prefetched_find;

    if (!prefetched_find)
        result = BufferList_find(
            CanUtilities_extract_source_alias_from_can_message(can_msg),
            CanUtilties_extract_dest_alias_from_can_message(can_msg),
            CanUtilties_convert_can_mti_to_openlcb_mti(can_msg)
            );


    if (!result) {

        // TODO: Send error, can't interleave the same message from the same node.  No way to resolve them (Streams you can as it has an ID)
        printf("Error: HandleIncoming_CAN_LastFrame got a last frame but did not get a first\n");

    }

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

        _handle_first_frame(can_msg, can_buffer_start_index, data_size, openlcb_msg_inprocess);

    } else { // Yes we have one in process   


        if (CanUtilities_count_nulls_in_payloads(openlcb_msg_inprocess, can_msg) < 6)

            _handle_middle_frame(can_msg, can_buffer_start_index, openlcb_msg_inprocess);

        else

            _handle_last_frame(can_msg, can_buffer_start_index, openlcb_msg_inprocess);

    };

    return;

}

void _handle_global_addressed_messages(can_msg_t* can_msg) {

    if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT) {

        // TODO:  Eventually we should only process this if the message is addressed to us to save bandwidth internally...
        //     if (!IsCanMessageAddressedToNode(&openlcb_nodes, msg))
        //         return;

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

                    _handle_first_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_SNIP, (void*) 0);

                else

                    // TODO: This could be dangerous if a future message used more than 2 frames.... (larger than LEN_MESSAGE_BYTES_BASIC)
                    _handle_first_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_BASIC, (void*) 0);

                break;

            case MULTIFRAME_MIDDLE:

                _handle_middle_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, (void*) 0);

                break;

            case MULTIFRAME_FINAL:

                _handle_last_frame(can_msg, OFFSET_DEST_ID_IN_PAYLOAD, (void*) 0);

                break;
        }
    } else { // No Destination Address

        _handle_single_frame(can_msg, OFFSET_NO_DEST_ID, LEN_MESSAGE_BYTES_BASIC);

    }

}

//void _flush_alias_node_id_mappings() {
//
//    int i = 0;
//
//    i = i + 1;
//
//    //   TODO:  
//
//}
//
//openlcb_node_t* _test_can_message_for_alias_conflict(can_msg_t* can_msg) {
//
//    // TODO: HOW TO HANDLE THIS ACROSS PROCESS BOUNDRIES
//
//    return (void*) 0;
//
//}
//
//uint8_t _hard_alias_conflict_check(can_msg_t* can_msg) {
//
//    openlcb_node_t* conflicting_node = _test_can_message_for_alias_conflict(can_msg);
//
//    if (conflicting_node) {
//
//        can_msg_t* new_msg;
//
//        new_msg = CanBufferFifo_push();
//
//        if (!new_msg) {
//
//            printf("Unable to allocate buffer _handle_hard_alias_conflict_check");
//            return FALSE;
//
//        }
//
//
//        CanUtilities_copy_node_id_to_payload(new_msg, conflicting_node->id, 0);
//        new_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | conflicting_node->alias;
//
//        conflicting_node->state.permitted = 0;
//        conflicting_node->state.initalized = 0;
//        conflicting_node->state.run = RUNSTATE_GENERATE_SEED;
//
//        return TRUE;
//
//    }
//
//    return FALSE;
//
//}
//
//uint8_t _soft_alias_conflict_check(can_msg_t* can_msg) {
//
//    openlcb_node_t* conflicting_node = _test_can_message_for_alias_conflict(can_msg);
//
//    if (conflicting_node) {
//
//        can_msg_t* new_msg;
//
//        new_msg = CanBufferFifo_push();
//
//        if (!new_msg) {
//
//            printf("Unable to allocate buffer _handle_hard_alias_conflict_check");
//            return FALSE;
//
//        }
//
//        new_msg->payload_count = 0;
//        new_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | conflicting_node->alias;
//
//        return TRUE;
//
//    }
//
//    return FALSE;
//
//}
//
//uint8_t _reply_to_ame_control_frame(can_msg_t* can_msg) {
//
//    can_msg_t new_msg;
//
//    // TODO: HOW TO HANDLE THIS ACROSS PROCESS BOUNDRIES
//
//    //    for (int iIndex = 0; iIndex < LEN_OPENLCB_NODE_ARRAY; iIndex++) {
//    //
//    //        if (openlcb_nodes.node[iIndex].state.permitted) {
//    //
//    //            if ((msg->payload_count == 0) || (CanUtilities_extract_can_payload_as_node_id(&msg->payload) == openlcb_nodes.node[iIndex].id)) {
//    //
//    //                new_msg = CanBufferFifo_push();
//    //
//    //                if (!new_msg) {
//    //
//    //                    printf("Unable to allocate buffer _handle_hard_alias_conflict_check");
//    //                    return FALSE;
//    //
//    //                }
//    //
//    //                CanUtilities_copy_node_id_to_payload(&new_msg, openlcb_nodes.node[iIndex].id, 0);
//    //                new_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | openlcb_nodes.node[iIndex].alias;
//    //                
//    //                return TRUE;
//    //
//    //            }
//    //
//    //        }
//
//    return FALSE;
//
//}

void _handle_rid_control_frame(can_msg_t* can_msg) {

    //   _hard_alias_conflict_check(can_msg);

    // for now just copy and handle it in the main loop, may revisit but this is the 
    // easiest to deal with crossing interrupt/processes

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) {

        printf("Could not allocate a CAN buffer: _handle_rid_control_frame");

        return;

    }

    CanUtilities_copy_can_message(can_msg, new_can_msg);

}

void _handle_amd_control_frame(can_msg_t* can_msg) {

    //    _hard_alias_conflict_check(can_msg);

    // for now just copy and handle it in the main loop, may revisit but this is the 
    // easiest to deal with crossing interrupt/processes

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) {

        printf("Could not allocate a CAN buffer: _handle_amd_control_frame");

        return;

    }

   CanUtilities_copy_can_message(can_msg, new_can_msg);

}

void _handle_ame_control_frame(can_msg_t* can_msg) {

    // Someone is requesting we reply with Alias Mapping Definitions for our Node(s)

    //    if (can_msg->payload_count == 0)
    //        _flush_alias_node_id_mappings();
    //
    //    if (_hard_alias_conflict_check(can_msg))
    //        return;
    //
    //    _reply_to_ame_control_frame(can_msg);

    // for now just copy and handle it in the main loop, may revisit but this is the 
    // easiest to deal with crossing interrupt/processes

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) {

        printf("Could not allocate a CAN buffer: _handle_ame_control_frame");

        return;

    }

    CanUtilities_copy_can_message(can_msg, new_can_msg);

}

void _handle_amr_control_frame(can_msg_t* can_msg) {

    // for now just copy and handle it in the main loop, may revisit but this is the 
    // easiest to deal with crossing interrupt/processes


    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) {

        printf("Could not allocate a CAN buffer: _handle_amr_control_frame");

        return;

    }

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

    // called in the context of a possible interrupt, make sure the main loop it protecting this with the 
    // mcu_driver.h Pause_CAN_Receive and Resume_CAN_Receive when it needs to modify the Node Buffer.

    //   _soft_alias_conflict_check(can_msg);
    
        // for now just copy and handle it in the main loop, may revisit but this is the 
    // easiest to deal with crossing interrupt/processes

    can_msg_t* new_can_msg = CanBufferFifo_push();

    if (!new_can_msg) {

        printf("Could not allocate a CAN buffer: _handle_incoming_cid");

        return;

    }
    
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

                _handle_first_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, LEN_MESSAGE_BYTES_DATAGRAM, (void*) 0);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:

                _handle_middle_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, (void*) 0);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_FINAL:

                _handle_last_frame(can_msg, OFFSET_DEST_ID_IN_IDENTIFIER, (void*) 0);
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
    Incoming_CAN_Message_Callback_Func = &_state_machine_incoming_can;


}





