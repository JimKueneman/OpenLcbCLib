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
 * \file openlcb_main_statemachine.c
 *
 * Implementation of the Openlcb main statemachine.  You typically do not call this
 * statemachine directly it will depend on what the physical transport is and if that 
 * needs to deal with CAN Adaptations of the protocol.  
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "openlcb_main_statemachine.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_buffer_store.h"
#include "openlcb_buffer_list.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_buffer_fifo.h"
#include "protocol_event_transport.h"
#include "protocol_message_network.h"
#include "protocol_datagram.h"
#include "protocol_snip.h"



static openlcb_statemachine_worker_t _openlcb_helper;

static interface_openlcb_main_statemachine_t* _interface;

void OpenLcbMainStatemachine_initialize(const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine) {

    _interface = (interface_openlcb_main_statemachine_t*) interface_openlcb_main_statemachine;

    for (int i = 0; i < LEN_MESSAGE_BYTES_STREAM; i++) {

        _openlcb_helper.worker_buffer[i] = 0x00;

    }

    _openlcb_helper.active_msg = NULL;

    _openlcb_helper.worker.source_alias = 0;
    _openlcb_helper.worker.source_id = 0;
    _openlcb_helper.worker.dest_alias = 0;
    _openlcb_helper.worker.dest_id = 0;
    _openlcb_helper.worker.mti = 0;
    _openlcb_helper.worker.timerticks = 0;
    _openlcb_helper.worker.state.inprocess = false;
    _openlcb_helper.worker.payload = (openlcb_payload_t*) & _openlcb_helper.worker_buffer;
    _openlcb_helper.worker.payload_type = STREAM; // make it the largest so it can fake as any
    _openlcb_helper.worker.state.allocated = true;

}

static void _process_main_statemachine(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    return;




    switch (openlcb_msg->mti) {

        case MTI_SIMPLE_NODE_INFO_REQUEST:

            if (_interface->snip_simple_node_info_request) {

                _interface->snip_simple_node_info_request(openlcb_node, openlcb_msg, worker_msg);
                
            }

            return;

        case MTI_PROTOCOL_SUPPORT_INQUIRY:

            if (_interface->message_network_protocol_support_inquiry) {

                _interface->message_network_protocol_support_inquiry(openlcb_node, openlcb_msg, worker_msg);
                
            }

            return;

        case MTI_VERIFY_NODE_ID_ADDRESSED:

            if (_interface->message_network_verify_node_id_addressed) {
                
                _interface->message_network_verify_node_id_addressed(openlcb_node, openlcb_msg, worker_msg);
            
            }
   
            return;

        case MTI_VERIFY_NODE_ID_GLOBAL:
            
            if (_interface->message_network_verify_node_id_global) {
                
                _interface->message_network_verify_node_id_global(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_VERIFIED_NODE_ID:
            
            
            if (_interface->message_network_verified_node_id_addressed) {
                
                _interface->message_network_verify_node_id_global(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

#ifndef SUPPORT_FIRMWARE_BOOTLOADER
        case MTI_CONSUMER_IDENTIFY:

            if (_interface->event_transport_consumer_identify) {
                
                _interface->event_transport_consumer_identify(openlcb_node, openlcb_msg, worker_msg);
            
            }
         
            return;

        case MTI_CONSUMER_IDENTIFY_RANGE:

            if (_interface->event_transport_consumer_identify_range) {
                
                _interface->event_transport_consumer_identify_range(openlcb_node, openlcb_msg, worker_msg);
            
            }
        
            return;

        case MTI_CONSUMER_IDENTIFIED_UNKNOWN:
            
            if (_interface->event_transport_consumer_identified_unknown) {
                
                _interface->event_transport_consumer_identified_unknown(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_CONSUMER_IDENTIFIED_SET:
            
            if (_interface->event_transport_consumer_identified_set) {
                
                _interface->event_transport_consumer_identified_set(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_CONSUMER_IDENTIFIED_CLEAR:
            
            if (_interface->event_transport_consumer_identified_clear) {
                
                _interface->event_transport_consumer_identified_clear(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_CONSUMER_IDENTIFIED_RESERVED:
            
            if (_interface->event_transport_consumer_identified_reserved) {
                
                _interface->event_transport_consumer_identified_reserved(openlcb_node, openlcb_msg, worker_msg);
            
            }
            
            return;

        case MTI_PRODUCER_IDENTIFY:

            if (_interface->event_transport_producer_identify) {
                
                _interface->event_transport_producer_identify(openlcb_node, openlcb_msg, worker_msg);
            
            }
      
            return;

        case MTI_PRODUCER_IDENTIFY_RANGE:
            
            if (_interface->event_transport_producer_identify_range) {
                
                _interface->event_transport_producer_identify_range(openlcb_node, openlcb_msg, worker_msg);
            
            }
  
            return;

        case MTI_PRODUCER_IDENTIFIED_UNKNOWN:
            
            if (_interface->event_transport_producer_identified_unknown) {
                
                _interface->event_transport_producer_identified_unknown(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_PRODUCER_IDENTIFIED_SET:

            if (_interface->event_transport_producer_identified_set) {
                
                _interface->event_transport_producer_identified_set(openlcb_node, openlcb_msg, worker_msg);
            
            }
            
            return;

        case MTI_PRODUCER_IDENTIFIED_CLEAR:
            
            if (_interface->event_transport_producer_identified_clear) {
                
                _interface->event_transport_producer_identified_clear(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_PRODUCER_IDENTIFIED_RESERVED:
            
            if (_interface->event_transport_producer_identified_reserved) {
                
                _interface->event_transport_producer_identified_reserved(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_EVENTS_IDENTIFY_DEST:
            
            if (_interface->event_transport_identify_dest) {
                
                _interface->event_transport_identify_dest(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_EVENTS_IDENTIFY:
            
            if (_interface->event_transport_identify) {
                
                _interface->event_transport_identify(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_EVENT_LEARN:
            
            if (_interface->event_transport_learn) {
                
                _interface->event_transport_learn(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_PC_EVENT_REPORT:

            if (_interface->event_transport_pc_report) {
                
                _interface->event_transport_pc_report(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD:
            
            if (_interface->event_transport_pc_report_with_payload) {
                
                _interface->event_transport_pc_report_with_payload(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

#endif // SUPPORT_FIRMWARE_BOOTLOADER
        case MTI_DATAGRAM:

            if (_interface->datagram) {
                
                _interface->datagram(openlcb_node, openlcb_msg, worker_msg);
            
            }
            
            return;

        case MTI_DATAGRAM_OK_REPLY:
            
            if (_interface->datagram_ok_reply) {
                
                _interface->datagram_ok_reply(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_DATAGRAM_REJECTED_REPLY:
            
            if (_interface->datagram_rejected_reply) {
                
                _interface->datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        case MTI_OPTIONAL_INTERACTION_REJECTED:

            if (_interface->optional_interaction_rejected) {
                
                _interface->optional_interaction_rejected(openlcb_node, openlcb_msg, worker_msg);
            
            }

            return;

        default:

            if (_interface->send_interaction_rejected) {
                
                _interface->send_interaction_rejected(openlcb_node, openlcb_msg, worker_msg);
            
            }
        
            return;

    }


}

void OpenLcbMainStatemachine_run(void) {
    //
    //    uint8_t reset = false;
    //    
    //    if (!openlcb_helper.active_msg) {
    //
    //        McuDriver_pause_can_rx();
    //        openlcb_helper.active_msg = BufferFifo_pop();
    //        McuDriver_resume_can_rx();
    //        
    //        reset = true;
    //
    //    }
    //
    //    if (!openlcb_helper.active_msg)
    //        return;
    //
    //
    //    uint8_t all_nodes_done = true;
    //    openlcb_node_t* next_node = Node_get_first(2);
    //
    //    while (next_node) {
    //        
    //        
    //        
    //        if (reset)
    //            next_node->state.openlcb_msg_handled = false;
    //      
    //        _process_main_statemachine(next_node, openlcb_helper.active_msg);
    //       
    //        // The Buffer_pop() clears the owner and it is a free agent.  
    //        // If one of the handlers took the message it has set the owner to its own
    //        // internal storage system
    //        // WARNING: The only time the message should be taken is if it was addressed to the node!!!
    //        if (openlcb_helper.active_msg->owner) {
    //            
    //             openlcb_helper.active_msg = NULL;
    //             break;
    //            
    //        }
    //       
    //        if (!next_node->state.openlcb_msg_handled) {
    //            
    //           all_nodes_done = false;
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
    //        openlcb_helper.active_msg = NULL;
    //
    //    }


}

openlcb_statemachine_worker_t* OpenLcbMainStatemachine_get_openlcb_helper(void) {

    return &_openlcb_helper;

}

void OpenLcbMainStatemachine_run_single_node(openlcb_node_t* openlcb_node) {

    _process_main_statemachine(openlcb_node, NULL, NULL);

}




