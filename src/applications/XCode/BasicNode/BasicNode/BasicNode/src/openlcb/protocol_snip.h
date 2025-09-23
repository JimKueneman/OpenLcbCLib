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
 * \file protocol_snip.
 * 
 * Implementation of the Openlcb Simple Node Identification Protocol (SNIP).
 * Handlers are call from the openlcb_main_statemachine.c processing when a 
 * datagram message is being processed from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __PROTOCOL_SNIP__
#define	__PROTOCOL_SNIP__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {
    
    uint16_t(*configuration_memory_read)(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

} interface_openlcb_protocol_snip_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void ProtocolSnip_initialize(const interface_openlcb_protocol_snip_t *interface_openlcb_protocol_snip);

    extern void ProtocolSnip_handle_simple_node_info_request(openlcb_statemachine_info_t *statemachine_info);

    extern void ProtocolSnip_handle_simple_node_info_reply(openlcb_statemachine_info_t *statemachine_info);

    extern uint16_t ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

    extern uint16_t ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes);

    extern uint16_t ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes);

    extern uint16_t ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes);

    extern uint16_t ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes);

    extern uint16_t ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes);

    extern uint16_t ProtocolSnip_load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes);

    extern uint16_t ProtocolSnip_load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_offset, uint16_t requested_bytes);
    
    extern bool ProtocolSnip_validate_snip_reply(openlcb_msg_t* snip_reply_msg);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __PROTOCOL_SNIP__ */

