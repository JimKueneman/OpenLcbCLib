/*
 * File:   protocol_datagram.c
 * Author: jimkueneman
 *
 * Created on November 15, 2024, 8:50 PM
 */


#include "xc.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_defines.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"


// Little use having a buffer larger than the number of Datagram messages we can create;
#define LEN_DATAGRAM_REPLY_WAITING_LIST LEN_DATAGRAM_BUFFER
#define MAX_RETRY_COUNT 3
#define RETRY_TIME_MS   300

openlcb_msg_t* reply_waiting_list[LEN_DATAGRAM_REPLY_WAITING_LIST];

void ProtocolDatagram_initialize() {

    for (int i = 0; i < sizeof (reply_waiting_list) / sizeof (openlcb_msg_t*); i++)

        reply_waiting_list[i] = (void*) 0;

}

void _send_datagram(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    //  TODO: implement


}

void ProtocolDatagram_handle_datagram(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (!openlcb_node->state.openlcb_msg_handled)
        return;

    if (!Utilities_is_message_for_node(openlcb_node, openlcb_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

        return;
    }
    


    for (int i = 0; i < LEN_DATAGRAM_REPLY_WAITING_LIST; i++) {

        if (!reply_waiting_list[i]) {


            // TODO: Need to try to do what is requested in the Datagram
            switch (*openlcb_msg->payload[0]) {

                case 0x20:

                    openlcb_msg->owner = &reply_waiting_list; 
                    openlcb_node->state.openlcb_msg_handled = TRUE;

                    reply_waiting_list[i] = openlcb_msg;
                    openlcb_node->timerticks = 0;
                    // Do not reset retry as it may be a message we reinserted 
                    // into the Buffer_push_existing to try again and need to keep track of the times we try
                    // need to trust it was reset by the original allocation of this message in the BufferStore_allocate())
                    
                    // TODO:  Reply to this datagram and then wait for the ACK or NoACK
    
                    break;

                default:
                {

                    openlcb_node->state.openlcb_msg_handled = TRUE;
                    
                    // TODO: Send unknown Datagram Type

                }

            }

        }

    }

}

void Protocol_Datagram_handle_datagram_ok_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (!openlcb_node->state.openlcb_msg_handled)
        return;

    for (int i = 0; i < LEN_DATAGRAM_REPLY_WAITING_LIST; i++) {

        if (Utilities_is_message_for_node(openlcb_node, reply_waiting_list[i])) {

            BufferStore_freeBuffer(reply_waiting_list[i]);
            openlcb_node->state.openlcb_msg_handled = TRUE;

        }

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolDatagram_handle_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (!openlcb_node->state.openlcb_msg_handled)
        return;

    for (int i = 0; i < LEN_DATAGRAM_REPLY_WAITING_LIST; i++) {

        if (Utilities_is_message_for_node(openlcb_node, reply_waiting_list[i])) {

            if (openlcb_msg->retry_count < MAX_RETRY_COUNT) {

                _send_datagram(openlcb_node, openlcb_msg, worker_msg);

                openlcb_msg->retry_count = openlcb_msg->retry_count + 1;

            } else {

                BufferStore_freeBuffer(reply_waiting_list[i]);
                openlcb_node->state.openlcb_msg_handled = TRUE;

            }

        }

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void DatagramProtocol_100ms_time_tick() {

    _RB4 = 1;

    for (int i = 0; i < LEN_DATAGRAM_REPLY_WAITING_LIST; i++) {

        if (reply_waiting_list[i] != (void*) 0) {

            if (reply_waiting_list[i]-> timerticks < RETRY_TIME_MS)

                reply_waiting_list[i]-> timerticks = reply_waiting_list[i]-> timerticks + 1;

            else {

                if (reply_waiting_list[i]->retry_count < MAX_RETRY_COUNT) {

                    // Try it again
                    // THIS WON'T WORK AS THE RETRY COUNT WILL GET SMASHED NEXT PASS THROUGH
                    BufferFifo_push_existing(reply_waiting_list[i]);

                    reply_waiting_list[i]->retry_count = reply_waiting_list[i]->retry_count + 1;

                }
                // Give up time to drop it
                BufferStore_freeBuffer(reply_waiting_list[i]);
                reply_waiting_list[i] = (void*) 0;

            }

        }


    }

    _RB4 = 0;


}




