
/*
 * File:   can_outgoing_msg_statemachine.c
 * Author: Jim Kueneman
 *
 * Created on November 7, 2024, 2:57 PM
 */

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
#include "../mcu_driver.h"


void InitializeCanOutgoingStateMachine() {


}

uint32_t Construct_Identfier_For_Datagram_OnlyFrameMsg(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_ONLY | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t Construct_Identfier_For_Datagram_FirstFrameMsg(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FIRST | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t Construct_Identfier_For_Datagram_MiddleFrameMsg(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_MIDDLE | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t Construct_Identfier_For_Datagram_LastFrameMsg(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FINAL | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

uint32_t Construct_Identfier_For_UnAddressMsg(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

uint32_t Construct_Identfier_For_AddressedMsg(openlcb_msg_t* openlcb_msg) {

    return (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

void Handle_OpenLcb_Msg_Transmit(can_msg_t* can_msg) {

    TransmitRawCanFrame(TX_CHANNEL_OPENLCB_MSG, can_msg);

}

void Handle_OpenLcb_Msg_Last_Frame_Transmit(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    TransmitRawCanFrame(TX_CHANNEL_OPENLCB_MSG, can_msg);
    ClearOpenLcbInprocessBuffer(&outgoing_fifo);
    FreeOpenLcbMessage(openlcb_msg);

}

void Handle_OpenLcb_Msg_Only_Frame_Transmit(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    TransmitRawCanFrame(TX_CHANNEL_OPENLCB_MSG, can_msg);
    ClearOpenLcbInprocessBuffer(&outgoing_fifo);
    FreeOpenLcbMessage(openlcb_msg);

}

void Handle_Outgoing_Datagram_Only_Frame(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    can_msg->identifier = Construct_Identfier_For_Datagram_OnlyFrameMsg(openlcb_msg);
    Handle_OpenLcb_Msg_Only_Frame_Transmit(can_msg, openlcb_msg);

}

void Handle_Outgoing_Datagram_First_Frame(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    can_msg->identifier = Construct_Identfier_For_Datagram_FirstFrameMsg(openlcb_msg);
    Handle_OpenLcb_Msg_Transmit(can_msg);

}

void Handle_Outgoing_Datagram_Middle_Frame(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    can_msg->identifier = Construct_Identfier_For_Datagram_MiddleFrameMsg(openlcb_msg);
    Handle_OpenLcb_Msg_Transmit(can_msg);

}

void Handle_Outgoing_Datagram_Last_Frame(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    can_msg->identifier = Construct_Identfier_For_Datagram_LastFrameMsg(openlcb_msg);
    Handle_OpenLcb_Msg_Last_Frame_Transmit(can_msg, openlcb_msg);

}

void Handle_Outgoing_Datagram(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg, uint16_t* tx_payload_index) {

    CopyOpenLcbPayloadToCanPayload(openlcb_msg, can_msg, 0, tx_payload_index);

    if (openlcb_msg->payload_count <= 8)

        Handle_Outgoing_Datagram_Only_Frame(can_msg, openlcb_msg);

    else if (*tx_payload_index <= LEN_CAN_DATA_ARRAY)

        Handle_Outgoing_Datagram_First_Frame(can_msg, openlcb_msg);

    else if (*tx_payload_index < openlcb_msg->payload_count)

        Handle_Outgoing_Datagram_Middle_Frame(can_msg, openlcb_msg);

    else

        Handle_Outgoing_Datagram_Last_Frame(can_msg, openlcb_msg);

}

void Handle_Outgoing_UnaddresedMsg(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg, uint16_t* tx_payload_index) {

    if (openlcb_msg->payload_count <= LEN_CAN_DATA_ARRAY) { // single frame

        uint16_t openlcb_msg_payload_index = 0;

        CopyOpenLcbPayloadToCanPayload(openlcb_msg, can_msg, 0, &openlcb_msg_payload_index);
        can_msg->identifier = Construct_Identfier_For_UnAddressMsg(openlcb_msg);
        Handle_OpenLcb_Msg_Last_Frame_Transmit(can_msg, openlcb_msg);

    } else { // multi frame

        // TODO Is there such a thing as a unaddressed multi frame?

    }

}

void Handle_Outgoing_AddressedMsg_Only_Frame(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    SetMultiFrameFlag(&can_msg->payload[0], MULTIFRAME_ONLY);
    Handle_OpenLcb_Msg_Last_Frame_Transmit(can_msg, openlcb_msg);

}

void Handle_Outgoing_AddressedMsg_First_Frame(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    SetMultiFrameFlag(&can_msg->payload[0], MULTIFRAME_FIRST);
    Handle_OpenLcb_Msg_Transmit(can_msg);

}

void Handle_Outgoing_AddressedMsg_Middle(can_msg_t* can_msg) {

    SetMultiFrameFlag(&can_msg->payload[0], MULTIFRAME_MIDDLE);
    Handle_OpenLcb_Msg_Transmit(can_msg);

}

void Handle_Outgoing_AddressedMsg_Last(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    SetMultiFrameFlag(&can_msg->payload[0], MULTIFRAME_FINAL);
    Handle_OpenLcb_Msg_Last_Frame_Transmit(can_msg, openlcb_msg);

}

void Handle_Outgoing_AddressedMsg_Destination_Address(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {
    
    can_msg->payload[0] = (openlcb_msg->dest_alias >> 8) & 0xFF; // Setup the first two CAN data bytes with the destination address
    can_msg->payload[1] = openlcb_msg->dest_alias & 0xFF;
    
}

void Handle_Outgoing_AddressedMsg(can_msg_t* can_msg, openlcb_msg_t* openlcb_msg, uint16_t* tx_payload_index) {
    

    Handle_Outgoing_AddressedMsg_Destination_Address(can_msg, openlcb_msg);
    
    can_msg->identifier = Construct_Identfier_For_AddressedMsg(openlcb_msg);
    
    CopyOpenLcbPayloadToCanPayload(openlcb_msg, can_msg, 2, tx_payload_index);

    if (openlcb_msg->payload_count <= 6)  // Account for 2 bytes used for dest alias

        Handle_Outgoing_AddressedMsg_Only_Frame(can_msg, openlcb_msg);

    if (*tx_payload_index <= 6) // Account for 2 bytes used for dest alias

        Handle_Outgoing_AddressedMsg_First_Frame(can_msg, openlcb_msg);

    else if (*tx_payload_index < openlcb_msg->payload_count)

        Handle_Outgoing_AddressedMsg_Middle(can_msg);

    else

        Handle_Outgoing_AddressedMsg_Last(can_msg, openlcb_msg);

}

uint8_t Try_Load_Next_ControlFrame_Message(can_msg_t* can_msg) {

    if (IsCanControlFrameTxBufferEmpty(&outgoing_can_control_frame_fifo)) {

        if (PopCanControlFrameMessage(&outgoing_can_control_frame_fifo, can_msg)) {

            SetCanControlFrameTxBuffer(&outgoing_can_control_frame_fifo, can_msg);

            return TRUE;

        } else

            return FALSE;

    } else

        return TRUE;

}

void Handle_CAN_Frame_Transmit() { // Function assumes that TX_CHANNEL_CAN_CONTROL is clear

    can_msg_t* tx_can_msg = GetCanControlFrameTxBuffer(&outgoing_can_control_frame_fifo);

    if (Try_Load_Next_ControlFrame_Message(tx_can_msg)) {

        TransmitRawCanFrame(TX_CHANNEL_CAN_CONTROL, tx_can_msg);

        ClearCanControlFrameTxBuffer(&outgoing_can_control_frame_fifo);
        
    }

}

uint8_t Try_Load_Next_OpenLcb_Message() {

    if (IsOpenLcbInprocessBufferEmpty(&outgoing_fifo)) {

        openlcb_msg_t* tx_msg = PopOpenLcbMessage(&outgoing_fifo);

        if (tx_msg) {

            SetOpenLcbInprocessBuffer(&outgoing_fifo, tx_msg);

            return TRUE;

        } else

            return FALSE;

    } else

        return TRUE;

}

void Handle_OpenLcb_Transmit() { // Function assumes that TX_CHANNEL_OPENLCB_MSG is clear

    can_msg_t can_msg;

    if (Try_Load_Next_OpenLcb_Message()) {

        openlcb_msg_t* tx_msg = GetOpenLcbInprocessMsgPtr(&outgoing_fifo);
        uint16_t* tx_payload_index = GetOpenLcbInprocessPayloadIndexPtr(&outgoing_fifo);

        if (IsAddressedOpenLcbMessage(tx_msg)) {

            switch (tx_msg->mti) {

                case MTI_DATAGRAM:
                {
                    Handle_Outgoing_Datagram(&can_msg, tx_msg, tx_payload_index);

                    return;

                }

                case MTI_STREAM_COMPLETE:
                case MTI_STREAM_INIT_REPLY:
                case MTI_STREAM_INIT_REQUEST:
                case MTI_STREAM_PROCEED:
                {

                    return;

                }

                default:

                    Handle_Outgoing_AddressedMsg(&can_msg, tx_msg, tx_payload_index);

                    return;

            }


        } else {

            Handle_Outgoing_UnaddresedMsg(&can_msg, tx_msg, tx_payload_index);

            return;

        }

    }

}

void StatemachineOutgoingCanFrameBuffer() {

    // As of today this is all run in the context of the main loop so no need to disable interrupts
    // Insertion and Deletion of the buffers is handled in the main loop as of 11/11/2024

    if (IsCanTxBufferClear(TX_CHANNEL_CAN_CONTROL))
        Handle_CAN_Frame_Transmit();

    if (IsCanTxBufferClear(TX_CHANNEL_OPENLCB_MSG))
        Handle_OpenLcb_Transmit();

}
