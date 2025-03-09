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
#include "turnoutboss_bootloader_hex_file_statemachine.h"

#define HEX_STATE_FIND_COLON 0
#define HEX_STATE_READ_ADDRESS 1
#define HEX_STATE_READ_INSTRUCTION_COUNT 2
#define HEX_STATE_READ_DATA 3
#define HEX_STATE_READ_GUID_LEN 4
#define HEX_STATE_READ_GUID 5

uint8_olcb_t state_machine_state = HEX_STATE_FIND_COLON;

char* TurnoutBossBootloaderHexFileStateMachine_key = "MustangpeakEngineeringTurnoutBoss2.0";

char temp[20] = {};
uint8_olcb_t byte_index = 0;

uint32_olcb_t start_erase_block_address = 0;
uint32_olcb_t instruction_count = 0;
uint32_olcb_t data_byte_1 = 0;
uint32_olcb_t data_byte_2 = 0;
uint8_olcb_t is_data_byte_1 = TRUE;
uint16_olcb_t running_instruction_count = 0;
uint32_olcb_t running_address = 0;
uint8_olcb_t dword_state = 0;
uint8_olcb_t key_found = FALSE;

void TurnoutbossBootloaderHexFileStateMachine_reset(void) {

    dword_state = 0;
    running_address = 0;
    running_instruction_count = 0;
    is_data_byte_1 = TRUE;
    data_byte_2 = 0;
    data_byte_1 = 0;
    instruction_count = 0;
    start_erase_block_address = 0;
    byte_index = 0;
    state_machine_state = 0;
    key_found = FALSE;

    memset(&temp, 0x00, sizeof ( temp));

}

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

uint64_olcb_t TurnoutbossBootloaderHexFileStateMachine_extract_node_id_from_flash(void) {

    uint64_olcb_t upper, lower;

    upper = FLASH_ReadWord24(GLOBAL_NODE_ID_ADDRESS);
    lower = FLASH_ReadWord24(GLOBAL_NODE_ID_ADDRESS + 2);

    //   printf("Upper: 0x%04X%04X\n", (uint16_olcb_t)(upper >> 16), (uint16_olcb_t)upper);
    //   printf("Lower: 0x%04X%04X\n", (uint16_olcb_t)(lower >> 16), (uint16_olcb_t)lower);


    //    printf("0x%04X%04X\n", (uint16_olcb_t) (FLASH_ReadWord24(GLOBAL_NODE_ID_ADDRESS) << 24), FLASH_ReadWord24(GLOBAL_NODE_ID_ADDRESS+2));

    return ( (upper << 24) | lower);

}

uint8_olcb_t TurnoutbossBootloaderHexFileStateMachine_is_valid_checksum(void) {

    // Checksum information is stored in the flash page directly before the Configuration Bits
    // It stores where to start and finish the calculation then the value it should be note this 
    // are 32 bit words so it can reach into the full 24 bit address space

    // debug printout of start of checksummed memory
    // for (int i = 0; i<=8; i++) {
    //     uint32_olcb_t address = 0x6000+i;
    //     unsigned int addrupper = (address >>16)&0xFFFF;
    //     unsigned int addrlower = address&0xFFFF;
    //     printf(" at 0x%04X%04X", addrupper, addrlower); 

    //     uint32_olcb_t content = FLASH_ReadWord24(address);
    //     unsigned int contentupper = (content >> 16) & 0xFFFF;
    //     unsigned int contentlower = content & 0xFFFF;
    //     printf(" found 0x%04X%04X\n", contentupper, contentlower); 
    // }


    uint32_olcb_t start_address = FLASH_ReadWord24(APPLICATION_CHECKSUM_ADDRESS);
    uint32_olcb_t end_address = FLASH_ReadWord24(APPLICATION_CHECKSUM_ADDRESS + 2);
    uint32_olcb_t checksum = FLASH_ReadWord24(APPLICATION_CHECKSUM_ADDRESS + 4);
    uint32_olcb_t running_checksum = 0;

    printf("Start Address: 0x%04X%04X\n", (uint16_olcb_t) (start_address >> 16), (uint16_olcb_t) start_address);
    printf("End Address: 0x%04X%04X\n", (uint16_olcb_t) (end_address >> 16), (uint16_olcb_t) end_address);
    printf("Stored Checksum : 0x%04X%04X\n", (uint16_olcb_t) (checksum >> 16), (uint16_olcb_t) checksum);

    if (start_address == end_address) return false; // fail if memory erased

    // checksum: read and sum every byte 
    while (start_address <= end_address) {

        uint32_olcb_t value = FLASH_ReadWord24(start_address);
        unsigned int byte1 = value & 0xFF;
        unsigned int byte2 = (value >> 8) & 0xFF;
        unsigned int byte3 = (value >> 16) & 0xFF;

        running_checksum = running_checksum + byte1 + byte2 + byte3;

        start_address = start_address + 2;
    }

    // We only sum 8 bits in the checksum
    if ((checksum & 0xFF) == (running_checksum & 0xFF)) {
        printf("Valid calculated checksum: 0x%04X%04X\n", (uint16_olcb_t) (running_checksum >> 16), (uint16_olcb_t) running_checksum);
    } else {
        printf("Invalid calculated checksum: 0x%04X%04X\n", (uint16_olcb_t) (running_checksum >> 16), (uint16_olcb_t) running_checksum);
    }

    return ((checksum & 0xFF) == (running_checksum & 0xFF));

}


uint8_olcb_t TurnoutbossBootloaderHexFileStateMachine_run(uint8_olcb_t next_char) {

    switch (state_machine_state) {

        case HEX_STATE_FIND_COLON:

            if (next_char == ':') {

                if (!key_found) {

                    printf("WARNING, dsHex file did not have a unique key to identify as a valid image for this board\n");

                    key_found = TRUE; // fake it

                }

                memset(&temp, 0x00, sizeof ( temp));

                state_machine_state = state_machine_state + 1;

            } else {

                if (next_char == ';') {

                    // Read in the Unique ID to make sure this file is for this application

                    state_machine_state = HEX_STATE_READ_GUID_LEN;

                }

            }

            break;

        case HEX_STATE_READ_ADDRESS:

            if (_read_dword(next_char)) {

                start_erase_block_address = strtoul((char*) &temp, NULL, 16);

                if (start_erase_block_address == EOF_ADDRESS) {
                    
                    printf("EOF Found: Checking Checksum\n");

                    if (!TurnoutbossBootloaderHexFileStateMachine_is_valid_checksum()) {

                        printf("Failed CheckSum after Loading\n");

                        return FALSE;

                    }

                    return TRUE;
 

                } else {

                    state_machine_state = state_machine_state + 1;

                }

            }

            break;

        case HEX_STATE_READ_INSTRUCTION_COUNT:

            if (_read_dword(next_char)) {

                instruction_count = strtoul((char*) &temp, NULL, 16);

                // prepare for next
                is_data_byte_1 = TRUE;
                running_instruction_count = 0;
                running_address = start_erase_block_address;

                if ((start_erase_block_address >= APPLICATION_START_ADDRESS) && (start_erase_block_address < APPLICATION_END_ADDRESS)) {


                    FLASH_Unlock(FLASH_UNLOCK_KEY);

                    if (!FLASH_ErasePage(start_erase_block_address)) {

                        if (!FLASH_ErasePage(start_erase_block_address)) { // try it twice then fail

                            printf("Erasing failed:\n");

                            return FALSE;

                        }
                    };

                    FLASH_Lock();

                }

                is_data_byte_1 = TRUE;
                state_machine_state = state_machine_state + 1;

            }

            break;

        case HEX_STATE_READ_DATA:

            if (_read_dword(next_char)) {

                if (is_data_byte_1) {

                    data_byte_1 = strtoul((char*) &temp, NULL, 16);

                    running_instruction_count = running_instruction_count + 1;
                    is_data_byte_1 = FALSE; // ping pong

                } else {

                    data_byte_2 = strtoul((char*) &temp, NULL, 16);

                    running_instruction_count = running_instruction_count + 1;
                    is_data_byte_1 = TRUE; // ping pong

                    FLASH_Unlock(FLASH_UNLOCK_KEY);

                    if (!FLASH_WriteDoubleWord24(running_address, data_byte_1, data_byte_2)) {

                        printf("Flash Write Failed");

                        return FALSE;

                    }

                    FLASH_Lock();

                    if (FLASH_ReadWord24(running_address) != data_byte_1) {

                        printf("Read-back on data_byte_1 failed\n");

                        return FALSE;

                    }

                    if (FLASH_ReadWord24(running_address + 2) != data_byte_2) {

                        printf("Read-back on data_byte_1 failed\n");

                        return FALSE;

                    }

                    running_address = running_address + 4;

                    if (running_instruction_count >= instruction_count) {

                        state_machine_state = HEX_STATE_FIND_COLON;

                    }

                }

            }

            break;

        case HEX_STATE_READ_GUID_LEN:

            if (_read_dword(next_char)) {

                instruction_count = strtoul((char*) &temp, NULL, 16);
                running_address = 0;

                printf("GUID Length: %d\n", (uint16_olcb_t) instruction_count);

                uint16_olcb_t key_len = strlen(TurnoutBossBootloaderHexFileStateMachine_key);

                if ((instruction_count < key_len) || (instruction_count > key_len)) {

                    printf("Invalid GUID\n");

                    return FALSE;

                }

                state_machine_state = state_machine_state + 1;
            }

            break;

        case HEX_STATE_READ_GUID:

            if ((char) next_char == TurnoutBossBootloaderHexFileStateMachine_key[running_address]) {

                running_address = running_address + 1;

                if (running_address >= instruction_count) {

                    printf("Valid GUID\n");

                    key_found = TRUE;

                    state_machine_state = HEX_STATE_FIND_COLON;

                }

            } else {

                printf("Invalid GUID\n");

                // Not a valid image 
                return FALSE;

            }

            break;

    }

    return TRUE;

}
