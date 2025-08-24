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
 * \file protocol_datagram_handlers.h
 *
 *
 * @author Jim Kueneman
 * @date 28 July 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __PROTOCOL_DATAGRAM_CONFIG_MEM_HANDLER__
#define	__PROTOCOL_DATAGRAM_CONFIG_MEM_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {
    uint16_t(*configuration_memory_read)(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);
    uint16_t(*configuration_memory_write) (uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);
    void (*reboot)(void);
    void (*configuration_memory_factory_reset)(void);
    uint16_t(*snip_load_manufacturer_version_id)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
    uint16_t(*snip_load_name)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
    uint16_t(*snip_load_model)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
    uint16_t(*snip_load_hardware_version)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
    uint16_t(*snip_load_software_version)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
    uint16_t(*snip_load_user_version_id)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
    uint16_t(*snip_load_user_name)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
    uint16_t(*snip_load_user_description)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);
    // Callback events
    void (*on_configuration_memory_factory_reset)(void);
    void (*on_config_mem_write)(uint32_t address, uint16_t bytes_written, configuration_memory_buffer_t* config_mem_buffer);
    void (*onconfig_mem_freeze_firmware_update) (openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);
    void (*onconfig_mem_unfreeze_firmware_update) (openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

} interface_openlcb_protocol_datagram_config_mem_handler_t;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


    extern void ProtocolDatagramConfigMemHandler_initialize(const interface_openlcb_protocol_datagram_config_mem_handler_t *interface_openlcb_protocol_datagram_config_mem_handler);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_read_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_read_reply_ok_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_read_reply_fail_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_write_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_write_reply_ok_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_write_reply_fail_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_write_under_mask_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_options_cmd_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_options_reply_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_get_address_space_info_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_get_address_space_info_reply_not_present_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_get_address_space_info_reply_present_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_reserve_lock_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_get_unique_id_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_unfreeze_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_freeze_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_update_complete_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_reset_reboot_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramConfigMemHandler_handle_memory_factory_reset_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramConfigMemHandler_send_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint16_t error_code);

    extern void ProtocolDatagramConfigMemHandler_clear_resend_datagram_message(openlcb_node_t* openlcb_node);

    extern void ProtocolDatagramConfigMemHandler_try_transmit(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __PROTOCOL_DATAGRAM_CONFIG_MEM_HANDLER__ */

