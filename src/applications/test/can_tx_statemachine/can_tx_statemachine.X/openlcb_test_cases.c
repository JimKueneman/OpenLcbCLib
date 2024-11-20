/*
 * File:   openlcb_test_cases.c
 * Author: jimkueneman
 *
 * Created on November 11, 2024, 6:34 AM
 */


#include "xc.h"



//static const char* MULTI_FRAME_DATAGRAM_PAYLOAD = "20.53.0.0.0.0.3C.3F.78.6D.6C.20.76.65.72.73.69.6F.6E.3D.27.31.2E.30.27.20.65.6E.63.6F.64.69.6E.67.3D.27.55.54.46.2D.38.27.3F.3E.3C.3F";
//static const char* SINGLE_FRAME_DATAGRAM_PAYLOAD = "20.43.0.0.0.0.28";

void TestCase_Datagram_Single_Frame_Inquiry_Middle(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint16_t dest_alias) {

    openlcb_msg->mti = MTI_DATAGRAM;
    openlcb_msg->dest_alias = dest_alias;
    openlcb_msg->source_alias = source_alias;

    *openlcb_msg->payload[0] = 0x20;
    *openlcb_msg->payload[1] = 0x43;
    *openlcb_msg->payload[2] = 0x00;
    *openlcb_msg->payload[3] = 0x00;
    *openlcb_msg->payload[4] = 0x00;
    *openlcb_msg->payload[5] = 0x00;
    *openlcb_msg->payload[6] = 0x28;
    *openlcb_msg->payload[7] = 0xFF;

    openlcb_msg->payload_count = 7;

}

void TestCase_Datagram_Single_Frame_Inquiry_End(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint16_t dest_alias) {

    TestCase_Datagram_Single_Frame_Inquiry_Middle(openlcb_msg, source_alias, dest_alias);

    openlcb_msg->payload_count = 8;

}

void TestCase_Datagram_Single_Frame_Inquiry_Start(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint16_t dest_alias) {

    TestCase_Datagram_Single_Frame_Inquiry_Middle(openlcb_msg, source_alias, dest_alias);

    openlcb_msg->payload_count = 1;

}

void TestCase_Datagram_MultiFrame_Reply_Middle(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint16_t dest_alias) {

    openlcb_msg->mti = MTI_DATAGRAM;
    openlcb_msg->dest_alias = dest_alias;
    openlcb_msg->source_alias = source_alias;

    *openlcb_msg->payload[0] = 0x20;
    *openlcb_msg->payload[1] = 0x53;
    *openlcb_msg->payload[2] = 0x00;
    *openlcb_msg->payload[3] = 0x00;
    *openlcb_msg->payload[4] = 0x00;
    *openlcb_msg->payload[5] = 0x00;
    *openlcb_msg->payload[6] = 0x3C;
    *openlcb_msg->payload[7] = 0x3F;
    *openlcb_msg->payload[8] = 0x78;
    *openlcb_msg->payload[9] = 0x6D;
    *openlcb_msg->payload[10] = 0x6C;
    *openlcb_msg->payload[11] = 0x20;
    *openlcb_msg->payload[12] = 0x76;
    *openlcb_msg->payload[13] = 0x65;
    *openlcb_msg->payload[14] = 0x72;
    *openlcb_msg->payload[15] = 0x73;
    *openlcb_msg->payload[16] = 0x69;
    *openlcb_msg->payload[17] = 0x6F;
    *openlcb_msg->payload[18] = 0x6E;
    *openlcb_msg->payload[19] = 0x3D;
    *openlcb_msg->payload[20] = 0x27;
    *openlcb_msg->payload[21] = 0x31;
    *openlcb_msg->payload[22] = 0x2E;
    *openlcb_msg->payload[23] = 0x30;
    *openlcb_msg->payload[24] = 0x27;
    *openlcb_msg->payload[25] = 0x20;
    *openlcb_msg->payload[26] = 0x65;
    *openlcb_msg->payload[27] = 0x6E;
    *openlcb_msg->payload[28] = 0x63;
    *openlcb_msg->payload[29] = 0x6F;
    *openlcb_msg->payload[30] = 0x6E;
    *openlcb_msg->payload[31] = 0x67;
    *openlcb_msg->payload[32] = 0x3D;
    *openlcb_msg->payload[33] = 0x27;
    *openlcb_msg->payload[34] = 0x55;
    *openlcb_msg->payload[35] = 0x54;
    *openlcb_msg->payload[36] = 0x46;
    *openlcb_msg->payload[37] = 0x2D;
    *openlcb_msg->payload[30] = 0x38;
    *openlcb_msg->payload[39] = 0x27;
    *openlcb_msg->payload[40] = 0x37;
    *openlcb_msg->payload[41] = 0x3E;
    *openlcb_msg->payload[40] = 0x3C;
    *openlcb_msg->payload[41] = 0x3F;

    openlcb_msg->payload_count = 42;

}

void TestCase_Datagram_MultiFrame_Reply_End(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint16_t dest_alias) {

     TestCase_Datagram_MultiFrame_Reply_Middle(openlcb_msg, source_alias, dest_alias);
     
     openlcb_msg->payload_count = 32;
     
}

void TestCase_Datagram_MultiFrame_Reply_Start(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint16_t dest_alias) {

     TestCase_Datagram_MultiFrame_Reply_Middle(openlcb_msg, source_alias, dest_alias);
     
     openlcb_msg->payload_count = 33;
     
}


void TestCase_Snip_Reply(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint16_t dest_alias) {

    openlcb_msg->mti = MTI_SIMPLE_NODE_INFO_REPLY;
    openlcb_msg->dest_alias = dest_alias;
    openlcb_msg->source_alias = source_alias;

    *openlcb_msg->payload[0] = 0x01;
    *openlcb_msg->payload[1] = 'J';
    *openlcb_msg->payload[2] = 'i';
    *openlcb_msg->payload[3] = 'm';
    *openlcb_msg->payload[4] = 's';
    *openlcb_msg->payload[5] = ' ';
    *openlcb_msg->payload[6] = 'N';
    *openlcb_msg->payload[7] = 'o';
    *openlcb_msg->payload[8] = 'd';
    *openlcb_msg->payload[9] = 'e';
    *openlcb_msg->payload[10] = 0x00;
    *openlcb_msg->payload[11] = 'M';
    *openlcb_msg->payload[12] = 'o';
    *openlcb_msg->payload[13] = 'd';
    *openlcb_msg->payload[14] = 'e';
    *openlcb_msg->payload[15] = 'l';
    *openlcb_msg->payload[16] = 0x00;
    *openlcb_msg->payload[17] = 'v';
    *openlcb_msg->payload[18] = '1';
    *openlcb_msg->payload[19] = 0x00;
    *openlcb_msg->payload[20] = 'v';
    *openlcb_msg->payload[21] = '2';
    *openlcb_msg->payload[22] = 0x00;
    *openlcb_msg->payload[23] = 0x01;
    *openlcb_msg->payload[24] = 'U';
    *openlcb_msg->payload[25] = 's';
    *openlcb_msg->payload[26] = 'e';
    *openlcb_msg->payload[27] = 'r';
    *openlcb_msg->payload[28] = 0x00;
    *openlcb_msg->payload[29] = 'H';
    *openlcb_msg->payload[30] = 'i';
    *openlcb_msg->payload[31] = 0x00;

    openlcb_msg->payload_count = 32;

}

void TestCase_VerifyNodeID(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint16_t dest_alias, uint64_t node_id) {

    openlcb_msg->dest_alias = dest_alias;
    openlcb_msg->source_alias = source_alias;


    if (dest_alias == 0) {

        openlcb_msg->mti = MTI_VERIFY_NODE_ID_GLOBAL;

        if (dest_alias != 0)

            CopyNodeIdToOpenLcb_Ms(openlcb_msg, node_id);
        
        else

            openlcb_msg->payload_count = 0;

    } else {

        openlcb_msg->mti = MTI_VERIFY_NODE_ID_ADDRESSED;

        if (dest_alias != 0) {

            CopyNodeIdToOpenLcb_Ms(openlcb_msg, node_id);

        } else

            openlcb_msg->payload_count = 0;

    }

}

void TestCase_VerifiedNodeID(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t node_id) {

    openlcb_msg->mti = MTI_VERIFIED_NODE_ID;
    openlcb_msg->source_alias = source_alias;

    CopyNodeIdToOpenLcb_Ms(openlcb_msg, node_id);

}

void TestCase_VerifiedNodeID_Simple(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t node_id) {

    openlcb_msg->mti = MTI_VERIFIED_NODE_ID;
    openlcb_msg->source_alias = source_alias;
    openlcb_msg->source_id = node_id;

    CopyNodeIdToOpenLcb_Ms(openlcb_msg, node_id);

}

void TestCase_Protocol_Support_Inquiry(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t node_id) {

    openlcb_msg->mti = MTI_PROTOCOL_SUPPORT_INQUIRY;
    openlcb_msg->source_alias = source_alias;
    openlcb_msg->payload_count = 0;


}

void TestCase_Protocol_Support_Reply(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t code) {

    openlcb_msg->mti = MTI_PROTOCOL_SUPPORT_REPLY;
    openlcb_msg->source_alias = source_alias;

    Copy64BitToOpenLcbMsg(openlcb_msg, code);

}

void TestCase_CID7(can_msg_t* can_msg, uint16_t source_alias, uint64_t node_id) {

    can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | ((node_id >> 24) & 0xFFF000) | (source_alias & 0xFFF);
    can_msg->payload_count = 0;

}

void TestCase_CID6(can_msg_t* can_msg, uint16_t source_alias, uint64_t node_id) {

    can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | ((node_id >> 12) & 0xFFF000) | (source_alias & 0xFFF);
    can_msg->payload_count = 0;

}

void TestCase_CID5(can_msg_t* can_msg, uint16_t source_alias, uint64_t node_id) {

    can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((node_id >> 24) & 0xFFF000) | (source_alias & 0xFFF);
    can_msg->payload_count = 0;

}

void TestCase_CID4(can_msg_t* can_msg, uint16_t source_alias, uint64_t node_id) {

    can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | ((node_id << 12) & 0xFFF000) | (source_alias & 0xFFF);
    can_msg->payload_count = 0;

}

void TestCase_RID(can_msg_t* can_msg, uint16_t source_alias) {

    can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | (source_alias & 0xFFF);
    can_msg->payload_count = 0;

}

void TestCase_AMD(can_msg_t* can_msg, uint16_t source_alias, uint64_t node_id) {

    can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (source_alias & 0xFFF);
    CopyNodeIdToCanMsg(can_msg, node_id, 0);

}

void TestCase_Receive_Legacy_Snip(can_msg_t* can_msg) {

    ////  LEGACY SNIP
    //[[19de8479] 04 90                  ]  S: 02.01.12.FE.27.F3 - AA.02.03.04.05.07 SimpleNodeIdentInfoRequest with no payload
    LoadCanMsg(can_msg, 0x19de8479, 2, 0x04, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 01 52 61 69 6C 73]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '1,Rails'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x01, 0x52, 0x61, 0x69, 0x6C, 0x73);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 74 61 72 73 20 4C]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'tars L'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x74, 0x61, 0x72, 0x73, 0x20, 0x4C);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 69 6D 69 74 65 64]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'imited'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x69, 0x6D, 0x69, 0x74, 0x65, 0x64);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 00 49 6F 20 44 65]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content ',Io De'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x00, 0x49, 0x6F, 0x20, 0x44, 0x65);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 76 65 6C 6F 70 65]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'velope'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x76, 0x65, 0x6C, 0x6F, 0x70, 0x65);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 72 27 73 20 42 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'r's Bo'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x72, 0x27, 0x73, 0x20, 0x42, 0x6F);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 61 72 64 00 31 2E]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'ard,1.'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x61, 0x72, 0x64, 0x00, 0x31, 0x2E);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 30 00 31 2E 34 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '0,1.4,'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x30, 0x00, 0x31, 0x2E, 0x34, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 01 4A 69 6D 27 73]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '1,Jim's'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x01, 0x4A, 0x69, 0x6D, 0x27, 0x73);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 20 49 4F 20 4E 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content ' IO No'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x20, 0x49, 0x4F, 0x20, 0x4E, 0x6F);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 64 65 20 23 31 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'de #1,'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x64, 0x65, 0x20, 0x23, 0x31, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 4D 79 20 66 69 72]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'My fir'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x4D, 0x79, 0x20, 0x66, 0x69, 0x72);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 73 74 20 62 61 74]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'st bat'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x73, 0x74, 0x20, 0x63, 0x61, 0x74);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 63 68 20 6F 66 20]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'ch of '
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x63, 0x68, 0x20, 0x6F, 0x66, 0x20);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 49 4F 20 4E 6F 64]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'IO Nod'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x49, 0x4F, 0x20, 0x4E, 0x6F, 0x64);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 65 73 20 66 72 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'es fro'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x65, 0x73, 0x20, 0x66, 0x72, 0x6F);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 6D 20 44 6F 6E 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'm Don,'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x6D, 0x20, 0x44, 0x6F, 0x6E, 0x00);
    StatemachineIncomingCan(can_msg);



}

void TestCase_Receive_MultiFrame_Snip(can_msg_t* can_msg) {

    ////  MULTI-FRAME SNIP

    //[[19de8479] 04 90                  ]  S: 02.01.12.FE.27.F3 - AA.02.03.04.05.07 SimpleNodeIdentInfoRequest with no payload
    LoadCanMsg(can_msg, 0x19de8479, 2, 0x04, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 01 52 61 69 6C 73]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '1,Rails'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x44, 0x79, 0x01, 0x52, 0x61, 0x69, 0x6C, 0x73);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 74 61 72 73 20 4C]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'tars L'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x74, 0x61, 0x72, 0x73, 0x20, 0x4C);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 69 6D 69 74 65 64]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'imited'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x69, 0x6D, 0x69, 0x74, 0x65, 0x64);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 00 49 6F 20 44 65]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content ',Io De'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x00, 0x49, 0x6F, 0x20, 0x44, 0x65);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 76 65 6C 6F 70 65]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'velope'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x76, 0x65, 0x6C, 0x6F, 0x70, 0x65);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 72 27 73 20 42 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'r's Bo'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x72, 0x27, 0x73, 0x20, 0x42, 0x6F);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 61 72 64 00 31 2E]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'ard,1.'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x61, 0x72, 0x64, 0x00, 0x31, 0x2E);
    StatemachineIncomingCan(can_msg);


    //[[19a0803a] 04 79 30 00 31 2E 34 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '0,1.4,'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x30, 0x00, 0x31, 0x2E, 0x34, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 01 4A 69 6D 27 73]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '1,Jim's'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x01, 0x4A, 0x69, 0x6D, 0x27, 0x73);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 20 49 4F 20 4E 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content ' IO No'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x20, 0x49, 0x4F, 0x20, 0x4E, 0x6F);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 64 65 20 23 31 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'de #1,'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x64, 0x65, 0x20, 0x23, 0x31, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 4D 79 20 66 69 72]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'My fir'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x4D, 0x79, 0x20, 0x66, 0x69, 0x72);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 73 74 20 62 61 74]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'st bat'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x73, 0x74, 0x20, 0x63, 0x61, 0x74);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 63 68 20 6F 66 20]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'ch of '
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x63, 0x68, 0x20, 0x6F, 0x66, 0x20);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 49 4F 20 4E 6F 64]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'IO Nod'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x49, 0x4F, 0x20, 0x4E, 0x6F, 0x64);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 65 73 20 66 72 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'es fro'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x65, 0x73, 0x20, 0x66, 0x72, 0x6F);
    StatemachineIncomingCan(can_msg);

    //[[19a0803a] 04 79 6D 20 44 6F 6E 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'm Don,'
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x84, 0x79, 0x6D, 0x20, 0x44, 0x6F, 0x6E, 0x00);
    StatemachineIncomingCan(can_msg);

}

void TestCase_Simple_Legacy_Snip(can_msg_t* can_msg) {

    // SIMPLE LEGACY SNIP
    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x01, 0xAA, 0x00, 0xBB, 0x00, 0xCC);
    StatemachineIncomingCan(can_msg);

    LoadCanMsg(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x00, 0xDD, 0x00, 0x01, 0xFF, 0x00);
    StatemachineIncomingCan(can_msg);

    LoadCanMsg(can_msg, 0x19a0803a, 4, 0x04, 0x79, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00);
    StatemachineIncomingCan(can_msg);


}

void TestCase_Receive_Protocol_Support(can_msg_t* can_msg) {

    //     PROTOCOL SUPPORT 
    //[[19828479] 00 3A                  ]  S: 02.01.12.FE.27.F3 - 05.02.01.02.02.4B ProtocolSupportInquiry with no payload
    LoadCanMsg(can_msg, 0x19828479, 2, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[1966803a] 04 79 D5 18 00 00 00 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 ProtocolSupportReply with payload D5 18 00 00 00 00
    LoadCanMsg(can_msg, 0x1966803a, 8, 0x04, 0x79, 0xD5, 0x18, 0x00, 0x00, 0x00, 0x00);
    StatemachineIncomingCan(can_msg);

}

void TestCase_Receive_Datagram(can_msg_t* can_msg) {

    //  DATAGRAM
    //[[1a03a479] 20 43 00 00 00 00 28   ]  S: 02.01.12.FE.27.F3 - 05.02.01.02.02.4B Datagram: (7) 20.43.0.0.0.0.28 
    LoadCanMsg(can_msg, 0x1a03a479, 7, 0x20, 0x43, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[19a2803a] 04 79                  ]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 DatagramReceivedOK with no payload
    LoadCanMsg(can_msg, 0x19a2803a, 2, 0x04, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[1b47903a] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    LoadCanMsg(can_msg, 0x1b47903a, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    StatemachineIncomingCan(can_msg);

    //[[1c47903a] 78 6D 6C 20 76 65 72 73]  R: (Middle of Datagram)
    LoadCanMsg(can_msg, 0x1c47903a, 8, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73);
    StatemachineIncomingCan(can_msg);

    //[[1c47903a] 69 6F 6E 3D 27 31 2E 30]  R: (Middle of Datagram)
    LoadCanMsg(can_msg, 0x1c47903a, 8, 0x69, 0x6F, 0x6E, 0x3D, 0x27, 0x31, 0x2E, 0x30);
    StatemachineIncomingCan(can_msg);

    //[[1c47903a] 27 20 65 6E 63 6F 64 69]  R: (Middle of Datagram)
    LoadCanMsg(can_msg, 0x1c47903a, 8, 0x27, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69);
    StatemachineIncomingCan(can_msg);

    //[[1c47903a] 6E 67 3D 27 55 54 46 2D]  R: (Middle of Datagram)
    LoadCanMsg(can_msg, 0x1c47903a, 8, 0x6E, 0x67, 0x3D, 0x27, 0x55, 0x54, 0x46, 0x2D);
    StatemachineIncomingCan(can_msg);

    //[[1d47903a] 38 27 3F 3E 3C 3F      ]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Datagram: (46) 20.53.0.0.0.0.3C.3F.78.6D.6C.20.76.65.72.73.69.6F.6E.3D.27.31.2E.30.27.20.65.6E.63.6F.64.69.6E.67.3D.27.55.54.46.2D.38.27.3F.3E.3C.3F
    LoadCanMsg(can_msg, 0x1d47903a, 6, 0x38, 0x27, 0x3F, 0x3E, 0x3C, 0x3F, 0x00, 0x00);
    StatemachineIncomingCan(can_msg);

    //[[19a28479] 00 3A                  ]  S: 02.01.12.FE.27.F3 - 05.02.01.02.02.4B DatagramReceivedOK with no payload
    LoadCanMsg(can_msg, 0x19a28479, 2, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00);
    StatemachineIncomingCan(can_msg);

}