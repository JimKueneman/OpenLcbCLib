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
 * \file can_login_message_handler.c
 *
 * When a node is logging into the network on a CAN bus it must follow a specific
 * flow to allocate a unique alias ID and broadcast its events.  This is the handler 
 * that is called from the CAN main statemachine to accomplish that when a new node
 * is created.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_login_message_handler.h"

#include "stdio.h"  // printf
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_main_statemachine.h"
#include "../../openlcb/callback_hooks.h"
#include "../../openlcb/protocol_event_transport.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "can_tx_statemachine.h"
#include "can_utilities.h"
#include "can_buffer_fifo.h"
#include "can_frame_message_handler.h"
#include "can_buffer_store.h"
#include "../driver_mcu.h"





void CanLoginMessageHandler_init(openlcb_node_t* next_node) {

    next_node->seed = next_node->id;
    next_node->alias = Node_generate_alias(next_node->seed);

    next_node->state.run_state = RUNSTATE_GENERATE_ALIAS; // Jump over Generate Seed that only is if we have an Alias conflict and have to jump back

}

void CanFrameMessageHandler_generate_seed(openlcb_node_t* next_node) {

    next_node->seed = Node_generate_seed(next_node->seed);
    next_node->state.run_state = RUNSTATE_GENERATE_ALIAS;

}

void CanFrameMessageHandler_generate_alias(openlcb_node_t* next_node) {

    next_node->alias = Node_generate_alias(next_node->seed);
    
    alias_change_callback_t alias_change_callback = CallbackHooks_get_alias_change();

    if (alias_change_callback)
        alias_change_callback(next_node->alias, next_node->id);

    next_node->state.run_state = RUNSTATE_SEND_CHECK_ID_07;

}

void CanFrameMessageHandler_transmit_cid07(openlcb_node_t* next_node, can_msg_t* worker_msg) {

    worker_msg->payload_count = 0;
    worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((next_node->id >> 24) & 0xFFF000) | next_node->alias); // AA0203040506


    if (CanTxStatemachine_try_transmit_can_message(worker_msg))
        next_node->state.run_state = RUNSTATE_SEND_CHECK_ID_06;

}

void CanFrameMessageHandler_transmit_cid06(openlcb_node_t* next_node, can_msg_t* worker_msg) {

    worker_msg->payload_count = 0;
    worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((next_node->id >> 12) & 0xFFF000) | next_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(worker_msg))
        next_node->state.run_state = RUNSTATE_SEND_CHECK_ID_05;

}

void CanFrameMessageHandler_transmit_cid05(openlcb_node_t* next_node, can_msg_t* worker_msg) {

    worker_msg->payload_count = 0;
    worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((next_node->id & 0xFFF000) | next_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(worker_msg))
        next_node->state.run_state = RUNSTATE_SEND_CHECK_ID_04;

}

void CanFrameMessageHandler_transmit_cid04(openlcb_node_t* next_node, can_msg_t* worker_msg) {

    worker_msg->payload_count = 0;
    worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((next_node->id << 12) & 0xFFF000) | next_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(worker_msg))
        next_node->state.run_state = RUNSTATE_WAIT_200ms;

}

void CanFrameMessageHandler_wait_200ms(openlcb_node_t* next_node) {

    if (next_node->timerticks > 3)
        next_node->state.run_state = RUNSTATE_TRANSMIT_RESERVE_ID;

}

void CanFrameMessageHandler_transmit_rid(openlcb_node_t* next_node, can_msg_t* worker_msg) {

    worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | next_node->alias;
    worker_msg->payload_count = 0;

    if (CanTxStatemachine_try_transmit_can_message(worker_msg))
        next_node->state.run_state = RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION;

}

void CanFrameMessageHandler_transmit_amd(openlcb_node_t* next_node, can_msg_t* worker_msg) {

    worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | next_node->alias;
    CanUtilities_copy_node_id_to_payload(worker_msg, next_node->id, 0);


    if (CanTxStatemachine_try_transmit_can_message(worker_msg)) {

        next_node->state.permitted = TRUE;
        next_node->state.run_state = RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE;

    }

}

void CanFrameMessageHandler_transmit_initialization_complete(openlcb_node_t* next_node, can_msg_t* can_worker, openlcb_msg_t* openlcb_worker) {

    Utilities_load_openlcb_message(
            openlcb_worker,
            next_node->alias,
            next_node->id,
            0,
            0,
            MTI_INITIALIZATION_COMPLETE,
            6
            );


    if (next_node->parameters->protocol_support & PSI_SIMPLE)
        openlcb_worker->mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    Utilities_copy_node_id_to_openlcb_payload(openlcb_worker, next_node->id, 0);


    if (CanTxStatemachine_try_transmit_openlcb_message(can_worker, openlcb_worker, 0)) {

        next_node->state.initalized = TRUE;
        next_node->producers.enumerator.running = TRUE;
        next_node->producers.enumerator.enum_index = 0;
        next_node->state.run_state = RUNSTATE_TRANSMIT_PRODUCER_EVENTS;

    }

}

void CanFrameMessageHandler_transmit_producer_events(openlcb_node_t* next_node, can_msg_t* can_worker, openlcb_msg_t* openlcb_worker) {


    if (next_node->producers.enumerator.running) {

        if (next_node->producers.enumerator.enum_index < next_node->parameters->producer_count) {

            Utilities_load_openlcb_message(
                    openlcb_worker,
                    next_node->alias,
                    next_node->id,
                    0,
                    0,
                    ProtocolEventTransport_extract_producer_event_state_mti(next_node, next_node->producers.enumerator.enum_index),
                    6
                    );

            Utilities_copy_event_id_to_openlcb_payload(openlcb_worker, next_node->producers.list[next_node->producers.enumerator.enum_index]);


            if (CanTxStatemachine_try_transmit_openlcb_message(can_worker, openlcb_worker, 0)) {

                next_node->producers.enumerator.enum_index = next_node->producers.enumerator.enum_index + 1;

                if (next_node->producers.enumerator.enum_index >= next_node->parameters->producer_count) {

                    next_node->producers.enumerator.enum_index = 0;
                    next_node->producers.enumerator.running = FALSE;
                    next_node->consumers.enumerator.enum_index = 0;
                    next_node->consumers.enumerator.running = TRUE;

                    next_node->state.run_state = RUNSTATE_TRANSMIT_CONSUMER_EVENTS;

                }

            }

        }

    }

}

void CanFrameMessageHandler_transmit_consumer_events(openlcb_node_t* next_node, can_msg_t* can_worker, openlcb_msg_t* openlcb_worker) {

    if (next_node->consumers.enumerator.running) {

        if (next_node->consumers.enumerator.enum_index < next_node->parameters->producer_count) {

            Utilities_load_openlcb_message(
                    openlcb_worker,
                    next_node->alias,
                    next_node->id,
                    0,
                    0,
                    ProtocolEventTransport_extract_consumer_event_state_mti(next_node, next_node->producers.enumerator.enum_index),
                    6
);

            Utilities_copy_event_id_to_openlcb_payload(openlcb_worker, next_node->consumers.list[next_node->consumers.enumerator.enum_index]);


            if (CanTxStatemachine_try_transmit_openlcb_message(can_worker, openlcb_worker, 0)) {

                next_node->consumers.enumerator.enum_index = next_node->consumers.enumerator.enum_index + 1;

                if (next_node->consumers.enumerator.enum_index >= next_node->parameters->producer_count) {

                    next_node->consumers.enumerator.running = FALSE;
                    next_node->consumers.enumerator.enum_index = 0;

                    next_node->state.run_state = RUNSTATE_RUN;

                }

            }

        }

    }


}
