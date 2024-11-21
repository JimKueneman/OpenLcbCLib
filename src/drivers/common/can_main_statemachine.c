/*
 * File:   can_main_statemachine.c
 * Author: jimkueneman
 *
 * Created on November 20, 2024, 7:25 PM
 */


#include "xc.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_utilities.h"
#include "can_tx_statemachine.h"
#include "can_utilities.h"


openlcb_msg_t openlcb_msg_worker;
payload_basic_t openlcb_basic_worker;
can_msg_t can_msg_worker;

void CanMainStatemachine_initialize() {

    openlcb_msg_worker.payload = (openlcb_payload_t*) &openlcb_basic_worker;
    openlcb_msg_worker.payload_size = LEN_MESSAGE_BYTES_BASIC;

}

void _handle_run_Init(openlcb_node_t* openlcb_node) {

    openlcb_node->seed = openlcb_node->id;
    openlcb_node->alias = Node_generate_alias(openlcb_node->seed);

    openlcb_node->state.run = RUNSTATE_GENERATE_ALIAS; // Jump over Generate Seed that only is if we have an Alias conflict and have to jump back

}

void _handle_run_generate_seed(openlcb_node_t* openlcb_node) {

    openlcb_node->seed = Node_generate_seed(openlcb_node->seed);
    openlcb_node->state.run = RUNSTATE_GENERATE_ALIAS;

}

void _handle_run_generate_alias(openlcb_node_t* openlcb_node) {

    openlcb_node->alias = Node_generate_alias(openlcb_node->seed);

    //   if (AliasChangeCallbackFunc)
    //      AliasChangeCallbackFunc(openlcb_node->alias, openlcb_node->id);

    openlcb_node->state.run = RUNSTATE_SEND_CHECK_ID_07;

}

void _handle_run_transmit_cid07(openlcb_node_t* openlcb_node) {

    can_msg_t can_msg;

    can_msg.payload_count = 0;
    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((openlcb_node->id >> 24) & 0xFFF000) | openlcb_node->alias); // AA0203040506


    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        openlcb_node->state.run = RUNSTATE_SEND_CHECK_ID_06;

}

void _handle_run_transmit_cid06(openlcb_node_t* openlcb_node) {

    can_msg_t can_msg;

    can_msg.payload_count = 0;
    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((openlcb_node->id >> 12) & 0xFFF000) | openlcb_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        openlcb_node->state.run = RUNSTATE_SEND_CHECK_ID_05;

}

void _handle_run_transmit_cid05(openlcb_node_t* openlcb_node) {

    can_msg_t can_msg;

    can_msg.payload_count = 0;
    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((openlcb_node->id & 0xFFF000) | openlcb_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        openlcb_node->state.run = RUNSTATE_SEND_CHECK_ID_04;

}

void _handle_run_transmit_cid04(openlcb_node_t* openlcb_node) {

    can_msg_t can_msg;

    can_msg.payload_count = 0;
    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((openlcb_node->id << 12) & 0xFFF000) | openlcb_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        openlcb_node->state.run = RUNSTATE_WAIT_200ms;

}

void _handle_run_wait_200ms(openlcb_node_t* openlcb_node) {

    if (openlcb_node->timerticks > 3)

        openlcb_node->state.run = RUNSTATE_TRANSMIT_RESERVE_ID;

}

void handle_run_transmit_rid(openlcb_node_t* openlcb_node) {

    can_msg_t can_msg;

    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | openlcb_node->alias;
    can_msg.payload_count = 0;

    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        openlcb_node->state.run = RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION;

}

void _handle_run_transmit_amd(openlcb_node_t* openlcb_node) {

    can_msg_worker.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | openlcb_node->alias;
    CanUtilities_copy_node_id_to_payload(&can_msg_worker, openlcb_node->id, 0);


    if (CanTxStatemachine_try_transmit_can_message(&can_msg_worker)) {

        openlcb_node->state.permitted = TRUE;
        openlcb_node->state.run = RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE;

    }

}

void _handle_run_transmit_initialization_complete(openlcb_node_t* openlcb_node) {

    openlcb_node->state.initalized = TRUE;

    openlcb_msg_worker.dest_alias = 0;
    openlcb_msg_worker.source_alias = openlcb_node->alias;

    // TODO: How to get User defined node info into this file 
    //    if (USER_DEFINED_PROTOCOL_SUPPORT & PSI_SIMPLE)
    //        openlcb_msg_worker.mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;
    //    else
        openlcb_msg_worker.mti = MTI_INITIALIZATION_COMPLETE;

    Utilities_copy_node_id_to_openlcb_payload(&openlcb_msg_worker, openlcb_node->id);

    if (CanTxStatemachine_try_transmit_openlcb_message(&openlcb_msg_worker, &can_msg_worker, 0))
        openlcb_node->state.run = RUNSTATE_TRANSMIT_PRODUCER_EVENTS;



}

uint8_t _load_next_producer_id(openlcb_node_t* openlcb_node) {


    // TODO: Finish
    
//    openlcb_node->dest_alias = 0;
//    openlcb_node->source_alias = openlcb_node->alias;
//    openlcb_node->mti = ExtractProducerEventStateMTI(openlcb_node, openlcb_node->working_msg_index);
//    Utilities_copy_event_id_to_openlcb_payload(openlcb_node, openlcb_node->producers.list[openlcb_node->working_msg_index]);

}

void _handle_run_transmit_producer_events(openlcb_node_t* openlcb_node) {
    
      // TODO: Finish
//
//    if (GetFreeCountOpenLcbFIFO(&outgoing_fifo) >= LEN_FIFO_BUFFER / 2)
//        return;
//
//    if (!openlcb_node->producers.enumerator.running) {
//
//        openlcb_node->producers.enumerator.running = 1;
//        openlcb_node->working_msg_index = 0;
//
//    }
//
//    if (_load_next_producer_id(&openlcb_node_worker))
//        openlcb_node_worker->working_msg_index = openlcb_node_worker->working_msg_index + 1;
//
//
//    if (openlcb_node->working_msg_index == USER_DEFINED_PRODUCER_COUNT) {
//
//        openlcb_node->producers.enumerator.running = 0;
//        openlcb_node->state.run = RUNSTATE_TRANSMIT_CONSUMER_EVENTS;
//
//    }

    openlcb_node->state.run = RUNSTATE_TRANSMIT_CONSUMER_EVENTS;
    
}

uint8_t _load_next_consumer_id(openlcb_node_t* openlcb_node) {
    
      // TODO: Finish

//    openlcb_msg_t* new_msg;
//
//    new_msg = PushOpenLcbMessage(&outgoing_fifo, ID_DATA_SIZE_BASIC);
//
//    if (new_msg) {
//
//        LoadOpenLcbMessage(
//                new_msg,
//                openlcb_node->alias,
//                openlcb_node->id,
//                0,
//                0,
//                ExtractConsumerEventStateMTI(openlcb_node, openlcb_node->working_msg_index),
//                8
//                );
//        Utilities_copy_event_id_to_openlcb_payload(openlcb_msg_worker, openlcb_node->consumers.list[openlcb_node->working_msg_index]);
//
//        return TRUE;
//
//    }
//
//    return FALSE;
}

void _handle_run_transmit_consumer_events(openlcb_node_t * openlcb_node) {
    
      // TODO: Finish

//    if (GetFreeCountOpenLcbFIFO(&outgoing_fifo) >= LEN_FIFO_BUFFER / 2)
//        return;
//
//
//    if (!openlcb_node->consumers.enumerator.running) {
//
//        openlcb_node->consumers.enumerator.running = 1;
//        openlcb_node->working_msg_index = 0;
//
//    }
//
//    if (_load_next_consumer_id(openlcb_node))
//        openlcb_node->working_msg_index = openlcb_node->working_msg_index + 1;
//
//
//    if (openlcb_node->working_msg_index == USER_DEFINED_CONSUMER_COUNT) {
//
//        openlcb_node->consumers.enumerator.running = 0;
//        openlcb_node->state.run = RUNSTATE_RUN;
//
//    }
    
    openlcb_node->state.run = RUNSTATE_RUN;

}

void _handle_run_run(openlcb_node_t * openlcb_node) {


}

void _handle_run_duplicate_id_detected(openlcb_node_t * openlcb_node) {


}

void CanMainStateMachine_run(openlcb_node_t * openlcb_node) {

    switch (openlcb_node->state.run) {

        case RUNSTATE_INIT:
            _handle_run_Init(openlcb_node);
            return;
        case RUNSTATE_GENERATE_SEED:
            _handle_run_generate_seed(openlcb_node);
            return;
        case RUNSTATE_GENERATE_ALIAS:
            _handle_run_generate_alias(openlcb_node);
            return;
        case RUNSTATE_SEND_CHECK_ID_07:
            _handle_run_transmit_cid07(openlcb_node);
            return;
        case RUNSTATE_SEND_CHECK_ID_06:
            _handle_run_transmit_cid06(openlcb_node);
            return;
        case RUNSTATE_SEND_CHECK_ID_05:
            _handle_run_transmit_cid05(openlcb_node);
            return;
        case RUNSTATE_SEND_CHECK_ID_04:
            _handle_run_transmit_cid04(openlcb_node);
            return;
        case RUNSTATE_WAIT_200ms:
            _handle_run_wait_200ms(openlcb_node);
            return;
        case RUNSTATE_TRANSMIT_RESERVE_ID:
            handle_run_transmit_rid(openlcb_node);
            return;
        case RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION:
            _handle_run_transmit_amd(openlcb_node);
            return;
        case RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE:
            _handle_run_transmit_initialization_complete(openlcb_node);
            return;
        case RUNSTATE_TRANSMIT_PRODUCER_EVENTS:
            _handle_run_transmit_producer_events(openlcb_node);
            return;
        case RUNSTATE_TRANSMIT_CONSUMER_EVENTS:
            _handle_run_transmit_consumer_events(openlcb_node);
            return;
        case RUNSTATE_RUN:
            _handle_run_run(openlcb_node);
            return;
        case RUNSTATE_DUPLICATE_NODEID:
            _handle_run_duplicate_id_detected(openlcb_node);
            return;

    }

}
