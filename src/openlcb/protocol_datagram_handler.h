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
 * \file protocol_datagram.h
 *
 * Implementation of the Openlcb Datagram Protocol.  Handlers are call from the 
 * openlcb_main_statemachine.c processing when a datagram message is being processed
 * from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __PROTOCOL_DATAGRAM_HANDLER__
#define	__PROTOCOL_DATAGRAM_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {
    // Config Memory Read
    void (*memory_read_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_all)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Read Reply = Ok
    void (*memory_read_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Read Reply = Failed
    void (*memory_read_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Stream Read
    void (*memory_read_stream_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_all)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Stream Read Reply = Ok
    void (*memory_read_stream_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Stream Read Reply = Failed
    void (*memory_read_stream_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Write
    void (*memory_write_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_all)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Write Reply = Ok
    void (*memory_write_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Write Reply = Fail
    void (*memory_write_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Write Under Mask
    void (*memory_write_under_mask_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_all)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);
    
    // Config Memory Write Stream
    void (*memory_write_stream_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_all)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Write Stream Reply = Ok
    void (*memory_write_stream_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

    // Config Memory Write Stream Reply = Fail
    void (*memory_write_stream_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // Config Commands
    void (*memory_options_cmd)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_options_reply)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_address_space_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_address_space_info_reply_not_present)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_address_space_info_reply_present)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_reserve_lock)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_reserve_lock_reply)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_unique_id)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_unique_id_reply)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_unfreeze)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_freeze)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_update_complete)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_reset_reboot)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_factory_reset)(openlcb_statemachine_info_t *statemachine_info);

    void (*lock_shared_resources)(void);
    void (*unlock_shared_resources)(void);

} interface_protocol_datagram_handler_t;


typedef void(*memory_handler_t)(openlcb_statemachine_info_t *statemachine_info);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void ProtocolDatagramHandler_initialize(const interface_protocol_datagram_handler_t *interface_protocol_datagram_handler);

    extern void ProtocolDatagramHandler_load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

    extern void ProtocolDatagramHandler_load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

    extern void ProtocolDatagramHandler_handle_datagram(openlcb_statemachine_info_t * statemachine_info);

    extern void Protocol_DatagramHandler_handle_datagram_received_ok(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolDatagramHandler_handle_datagram_rejected(openlcb_statemachine_info_t * statemachine_info);

    extern void ProtocolDatagramHandler_clear_resend_datagram_message(openlcb_node_t * openlcb_node);

    extern void ProtocolDatagramHandler_100ms_timer_tick(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __PROTOCOL_DATAGRAM_HANDLER__ */

