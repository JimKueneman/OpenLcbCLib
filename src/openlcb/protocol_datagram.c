/*
 * File:   protocol_datagram.c
 * Author: jimkueneman
 *
 * Created on November 15, 2024, 8:50 PM
 */


#include "xc.h"
#include "stdio.h" // printf
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_buffer_store.h"
#include "openlcb_tx_driver.h"
#include "../drivers/25AA1024/25AA1024_driver.h"


// Little use having a buffer larger than the number of Datagram messages we can create;
#define LEN_DATAGRAM_REPLY_WAITING_LIST LEN_DATAGRAM_BUFFER
#define MAX_RETRY_COUNT 3
#define RETRY_TIME_MS   300

openlcb_msg_t* reply_waiting_list[LEN_DATAGRAM_REPLY_WAITING_LIST];

void ProtocolDatagram_initialize() {

    for (int i = 0; i < sizeof (reply_waiting_list) / sizeof (openlcb_msg_t*); i++)

        reply_waiting_list[i] = (void*) 0;

}

uint8_t _address_space_enabled(openlcb_node_t* openlcb_node, uint8_t address_space) {

    switch (address_space) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:
            if (openlcb_node->parameters->address_space_configuration_definition.present)
                return TRUE;

        case ADDRESS_SPACE_ALL:
            if (openlcb_node->parameters->address_space_all.present)
                return TRUE;

        case ADDRESS_SPACE_CONFIGURATION_MEMORY:
            if (openlcb_node->parameters->address_space_config_memory.present)
                return TRUE;

        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:
            if (openlcb_node->parameters->address_space_acdi_manufacturer.present)
                return TRUE;

        case ADDRESS_SPACE_ACDI_USER_ACCESS:
            if (openlcb_node->parameters->address_space_acdi_user.present)
                return TRUE;

        case ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:
            if (openlcb_node->parameters->address_space_train_function_definition.present)
                return TRUE;

        case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:
            if (openlcb_node->parameters->address_space_train_function_config_memory.present)
                return TRUE;

    }

    return FALSE;

}

void _send_datagram(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    //  TODO: implement


}

void _send_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint16_t error_code) {

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_DATAGRAM_REJECTED_REPLY,
            2
            );
    Utilities_copy_word_to_openlcb_payload(worker_msg, error_code, 0);

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg))

        openlcb_node->state.openlcb_msg_handled = TRUE;


}

void _send_datagram_ack_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint16_t reply_pending_code) {

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_DATAGRAM_OK_REPLY,
            2
            );

    *worker_msg->payload[0] = reply_pending_code;
    worker_msg->payload_count = 1;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg))

        openlcb_node->state.openlcb_datagram_ack_sent = TRUE;


}

void _handle_memory_read(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_DATAGRAM,
            0
            );

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;

    uint32_t data_address = Utilities_extract_dword_from_openlcb_payload(openlcb_msg, 2);
    uint16_t data_count = Utilities_extract_word_from_openlcb_payload(openlcb_msg, 6);

    if (_address_space_enabled(openlcb_node, space)) {

        *worker_msg->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;


        switch (space) {

            case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:

                break;
            case ADDRESS_SPACE_ALL:

                break;
            case ADDRESS_SPACE_CONFIGURATION_MEMORY:

                _25AA1024_Driver_read(data_address, data_count, (_eeprom_read_buffer_t*) (worker_msg->payload[0]));
                break;
            case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:

                break;
            case ADDRESS_SPACE_ACDI_USER_ACCESS:

                break;
            case ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

                break;
            case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:
                _25AA1024_Driver_read(data_address, data_count, (_eeprom_read_buffer_t*) (worker_msg->payload[0]));
                break;

        }

    } else {

        *worker_msg->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;

        Utilities_copy_word_to_openlcb_payload(worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED, 7);

        worker_msg->payload_count = 8;

    }

    Utilities_copy_dword_to_openlcb_payload(worker_msg, data_address, 2);

    *worker_msg->payload[6] = space;


    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg))

        openlcb_node->state.openlcb_datagram_ack_sent = TRUE;

}

void _handle_memory_read_reply_ok(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_read_reply_fail(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_write(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {


        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }


    openlcb_node->state.openlcb_msg_handled = TRUE;


}

void _handle_memory_write_under_mask(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;


}

void _handle_memory_write_reply_ok(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_write_reply_fail(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg, uint8_t space) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_options_cmd(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }


    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_DATAGRAM,
            0
            );

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;

    *worker_msg->payload[1] = DATAGRAM_MEMORY_OPTIONS_REPLY;

    uint16_t available_commands = 0x00;

    if (openlcb_node->parameters->configuration_options.write_under_mask_supported)
        available_commands = available_commands | 0x8000;
    if (openlcb_node->parameters->configuration_options.unaligned_reads_supported)
        available_commands = available_commands | 0x4000;
    if (openlcb_node->parameters->configuration_options.unaligned_writes_supported)
        available_commands = available_commands | 0x2000;
    if (openlcb_node->parameters->configuration_options.read_from_manufacturer_space_0xfc_supported)
        available_commands = available_commands | 0x0800;
    if (openlcb_node->parameters->configuration_options.read_from_user_space_0xfb_supported)
        available_commands = available_commands | 0x0400;
    if (openlcb_node->parameters->configuration_options.write_to_user_space_0xfb_supported)
        available_commands = available_commands | 0x0200;
    if (openlcb_node->parameters->configuration_options.stream_read_write_supported)
        available_commands = available_commands | 0x0001;
    Utilities_copy_word_to_openlcb_payload(worker_msg, available_commands, 2);

    uint8_t write_lengths = 0x80 | 0x40 | 0x020 | 0x02;

    if (openlcb_node->parameters->configuration_options.stream_read_write_supported)
        write_lengths = write_lengths | 0x01;

    *worker_msg->payload[4] = write_lengths;
    *worker_msg->payload[5] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    *worker_msg->payload[6] = ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY;

    worker_msg->payload_count = 7;

    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void _handle_memory_options_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

const user_address_space_info_t* _decode_space_definition(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg) {

    switch (*openlcb_msg->payload[2]) {

        case ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO:
            return (&openlcb_node->parameters->address_space_configuration_definition);
        case ADDRESS_SPACE_ALL:
            return &openlcb_node->parameters->address_space_all;
        case ADDRESS_SPACE_CONFIGURATION_MEMORY:
            return&openlcb_node->parameters->address_space_config_memory;
        case ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS:
            return&openlcb_node->parameters->address_space_acdi_manufacturer;
        case ADDRESS_SPACE_ACDI_USER_ACCESS:
            return&openlcb_node->parameters->address_space_acdi_user;
        case ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:
            return &openlcb_node->parameters->address_space_train_function_definition;
        case ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:
            return &openlcb_node->parameters->address_space_train_function_config_memory;

        default:

            return (void*) 0;

    }

}

void _handle_memory_get_address_space_info(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t* worker_msg) {

    // TODO: CAN I MOVE ALL THESE TO THE CALLER OF ALL THESE??????

    if (!openlcb_node->state.openlcb_datagram_ack_sent) {

        _send_datagram_ack_reply(openlcb_node, openlcb_msg, worker_msg, 0);

        return;

    }


    Utilities_load_openlcb_message(
            worker_msg,
            openlcb_node->alias,
            openlcb_node->id,
            openlcb_msg->source_alias,
            openlcb_msg->source_id,
            MTI_DATAGRAM,
            0
            );

    const user_address_space_info_t* target_space = _decode_space_definition(openlcb_node, openlcb_msg);
    uint8_t invalid_space;

    if (target_space)
        invalid_space = (!target_space->present);
    else
        invalid_space = TRUE;

  
    if (invalid_space) {
        
        Utilities_clear_openlcb_message_payload(worker_msg);

        *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
        *worker_msg->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_NOT_PRESENT;
        *worker_msg->payload[2] = *openlcb_msg->payload[2];
        *worker_msg->payload[7] = 0x01;

        worker_msg->payload_count = 8;

        if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) 
            openlcb_node->state.openlcb_msg_handled = TRUE;

        return;

    }

    *worker_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *worker_msg->payload[1] = DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_PRESENT;
    *worker_msg->payload[2] = target_space->address_space;
    Utilities_copy_dword_to_openlcb_payload(worker_msg, target_space->highest_address, 3);
    *worker_msg->payload[7] = 0x00;

    if (!target_space->read_only)
        *worker_msg->payload[7] = *worker_msg->payload[7] | 0x01;

    if (target_space->low_address_valid) {

        *worker_msg->payload[7] = *worker_msg->payload[7] | 0x02;

        Utilities_copy_dword_to_openlcb_payload(worker_msg, target_space->low_address, 8);

        worker_msg->payload_count = 12;

    } else
        worker_msg->payload_count = 8;


    if (OpenLcbTxDriver_try_transmit(openlcb_node, worker_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void _handle_memory_get_address_space_info_reply_not_present(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_get_address_space_info_reply_present(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_reserve_lock(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_get_unique_id(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_unfreeze(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_freeze(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_update_complete(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_reset_reboot(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void _handle_memory_factory_reset(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {


    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolDatagram_handle_datagram(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    if (!Utilities_is_message_for_node(openlcb_node, openlcb_msg)) {

        openlcb_node->state.openlcb_msg_handled = TRUE;

        return;
    }

    for (int i = 0; i < LEN_DATAGRAM_REPLY_WAITING_LIST; i++) {

        if (!reply_waiting_list[i]) {

            // TODO: Need to try to do what is requested in the Datagram
            switch (*openlcb_msg->payload[0]) {

                case DATAGRAM_MEMORY_CONFIGURATION:


                    switch (*openlcb_msg->payload[1]) {

                        case DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6:
                            _handle_memory_read(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                            return;
                        case DATAGRAM_MEMORY_READ_SPACE_FD:
                            _handle_memory_read(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                            return;
                        case DATAGRAM_MEMORY_READ_SPACE_FE:
                            _handle_memory_read(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                            return;
                        case DATAGRAM_MEMORY_READ_SPACE_FF:
                            _handle_memory_read(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                            return;
                        case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6:
                            _handle_memory_read_reply_ok(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                            return;
                        case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FD:
                            _handle_memory_read_reply_ok(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                            return;
                        case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FE:
                            _handle_memory_read_reply_ok(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                            return;
                        case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FF:
                            _handle_memory_read_reply_ok(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                            return;
                        case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6:
                            _handle_memory_read_reply_fail(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                            return;
                        case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FD:
                            _handle_memory_read_reply_fail(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                            return;
                        case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FE:
                            _handle_memory_read_reply_fail(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                            return;
                        case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FF:
                            _handle_memory_read_reply_fail(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                            return;
                        case DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6:
                            _handle_memory_write(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                            return;
                        case DATAGRAM_MEMORY_WRITE_SPACE_FD:
                            _handle_memory_write(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                            return;
                        case DATAGRAM_MEMORY_WRITE_SPACE_FE:
                            _handle_memory_write(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                            return;
                        case DATAGRAM_MEMORY_WRITE_SPACE_FF:
                            _handle_memory_write(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                            return;
                        case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6:
                            _handle_memory_write_reply_ok(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                            return;
                        case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD:
                            _handle_memory_write_reply_ok(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                            return;
                        case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE:
                            _handle_memory_write_reply_ok(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                            return;
                        case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF:
                            _handle_memory_write_reply_ok(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                            return;
                        case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6:
                            _handle_memory_write_reply_fail(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                            return;
                        case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD:
                            _handle_memory_write_reply_fail(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                            return;
                        case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE:
                            _handle_memory_write_reply_fail(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                            return;
                        case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF:
                            _handle_memory_write_reply_fail(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                            return;
                        case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6:
                            _handle_memory_write_under_mask(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                            return;
                        case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FD:
                            _handle_memory_write_under_mask(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                            return;
                        case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FE:
                            _handle_memory_write_under_mask(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                            return;
                        case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FF:
                            _handle_memory_write_under_mask(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                            return;
                        case DATAGRAM_MEMORY_OPTIONS_CMD:
                            _handle_memory_options_cmd(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_OPTIONS_REPLY:
                            _handle_memory_options_reply(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_INFO_CMD:
                            _handle_memory_get_address_space_info(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_NOT_PRESENT:
                            _handle_memory_get_address_space_info_reply_not_present(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_PRESENT:
                            _handle_memory_get_address_space_info_reply_present(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_RESERVE_LOCK:
                            _handle_memory_reserve_lock(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_GET_UNIQUE_ID:
                            _handle_memory_get_unique_id(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_UNFREEZE:
                            _handle_memory_unfreeze(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_FREEZE:
                            _handle_memory_freeze(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_UPDATE_COMPLETE:
                            _handle_memory_update_complete(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_RESET_REBOOT:
                            _handle_memory_reset_reboot(openlcb_node, openlcb_msg, worker_msg);
                            return;
                        case DATAGRAM_MEMORY_CONFIGURATION_FACTORY_RESET:
                            _handle_memory_factory_reset(openlcb_node, openlcb_msg, worker_msg);
                            return;

                        default:

                            _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND);

                    } // switch sub-command

                default:

                    _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_COMMAND);

                    return;

            } // switch command

        } // if we find an empty waiting list slot

    } // for() looking for a waiting list slot)

    _send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_TEMPORARY_BUFFER_UNAVAILABLE);

}

void Protocol_Datagram_handle_datagram_ok_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    for (int i = 0; i < LEN_DATAGRAM_REPLY_WAITING_LIST; i++) {

        if (Utilities_is_message_for_node(openlcb_node, reply_waiting_list[i])) {

            BufferStore_freeBuffer(reply_waiting_list[i]);
            openlcb_node->state.openlcb_msg_handled = TRUE;

        }

    }

    openlcb_node->state.openlcb_msg_handled = TRUE;

}

void ProtocolDatagram_handle_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    for (int i = 0; i < LEN_DATAGRAM_REPLY_WAITING_LIST; i++) {

        if (Utilities_is_message_for_node(openlcb_node, reply_waiting_list[i])) {

            if (openlcb_msg->retry_count < MAX_RETRY_COUNT) {

                _send_datagram(openlcb_node, openlcb_msg, worker_msg);

                openlcb_msg->retry_count = openlcb_msg->retry_count + 1;

            } else {

                BufferStore_freeBuffer(reply_waiting_list[i]);
                openlcb_node->state.openlcb_msg_handled = TRUE;

            }

        }

        openlcb_node->state.openlcb_msg_handled = TRUE;

    }

}

void DatagramProtocol_100ms_time_tick() {

    //  _RB4 = 1;

    for (int i = 0; i < LEN_DATAGRAM_REPLY_WAITING_LIST; i++) {

        if (reply_waiting_list[i] != (void*) 0) {

            if (reply_waiting_list[i]-> timerticks < RETRY_TIME_MS)

                reply_waiting_list[i]-> timerticks = reply_waiting_list[i]-> timerticks + 1;

            else {

                if (reply_waiting_list[i]->retry_count < MAX_RETRY_COUNT) {


                    BufferFifo_push_existing(reply_waiting_list[i]); // Try it again

                    reply_waiting_list[i]->retry_count = reply_waiting_list[i]->retry_count + 1;

                }

                // Give up time to drop it
                reply_waiting_list[i] = (void*) 0;

                BufferStore_freeBuffer(reply_waiting_list[i]);


            }

        }


    }

    //  _RB4 = 0;


}




