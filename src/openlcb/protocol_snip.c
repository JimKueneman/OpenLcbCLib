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
#include "../drivers/driver_configuration_memory.h"

uint16_t _load_null(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index) {

    *worker_msg->payload[payload_index] = 0x00;

    return payload_index + 1;

}

uint16_t ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint8_t data_count) {

    if (data_count == 0)
        return 0;

    *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.mfg_version;

    return payload_index + 1;

}

uint16_t ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint8_t data_count) {

    if (data_count > LEN_SNIP_NAME - 1)
        data_count = LEN_SNIP_NAME - 1;

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.name[string_index] != 0x00) {

        if (string_index < data_count) {

            *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.name[string_index];
            payload_index++;
            string_index++;

        }

    }

    payload_index = _load_null(openlcb_node, worker_msg, payload_index);

    return payload_index;

}

uint16_t ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint8_t data_count) {

    if (data_count > LEN_SNIP_MODEL - 1)
        data_count = LEN_SNIP_MODEL - 1;

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.model[string_index] != 0x00) {

        if (string_index < data_count) {

            *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.model[string_index];
            payload_index++;
            string_index++;

        }

    }

    payload_index = _load_null(openlcb_node, worker_msg, payload_index);

    return payload_index;

}

uint16_t ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint8_t data_count) {

    if (data_count > LEN_SNIP_HARDWARE_VERSION - 1)
        data_count = LEN_SNIP_HARDWARE_VERSION - 1;

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.hardware_version[string_index] != 0x00) {

        if (string_index < data_count) {

            *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.hardware_version[string_index];
            payload_index++;
            string_index++;

        }

    }

    payload_index = _load_null(openlcb_node, worker_msg, payload_index);

    return payload_index;

}

uint16_t ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint8_t data_count) {

    if (data_count > LEN_SNIP_SOFTWARE_VERSION - 1)
        data_count = LEN_SNIP_SOFTWARE_VERSION - 1;

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.software_version[string_index] != 0x00) {

        if (string_index < data_count) {

            *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.software_version[string_index];
            payload_index++;
            string_index++;

        }

    }

    payload_index = _load_null(openlcb_node, worker_msg, payload_index);

    return payload_index;

}

uint16_t ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint8_t data_count) {

    if (data_count == 0)
        return 0;

    *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.user_version;

    return payload_index + 1;

}

uint16_t ProtocolSnip_load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint8_t data_count) {

    if (data_count > LEN_SNIP_USER_NAME - 1)
        data_count = LEN_SNIP_USER_NAME - 1;


    uint32_t data_address = 0; // User Name is always the first 63 Bytes in the Configuration Space
    if (openlcb_node->parameters->address_space_config_memory.low_address_valid)
        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    data_count = DriverConfigurationMemory_read(data_address, data_count, (DriverConfigurationMemory_buffer_t*) (&worker_msg->payload[payload_index]));

    uint8_t original_payload_index = payload_index;
  
    while ( (*worker_msg->payload[payload_index] != 0x00)  ) {

        if ( (payload_index - original_payload_index) >=  LEN_SNIP_USER_NAME) {
            
          *worker_msg->payload[payload_index] = 0x00;
          break;
          
        }
        
        payload_index++;
    }

    return payload_index + 1; // add the null;

}

uint16_t ProtocolSnip_load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint8_t data_count) {

    if (data_count > LEN_SNIP_USER_DESCRIPTION - 1)
        data_count = LEN_SNIP_USER_DESCRIPTION - 1;

    uint32_t data_address = LEN_SNIP_USER_NAME; // User Name is always the first 64 Bytes in the Configuration Space and Description next 64 bytes
    if (openlcb_node->parameters->address_space_config_memory.low_address_valid)
        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    data_count = DriverConfigurationMemory_read(data_address, data_count, (DriverConfigurationMemory_buffer_t*) (&worker_msg->payload[payload_index]));

    uint8_t original_payload_index = payload_index;
  
    while ( (*worker_msg->payload[payload_index] != 0x00)  ) {

        if ( (payload_index - original_payload_index) >=  LEN_SNIP_USER_DESCRIPTION) {
            
          *worker_msg->payload[payload_index] = 0x00;
          break;
          
        }
        
        payload_index++;
    }

    return payload_index + 1; // add the null;

}

void ProtocolSnip_handle_simple_node_info_request(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t data_count) {

    if (openlcb_node->state.openlcb_msg_handled)
        return; // finished with the message

    if (!Utilities_is_message_for_node(openlcb_node, openlcb_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

        return;
    }
  

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_SIMPLE_NODE_INFO_REPLY,0);

    uint16_t payload_index = 0;

    payload_index = ProtocolSnip_load_manufacturer_version_id(openlcb_node, worker_msg, payload_index, 1);

    payload_index = ProtocolSnip_load_name(openlcb_node, worker_msg, payload_index, LEN_SNIP_NAME);

    payload_index = ProtocolSnip_load_model(openlcb_node, worker_msg, payload_index, LEN_SNIP_MODEL);

    payload_index = ProtocolSnip_load_hardware_version(openlcb_node, worker_msg, payload_index, LEN_SNIP_HARDWARE_VERSION);

    payload_index = ProtocolSnip_load_software_version(openlcb_node, worker_msg, payload_index, LEN_SNIP_SOFTWARE_VERSION);

    payload_index = ProtocolSnip_load_user_version_id(openlcb_node, worker_msg, payload_index, 1);

    payload_index = ProtocolSnip_load_user_name(openlcb_node, worker_msg, payload_index, LEN_SNIP_USER_NAME);

    payload_index = ProtocolSnip_load_user_description(openlcb_node, worker_msg, payload_index, LEN_SNIP_USER_DESCRIPTION);

    worker_msg->payload_count = payload_index;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }


}

