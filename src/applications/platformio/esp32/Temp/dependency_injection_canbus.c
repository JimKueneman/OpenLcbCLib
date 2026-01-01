/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file dependency_injection.c
 * 
 * This dependency injection file can be reused as is and the Application level drivers can be assigned
 * in the dependency_injection_defines.h file.
 *
 *
 * @author Jim Kueneman
 * @date 27 Nov 2025
 */

#include "dependency_injection_canbus.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "dependency_injection_defines.h"

#include "../../../drivers/canbus/can_types.h"
#include "../../../drivers/canbus/can_utilities.h"
#include "../../../drivers/canbus/can_buffer_store.h"
#include "../../../drivers/canbus/can_buffer_fifo.h"
#include "../../../drivers/canbus/can_login_message_handler.h"
#include "../../../drivers/canbus/can_login_statemachine.h"
#include "../../../drivers/canbus/can_rx_message_handler.h"
#include "../../../drivers/canbus/can_rx_statemachine.h"
#include "../../../drivers/canbus/can_tx_message_handler.h"
#include "../../../drivers/canbus/can_tx_statemachine.h"
#include "../../../drivers/canbus/can_main_statemachine.h"
#include "../../../drivers/canbus/alias_mappings.h"

const interface_can_login_message_handler_t interface_can_login_message_handler = {

    .alias_mapping_register = &AliasMappings_register,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    // Callback events
    .on_alias_change = ON_ALIAS_CHANGE_CALLBACK // application defined
};

const interface_can_login_state_machine_t interface_can_login_state_machine = {

    .state_init = &CanLoginMessageHandler_state_init,
    .state_generate_seed = &CanLoginMessageHandler_state_generate_seed,
    .state_generate_alias = &CanLoginMessageHandler_state_generate_alias,
    .state_load_cid07 = &CanLoginMessageHandler_state_load_cid07,
    .state_load_cid06 = &CanLoginMessageHandler_state_load_cid06,
    .state_load_cid05 = &CanLoginMessageHandler_state_load_cid05,
    .state_load_cid04 = &CanLoginMessageHandler_state_load_cid04,
    .state_wait_200ms = &CanLoginMessageHandler_state_wait_200ms,
    .state_load_rid = &CanLoginMessageHandler_state_load_rid,
    .state_load_amd = &CanLoginMessageHandler_state_load_amd

};

const interface_can_rx_message_handler_t interface_can_rx_message_handler = {

    .can_buffer_store_allocate_buffer = &CanBufferStore_allocate_buffer,
    .openlcb_buffer_store_allocate_buffer = OPENLCB_BUFFER_STORE_ALLOCATE_BUFFER_FUNC,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    .alias_mapping_find_mapping_by_node_id = &AliasMappings_find_mapping_by_node_id,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_set_has_duplicate_alias_flag = &AliasMappings_set_has_duplicate_alias_flag

};

const interface_can_rx_statemachine_t interface_can_rx_statemachine = {

    .handle_can_legacy_snip = &CanRxMessageHandler_can_legacy_snip,
    .handle_single_frame = &CanRxMessageHandler_single_frame,
    .handle_first_frame = &CanRxMessageHandler_first_frame,
    .handle_middle_frame = &CanRxMessageHandler_middle_frame,
    .handle_last_frame = &CanRxMessageHandler_last_frame,
    .handle_stream_frame = &CanRxMessageHandler_stream_frame,
    .handle_rid_frame = CanRxMessageHandler_rid_frame,
    .handle_amd_frame = CanRxMessageHandler_amd_frame,
    .handle_ame_frame = CanRxMessageHandler_ame_frame,
    .handle_amr_frame = CanRxMessageHandler_amr_frame,
    .handle_error_info_report_frame = CanRxMessageHandler_error_info_report_frame,
    .handle_cid_frame = CanRxMessageHandler_cid_frame,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    
    // Callback events
    .on_receive = ON_CAN_RX_CALLBACK // application defined

};

const interface_can_tx_message_handler_t interface_can_tx_message_handler = {

    .transmit_can_frame = TRANSMIT_CAN_FRAME_FUNC, //  HARDWARE INTERFACE
    
    // Callback events
    .on_transmit = ON_CAN_TX_CALLBACK // application defined

};

const interface_can_tx_statemachine_t interface_can_tx_statemachine = {

    .is_tx_buffer_empty = IS_TX_BUFFER_EMPTY_FUNC, //  HARDWARE INTERFACE
    .handle_addressed_msg_frame = &CanTxMessageHandler_addressed_msg_frame,
    .handle_unaddressed_msg_frame = &CanTxMessageHandler_unaddressed_msg_frame,
    .handle_datagram_frame = &CanTxMessageHandler_datagram_frame,
    .handle_stream_frame = &CanTxMessageHandler_stream_frame,
    .handle_can_frame = &CanTxMessageHandler_can_frame

};

const interface_can_main_statemachine_t interface_can_main_statemachine = {

    .lock_shared_resources = LOCK_SHARED_RESOURCES_FUNC,     //  HARDWARE INTERFACE
    .unlock_shared_resources = UNLOCK_SHARED_RESOURCES_FUNC, //  HARDWARE INTERFACE
    .send_can_message = &CanTxStatemachine_send_can_message,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .openlcb_node_find_by_alias = &OpenLcbNode_find_by_alias,
    .login_statemachine_run = &CanLoginStateMachine_run,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_unregister = &AliasMappings_unregister,

    .handle_duplicate_aliases = &CanMainStatemachine_handle_duplicate_aliases,
    .handle_outgoing_can_message = &CanMainStatemachine_handle_outgoing_can_message,
    .handle_login_outgoing_can_message = &CanMainStatemachine_handle_login_outgoing_can_message,
    .handle_try_enumerate_first_node = &CanMainStatemachine_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &CanMainStatemachine_handle_try_enumerate_next_node

};

void DependencyInjectionCanBus_initialize(void)
{

  CanBufferStore_initialize();
  CanBufferFifo_initialize();

  CanRxMessageHandler_initialize(&interface_can_rx_message_handler);
  CanRxStatemachine_initialize(&interface_can_rx_statemachine);

  CanTxMessageHandler_initialize(&interface_can_tx_message_handler);
  CanTxStatemachine_initialize(&interface_can_tx_statemachine);

  CanLoginMessageHandler_initialize(&interface_can_login_message_handler);
  CanLoginStateMachine_initialize(&interface_can_login_state_machine);
  CanMainStatemachine_initialize(&interface_can_main_statemachine);

  AliasMappings_initialize();

}
