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
 * Test application to verify the library code
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


// DSPIC33EP64GP502 Configuration Bit Settings

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


#include "xc.h"
#include "stdio.h"  // printf
#include "string.h"
#include "stdlib.h"
#include <libpic30.h> // Delay

#include "../../../../drivers/common/can_buffer_store.h"
#include "../../../../drivers/common/can_main_statemachine.h"
#include "../../../../openlcb/openlcb_buffer_store.h"
#include "../../../../drivers/common/../driver_mcu.h"
#include "../../../../drivers/driver_can.h"
#include "../../../../drivers/driver_configuration_memory.h"
#include "../../../../openlcb/openlcb_main_statemachine.h"
#include "../../../../drivers/25AA1024/25AA1024_driver.h"
#include "../../../../openlcb/openlcb_node.h"
#include "node_parameters.h"
#include "../../../../openlcb/callback_hooks.h"


#include "debug.h"

uint64_olcb_t node_id_base = 0x050101010700;

void _uart_callback(uint16_olcb_t code) {
    
    configuration_memory_buffer_t buffer;

    switch (code) {
        case 'B':
        case 'b':

            printf("\nCan Buffers Currently Allocated: %d\n", CanBufferStore_messages_allocated());
            printf("Openlcb Buffers Currently Allocated: %d\n", BufferStore_messages_allocated());

            printf("\nMax Can Buffers Allocated: %d\n", CanBufferStore_messages_max_allocated());
            printf("Max OpenLCB Buffers Allocated: %d\n", BufferStore_messages_max_allocated());
            printf("Max CAN FIFO depth: %d\n", DriverCan_max_can_fifo_depth);

            return;

        case 'P':
        case 'p':

            if (openlcb_helper.active_msg)
                PrintOpenLcbMsg(openlcb_helper.active_msg);

            return;

        case 'C':
        case 'c':

            if (can_helper.active_msg) {

                PrintCanMsg(can_helper.active_msg);
                printf("\n");
                PrintCanFrameIdentifierName(can_helper.active_msg->identifier);

                return;

            }
        case 'N':
        case 'n':

            if (Node_get_first(0))
                PrintNode(Node_get_first(0));

            return;

        case 'L':
        case 'l':

            node_id_base++;
            Node_allocate(node_id_base, &NodeParameters_main_node);

            return;
            
        case 'W':
        case 'w':
                       
               
            DriverConfigurationMemory_read(0x00, 64, &buffer);
            
            printf("Address 0x00 (0)\n");
            for (int i = 0; i < 64; i++)
                printf("%c", buffer[i]);
            
            DriverConfigurationMemory_read(0x40, 64, &buffer);
            
            printf("Address 0x40 (64)\n");
            for (int i = 0; i < 64; i++)
                printf("%c", buffer[i]);
            
            printf("\n");
            
            return;
            
        case 'E':
        case 'e':
            
            _25AA1024_Driver_write_latch_enable();
            _25AA1024_Driver_erase_chip();
            
            return;

        case 'H':
        case 'h':

            printf("B - Print Buffer Storage state\n");
            printf("P - Print the active message in the CanHelper\n");
            printf("C - Print the active message in the OpenLcbHelper\n");
            printf("N - Print the state of the first allocated Node\n");
            printf("L - Allocate a new Node\n");
            printf("P - Print address of the current OpenLcb Buffer being processed");
            printf("C - Print address of the current CAN Buffer being processed");
            printf("E = Erase the EEPROM chip");
            printf("W - Print contents of EEPROM at 0x00 and 0x40 (User Name/Description) for Node 0");

            return;

    }

    return;

}

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

void _pin_assignment_callback(void) {

    // Peripheral Pin Select Initialize ----------------------------------------
    // Make sure PPS Multiple reconfigurations is selected in the Configuration Fuse Bits

    // CAN Pins
    RPINR26bits.C1RXR = 45; // RPI45 CAN RX
    RPOR4bits.RP43R = _RPOUT_C1TX; // RP43 CAN TX

    // UART Pins
    RPINR18bits.U1RXR = 44; // RPI44 UART RX
    RPOR4bits.RP42R = _RPOUT_U1TX; // RP42  UART TX

    // SPI1 for the 25AAxxx EEProm access are on the default SPI1 pins
    _TRISB7 = 0; // CLK
    _TRISB8 = 0; // SDO
    _TRISB6 = 0; // CS

    _RB7 = 0;
    _RB8 = 0;
    _RB6 = 1;

    IFS0bits.SPI1IF = 0; // Clear the Interrupt flag
    IEC0bits.SPI1IE = 0; // Disable the interrupt

    SPI1CON1bits.SPRE = 0b000;
    SPI1CON1bits.PPRE = 0b10;

    SPI1CON1bits.DISSCK = 0; // Internal serial clock is enabled
    SPI1CON1bits.DISSDO = 0; // SDOx pin is controlled by the module
    SPI1CON1bits.MODE16 = 0; // Communication is byte-wide (8 bits)
    SPI1CON1bits.MSTEN = 1; // Master mode enabled
    SPI1CON1bits.SMP = 0; // Input data is sampled at the middle of data output time
    SPI1CON1bits.CKE = 1; // Serial output data changes on transition from
    // Idle clock state to active clock state
    SPI1CON1bits.CKP = 0; // Idle state for clock is a low level;
    // active state is a high level
    SPI1STATbits.SPIEN = 1; // Enable SPI module


}


unsigned int __attribute__((space(prog), aligned(_FLASH_PAGE * 2))) dat[_FLASH_PAGE] = {
        0xDEAD, 0xBEEF, 0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD
    };


    _prog_addressT p;


// #define  _SIMULATOR_

int main(void) {


    _TRISB4 = 0;
    _RB4 = 0;
    
#ifdef _SIMULATOR_

    // Setup the UART to send to the console in the simulator 
    //  https://developerhelp.microchip.com/xwiki/bin/view/software-tools/xc8/simulator-console/

    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

#else
    McuDriver_uart_rx_callback_func = &_uart_callback;
    CallbackHooks_alias_change = &_alias_change_callback;

    CanMainStatemachine_initialize();
    MainStatemachine_initialize();

    McuDriver_initialization(&_pin_assignment_callback);
#endif

    printf("\n\nBooted\n");

    openlcb_node_t* node = Node_allocate(node_id_base, &NodeParameters_main_node);

    printf("Node Created\n");

    uint16_olcb_t x = node->index * LEN_SNIP_USER_DATA;

#ifdef _SIMULATOR_


#endif

   

    while (1) {

        CanMainStateMachine_run(); // Runnning a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();)

    }

}
