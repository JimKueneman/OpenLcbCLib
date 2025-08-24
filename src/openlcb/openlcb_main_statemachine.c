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

#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_list.h"
#include "openlcb_defines.h"
#include "openlcb_buffer_fifo.h"



static interface_openlcb_main_statemachine_t* _interface;

void OpenLcbMainStatemachine_initialize(const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine) {

    _interface = (interface_openlcb_main_statemachine_t*) interface_openlcb_main_statemachine;

}

static bool _process_main_statemachine(openlcb_node_t* openlcb_node, openlcb_msg_t* incoming_msg, openlcb_msg_t* outgoing_msg) {

    // TODO TEMPRARY

    return true;

    switch (incoming_msg->mti) {

        case MTI_SIMPLE_NODE_INFO_REQUEST:

            if (_interface->snip_simple_node_info_request) {

                return _interface->snip_simple_node_info_request(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PROTOCOL_SUPPORT_INQUIRY:

            if (_interface->message_network_protocol_support_inquiry) {

                return _interface->message_network_protocol_support_inquiry(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_VERIFY_NODE_ID_ADDRESSED:

            if (_interface->message_network_verify_node_id_addressed) {

                return _interface->message_network_verify_node_id_addressed(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_VERIFY_NODE_ID_GLOBAL:

            if (_interface->message_network_verify_node_id_global) {

                return _interface->message_network_verify_node_id_global(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_VERIFIED_NODE_ID:


            if (_interface->message_network_verified_node_id_addressed) {

                return _interface->message_network_verify_node_id_global(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_CONSUMER_IDENTIFY:

            if (_interface->event_transport_consumer_identify) {

                return _interface->event_transport_consumer_identify(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_CONSUMER_IDENTIFY_RANGE:

            if (_interface->event_transport_consumer_identify_range) {

                return _interface->event_transport_consumer_identify_range(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_CONSUMER_IDENTIFIED_UNKNOWN:

            if (_interface->event_transport_consumer_identified_unknown) {

                return _interface->event_transport_consumer_identified_unknown(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_CONSUMER_IDENTIFIED_SET:

            if (_interface->event_transport_consumer_identified_set) {

                return _interface->event_transport_consumer_identified_set(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_CONSUMER_IDENTIFIED_CLEAR:

            if (_interface->event_transport_consumer_identified_clear) {

                return _interface->event_transport_consumer_identified_clear(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_CONSUMER_IDENTIFIED_RESERVED:

            if (_interface->event_transport_consumer_identified_reserved) {

                return _interface->event_transport_consumer_identified_reserved(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PRODUCER_IDENTIFY:

            if (_interface->event_transport_producer_identify) {

                return _interface->event_transport_producer_identify(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PRODUCER_IDENTIFY_RANGE:

            if (_interface->event_transport_producer_identify_range) {

                return _interface->event_transport_producer_identify_range(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PRODUCER_IDENTIFIED_UNKNOWN:

            if (_interface->event_transport_producer_identified_unknown) {

               return  _interface->event_transport_producer_identified_unknown(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PRODUCER_IDENTIFIED_SET:

            if (_interface->event_transport_producer_identified_set) {

                return _interface->event_transport_producer_identified_set(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PRODUCER_IDENTIFIED_CLEAR:

            if (_interface->event_transport_producer_identified_clear) {

                _interface->event_transport_producer_identified_clear(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PRODUCER_IDENTIFIED_RESERVED:

            if (_interface->event_transport_producer_identified_reserved) {

                return _interface->event_transport_producer_identified_reserved(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_EVENTS_IDENTIFY_DEST:

            if (_interface->event_transport_identify_dest) {

               return  _interface->event_transport_identify_dest(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_EVENTS_IDENTIFY:

            if (_interface->event_transport_identify) {

               return  _interface->event_transport_identify(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_EVENT_LEARN:

            if (_interface->event_transport_learn) {

               return  _interface->event_transport_learn(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PC_EVENT_REPORT:

            if (_interface->event_transport_pc_report) {

                return _interface->event_transport_pc_report(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD:

            if (_interface->event_transport_pc_report_with_payload) {

               return  _interface->event_transport_pc_report_with_payload(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_DATAGRAM:

            if (_interface->datagram) {

                _interface->datagram(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_DATAGRAM_OK_REPLY:

            if (_interface->datagram_ok_reply) {

                _interface->datagram_ok_reply(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_DATAGRAM_REJECTED_REPLY:

            if (_interface->datagram_rejected_reply) {

                _interface->datagram_rejected_reply(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        case MTI_OPTIONAL_INTERACTION_REJECTED:

            if (_interface->optional_interaction_rejected) {

                _interface->optional_interaction_rejected(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

        default:

            if (_interface->send_interaction_rejected) {

                _interface->send_interaction_rejected(openlcb_node, incoming_msg, outgoing_msg);

            }

            return true;

    }


}

bool _process_msg_on_node(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg) {

    return ( (openlcb_node->state.initalized) &&
            (((openlcb_msg->mti & MASK_DEST_ADDRESS_PRESENT) != MASK_DEST_ADDRESS_PRESENT) || // if not addressed process it
            (((openlcb_node->alias == openlcb_msg->dest_alias) || (openlcb_node->id == openlcb_msg->dest_id)) && ((openlcb_msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT)))
            );

}

bool _try_transmit_active_msg(openlcb_msg_t *active_outgoing_msg) {

    if (active_outgoing_msg) {

        if (!_interface->transmit_openlcb_message(active_outgoing_msg)) {

            return false; // try again next loop

        }

    }

    return true;

}

void _try_free_current_and_pop_next_incoming_msg(openlcb_msg_t **active_incoming_msg) {

    _interface->lock_openlcb_buffer_fifo();
    OpenLcbBufferStore_free_buffer(*active_incoming_msg); // When we reach a repeat of the nodes the previous message should have been completely handled
    *active_incoming_msg = OpenLcbBufferFifo_pop();
    _interface->unlock_openlcb_buffer_fifo();

}

bool _try_reprocess_active_node(openlcb_node_t *active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg) {

    if (!active_node->state.initalized) {

        return _process_main_statemachine(active_node, active_incoming_msg, active_outgoing_msg);

    }

    return true;
}

bool _process_node(openlcb_node_t *active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg) {

    if (_process_msg_on_node(active_node, active_incoming_msg)) {

        return _process_main_statemachine(active_node, active_incoming_msg, active_outgoing_msg);

    }

    return true;

}

bool _try_process_first_node(openlcb_node_t **active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg) {

    *active_node = _interface->node_get_first(OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX);


    if (!*active_node || !active_incoming_msg) {

        return false;

    }

    return _process_node(*active_node, active_incoming_msg, active_outgoing_msg);

}

bool _try_process_next_node(openlcb_node_t **active_node, openlcb_msg_t *active_incoming_msg, openlcb_msg_t *active_outgoing_msg) {

    *active_node = _interface->node_get_next(OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX);


    if (!*active_node || !active_incoming_msg) {

        return false;

    }

    return _process_node(*active_node, active_incoming_msg, active_outgoing_msg);

}

void OpenLcbMainStatemachine_run(void) {

    // Each node is checked for being initialized and if not is skipped after any messages it may hold are released (in case of duplicate Node ID and it is taken off line)
    // Each call to run only handles one enumeration of the nodes
    // One at a time nodes are loaded with the current incoming message
    // If the node needs to process it then it will process it and fill in the outgoing message, if it does not need to do anything it leaves outgoing message NULL
    // The loop then attempts to send it and if it can't then it will stall on that node and focus on getting the message out
    // When the message is sent the loop moves to the next node and repeats until all nodes are serviced
    // The next incoming message is popped off the FIFO and wash/rinse/repeat


    static openlcb_node_t *active_node = NULL;
    static openlcb_msg_t *outgoing_msg = NULL;
    static openlcb_msg_t *active_incoming_msg = NULL;
    static bool reprocess_active_node = false;
    static bool outgoing_msg_valid = false;


    // The message must be transmitted before the statemachine will continue
    if (outgoing_msg_valid) {
        
        if (_try_transmit_active_msg(outgoing_msg)) {

            outgoing_msg_valid = false;

        } else {
            
            return;
            
        }   
        
    }

    if (reprocess_active_node) {

        reprocess_active_node = !_try_reprocess_active_node(active_node, active_incoming_msg, outgoing_msg);

        return;

    }

    if (!active_node) {

        reprocess_active_node = false;
        _try_free_current_and_pop_next_incoming_msg(&active_incoming_msg);
        outgoing_msg_valid = _try_process_first_node(&active_node, active_incoming_msg, outgoing_msg);

    } else {

        reprocess_active_node = false;
        outgoing_msg_valid = _try_process_next_node(&active_node, active_incoming_msg, outgoing_msg);

    }

}

void OpenLcbMainStatemachine_run_single_node(openlcb_node_t * openlcb_node) {

    _process_main_statemachine(openlcb_node, NULL, NULL);

}




