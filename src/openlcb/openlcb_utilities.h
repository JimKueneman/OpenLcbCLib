
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
 * \file openlcb_utilities.h
 *
 * Common utility functions that are useful to work on buffers and node structures
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_UTILITIES__
#define	__OPENLCB_UTILITIES__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

extern void OpenLcbUtilities_load_openlcb_message(openlcb_msg_t* openlcb_msg, uint16_t source_alias, uint64_t source_id, uint16_t dest_alias, uint64_t dest_id, uint16_t mti, uint16_t payload_count);

extern void OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, event_id_t event_id);

extern void OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg_t* openlcb_msg, node_id_t node_id, uint16_t index);

extern void OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t word, uint16_t index);

extern void OpenLcbUtilities_copy_dword_to_openlcb_payload(openlcb_msg_t* openlcb_msg, uint32_t doubleword, uint16_t index);

extern uint16_t OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const char string[], uint16_t index);

extern uint16_t OpenLcbUtilities_copy_byte_array_to_openlcb_payload(openlcb_msg_t* openlcb_msg, const uint8_t byte_array[], uint16_t payload_index, uint16_t requested_bytes);

extern void OpenLcbUtilities_clear_openlcb_message_payload(openlcb_msg_t* openlcb_msg); 

extern node_id_t OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t index);

extern event_id_t OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg_t* openlcb_msg);

extern uint16_t OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t index);

extern uint32_t OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg_t* openlcb_msg, uint16_t index);

extern uint8_t OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg_t* openlcb_msg);

extern bool OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg_t* openlcb_msg);

extern void OpenLcbUtilities_set_multi_frame_flag( uint8_t* target,  uint8_t flag);  // MULTIFRAME_ONLY, MULTIFRAME_FIRST, MULTIFRAME_MIDDLE, MULTIFRAME_FINAL

extern bool OpenLcbUtilities_is_addressed_message_for_node(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern bool OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node_t* openlcb_node, event_id_t event_id, uint16_t* event_index);

extern bool OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node_t* openlcb_node, event_id_t event_id, uint16_t* event_index);

extern bool OpenLcbUtilities_addressed_message_needs_processing(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg);

extern uint32_t OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node_t* openlcb_node);

extern uint16_t OpenLcbUtilities_payload_type_to_len(payload_type_enum_t payload_type);

extern node_id_t OpenLcbUtilities_extract_node_id_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

extern uint16_t OpenLcbUtilities_extract_word_from_config_mem_buffer(configuration_memory_buffer_t *buffer, uint8_t index);

extern void OpenLcbUtilities_copy_node_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, node_id_t node_id, uint8_t index);

extern void OpenLcbUtilities_copy_event_id_to_config_mem_buffer(configuration_memory_buffer_t *buffer, event_id_t event_id, uint8_t index);

extern event_id_t OpenLcbUtilities_copy_config_mem_buffer_to_event_id(configuration_memory_buffer_t *buffer, uint8_t index);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_UTILITIES__ */

