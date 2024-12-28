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

#include "../../../drivers/common/can_buffer_store.h"
#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../openlcb/openlcb_buffer_store.h"
#include "../../../drivers/common/../driver_mcu.h"
#include "../../../drivers/driver_can.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "node_parameters.h"
#include "../../../openlcb/callback_hooks.h"


#include "debug.h"

uint64_olcb_t node_id_base = 0x050101010700;


#define PORT_EXPANDER_CS _RC4
#define PORT_EXPANDER_CS_TRIS _TRISC4 
#define PORT_EXPANDER_RESET _RB4
#define PORT_EXPANDER_RESET_TRIS _TRISB4

#define TRACK_DETECT_GAIN_1_CS _RB14
#define TRACK_DETECT_GAIN_1_CS_TRIS _TRISB14
#define TRACK_DETECT_GAIN_2_CS _RA1
#define TRACK_DETECT_GAIN_2_CS_TRIS _TRISA1
#define TRACK_DETECT_GAIN_3_CS _RB1
#define TRACK_DETECT_GAIN_3_CS_TRIS _TRISB1
#define TRACK_DETECT_GAIN _RB15
#define TRACK_DETECT_GAIN_TRIS _TRISB15
#define TRACK_DETECT_1 _RA7
#define TRACK_DETECT_1_TRIS _TRISA7
#define TRACK_DETECT_2 _RA0
#define TRACK_DETECT_2_TRIS _TRISA0
#define TRACK_DETECT_3 _RB0
#define TRACK_DETECT_3_TRIS _TRISB0

#define TURNOUT_POSITION_STRAIGHT _RB13
#define TURNOUT_POSITION_STRAIGHT_TRIS _TRISB13
#define TURNOUT_POSITION_DIVERGING _RA10
#define TURNOUT_POSITION_DIVERGING_TRIS _TRISA10

#define TURNOUT_DRIVER _RC6
#define TURNOUT_DRIVER_TRIS _TRISC6

#define TURNOUT_PUSHBUTTON_STRAIGHT _RC7
#define TURNOUT_PUSHBUTTON_STRAIGHT_TRIS _TRISC7
#define TURNOUT_PUSHBUTTON_DIVERGING _RC8
#define TURNOUT_PUSHBUTTON_DIVERGING_TRIS _TRISC8

#define UART_TX _RB10
#define UART_RX _RB11
#define CTS _RC9
#define CTS_TRIS _TRISC9
#define RTS _RB12
#define RTS_TRIS _TRISB12

#define CAN_TX _RB6
#define CAN_RX _RB5

#define SPI_CLK _RC3
#define SPI_CLK_TRIS _TRISC3
#define SPI_SDI _RA9
#define SPI_SDI_TRIS _TRISA9
#define SPI_SDO _RA4
#define SPI_SDO_TRIS _TRISA4

#define LED _RB9
#define LED_TRIS _TRISB9


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

void _pin_assignment_callback(void) {
    
        // Make sure PPS Multiple reconfigurations is selected in the Configuration Fuse Bits

    // CAN Pin Mapping
    RPINR26bits.C1RXR = 38; // RP38 CAN Rx (schematic naming is with respect to the MCU so this is the CAN_rx line)
    RPOR1bits.RP37R = _RPOUT_C1TX; // RP37 CAN Tx (schematic naming is with respect to the MCU so this is the CAN_tx line)

    // UART Pin Mapping
    RPINR18bits.U1RXR = 42; // RP42 UART RX (schematic naming is with respect to the FTDI cable so this is the uart_tx line)
    RPOR4bits.RP43R = _RPOUT_U1TX; // RP43  UART TX (schematic naming is with respect to the FTDI cable so this is the uart_rx line)

    PORT_EXPANDER_CS_TRIS = 0;
    PORT_EXPANDER_CS = 0;

    PORT_EXPANDER_RESET_TRIS = 0;
    PORT_EXPANDER_RESET = 0;

    TRACK_DETECT_GAIN_1_CS_TRIS = 0;
    TRACK_DETECT_GAIN_1_CS = 0;
    TRACK_DETECT_GAIN_2_CS_TRIS = 0;
    TRACK_DETECT_GAIN_2_CS = 0;
    TRACK_DETECT_GAIN_3_CS_TRIS = 0;
    TRACK_DETECT_GAIN_3_CS = 0;
    TRACK_DETECT_GAIN_TRIS = 0;
    TRACK_DETECT_GAIN = 0;
    TRACK_DETECT_1_TRIS = 1;
    TRACK_DETECT_2_TRIS = 1;
    TRACK_DETECT_3_TRIS = 1;

    TURNOUT_POSITION_STRAIGHT_TRIS = 1;
    TURNOUT_POSITION_DIVERGING_TRIS = 1;

    TURNOUT_DRIVER_TRIS = 0;
    TURNOUT_DRIVER = 0;

    TURNOUT_PUSHBUTTON_STRAIGHT_TRIS = 1;
    TURNOUT_PUSHBUTTON_DIVERGING_TRIS = 1;

    //UART_TX auto-set by the PPS 
    //UART_RX
    CTS_TRIS = 0;
    CTS = 0;
    RTS_TRIS = 1;

    //CAN_TX auto-set by the PPS 
    //CAN_RX 
    
    LED_TRIS = 0;
    LED = 0;

    SPI_CLK_TRIS = 0;
    SPI_CLK = 0;
    SPI_SDO_TRIS = 0;
    SPI_SDO = 0;
    SPI_SDI_TRIS = 1;

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

        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();)


    }

}
