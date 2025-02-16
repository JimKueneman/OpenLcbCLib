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
 * \file main.c
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
#include "turnoutboss_bootloader_ecan1_helper.h"
#include "turnoutboss_bootloader_drivers.h"
#include "debug.h"
#include "turnoutboss_bootloader_uart_handler.h"
#include "turnoutboss_bootloader_traps.h"
#include "mcc_generated_files/memory/flash.h"
#include "common_loader_app.h"

#include "turnoutboss_bootloader_node_parameters.h"


#define HEX_STATE_FIND_COLON 0

#define HEX_STATE_READ_ADDRESS_CHAR_0 1
#define HEX_STATE_READ_ADDRESS_CHAR_1 2
#define HEX_STATE_READ_ADDRESS_CHAR_2 3
#define HEX_STATE_READ_ADDRESS_CHAR_3 4
#define HEX_STATE_READ_ADDRESS_CHAR_4 5
#define HEX_STATE_READ_ADDRESS_CHAR_5 6
#define HEX_STATE_READ_ADDRESS_CHAR_6 7
#define HEX_STATE_READ_ADDRESS_CHAR_FINAL 8

#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_0 9
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_1 10
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_2 11
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_3 12
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_4 13
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_5 14
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_6 15
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_FINAL 16

#define HEX_STATE_READ_DATA_CHAR_0 17
#define HEX_STATE_READ_DATA_CHAR_1 18
#define HEX_STATE_READ_DATA_CHAR_2 19
#define HEX_STATE_READ_DATA_CHAR_3 20
#define HEX_STATE_READ_DATA_CHAR_4 21
#define HEX_STATE_READ_DATA_CHAR_5 22
#define HEX_STATE_READ_DATA_CHAR_6 23
#define HEX_STATE_READ_DATA_CHAR_FINAL 24



uint8_olcb_t state_machine_state = HEX_STATE_FIND_COLON;

char temp[16] = {};
uint8_olcb_t byte_index = 0;


uint32_olcb_t start_erase_block_address = 0;
uint32_olcb_t instruction_count = 0;
uint32_olcb_t data1 = 0;
uint32_olcb_t data2 = 0;
uint8_olcb_t is_data_1 = TRUE;
uint16_olcb_t running_instruction_count = 0;
uint32_olcb_t running_address = 0;

uint8_olcb_t _run_hex_file_state_machine(uint8_olcb_t next_char) {



    switch (state_machine_state) {

        case HEX_STATE_FIND_COLON:
        {

            if (next_char == ':') {

                memset(&temp, 0x00, sizeof ( temp));

                state_machine_state = state_machine_state + 1;

            }

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_1:
        {

            temp[1] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_2:
        {

            temp[2] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_3:
        {

            temp[3] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_4:
        {

            temp[4] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_5:
        {

            temp[5] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_6:
        {

            temp[6] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }


        case HEX_STATE_READ_ADDRESS_CHAR_FINAL:
        {

            temp[7] = next_char;

            start_erase_block_address = strtol((char*) &temp, NULL, 16);

            // prepare for next
            memset(&temp, 0x00, sizeof ( temp));
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_1:
        {

            temp[1] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_2:
        {

            temp[2] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_3:
        {

            temp[3] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_4:
        {

            temp[4] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_5:
        {

            temp[5] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_6:
        {

            temp[6] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_FINAL:
        {

            temp[7] = next_char;
            instruction_count = strtol((char*) &temp, NULL, 16);

            // prepare for next
            is_data_1 = TRUE;
            running_instruction_count = 0;
            running_address = start_erase_block_address;

            if ((start_erase_block_address >= APPLICATION_START_ADDRESS) && (start_erase_block_address < APPLICATION_END_ADDRESS))
            {
                FLASH_ErasePage(start_erase_block_address);
                
                FLASH_Unlock(FLASH_UNLOCK_KEY);
            }

            memset(&temp, 0x00, sizeof ( temp));
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_DATA_CHAR_1:
        {

            temp[1] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_2:
        {

            temp[2] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_3:
        {

            temp[3] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_4:
        {

            temp[4] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_5:
        {

            temp[5] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_6:
        {

            temp[6] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_FINAL:
        {

            temp[7] = next_char;
            if (is_data_1) {

                data1 = strtol((char*) &temp, NULL, 16);

                running_instruction_count = running_instruction_count + 1;
                is_data_1 = FALSE; // ping pong

                // Need to find the second byte, the dsHEX format ensures we have a power of 2
                state_machine_state = HEX_STATE_READ_DATA_CHAR_0;

            } else {

                data2 = strtol((char*) &temp, NULL, 16);

                running_instruction_count = running_instruction_count + 1;
                is_data_1 = TRUE; // ping pong

                FLASH_WriteDoubleWord24(running_address, data1, data2);

                if (FLASH_ReadWord24(running_address) != data1) {

                    printf("Error on Read-back data1\n");

                }

                if (FLASH_ReadWord24(running_address + 2) != data2) {

                    printf("Error on Read-back data2\n");

                }

                running_address = running_address + 4;

                if (running_instruction_count < instruction_count) {

                    state_machine_state = HEX_STATE_READ_DATA_CHAR_0;

                } else {

                    FLASH_Lock();

                    state_machine_state = HEX_STATE_FIND_COLON;

                }

            }

            // prepare for next
            memset(&temp, 0x00, sizeof ( temp));


            break;

        }
    }

    return FALSE;
}

uint16_olcb_t _config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    for (int i = 0; i < count; i++) {

        _run_hex_file_state_machine((*buffer)[i]);

    }

    return count;
}


uint64_olcb_t node_id_base = 0x0507010100AA;

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

int main(void) {

    
    CommonLoaderApp_app_running = FALSE;
    CommonLoaderApp_node_alias = 0x0000;
    CommonLoaderApp_node_id = NULL_NODE_ID;


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
            &_config_mem_write,
            &TurnoutBossBootloaderDrivers_pause_100ms_timer,
            &TurnoutBossBootloaderDrivers_resume_100ms_timer
            );

    TurnoutBossBootloaderDrivers_assign_uart_rx_callback(&UartHandler_handle_rx);

    Application_Callbacks_set_alias_change(&_alias_change_callback);
    
    

    CommonLoaderApp_u1_tx_interrupt = __builtin_tblrdl(VIVT_ADDRESS_U1_TX_INTERRUPT);
    CommonLoaderApp_u1_rx_interrupt = __builtin_tblrdl(VIVT_ADDRESS_U1_RX_INTERRUPT);
    CommonLoaderApp_c1_interrupt = __builtin_tblrdl(VIVT_ADDRESS_C1_INTERRUPT);
    CommonLoaderApp_t2_interrupt = __builtin_tblrdl(VIVT_ADDRESS_T2_INTERRUPT);
    

    printf("\nBootloader Booted\n");
    openlcb_node_t * node = Node_allocate(node_id_base, &NodeParameters_main_node);
    printf("Node Created\n");

    printf("Press 'l' to start the main app'\n");
   
   
    while (!TurnoutBossBootloaderUartHandler_start_app) {

        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();

    }
    
    // Create a pointer to a function at the app entry point
    void (*startApplication)() = (void*) APPLICATION_START_ADDRESS;
     
    _GIE = 0;    // Disable Interrupts
    CommonLoaderApp_app_running = TRUE;
    startApplication();

}
