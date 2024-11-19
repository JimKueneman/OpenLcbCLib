/*
 * File:   can_utilities.c
 * Author: jimkueneman
 *
 * Created on November 16, 2024, 12:58 PM
 */


#include "xc.h"

#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "can_types.h"
#include "../../openlcb/openlcb_utilities.h"


uint16_t CanUtilities_extract_source_alias_from_can_message(can_msg_t* can_msg) {

    return (can_msg->identifier & 0x000000FFF);
    
}

uint16_t CanUtilties_extract_dest_alias_from_can_message(can_msg_t* can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

        case CAN_FRAME_TYPE_GLOBAL_ADDRESSED:

            if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT)

                return ( ((can_msg->payload[0] & 0x0F) << 8) | (can_msg->payload[1]));

            break;

        case CAN_FRAME_TYPE_DATAGRAM_FIRST:
        case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:
        case CAN_FRAME_TYPE_DATAGRAM_FINAL:

            return (can_msg->identifier >> 12) & 0x000000FFF;

            // TODO: Handle Streams

    }

    return 0;
}

uint16_t CanUtilties_convert_can_mti_to_openlcb_mti(can_msg_t* can_msg) {

    switch (can_msg->identifier & MASK_CAN_FRAME_TYPE) {

        case CAN_FRAME_TYPE_GLOBAL_ADDRESSED:

            return (can_msg->identifier >> 12) & 0x0FFF;

        case CAN_FRAME_TYPE_DATAGRAM_ONLY:
        case CAN_FRAME_TYPE_DATAGRAM_FIRST:
        case CAN_FRAME_TYPE_DATAGRAM_MIDDLE:
        case CAN_FRAME_TYPE_DATAGRAM_FINAL:

            return MTI_DATAGRAM;

        case CAN_FRAME_TYPE_STREAM:

            // TODO: Figure out Streams

            return 0;

    }

    return 0;

}


uint8_t CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t start_index) {


    uint16_t result = 0;

    for (int can_payload_index = start_index; can_payload_index < can_msg->payload_count; can_payload_index++) {

        if (openlcb_msg->payload_count < openlcb_msg->payload_size) {
            
            *openlcb_msg->payload[openlcb_msg->payload_count] = can_msg->payload[can_payload_index];

            result = result + 1;

        }

        openlcb_msg->payload_count = openlcb_msg->payload_count + 1;
    }

    return result;

}


uint8_t CanUtilities_copy_can_payload_to_openlcb_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t start_index) {

    openlcb_msg->payload_count = 0; // replacing the contents so reset to index 0

    return CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg, can_msg, start_index);

}

uint8_t CanUtilities_count_nulls_in_can_payload(can_msg_t* can_msg) {

    uint8_t count = 0;

    for (int i = 0; i < can_msg->payload_count; i++) {

        if (can_msg->payload[i] == 0x00)

            count = count + 1;

    };

    return count;

}

uint8_t CanUtilities_count_nulls_in_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    return CanUtilities_count_nulls_in_can_payload(can_msg) + Utilities_count_nulls_in_openlcb_payload(openlcb_msg);

}

uint8_t CanUtilities_is_openlcb_message(can_msg_t* msg) {
    
    return (msg->identifier & CAN_OPENLCB_MSG) == CAN_OPENLCB_MSG;
    
}




uint16_t Copy_Datagram_CAN_Buffer_To_OpenLcb_Buffer(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t start_index, payload_datagram_t* payload) {

    uint16_t result = 0;


    for (int can_payload_index = start_index; can_payload_index < can_msg->payload_count; can_payload_index++) {

        if (openlcb_msg->payload_count < openlcb_msg->payload_size) {

            *openlcb_msg->payload[openlcb_msg->payload_count] = can_msg->payload[can_payload_index];

            result = result + 1;

        }

        openlcb_msg->payload_count = openlcb_msg->payload_count + 1;

    }

    return result;

}

uint64_t CanUtilities_extract_can_payload_as_node_id(payload_bytes_can_t* payload) {

    return ((uint64_t) (*payload)[0] << 40) |
            ((uint64_t) (*payload)[1] << 32) |
            ((uint64_t) (*payload)[2] << 24) |
            ((uint64_t) (*payload)[3] << 16) |
            ((uint64_t) (*payload)[4] << 8) |
            ((uint64_t) (*payload)[5]);

}

uint16_t CanUtilties_extract_can_mti_from_can_message(can_msg_t* can_msg) {

    if ((can_msg->identifier & MASK_CAN_FRAME_TYPE) == CAN_FRAME_TYPE_GLOBAL_ADDRESSED)

        return (can_msg->identifier >> 12) & 0x0FFF;

    return 0;
}

uint16_t CanUtilities_is_dest_alias_in_can_payload(can_msg_t* can_msg) {

    uint16_t result = FALSE;

    if ((can_msg->identifier & MASK_CAN_FRAME_TYPE) == CAN_FRAME_TYPE_GLOBAL_ADDRESSED) {

        if (can_msg->identifier & MASK_CAN_DEST_ADDRESS_PRESENT)

            return TRUE; // TODO: Handle Streams

    }

    return result;

}

uint8_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t can_bytes_start, uint16_t* openlcb_msg_payload_index) {

    int can_payload_index = can_bytes_start;
    uint8_t result = can_bytes_start;

    // Don't overrun either buffer
    while ((can_payload_index < LEN_CAN_DATA_ARRAY) && (*openlcb_msg_payload_index < openlcb_msg->payload_count)) {

        can_msg->payload[can_payload_index] = (*((payload_bytes_can_t*) (openlcb_msg->payload)))[*openlcb_msg_payload_index];

        can_payload_index = can_payload_index + 1;
        *openlcb_msg_payload_index = *openlcb_msg_payload_index + 1;
        result = result + 1;

    }

    can_msg->payload_count = result;

    return result;

}

void CanUtilties_load_can_message(can_msg_t* can_msg, uint32_t identifier, uint8_t payload_size, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8) {

    can_msg->identifier = identifier;
    can_msg->payload_count = payload_size;
    can_msg->payload[0] = byte1;
    can_msg->payload[1] = byte2;
    can_msg->payload[2] = byte3;
    can_msg->payload[3] = byte4;
    can_msg->payload[4] = byte5;
    can_msg->payload[5] = byte6;
    can_msg->payload[6] = byte7;
    can_msg->payload[7] = byte8;

}


void CanUtilities_copy_node_id_to_payload(can_msg_t* can_msg, uint64_t node_id, uint8_t start_offset) {

    if (start_offset <= 2) {

        can_msg->payload_count = 6 + start_offset;

        for (int iIndex = (start_offset + 5); iIndex >= (0 + start_offset); iIndex--) { // This is a count down...
            can_msg->payload[iIndex] = node_id & 0xFF;
            node_id = node_id >> 8;
        }

    } else

        can_msg->payload_count = 0;

}

void CanUtilities_copy_64_bit_to_can_message(can_msg_t* can_msg, uint64_t data) {

    for (int i = 0; i < 8; i++) {

        can_msg->payload[i] = data & 0xFF;
        data = data >> 8;

    }

    can_msg->payload_count = 8;

}
//
//uint8_t IsCanMessageAddressedToNode(openlcb_nodes_t* openlcb_nodes, can_msg_t* msg) {
//
//    for (int i = 0; i < openlcb_nodes->count; i++) {
//
//        if (openlcb_nodes->node[i].state.allocated) {
//
//            if (openlcb_nodes->node[i].alias == ExtractDestAliasFromCanMessage(msg))
//
//                return TRUE;
//
//        }
//
//    }
//
//    return FALSE;
//
//}
//
//openlcb_node_t * TestCanMessageForAliasConflict(openlcb_nodes_t* openlcb_nodes, can_msg_t * msg) {
//
//    uint16_t test_alias = msg->identifier & 0xFFF;
//
//    for (int iIndex = 0; iIndex < openlcb_nodes->count; iIndex++) {
//
//        if (openlcb_nodes->node[iIndex].state.allocated && openlcb_nodes->node[iIndex].state.permitted) {
//
//            if ((openlcb_nodes->node[iIndex].alias == (test_alias))) { // Check if we own this Alias
//
//                return (&openlcb_nodes->node[iIndex]);
//
//            }
//
//        }
//
//    }
//
//    return (void*) 0;
//
//}

//
//uint16_t Copy_Snip_CAN_Buffer_To_OpenLcb_Buffer(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t start_index, payload_snip_t* payload) {
//
//    uint16_t result = 0;
//
//    for (int can_payload_index = start_index; can_payload_index < can_msg->payload_count; can_payload_index++) {
//
//        if (openlcb_msg->payload_count < openlcb_msg->payload_size) {
//
//            (*payload)[openlcb_msg->payload_count] = can_msg->payload[can_payload_index];
//
//            result = result + 1;
//
//        }
//
//        openlcb_msg->payload_count = openlcb_msg->payload_count + 1;
//
//    }
//
//    return result;
//
//}
//
//uint16_t Copy_Stream_CAN_Buffer_To_OpenLcb_Buffer(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t start_index, payload_stream_t* payload) {
//
//    uint16_t result = 0;
//
//    for (int can_payload_index = start_index; can_payload_index < can_msg->payload_count; can_payload_index++) {
//
//        if (openlcb_msg->payload_count < openlcb_msg->payload_size) {
//
//            (*payload)[openlcb_msg->payload_count] = can_msg->payload[can_payload_index];
//
//            result = result + 1;
//
//        }
//
//        openlcb_msg->payload_count = openlcb_msg->payload_count + 1;
//
//    }
//
//    return result;
//
//}
//
