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

#include "stdio.h"  // printf
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_main_statemachine.h"
#include "../../openlcb/openlcb_tx_driver.h"
#include "../../openlcb/protocol_message_network.h"
#include "../../openlcb/protocol_datagram.h"
#include "can_types.h"
#include "can_buffer_fifo.h"
#include "can_frame_message_handler.h"
#include "can_buffer_store.h"
#include "can_login_message_handler.h"
#include "can_tx_statemachine.h"
#include "can_rx_statemachine.h"
#include "../driver_can.h"
#include "../driver_100ms_clock.h"


can_main_statemachine_t can_helper;

void CanMainStatemachine_initialize() {

    CanBufferStore_initialize();
    CanBufferFifo_initialiaze();
    CanRxStatemachine_initialize();
    CanTxStatemachine_initialize();

    can_helper.openlcb_worker = &openlcb_helper;
    can_helper.active_msg = (void*) 0;
    can_helper.can_worker.state.allocated = TRUE;
    can_helper.can_worker.state.direct_tx = FALSE;
    can_helper.can_worker.identifier = 0x0000000000;
    can_helper.can_worker.payload_count = 0;
    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        can_helper.can_worker.payload[i] = 0x00;

}

void _run_can_frame_statemachine(openlcb_node_t* openlcb_node, can_msg_t* can_msg, can_msg_t* worker_msg) {


    if (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

        switch (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

            case CAN_CONTROL_FRAME_CID7:
            case CAN_CONTROL_FRAME_CID6:
            case CAN_CONTROL_FRAME_CID5:
            case CAN_CONTROL_FRAME_CID4:

                CanFrameMessageHandler_cid(openlcb_node, can_msg, worker_msg);

                return;

            case CAN_CONTROL_FRAME_CID3:
            case CAN_CONTROL_FRAME_CID2:
            case CAN_CONTROL_FRAME_CID1:

                openlcb_node->state.can_msg_handled = TRUE;

                return;

            default:

                openlcb_node->state.can_msg_handled = TRUE;

                return;

        }

    } else {

        switch (can_msg->identifier & MASK_CAN_VARIABLE_FIELD) {

            case CAN_CONTROL_FRAME_RID: // Reserve ID

                CanFrameMessageHandler_rid(openlcb_node, can_msg, worker_msg);

                return;

            case CAN_CONTROL_FRAME_AMD: // Alias Map Definition

                CanFrameMessageHandler_amd(openlcb_node, can_msg, worker_msg);

                return;

            case CAN_CONTROL_FRAME_AME:

                CanFrameMessageHandler_ame(openlcb_node, can_msg, worker_msg);

                return;

            case CAN_CONTROL_FRAME_AMR:

                CanFrameMessageHandler_amr(openlcb_node, can_msg, worker_msg);

                return;

            case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0:
            case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1:
            case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2:
            case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3:
                // Advanced feature for gateways/routers/etc.

                openlcb_node->state.can_msg_handled = TRUE;

                return;

            default:

                openlcb_node->state.can_msg_handled = TRUE;

                return;

        }

    }


}

void _run_can_login_statemachine(openlcb_node_t* openlcb_node, can_msg_t* can_msg, openlcb_msg_t* openlcb_msg) {

    switch (openlcb_node->state.run_state) {

        case RUNSTATE_INIT:
            CanLoginMessageHandler_init(openlcb_node);
            return;
        case RUNSTATE_GENERATE_SEED:
            CanFrameMessageHandler_generate_seed(openlcb_node);
            return;
        case RUNSTATE_GENERATE_ALIAS:
            CanFrameMessageHandler_generate_alias(openlcb_node);
            return;
        case RUNSTATE_SEND_CHECK_ID_07:
            CanFrameMessageHandler_transmit_cid07(openlcb_node, can_msg);
            return;
        case RUNSTATE_SEND_CHECK_ID_06:
            CanFrameMessageHandler_transmit_cid06(openlcb_node, can_msg);
            return;
        case RUNSTATE_SEND_CHECK_ID_05:
            CanFrameMessageHandler_transmit_cid05(openlcb_node, can_msg);
            return;
        case RUNSTATE_SEND_CHECK_ID_04:
            CanFrameMessageHandler_transmit_cid04(openlcb_node, can_msg);
            return;
        case RUNSTATE_WAIT_200ms:
            CanFrameMessageHandler_wait_200ms(openlcb_node);
            return;
        case RUNSTATE_TRANSMIT_RESERVE_ID:
            CanFrameMessageHandler_transmit_rid(openlcb_node, can_msg);
            return;
        case RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION:
            CanFrameMessageHandler_transmit_amd(openlcb_node, can_msg);
            return;
        case RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE:
            CanFrameMessageHandler_transmit_initialization_complete(openlcb_node, can_msg, openlcb_msg);
            return;
        case RUNSTATE_TRANSMIT_PRODUCER_EVENTS:
            CanFrameMessageHandler_transmit_producer_events(openlcb_node, can_msg, openlcb_msg);
            return;
        case RUNSTATE_TRANSMIT_CONSUMER_EVENTS:
            CanFrameMessageHandler_transmit_consumer_events(openlcb_node, can_msg, openlcb_msg);
            return;
        case RUNSTATE_RUN:

            return;

    }

}

uint8_t _pop_next_can_helper_active_message() {

    if (can_helper.active_msg)
        return FALSE;

    Driver100msClock_pause_100ms_timer();
    DriverCan_pause_can_rx();
    can_helper.active_msg = CanBufferFifo_pop();
    DriverCan_resume_can_rx();
    Driver100msClock_resume_100ms_timer();

    if (can_helper.active_msg)

        return TRUE

    else

        return FALSE;

}

uint8_t _pop_next_openlcb_worker_active_message() {

    if (can_helper.openlcb_worker->active_msg)
        return FALSE;

    Driver100msClock_pause_100ms_timer();
    DriverCan_pause_can_rx();
    can_helper.openlcb_worker->active_msg = BufferFifo_pop();
    DriverCan_resume_can_rx();
    Driver100msClock_resume_100ms_timer();


    if (can_helper.openlcb_worker->active_msg)

        return TRUE

    else

        return FALSE;

}

uint8_t is_direct_tx_message(uint8_t is_new_popped_message) {

    // Direct Tx messages are added in the CAN Rx statemachine but actually replies
    // that have already been dealt with, such as error messages created in response 
    // out of order frames, etc so they just need to be sent out and not dispatched 
    // to the nodes.

    if (is_new_popped_message && can_helper.active_msg->state.direct_tx) {


        if (CanTxStatemachine_try_transmit_can_message(can_helper.active_msg)) {

            can_helper.active_msg->state.direct_tx = FALSE;
            CanBufferStore_freeBuffer(can_helper.active_msg);
            can_helper.active_msg = (void*) 0;

            return TRUE;

        }

        return TRUE; // still trying but did not make it yet.

    }

    return FALSE;

}

void _reset_message_handled_flags_if_required(openlcb_node_t* next_node, uint8_t newly_popped_can_active_msg, uint8_t newly_popped_openlcb_active_msg) {

    if (newly_popped_can_active_msg) {
        next_node->state.can_msg_handled = FALSE;

    }

    if (newly_popped_openlcb_active_msg) {

        next_node->state.openlcb_datagram_ack_sent = FALSE;
        next_node->state.openlcb_msg_handled = FALSE;

    }

}

void _reset_active_message_buffers_if_done(uint8_t active_can_msg_processiong_complete, uint8_t active_openlcb_msg_processing_complete) {

    // Are all the nodes finished handling the incoming CAN message?
    if (active_can_msg_processiong_complete) {

        CanBufferStore_freeBuffer(can_helper.active_msg);
        can_helper.active_msg = (void*) 0; // Clear the "flag" the next loop _pop_next_can_message() will get a new message)

    }

    // Are all the nodes finished handling the incoming Openlcb message?
    if (active_openlcb_msg_processing_complete) {

        BufferStore_freeBuffer(can_helper.openlcb_worker->active_msg);
        can_helper.openlcb_worker->active_msg = (void*) 0; // Clear the "flag" the next loop _pop_next_openlcb_message() will get a new message)

    }

}

void _process_login_statemachine(openlcb_node_t* next_node) {

    // Process any login states ( )
    if (next_node->state.run_state != RUNSTATE_RUN)
        _run_can_login_statemachine(next_node, &can_helper.can_worker, &can_helper.openlcb_worker->worker);

}

uint8_t _resend_datagram_message_from_ack_failure_reply(openlcb_node_t* next_node) {

    if (next_node->state.resend_datagram && next_node->last_received_datagram) {

        next_node->state.openlcb_msg_handled = FALSE;

        MainStatemachine_run_single_node(next_node, next_node->last_received_datagram, &can_helper.openlcb_worker->worker);

        if (next_node->state.openlcb_msg_handled) {

            ProtocolDatagram_clear_resend_datagram_message(next_node);

            return FALSE

        }

        return TRUE; // need to retry

    }

    return FALSE;

}

uint8_t _resend_optional_message_from_oir_reply(openlcb_node_t* next_node) {

    if (next_node->state.resend_optional_message && next_node->last_received_optional_interaction) {

        next_node->state.openlcb_msg_handled = FALSE;

        MainStatemachine_run_single_node(next_node, next_node->last_received_optional_interaction, &can_helper.openlcb_worker->worker);

        if (next_node->state.openlcb_msg_handled) {

            ProtocolMessageNetwork_clear_resend_optional_message(next_node);

            return FALSE

        }

        return TRUE; // need to retry

    }

    return FALSE;

}

void _dispatch_next_can_message_to_node(openlcb_node_t* next_node, uint8_t* is_active_can_msg_processiong_complete) {

    if (can_helper.active_msg) {

        _run_can_frame_statemachine(next_node, can_helper.active_msg, &can_helper.can_worker);

        if (!next_node->state.can_msg_handled)

            *is_active_can_msg_processiong_complete = FALSE;

    }

}

void _dispatch_next_openlcb_message_to_node(openlcb_node_t* next_node, uint8_t* is_active_openlcb_msg_processiong_complete) {

    if (can_helper.openlcb_worker->active_msg) {

        MainStatemachine_run_single_node(next_node, can_helper.openlcb_worker->active_msg, &can_helper.openlcb_worker->worker);

        if (!next_node->state.openlcb_msg_handled)
            
            *is_active_openlcb_msg_processiong_complete = FALSE;

    }

}

void CanMainStateMachine_run() {


    //    probably should have a separate loop to run the resends.....
    //    the can get the buffer handled flags screwed up....        
    //            
    uint8_t is_newly_popped_can_active_msg = _pop_next_can_helper_active_message();
    uint8_t is_newly_popped_openlcb_active_msg = _pop_next_openlcb_worker_active_message();

    // optimistic from the beginning
    uint8_t is_active_can_msg_processiong_complete = TRUE;
    uint8_t is_active_openlcb_msg_processing_complete = TRUE;

    // handle the can message if it is a direct send (there is no node specific processing on it to do, it just needs to get sent)
    if (is_direct_tx_message(is_newly_popped_can_active_msg))
        return;



    openlcb_node_t* next_node = Node_get_first(0);

    while (next_node) {

        _reset_message_handled_flags_if_required(next_node, is_newly_popped_can_active_msg, is_newly_popped_openlcb_active_msg);


        if (next_node->state.initalized) { // process any incoming messages that were popped if the node is initialized

            // these need to get out asap so don't waste time processing normal messages below until we can get these out
            if (_resend_datagram_message_from_ack_failure_reply(next_node))

                break;

            // these need to get out asap so don't waste time processing normal messages below until we can get these out
            if (_resend_optional_message_from_oir_reply(next_node))

                break;

             _dispatch_next_can_message_to_node(next_node, &is_active_can_msg_processiong_complete);

             _dispatch_next_openlcb_message_to_node(next_node, &is_active_openlcb_msg_processing_complete);

        }

        _process_login_statemachine(next_node);

        next_node = Node_get_next(0);

    }

    _reset_active_message_buffers_if_done(is_active_can_msg_processiong_complete, is_active_openlcb_msg_processing_complete);

}