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
#define HEX_STATE_READ_ADDRESS 1
#define HEX_STATE_READ_INSTRUCTION_COUNT 2
#define HEX_STATE_READ_DATA 3


//uint32_olcb_t buffer[1024];



uint8_olcb_t state_machine_state = HEX_STATE_FIND_COLON;

char temp[16] = {};
uint8_olcb_t byte_index = 0;


uint32_olcb_t start_erase_block_address = 0;
uint32_olcb_t instruction_count = 0;
uint32_olcb_t data_byte_1 = 0;
uint32_olcb_t data_byte_2 = 0;
uint32_olcb_t data_byte_readback_1 = 0;
uint32_olcb_t data_byte_readback_2 = 0;
//uint32_olcb_t data_byte_buffered_1 = 0;
//uint32_olcb_t data_byte_buffered_2 = 0;
uint8_olcb_t is_data_byte_1 = TRUE;
uint16_olcb_t running_instruction_count = 0;
uint32_olcb_t running_address = 0;


uint8_olcb_t dword_state = 0;

uint8_olcb_t _read_dword(uint8_olcb_t next_char) {

    uint8_olcb_t result = FALSE;

    switch (dword_state) {

        case 0:
        {

            memset(&temp, 0x00, sizeof ( temp));

            temp[0] = next_char;
            dword_state = dword_state + 1;
            break;
        }

        case 1:
        {

            temp[1] = next_char;
            dword_state = dword_state + 1;
            break;
        }

        case 2:
        {

            temp[2] = next_char;
            dword_state = dword_state + 1;
            break;
        }

        case 3:
        {

            temp[3] = next_char;
            dword_state = dword_state + 1;
            break;
        }

        case 4:
        {

            temp[4] = next_char;
            dword_state = dword_state + 1;
            break;
        }

        case 5:
        {

            temp[5] = next_char;
            dword_state = dword_state + 1;
            break;
        }

        case 6:
        {

            temp[6] = next_char;
            dword_state = dword_state + 1;
            break;
        }

        case 7:
        {

            temp[7] = next_char;
            dword_state = 0;

            result = TRUE;
            break;
        }

    }

    return result;

}

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

        case HEX_STATE_READ_ADDRESS:
        {

            if (_read_dword(next_char)) {

                start_erase_block_address = strtol((char*) &temp, NULL, 16);

                //           printf("Erase Block Address: 0x%04X%04X\n", (uint16_olcb_t)  (start_erase_block_address >> 16), (uint16_olcb_t) start_erase_block_address);

                state_machine_state = state_machine_state + 1;
            }

            break;
        }

        case HEX_STATE_READ_INSTRUCTION_COUNT:
        {

            if (_read_dword(next_char)) {

                instruction_count = strtol((char*) &temp, NULL, 16);

                //      printf("Instruction Count: 0x%04X%04X\n", (uint16_olcb_t)(instruction_count >> 16), (uint16_olcb_t) instruction_count);

                // prepare for next
                is_data_byte_1 = TRUE;
                running_instruction_count = 0;
                running_address = start_erase_block_address;



                if ((start_erase_block_address >= APPLICATION_START_ADDRESS) && (start_erase_block_address < APPLICATION_END_ADDRESS)) {
 
                    // Make a copy of this block for later use
//                    for (int i = 0; i < INSTRUCTIONS_PER_ERASE_BLOCK; i++) {
//
//                        buffer[i] = FLASH_ReadWord24(start_erase_block_address + (i * 2));
//
//                    }

                    FLASH_Unlock(FLASH_UNLOCK_KEY);

                    if (!FLASH_ErasePage(start_erase_block_address)) {

                        printf("Erasing failed:\n");

                    };

                    FLASH_Lock();

                }

                is_data_byte_1 = TRUE;
                state_machine_state = state_machine_state + 1;
            }

            break;
        }


        case HEX_STATE_READ_DATA:
        {

            if (_read_dword(next_char)) {

                if (is_data_byte_1) {

                    data_byte_1 = strtol((char*) &temp, NULL, 16);
               //     data_byte_buffered_1 = buffer[running_instruction_count];

                    running_instruction_count = running_instruction_count + 1;
                    is_data_byte_1 = FALSE; // ping pong

                } else {

                    data_byte_2 = strtol((char*) &temp, NULL, 16);
               //     data_byte_buffered_2 = buffer[running_instruction_count];
                    
                    running_instruction_count = running_instruction_count + 1;
                    is_data_byte_1 = TRUE; // ping pong

                    FLASH_Unlock(FLASH_UNLOCK_KEY);

                    if (!FLASH_WriteDoubleWord24(running_address, data_byte_1, data_byte_2)) {

                        printf("Flash Write Failed");

                    }

                    FLASH_Lock();
                    
                    data_byte_readback_1 = FLASH_ReadWord24(running_address);
                    data_byte_readback_2 = FLASH_ReadWord24(running_address + 2);
                    
//                    printf("Address 0x%04X%04X: Wrote (buffer[]): 0x%04X%04X; Wrote (data_byte_1): 0x%04X%04X; Read: 0x%04X%04X\n",   
//                                (uint16_olcb_t) (running_address >> 16), 
//                                (uint16_olcb_t) (running_address), 
//                                (uint16_olcb_t) (data_byte_buffered_1 >> 16), 
//                                (uint16_olcb_t) (data_byte_buffered_1), 
//                                (uint16_olcb_t) (data_byte_1 >> 16), 
//                                (uint16_olcb_t) data_byte_1,
//                                (uint16_olcb_t) (data_byte_readback_1 >> 16), 
//                                (uint16_olcb_t) data_byte_readback_1
//                           );
//                    
//                    printf("Address 0x%04X%04X: Wrote (buffer[]): 0x%04X%04X; Wrote (data_byte_2): 0x%04X%04X; Read: 0x%04X%04X\n",  
//                                (uint16_olcb_t) ((running_address + 2) >> 16), 
//                                (uint16_olcb_t) (running_address + 2), 
//                                (uint16_olcb_t) (data_byte_buffered_2 >> 16), 
//                                (uint16_olcb_t) (data_byte_buffered_2), 
//                                (uint16_olcb_t) (data_byte_2 >> 16), 
//                                (uint16_olcb_t) data_byte_2,
//                                (uint16_olcb_t) (data_byte_readback_2 >> 16), 
//                                (uint16_olcb_t) data_byte_readback_2
//                          );

                    if (data_byte_readback_1 != data_byte_1) {

                        printf("Read-back on data_byte_1 failed\n");

                        //      return FALSE;

                    }

                    if (data_byte_readback_2 != data_byte_2) {

                        printf("Read-back on data_byte_1 failed\n");
                        
                        //      return FALSE;

                    }

                    running_address = running_address + 4;

                    if (running_instruction_count >= instruction_count) {

                        state_machine_state = HEX_STATE_FIND_COLON;

                    }

                }

            }

            break;
        }

    }

    return TRUE;
}
