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

#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../drivers/common/../driver_mcu.h"
#include "../../../drivers/driver_can.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "../../../openlcb/application_callbacks.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_tx_driver.h"
#include "../../../openlcb/protocol_datagram.h"
#include "turnoutboss_bootloader_ecan1_helper.h"
#include "turnoutboss_bootloader_drivers.h"
#include "turnoutboss_bootloader_uart_handler.h"
#include "turnoutboss_bootloader_traps.h"

#include "turnoutboss_bootloader_node_parameters.h"
#include "../TurnoutBossCommon/common_debug_helper.h"
#include "../TurnoutBossCommon/common_loader_app.h"

#include "turnoutboss_bootloader_hex_file_statemachine.h"
#include "mcc_generated_files/memory/flash.h"


uint8_olcb_t _start_app = FALSE;

node_id_t _extract_node_id(void) {

    return (0x0507010100AA);

}

uint16_olcb_t _config_mem_write_callback(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    for (int i = 0; i < count; i++) {

        if (!TurnoutbossBootloaderHexFileStateMachine_run((*buffer)[i])) {

            return ERROR_TEMPORARY_TRANSFER_ERROR; // RETURN AN ERROR CODE HERE TO SEND A WRITE FAIL MESSAGE

        }

    }

    return count;
}


void _config_memory_freeze_bootloader_callback(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    Utilities_load_openlcb_message(worker_msg, openlcb_node->alias, openlcb_node->id, 0, 0, MTI_INITIALIZATION_COMPLETE, 6);

    if (openlcb_node->parameters->protocol_support & PSI_SIMPLE)
        worker_msg->mti = MTI_INITIALIZATION_COMPLETE_SIMPLE;

    Utilities_copy_node_id_to_openlcb_payload(worker_msg, openlcb_node->id, 0);

    ProtocolDatagram_try_transmit(openlcb_node, openlcb_msg, worker_msg);

    if (openlcb_node->state.openlcb_msg_handled)

        openlcb_node->state.firmware_upgrade = TRUE;

}

void _config_memory_unfreeze_bootloader_callback(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    if (openlcb_node->state.openlcb_msg_handled) {

        openlcb_node->state.firmware_upgrade = FALSE;

        _start_app = TRUE;

    }

}

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

uint8_olcb_t _is_application_valid(void) {

    return (FLASH_ReadWord24(APPLICATION_START_ADDRESS) != 0x00FFFFFF);

}

void _initialize(void) {

    CommonLoaderApp_app_running = FALSE;

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
            &TurnoutBossBootloaderDrivers_pause_100ms_timer,
            &TurnoutBossBootloaderDrivers_resume_100ms_timer
            );

    TurnoutBossBootloaderDrivers_assign_uart_rx_callback(&UartHandler_handle_rx);

}

void _enter_bootloader_mode(uint16_olcb_t node_alias) {
    
    // Already Initialized with _initialize()

    ApplicationCallbacks_set_alias_change(&_alias_change_callback);
    ApplicationCallbacks_set_config_mem_unfreeze_firmware_update(&_config_memory_unfreeze_bootloader_callback);
    ApplicationCallbacks_set_config_mem_freeze_firmware_update(&_config_memory_freeze_bootloader_callback);

    openlcb_node_t * openlcb_node = Node_allocate(_extract_node_id(), &NodeParameters_main_node);


    if (node_alias != 0) {
        
        printf("Node Alias: %04X\n", node_alias);

        openlcb_node->alias = node_alias;
        openlcb_node->state.permitted = TRUE;
        openlcb_node->state.run_state = RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE;
        openlcb_node->state.firmware_upgrade = TRUE;
        
        printf("pre-loaded the node, entering bootloader mode from the main application.\n");

    } else {

        printf("allocating node for the a fresh boot load.\n");

    }

    while (!_start_app) {

        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();

    }

    CommonLoaderApp_node_alias = openlcb_node->alias;

}

int main(void) {

    _initialize();

    printf("Bootloader Starting\n");

    _start_app = FALSE;
    _GIE = 1;

    if (RCONbits.SWR) { // Power On or Brownout Reset
        
        RCONbits.SWR = 0;
        
        if (CommonLoaderApp_node_alias == 0) {

            printf("Entering Bootloader mode: coming from running application but something wrong with node id or alias.\n");

        } else {

            printf("Entering Bootloader mode: coming from running application.\n");

        }
        
        _enter_bootloader_mode(CommonLoaderApp_node_alias);

    } else { // Software Reset back to the bootloader

        CommonLoaderApp_node_alias = 0;

        if (!_is_application_valid()) {

            printf("Entering Bootloader mode: POR/BOR reset without finding a valid application.\n");

            _enter_bootloader_mode(0x0000); // No valid application image need to get one loaded

        }

        printf("falling through to the application.\n");

    }

    
   
    __delay32(10000);
    __delay32(10000);
    __delay32(10000);
    __delay32(10000);
    __delay32(10000);

    // if we fall through to this its time to run the application

    // Create a pointer to a function at the app entry point
    void (*startApplication)() = (void*) APPLICATION_START_ADDRESS;

    RCONbits.SWR = 1; // Signal application that it is starting through the bootloader that has already logged into the network
    _GIE = 0; // Disable Interrupts

    startApplication();

}
