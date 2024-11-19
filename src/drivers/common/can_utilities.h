
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef ___CAN_UTILITIES__
#define	___CAN_UTILITIES__

#include <xc.h> // include processor files - each processor file is guarded. 
#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

extern void CanUtilties_load_can_message(can_msg_t* can_msg, uint32_t identifier, uint8_t payload_size, uint8_t byte1, uint8_t byte2 , uint8_t byte3 , uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8);

extern void CanUtilities_clear_can_message(can_msg_t* can_msg);


extern void CanUtilities_copy_node_id_to_payload(can_msg_t* can_msg, uint64_t node_id, uint8_t start_offset);

extern uint8_t CanUtilities_copy_can_payload_to_openlcb_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t can_start_index);

//extern uint8_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t can_start_index);

extern uint8_t CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_t can_start_index);

extern void CanUtilities_copy_64_bit_to_can_message(can_msg_t* can_msg, uint64_t data);


extern uint64_t CanUtilities_extract_can_payload_as_node_id(payload_bytes_can_t* payload);

extern uint16_t CanUtilities_extract_source_alias_from_can_message(can_msg_t* can_msg);

extern uint16_t CanUtilties_extract_dest_alias_from_can_message(can_msg_t* can_msg);

extern uint16_t CanUtilties_extract_can_mti_from_can_identifier(can_msg_t* can_msg);  // caution this return 0 if the can message is a datagram/stream or other message that has a MTI that won't fit in 12 bits



extern uint16_t CanUtilties_convert_can_mti_to_openlcb_mti(can_msg_t* can_msg);

extern uint16_t CanUtilities_is_dest_alias_in_can_payload(can_msg_t* can_msg);

extern uint8_t CanUtilities_count_nulls_in_can_payload(can_msg_t* can_msg);

extern uint8_t CanUtilities_count_nulls_in_payloads(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg);

extern uint8_t CanUtilities_is_openlcb_message(can_msg_t* msg);


//extern uint8_t IsCanMessageAddressedToNode(openlcb_nodes_t* openlcb_nodes, can_msg_t* msg);

//extern openlcb_node_t* TestCanMessageForAliasConflict(openlcb_nodes_t* openlcb_nodes, can_msg_t* msg);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

