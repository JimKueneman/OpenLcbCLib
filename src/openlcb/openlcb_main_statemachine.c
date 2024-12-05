/*
 * File:   openlcb_main_statemachine.c
 * Author: jimkueneman
 *
 * Created on November 20, 2024, 6:18 PM
 */


#include "xc.h"
#include "stdio.h"  // printf
#include "openlcb_buffer_store.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_utilities.h"
#include "protocol_event_transport.h"
#include "protocol_message_network.h"
#include "protocol_datagram.h"
#include "protocol_snip.h"
#include "../drivers/driver_mcu.h"
#include "../openlcb/clock_distribution.h"
#include "../openlcb/openlcb_buffer_list.h"
#include "../drivers/driver_configuration_memory.h"

#include "openlcb_main_statemachine.h"


openlcb_statemachine_worker_t openlcb_helper;



void MainStatemachine_initialize() {
    
    BufferStore_initialize();
    BufferList_initialiaze();
    BufferFifo_initialiaze();
    
    Node_initialize();
    ProtocolDatagram_initialize();
    ClockDistribution_initialize();
    DriverConfigurationMemory_initialization();
    
    for (int i = 0; i < LEN_MESSAGE_BYTES_STREAM; i++)
        openlcb_helper.worker_buffer[i] = 0x00;
    
    openlcb_helper.active_msg = (void*) 0;
    
    openlcb_helper.worker.source_alias = 0;
    openlcb_helper.worker.source_id = 0;
    openlcb_helper.worker.dest_alias = 0;
    openlcb_helper.worker.dest_id = 0;
    openlcb_helper.worker.mti = 0;
    openlcb_helper.worker.timerticks = 0;
    openlcb_helper.worker.retry_count = 0;
    openlcb_helper.worker.state.inprocess = FALSE;
    openlcb_helper.worker.payload = (openlcb_payload_t*) &openlcb_helper.worker_buffer;
    openlcb_helper.worker.payload_size = LEN_MESSAGE_BYTES_STREAM;
    openlcb_helper.worker.state.allocated = TRUE;

}

void _process_main_statemachine(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    switch (openlcb_msg->mti) {
       
        case MTI_SIMPLE_NODE_INFO_REQUEST:
        {
 
            ProtocolSnip_handle_simple_node_info_request(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PROTOCOL_SUPPORT_INQUIRY:
        { 
            ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_VERIFY_NODE_ID_ADDRESSED:
        {
            ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_VERIFY_NODE_ID_GLOBAL:
        {
            ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_VERIFIED_NODE_ID:
        {
            ProtocolMessageNetwork_handle_verified_node_id(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_CONSUMER_IDENTIFY:
        {
            ProtocolEventTransport_handle_consumer_identify(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_CONSUMER_IDENTIFY_RANGE:
        {
            ProtocolEventTransport_handle_consumer_identify_range(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_CONSUMER_IDENTIFIED_UNKNOWN:
        {
            ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_CONSUMER_IDENTIFIED_SET:
        {
            ProtocolEventTransport_handle_consumer_identified_set(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_CONSUMER_IDENTIFIED_CLEAR:
        {
            ProtocolEventTransport_handle_consumer_identified_clear(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_CONSUMER_IDENTIFIED_RESERVED:
        {
            ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PRODUCER_IDENDIFY:
        {
            ProtocolEventTransport_handle_producer_identify(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PRODUCER_IDENTIFY_RANGE:
        {
            ProtocolEventTransport_handle_producer_identify_range(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PRODUCER_IDENTIFIED_UNKNOWN:
        {
            ProtocolEventTransport_handle_producer_identified_unknown(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PRODUCER_IDENTIFIED_SET:
        {
            ProtocolEventTransport_handle_producer_identified_set(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PRODUCER_IDENTIFIED_CLEAR:
        {
            ProtocolEventTransport_handle_producer_identified_clear(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PRODUCER_IDENTIFIED_RESERVED:
        {
            ProtocolEventTransport_handle_producer_identified_reserved(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_EVENTS_IDENTIFY_DEST:
        {
            ProtocolEventTransport_handle_identify_dest(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_EVENTS_IDENTIFY:
        {
            ProtocolEventTransport_handle_identify(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_EVENT_LEARN:
        {
            ProtocolEventTransport_handle_event_learn(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PC_EVENT_REPORT:
        {
            ProtocolEventTransport_handle_pc_event_report(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD:
        {
            ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_DATAGRAM:
        {
            ProtocolDatagram_handle_datagram(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_DATAGRAM_OK_REPLY:
        {
            Protocol_Datagram_handle_datagram_ok_reply(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_DATAGRAM_REJECTED_REPLY:
        {
            ProtocolDatagram_handle_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        case MTI_OPTIONAL_INTERACTION_REJECTED:
        {
            ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_node, openlcb_msg, worker_msg);
            return;
        }
        default:
            
            ProtocolMessageNetwork_send_interaction_rejected(openlcb_node, openlcb_msg, worker_msg);

            return;

    }


}

void MainStatemachine_run() {
//
//    uint8_t reset = FALSE;
//    
//    if (!openlcb_helper.active_msg) {
//
//        McuDriver_pause_can_rx();
//        openlcb_helper.active_msg = BufferFifo_pop();
//        McuDriver_resume_can_rx();
//        
//        reset = TRUE;
//
//    }
//
//    if (!openlcb_helper.active_msg)
//        return;
//
//
//    uint8_t all_nodes_done = TRUE;
//    openlcb_node_t* next_node = Node_get_first(2);
//
//    while (next_node) {
//        
//        
//        
//        if (reset)
//            next_node->state.openlcb_msg_handled = FALSE;
//      
//        _process_main_statemachine(next_node, openlcb_helper.active_msg);
//       
//        // The Buffer_pop() clears the owner and it is a free agent.  
//        // If one of the handlers took the message it has set the owner to its own
//        // internal storage system
//        // WARNING: The only time the message should be taken is if it was addressed to the node!!!
//        if (openlcb_helper.active_msg->owner) {
//            
//             openlcb_helper.active_msg = (void*) 0;
//             break;
//            
//        }
//       
//        if (!next_node->state.openlcb_msg_handled) {
//            
//           all_nodes_done = FALSE;
//                 
//        }
//            
//        next_node = Node_get_next(2);
//
//    }
//
//    if (all_nodes_done) {
//        
//        BufferStore_freeBuffer(openlcb_helper.active_msg);
//
//        openlcb_helper.active_msg = (void*) 0;
//
//    }


}


void MainStatemachine_run_single_node(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {
    
    _process_main_statemachine(openlcb_node, openlcb_msg, worker_msg);
    
}




