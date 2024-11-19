
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
#include "../../drivers/mcu_driver.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "can_utilities.h"
#include "can_types.h"


#define OFFSET_DEST_ID_IN_PAYLOAD     2
#define OFFSET_DEST_ID_IN_IDENTIFIER  0
#define OFFSET_NO_DEST_ID             0

openlcb_msg_t* _HandleIncomingFirstFrame(can_msg_t* can_msg, uint16_t can_buffer_start_index, uint16_t data_size, openlcb_msg_t* prefetched_find) {

    uint16_t source_alias = ExtractSourceAliasFromCanMessage(can_msg);
    uint16_t dest_alias = ExtractDestAliasFromCanMessage(can_msg);
    uint16_t mti = ConvertCanMtiToOpenLcbMti(can_msg);

    openlcb_msg_t* result = prefetched_find;

    if (!prefetched_find)
        result = BufferList_find(source_alias, dest_alias, mti);

    if (result) {
        
        

        printf("Error: HandleIncoming_CAN_Datagram_FirstFrame found an existing queued message\n"); // TODO: Send error, can't interleave the same message from the same node.  No way to resolve them (Streams you can as it has an ID)

        result = (void*) 0;

    }

    result = BufferList_allocate(data_size);

    if (!result) {

        printf("Error: HandleIncoming_CAN_Datagram_FirstFrame failed to find a free buffer\n"); // TODO Send Error No Buffer

        result = (void*) 0;

    }

    result->mti = mti;
    result->source_alias = source_alias;
    result->dest_alias = dest_alias;
    result->state.inprocess = TRUE;
    CopyCanPayloadToOpenLcbPayload(result, can_msg, can_buffer_start_index);

    return result;

}

openlcb_msg_t* _HandleIncomingMiddleFrame(can_msg_t* can_msg, uint16_t can_buffer_start_index, openlcb_msg_t* prefetched_find) {

    openlcb_msg_t* result = prefetched_find;

    if (!prefetched_find)
        result = BufferList_find(ExtractSourceAliasFromCanMessage(can_msg), ExtractDestAliasFromCanMessage(can_msg), ConvertCanMtiToOpenLcbMti(can_msg));

    if (!result) {

        // TODO: Send error, can't interleave the same message from the same node.  No way to resolve them (Streams you can as it has an ID)

        printf("Error: HandleIncoming_CAN_MiddleFrame got a middle frame but did not get a first\n");

    }

    AppendCanPayloadToOpenLcbPayload(result, can_msg, can_buffer_start_index);

    return result;

}

openlcb_msg_t* _HandleIncomingLastFrame(can_msg_t* can_msg, uint16_t can_buffer_start_index, openlcb_msg_t* prefetched_find) {

    openlcb_msg_t * result = prefetched_find;

    if (!prefetched_find)
        result = BufferList_find(ExtractSourceAliasFromCanMessage(can_msg), ExtractDestAliasFromCanMessage(can_msg), ConvertCanMtiToOpenLcbMti(can_msg));


    if (!result) {
        // TODO: Send error, can't interleave the same message from the same node.  No way to resolve them (Streams you can as it has an ID)

        printf("Error: HandleIncoming_CAN_LastFrame got a last frame but did not get a first\n");

    }

    AppendCanPayloadToOpenLcbPayload(result, can_msg, can_buffer_start_index);
    result->state.inprocess = FALSE;

    BufferList_release(result);
    BufferFifo_push_existing(result);
    
    return result;

}

openlcb_msg_t* _HandleIncomingSingleFrame(can_msg_t* can_msg, uint16_t can_buffer_start_index, uint16_t data_size) {

    openlcb_msg_t* new_msg = BufferFifo_push(data_size);

    if (new_msg) {

        new_msg->mti = ConvertCanMtiToOpenLcbMti(can_msg);
        new_msg->source_alias = ExtractSourceAliasFromCanMessage(can_msg);
        new_msg->dest_alias = ExtractDestAliasFromCanMessage(can_msg);
        new_msg->dest_id = 0;
        new_msg->source_id = 0;
        new_msg->payload_count = 0;
        CopyCanPayloadToOpenLcbPayload(new_msg, can_msg, can_buffer_start_index);

        return new_msg;

    }

    return (void*) 0;

}

void _HandleIncomingCanLegacySNIP(can_msg_t* source_can_msg, uint16_t can_buffer_start_index, uint16_t data_size) {

    // Early implementations did not have the multi-frame bits to use... special case

    openlcb_msg_t* openlcb_msg_inprocess = BufferList_find(ExtractSourceAliasFromCanMessage(source_can_msg), ExtractDestAliasFromCanMessage(source_can_msg), ConvertCanMtiToOpenLcbMti(source_can_msg));


    if (!openlcb_msg_inprocess) { // Do we have one in process?

        _HandleIncomingFirstFrame(source_can_msg, can_buffer_start_index, data_size, openlcb_msg_inprocess);

    } else { // Yes we have one in process   

        if (CountNullsInPayload(openlcb_msg_inprocess, source_can_msg) < 6)

            _HandleIncomingMiddleFrame(source_can_msg, can_buffer_start_index, openlcb_msg_inprocess);

        else

            _HandleIncomingLastFrame(source_can_msg, can_buffer_start_index, openlcb_msg_inprocess);

    };

    return;

}

void _HandleIncomingGlobalAddressedMessages(can_msg_t* msg) {

    if (msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT) {

        // TODO:  Eventually we should only process this if the message is addressed to us to save bandwidth internally...


   //     if (!IsCanMessageAddressedToNode(&openlcb_nodes, msg))
   //         return;

        uint16_t can_mti = (msg->identifier & 0x00FFF000) >> 12;

        switch (msg->payload[0] & 0xF0) { // Extract Framing Bits

            case MULTIFRAME_ONLY:

                if (can_mti == MTI_SIMPLE_NODE_INFO_REPLY) {

                    _HandleIncomingCanLegacySNIP(msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_SNIP);

                } else

                    _HandleIncomingSingleFrame(msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_BASIC);

                break;

            case MULTIFRAME_FIRST:

                if (can_mti == MTI_SIMPLE_NODE_INFO_REPLY)

                    _HandleIncomingFirstFrame(msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_SNIP, (void*) 0);
                else

                    // TODO: This could be dangerous if a future message used more than 2 frames.... (larger than LEN_MESSAGE_BYTES_BASIC)
                    _HandleIncomingFirstFrame(msg, OFFSET_DEST_ID_IN_PAYLOAD, LEN_MESSAGE_BYTES_BASIC, (void*) 0);

                break;

            case MULTIFRAME_MIDDLE:

                _HandleIncomingMiddleFrame(msg, OFFSET_DEST_ID_IN_PAYLOAD, (void*) 0);

                break;

            case MULTIFRAME_FINAL:

                _HandleIncomingLastFrame(msg, OFFSET_DEST_ID_IN_PAYLOAD, (void*) 0);

                break;
        }
    } else { // No Destination Address

        _HandleIncomingSingleFrame(msg, OFFSET_NO_DEST_ID, LEN_MESSAGE_BYTES_BASIC);

    }

}

void _StatemachineIncomingCan(uint8_t channel, can_msg_t* msg) {

    if (CanUtilities_is_openlcb_message(msg)) { // Only handle OpenLCB Messages

        switch (msg->identifier & MASK_CAN_FRAME_TYPE) {

            case CAN_FRAME_TYPE_GLOBAL_ADDRESSED:

                _HandleIncomingGlobalAddressedMessages(msg);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_ONLY:

                _HandleIncomingSingleFrame(msg, OFFSET_DEST_ID_IN_IDENTIFIER, LEN_MESSAGE_BYTES_BASIC);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_FIRST:

                _HandleIncomingFirstFrame(msg, OFFSET_DEST_ID_IN_IDENTIFIER, LEN_MESSAGE_BYTES_DATAGRAM, (void*) 0);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:

                _HandleIncomingMiddleFrame(msg, OFFSET_DEST_ID_IN_IDENTIFIER, (void*) 0);
                break;

            case CAN_FRAME_TYPE_DATAGRAM_FINAL:

                _HandleIncomingLastFrame(msg, OFFSET_DEST_ID_IN_IDENTIFIER, (void*) 0);
                break;

            case CAN_FRAME_TYPE_RESERVED:

                break;
            case CAN_FRAME_TYPE_STREAM:

                break;
        }

    } else { // CAN Control Messages

   //     HandleIncomingCanFrameMessages(msg);

    };

};

// Call on startup to initialize variables and callbacks

void CanRxStatemachine_initialize() {

    // The mcu_driver.h file exports a function pointer that is used to hook into the incoming CAN message stream
    // This allows the mcu_driver to call into this unit with incoming CAN frames.
    Incoming_CAN_Message_Callback_Func = &_StatemachineIncomingCan;

}


