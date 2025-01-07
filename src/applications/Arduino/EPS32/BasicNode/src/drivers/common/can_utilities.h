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
 * \file can_utilities.h
 *
 * Utilities to help deal with the CAN buffers
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef ___CAN_UTILITIES__
#define	___CAN_UTILITIES__

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


extern void CanUtilties_load_can_message(can_msg_t* can_msg, uint32_olcb_t identifier, uint8_olcb_t payload_size, uint8_olcb_t byte1, uint8_olcb_t byte2 , uint8_olcb_t byte3 , uint8_olcb_t byte4, uint8_olcb_t byte5, uint8_olcb_t byte6, uint8_olcb_t byte7, uint8_olcb_t byte8);

extern void CanUtilities_clear_can_message(can_msg_t* can_msg);


extern void CanUtilities_copy_node_id_to_payload(can_msg_t* can_msg, uint64_olcb_t node_id, uint8_olcb_t start_offset);

extern uint8_olcb_t CanUtilities_copy_can_payload_to_openlcb_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_olcb_t can_start_index);

extern uint8_olcb_t CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint16_olcb_t openlcb_start_index, uint8_olcb_t can_start_index);

extern uint8_olcb_t CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg, uint8_olcb_t can_start_index);

extern void CanUtilities_copy_64_bit_to_can_message(can_msg_t* can_msg, uint64_olcb_t data);

extern void CanUtilities_copy_can_message(can_msg_t* can_msg_source, can_msg_t* can_msg_target);


extern uint64_olcb_t CanUtilities_extract_can_payload_as_node_id(can_msg_t* can_msg);

extern uint16_olcb_t CanUtilities_extract_source_alias_from_can_message(can_msg_t* can_msg);

extern uint16_olcb_t CanUtilties_extract_dest_alias_from_can_message(can_msg_t* can_msg);

extern uint16_olcb_t CanUtilties_extract_can_mti_from_can_identifier(can_msg_t* can_msg);  // caution this return 0 if the can message is a datagram/stream or other message that has a MTI that won't fit in 12 bits



extern uint16_olcb_t CanUtilties_convert_can_mti_to_openlcb_mti(can_msg_t* can_msg);

extern uint16_olcb_t CanUtilities_is_dest_alias_in_can_payload(can_msg_t* can_msg);

extern uint8_olcb_t CanUtilities_count_nulls_in_can_payload(can_msg_t* can_msg);

extern uint8_olcb_t CanUtilities_count_nulls_in_payloads(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg);

extern uint8_olcb_t CanUtilities_is_openlcb_message(can_msg_t* msg);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

