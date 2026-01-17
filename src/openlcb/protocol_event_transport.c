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
 * @file protocol_event_transport.c
 * @brief Implementation of event transport protocol
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "protocol_event_transport.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf
#include <string.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"

static interface_openlcb_protocol_event_transport_t *_interface;

void ProtocolEventTransport_initialize(const interface_openlcb_protocol_event_transport_t *interface_openlcb_protocol_event_transport) {

    _interface = (interface_openlcb_protocol_event_transport_t*) interface_openlcb_protocol_event_transport;

}

static void _identify_producers(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info->openlcb_node->producers.enumerator.running) {

        statemachine_info->openlcb_node->producers.enumerator.running = true; // Kick off the enumeration next loop
        statemachine_info->incoming_msg_info.enumerate = true;

    }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            ProtocolEventTransport_extract_producer_event_status_mti(statemachine_info->openlcb_node, statemachine_info->openlcb_node->producers.enumerator.enum_index));

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->producers.list[statemachine_info->openlcb_node->producers.enumerator.enum_index].event);

 //   statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;

    statemachine_info->openlcb_node->producers.enumerator.enum_index++;

    statemachine_info->outgoing_msg_info.valid = true;

}

static void _identify_consumers(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info->openlcb_node->consumers.enumerator.running) {

        statemachine_info->openlcb_node->consumers.enumerator.running = true; // Kick off the enumeration next loop
        statemachine_info->incoming_msg_info.enumerate = true;

    }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            ProtocolEventTransport_extract_consumer_event_status_mti(statemachine_info->openlcb_node, statemachine_info->openlcb_node->consumers.enumerator.enum_index));

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->consumers.list[statemachine_info->openlcb_node->consumers.enumerator.enum_index].event);

 //   statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;

    statemachine_info->openlcb_node->consumers.enumerator.enum_index++;

    statemachine_info->outgoing_msg_info.valid = true;

}

uint16_t ProtocolEventTransport_extract_consumer_event_status_mti(openlcb_node_t* openlcb_node, uint16_t event_index) {

    switch (openlcb_node->consumers.list[event_index].status) {

        case EVENT_STATUS_SET:

            return MTI_CONSUMER_IDENTIFIED_SET;

        case EVENT_STATUS_CLEAR:

            return MTI_CONSUMER_IDENTIFIED_CLEAR;

        default:

            return MTI_CONSUMER_IDENTIFIED_UNKNOWN;

    }

}

uint16_t ProtocolEventTransport_extract_producer_event_status_mti(openlcb_node_t* openlcb_node, uint16_t event_index) {

    switch (openlcb_node->producers.list[event_index].status) {

        case EVENT_STATUS_SET:

            return MTI_PRODUCER_IDENTIFIED_SET;

        case EVENT_STATUS_CLEAR:

            return MTI_PRODUCER_IDENTIFIED_CLEAR;

        default:

            return MTI_PRODUCER_IDENTIFIED_UNKNOWN;

    }

}

void ProtocolEventTransport_handle_consumer_identify(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t event_index = 0;
    event_id_t target_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

    if (!OpenLcbUtilities_is_consumer_event_assigned_to_node(statemachine_info->openlcb_node, target_event_id, &event_index)) {

        statemachine_info->outgoing_msg_info.valid = false;

        return; //  done

    }

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            ProtocolEventTransport_extract_consumer_event_status_mti(statemachine_info->openlcb_node, event_index));

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->consumers.list[event_index].event);

  //  statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;

    statemachine_info->outgoing_msg_info.valid = true;

}

void ProtocolEventTransport_handle_consumer_range_identified(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_range_identified) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_range_identified(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_identified_unknown) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_identified_unknown(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_consumer_identified_set(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_identified_set) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_identified_set(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_consumer_identified_clear(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_identified_clear) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_identified_clear(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_consumer_identified_reserved) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_consumer_identified_reserved(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_producer_identify(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t event_index = 0;
    event_id_t target_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

    if (!OpenLcbUtilities_is_producer_event_assigned_to_node(statemachine_info->openlcb_node, target_event_id, &event_index)) {

        statemachine_info->outgoing_msg_info.valid = false;

        return;

    }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            ProtocolEventTransport_extract_producer_event_status_mti(statemachine_info->openlcb_node, event_index));

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->producers.list[event_index].event);

  //  statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8;

    statemachine_info->outgoing_msg_info.valid = true;

}

void ProtocolEventTransport_handle_producer_range_identified(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_producer_range_identified) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_range_identified(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_producer_identified_unknown(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_producer_identified_unknown) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_identified_unknown(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_producer_identified_set(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_producer_identified_set) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_identified_set(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_producer_identified_clear(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_producer_identified_clear) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_identified_clear(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_producer_identified_reserved(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_producer_identified_reserved) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_producer_identified_reserved(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_events_identify(openlcb_statemachine_info_t *statemachine_info) {

    if (statemachine_info->openlcb_node->producers.enumerator.enum_index < statemachine_info->openlcb_node->producers.count) {

        _identify_producers(statemachine_info);

        return;

    } else {

        statemachine_info->openlcb_node->producers.enumerator.running = false;

    }

    if (statemachine_info->openlcb_node->consumers.enumerator.enum_index < statemachine_info->openlcb_node->consumers.count) {

        _identify_consumers(statemachine_info);

        return;

    } else {

        statemachine_info->openlcb_node->consumers.enumerator.enum_index = 0; // Reset for next enumeration
        statemachine_info->openlcb_node->producers.enumerator.enum_index = 0; // Reset for next enumeration
        statemachine_info->openlcb_node->consumers.enumerator.running = false;
        statemachine_info->incoming_msg_info.enumerate = false;

    }

}

void ProtocolEventTransport_handle_events_identify_dest(openlcb_statemachine_info_t *statemachine_info) {


    if (OpenLcbUtilities_is_addressed_message_for_node(statemachine_info->openlcb_node, statemachine_info->incoming_msg_info.msg_ptr)) {

        ProtocolEventTransport_handle_events_identify(statemachine_info);

        return;

    }


    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_event_learn(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_event_learn) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_event_learn(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_pc_event_report(openlcb_statemachine_info_t *statemachine_info) {

    if (_interface->on_pc_event_report) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

        _interface->on_pc_event_report(statemachine_info->openlcb_node, &eventid);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

void ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_statemachine_info_t *statemachine_info) {

    if (statemachine_info->incoming_msg_info.msg_ptr->payload_count <= sizeof (event_id_t)) {

        statemachine_info->outgoing_msg_info.valid = false;

        return;

    }

    if (_interface->on_pc_event_report_with_payload) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);
        uint16_t payload_count = (statemachine_info->incoming_msg_info.msg_ptr->payload_count - sizeof (event_id_t));

        _interface->on_pc_event_report_with_payload(statemachine_info->openlcb_node, &eventid, payload_count, (event_payload_t*) statemachine_info->incoming_msg_info.msg_ptr->payload[sizeof (event_id_t)]);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

