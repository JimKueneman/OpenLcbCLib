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


// DSPIC33EP512GP504 Configuration Bit Settings

// 'C' source line config statements

// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

// FPOR
#pragma config ALTI2C1 = OFF            // Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins)
#pragma config ALTI2C2 = OFF            // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
#pragma config WDTWIN = WIN25           // Watchdog Window Select bits (WDT Window is 25% of WDT period)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON              // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FOSC
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = OFF            // Peripheral pin select configuration (Allow multiple reconfigurations)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = PRIPLL           // Oscillator Source Selection (Primary Oscillator with PLL module (XT + PLL, HS + PLL, EC + PLL))
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


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
#include "../dsPIC_Common/ecan1_helper.h"
#include "turnoutboss_drivers.h"
#include "debug.h"
#include "uart_handler.h"
#include "traps.h"
#include "mcc_generated_files/memory/flash.h"

#include "node_parameters.h"


#define HEX_STATE_FIND_COLON 0
#define HEX_STATE_READ_BYTECOUNT_CHAR_0 1
#define HEX_STATE_READ_BYTECOUNT_CHAR_FINAL 2
#define HEX_STATE_READ_ADDRESS_CHAR_0 3
#define HEX_STATE_READ_ADDRESS_CHAR_1 4
#define HEX_STATE_READ_ADDRESS_CHAR_2 5
#define HEX_STATE_READ_ADDRESS_CHAR_FINAL 6
#define HEX_STATE_READ_RECORD_TYPE_CHAR_0 7
#define HEX_STATE_READ_RECORD_TYPE_CHAR_FINAL 8
#define HEX_STATE_READ_DATA_CHAR_0 9
#define HEX_STATE_READ_DATA_CHAR_FINAL 10
#define HEX_STATE_READ_CHECKSUM_CHAR_0 11
#define HEX_STATE_READ_CHECKSUM_CHAR_FINAL 12


#define RECORD_TYPE_DATA 0x00
#define RECORD_TYPE_EOF 0x01
#define RECORD_TYPE_EXTENDED_SEGMENT_ADDRESS 0x02
#define RECORD_TYPE_START_SEGMENT_ADDRESS 0x03
#define RECORD_TYPE_EXTENDED_LINEAR_ADDRESS 0x04
#define RECORD_TYPE_START_LINEAR_ADDRESS 0x05


uint8_olcb_t state_machine_state = HEX_STATE_FIND_COLON;

char temp[5] = {};
uint8_olcb_t byte_count = 0;
uint16_olcb_t base_address = 0;
uint16_olcb_t extended_address = 0;
uint8_olcb_t record_type = 0;
uint8_olcb_t data[0xFF];
uint8_olcb_t byte_index = 0;
uint8_olcb_t checksum = 0;
uint16_olcb_t running_checksum = 0;
uint32_olcb_t start_segment_adddress_80x86 = 0;
uint32_olcb_t start_linear_address = 0;

uint8_olcb_t _run_hex_file_state_machine(uint8_olcb_t next_char) {

    switch (state_machine_state) {

        case HEX_STATE_FIND_COLON:
        {

            if (next_char == ':') {

                memset(&temp, 0x00, sizeof ( temp));
                memset(&checksum, 0x00, sizeof ( checksum));
                running_checksum = 0;

                state_machine_state = HEX_STATE_READ_BYTECOUNT_CHAR_0;

            }

            break;
        }

        case HEX_STATE_READ_BYTECOUNT_CHAR_0:
        {

            temp[0] = next_char;

            state_machine_state = HEX_STATE_READ_BYTECOUNT_CHAR_FINAL;

            break;
        }

        case HEX_STATE_READ_BYTECOUNT_CHAR_FINAL:
        {

            temp[1] = next_char;
            byte_count = strtol(&temp[0], NULL, 16);

            running_checksum = byte_count;

            // prepare for next      
            memset(&temp, 0x00, sizeof ( temp));
            state_machine_state = HEX_STATE_READ_ADDRESS_CHAR_0;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = HEX_STATE_READ_ADDRESS_CHAR_1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_1:
        {

            temp[1] = next_char;
            state_machine_state = HEX_STATE_READ_ADDRESS_CHAR_2;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_2:
        {

            temp[2] = next_char;
            state_machine_state = HEX_STATE_READ_ADDRESS_CHAR_FINAL;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_FINAL:
        {

            temp[3] = next_char;
            base_address = strtol(&temp[0], NULL, 16);

            running_checksum = running_checksum + ((base_address >> 8) & 0x00FF) + (base_address & 0x00FF);

            // prepare for next
            memset(&temp, 0x00, sizeof ( temp));
            state_machine_state = HEX_STATE_READ_RECORD_TYPE_CHAR_0;

            break;
        }

        case HEX_STATE_READ_RECORD_TYPE_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = HEX_STATE_READ_RECORD_TYPE_CHAR_FINAL;

            break;
        }

        case HEX_STATE_READ_RECORD_TYPE_CHAR_FINAL:
        {

            temp[1] = next_char;
            record_type = strtol(&temp[0], NULL, 16);

            running_checksum = running_checksum + record_type;

            if (byte_count == 0) {

                state_machine_state = HEX_STATE_READ_CHECKSUM_CHAR_0; // has no data

            } else {
                // prepare for next
                byte_index = 0;
                memset(&data, 0x00, sizeof ( data));
                memset(&temp, 0x00, sizeof ( temp));
                state_machine_state = HEX_STATE_READ_DATA_CHAR_0;

            }
            
            break;


        }

        case HEX_STATE_READ_DATA_CHAR_0:
        {

            // reading these (and the checksum) one byte at a time so the terminating null will persist at the correct index

            temp[0] = next_char;

            state_machine_state = HEX_STATE_READ_DATA_CHAR_FINAL;

            break;
        }

        case HEX_STATE_READ_DATA_CHAR_FINAL:
        {

            // reading these (and the checksum) one byte at a time so the terminating null will persist at the correct index

            temp[1] = next_char;
            data[byte_index] = strtol(&temp[0], NULL, 16);
            byte_index = byte_index + 1;

            running_checksum = running_checksum + data[byte_index-1];

            if (byte_index >= byte_count) {

                memset(&temp, 0x00, sizeof ( temp));
                state_machine_state = HEX_STATE_READ_CHECKSUM_CHAR_0;

            } else {
                
                state_machine_state = HEX_STATE_READ_DATA_CHAR_0;

                
            }

            break;
        }

        case HEX_STATE_READ_CHECKSUM_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = HEX_STATE_READ_CHECKSUM_CHAR_FINAL;

            break;
        }

        case HEX_STATE_READ_CHECKSUM_CHAR_FINAL:
        {

            temp[1] = next_char;
            checksum = strtol(&temp[0], NULL, 16);
            
            if (checksum == ((~running_checksum + 1) & 0x00FF)) {

                switch (record_type) {

                    case RECORD_TYPE_DATA: // Data


                        break;

                    case RECORD_TYPE_EOF: // EOF

                        break;

                    case RECORD_TYPE_EXTENDED_SEGMENT_ADDRESS: // Extended Segment Address

                        // The data buffer was filled with nulls before the payload was read

                        start_segment_adddress_80x86 = strtol((char*) &data[0], NULL, 16) * 16;

                        break;

                    case RECORD_TYPE_START_SEGMENT_ADDRESS: // Start Segment Address

                        // for 80x86 processors.. 

                        // The data buffer was filled with nulls before the payload was read

                        start_segment_adddress_80x86 = strtol((char*) &data[0], NULL, 16);

                        break;

                    case RECORD_TYPE_EXTENDED_LINEAR_ADDRESS: // Extended Linear Address

                        // The data buffer was filled with nulls before the payload was read

                        extended_address = strtol((char*) &data[0], NULL, 16);

                        break;

                    case RECORD_TYPE_START_LINEAR_ADDRESS: // Start Linear Address

                        // The data buffer was filled with nulls before the payload was read

                        start_linear_address = strtol((char*) &data[0], NULL, 16);
                        break;

                }

            } else {


                printf("Checksum Failed\n");

            }


            state_machine_state = HEX_STATE_FIND_COLON;
            break;
        }
    }

    return FALSE;
}

uint16_olcb_t _config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    for (int i = 0; i < count; i++)
        _run_hex_file_state_machine(*buffer[i]);

    //  PrintDWord(address);

    return count;
}


uint64_olcb_t node_id_base = 0x0507010100AA;

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

int main(void) {

    char* test = ":106000002FBD23000EFF27000E01880000000000B6";

    for (int i = 0; i < strlen(test); i++)
        _run_hex_file_state_machine(test[i]);


    // RB7 and RB8 are test outputs
    // we also have the LED variable for RB9 and the LED output
    _TRISB7 = 0;
    _RB7 = 0;
    _TRISB8 = 0;
    _RB8 = 0;


    CanMainStatemachine_initialize(
            &Ecan1Helper_setup,
            &Ecan1Helper_transmit_raw_can_frame,
            &Ecan1Helper_is_can_tx_buffer_clear,
            &Ecan1Helper_pause_can_rx,
            &Ecan1Helper_resume_can_rx
            );
    MainStatemachine_initialize(
            &TurnoutBossDrivers_setup,
            &TurnoutBossDrivers_reboot,
            &TurnoutBossDrivers_config_mem_read,
            &_config_mem_write,
            &TurnoutBossDrivers_pause_100ms_timer,
            &TurnoutBossDrivers_resume_100ms_timer
            );

    TurnoutBossDrivers_assign_uart_rx_callback(&UartHandler_handle_rx);

    Application_Callbacks_set_alias_change(&_alias_change_callback);


    printf("\nBooted\n");
    openlcb_node_t* node = Node_allocate(node_id_base, &NodeParameters_main_node);
    printf("Node Created\n");


    while (1) {

        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();

    }

}
