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
 * \file can_main_statemachine.c
 *
 * Where the real work in dispatching the incoming OpenLcb messages to the various
 * handlers to process.  It will call the OpenLcb main statemachine when needed.  
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_main_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_buffer_fifo.h"
#include "can_frame_message_handler.h"
#include "can_buffer_store.h"
#include "can_login_message_handler.h"
#include "can_tx_statemachine.h"
#include "can_rx_statemachine.h"

#include "../driver_can.h"
#include "../driver_100ms_clock.h"

#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_main_statemachine.h"
#include "../../openlcb/protocol_message_network.h"
#include "../../openlcb/protocol_datagram.h"
#include "../../openlcb/protocol_datagram_handlers.h"
#include "../../openlcb/openlcb_application_callbacks.h"

can_main_statemachine_t CanMainStatemachine_can_helper;

void CanMainStatemachine_initialize(
        can_rx_driver_callback_t can_rx_driver_callback,
        transmit_raw_can_frame_func_t transmit_raw_can_frame_callback,
        is_can_tx_buffer_clear_func_t is_can_tx_buffer_clear_callback,
        parameterless_callback_t pause_can_rx_callback,
        parameterless_callback_t resume_can_rx_callback
        ) {

    DriverCan_initialization(transmit_raw_can_frame_callback, is_can_tx_buffer_clear_callback, pause_can_rx_callback, resume_can_rx_callback);

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    CanRxStatemachine_initialize(can_rx_driver_callback);
    CanTxStatemachine_initialize();

    // Just use an existing openlcb_helper structure vs allocating another one
    CanMainStatemachine_can_helper.openlcb_worker = OpenLcbMainStatemachine_get_openlcb_helper();

}

static void _run_can_login_statemachine(openlcb_node_t* openlcb_node, can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    switch (openlcb_node->state.run_state) {

        case RUNSTATE_INIT:

            CanLoginMessageHandler_init(openlcb_node);

            return;

        case RUNSTATE_GENERATE_SEED:

            CanLoginMessageHandler_generate_seed(openlcb_node);

            return;

        case RUNSTATE_GENERATE_ALIAS:

            CanLoginMessageHandler_generate_alias(openlcb_node);

            return;

        case RUNSTATE_SEND_CHECK_ID_07:

            CanLoginMessageHandler_transmit_cid07(openlcb_node, can_msg);

            return;

        case RUNSTATE_SEND_CHECK_ID_06:

            CanLoginMessageHandler_transmit_cid06(openlcb_node, can_msg);

            return;

        case RUNSTATE_SEND_CHECK_ID_05:

            CanLoginMessageHandler_transmit_cid05(openlcb_node, can_msg);

            return;

        case RUNSTATE_SEND_CHECK_ID_04:

            CanLoginMessageHandler_transmit_cid04(openlcb_node, can_msg);

            return;

        case RUNSTATE_WAIT_200ms:

            CanLoginMessageHandler_wait_200ms(openlcb_node);

            return;

        case RUNSTATE_TRANSMIT_RESERVE_ID:

            CanLoginMessageHandler_transmit_rid(openlcb_node, can_msg);

            return;

        case RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION:

            CanLoginMessageHandler_transmit_amd(openlcb_node, can_msg);

            return;

        case RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE:

            CanLoginMessageHandler_transmit_initialization_complete(openlcb_node, openlcb_msg);

            return;

        case RUNSTATE_TRANSMIT_PRODUCER_EVENTS:

            CanLoginMessageHandler_transmit_producer_events(openlcb_node, openlcb_msg);

            return;

        case RUNSTATE_TRANSMIT_CONSUMER_EVENTS:

            CanLoginMessageHandler_transmit_consumer_events(openlcb_node, openlcb_msg);

            return;

        case RUNSTATE_RUN:

            return;

    }

}

bool _pop_next_openlcb_worker_active_message(can_main_statemachine_t* can_helper) {

    if (can_helper->openlcb_worker->active_msg) {

        return false;

    }

    Driver100msClock_pause_100ms_timer();
    DriverCan_pause_can_rx();
    can_helper->openlcb_worker->active_msg = OpenLcbBufferFifo_pop();
    DriverCan_resume_can_rx();
    Driver100msClock_resume_100ms_timer();


    if (can_helper->openlcb_worker->active_msg) {

        parameterless_callback_t rx_callback = OpenLcbApplicationCallbacks_get_can_rx();

        if (rx_callback) {

            rx_callback();

        }

        return true;

    } else {

        return false;

    }

}

static void _reset_message_handled_flags_if_required(openlcb_node_t* next_node, uint8_t newly_popped_openlcb_active_msg) {

    if (newly_popped_openlcb_active_msg) {

        next_node->state.openlcb_datagram_ack_sent = false;
        next_node->state.openlcb_msg_handled = false;

    }

}

static void _free_active_message_buffers_if_processing_complete(can_main_statemachine_t* can_helper, uint8_t active_openlcb_msg_processing_complete) {

    // Are all the nodes finished handling the incoming Openlcb message?
    if (active_openlcb_msg_processing_complete && can_helper->openlcb_worker->active_msg) {

        OpenLcbBufferStore_free_buffer(can_helper->openlcb_worker->active_msg);
        can_helper->openlcb_worker->active_msg = NULL; // Clear the "flag" the next loop _pop_next_openlcb_message() will get a new message)

    }

}

static bool _resend_datagram_message_from_ack_failure_reply(can_main_statemachine_t* can_helper, openlcb_node_t* next_node) {

    if (next_node->state.resend_datagram && next_node->last_received_datagram) {

        next_node->state.openlcb_msg_handled = false;

        OpenLcbMainStatemachine_run_single_node(next_node, next_node->last_received_datagram, &can_helper->openlcb_worker->worker);

        if (next_node->state.openlcb_msg_handled) {

            ProtocolDatagramHandlers_clear_resend_datagram_message(next_node);

            return false;

        }

        return true; // need to retry

    }

    return false;

}

static bool _resend_optional_message_from_oir_reply(can_main_statemachine_t* can_helper, openlcb_node_t* next_node) {

    if (next_node->state.resend_optional_message && next_node->last_received_optional_interaction) {

        next_node->state.openlcb_msg_handled = false;

        OpenLcbMainStatemachine_run_single_node(next_node, next_node->last_received_optional_interaction, &can_helper->openlcb_worker->worker);

        if (next_node->state.openlcb_msg_handled) {

            ProtocolMessageNetwork_clear_resend_optional_message(next_node);

            return false;
        }

        return true; // need to retry

    }

    return false;

}

static void _dispatch_next_openlcb_message_to_node(can_main_statemachine_t* can_helper, openlcb_node_t* next_node, bool* is_active_openlcb_msg_processiong_complete) {

    if (can_helper->openlcb_worker->active_msg) {

        OpenLcbMainStatemachine_run_single_node(next_node, can_helper->openlcb_worker->active_msg, &can_helper->openlcb_worker->worker);

        if (!next_node->state.openlcb_msg_handled)

            *is_active_openlcb_msg_processiong_complete = false;

    }

}

void CanMainStateMachine_run(void) {
    
    
    // CAN Fifo Buffer only contains messages that are ready for sending 
    
    if (DriverCan_is_can_tx_buffer_clear(TX_CHANNEL_CAN_CONTROL)) {
        
        
        can_msg_t *can_msg = CanBufferFifo_pop();
        
        if (can_msg) {
            
            DriverCan_transmit_raw_can_frame(TX_CHANNEL_CAN_CONTROL, can_msg);
            
            return;
             
        }
        
    }

    can_msg_t can_helper;
    
    bool is_newly_popped_openlcb_active_msg = _pop_next_openlcb_worker_active_message(&CanMainStatemachine_can_helper);

    // optimistic from the beginning
    bool is_active_openlcb_msg_processing_complete = true;

    openlcb_node_t* next_node = OpenLcbNode_get_first(0);

    while (next_node) {

        DriverCan_pause_can_rx();
        OpenLcbNode_check_and_handle_duplicate_alias(next_node);
        DriverCan_resume_can_rx();

        _reset_message_handled_flags_if_required(next_node, is_newly_popped_openlcb_active_msg);

        if (next_node->state.run_state == RUNSTATE_RUN) { // process any incoming messages that were popped if the node is initialized

            // these need to get out asap so don't waste time processing normal messages below until we can get these out
            if (_resend_datagram_message_from_ack_failure_reply(&CanMainStatemachine_can_helper, next_node)) {

                break;

            }

            // these need to get out asap so don't waste time processing normal messages below until we can get these out
            if (_resend_optional_message_from_oir_reply(&CanMainStatemachine_can_helper, next_node)) {

                break;

            }

            _dispatch_next_openlcb_message_to_node(&CanMainStatemachine_can_helper, next_node, &is_active_openlcb_msg_processing_complete);

        } else {

            // We don't process any OpenLCB messages since we can't reply until after the node is initialized anyway


            // Process any login states
            _run_can_login_statemachine(next_node, &can_helper, &CanMainStatemachine_can_helper.openlcb_worker->worker);

        }

        next_node = OpenLcbNode_get_next(0);
    }

    _free_active_message_buffers_if_processing_complete(&CanMainStatemachine_can_helper, is_active_openlcb_msg_processing_complete);

}
