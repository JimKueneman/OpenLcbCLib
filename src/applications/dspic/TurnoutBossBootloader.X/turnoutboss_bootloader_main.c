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
 * \file turnoutboss_bootloader_main.c
 *
 * TurnoutBoss Bootloader main
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


#include <libpic30.h>

#include "xc.h"
#include <stdio.h>  // printf
#include <string.h>
#include <stdlib.h>


#include "turnoutboss_bootloader_ecan1_helper.h"
#include "turnoutboss_bootloader_drivers.h"
#include "turnoutboss_bootloader_uart_handler.h"
#include "turnoutboss_bootloader_node_parameters.h"
#include "turnoutboss_bootloader_hex_file_statemachine.h"

#include "../TurnoutBossCommon/common_debug_helper.h"
#include "../TurnoutBossCommon/common_loader_app.h"

#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "../../../openlcb/application_callbacks.h"
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/protocol_datagram.h"

#include "mcc_generated_files/memory/flash.h"
#include "local_drivers/_25AA1024/25AA1024_driver.h"

//
// The firmware is written through the OpenLcb Configuration Memory Protocol so we hook into it here
// to pass it to the statemachine to bisect it and write it to flash

void _can_rx_callback(void) {
    
       LED_YELLOW = !LED_YELLOW;

}

void _can_tx_callback(void) {
     
        LED_BLUE = !LED_BLUE;
        
}

//

uint16_olcb_t _config_mem_write_callback(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    for (int i = 0; i < count; i++) {

        if (!TurnoutbossBootloaderHexFileStateMachine_run((*buffer)[i])) {

            // Something happened
            CommonLoaderApp_bootloader_state.update_succeeded = FALSE;

            return ERROR_TEMPORARY_TRANSFER_ERROR; // RETURN AN ERROR CODE HERE TO SEND A WRITE FAIL MESSAGE

        }

    }

    return count;
}

void _config_memory_freeze_bootloader_callback(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {
    
    // Does not get called if dropping in from the application, it got the Freeze Call then dropped into the bootloader

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, 0, 0, MTI_INITIALIZATION_COMPLETE, 6);

    if (openlcb_node->parameters->protocol_support & PSI_SIMPLE)
        worker_msg->mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    Utilities_copy_node_id_to_openlcb_payload(worker_msg, openlcb_node->id, 0);

    ProtocolDatagram_try_transmit(openlcb_node, openlcb_msg, worker_msg);

    if (openlcb_node->state.openlcb_msg_handled) {

        openlcb_node->state.firmware_upgrade_active = TRUE;

        // Lets be optimistic
        CommonLoaderApp_bootloader_state.update_succeeded = TRUE;

        TurnoutbossBootloaderHexFileStateMachine_reset();

    }

}

void _config_memory_unfreeze_bootloader_callback(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    // Make sure to wait until we know the library has responded to the caller before acting
    if (openlcb_node->state.openlcb_msg_handled) {

        openlcb_node->state.firmware_upgrade_active = FALSE;

        // Only exit and start the app if it succeeded
        if (CommonLoaderApp_bootloader_state.update_succeeded) {
            
            printf("Update Succeeded\n");
            
            CommonLoaderApp_bootloader_state.started_from_app = FALSE;
            CommonLoaderApp_bootloader_state.started_from_bootloader = FALSE;
            CommonLoaderApp_bootloader_state.interrupt_redirect = FALSE;
            CommonLoaderApp_bootloader_state.do_start = FALSE;

            asm("RESET");

        }

    }

}

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

void _initialize(void) {

    CanMainStatemachine_initialize(
            &TurnoutbossBootloader_ecan1helper_setup,
            &TurnoutbossBootloader_ecan1helper_transmit_raw_can_frame,
            &TurnoutbossBootloader_ecan1helper_is_can_tx_buffer_clear,
            &TurnoutbossBootloader_ecan1helper_pause_can_rx,
            &TurnoutbossBootloader_ecan1helper_resume_can_rx
            );
    MainStatemachine_initialize(
            &TurnoutBossBootloaderDrivers_setup,
            &TurnoutBossBootloaderDrivers_reboot,
            &TurnoutBossBootloaderDrivers_config_mem_read,
            &_config_mem_write_callback,
            &TurnoutBossBootloaderDrivers_factory_reset,
            &TurnoutBossBootloaderDrivers_pause_100ms_timer,
            &TurnoutBossBootloaderDrivers_resume_100ms_timer
            );

    TurnoutBossBootloaderDrivers_assign_uart_rx_callback(&UartHandler_handle_rx);
    ApplicationCallbacks_set_alias_change(&_alias_change_callback);
    ApplicationCallbacks_set_config_mem_unfreeze_firmware_update(&_config_memory_unfreeze_bootloader_callback);
    ApplicationCallbacks_set_config_mem_freeze_firmware_update(&_config_memory_freeze_bootloader_callback);
    ApplicationCallbacks_set_can_rx(&_can_rx_callback);
    ApplicationCallbacks_set_can_tx(&_can_tx_callback);


}

node_id_t _extract_node_id_from_eeprom(uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    if (_25AA1024_Driver_read(config_mem_address, 6, config_mem_buffer, EEPROM_ADDRESS_SIZE_IN_BITS) == 6) {

        node_id_t result = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);

        if ((result != NULL_NODE_ID) && (result != 0xFFFFFFFFFFFF)) {

            return result;

        }
    }

    printf("Node ID not found in EEPROM\n");

    return NODE_ID_DEFAULT;

}

node_id_t _extract_node_id_from_flash_or_eeprom(void) {

    configuration_memory_buffer_t config_mem_buffer;

    node_id_t result = TurnoutbossBootloaderHexFileStateMachine_extract_node_id_from_flash();

    if ((result == 0x00FFFFFFFFFFFF) || (result == 0x000000000000)) {

        printf("Node ID not found in FLASH\n");

        result = _extract_node_id_from_eeprom(NODE_ID_ADDRESS, &config_mem_buffer);

    }

    return result;

}

void _run_bootloader(uint16_olcb_t node_alias) {

    printf("Running the Bootloader mode: alias is 0x%04X.\n", node_alias);

    openlcb_node_t * openlcb_node = Node_allocate(CommonLoaderApp_node_id, &NodeParameters_main_node);

    if ((CommonLoaderApp_bootloader_state.started_from_app) & (CommonLoaderApp_node_alias != 0)) {

        openlcb_node->alias = CommonLoaderApp_node_alias;
        openlcb_node->state.permitted = TRUE;
        openlcb_node->state.run_state = RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE;
        openlcb_node->state.firmware_upgrade_active = TRUE;

        printf("pre-loaded the node, entering bootloader mode from the main application.\n");

    } else {

        printf("allocating node for the a fresh boot load.\n");

    }

    while (!CommonLoaderApp_bootloader_state.do_start) {

        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();

    }

    CommonLoaderApp_node_alias = openlcb_node->alias;

    printf("Exiting the bootloader\n");

}


// The only time POR and/or BOR is set is with a true start from 0V so it is guaranteed to be the first boot.  

void _initialize_state(void) {

    if (RCONbits.POR || RCONbits.BOR) {

        memset(&CommonLoaderApp_bootloader_state, 0x0000, sizeof (CommonLoaderApp_bootloader_state));

        // Clear it so the app knows the CommonLoaderApp_bootloader_state is valid
        RCONbits.POR = 0;
        RCONbits.BOR = 0;

    }

    CommonLoaderApp_bootloader_state.update_succeeded = TRUE; // Assume the best
    CommonLoaderApp_bootloader_state.do_start = FALSE;
    CommonLoaderApp_bootloader_state.interrupt_redirect = FALSE;

}

int main(void) {

    _initialize_state();
    _initialize();
    CommonLoaderApp_node_id = _extract_node_id_from_flash_or_eeprom();

    _GIE = 1; // Enable Interrupts

    printf("Bootloader Starting\n");

    if (CommonLoaderApp_bootloader_state.started_from_app) {

        printf("Bootloader running: Started from app");

        _run_bootloader(CommonLoaderApp_node_alias);

        if (!TurnoutbossBootloaderHexFileStateMachine_is_valid_checksum()) {

            printf("Starting application but checksum is invalid, rebooting.\n");
            asm("RESET");

        }


    } else {

        CommonLoaderApp_node_alias = 0;

        if (!TurnoutbossBootloaderHexFileStateMachine_is_valid_checksum()) {

            printf("Bootloader running: Cold start no app image found");

            _run_bootloader(CommonLoaderApp_node_alias); // No valid application image need to get one loaded

        }

    }


    printf("Starting application............\n");

    _GIE = 0; // Disable Interrupts

    // Create a pointer to a function at the app entry point
    void (*startApplication)() = (void*) APPLICATION_START_ADDRESS;

    CommonLoaderApp_bootloader_state.started_from_bootloader = TRUE;
    CommonLoaderApp_bootloader_state.do_start = FALSE;
    CommonLoaderApp_bootloader_state.started_from_app = FALSE;

    startApplication();

}
