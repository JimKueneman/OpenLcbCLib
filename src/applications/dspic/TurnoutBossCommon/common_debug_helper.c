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
 * \file debug.c
 *
 * Functions to print out various structures for debugging aid.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "stdio.h"  // printf
#include "common_debug_helper.h"

#ifdef MPLAB
#include "xc.h"
#include "../../../openlcb/openlcb_defines.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../drivers/common/can_types.h"
#include "../../../openlcb/openlcb_buffer_fifo.h"
#include "../../../openlcb/openlcb_defines.h"
#else
#include "src/openlcb/openlcb_defines.h"
#include "src/openlcb/openlcb_types.h"
#include "src/drivers/common/can_types.h"
#include "src/openlcb/openlcb_buffer_fifo.h"
#include "src/openlcb/openlcb_defines.h"
#endif


void PrintInt64(uint64_olcb_t n) {

    printf("0x%04X", (uint16_olcb_t) ((n >> 48) & 0xFFFF));
    printf("%04X", (uint16_olcb_t) ((n >> 32) & 0xFFFF));
    printf("%04X", (uint16_olcb_t) ((n >> 16) & 0xFFFF));
    printf("%04X\n", (uint16_olcb_t) ((n >> 0) & 0xFFFF));

}

void PrintAlias(uint16_olcb_t alias) {

    printf("Alias: %04X\n", alias);

}

void PrintNodeID(uint64_olcb_t node_id) {

    printf("NodeID: 0x%04X", (uint16_olcb_t) (node_id >> 32));
    printf("%04X", (uint16_olcb_t) (node_id >> 16));
    printf("%04X\n", (uint16_olcb_t) (node_id >> 0));

}

void PrintAliasAndNodeID(uint16_olcb_t alias, uint64_olcb_t node_id) {

    PrintAlias(alias);
    PrintNodeID(node_id);

}

void PrintMtiName(uint16_olcb_t mti) {
    switch (mti) {
        case MTI_INITIALIZATION_COMPLETE:
            printf("MTI_INITIALIZATION_COMPLETE");
            break;
        case MTI_INITIALIZATION_COMPLETE_SIMPLE:
            printf("MTI_INITIALIZATION_COMPLETE_SIMPLE");
            break;
        case MTI_VERIFY_NODE_ID_ADDRESSED:
            printf("MTI_VERIFY_NODE_ID_ADDRESSED");
            break;
        case MTI_VERIFY_NODE_ID_GLOBAL:
            printf("MTI_VERIFY_NODE_ID_GLOBAL");
            break;
        case MTI_VERIFIED_NODE_ID:
            printf("MTI_VERIFIED_NODE_ID");
            break;
        case MTI_VERIFIED_NODE_ID_SIMPLE:
            printf("MTI_VERIFIED_NODE_ID_SIMPLE");
            break;
        case MTI_OPTIONAL_INTERACTION_REJECTED:
            printf("MTI_OPTIONAL_INTERACTION_REJECTED");
            break;
        case MTI_TERMINATE_DO_TO_ERROR:
            printf("MTI_TERMINATE_DO_TO_ERROR");
            break;
        case MTI_PROTOCOL_SUPPORT_INQUIRY:
            printf("MTI_PROTOCOL_SUPPORT_INQUIRY");
            break;
        case MTI_PROTOCOL_SUPPORT_REPLY:
            printf("MTI_PROTOCOL_SUPPORT_REPLY");
            break;
        case MTI_CONSUMER_IDENTIFY:
            printf("MTI_CONSUMER_IDENTIFY");
            break;
        case MTI_CONSUMER_IDENTIFY_RANGE:
            printf("MTI_CONSUMER_IDENTIFY_RANGE");
            break;
        case MTI_CONSUMER_IDENTIFIED_UNKNOWN:
            printf("MTI_CONSUMER_IDENTIFIED_UNKNOWN");
            break;
        case MTI_CONSUMER_IDENTIFIED_SET:
            printf("MTI_CONSUMER_IDENTIFIED_SET");
            break;
        case MTI_CONSUMER_IDENTIFIED_CLEAR:
            printf("MTI_CONSUMER_IDENTIFIED_CLEAR");
            break;
        case MTI_CONSUMER_IDENTIFIED_RESERVED:
            printf("MTI_CONSUMER_IDENTIFIED_RESERVED");
            break;
        case MTI_PRODUCER_IDENTIFY:
            printf("MTI_PRODUCER_IDENTIFY");
            break;
        case MTI_PRODUCER_IDENTIFY_RANGE:
            printf("MTI_PRODUCER_IDENTIFY_RANGE");
            break;
        case MTI_PRODUCER_IDENTIFIED_UNKNOWN:
            printf("MTI_PRODUCER_IDENTIFIED_UNKNOWN");
            break;
        case MTI_PRODUCER_IDENTIFIED_SET:
            printf("MTI_PRODUCER_IDENTIFIED_SET");
            break;
        case MTI_PRODUCER_IDENTIFIED_CLEAR:
            printf("MTI_PRODUCER_IDENTIFIED_CLEAR");
            break;
        case MTI_PRODUCER_IDENTIFIED_RESERVED:
            printf("MTI_PRODUCER_IDENTIFIED_RESERVED");
            break;
        case MTI_EVENTS_IDENTIFY_DEST:
            printf("MTI_EVENTS_IDENTIFY_DEST");
            break;
        case MTI_EVENTS_IDENTIFY:
            printf("MTI_EVENTS_IDENTIFY");
            break;
        case MTI_EVENT_LEARN:
            printf("MTI_EVENT_LEARN");
            break;
        case MTI_PC_EVENT_REPORT:
            printf("MTI_PC_EVENT_REPORT");
            break;
        case MTI_SIMPLE_NODE_INFO_REQUEST:
            printf("MTI_SIMPLE_NODE_INFO_REQUEST");
            break;
        case MTI_SIMPLE_NODE_INFO_REPLY:
            printf("MTI_SIMPLE_NODE_INFO_REPLY");
            break;
        case MTI_SIMPLE_TRAIN_INFO_REQUEST:
            printf("MTI_SIMPLE_TRAIN_INFO_REQUEST");
            break;
        case MTI_SIMPLE_TRAIN_INFO_REPLY:
            printf("MTI_SIMPLE_TRAIN_INFO_REPLY");
            break;
        case MTI_TRACTION_PROTOCOL:
            printf("MTI_TRACTION_PROTOCOL");
            break;
        case MTI_TRACTION_REPLY:
            printf("MTI_TRACTION_REPLY ");
            break;
        case MTI_STREAM_INIT_REQUEST:
            printf("MTI_STREAM_INIT_REQUEST");
            break;
        case MTI_STREAM_INIT_REPLY:
            printf("MTI_STREAM_INIT_REPLY");
            break;
        case MTI_FRAME_TYPE_CAN_STREAM_SEND:
            printf("MTI_FRAME_TYPE_CAN_STREAM_SEND");
            break;
        case MTI_STREAM_PROCEED:
            printf("MTI_STREAM_PROCEED");
            break;
        case MTI_STREAM_COMPLETE:
            printf("MTI_STREAM_COMPLETE");
            break;
        case MTI_DATAGRAM:
            printf("MTI_DATAGRAM");
            break;
        case MTI_DATAGRAM_OK_REPLY:
            printf("MTI_DATAGRAM_OK_REPLY");
            break;
        case MTI_DATAGRAM_REJECTED_REPLY:
            printf("MTI_DATAGRAM_REJECTED_REPLY");
            break;
        default:
            printf("[UNKNOWN MTI]");
            break;
    };

    printf("\n");
};

void PrintOpenLcbMsg(openlcb_msg_t* openlcb_msg) {

    if (openlcb_msg) {

        printf("Source : ");
        PrintAliasAndNodeID(openlcb_msg->source_alias, openlcb_msg->source_id);
        printf("Dest : ");
        PrintAliasAndNodeID(openlcb_msg->dest_alias, openlcb_msg->dest_id);
        printf("mti : %04X\n", openlcb_msg->mti);
        PrintMtiName(openlcb_msg->mti);
        printf("Payload Count: %d = ", openlcb_msg->payload_count);
        printf("0x");
        for (int i = 0; i < openlcb_msg->payload_count; i++)
            printf("%02X", ((uint8_olcb_t*) openlcb_msg->payload)[i]);
        printf("\n");
        if (openlcb_msg->state.allocated)
            printf("Allocated: True\n");
        else
            printf("Allocated: False\n");

    }

}

void PrintCanMsg(can_msg_t* can_msg) {

    printf("Identifier: ");
    PrintCanIdentifier(can_msg->identifier);
    printf("[ ");
    for (int i = 0; i < can_msg->payload_count; i++) {

        if (i < can_msg->payload_count - 1)
            printf("%02X.", can_msg->payload[i]);
        else
            printf("%02X", can_msg->payload[i]);

    }
    printf(" ]");

}

void PrintNode(openlcb_node_t* node) {
    
    printf("State Info\n");
    printf("  allocated = 0x%02X\n", node->state.allocated);
    printf("  permitted = 0x%02X\n", node->state.permitted);
    printf("  initalized = 0x%02X\n", node->state.initalized);
    printf("  duplicate_id_detected = 0x%02X\n", node->state.duplicate_id_detected);
    printf("  can_msg_handled = 0x%02X\n", node->state.can_msg_handled);
    printf("  openlcb_datagram_ack_sent = 0x%02X\n", node->state.openlcb_datagram_ack_sent);
    printf("  resend_datagram = 0x%02X\n", node->state.resend_datagram);
    printf("  resend_optional_message = 0x%02X\n", node->state.resend_optional_message);
    printf("  State = %d\n", node->state.run_state);
    
    printf("ID: ");
    PrintInt64(node->id);
    printf("Alias: %04X\n", node->alias);
    printf("Parameters: 0x%p\n", node->parameters);
    printf("Sent Datagrams: 0x%p\n", &node->last_received_datagram);
    if (node->last_received_datagram)
      PrintOpenLcbMsg(node->last_received_datagram);
    else
        printf("  null\n");
    printf("Sent Optional Messages: 0x%p\n", &node->last_received_optional_interaction);
    if (node->last_received_optional_interaction)
      PrintOpenLcbMsg(node->last_received_optional_interaction);
    else
        printf("  null\n");
    printf("NodeLock ID: ");
    PrintInt64(node->lock_node);
    printf("Timer Ticks: %d", node->timerticks);
     
}

void PrintEventID(event_id_t event_id) {

    printf("EventID: 0x%04X", (uint16_olcb_t) (event_id >> 48));
    printf("%04X", (uint16_olcb_t) (event_id >> 32));
    printf("%04X", (uint16_olcb_t) (event_id >> 16));
    printf("%04X\n", (uint16_olcb_t) (event_id >> 0));

}

void PrintCanFrameIdentifierName(uint32_olcb_t identifier) {


    if (identifier & 0xFF000000 & ~RESERVED_TOP_BIT) {

        switch (identifier & 0xFF000000 & ~RESERVED_TOP_BIT) {

            case CAN_CONTROL_FRAME_CID1:
                printf("CAN_CONTROL_FRAME_CID1\n");
                return;
            case CAN_CONTROL_FRAME_CID2:
                printf("CAN_CONTROL_FRAME_CID2\n");
                return;
            case CAN_CONTROL_FRAME_CID3:
                printf("CAN_CONTROL_FRAME_CID3\n");
                return;
            case CAN_CONTROL_FRAME_CID4:
                printf("CAN_CONTROL_FRAME_CID4\n");
                return;
            case CAN_CONTROL_FRAME_CID5:
                printf("CAN_CONTROL_FRAME_CID5\n");
                return;
            case CAN_CONTROL_FRAME_CID6:
                printf("CAN_CONTROL_FRAME_CID6\n");
                return;
            case CAN_CONTROL_FRAME_CID7:
                printf("CAN_CONTROL_FRAME_CID7\n");
                return;
            default:
                printf("[UNKNOWN]\n");
                return;

        }

    }


    switch (identifier & 0xFFFFF000 & ~RESERVED_TOP_BIT) {

        case CAN_CONTROL_FRAME_AMD:
            printf("CAN_CONTROL_FRAME_AMD\n");
            return;
        case CAN_CONTROL_FRAME_AME:
            printf("CAN_CONTROL_FRAME_AME\n");
            return;
        case CAN_CONTROL_FRAME_AMR:
            printf("CAN_CONTROL_FRAME_AMR\n");
            return;
        case CAN_CONTROL_FRAME_RID:
            printf("CAN_CONTROL_FRAME_RID\n");
            return;
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0:
            printf("CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0\n");
            return;
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1:
            printf("CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1\n");
            return;
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2:
            printf("CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2\n");
            return;
        case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3:
            printf("CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3\n");
            return;
        default:
            printf("[UNKNOWN]\n");
            return;

    }

}

void PrintCanIdentifier(uint32_olcb_t identifier) {

    printf("0x%04X", (uint16_olcb_t) (identifier >> 16));
    printf("%04X\n", (uint16_olcb_t) (identifier));

}

void PrintDWord(uint32_olcb_t dword) {

    PrintCanIdentifier(dword);

}


void PrintCAN1Registers(void) {

    printf("C1CTRL1: 0x%x\n", C1CTRL1);
    printf("C1CTRL1: 0x%x\n", C1CTRL2);
    printf("C1VEC: 0x%x\n", C1VEC);
    printf("C1FCTRL: 0x%x\n", C1FCTRL);
    printf("C1FIFO: 0x%x\n", C1FIFO);
    printf("C1INTF: 0x%x\n", C1INTF);
    printf("C1INTE: 0x%x\n", C1INTE);
    printf("C1EC: 0x%x\n", C1EC);
    printf("C1CFG1: 0x%x\n", C1CFG1);
    printf("C1CFG2: 0x%x\n", C1CFG2);
    printf("C1FEN1: 0x%x\n", C1FEN1);
    printf("C1FMSKSEL1: 0x%x\n", C1FMSKSEL1);
    printf("C1FMSKSEL2: 0x%x\n", C1FMSKSEL2);

    C1CTRL1bits.WIN = 0;

    printf("C1RXFUL1: 0x%x\n", C1RXFUL1);
    printf("C1RXFUL2: 0x%x\n", C1RXFUL2);
    printf("C1RXOVF1: 0x%x\n", C1RXOVF1);
    printf("C1RXOVF2: 0x%x\n", C1RXOVF2);
    printf("C1TR01CON: 0x%x\n", C1TR01CON);
    printf("C11TR23CON: 0x%x\n", C1TR23CON);
    printf("C1TR45CON: 0x%x\n", C1TR45CON);
    printf("C1TR67CON: 0x%x\n", C1TR67CON);
    printf("C1RXD: 0x%x\n", C1RXD);
    printf("C1TXD: 0x%x\n", C1TXD);

    C1CTRL1bits.WIN = 1;
    printf("C1BUFPNT1: 0x%x\n", C1BUFPNT1);
    printf("C1BUFPNT2: 0x%x\n", C1BUFPNT2);
    printf("C1BUFPNT3: 0x%x\n", C1BUFPNT3);
    printf("C1BUFPNT4: 0x%x\n", C1BUFPNT4);
    printf("\n");

    printf("MASKS\n");
    printf("C1RXM0SID: 0x%x\n", C1RXM0SID);
    printf("C1RXM0EID: 0x%x\n", C1RXM0EID);
    printf("\n");
    printf("C1RXM1SID: 0x%x\n", C1RXM1SID);
    printf("C1RXM1EID: 0x%x\n", C1RXM1EID);
    printf("\n");
    printf("C1RXM2SID: 0x%x\n", C1RXM2SID);
    printf("C1RXM2EID: 0x%x\n", C1RXM2EID);
    printf("\n");
    printf("\n");
    printf("FILTERS\n");
    printf("C1RXF0SID: 0x%x\n", C1RXF0SID);
    printf("C1RXF0EID: 0x%x\n", C1RXF0EID);
    printf("\n");
    printf("C1RXF1SID: 0x%x\n", C1RXF1SID);
    printf("C1RXF1EID: 0x%x\n", C1RXF1EID);
    printf("\n");
    printf("C1RXF2SID: 0x%x\n", C1RXF2SID);
    printf("C1RXF2EID: 0x%x\n", C1RXF2EID);
    printf(".....\n");
    printf("\n");
    C1CTRL1bits.WIN = 0;
}

void PrintDMA0Registers(void) {
    printf("DMA 0\n");
    printf("DMA0CON: 0x%x\n", DMA0CON);
    printf("DMA0REQ: 0x%x\n", DMA0REQ);
    printf("DMA0STAH: 0x%x\n", DMA0STAH);
    printf("DMA0STAL: 0x%x\n", DMA0STAL);
    printf("DMA0STBH: 0x%x\n", DMA0STBH);
    printf("DMA0STBL: 0x%x\n", DMA0STBL);
    printf("DMA0PAD: 0x%x\n", DMA0PAD);
    printf("DMA0CNT: 0x%x\n", DMA0CNT);
    printf("\n");
}

void PrintDMA1Registers(void) {
    printf("DMA 1\n");
    printf("DMA1CON: 0x%x\n", DMA1CON);
    printf("DMA2REQ: 0x%x\n", DMA1REQ);
    printf("DMA1STAH: 0x%x\n", DMA1STAH);
    printf("DMA1STAL: 0x%x\n", DMA1STAL);
    printf("DMA1STBH: 0x%x\n", DMA1STBH);
    printf("DMA1STBL: 0x%x\n", DMA1STBL);
    printf("DMA1PAD: 0x%x\n", DMA1PAD);
    printf("DMA1CNT: 0x%x\n", DMA1CNT);
    printf("\n");
}

void PrintDMA2Registers(void) {
    printf("DMA 2\n");
    printf("DMA2CON: 0x%x\n", DMA2CON);
    printf("DMA2REQ: 0x%x\n", DMA2REQ);
    printf("DMA2STAH: 0x%x\n", DMA2STAH);
    printf("DMA2STAL: 0x%x\n", DMA2STAL);
    printf("DMA2STBH: 0x%x\n", DMA2STBH);
    printf("DMA2STBL: 0x%x\n", DMA2STBL);
    printf("DMA2PAD: 0x%x\n", DMA2PAD);
    printf("DMA2CNT: 0x%x\n", DMA2CNT);
    printf("\n");
}

void PrintDMA3Registers(void) {
    printf("DMA 3\n");
    printf("DMA3CON: 0x%x\n", DMA3CON);
    printf("DMA3REQ: 0x%x\n", DMA3REQ);
    printf("DMA3STAH: 0x%x\n", DMA3STAH);
    printf("DMA3STAL: 0x%x\n", DMA3STAL);
    printf("DMA3STBH: 0x%x\n", DMA3STBH);
    printf("DMA3STBL: 0x%x\n", DMA3STBL);
    printf("DMA3PAD: 0x%x\n", DMA3PAD);
    printf("DMA3CNT: 0x%x\n", DMA3CNT);
    printf("\n");
}

void PrintDMACommonRegisters(void) {
    printf("DMA Common\n");
    printf("DSADRL: 0x%x\n", DSADRL);
    printf("DSADRH: 0x%x\n", DSADRH);
    printf("DMAPWC: 0x%x\n", DMAPWC);
    printf("DMARQC: 0x%x\n", DMARQC);
    printf("DMALCA: 0x%x\n", DMALCA);
    printf("DMAPPS: 0x%x\n", DMAPPS);
    printf("\n");
}
