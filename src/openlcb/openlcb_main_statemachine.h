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
#ifndef __OPENLCB_MAIN_STATEMACHINE__
#define __OPENLCB_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {
    // SNIP
    bool (*snip_simple_node_info_request)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);

    // MESSAGE NETWORK
    bool (*message_network_protocol_support_inquiry)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*message_network_verify_node_id_addressed)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*message_network_verify_node_id_global)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*message_network_verified_node_id_addressed)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*optional_interaction_rejected)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);

    // EVENTS
    bool (*event_transport_consumer_identify)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_consumer_identify_range)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_consumer_identified_unknown)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_consumer_identified_set)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_consumer_identified_clear)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_consumer_identified_reserved)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_producer_identify)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_producer_identify_range)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_producer_identified_unknown)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_producer_identified_set)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_producer_identified_clear)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_producer_identified_reserved)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_identify_dest)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_identify)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_learn)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_pc_report)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*event_transport_pc_report_with_payload)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    // DATAGRAM
    bool (*datagram)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*datagram_ok_reply)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*datagram_rejected_reply)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);

    void (*send_interaction_rejected)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    openlcb_node_t *(*node_get_first)(uint8_t key);
    openlcb_node_t *(*node_get_next)(uint8_t key);
    bool (*transmit_openlcb_message)(openlcb_msg_t *openlcb_msg);
    void (*lock_openlcb_buffer_fifo)(void);
    void (*unlock_openlcb_buffer_fifo)(void);
    bool (*try_transmit_active_msg)(openlcb_msg_t *active_outgoing_msg);

    // for test injection, leave null to use the default functions
    bool (*process_main_statemachine)(openlcb_node_t *openlcb_node, openlcb_msg_t *incoming_msg, openlcb_msg_t *outgoing_msg);
    bool (*does_node_process_msg)(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg);
    openlcb_msg_t *(*try_free_current_and_pop_next_incoming_msg)(openlcb_msg_t *active_incoming_msg);
    bool (*try_reprocess_active_node)(openlcb_node_t *active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg);
    bool (*process_node)(openlcb_node_t *active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg);
    bool (*try_process_first_node)(openlcb_node_t **active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg);
    bool (*try_process_next_node)(openlcb_node_t **active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg);

} interface_openlcb_main_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void OpenLcbMainStatemachine_initialize(const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine);

    extern void OpenLcbMainStatemachine_run(void);

    extern void OpenLcbMainStatemachine_run_single_node(openlcb_node_t *openlcb_node);

    // exposed for test
    extern bool OpenLcbMainStatemachine_process_main_statemachine(openlcb_node_t* openlcb_node, openlcb_msg_t* incoming_msg, openlcb_msg_t* outgoing_msg);
    extern bool OpenLcbMainStatemachine_does_node_process_msg(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg);
    extern bool OpenLcbMainStatemachine_try_transmit_active_msg(openlcb_msg_t *active_outgoing_msg);
    extern openlcb_msg_t *OpenLcbMainStatemachine_try_free_current_and_pop_next_incoming_msg(openlcb_msg_t *active_incoming_msg);
    extern bool OpenLcbMainStatemachine_try_reprocess_active_node(openlcb_node_t *active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg);
    extern bool OpenLcbMainStatemachine_process_node(openlcb_node_t *active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg);
    extern bool OpenLcbMainStatemachine_try_process_first_node(openlcb_node_t **active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg);
    extern bool OpenLcbMainStatemachine_try_process_next_node(openlcb_node_t **active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_MAIN_STATEMACHINE__ */
