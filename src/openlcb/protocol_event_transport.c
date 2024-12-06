/*
 * File:   protocol_event_transport.c
 * Author: jimkueneman
 *
 * Created on November 14, 2024, 5:24 AM
 */

/*
 * NOTE:  All Function for all Protocols expect that the incoming CAN messages have been
 *        blocked so there is not a race on the incoming message buffer.
 */


#include "stdio.h" // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_tx_driver.h"
#include "protocol_event_transport.h"

typedef enum {
    EVENT_STATE_UNKNOWN,
    EVENT_STATE_SET,
    EVENT_STATE_CLEAR

} event_enum_state_t;

void _encode_event_state(uint64_t* state_container, uint8_t event_index, event_enum_state_t new_state) {

    uint64_t mask;

    switch (new_state) {

        case EVENT_STATE_UNKNOWN:
            mask = 0b11; // Set bit is what will be cleared gives 0b00)
            break;
        case EVENT_STATE_SET: // Set bit is what will be cleared gives 0b01)
            mask = 0b10;
            break;
        case EVENT_STATE_CLEAR: // Set bit is what will be cleared gives 0b10)
            mask = 0b01;
            break;
    }

    *state_container = *state_container & ~(mask << (event_index * 2));

}

void _encode_consumer_event_state(openlcb_node_t* openlcb_node, uint8_t event_index, event_enum_state_t new_state) {

    _encode_event_state(&openlcb_node->consumers.event_state, event_index, new_state);

}

void _encode_producer_event_state(openlcb_node_t* openlcb_node, uint8_t event_index, event_enum_state_t new_state) {

    _encode_event_state(&openlcb_node->producers.event_state, event_index, new_state);

}

event_enum_state_t _decode_event_state(uint64_t* state_container, uint8_t event_index) {

    switch ((*state_container >> (2 * event_index)) & 0x3) {
        
        case 0x00: return EVENT_STATE_UNKNOWN;
        
        case 0x01: return EVENT_STATE_SET;
        
        case 0x02: return EVENT_STATE_CLEAR;
        
        default:
            
            return EVENT_STATE_UNKNOWN;

    }

}

void _identify_producers(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    if (!openlcb_node->producers.enumerator.running) 

        openlcb_node->producers.enumerator.running = TRUE; // Kick off the enumeration next loop

    
    if (openlcb_node->producers.enumerator.enum_index >= openlcb_node->parameters->producer_count) {
        
        openlcb_node->producers.enumerator.running = FALSE; 
        
        return;
        
    }
   
    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            ProtocolEventTransport_extract_producer_event_state_mti(openlcb_node, openlcb_node->producers.enumerator.enum_index),
            8
            );

    Utilities_copy_event_id_to_openlcb_payload(worker_msg, openlcb_node->producers.list[openlcb_node->producers.enumerator.enum_index]);

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) 
        
        openlcb_node->producers.enumerator.enum_index = openlcb_node->producers.enumerator.enum_index + 1;

}

void _identify_consumers(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;
 
    if (!openlcb_node->consumers.enumerator.running) 

        openlcb_node->consumers.enumerator.running = TRUE; // Kick off the enumeration next loop

    
    if (openlcb_node->consumers.enumerator.enum_index >= openlcb_node->parameters->consumer_count) {
        
        openlcb_node->consumers.enumerator.running = FALSE; 
        
        return;
        
    }

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            ProtocolEventTransport_extract_consumer_event_state_mti(openlcb_node, openlcb_node->consumers.enumerator.enum_index),
            8
            );

    Utilities_copy_event_id_to_openlcb_payload(worker_msg, openlcb_node->consumers.list[openlcb_node->consumers.enumerator.enum_index]);

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) 
        
        openlcb_node->consumers.enumerator.enum_index = openlcb_node->consumers.enumerator.enum_index + 1;

}

uint16_t ProtocolEventTransport_extract_consumer_event_state_mti(openlcb_node_t* openlcb_node, uint8_t event_index) {

    switch (_decode_event_state(&openlcb_node->consumers.event_state, event_index)) {

        case EVENT_STATE_UNKNOWN:
            
            return MTI_CONSUMER_IDENTIFIED_UNKNOWN;
            
        case EVENT_STATE_SET:
            
            return MTI_CONSUMER_IDENTIFIED_SET;
            
        case EVENT_STATE_CLEAR:
            
            return MTI_CONSUMER_IDENTIFIED_CLEAR;
            
        default:
            
            return MTI_CONSUMER_IDENTIFIED_UNKNOWN;

    }

}

uint16_t ProtocolEventTransport_extract_producer_event_state_mti(openlcb_node_t* openlcb_node, uint8_t event_index) {

    switch (_decode_event_state(&openlcb_node->producers.event_state, event_index)) {

        case 0x00:
            
            return MTI_PRODUCER_IDENTIFIED_UNKNOWN;
            
        case 0x01:
            
            return MTI_PRODUCER_IDENTIFIED_SET;
            
        case 0x02:
            
            return MTI_PRODUCER_IDENTIFIED_CLEAR;
            
        default:
            
            return MTI_PRODUCER_IDENTIFIED_UNKNOWN;

    }

}

void ProtocolEventTransport_handle_consumer_identify(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (openlcb_node->state.openlcb_msg_handled)
        return;
    
    int event_index = Utilities_is_consumer_event_assigned_to_node(openlcb_node, Utilities_extract_event_id_from_openlcb_payload(openlcb_msg));
       
    if (event_index < 0) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

        return;

    }
 
    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            ProtocolEventTransport_extract_consumer_event_state_mti(openlcb_node, event_index),
            8
            );

    Utilities_copy_event_id_to_openlcb_payload(worker_msg, openlcb_node->consumers.list[event_index]);

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void ProtocolEventTransport_handle_consumer_identify_range(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("consumer range\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("consumer unknown\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_consumer_identified_set(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("consumer set\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_consumer_identified_clear(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("consumer clear\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("consumer reserved\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_producer_identify(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    int event_index = Utilities_is_producer_event_assigned_to_node(openlcb_node, Utilities_extract_event_id_from_openlcb_payload(openlcb_msg));

    if (event_index < 0) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

        return;

    }

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            ProtocolEventTransport_extract_producer_event_state_mti(openlcb_node, event_index),
            8
            );

    Utilities_copy_event_id_to_openlcb_payload(worker_msg, openlcb_node->producers.list[event_index]);

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void ProtocolEventTransport_handle_producer_identify_range(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("producer range\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_producer_identified_unknown(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("producer unknown\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_producer_identified_set(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("producer set\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_producer_identified_clear(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
#ifdef PRINT_EVENT_MSG
    printf("producer clear\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_producer_identified_reserved(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

#ifdef PRINT_EVENT_MSG
    printf("producer identified\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_identify(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    if (openlcb_node->producers.enumerator.enum_index < openlcb_node->parameters->producer_count) {

        _identify_producers(openlcb_node, openlcb_msg, worker_msg);

    } else if (openlcb_node->consumers.enumerator.enum_index < openlcb_node->parameters->consumer_count) {

        _identify_consumers(openlcb_node, openlcb_msg, worker_msg);

    } else {
        
        openlcb_node->producers.enumerator.enum_index = 0;
        openlcb_node->consumers.enumerator.enum_index = 0;
        
        openlcb_node->state.openlcb_msg_handled = TRUE;
         
    }

}

void ProtocolEventTransport_handle_identify_dest(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;
    
    if (Utilities_is_message_for_node(openlcb_node, openlcb_msg)) {
        
        ProtocolEventTransport_handle_identify(openlcb_node, openlcb_msg, worker_msg);
        
    } else {
        
        openlcb_node->state.openlcb_msg_handled = TRUE;
        
    }
        
}

void ProtocolEventTransport_handle_event_learn(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_pc_event_report(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

#ifdef PRINT_EVENT_MSG
    printf("PCER no payload\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

#ifdef PRINT_EVENT_MSG
    printf("PCER payload\n");
#endif

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

