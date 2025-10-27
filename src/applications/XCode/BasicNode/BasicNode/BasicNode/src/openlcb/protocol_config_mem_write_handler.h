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
 * \file protocol_config_mem_write_handler.h
 *
 *
 * @author Jim Kueneman
 * @date 22 Oct 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef _PROTOCOL_CONFIG_MEM_WRITE_HANDLER_
#define	_PROTOCOL_CONFIG_MEM_WRITE_HANDLER_

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {
      
    void (*load_datagram_received_ok_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);
    
    // Callbacks
    void (*on_write_space_config_decscription_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
    void (*on_write_space_all)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
    void (*on_write_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
    void (*on_write_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
    void (*on_write_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
    void (*on_write_space_traction_config_decscription_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
    void (*on_write_space_traction_config_memory)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);
    
} interface_protocol_config_mem_write_handler_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void ProtocolConfigMemWriteHandler_initialize(const interface_protocol_config_mem_write_handler_t *interface_protocol_config_mem_write_handler);

    extern void ProtocolConfigMemWriteHandler_memory_write_space_config_description_info(openlcb_statemachine_info_t *statemachine_info);
    extern void ProtocolConfigMemWriteHandler_memory_write_space_all(openlcb_statemachine_info_t *statemachine_info);
    extern void ProtocolConfigMemWriteHandler_memory_write_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info);
    extern void ProtocolConfigMemWriteHandler_memory_write_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info);
    extern void ProtocolConfigMemWriteHandler_memory_write_space_acdi_user(openlcb_statemachine_info_t *statemachine_info);
    extern void ProtocolConfigMemWriteHandler_memory_write_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info);
    extern void ProtocolConfigMemWriteHandler_memory_write_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info);
    
    extern void ProtocolConfigMemWriteHandler_memory_write_under_mask_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

    extern void ProtocolConfigMemWriteHandler_memory_write_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);
    extern void ProtocolConfigMemWriteHandler_memory_write_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);
    extern void ProtocolConfigMemWriteHandler_memory_write_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);
    

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* _PROTOCOL_CONFIG_MEM_WRITE_HANDLER_ */