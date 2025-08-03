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
 * \file protocol_snip.c
 *
 * Implementation of the Openlcb Simple Node Identification Protocol (SNIP).
 * Handlers are call from the openlcb_main_statemachine.c processing when a 
 * datagram message is being processed from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "protocol_snip.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
//#include "openlcb_buffer_fifo.h"
//#include "openlcb_buffer_store.h"
#include "openlcb_tx_driver.h"
#include "../drivers/driver_configuration_memory.h"
#include "protocol_message_network.h"

uint16_t _load_null(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index) {

    *worker_msg->payload[payload_index] = 0x00;

    return payload_index + 1;

}

uint16_t ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes) {

    if (requested_bytes == 0) {

        return 0;

    }

    *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.mfg_version;

    return payload_index + 1;

}

uint16_t ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes) {

    if (requested_bytes > LEN_SNIP_NAME - 1) { // need a null at the end

        requested_bytes = LEN_SNIP_NAME - 1;

    }

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.name[string_index] != 0x00) {

        if (string_index < requested_bytes) {

            *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.name[string_index];
            payload_index++;
            string_index++;

        }

    }

    payload_index = _load_null(openlcb_node, worker_msg, payload_index);

    return payload_index;

}

uint16_t ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes) {

    if (requested_bytes > LEN_SNIP_MODEL - 1) { // need a null at the end

        requested_bytes = LEN_SNIP_MODEL - 1;

    }

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.model[string_index] != 0x00) {

        if (string_index < requested_bytes) {

            *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.model[string_index];
            payload_index++;
            string_index++;

        }

    }

    payload_index = _load_null(openlcb_node, worker_msg, payload_index);

    return payload_index;

}

uint16_t ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes) {

    assert(openlcb_node && worker_msg);

    if (requested_bytes > LEN_SNIP_HARDWARE_VERSION - 1) {// need a null at the end

        requested_bytes = LEN_SNIP_HARDWARE_VERSION - 1;

    }

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.hardware_version[string_index] != 0x00) {

        if (string_index < requested_bytes) {

            *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.hardware_version[string_index];
            payload_index++;
            string_index++;

        }

    }

    payload_index = _load_null(openlcb_node, worker_msg, payload_index);

    return payload_index;

}

uint16_t ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes) {

    assert(openlcb_node && worker_msg);

    if (requested_bytes > LEN_SNIP_SOFTWARE_VERSION - 1) { // need a null at the end

        requested_bytes = LEN_SNIP_SOFTWARE_VERSION - 1;

    }

    uint16_t string_index = 0;
    while (openlcb_node->parameters->snip.software_version[string_index] != 0x00) {

        if (string_index < requested_bytes) {

            *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.software_version[string_index];
            payload_index++;
            string_index++;

        }

    }

    payload_index = _load_null(openlcb_node, worker_msg, payload_index);

    return payload_index;

}

uint16_t ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes) {

    if (requested_bytes == 0) {

        return 0;

    }

    *worker_msg->payload[payload_index] = openlcb_node->parameters->snip.user_version;

    return payload_index + 1;

}

uint16_t ProtocolSnip_load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes) {

    if (requested_bytes > LEN_SNIP_USER_NAME - 1)
        requested_bytes = LEN_SNIP_USER_NAME - 1;


    uint32_t data_address = 0; // User Name is always the first 63 Bytes in the Configuration Space

    if (openlcb_node->parameters->address_space_config_memory.low_address_valid) {

        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    }

    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node); // offset for multiple nodes

    DriverConfigurationMemory_get_read_callback()(data_address, requested_bytes, (configuration_memory_buffer_t*) (&worker_msg->payload[payload_index]));

    uint16_t original_payload_index = payload_index;

    while ((*worker_msg->payload[payload_index] != 0x00)) {

        if ((payload_index - original_payload_index) >= LEN_SNIP_USER_NAME) {

            *worker_msg->payload[payload_index] = 0x00;

            break;

        }

        payload_index++;
    }

    return payload_index + 1; // add the null;

}

uint16_t ProtocolSnip_load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes) {

    if (requested_bytes > LEN_SNIP_USER_DESCRIPTION - 1) {

        requested_bytes = LEN_SNIP_USER_DESCRIPTION - 1;

    }

    uint32_t data_address = LEN_SNIP_USER_NAME; // User Name is always the first 63 Bytes in the Configuration Space and Description next 64 bytes
    if (openlcb_node->parameters->address_space_config_memory.low_address_valid)
        data_address = data_address + openlcb_node->parameters->address_space_config_memory.low_address;

    data_address = data_address + OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node); // offset for multiple nodes

    DriverConfigurationMemory_get_read_callback()(data_address, requested_bytes, (configuration_memory_buffer_t*) (&worker_msg->payload[payload_index]));

    uint16_t original_payload_index = payload_index;

    while ((*worker_msg->payload[payload_index] != 0x00)) {

        if ((payload_index - original_payload_index) >= LEN_SNIP_USER_DESCRIPTION) {

            *worker_msg->payload[payload_index] = 0x00;

            break;

        }

        payload_index++;
    }

    return payload_index + 1; // add the null;

}

void ProtocolSnip_handle_simple_node_info_request(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled) {

        return; // finished with the message

    }

    if (!OpenLcbUtilities_is_message_for_node(openlcb_node, openlcb_msg)) {

        openlcb_node->state.openlcb_msg_handled = true;

        return;
    }

    OpenLcbUtilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, openlcb_msg->source_alias, openlcb_msg->source_id, MTI_SIMPLE_NODE_INFO_REPLY, 0);

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

        openlcb_node->state.openlcb_msg_handled = true;

        if (!openlcb_node->state.resend_optional_message) // if we are currently processing a resend don't reload it

            ProtocolMessageNetwork_buffer_optional_interaction_message_for_resend(openlcb_node, openlcb_msg);

    }

}

