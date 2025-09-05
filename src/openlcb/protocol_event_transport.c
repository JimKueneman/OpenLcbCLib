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
 * \file protocol_event_transport.c
 *
 * Implementation of the Openlcb event protocol. Handlers are call from the 
 * openlcb_main_statemachine.c processing when a datagram message is being processed
 * from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
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

static void _encode_event_status(uint8_t* state_byte, uint8_t event_offset, event_enum_state_t new_status) {

    uint8_t mask;

    switch (new_status) {

        case EVENT_STATUS_UNKNOWN:

            mask = 0b11; // Set bit is what will be cleared gives 0b00)
            break;

        case EVENT_STATUS_SET: // Set bit is what will be cleared gives 0b01)

            mask = 0b10;
            break;

        case EVENT_STATUS_CLEAR: // Set bit is what will be cleared gives 0b10)

            mask = 0b01;
            break;

        default:

            mask = 0b00;
    }

    // Clear the bits in that offset in the byte
    (*state_byte) = (*state_byte) & ~(0b00000011 << ((3 - event_offset) * 2));
    // Set the index offset to the new value
    (*state_byte) = (*state_byte) | (uint8_t) ((mask << (3 - event_offset) * 2));

}

void ProtocolEventTransport_set_consumer_event_status(openlcb_node_t* openlcb_node, uint8_t event_index, event_enum_state_t new_status) {

    uint8_t* event_byte_ptr = &openlcb_node->consumers.event_status_array[event_index / EVENTS_ENCODED_IN_BYTE]; // Find the Byte that contain this events encoded state
    uint8_t event_offset = event_index % EVENTS_ENCODED_IN_BYTE; // Find the Offset of the encoded state within that byte

    _encode_event_status(event_byte_ptr, event_offset, new_status);

}

void ProtocolEventTransport_set_producer_event_status(openlcb_node_t* openlcb_node, uint8_t event_index, event_enum_state_t new_status) {

    uint8_t* event_byte_ptr = &openlcb_node->producers.event_status_array[event_index / EVENTS_ENCODED_IN_BYTE]; // Find the Byte that contain this events encoded state
    uint8_t event_offset = event_index % EVENTS_ENCODED_IN_BYTE; // Find the Offset of the encoded state within that byte

    _encode_event_status(event_byte_ptr, event_offset, new_status);

}

static event_enum_state_t _decode_event_state(uint8_t state_byte, uint8_t event_offset) {


    switch ((state_byte << ((3 - event_offset) * 2)) & 0x03) {

        case 0x00:

            return EVENT_STATUS_UNKNOWN;

        case 0x01:

            return EVENT_STATUS_SET;

        case 0x02:

            return EVENT_STATUS_CLEAR;

        default:

            return EVENT_STATUS_UNKNOWN;

    }

}

static void _identify_producers(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (!openlcb_node->producers.enumerator.running) {

        openlcb_node->producers.enumerator.enum_index = 0;
        openlcb_node->producers.enumerator.running = true; // Kick off the enumeration

    }

    if (openlcb_node->producers.enumerator.enum_index >= openlcb_node->producers.count) {

        openlcb_node->producers.enumerator.running = false;

        return; // done

    }

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, ProtocolEventTransport_extract_producer_event_status_mti(openlcb_node, openlcb_node->producers.enumerator.enum_index), 8);
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(worker_msg, openlcb_node->producers.list[openlcb_node->producers.enumerator.enum_index]);
    openlcb_node->producers.enumerator.enum_index = openlcb_node->producers.enumerator.enum_index + 1;

}

static void _identify_consumers(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (!openlcb_node->consumers.enumerator.running) {

        openlcb_node->consumers.enumerator.enum_index = 0;
        openlcb_node->consumers.enumerator.running = true; // Kick off the enumeration next loop

    }

    if (openlcb_node->consumers.enumerator.enum_index >= openlcb_node->consumers.count) {

        openlcb_node->consumers.enumerator.running = false;

        return; // done

    }

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, ProtocolEventTransport_extract_consumer_event_status_mti(openlcb_node, openlcb_node->consumers.enumerator.enum_index), 8);
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(worker_msg, openlcb_node->consumers.list[openlcb_node->consumers.enumerator.enum_index]);
    openlcb_node->consumers.enumerator.enum_index = openlcb_node->consumers.enumerator.enum_index + 1;

}

uint16_t ProtocolEventTransport_extract_consumer_event_status_mti(openlcb_node_t* openlcb_node, uint16_t event_index) {

    uint8_t event_state = openlcb_node->consumers.event_status_array[event_index / EVENTS_ENCODED_IN_BYTE];
    uint8_t event_offset = event_index % EVENTS_ENCODED_IN_BYTE;

    switch (_decode_event_state(event_state, event_offset)) {

        case EVENT_STATUS_UNKNOWN:

            return MTI_CONSUMER_IDENTIFIED_UNKNOWN;

        case EVENT_STATUS_SET:

            return MTI_CONSUMER_IDENTIFIED_SET;

        case EVENT_STATUS_CLEAR:

            return MTI_CONSUMER_IDENTIFIED_CLEAR;

        default:

            return MTI_CONSUMER_IDENTIFIED_UNKNOWN;

    }

}

uint16_t ProtocolEventTransport_extract_producer_event_status_mti(openlcb_node_t* openlcb_node, uint16_t event_index) {

    uint8_t event_state = openlcb_node->producers.event_status_array[event_index / EVENTS_ENCODED_IN_BYTE];
    uint8_t event_offset = event_index % EVENTS_ENCODED_IN_BYTE;

    switch (_decode_event_state(event_state, event_offset)) {

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

bool ProtocolEventTransport_handle_consumer_identify(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    uint16_t event_index = 0;
    event_id_t target_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

    if (!OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node, target_event_id, &event_index)) {

        return true; //  done 

    }

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, ProtocolEventTransport_extract_consumer_event_status_mti(openlcb_node, event_index), 8);
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(worker_msg, openlcb_node->consumers.list[event_index]);

    return true; //  done 
}

bool ProtocolEventTransport_handle_consumer_range_identified(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_consumer_range_identified) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_consumer_range_identified(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_consumer_identified_unknown) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_consumer_identified_unknown(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_consumer_identified_set(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_consumer_identified_set) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_consumer_identified_set(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_consumer_identified_clear(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_consumer_identified_clear) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_consumer_identified_clear(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_consumer_identified_reserved) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_consumer_identified_reserved(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_producer_identify(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    uint16_t event_index = 0;
    event_id_t target_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

    if (!OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node, target_event_id, &event_index)) {

        return true; //  done 

    }

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, ProtocolEventTransport_extract_producer_event_status_mti(openlcb_node, event_index), 8);
    OpenLcbUtilities_copy_event_id_to_openlcb_payload(worker_msg, openlcb_node->producers.list[event_index]);

    return true; //  done 

}

bool ProtocolEventTransport_handle_producer_range_identified(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_producer_range_identified) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_producer_range_identified(openlcb_node, &eventid);

    }

    return true;

}

bool ProtocolEventTransport_handle_producer_identified_unknown(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_producer_identified_unknown) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_producer_identified_unknown(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_producer_identified_set(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_producer_identified_set) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_producer_identified_set(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_producer_identified_clear(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_producer_identified_clear) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_producer_identified_clear(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_producer_identified_reserved(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_producer_identified_reserved) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_producer_identified_reserved(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_identify(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->producers.enumerator.enum_index < openlcb_node->producers.count) {

        _identify_producers(openlcb_node, openlcb_msg, worker_msg);

        return (!openlcb_node->producers.enumerator.running);
    }

    if (openlcb_node->consumers.enumerator.enum_index < openlcb_node->consumers.count) {

        _identify_consumers(openlcb_node, openlcb_msg, worker_msg);

        return (!openlcb_node->consumers.enumerator.running);

    }

    return true; // done

}

bool ProtocolEventTransport_handle_identify_dest(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (OpenLcbUtilities_is_addressed_message_for_node(openlcb_node, openlcb_msg)) {

        return ProtocolEventTransport_handle_identify(openlcb_node, openlcb_msg, worker_msg);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_event_learn(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_event_learn) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_event_learn(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_pc_event_report(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (_interface->on_pc_event_report) {

        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        _interface->on_pc_event_report(openlcb_node, &eventid);

    }

    return true; //  done 

}

bool ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_node_t * openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_msg->payload_count < sizeof (event_id_t)) {
        
        return true;
        
    }
    
    if (_interface->on_pc_event_report_with_payload) {

        event_payload_t payload; 
        event_id_t eventid = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);
        uint16_t payload_count = (openlcb_msg->payload_count - sizeof (eventid));
        if (payload_count > LEN_EVENT_PAYLOAD) {
            
            payload_count = LEN_EVENT_PAYLOAD;
        }

        memcpy(&payload, &openlcb_msg->payload[sizeof (event_id_t)], payload_count);

        _interface->on_pc_event_report_with_payload(openlcb_node, &eventid, payload_count, (event_payload_t*) &openlcb_msg->payload[sizeof (event_id_t)]);
    }

    return true; //  done 

}

