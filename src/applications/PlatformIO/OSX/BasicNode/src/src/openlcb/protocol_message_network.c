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
 * \file protocol_message_network.c
 *
 * Implementation of the Openlcb core Messages that must be handled by all nodes.
 * Handlers are call from the openlcb_main_statemachine.c processing when a 
 * datagram message is being processed from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "protocol_message_network.h"

#include "stdio.h" // printf
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_buffer_store.h"
#include "openlcb_tx_driver.h"
#include "../drivers/driver_100ms_clock.h"
#include "../drivers/driver_can.h"

void _send_duplicate_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message


    if (openlcb_node->state.duplicate_id_detected) { // Already handled this once

        openlcb_node->state.openlcb_msg_handled = TRUE; // Done with the message

        return;

    }

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_PC_EVENT_REPORT, 8);
    Utilities_copy_event_id_to_openlcb_payload(worker_msg, EVENT_ID_DUPLICATE_NODE_DETECTED);

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.duplicate_id_detected = TRUE;
        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void _send_verified_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_VERIFIED_NODE_ID, 3);
    Utilities_copy_node_id_to_openlcb_payload(worker_msg, openlcb_node->id, 0);

    if (openlcb_node->parameters->protocol_support & PSI_SIMPLE)
        worker_msg->mti = MTI_VERIFIED_NODE_ID_SIMPLE;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (!Utilities_addressed_message_needs_processing(openlcb_node, openlcb_msg))
        return;

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_PROTOCOL_SUPPORT_REPLY, 6);

    uint64_olcb_t temp = openlcb_node->parameters->protocol_support;

    if (openlcb_node->state.firmware_upgrade)
        temp = temp | PSI_FIRMWARE_UPGRADE_ACTIVE;

    if (temp > 0)

        while ((temp & 0xFF00000000000000) == 0)

            temp = temp << 8;

    *worker_msg->payload[0] = (uint8_olcb_t) (temp >> 56) & 0xFF;
    *worker_msg->payload[1] = (uint8_olcb_t) (temp >> 48) & 0xFF;
    *worker_msg->payload[2] = (uint8_olcb_t) (temp >> 40) & 0xFF;
    *worker_msg->payload[3] = (uint8_olcb_t) (temp >> 32) & 0xFF;
    *worker_msg->payload[4] = (uint8_olcb_t) (temp >> 24) & 0xFF;
    *worker_msg->payload[5] = (uint8_olcb_t) (temp >> 16) & 0xFF;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg))

        openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message


    if (openlcb_msg->payload_count > 0) {

        if (Utilities_extract_node_id_from_openlcb_payload(openlcb_msg, 0) == openlcb_node->id)

            _send_verified_node_id(openlcb_node, openlcb_msg, worker_msg);

        else

            openlcb_node->state.openlcb_msg_handled = TRUE; // Done with the message  

    } else

        _send_verified_node_id(openlcb_node, openlcb_msg, worker_msg);

    return;

}

void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (Utilities_addressed_message_needs_processing(openlcb_node, openlcb_msg))

        _send_verified_node_id(openlcb_node, openlcb_msg, worker_msg);

    else

        openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_handle_verified_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message

    if (Utilities_extract_node_id_from_openlcb_payload(openlcb_msg, 0) == openlcb_node->id)

        _send_duplicate_node_id(openlcb_node, openlcb_msg, worker_msg);

    else

        openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_send_interaction_rejected(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (!Utilities_addressed_message_needs_processing(openlcb_node, openlcb_msg))
        return;

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_OPTIONAL_INTERACTION_REJECTED, 4);
    Utilities_copy_word_to_openlcb_payload(worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL, 0);
    Utilities_copy_word_to_openlcb_payload(worker_msg, openlcb_msg->mti, 2);


    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void ProtocolMessageNetwork_clear_resend_optional_message(openlcb_node_t* openlcb_node) {

    if (openlcb_node->last_received_optional_interaction) {

        BufferStore_freeBuffer(openlcb_node->last_received_optional_interaction);

        openlcb_node->last_received_optional_interaction = (void*) 0;

    }

    openlcb_node->state.resend_optional_message = FALSE;

}

void ProtocolMessageNetwork_buffer_optional_interaction_message_for_resend(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    ProtocolMessageNetwork_clear_resend_optional_message(openlcb_node);

    // Take a reference and store the sent message in case we have to resend it
    BufferStore_inc_reference_count(openlcb_msg);

    openlcb_node->last_received_optional_interaction = openlcb_msg;

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (openlcb_node->state.openlcb_msg_handled)
        return;

    uint16_olcb_t should_resend = Utilities_extract_word_from_openlcb_payload(openlcb_msg, 0) && ERROR_TEMPORARY == ERROR_TEMPORARY;

    if (should_resend && openlcb_node->last_received_optional_interaction) {

        if (Utilities_extract_word_from_openlcb_payload(openlcb_msg, 2) == openlcb_node->last_received_optional_interaction->mti) {

            openlcb_node->state.resend_optional_message = TRUE;

        } else {

            ProtocolMessageNetwork_clear_resend_optional_message(openlcb_node);

        }

    } else {

        ProtocolMessageNetwork_clear_resend_optional_message(openlcb_node);

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;


}