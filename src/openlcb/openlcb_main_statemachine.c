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
#include "../openlcb/openlcb_buffer_list.h"
#include "../drivers/driver_configuration_memory.h"
#include "../drivers/driver_100ms_clock.h"

#include "openlcb_main_statemachine.h"


openlcb_statemachine_worker_t _openlcb_helper;

void MainStatemachine_initialize(
        mcu_driver_callback_t mcu_setup_callback,
        parameterless_callback_t reboot_callback,
        configuration_mem_callback_t configuration_mem_read_callback,
        configuration_mem_callback_t configuration_mem_write_callback,
        parameterless_callback_t configuration_factory_reset_callback,
        parameterless_callback_t _100ms_clock_pause_callback,
        parameterless_callback_t _100ms_clock_resume_callback
        ) {

    BufferStore_initialize();
    BufferList_initialize();
    BufferFifo_initialize();

    Node_initialize();
    ProtocolDatagram_initialize();

    Driver100msClock_initialization(_100ms_clock_pause_callback, _100ms_clock_resume_callback);
    DriverConfigurationMemory_initialization(configuration_mem_read_callback, configuration_mem_write_callback, configuration_factory_reset_callback);
    DriverMcu_initialization(mcu_setup_callback, reboot_callback);

    for (uint16_olcb_t i = 0; i < LEN_MESSAGE_BYTES_STREAM; i++)
        _openlcb_helper.worker_buffer[i] = 0x00;

    _openlcb_helper.active_msg = (void*) 0;

    _openlcb_helper.worker.source_alias = 0;
    _openlcb_helper.worker.source_id = 0;
    _openlcb_helper.worker.dest_alias = 0;
    _openlcb_helper.worker.dest_id = 0;
    _openlcb_helper.worker.mti = 0;
    _openlcb_helper.worker.timerticks = 0;
    _openlcb_helper.worker.state.inprocess = FALSE;
    _openlcb_helper.worker.payload = (openlcb_payload_t*) & _openlcb_helper.worker_buffer;
    _openlcb_helper.worker.payload_type = STREAM;   // make it the largest so it can fake as any
    _openlcb_helper.worker.state.allocated = TRUE;

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
#ifndef SUPPORT_FIRMWARE_BOOTLOADER
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
        case MTI_PRODUCER_IDENTIFY:
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
#endif // SUPPORT_FIRMWARE_BOOTLOADER
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

void MainStatemachine_run(void) {
    //
    //    uint8_olcb_t reset = FALSE;
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
    //    uint8_olcb_t all_nodes_done = TRUE;
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

openlcb_statemachine_worker_t* MainStatemachine_get_openlcb_helper(void) {
    
    return &_openlcb_helper;
    
}

void MainStatemachine_run_single_node(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    _process_main_statemachine(openlcb_node, openlcb_msg, worker_msg);

}




