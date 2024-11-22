
/*
 * File:   openlcb_utilities.c
 * Author: jimkueneman
 *
 * Created on November 6, 2024, 5:44 AM
 */


#include "xc.h"
#include "stdio.h" // printf
#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"


void Utilities_load_openlcb_message(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t source_id, uint16_t dest_alias, uint64_t dest_id, uint16_t mti, uint16_t payload_count) {

    if (openlcb_msg) {

        openlcb_msg->dest_alias = dest_alias;
        openlcb_msg->dest_id = dest_id;
        openlcb_msg->source_alias = source_alias;
        openlcb_msg->source_id = source_id;
        openlcb_msg->mti = mti;
        openlcb_msg->payload_count = payload_count;
        openlcb_msg->retry_count = 0;
        openlcb_msg->timerticks = 0;

    }

}

void Utilities_clear_openlcb_message_payload(openlcb_msg_t* openlcb_msg) {
    
    for (int i = 0; i < openlcb_msg->payload_size; i++) 
        *openlcb_msg->payload[i] = 0;
    
    openlcb_msg->payload_count = 0;
    
}

void Utilities_copy_event_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, event_id_t event_id) {
    
   for (int i = 7; i >= 0; i--) {

        *openlcb_msg->payload[i] = event_id & 0xFF;
        event_id = event_id >> 8;

    }

    openlcb_msg->payload_count = 8;
 
}

void Utilities_copy_openlcb_message(openlcb_msg_t* source, openlcb_msg_t* target) {

    if (source->payload_size == target->payload_size) {

        target->dest_alias = source->dest_alias;
        target->dest_id = source->dest_id;
        target->mti = source->mti;
        target->payload_count = source->payload_count;
        target->source_alias = source->source_alias;
        target->source_id = source->source_id;

        for (int i = 0; i < source->payload_count; i++)
            *target->payload[i] = *source->payload[i];
        
    }

}

void Utilities_copy_word_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t word, uint8_t offset) {
    
    *openlcb_msg->payload[0+offset] = (uint8_t) ((word >> 8) & 0xFF);
    *openlcb_msg->payload[1+offset] = (uint8_t) (word & 0xFF);
    
}

void Utilities_copy_dword_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint32_t doubleword, uint8_t offset) {
    
    *openlcb_msg->payload[0+offset] = (uint8_t) ((doubleword >> 24) & 0xFF);
    *openlcb_msg->payload[1+offset] = (uint8_t) ((doubleword >> 16) & 0xFF);
    *openlcb_msg->payload[2+offset] = (uint8_t) ((doubleword >> 8) & 0xFF);
    *openlcb_msg->payload[3+offset] = (uint8_t) (doubleword & 0xFF);
    
}

void Utilities_copy_node_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, node_id_t node_id) {

    openlcb_msg->payload_count = 6;

    for (int iIndex = 5; iIndex >= 0; iIndex--) {
        *openlcb_msg->payload[iIndex] = node_id & 0xFF;
        node_id = node_id >> 8;

    }

}

void Utilities_copy_64_bit_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint64_t data) {

    for (int i = 7; i >= 0; i--) {

        *openlcb_msg->payload[i] = data & 0xFF;
        data = data >> 8;

    }

    openlcb_msg->payload_count = 8;

}

node_id_t Utilities_extract_node_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    return ( 
            ((uint64_t) *openlcb_msg->payload[0] << 40) |
            ((uint64_t)*openlcb_msg->payload[1] << 32) |
            ((uint64_t)*openlcb_msg->payload[2] << 24) |
            ((uint64_t)*openlcb_msg->payload[3] << 16) |
            ((uint64_t)*openlcb_msg->payload[4] << 8) |
            ((uint64_t)*openlcb_msg->payload[5])
            );

}


event_id_t Utilities_extract_event_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    return (
            ((uint64_t)*openlcb_msg->payload[0] << 56) |
            ((uint64_t)*openlcb_msg->payload[1] << 48) |
            ((uint64_t)*openlcb_msg->payload[2] << 40) |
            ((uint64_t)*openlcb_msg->payload[3] << 32) |
            ((uint64_t)*openlcb_msg->payload[4] << 24) |
            ((uint64_t)*openlcb_msg->payload[5] << 16) |
            ((uint64_t)*openlcb_msg->payload[6] << 8) |
            ((uint64_t)*openlcb_msg->payload[7])
            );
}

uint16_t Utilities_extract_word_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {
    
    return (
            ((uint16_t)*openlcb_msg->payload[0 + offset] << 8) |
            ((uint16_t)*openlcb_msg->payload[1 + offset])
            );
    
}

 uint32_t Utilities_extract_dword_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t offset) {
     
         return (
             ((uint32_t)*openlcb_msg->payload[0 + offset] << 24) |
             ((uint32_t)*openlcb_msg->payload[1 + offset] << 16) |
             ((uint32_t)*openlcb_msg->payload[2 + offset] << 8) |
             ((uint32_t)*openlcb_msg->payload[3 + offset])
            );
         
 }
 

void Utilities_set_multi_frame_flag(uint8_t* target, uint8_t flag) {

    *target = *target | flag;

}

uint8_t Utilities_is_addressed_openlcb_message(openlcb_msg_t* openlcb_msg) {

    return ((openlcb_msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT);

}

uint8_t Utilities_count_nulls_in_openlcb_payload(openlcb_msg_t* openlcb_msg) {

    uint8_t count = 0;

    for (int i = 0; i < openlcb_msg->payload_count; i++) {

        if (*openlcb_msg->payload[i] == 0x00)

            count = count + 1;

    }

    return count;

}



//openlcb_node_t* FindOpenLcbNodeByNodeId(openlcb_nodes_t* openlcb_nodes, uint64_t node_id) {
//    
//    for (int i = 0; i < openlcb_nodes->count; i++)
//
//        if (openlcb_nodes->node[i].id == node_id) {
//            
//            return &openlcb_nodes->node[i];
//            
//            printf("you\n");
//            
//        }
//           
//    return (void*) 0;
//
//}
//
//
////////void CAN_CopyBuffers(uint8_t start, uint8_t count, payload_bytes_can_t* payload_source, payload_bytes_can_t* payload_target) {
////////
////////    for (int iIndex = start; iIndex < count; iIndex++)
////////        *payload_target[iIndex] = *payload_source[iIndex];
////////
////////}
////////
////////uint8_t EqualBuffers(openlcb_msg_t* msg1, openlcb_msg_t* msg2) {
////////
////////    if ((msg1->payload_count == msg2->payload_count) && (msg1->state.data_struct_size == msg2->state.data_struct_size)) {
////////
////////        for (int iIndex = 0; iIndex < msg1->payload_count; iIndex++) {
////////            if ((*(payload_basic_t*) (msg1->payload_ptr))[iIndex] != (*(payload_basic_t*) (msg2->payload_ptr))[iIndex]) {
////////
////////                return FALSE;
////////
////////            }
////////        }
////////
////////        return TRUE;
////////    }
////////
////////    return FALSE;
////////}