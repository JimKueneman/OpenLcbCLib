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
 * \file can_rx_statemachine.h
 *
 * Handles the raw CAN frame structures and Pushes them onto the FIFO stack to be
 * processed in the main loop.  If the message is across multiple CAN frames it uses 
 * the Buffer List to hold the OpenLcb message as it builds it through gathering the
 * various CAN frames that make it up.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__
#define	__DRIVERS_CANBUS_CAN_RX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

typedef struct {
    void (*handle_can_legacy_snip)(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);
    void (*handle_single_frame)(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);
    void (*handle_first_frame)(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);
    void (*handle_middle_frame)(can_msg_t* can_msg, uint8_t can_buffer_start_index);
    void (*handle_last_frame)(can_msg_t* can_msg, uint8_t can_buffer_start_index);
    void (*handle_stream_frame)(can_msg_t* can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);
    void (*handle_rid_frame)(can_msg_t* can_msg);
    void (*handle_amd_frame)(can_msg_t* can_msg);
    void (*handle_ame_frame)(can_msg_t* can_msg);
    void (*handle_amr_frame)(can_msg_t* can_msg);
    void (*handle_error_info_report_frame)(can_msg_t* can_msg);
    void (*handle_cid_frame)(can_msg_t* can_msg);
    alias_mapping_t*(*alias_mapping_find_mapping_by_alias)(uint16_t alias);
    // Callback events
    void (*on_receive)(can_msg_t* can_msg);

} interface_can_rx_statemachine_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void CanRxStatemachine_initialize(const interface_can_rx_statemachine_t *interface_can_rx_statemachine);
    
    extern void CanRxStatemachine_incoming_can_driver_callback(can_msg_t* can_msg);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVERS_CANBUS_CAN_RX_STATEMACHINE__ */

