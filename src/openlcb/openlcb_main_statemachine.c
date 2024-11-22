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
#include "protocol_snip.h"
#include "../drivers/mcu_driver.h"

typedef struct {
    
   
    openlcb_msg_t worker;
    payload_stream_t worker_buffer;
    openlcb_msg_t* active_msg;
   
} openlcb_statemachine_worker_t;


openlcb_statemachine_worker_t openlcb_helper;

void MainStatemachine_initialize() {
    
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

void _main_statemachine(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    switch (openlcb_msg->mti) {

        case MTI_SIMPLE_NODE_INFO_REQUEST:
        {
            ProtocolSnip_handle_simple_node_info_request(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PROTOCOL_SUPPORT_INQUIRY:
        { 
            ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_VERIFY_NODE_ID_ADDRESSED:
        {
            ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_VERIFY_NODE_ID_GLOBAL:
        {
            ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_VERIFIED_NODE_ID:
        {
            ProtocolMessageNetwork_handle_verified_node_id(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_CONSUMER_IDENTIFY:
        {
            ProtocolEventTransport_handle_consumer_identify(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_CONSUMER_IDENTIFY_RANGE:
        {
            ProtocolEventTransport_handle_consumer_identify_range(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_CONSUMER_IDENTIFIED_UNKNOWN:
        {
            ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_CONSUMER_IDENTIFIED_SET:
        {
            ProtocolEventTransport_handle_consumer_identified_set(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_CONSUMER_IDENTIFIED_CLEAR:
        {
            ProtocolEventTransport_handle_consumer_identified_clear(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_CONSUMER_IDENTIFIED_RESERVED:
        {
            ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PRODUCER_IDENDIFY:
        {
            ProtocolEventTransport_handle_producer_identify(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PRODUCER_IDENTIFY_RANGE:
        {
            ProtocolEventTransport_handle_producer_identify_range(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PRODUCER_IDENTIFIED_UNKNOWN:
        {
            ProtocolEventTransport_handle_producer_identified_unknown(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PRODUCER_IDENTIFIED_SET:
        {
            ProtocolEventTransport_handle_producer_identified_set(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PRODUCER_IDENTIFIED_CLEAR:
        {
            ProtocolEventTransport_handle_producer_identified_clear(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PRODUCER_IDENTIFIED_RESERVED:
        {
            ProtocolEventTransport_handle_producer_identified_reserved(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_EVENTS_IDENTIFY_DEST:
        {
            ProtocolEventTransport_handle_identify_dest(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_EVENTS_IDENTIFY:
        {
            ProtocolEventTransport_handle_identify(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_EVENT_LEARN:
        {
            ProtocolEventTransport_handle_event_learn(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PC_EVENT_REPORT:
        {
            ProtocolEventTransport_handle_pc_event_report(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD:
        {
            ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_node, openlcb_msg, &openlcb_helper.worker);
            return;
        }
        case MTI_DATAGRAM:
        {
            //        HandleDatagram(openlcb_node);
            openlcb_node->state.openlcb_msg_handled = TRUE;
            return;
        }
        case MTI_DATAGRAM_OK_REPLY:
        {
            //        HandleDatagramOkReply(openlcb_node);
            openlcb_node->state.openlcb_msg_handled = TRUE;
            return;
        }
        case MTI_DATAGRAM_REJECTED_REPLY:
        {
            //       HandleDatagramRejectedReply(openlcb_node);
            openlcb_node->state.openlcb_msg_handled = TRUE;
            return;
        }
        default:

            openlcb_node->state.openlcb_msg_handled = TRUE;

            //       HandleUnimplementedProtocol(openlcb_node);

            return;

    }


}

void MainStatemachine_run() {

    uint8_t reset = FALSE;
    
    if (!openlcb_helper.active_msg) {

        McuDriver_pause_can_rx();
        openlcb_helper.active_msg = BufferFifo_pop();
        McuDriver_resume_can_rx();
        
        reset = TRUE;

    }

    if (!openlcb_helper.active_msg)
        return;

    _RB4 = 1;

    uint8_t done = TRUE;
    openlcb_node_t* next_node = Node_get_first(1);

    while (next_node) {
        
        if (reset)
            next_node->state.openlcb_msg_handled = FALSE;
      
        _main_statemachine(next_node, openlcb_helper.active_msg);
       
        if (!next_node->state.openlcb_msg_handled) {
            
           done = FALSE;
                 
        }
            
        next_node = Node_get_next(1);

    }

    if (done) {
        
        BufferStore_freeBuffer(openlcb_helper.active_msg);

        openlcb_helper.active_msg = (void*) 0;

    }

    _RB4 = 0;

}



