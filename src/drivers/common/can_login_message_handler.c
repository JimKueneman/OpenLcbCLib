/*
 * File:   can_login_message_handler.c
 * Author: jimkueneman
 *
 * Created on November 24, 2024, 6:09 AM
 */

#include "xc.h"
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
#include "../mcu_driver.h"





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

    if (CallbackHooks_alias_change)
        CallbackHooks_alias_change(next_node->alias, next_node->id);

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

    Utilities_copy_node_id_to_openlcb_payload(openlcb_worker, next_node->id);


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
