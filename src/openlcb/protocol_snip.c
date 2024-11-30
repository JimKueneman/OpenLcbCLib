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

void _load_null(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index) {
    
    *worker_msg->payload[*payload_index] = 0x00;
    *payload_index = *payload_index + 1;
    
}

void ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index, uint8_t count) {

    if (count == 0)
        return;
    
    *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.mfg_version;
    
    *payload_index = *payload_index + 1;

}

void ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index, uint8_t count) {

    if (count > LEN_SNIP_NAME) 
      count = LEN_SNIP_NAME;
    
    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.name[string_index] != 0x00) {

        if (string_index < count) {

            *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.name[string_index];
            *payload_index = *payload_index + 1;
            string_index++;

        }

    }

}

void ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index, uint8_t count) {

    if (count > LEN_SNIP_MODEL) 
      count = LEN_SNIP_MODEL;
    
    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.model[string_index] != 0x00) {

        if (string_index < count) {

            *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.model[string_index];
            *payload_index = *payload_index + 1;
            string_index++;

        }

    }

}

void ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index, uint8_t count) {
    
    if (count > LEN_SNIP_HARDWARE_VERSION) 
      count = LEN_SNIP_HARDWARE_VERSION;

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.hardware_version[string_index] != 0x00) {

        if (string_index < count) {

            *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.hardware_version[string_index];
            *payload_index = *payload_index + 1;
            string_index++;

        }

    }

}

void ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index, uint8_t count) {

    if (count > LEN_SNIP_SOFTWARE_VERSION) 
      count = LEN_SNIP_SOFTWARE_VERSION;
    
    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.software_version[string_index] != 0x00) {

        if (string_index < count) {

            *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.software_version[string_index];
            *payload_index = *payload_index + 1;
            string_index++;

        }

    }

}

void ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index, uint8_t count) {

    if (count == 0)
        return;
    
    *worker_msg->payload[*payload_index] = openlcb_node->parameters->snip.user_version;
    *payload_index = *payload_index + 1;

}

void ProtocolSnip_load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index, uint8_t count) {

    // TODO: Interface with the mcu_driver to access configuration memory for these use spaces
    
    if (count > LEN_SNIP_USER_NAME) 
      count = LEN_SNIP_USER_NAME;


}

void ProtocolSnip_load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t* payload_index, uint8_t count) {

    // TODO: Interface with the mcu_driver to access configuration memory for these use spaces
    
    if (count > LEN_SNIP_USER_DESCRIPTION) 
      count = LEN_SNIP_USER_DESCRIPTION;

}

void ProtocolSnip_handle_simple_node_info_request(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t count) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message


    if (openlcb_node->alias != openlcb_msg->dest_alias) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

        return; // not for us

    }


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

    ProtocolSnip_load_manufacturer_version_id(openlcb_node, worker_msg, &payload_index, 1);
    ProtocolSnip_load_name(openlcb_node, worker_msg, &payload_index, LEN_SNIP_NAME);
    _load_null(openlcb_node, worker_msg, &payload_index);
    ProtocolSnip_load_model(openlcb_node, worker_msg, &payload_index, LEN_SNIP_MODEL);
    _load_null(openlcb_node, worker_msg, &payload_index);
    ProtocolSnip_load_hardware_version(openlcb_node, worker_msg, &payload_index, LEN_SNIP_HARDWARE_VERSION);
    _load_null(openlcb_node, worker_msg, &payload_index);
    ProtocolSnip_load_software_version(openlcb_node, worker_msg, &payload_index, LEN_SNIP_SOFTWARE_VERSION);
    _load_null(openlcb_node, worker_msg, &payload_index);

    ProtocolSnip_load_user_version_id(openlcb_node, worker_msg, &payload_index, 1);
    ProtocolSnip_load_user_name(openlcb_node, worker_msg, &payload_index, LEN_SNIP_USER_NAME);
    _load_null(openlcb_node, worker_msg, &payload_index);
    ProtocolSnip_load_user_description(openlcb_node, worker_msg, &payload_index, LEN_SNIP_USER_DESCRIPTION);
    _load_null(openlcb_node, worker_msg, &payload_index);

    worker_msg->payload_count = payload_index;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }


}

