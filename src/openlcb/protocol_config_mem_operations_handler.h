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
 * @file protocol_config_mem_operations_handler.h
 * @brief Configuration memory operations protocol handler
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__
#define    __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {

    // Required function assignments
    void (*load_datagram_received_ok_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code);

    // Optional request function assignments (Encapsulated functions to perform the particular Operations Task)
    void (*operations_request_options_cmd)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_options_cmd_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_get_address_space_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_get_address_space_info_reply_present)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_get_address_space_info_reply_not_present)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_reserve_lock)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_reserve_lock_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_get_unique_id)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_get_unique_id_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_freeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_unfreeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_update_complete)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_reset_reboot)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);
    void (*operations_request_factory_reset)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);


} interface_protocol_config_mem_operations_handler_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void ProtocolConfigMemOperationsHandler_initialize(const interface_protocol_config_mem_operations_handler_t *interface_protocol_config_mem_operations_handler);

    extern void ProtocolConfigMemOperationsHandler_options_cmd(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_options_reply(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_get_address_space_info(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_reserve_lock(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_reserve_lock_reply(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_get_unique_id(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_get_unique_id_reply(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_unfreeze(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_freeze(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_update_complete(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_reset_reboot(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolConfigMemOperationsHandler_factory_reset(openlcb_statemachine_info_t * statemachine_info);



    extern void ProtocolConfigMemOperationsHandler_request_options_cmd(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    extern void ProtocolConfigMemOperationsHandler_request_get_address_space_info(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    extern void ProtocolConfigMemOperationsHandler_request_reserve_lock(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__ */

