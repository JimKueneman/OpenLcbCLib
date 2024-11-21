/*
 * File:   protocol_message_network.c
 * Author: jimkueneman
 *
 * Created on November 14, 2024, 5:24 AM
 */


#include "xc.h"
#include "stdio.h" // printf
#include "openlcb_types.h"
#include "openlcb_utilities.h"


void _send_duplicate_node_id(openlcb_node_t* openlcb_node, openlcb_node_t* duplicate_node) {
    
//    if (!openlcb_node->working_msg)
//        return; // Finished with the message
//
//    if (duplicate_node->state.duplicate_id_detected) {
//
//        openlcb_node->working_msg = (void*) 0; // Done with the message
//        return;
//
//    }
//
//    if (GetFreeCountOpenLcbFIFO(&outgoing_fifo) >= (LEN_FIFO_BUFFER / 2))
//        return;
//
//    openlcb_msg_t* new_msg = PushOpenLcbMessage(&outgoing_fifo, ID_DATA_SIZE_BASIC);
//
//    if (new_msg) {
//
//        LoadOpenLcbMessage(
//                new_msg,
//                duplicate_node->alias,
//                duplicate_node->id,
//                0,
//                0,
//                MTI_PC_EVENT_REPORT,
//                8);
//        CopyEventIdToOpenLcbPayload(new_msg, EVENT_ID_DUPLICATE_NODE_DETECTED);
//
//        duplicate_node->state.duplicate_id_detected = TRUE;
//
//        openlcb_node->working_msg = (void*) 0; // Done with the message
//
//        return;
//
//    }
//
//    return;

}

void _send_verified_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

//    if (!openlcb_node->working_msg)
//        return; // Finished with the message
//
//    openlcb_msg_t* new_msg;
//
//    new_msg = PushOpenLcbMessage(&outgoing_fifo, ID_DATA_SIZE_BASIC);
//
//    if (new_msg) {
//
//        LoadOpenLcbMessage(
//                new_msg,
//                openlcb_node->alias,
//                openlcb_node->id,
//                openlcb_node->working_msg->source_alias,
//                openlcb_node->working_msg->source_id,
//                MTI_VERIFIED_NODE_ID,
//                0);
//        CopyNodeIdToOpenLcbPayload(new_msg, openlcb_node->id);
//
//        if (USER_DEFINED_PROTOCOL_SUPPORT & PSI_SIMPLE)
//            new_msg->mti = MTI_VERIFIED_NODE_ID_SIMPLE;
//
//        openlcb_node->working_msg = (void*) 0; // Done with the message
//
//        return;
//
//    }
//
//    return;

    
}

void ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

//    if (!openlcb_node->working_msg)
//        return; // Finished with the message
//
//    openlcb_msg_t* new_msg;
//
//    if (openlcb_node->working_msg->dest_alias == openlcb_node->alias) {
//
//        new_msg = PushOpenLcbMessage(&outgoing_fifo, ID_DATA_SIZE_BASIC);
//
//        if (new_msg) {
//
//            LoadOpenLcbMessage(
//                    new_msg,
//                    openlcb_node->alias,
//                    openlcb_node->id,
//                    openlcb_node->working_msg->source_alias,
//                    openlcb_node->working_msg->source_id,
//                    MTI_PROTOCOL_SUPPORT_REPLY,
//                    3);
//
//            cast_basic_dereference(new_msg->payload)[0] = (uint8_t) ((USER_DEFINED_PROTOCOL_SUPPORT >> 16) & 0xFF);
//            cast_basic_dereference(new_msg->payload)[1] = (uint8_t) ((USER_DEFINED_PROTOCOL_SUPPORT >> 8) & 0xFF);
//            cast_basic_dereference(new_msg->payload)[2] = (uint8_t) ((USER_DEFINED_PROTOCOL_SUPPORT >> 0) & 0xFF);
//
//            openlcb_node->working_msg = (void*) 0; // Done with the message
//
//            return;
//
//        }
//
//    }
//
//    return;
    
    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

//    if (!openlcb_node->working_msg)
//        return; // Finished with the message
//
//    if (openlcb_node->working_msg->payload_count > 0) {
//
//        uint64_t node_id = ExtractNodeIdFromOpenLcbPayload(openlcb_node->working_msg);
//
//        if (node_id == openlcb_node->id)
//
//            SendVerifiedNodeID(openlcb_node);
//
//        else
//
//            openlcb_node->working_msg = (void*) 0; // Done with the message  
//
//    } else
//
//        SendVerifiedNodeID(openlcb_node);
//
//    return;
    
    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

//    if (!openlcb_node->working_msg)
//        return; // Finished with the message
//
//    // Addressed always replies regardless if the payload NodeID matches or not but the alias much match
//
//    if (openlcb_node->working_msg->dest_alias == openlcb_node->alias)
//
//        SendVerifiedNodeID(openlcb_node);
//
//    else
//
//        openlcb_node->working_msg = (void*) 0; // Done with the message 
//
//    return;
    
    openlcb_node->state.openlcb_msg_handled = TRUE;
    
}



void ProtocolMessageNetwork_handle_verified_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {
//
//    uint64_t node_id = ExtractNodeIdFromOpenLcbPayload(openlcb_node->working_msg);
//
//    openlcb_node_t* duplicate_node = FindOpenLcbNodeByNodeId(&openlcb_nodes, node_id);
//
//
//    if (duplicate_node)
//
//        SendDuplicateNodeId(openlcb_node, duplicate_node);
//
//    else
//
//        openlcb_node->working_msg = (void*) 0; // Done with the message
    
    openlcb_node->state.openlcb_msg_handled = TRUE;

}