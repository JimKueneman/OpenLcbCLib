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

#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_main_statemachine.h"
#include "../../openlcb/openlcb_tx_driver.h"
#include "../../openlcb/protocol_message_network.h"
#include "../../openlcb/protocol_datagram.h"
#include "../../openlcb/protocol_datagram_handlers.h"
#include "../../openlcb/openlcb_application_callbacks.h"
#include "can_types.h"
#include "can_buffer_fifo.h"
#include "can_frame_message_handler.h"
#include "can_buffer_store.h"
#include "can_login_message_handler.h"
#include "can_tx_statemachine.h"
#include "can_rx_statemachine.h"
#include "../driver_can.h"
#include "../driver_100ms_clock.h"


can_main_statemachine_t _can_helper;

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
    _can_helper.openlcb_worker = OpenLcbMainStatemachine_get_openlcb_helper();
    _can_helper.active_msg = NULL;
    _can_helper.can_worker.state.allocated = true;
    _can_helper.can_worker.state.addressed_direct_tx = false;
    _can_helper.can_worker.identifier = 0x0000000000;
    _can_helper.can_worker.payload_count = 0;

    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++) {

        _can_helper.can_worker.payload[i] = 0x00;

    }

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

                openlcb_node->state.can_msg_handled = true;

                return;

            default:

                openlcb_node->state.can_msg_handled = true;

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

                openlcb_node->state.can_msg_handled = true;

                return;

            default:

                openlcb_node->state.can_msg_handled = true;

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

bool _pop_next_can_helper_active_message(can_main_statemachine_t* can_helper) {

    if (can_helper->active_msg) {

        return false;

    }

    Driver100msClock_pause_100ms_timer();
    DriverCan_pause_can_rx();
    can_helper->active_msg = CanBufferFifo_pop();
    DriverCan_resume_can_rx();
    Driver100msClock_resume_100ms_timer();

    if (can_helper->active_msg) {

        parameterless_callback_t rx_callback = OpenLcbApplicationCallbacks_get_can_rx();

        if (rx_callback) {

            rx_callback();

        }

        return true;

    } else {

        return false;

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

void _release_direct_tx_can_message(can_main_statemachine_t* can_helper) {

    can_helper->active_msg->state.addressed_direct_tx = false;
    CanBufferStore_free_buffer(can_helper->active_msg);
    can_helper->active_msg = NULL;

}

bool _try_transmit_addressed_direct_tx_can_message(can_main_statemachine_t* can_helper) {

    // Addressed Direct Tx messages are added by the CAN Rx statemachine , such as error messages created in response 
    // to out of order frames, etc so they just need to be sent out, if they were actually for a node alias
    // that is active, and not dispatched to the nodes.  These errors be generated for any multi-frame message that 
    // ends up have issues since the CAN Rx engine is running in the context of the interrupt we don't reach into the
    // node list from there.  We just receive all messages and wait until we get into the main loop state-machine to sort 
    // it all out.

    if (!can_helper->active_msg) {

        return false;

    }

    if (can_helper->active_msg->state.addressed_direct_tx) {

        openlcb_node_t* next_node = OpenLcbNode_get_first(0);

        while (next_node) {

            if (next_node->state.run_state == RUNSTATE_RUN) {

                if (can_helper->active_msg->identifier && 0xFFF == next_node->alias) {

                    if (CanTxStatemachine_try_transmit_can_message(can_helper->active_msg)) {

                        _release_direct_tx_can_message(can_helper);

                        return true;

                    }

                    return true; // still trying but did not make it yet.

                }

            }

            next_node = OpenLcbNode_get_next(0);

        }

        // Was not for us
        _release_direct_tx_can_message(can_helper);

        return true;

    }

    return false;

}

void _reset_message_handled_flags_if_required(openlcb_node_t* next_node, uint8_t newly_popped_can_active_msg, uint8_t newly_popped_openlcb_active_msg) {

    if (newly_popped_can_active_msg) {

        next_node->state.can_msg_handled = false;

    }

    if (newly_popped_openlcb_active_msg) {

        next_node->state.openlcb_datagram_ack_sent = false;
        next_node->state.openlcb_msg_handled = false;

    }

}

void _free_active_message_buffers_if_processing_complete(can_main_statemachine_t* can_helper, uint8_t active_can_msg_processiong_complete, uint8_t active_openlcb_msg_processing_complete) {

    // Are all the nodes finished handling the incoming CAN message?
    if (active_can_msg_processiong_complete && can_helper->active_msg) {

        CanBufferStore_free_buffer(can_helper->active_msg);
        can_helper->active_msg = NULL; // Clear the "flag" the next loop _pop_next_can_message() will get a new message)

    }

    // Are all the nodes finished handling the incoming Openlcb message?
    if (active_openlcb_msg_processing_complete && can_helper->openlcb_worker->active_msg) {

        OpenLcbBufferStore_free_buffer(can_helper->openlcb_worker->active_msg);
        can_helper->openlcb_worker->active_msg = NULL; // Clear the "flag" the next loop _pop_next_openlcb_message() will get a new message)

    }

}

bool _resend_datagram_message_from_ack_failure_reply(can_main_statemachine_t* can_helper, openlcb_node_t* next_node) {

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

bool _resend_optional_message_from_oir_reply(can_main_statemachine_t* can_helper, openlcb_node_t* next_node) {

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

void _dispatch_next_can_message_to_node(can_main_statemachine_t* can_helper, openlcb_node_t* next_node, uint8_t* is_active_can_msg_processiong_complete) {

    if (can_helper->active_msg) {

        _run_can_frame_statemachine(next_node, can_helper->active_msg, &can_helper->can_worker);

        if (!next_node->state.can_msg_handled) {

            *is_active_can_msg_processiong_complete = false;

        }

    }

}

void _dispatch_next_openlcb_message_to_node(can_main_statemachine_t* can_helper, openlcb_node_t* next_node, uint8_t* is_active_openlcb_msg_processiong_complete) {

    if (can_helper->openlcb_worker->active_msg) {

        OpenLcbMainStatemachine_run_single_node(next_node, can_helper->openlcb_worker->active_msg, &can_helper->openlcb_worker->worker);

        if (!next_node->state.openlcb_msg_handled)

            *is_active_openlcb_msg_processiong_complete = false;

    }

}

void CanMainStateMachine_run(void) {


    //    probably should have a separate loop to run the resends.....
    //    the can get the buffer handled flags screwed up....        
    //            
    bool is_newly_popped_can_active_msg = _pop_next_can_helper_active_message(&_can_helper);
    bool is_newly_popped_openlcb_active_msg = _pop_next_openlcb_worker_active_message(&_can_helper);

    // optimistic from the beginning
    bool is_active_can_msg_processiong_complete = true;
    bool is_active_openlcb_msg_processing_complete = true;


    if (_try_transmit_addressed_direct_tx_can_message(&_can_helper)) {

        return;

    }

    openlcb_node_t* next_node = OpenLcbNode_get_first(0);

    while (next_node) {

        _reset_message_handled_flags_if_required(next_node, is_newly_popped_can_active_msg, is_newly_popped_openlcb_active_msg);

        if (next_node->state.run_state == RUNSTATE_RUN) { // process any incoming messages that were popped if the node is initialized

            // these need to get out asap so don't waste time processing normal messages below until we can get these out
            if (_resend_datagram_message_from_ack_failure_reply(&_can_helper, next_node)) {

                break;

            }

            // these need to get out asap so don't waste time processing normal messages below until we can get these out
            if (_resend_optional_message_from_oir_reply(&_can_helper, next_node)) {

                break;

            }

            _dispatch_next_can_message_to_node(&_can_helper, next_node, &is_active_can_msg_processiong_complete);

            _dispatch_next_openlcb_message_to_node(&_can_helper, next_node, &is_active_openlcb_msg_processing_complete);

        } else {

            // We don't process any OpenLCB messages since we can't reply until after the node is initialized anyway

            _dispatch_next_can_message_to_node(&_can_helper, next_node, &is_active_can_msg_processiong_complete);

            // Process any login states
            _run_can_login_statemachine(next_node, &_can_helper.can_worker, &_can_helper.openlcb_worker->worker);

        }

        next_node = OpenLcbNode_get_next(0);
    }

    _free_active_message_buffers_if_processing_complete(&_can_helper, is_active_can_msg_processiong_complete, is_active_openlcb_msg_processing_complete);

}
