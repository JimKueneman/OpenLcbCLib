/*
 * File:   can_main_statemachine.c
 * Author: jimkueneman
 *
 * Created on November 20, 2024, 7:25 PM
 */


#include "xc.h"
#include "stdio.h"  // printf
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_main_statemachine.h"
#include "can_types.h"
#include "can_buffer_fifo.h"
#include "can_frame_message_handler.h"
#include "can_buffer_store.h"
#include "can_login_message_handler.h"
#include "can_tx_statemachine.h"
#include "can_rx_statemachine.h"
#include "../driver_can.h"


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

uint8_t _pop_next_can_message() {

    if (can_helper.active_msg)
        return FALSE;

    DriverCan_pause_can_rx();
    can_helper.active_msg = CanBufferFifo_pop();
    DriverCan_resume_can_rx();

    if (can_helper.active_msg)

        return TRUE

    else

        return FALSE;

}

uint8_t _pop_next_openlcb_message() {

    if (can_helper.openlcb_worker->active_msg)
        return FALSE;

    DriverCan_pause_can_rx();
    can_helper.openlcb_worker->active_msg = BufferFifo_pop();
    DriverCan_resume_can_rx();

    if (can_helper.openlcb_worker->active_msg)

        return TRUE

    else

        return FALSE;

}

void CanMainStateMachine_run() {

    uint8_t reset_can_active_msg = _pop_next_can_message();
    uint8_t reset_openlcb_active_msg = _pop_next_openlcb_message();

    uint8_t active_can_msg_done = TRUE;
    uint8_t active_openlcb_msg_done = TRUE;
    
    
      if (reset_can_active_msg && can_helper.active_msg->state.direct_tx) {
        
        CanFrameMessageHandler_direct_tx(can_helper.active_msg);
        
        if (!can_helper.active_msg->state.direct_tx) {
            
            CanBufferStore_freeBuffer(can_helper.active_msg);
            can_helper.active_msg = (void*) 0;
            
        }
        
      return;   
       
    }

    openlcb_node_t* next_node = Node_get_first(0);

    while (next_node) {

        if (reset_can_active_msg) {
            next_node->state.can_msg_handled = FALSE;
            
            printf("\n");
            printf("IN:\n");
            PrintCanMsg(can_helper.active_msg);
            printf("\n");
            
        }

        if (reset_openlcb_active_msg) {
            
            next_node->state.openlcb_datagram_ack_sent = FALSE;
            next_node->state.openlcb_msg_handled = FALSE;
            
            printf("\n");
            printf("IN:\n");
            PrintOpenLcbMsg(can_helper.openlcb_worker->active_msg);
            printf("\n");
            
        }


        // process any incoming messages that were popped if the node is initialized
        if (next_node->state.initalized) {

            // Process any waiting incoming CAN frame messages
            if (can_helper.active_msg) {

                _run_can_frame_statemachine(next_node, can_helper.active_msg, &can_helper.can_worker);

                if (!next_node->state.can_msg_handled)
                    active_can_msg_done = FALSE;

            }

            // Process any waiting incoming Openlcb messages
            if (can_helper.openlcb_worker->active_msg) {

                MainStatemachine_run_single_node(next_node, can_helper.openlcb_worker->active_msg, &can_helper.openlcb_worker->worker);

                if (!next_node->state.openlcb_msg_handled)
                    active_openlcb_msg_done = FALSE;

            }

        }


        // Process any login states ( )
        if (next_node->state.run_state != RUNSTATE_RUN)
            _run_can_login_statemachine(next_node, &can_helper.can_worker, &can_helper.openlcb_worker->worker);

        next_node = Node_get_next(0);

    }



    // Are all the nodes finished handling the incoming CAN message?
    if (active_can_msg_done) {

        CanBufferStore_freeBuffer(can_helper.active_msg);
        can_helper.active_msg = (void*) 0; // Clear the "flag" the next loop _pop_next_can_message() will get a new message)

    }

    // Are all the nodes finished handling the incoming Openlcb message?
    if (active_openlcb_msg_done) {

        BufferStore_freeBuffer(can_helper.openlcb_worker->active_msg);
        can_helper.openlcb_worker->active_msg = (void*) 0; // Clear the "flag" the next loop _pop_next_openlcb_message() will get a new message)

    }

}