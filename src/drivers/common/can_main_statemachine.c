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
#include "../../openlcb/openlcb_main_statemachine.h"
#include "../../openlcb/callback_hooks.h"
#include "../../openlcb/protocol_event_transport.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "can_tx_statemachine.h"
#include "can_utilities.h"
#include "can_buffer_fifo.h"
#include "can_msg_handler.h"

typedef struct {
    openlcb_msg_t openlcb_msg_worker;
    payload_basic_t openlcb_basic_worker;
    can_msg_t can_msg_worker;
    can_msg_t* active_msg;

} can_main_statemachine_t;


can_main_statemachine_t can_helper;

void CanMainStatemachine_initialize() {

    can_helper.openlcb_msg_worker.payload = (openlcb_payload_t*) & can_helper.openlcb_basic_worker;
    can_helper.openlcb_msg_worker.payload_size = LEN_MESSAGE_BYTES_BASIC;
    can_helper.active_msg = (void*) 0;

}

void _handle_run_Init(openlcb_node_t* next_node) {

    next_node->seed = next_node->id;
    next_node->alias = Node_generate_alias(next_node->seed);

    next_node->state.run_state = RUNSTATE_GENERATE_ALIAS; // Jump over Generate Seed that only is if we have an Alias conflict and have to jump back

}

void _handle_run_generate_seed(openlcb_node_t* next_node) {

    next_node->seed = Node_generate_seed(next_node->seed);
    next_node->state.run_state = RUNSTATE_GENERATE_ALIAS;

}

void _handle_run_generate_alias(openlcb_node_t* next_node) {

    next_node->alias = Node_generate_alias(next_node->seed);

    if (CallbackHooks_alias_change)
        CallbackHooks_alias_change(next_node->alias, next_node->id);

    next_node->state.run_state = RUNSTATE_SEND_CHECK_ID_07;

}

void _handle_run_transmit_cid07(openlcb_node_t* next_node) {

    can_msg_t can_msg;

    can_msg.payload_count = 0;
    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((next_node->id >> 24) & 0xFFF000) | next_node->alias); // AA0203040506


    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        next_node->state.run_state = RUNSTATE_SEND_CHECK_ID_06;

}

void _handle_run_transmit_cid06(openlcb_node_t* next_node) {

    can_msg_t can_msg;

    can_msg.payload_count = 0;
    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((next_node->id >> 12) & 0xFFF000) | next_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        next_node->state.run_state = RUNSTATE_SEND_CHECK_ID_05;

}

void _handle_run_transmit_cid05(openlcb_node_t* next_node) {

    can_msg_t can_msg;

    can_msg.payload_count = 0;
    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((next_node->id & 0xFFF000) | next_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        next_node->state.run_state = RUNSTATE_SEND_CHECK_ID_04;

}

void _handle_run_transmit_cid04(openlcb_node_t* next_node) {

    can_msg_t can_msg;

    can_msg.payload_count = 0;
    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((next_node->id << 12) & 0xFFF000) | next_node->alias);

    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        next_node->state.run_state = RUNSTATE_WAIT_200ms;

}

void _handle_run_wait_200ms(openlcb_node_t* next_node) {

    if (next_node->timerticks > 3)
        next_node->state.run_state = RUNSTATE_TRANSMIT_RESERVE_ID;

}

void handle_run_transmit_rid(openlcb_node_t* next_node) {

    can_msg_t can_msg;

    can_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | next_node->alias;
    can_msg.payload_count = 0;

    if (CanTxStatemachine_try_transmit_can_message(&can_msg))
        next_node->state.run_state = RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION;

}

void _handle_run_transmit_amd(openlcb_node_t* next_node) {

    can_helper.can_msg_worker.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | next_node->alias;
    CanUtilities_copy_node_id_to_payload(&can_helper.can_msg_worker, next_node->id, 0);


    if (CanTxStatemachine_try_transmit_can_message(&can_helper.can_msg_worker)) {

        next_node->state.permitted = TRUE;
        next_node->state.run_state = RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE;

    }

}

void _handle_run_transmit_initialization_complete(openlcb_node_t* next_node) {

    next_node->state.initalized = TRUE;

    BufferStore_clear_openlcb_message(&can_helper.openlcb_msg_worker);
    
    can_helper.openlcb_msg_worker.source_id = next_node->id;
    can_helper.openlcb_msg_worker.source_alias = next_node->alias;

    if (next_node->parameters->protocol_support & PSI_SIMPLE)
        can_helper.openlcb_msg_worker.mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;
    else
        can_helper.openlcb_msg_worker.mti = MTI_INITIALIZATION_COMPLETE;

    Utilities_copy_node_id_to_openlcb_payload(&can_helper.openlcb_msg_worker, next_node->id);
    

    if (CanTxStatemachine_try_transmit_openlcb_message(&can_helper.openlcb_msg_worker, &can_helper.can_msg_worker, 0)) {

        next_node->producers.enumerator.running = TRUE;
        next_node->producers.enumerator.enum_index = 0;
        next_node->state.run_state = RUNSTATE_TRANSMIT_PRODUCER_EVENTS;

    }
    
}

void _handle_run_transmit_producer_events(openlcb_node_t* next_node) {


    if (next_node->producers.enumerator.running) {

        if (next_node->producers.enumerator.enum_index < next_node->parameters->producer_count) {

            BufferStore_clear_openlcb_message(&can_helper.openlcb_msg_worker);
            
            can_helper.openlcb_msg_worker.source_alias = next_node->alias;
            can_helper.openlcb_msg_worker.source_id = next_node->id;
            can_helper.openlcb_msg_worker.mti = ProtocolEventTransport_extract_producer_event_state_mti(next_node, next_node->producers.enumerator.enum_index);
            Utilities_copy_event_id_to_openlcb_payload(&can_helper.openlcb_msg_worker, next_node->producers.list[next_node->producers.enumerator.enum_index]);

            
            if (CanTxStatemachine_try_transmit_openlcb_message(&can_helper.openlcb_msg_worker, &can_helper.can_msg_worker, 0)) {

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


void _handle_run_transmit_consumer_events(openlcb_node_t* next_node) {

    if (next_node->consumers.enumerator.running) {

        if (next_node->consumers.enumerator.enum_index < next_node->parameters->producer_count) {

            BufferStore_clear_openlcb_message(&can_helper.openlcb_msg_worker);
            
            can_helper.openlcb_msg_worker.source_alias = next_node->alias;
            can_helper.openlcb_msg_worker.source_id = next_node->id;
            can_helper.openlcb_msg_worker.mti = ProtocolEventTransport_extract_consumer_event_state_mti(next_node, next_node->consumers.enumerator.enum_index);
            Utilities_copy_event_id_to_openlcb_payload(&can_helper.openlcb_msg_worker, next_node->consumers.list[next_node->consumers.enumerator.enum_index]);

            
            if (CanTxStatemachine_try_transmit_openlcb_message(&can_helper.openlcb_msg_worker, &can_helper.can_msg_worker, 0)) {

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

void _handle_run_run(openlcb_node_t* next_node) {

    // Pop CAN messages and start dispatching....

    if (!can_helper.active_msg)
        can_helper.active_msg = CanBufferFifo_pop();

    if (can_helper.active_msg) {

        if (!next_node->state.can_msg_handled)
            CanMessageHandler_process(next_node, can_helper.active_msg);

    }

    MainStatemachine_run_with_node(next_node);

}

void _handle_run_duplicate_id_detected(openlcb_node_t * next_node) {


}

void CanMainStateMachine_run() {

    openlcb_node_t* next_node = Node_get_first();

    while (next_node) {

        _RB4 = !_RB4;

        switch (next_node->state.run_state) {

            case RUNSTATE_INIT:
                _handle_run_Init(next_node);
                return;
            case RUNSTATE_GENERATE_SEED:
                _handle_run_generate_seed(next_node);
                return;
            case RUNSTATE_GENERATE_ALIAS:
                _handle_run_generate_alias(next_node);
                return;
            case RUNSTATE_SEND_CHECK_ID_07:
                _handle_run_transmit_cid07(next_node);
                return;
            case RUNSTATE_SEND_CHECK_ID_06:
                _handle_run_transmit_cid06(next_node);
                return;
            case RUNSTATE_SEND_CHECK_ID_05:
                _handle_run_transmit_cid05(next_node);
                return;
            case RUNSTATE_SEND_CHECK_ID_04:
                _handle_run_transmit_cid04(next_node);
                return;
            case RUNSTATE_WAIT_200ms:
                _handle_run_wait_200ms(next_node);
                return;
            case RUNSTATE_TRANSMIT_RESERVE_ID:
                handle_run_transmit_rid(next_node);
                return;
            case RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION:
                _handle_run_transmit_amd(next_node);
                return;
            case RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE:
                _handle_run_transmit_initialization_complete(next_node);
                return;
            case RUNSTATE_TRANSMIT_PRODUCER_EVENTS:
                _handle_run_transmit_producer_events(next_node);
                return;
            case RUNSTATE_TRANSMIT_CONSUMER_EVENTS:
                _handle_run_transmit_consumer_events(next_node);
                return;
            case RUNSTATE_RUN:
                _handle_run_run(next_node);
                return;
            case RUNSTATE_DUPLICATE_NODEID:
                _handle_run_duplicate_id_detected(next_node);
                return;

                next_node = Node_get_next();

        }

    }

}
