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
#include "../../../../openlcb/openlcb_main_statemachine.h"
#include "../../../../openlcb/openlcb_node.h"
#include "node_parameters.h"
#include "../../../../openlcb/callback_hooks.h"


#include "debug.h"

uint64_olcb_t node_id_base = 0x050101010700;

void _uart_callback(uint16_olcb_t code) {

    switch (code) {
        case 'B':
        case 'b':

            printf("\nCan Buffers: %d\n", CanBufferStore_messages_allocated());
            printf("\nBuffers: %d\n", BufferStore_messages_allocated());

            printf("\nMax Can Buffers: %d\n", CanBufferStore_messages_max_allocated());
            printf("\nMax Buffers: %d\n", BufferStore_messages_max_allocated());
            printf("\nMax CAN FIFO depth: %d\n", DriverCan_max_can_fifo_depth);

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
            
        case 'H':
        case 'h':
            
            printf("B - Print Buffer Storage state\n");
            printf("P - Print the active message in the CanHelper\n");
            printf("C - Print the active message in the OpenLcbHelper\n");
            printf("N - Print the state of the first allocated Node\n");
            printf("L - Allocate a new Node\n");
            
            return;

    }

    return;

}

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}


  #define  _SIMULATOR_

#include "../../../../openlcb/openlcb_gridconnect.h"
    
    gridconnect_buffer_t main_buffer;
    
    gridconnect_buffer_t* main_buffer_ptr = &main_buffer;
    
int main(void) {  
    
    
    char str[MAX_GRID_CONNECT_LEN] = ":X19170640N0501010107015555;";
    
   // for (int i = 0; i < MAX_GRID_CONNECT_LEN; i++)
   //   test[i] = str[i];
    
    printf("Buffer Address: %p\n", main_buffer);
    printf("Buffer Address: %p\n", &main_buffer);
    
    uint8_olcb_t i = 0;
    while (!OpenLcbGridConnect_copy_out_gridconnect_when_done(str[i], &main_buffer)) {
        
        i++;
         
    }
    
    
    
    printf("buffer: %s\n", (char*) &main_buffer[0]);
 
    
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
    
    McuDriver_initialization();
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
