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
#include "unistd.h"

#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_list.h"
#include "openlcb_defines.h"
#include "openlcb_buffer_fifo.h"



static interface_openlcb_main_statemachine_t* _interface;

static openlcb_statemachine_info_t _statemachine_info;
static openlcb_msg_t _outgoing_msg;
static payload_stream_t _outgoing_msg_payload;
static openlcb_msg_t _login_openlcb_msg;
static payload_basic_t _login_openlcb_payload;

void OpenLcbMainStatemachine_initialize(const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine) {

    _interface = (interface_openlcb_main_statemachine_t*) interface_openlcb_main_statemachine;

    _outgoing_msg.payload = (openlcb_payload_t*) & _outgoing_msg_payload;
    _outgoing_msg.payload_type = STREAM;
    OpenLcbUtilities_clear_openlcb_message(&_outgoing_msg);
    OpenLcbUtilities_clear_openlcb_message_payload(&_outgoing_msg);
    _outgoing_msg.state.allocated = true;

    OpenLcbUtilities_clear_openlcb_message(&_login_openlcb_msg);
    _login_openlcb_msg.payload = (openlcb_payload_t*) & _login_openlcb_payload;
    _login_openlcb_msg.state.allocated = true;
    _login_openlcb_msg.payload_type = BASIC;

    _statemachine_info.login_outgoing_openlcb_msg = &_login_openlcb_msg;
    _statemachine_info.login_outgoing_openlcb_msg_valid = false;
    _statemachine_info.outgoing_msg = &_outgoing_msg;
    _statemachine_info.incoming_msg = NULL;
    _statemachine_info.openlcb_node = NULL;

}

void OpenLcbMainStatemachine_load_interaction_rejected(openlcb_statemachine_info_t *statemachine_info) {

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg->source_alias,
            statemachine_info->incoming_msg->source_id,
            MTI_OPTIONAL_INTERACTION_REJECTED,
            4);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg, statemachine_info->incoming_msg->mti, 2);

    statemachine_info->outgoing_msg_valid = true;

}

void OpenLcbMainStatemachine_process_main_statemachine(openlcb_statemachine_info_t *statemachine_info) {


    if (!statemachine_info) {

        return;

    }

    if ((!statemachine_info->incoming_msg) || (!statemachine_info->openlcb_node)) {

        return;

    }

    switch (statemachine_info->incoming_msg->mti) {

        case MTI_SIMPLE_NODE_INFO_REQUEST:

            if (_interface->snip_simple_node_info_request) {

                _interface->snip_simple_node_info_request(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_SIMPLE_NODE_INFO_REPLY:

            if (_interface->snip_simple_node_info_reply) {

                _interface->snip_simple_node_info_reply(statemachine_info);

            }

            break;

        case MTI_INITIALIZATION_COMPLETE:

            if (_interface->message_network_initialization_complete) {

                _interface->message_network_initialization_complete(statemachine_info);

            }

            break;

        case MTI_INITIALIZATION_COMPLETE_SIMPLE:

            if (_interface->message_network_initialization_complete_simple) {

                _interface->message_network_initialization_complete_simple(statemachine_info);

            }

            break;

        case MTI_PROTOCOL_SUPPORT_INQUIRY:

            if (_interface->message_network_protocol_support_inquiry) {

                _interface->message_network_protocol_support_inquiry(statemachine_info);

            }

            break;

        case MTI_PROTOCOL_SUPPORT_REPLY:

            if (_interface->message_network_protocol_support_reply) {

                _interface->message_network_protocol_support_reply(statemachine_info);

            }

            break;

        case MTI_VERIFY_NODE_ID_ADDRESSED:

            if (_interface->message_network_verify_node_id_addressed) {

                _interface->message_network_verify_node_id_addressed(statemachine_info);

            }

            break;

        case MTI_VERIFY_NODE_ID_GLOBAL:

            if (_interface->message_network_verify_node_id_global) {

                _interface->message_network_verify_node_id_global(statemachine_info);

            }

            break;

        case MTI_VERIFIED_NODE_ID:


            if (_interface->message_network_verified_node_id) {

                _interface->message_network_verified_node_id(statemachine_info);

            }

            break;

        case MTI_OPTIONAL_INTERACTION_REJECTED:

            if (_interface->message_network_optional_interaction_rejected) {

                _interface->message_network_optional_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_TERMINATE_DO_TO_ERROR:

            if (_interface->message_network_terminate_due_to_error) {

                _interface->message_network_terminate_due_to_error(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFY:

            if (_interface->event_transport_consumer_identify) {

                _interface->event_transport_consumer_identify(statemachine_info);

            }

            break;

        case MTI_CONSUMER_RANGE_IDENTIFIED:

            if (_interface->event_transport_consumer_range_identified) {

                _interface->event_transport_consumer_range_identified(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFIED_UNKNOWN:

            if (_interface->event_transport_consumer_identified_unknown) {

                _interface->event_transport_consumer_identified_unknown(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFIED_SET:

            if (_interface->event_transport_consumer_identified_set) {

                _interface->event_transport_consumer_identified_set(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFIED_CLEAR:

            if (_interface->event_transport_consumer_identified_clear) {

                _interface->event_transport_consumer_identified_clear(statemachine_info);

            }

            break;

        case MTI_CONSUMER_IDENTIFIED_RESERVED:

            if (_interface->event_transport_consumer_identified_reserved) {

                _interface->event_transport_consumer_identified_reserved(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFY:

            if (_interface->event_transport_producer_identify) {

                _interface->event_transport_producer_identify(statemachine_info);

            }

            break;

        case MTI_PRODUCER_RANGE_IDENTIFIED:

            if (_interface->event_transport_producer_range_identified) {

                _interface->event_transport_producer_range_identified(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFIED_UNKNOWN:

            if (_interface->event_transport_producer_identified_unknown) {

                _interface->event_transport_producer_identified_unknown(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFIED_SET:

            if (_interface->event_transport_producer_identified_set) {

                _interface->event_transport_producer_identified_set(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFIED_CLEAR:

            if (_interface->event_transport_producer_identified_clear) {

                _interface->event_transport_producer_identified_clear(statemachine_info);

            }

            break;

        case MTI_PRODUCER_IDENTIFIED_RESERVED:

            if (_interface->event_transport_producer_identified_reserved) {

                _interface->event_transport_producer_identified_reserved(statemachine_info);

            }

            break;

        case MTI_EVENTS_IDENTIFY_DEST:

            if (_interface->event_transport_identify_dest) {

                _interface->event_transport_identify_dest(statemachine_info);

            }

            break;

        case MTI_EVENTS_IDENTIFY:

            if (_interface->event_transport_identify) {

                _interface->event_transport_identify(statemachine_info);

            }

            break;

        case MTI_EVENT_LEARN:

            if (_interface->event_transport_learn) {

                _interface->event_transport_learn(statemachine_info);

            }

            break;

        case MTI_PC_EVENT_REPORT:

            if (_interface->event_transport_pc_report) {

                _interface->event_transport_pc_report(statemachine_info);

            }

            break;

        case MTI_PC_EVENT_REPORT_WITH_PAYLOAD:

            if (_interface->event_transport_pc_report_with_payload) {

                _interface->event_transport_pc_report_with_payload(statemachine_info);

            }

            break;

        case MTI_TRACTION_PROTOCOL:

            if (_interface->traction_control_command) {

                _interface->traction_control_command(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_TRACTION_REPLY:

            if (_interface->traction_control_reply) {

                _interface->traction_control_reply(statemachine_info);

            }

            break;

        case MTI_SIMPLE_TRAIN_INFO_REQUEST:

            if (_interface->simple_train_node_ident_info_request) {

                _interface->simple_train_node_ident_info_request(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

        case MTI_SIMPLE_TRAIN_INFO_REPLY:

            if (_interface->simple_train_node_ident_info_reply) {

                _interface->simple_train_node_ident_info_reply(statemachine_info);

            }

            break;

        case MTI_DATAGRAM:

            if (_interface->datagram) {

                _interface->datagram(statemachine_info);

            }

            break;

        case MTI_DATAGRAM_OK_REPLY:

            if (_interface->datagram_ok_reply) {

                _interface->datagram_ok_reply(statemachine_info);

            }

            break;

        case MTI_DATAGRAM_REJECTED_REPLY:

            if (_interface->datagram_rejected_reply) {

                _interface->datagram_rejected_reply(statemachine_info);

            }

            break;

        case MTI_STREAM_INIT_REQUEST:

            if (_interface->stream_initiate_request) {

                _interface->stream_initiate_request(statemachine_info);

            }

            break;

        case MTI_STREAM_INIT_REPLY:

            if (_interface->stream_initiate_reply) {

                _interface->stream_initiate_reply(statemachine_info);

            }

            break;

        case MTI_STREAM_SEND:

            if (_interface->stream_send_data) {

                _interface->stream_send_data(statemachine_info);

            }

            break;

        case MTI_STREAM_PROCEED:

            if (_interface->stream_data_proceed) {

                _interface->stream_data_proceed(statemachine_info);

            }

            break;

        case MTI_STREAM_COMPLETE:

            if (_interface->stream_data_complete) {

                _interface->stream_data_complete(statemachine_info);

            }

            break;

        default:

            if (OpenLcbUtilities_is_addressed_message_for_node(statemachine_info->openlcb_node, statemachine_info->incoming_msg)) {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;

    }


}

bool OpenLcbMainStatemachine_does_node_process_msg(openlcb_statemachine_info_t *_statemachine_info) {

    return ( (_statemachine_info->openlcb_node->state.initalized) &&
            (
            ((_statemachine_info->incoming_msg->mti & MASK_DEST_ADDRESS_PRESENT) != MASK_DEST_ADDRESS_PRESENT) || // if not addressed process it
            (((_statemachine_info->openlcb_node->alias == _statemachine_info->incoming_msg->dest_alias) || (_statemachine_info->openlcb_node->id == _statemachine_info->incoming_msg->dest_id)) && ((_statemachine_info->incoming_msg->mti & MASK_DEST_ADDRESS_PRESENT) == MASK_DEST_ADDRESS_PRESENT)) ||
            (_statemachine_info->incoming_msg->mti == MTI_VERIFY_NODE_ID_GLOBAL) // special case, the handler will decide if it should reply or not based on if there is a node id in the payload or not
            )
            );

}

static void _free_incoming_message(openlcb_statemachine_info_t *_statemachine_info) {

    if (!_statemachine_info->incoming_msg) {

        return;

    }

    _interface->lock_shared_resources();
    OpenLcbBufferStore_free_buffer(_statemachine_info->incoming_msg);
    _interface->unlock_shared_resources();
    _statemachine_info->incoming_msg = NULL;

}

static bool _handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg)) {

            _statemachine_info.outgoing_msg_valid = false; // done

        }

        return true; // keep trying till it can get set

    }
    return false;

}

static bool _handle_reenumerate_incoming_openlcb_message(void) {

    if (_statemachine_info.enumerating) {

        _interface->process_main_statemachine(&_statemachine_info); // Continue the processing of the incoming message on the node

        return true; // done until flag is cleared

    }

    return false;

}

//bool _handle_reenumerate_openlcb_message(void) {
//
//    if (_statemachine_info.enumerating) {
//
//        // Need to make sure the correct state-machine is run depending of if the Node had finished the login process
//
//        if (_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {
//
//            _run_statemachine(&_can_statemachine_info);
//
//        } else {
//
//            _interface->login_statemachine_run(&_statemachine_info);
//
//        }
//
//        return true; // done
//
//    }
//
//    return false;
//
//}

static bool _handle_login_outgoing_openlcb_message(void) {

    if (_statemachine_info.login_outgoing_openlcb_msg_valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.login_outgoing_openlcb_msg)) {

            _statemachine_info.login_outgoing_openlcb_msg_valid = false;

        }

        return true; // done for this loop, try again next time

    }

    return false;

}

static bool _handle_try_pop_next_incoming_openlcb_message(void) {

    if (!_statemachine_info.incoming_msg) {

        _interface->lock_shared_resources();

        _statemachine_info.incoming_msg = OpenLcbBufferFifo_pop();

        _interface->unlock_shared_resources();

        return true;

    }

    return false;

}

static bool _handle_try_enumerate_first_node(void) {

    if (!_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = _interface->openlcb_node_get_first(OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) {

            _free_incoming_message(&_statemachine_info); // no nodes are allocated yet, free the message buffer

            return true; // done

        }

        if (_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

            if (_interface->does_node_process_msg(&_statemachine_info)) {

                _interface->process_main_statemachine(&_statemachine_info); // Do the processing of the incoming message on the node

            }

        } else {

            _interface->login_statemachine_run(&_statemachine_info);

        }

        return true; // done

    }

    return false;

}

static bool _handle_try_enumerate_next_node(void) {

    if (_statemachine_info.openlcb_node) {

        _statemachine_info.openlcb_node = _interface->openlcb_node_get_next(OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX);

        if (!_statemachine_info.openlcb_node) { // reached the end of the list, free the incoming message

            _free_incoming_message(&_statemachine_info);

            return true; // done

        }

        if (_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

            if (_interface->does_node_process_msg(&_statemachine_info)) {

                _interface->process_main_statemachine(&_statemachine_info); // Do the processing of the incoming message on the node

            }

        } else {

            _interface->login_statemachine_run(&_statemachine_info);

        }

        return true; // done
    }

    return false;

}

static void _handle_process_main_statemachine(void) {

    if (_interface->does_node_process_msg(&_statemachine_info)) {

        _interface->process_main_statemachine(&_statemachine_info); // kick it off.

    }

}

void OpenLcbMainStatemachine_run(void) {


    //  usleep(25);


    if (_handle_outgoing_openlcb_message()) {

        return;

    }

    if (_handle_reenumerate_incoming_openlcb_message()) {

        return;

    }

    if (_handle_login_outgoing_openlcb_message()) {

        return;

    }

    if (_handle_try_pop_next_incoming_openlcb_message()) {

        return;

    }

    if (_handle_try_enumerate_first_node()) {

        return;

    }

    if (_handle_try_enumerate_next_node()) {

        return;

    }

    _handle_process_main_statemachine();

}
