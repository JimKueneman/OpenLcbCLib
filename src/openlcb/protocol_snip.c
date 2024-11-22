/*
 * File:   protocol_snip.c
 * Author: jimkueneman
 *
 * Created on November 14, 2024, 5:32 AM
 */

/*
 * NOTE:  All Function for all Protocols expect that the incoming CAN messages have been
 *        blocked so there is not a race on the incoming message buffer.
 */


#include "xc.h"
#include "stdio.h" // printf
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_buffer_store.h"
#include "openlcb_tx_driver.h"

void _load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index) {

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.name[string_index] != 0x00) {

        if (string_index < LEN_SNIP_NAME) {

            *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.name[string_index];
            *payload_index = *payload_index + 1;
            string_index++;

        }

    }

    *worker_msg->payload[*payload_index] = 0x00;
    *payload_index = *payload_index + 1;


}

void _load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index) {

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.model[string_index] != 0x00) {

        if (string_index < LEN_SNIP_MODEL) {

            *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.model[string_index];
            *payload_index = *payload_index + 1;
            string_index++;

        }

    }

    *worker_msg->payload[*payload_index] = 0x00;
    *payload_index = *payload_index + 1;

}

void _load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index) {

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.hardware_version[string_index] != 0x00) {

        if (string_index < LEN_SNIP_HARDWARE_VERSION) {

            *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.hardware_version[string_index];
            *payload_index = *payload_index + 1;
            string_index++;

        }

    }

    *worker_msg->payload[*payload_index] = 0x00;
    *payload_index = *payload_index + 1;

}

void _load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index) {

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.software_version[string_index] != 0x00) {

        if (string_index < LEN_SNIP_SOFTWARE_VERSION) {


            *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.software_version[string_index];
            *payload_index = *payload_index + 1;
            string_index++;

        }

    }

    *worker_msg->payload[*payload_index] = 0x00;
    *payload_index = *payload_index + 1;

}

void _load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index) {

    // TODO: Interface with the mcu_driver to access configuration memory for these use spaces

    *worker_msg->payload[*payload_index] = 0x00;
    *payload_index = *payload_index + 1;

}

void _load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index) {

    // TODO: Interface with the mcu_driver to access configuration memory for these use spaces

    *worker_msg->payload[*payload_index] = 0x00;
    *payload_index = *payload_index + 1;

}

void ProtocolSnip_handle_simple_node_info_request(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message


    if (openlcb_node->alias != openlcb_msg->dest_alias)
        return; // not for us

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_SIMPLE_NODE_INFO_REPLY,
            0
            );

    uint16_t payload_index = 0;

    *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.mfg_version;
    payload_index++;

    _load_name(openlcb_node, worker_msg, &payload_index);
    _load_model(openlcb_node, worker_msg, &payload_index);
    _load_hardware_version(openlcb_node, worker_msg, &payload_index);
    _load_software_version(openlcb_node, worker_msg, &payload_index);

    *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.user_version;
    payload_index++;

    _load_user_name(openlcb_node, worker_msg, &payload_index);
    _load_user_description(openlcb_node, worker_msg, &payload_index);


    worker_msg->payload_count = payload_index;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.duplicate_id_detected = TRUE;
        openlcb_node->state.openlcb_msg_handled = TRUE;

    }


}

