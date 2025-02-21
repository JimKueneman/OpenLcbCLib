/*
 * File:   turnoutboss_bootloader_hex_file_statemachine.c
 * Author: jimkueneman
 *
 * Created on February 17, 2025, 6:34 AM
 */


#include <stdio.h>  // printf
#include <string.h>
#include <stdlib.h>

#include "../../../openlcb/openlcb_types.h"
#include "mcc_generated_files/memory/flash.h"
#include "../TurnoutBossCommon/common_loader_app.h"

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

uint8_olcb_t TurnoutbossBootloaderHexFileStateMachine_run(uint8_olcb_t next_char) {

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
                    
                    return FALSE;

                }

                if (FLASH_ReadWord24(running_address + 2) != data2) {

                    printf("Error on Read-back data2\n");
                    
                    return FALSE;

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

    return TRUE;
}
