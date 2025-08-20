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
 * \file can_rx_message_hander.h
 *
 * As CAN only frame messages come in they need to be processed by the node(s) to see
 * if there is a response required.  These are the handlers called by the CAN main
 * statemachine to accomplish that.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_RX_MESSAGE_HANDLER__
#define	__CAN_RX_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_node.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    typedef struct {
        openlcb_node_t *(*find_by_alias)(uint16_t alias);
        openlcb_node_t* (*get_first)(uint8_t key);
        openlcb_node_t* (*get_next)(uint8_t key);
        openlcb_msg_t *(*openlcb_buffer_store_allocate_buffer)(payload_type_enum_t payload_type);

    } interface_can_frame_message_handler_t;



    extern void CanRxMessageHandler_initialize(const interface_can_frame_message_handler_t *interface);

    extern void CanRxMessageHandler_cid(can_msg_t* can_msg);

    extern void CanRxMessageHandler_rid(can_msg_t* can_msg);

    extern void CanRxMessageHandler_amd(can_msg_t* can_msg);

    extern void CanRxMessageHandler_ame(can_msg_t* can_msg);

    extern void CanRxMessageHandler_amr(can_msg_t* can_msg);

    extern void CanRxMessageHandler_error_information_report(can_msg_t* can_msg);

    extern void CanRxMessageHandler_handle_first_frame(can_msg_t* can_msg, uint8_t offset, payload_type_enum_t data_type);

    extern void CanRxMessageHandler_handle_middle_frame(can_msg_t* can_msg, uint8_t offset);

    extern void CanRxMessageHandler_handle_last_frame(can_msg_t* can_msg, uint8_t offset);

    extern void CanRxMessageHandler_handle_single_frame(can_msg_t* can_msg, uint8_t offset, payload_type_enum_t data_type);

    extern void CanRxMessageHandler_handle_can_legacy_snip(can_msg_t* can_msg, uint8_t offset, payload_type_enum_t data_type);

    extern void CanRxMessageHandler_handle_stream(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum_t data_type);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_FRAME_MESSAGE_HANDLER__ */

