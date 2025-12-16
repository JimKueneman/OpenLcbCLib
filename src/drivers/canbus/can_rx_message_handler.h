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
#ifndef __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__
#define __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_node.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief A structure to hold pointers to functions for dependencies this module requires, \ref can_rx_message_handler.h.
     *
     * @details OpenLcbCLib uses dependency injection to allow for writing full coverage tests as the
     * functions that are used can be modeled in the test and return valid OR invalid results to fully
     * test all program flows in the module.  It also allows for reducing the program size. If a particular
     * protocol does not need to be implemented simply filling in the dependency for that handler with a NULL
     * will strip out code for that protocols handlers and minimize the application size (bootloader is an example).
     * The library will automatically reply with the correct error/reply codes if the handler is defined as NULL
     */
    typedef struct
    {

        /*@{*/

        // REQUIRED FUNCTIONS

        can_msg_t *(*can_buffer_store_allocate_buffer)(void);
        openlcb_msg_t *(*openlcb_buffer_store_allocate_buffer)(payload_type_enum payload_type);
        alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);
        alias_mapping_t *(*alias_mapping_find_mapping_by_node_id)(node_id_t node_id);
        alias_mapping_info_t *(*alias_mapping_get_alias_mapping_info)(void);
        void (*alias_mapping_set_has_duplicate_alias_flag)(void);

        // OPTIONAL FUNCTION

        // CALLBACK FUNCTIONS

        /*@}*/

    } interface_can_rx_message_handler_t;

    extern void CanRxMessageHandler_initialize(const interface_can_rx_message_handler_t *interface_can_frame_message_handler);

    extern void CanRxMessageHandler_first_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);

    extern void CanRxMessageHandler_middle_frame(can_msg_t *can_msg, uint8_t offset);

    extern void CanRxMessageHandler_last_frame(can_msg_t *can_msg, uint8_t offset);

    extern void CanRxMessageHandler_single_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);

    extern void CanRxMessageHandler_can_legacy_snip(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);

    extern void CanRxMessageHandler_stream_frame(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);

    extern void CanRxMessageHandler_rid_frame(can_msg_t *can_msg);

    extern void CanRxMessageHandler_amd_frame(can_msg_t *can_msg);

    extern void CanRxMessageHandler_ame_frame(can_msg_t *can_msg);

    extern void CanRxMessageHandler_amr_frame(can_msg_t *can_msg);

    extern void CanRxMessageHandler_error_info_report_frame(can_msg_t *can_msg);

    extern void CanRxMessageHandler_cid_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_FRAME_MESSAGE_HANDLER__ */
