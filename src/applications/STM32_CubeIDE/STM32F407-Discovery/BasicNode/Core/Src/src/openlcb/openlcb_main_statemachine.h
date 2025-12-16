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
 * \file openlcb_main_statemachine.h
 *
 * Implementation of the Openlcb main statemachine.  You typically do not call this
 * statemachine directly it will depend on what the physical transport is and if that
 * needs to deal with CAN Adaptations of the protocol.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_MAIN_STATEMACHINE__
#define __OPENLCB_OPENLCB_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {
    // ************************************************************************
    // Required message handlers
    // ************************************************************************ 
    
    // MESSAGE NETWORK
    void (*message_network_initialization_complete)(openlcb_statemachine_info_t *statemachine_info);
    void (*message_network_initialization_complete_simple)(openlcb_statemachine_info_t *statemachine_info);
    void (*message_network_verify_node_id_addressed)(openlcb_statemachine_info_t *statemachine_info);
    void (*message_network_verify_node_id_global)(openlcb_statemachine_info_t *statemachine_info);
    void (*message_network_verified_node_id)(openlcb_statemachine_info_t *statemachine_info);
    void (*message_network_optional_interaction_rejected)(openlcb_statemachine_info_t *statemachine_info);
    void (*message_network_terminate_due_to_error)(openlcb_statemachine_info_t *statemachine_info);

    // PROTOCOL SUPPORT
    void (*message_network_protocol_support_inquiry)(openlcb_statemachine_info_t *statemachine_info);
    void (*message_network_protocol_support_reply)(openlcb_statemachine_info_t *statemachine_info);

    // General
    void (*lock_shared_resources)(void);
    void (*unlock_shared_resources)(void);
    bool (*send_openlcb_msg)(openlcb_msg_t *outgoing_msg);
    openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);
    openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);
    void (*load_interaction_rejected)(openlcb_statemachine_info_t *statemachine_info);

    // Test injection
    void (*process_main_statemachine)(openlcb_statemachine_info_t *statemachine_info);
    bool (*does_node_process_msg)(openlcb_statemachine_info_t *_statemachine_info); 
    bool (*handle_outgoing_openlcb_message)(void);
    bool (*handle_try_reenumerate)(void);
    bool (*handle_try_pop_next_incoming_openlcb_message)(void);
    bool (*handle_try_enumerate_first_node)(void);
    bool (*handle_try_enumerate_next_node)(void); 
    
    // ************************************************************************
    // Optional message handlers
    // ************************************************************************
    
    // SNIP
    void (*snip_simple_node_info_request)(openlcb_statemachine_info_t *statemachine_info);
    void (*snip_simple_node_info_reply)(openlcb_statemachine_info_t *statemachine_info);

    // EVENTS
    void (*event_transport_consumer_identify)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_consumer_range_identified)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_consumer_identified_unknown)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_consumer_identified_set)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_consumer_identified_clear)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_consumer_identified_reserved)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_producer_identify)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_producer_range_identified)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_producer_identified_unknown)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_producer_identified_set)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_producer_identified_clear)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_producer_identified_reserved)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_identify_dest)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_identify)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_learn)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_pc_report)(openlcb_statemachine_info_t *statemachine_info);
    void (*event_transport_pc_report_with_payload)(openlcb_statemachine_info_t *statemachine_info);

    // TRACTION
    void (*traction_control_command)(openlcb_statemachine_info_t *statemachine_info);
    void (*traction_control_reply)(openlcb_statemachine_info_t *statemachine_info);

    // TRACTION SNIP
    void (*simple_train_node_ident_info_request)(openlcb_statemachine_info_t *statemachine_info);
    void (*simple_train_node_ident_info_reply)(openlcb_statemachine_info_t *statemachine_info);

    // DATAGRAM
    void (*datagram)(openlcb_statemachine_info_t *statemachine_info);
    void (*datagram_ok_reply)(openlcb_statemachine_info_t *statemachine_info);
    void (*datagram_rejected_reply)(openlcb_statemachine_info_t *statemachine_info);

    // STREAM
    void (*stream_initiate_request)(openlcb_statemachine_info_t *statemachine_info);
    void (*stream_initiate_reply)(openlcb_statemachine_info_t *statemachine_info);
    void (*stream_send_data)(openlcb_statemachine_info_t *statemachine_info);
    void (*stream_data_proceed)(openlcb_statemachine_info_t *statemachine_info);
    void (*stream_data_complete)(openlcb_statemachine_info_t *statemachine_info);
    

} interface_openlcb_main_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void OpenLcbMainStatemachine_initialize(const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine);

    extern void OpenLcbMainStatemachine_run(void);

    // exposed for test
    extern void OpenLcbMainStatemachine_process_main_statemachine(openlcb_statemachine_info_t *statemachine_info);

    extern bool OpenLcbMainStatemachine_does_node_process_msg(openlcb_statemachine_info_t *statemachine_info);

    extern void OpenLcbMainStatemachine_load_interaction_rejected(openlcb_statemachine_info_t *statemachine_info);

    extern bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void);

    extern bool OpenLcbMainStatemachine_handle_try_reenumerate(void);

    extern bool OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message(void);

    extern bool OpenLcbMainStatemachine_handle_try_enumerate_first_node(void);

    extern bool OpenLcbMainStatemachine_handle_try_enumerate_next_node(void);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_MAIN_STATEMACHINE__ */
