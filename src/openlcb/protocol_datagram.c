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
 * \file protocol_datagram.c
 *
 * Implementation of the Openlcb Datagram Protocol.  Handlers are call from the 
 * openlcb_main_statemachine.c processing when a datagram message is being processed
 * from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "protocol_datagram.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_buffer_store.h"
#include "openlcb_tx_driver.h"
#include "protocol_snip.h"

#include "../drivers/driver_mcu.h"
#include "../drivers/driver_configuration_memory.h"
#include "application_callbacks.h"

#include "protocol_datagram_handlers.h"

void ProtocolDatagram_initialize(void) {



}


void ProtocolDatagram_handle_datagram(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (!OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg))
        return;

    switch (*openlcb_msg->payload[0]) {

        case DATAGRAM_MEMORY_CONFIGURATION: // are we 0x20?

            switch (*openlcb_msg->payload[1]) {

#ifndef SUPPORT_FIRMWARE_BOOTLOADER
                case DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6:
                    ProtocolDatagramHandlers_handle_memory_read_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6], DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6, DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6);
                    return;
                case DATAGRAM_MEMORY_READ_SPACE_FD:
                    ProtocolDatagramHandlers_handle_memory_read_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY, DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FD, DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FD);
                    return;
                case DATAGRAM_MEMORY_READ_SPACE_FE:
                    ProtocolDatagramHandlers_handle_memory_read_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL, DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FE, DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FE);
                    return;
                case DATAGRAM_MEMORY_READ_SPACE_FF:
                    ProtocolDatagramHandlers_handle_memory_read_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO, DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FF, DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FF);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6:
                    ProtocolDatagramHandlers_handle_memory_read_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FD:
                    ProtocolDatagramHandlers_handle_memory_read_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FE:
                    ProtocolDatagramHandlers_handle_memory_read_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FF:
                    ProtocolDatagramHandlers_handle_memory_read_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6:
                    ProtocolDatagramHandlers_handle_memory_read_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FD:
                    ProtocolDatagramHandlers_handle_memory_read_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FE:
                    ProtocolDatagramHandlers_handle_memory_read_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                    return;
                case DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FF:
                    ProtocolDatagramHandlers_handle_memory_read_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                    return;
#endif // SUPPORT_FIRMWARE_BOOTLOADER
                case DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6:
                    ProtocolDatagramHandlers_handle_memory_write_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6], DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
                    return;
#ifndef SUPPORT_FIRMWARE_BOOTLOADER
                case DATAGRAM_MEMORY_WRITE_SPACE_FD:
                    ProtocolDatagramHandlers_handle_memory_write_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD);
                    return;
                case DATAGRAM_MEMORY_WRITE_SPACE_FE:
                    ProtocolDatagramHandlers_handle_memory_write_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE);
                    return;
                case DATAGRAM_MEMORY_WRITE_SPACE_FF:
                    ProtocolDatagramHandlers_handle_memory_write_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6:
                    ProtocolDatagramHandlers_handle_memory_write_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD:
                    ProtocolDatagramHandlers_handle_memory_write_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE:
                    ProtocolDatagramHandlers_handle_memory_write_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF:
                    ProtocolDatagramHandlers_handle_memory_write_reply_ok_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6:
                    ProtocolDatagramHandlers_handle_memory_write_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6]);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD:
                    ProtocolDatagramHandlers_handle_memory_write_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE:
                    ProtocolDatagramHandlers_handle_memory_write_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL);
                    return;
                case DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF:
                    ProtocolDatagramHandlers_handle_memory_write_reply_fail_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO);
                    return;
                case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6:
                    ProtocolDatagramHandlers_handle_memory_write_under_mask_message(openlcb_node, openlcb_msg, worker_msg, *openlcb_msg->payload[6], DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
                    return;
                case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FD:
                    ProtocolDatagramHandlers_handle_memory_write_under_mask_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_MEMORY, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD);
                    return;
                case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FE:
                    ProtocolDatagramHandlers_handle_memory_write_under_mask_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_ALL, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE);
                    return;
                case DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FF:
                    ProtocolDatagramHandlers_handle_memory_write_under_mask_message(openlcb_node, openlcb_msg, worker_msg, ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO, DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF, DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF);
                    return;
                case DATAGRAM_MEMORY_OPTIONS_CMD:
                    ProtocolDatagramHandlers_handle_memory_options_cmd_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_OPTIONS_REPLY:
                    ProtocolDatagramHandlers_handle_memory_options_reply_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_INFO_CMD:
                    ProtocolDatagramHandlers_handle_memory_get_address_space_info_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_NOT_PRESENT:
                    ProtocolDatagramHandlers_handle_memory_get_address_space_info_reply_not_present_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_GET_ADDRESS_SPACE_REPLY_PRESENT:
                    ProtocolDatagramHandlers_handle_memory_get_address_space_info_reply_present_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_RESERVE_LOCK:
                    ProtocolDatagramHandlers_handle_memory_reserve_lock_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_GET_UNIQUE_ID:
                    ProtocolDatagramHandlers_handle_memory_get_unique_id_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
#endif
                case DATAGRAM_MEMORY_CONFIGURATION_UNFREEZE:
                    ProtocolDatagramHandlers_handle_memory_unfreeze_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_FREEZE:
                    ProtocolDatagramHandlers_handle_memory_freeze_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
#ifndef SUPPORT_FIRMWARE_BOOTLOADER
                case DATAGRAM_MEMORY_CONFIGURATION_UPDATE_COMPLETE:
                    ProtocolDatagramHandlers_handle_memory_update_complete_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_RESET_REBOOT:
                    ProtocolDatagramHandlers_handle_memory_reset_reboot_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
                case DATAGRAM_MEMORY_CONFIGURATION_FACTORY_RESET:
                    ProtocolDatagramHandlers_handle_memory_factory_reset_message(openlcb_node, openlcb_msg, worker_msg);
                    return;
#endif
                default:
                    ProtocolDatagramHandlers_send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND);
                    return;

            } // switch sub-command

        default:

            ProtocolDatagramHandlers_send_datagram_rejected_reply(openlcb_node, openlcb_msg, worker_msg, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_COMMAND);

            return;

    } // switch command

}

void Protocol_Datagram_handle_datagram_ok_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;

    if (openlcb_node->last_received_datagram) {

        OpenLcbBufferStore_free_buffer(openlcb_node->last_received_datagram);
        openlcb_node->last_received_datagram = NULL;

    }

    openlcb_node->state.resend_datagram = false;
    openlcb_node->state.openlcb_msg_handled = true;

}

void ProtocolDatagram_handle_datagram_rejected_reply(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled)
        return;


    if (OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg, 0) && ERROR_TEMPORARY == ERROR_TEMPORARY) {

        if (openlcb_node->last_received_datagram) {

            openlcb_node->state.resend_datagram = true;

        }

    } else {

        OpenLcbBufferStore_free_buffer(openlcb_node->last_received_datagram);
        openlcb_node->last_received_datagram = NULL;

    }

    openlcb_node->state.openlcb_msg_handled = true;

}

void DatagramProtocol_100ms_time_tick(void) {


}






