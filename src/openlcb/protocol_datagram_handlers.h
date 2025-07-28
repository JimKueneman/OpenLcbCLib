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
#ifndef __PROTOCOL_DATAGRAM_HANDLERS__
#define	__PROTOCOL_DATAGRAM_HANDLERS__

#include "openlcb_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


    extern void ProtocolDatagramHandlers_handle_memory_read_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space, uint8_olcb_t return_msg_ok, uint8_olcb_t return_msg_fail);

    extern void ProtocolDatagramHandlers_handle_memory_read_reply_ok_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space);

    extern void ProtocolDatagramHandlers_handle_memory_read_reply_fail_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space);

    extern void ProtocolDatagramHandlers_handle_memory_write_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space, uint8_olcb_t return_msg_ok, uint8_olcb_t return_msg_fail);

    extern void ProtocolDatagramHandlers_handle_memory_write_reply_ok_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space);

    extern void ProtocolDatagramHandlers_handle_memory_write_reply_fail_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space);

    extern void ProtocolDatagramHandlers_handle_memory_write_under_mask_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_olcb_t space, uint8_olcb_t return_msg_ok, uint8_olcb_t return_msg_fail);

    extern void ProtocolDatagramHandlers_handle_memory_options_cmd_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_options_reply_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_get_address_space_info_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_get_address_space_info_reply_not_present_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_get_address_space_info_reply_present_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_reserve_lock_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_get_unique_id_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_unfreeze_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_freeze_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_update_complete_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_reset_reboot_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_handle_memory_factory_reset_message(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg);

    extern void ProtocolDatagramHandlers_send_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint16_olcb_t error_code);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __PROTOCOL_DATAGRAM_HANDLERS__ */

