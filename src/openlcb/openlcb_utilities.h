
/* 
 * File: openlcb_utilities.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_UTILITIES__
#define	__OPENLCB_UTILITIES__

#include <xc.h> // include processor files - each processor file is guarded. 
#include "openlcb_defines.h"
#include "openlcb_types.h"


////extern uint64_t MessageDataToNodeID(openlcb_msg_t* msg);
////extern uint8_t EqualBuffers(openlcb_msg_t* msg1, openlcb_msg_t* msg2);

extern void Utilities_load_openlcb_message(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t source_id, uint16_t dest_alias, uint64_t dest_id, uint16_t mti, uint16_t payload_count);

extern void Utilities_copy_event_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, event_id_t event_id);

extern void Utilities_copy_openlcb_message(openlcb_msg_t* source, openlcb_msg_t* target);

extern void Utilities_copy_64_bit_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint64_t data);

extern void Utilities_copy_node_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, node_id_t node_id, uint8_t index);

extern void Utilities_copy_word_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t word, uint8_t index);

extern void Utilities_copy_dword_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint32_t doubleword, uint8_t index);

extern uint16_t Utilities_copy_string_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const char string[], uint8_t index);

extern uint16_t Utilities_copy_byte_array_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const uint8_t byte_array[], uint8_t payload_index, uint16_t data_count);

extern void Utilities_clear_openlcb_message_payload(openlcb_msg_t* openlcb_msg); 

extern node_id_t Utilities_extract_node_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint8_t index);

extern event_id_t Utilities_extract_event_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg);

extern uint16_t Utilities_extract_word_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t index);

extern uint32_t Utilities_extract_dword_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t index);

extern uint8_t Utilities_count_nulls_in_openlcb_payload(openlcb_msg_t* openlcb_msg);

extern uint8_t Utilities_is_addressed_openlcb_message(openlcb_msg_t* openlcb_msg);

extern void Utilities_set_multi_frame_flag( uint8_t* target,  uint8_t flag);  // MULTIFRAME_ONLY, MULTIFRAME_FIRST, MULTIFRAME_MIDDLE, MULTIFRAME_FINAL

extern uint8_t Utilities_is_message_for_node(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern int Utilities_is_producer_event_assigned_to_node(openlcb_node_t* openlcb_node, uint64_t event_id);

extern int Utilities_is_consumer_event_assigned_to_node(openlcb_node_t* openlcb_node, uint64_t event_id);

extern uint8_t Utilities_addressed_message_needs_processing(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern void Utilities_clone_openlcb_message(openlcb_msg_t* source, openlcb_msg_t* target);

extern uint32_t Utilities_calculate_memory_offset_into_node_space(openlcb_node_t* openlcb_node);

// OpenLcb message helpers



//extern openlcb_node_t* FindOpenLcbNodeByNodeId(openlcb_nodes_t* openlcb_nodes, uint64_t node_id);

// General helpers


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_UTILITIES__ */

