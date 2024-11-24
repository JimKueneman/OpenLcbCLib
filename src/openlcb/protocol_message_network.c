/*
 * File:   protocol_message_network.c
 * Author: jimkueneman
 *
 * Created on November 14, 2024, 5:24 AM
 */


#include "xc.h"
#include "stdio.h" // printf
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_buffer_store.h"
#include "openlcb_tx_driver.h"

void _send_duplicate_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message


    if (openlcb_node->state.duplicate_id_detected) { // Already handled this once

        openlcb_node->state.openlcb_msg_handled = TRUE; // Done with the message

        return;

    }

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_PC_EVENT_REPORT,
            8
            );
    Utilities_copy_event_id_to_openlcb_payload(worker_msg, EVENT_ID_DUPLICATE_NODE_DETECTED);

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.duplicate_id_detected = TRUE;
        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void _send_verified_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_VERIFIED_NODE_ID,
            3
            );

    Utilities_copy_node_id_to_openlcb_payload(worker_msg, openlcb_node->id);

    if (openlcb_node->parameters->protocol_support & PSI_SIMPLE)
        worker_msg->mti = MTI_VERIFIED_NODE_ID_SIMPLE;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void ProtocolMessageNetwork_handle_protocol_support_inquiry(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message

    if (!Utilities_is_message_for_node(openlcb_node, openlcb_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

        return;

    }

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_PROTOCOL_SUPPORT_REPLY,
            6
            );

    uint64_t temp = openlcb_node->parameters->protocol_support;

    if (temp > 0) 

        while ((temp & 0xFF00000000000000) == 0) 

            temp = temp << 8;

    *worker_msg->payload[0] = (uint8_t) (temp >> 56) & 0xFF;
    *worker_msg->payload[1] = (uint8_t) (temp >> 48) & 0xFF;
    *worker_msg->payload[2] = (uint8_t) (temp >> 40) & 0xFF;
    *worker_msg->payload[3] = (uint8_t) (temp >> 32) & 0xFF;
    *worker_msg->payload[4] = (uint8_t) (temp >> 24) & 0xFF;
    *worker_msg->payload[5] = (uint8_t) (temp >> 16) & 0xFF;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg))

        openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_handle_verify_node_id_global(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message


    if (openlcb_msg->payload_count > 0) {

        uint64_t node_id = Utilities_extract_node_id_from_openlcb_payload(openlcb_msg);

        if (node_id == openlcb_node->id)

            _send_verified_node_id(openlcb_node, openlcb_msg, worker_msg);

        else

            openlcb_node->state.openlcb_msg_handled = TRUE; // Done with the message  

    } else

        _send_verified_node_id(openlcb_node, openlcb_msg, worker_msg);

    return;

}

void ProtocolMessageNetwork_handle_verify_node_id_addressed(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message

    if (openlcb_node->alias == openlcb_msg->dest_alias) // Addressed always replies regardless if the payload NodeID matches or not but the alias much match

        _send_verified_node_id(openlcb_node, openlcb_msg, worker_msg);

    else

        openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_handle_verified_node_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message

    int64_t node_id = Utilities_extract_node_id_from_openlcb_payload(openlcb_msg);

    if (node_id == openlcb_node->id)

        _send_duplicate_node_id(openlcb_node, openlcb_msg, worker_msg);

    else

        openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolMessageNetwork_optional_interaction_rejected(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message


    if (!Utilities_is_message_for_node(openlcb_node, openlcb_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE; // Done with the message 

        return;

    }

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_OPTIONAL_INTERACTION_REJECTED,
            4
            );

    Utilities_copy_word_to_openlcb_payload(worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI, 0);
    Utilities_copy_word_to_openlcb_payload(worker_msg, openlcb_msg->mti, 2);


    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}