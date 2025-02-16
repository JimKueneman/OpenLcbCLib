
/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file turnoutboss_drivers.h
 *
 *
 * @author Jim Kueneman
 * @date 3 Jan 2025
 */

#include "common_loader_app.h"

#include "xc.h"
#include <libpic30.h> // delay
#include "../../../openlcb/openlcb_defines.h"


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



 uint16_olcb_t CommonLoaderApp_app_running __attribute__((persistent address(0x1000)));  // 2 bytes
 uint16_olcb_t CommonLoaderApp_t2_interrupt __attribute__((persistent address(0x1002)));
 uint16_olcb_t CommonLoaderApp_u1_tx_interrupt __attribute__((persistent address(0x1004)));
 uint16_olcb_t CommonLoaderApp_u1_rx_interrupt __attribute__((persistent address(0x1006)));
 uint16_olcb_t CommonLoaderApp_c1_interrupt __attribute__((persistent address(0x1008)));
 uint16_olcb_t CommonLoaderApp_node_alias __attribute__((persistent address(0x100A)));
 node_id_t CommonLoaderApp_node_id __attribute__((persistent address(0x100C)));          // 8 


void CommonLoaderApp_initialize_sfrs(void) {
    
    // RB7 and RB8 are test outputs
    // we also have the LED variable for RB9 and the LED output
    _TRISB7 = 0;
    _RB7 = 0;
    _TRISB8 = 0;
    _RB8 = 0;

    // IO Pin Initialize -------------------------------------------------------

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    ANSELC = 0x00;
    // -------------------------------------------------------------------------

    // Oscillator Initialize --------------------------------------------------- 
    // Make sure the Fuse bits are set too

    //   011 = Primary Oscillator with PLL (XTPLL, HSPLL, ECPLL)

    // Setting output frequency to 160MHz
    PLLFBDbits.PLLDIV = 60 + PLLDIV_OFFSET; // This should be 60 for 80 Mhz.  Need 80 Mhz because the CAN module is limited to Fcy = 40 Mhz
    CLKDIV = 0x0001; // PreScaler divide by 3; Post Scaler divide by 2

    // Make sure PPS Multiple reconfigurations is selected in the Configuration Fuse Bits
    // CAN Pin Mapping
    RPINR26bits.C1RXR = 37; // RP37 CAN Rx (schematic naming is with respect to the MCU so this is the CAN_rx line)
    RPOR2bits.RP38R = _RPOUT_C1TX; // RP38 CAN Tx (schematic naming is with respect to the MCU so this is the CAN_tx line)

    // UART Pin Mapping
    RPINR18bits.U1RXR = 42; // RP42 UART RX (schematic naming is with respect to the FTDI cable so this is the uart_tx line)
    RPOR4bits.RP43R = _RPOUT_U1TX; // RP43  UART TX (schematic naming is with respect to the FTDI cable so this is the uart_rx line)

    _MCP23S17_CS_TRIS = 0; // Output
    _MCP23S17_CS = 0;

    _MCP23S17_RESET_TRIS = 0; // Output
    _MCP23S17_RESET = 0;
    __delay32(100); // 1us min setup and hold
    _MCP23S17_RESET = 1;

    _25AAxxx_CS_TRIS = 0; // Output
    _25AAxxx_CS = 0;

    _25AAxxx_HOLD_TRIS = 0; // Output
    _25AAxxx_HOLD = 1;

    OCCUPANCY_DETECT_GAIN_1_CS_TRIS = 0; // Output
    OCCUPANCY_DETECT_GAIN_1_CS_PIN = 1;
    __delay32(100); // strobe CS
    OCCUPANCY_DETECT_GAIN_1_CS_PIN = 0;
    __delay32(100); // 1us min setup and hold
    OCCUPANCY_DETECT_GAIN_1_CS_PIN = 1;

                    \
    OCCUPANCY_DETECT_GAIN_2_CS_TRIS = 0; // Output
    OCCUPANCY_DETECT_GAIN_2_CS_PIN = 1;
    __delay32(100); // strobe CS
    OCCUPANCY_DETECT_GAIN_2_CS_PIN = 0;
    __delay32(100); // 1us min setup and hold
    OCCUPANCY_DETECT_GAIN_2_CS_PIN = 1;

    OCCUPANCY_DETECT_GAIN_3_CS_TRIS = 0; // Output
    OCCUPANCY_DETECT_GAIN_3_CS_PIN = 1;
    __delay32(100); // strobe CS
    OCCUPANCY_DETECT_GAIN_3_CS_PIN = 0;
    __delay32(100); // 1us min setup and hold
    OCCUPANCY_DETECT_GAIN_3_CS_PIN = 1;

    OCCUPANCY_DETECT_GAIN_TRIS = 0; // Output
    OCCUPANCY_DETECT_GAIN_PIN = 0;
    OCCUPANCY_DETECT_1_TRIS = 1; // Input
    OCCUPANCY_DETECT_2_TRIS = 1; // Input
    OCCUPANCY_DETECT_3_TRIS = 1; // Input

    TURNOUT_POSITION_STRAIGHT_TRIS = 1; // Input
    TURNOUT_POSITION_DIVERGING_TRIS = 1; // Input

    TURNOUT_DRIVER_TRIS = 0; // Output
    TURNOUT_DRIVER_PIN = 0;

    TURNOUT_PUSHBUTTON_STRAIGHT_TRIS = 1; // Input
    TURNOUT_PUSHBUTTON_DIVERGING_TRIS = 1; // Input

    //UART_TX/RX auto-set by the PPS 
    UART_CTS_TRIS = 0; // Output
    UART_RTS_TRIS = 1; // Input
    UART_CTS = 0; // Set to Clear to Send (low)

    LED_TRIS = 0;
    LED = 0;

    // Using default SPI 1 pins
    SPI_CLK_TRIS = 0; // Output
    SPI_CLK = 0;
    SPI_SDO_TRIS = 0; // Output
    SPI_SDO = 0;
    SPI_SDI_TRIS = 1; // Input

    // Setup the SPI 1 SFRs

    IFS0bits.SPI1IF = 0; // Clear the Interrupt flag
    IEC0bits.SPI1IE = 0; // Disable the interrupt

    SPI1CON1bits.SPRE = 0b011; // ~8Mhz
    SPI1CON1bits.PPRE = 0b11;

    SPI1CON1bits.DISSCK = 0; // Internal serial clock is enabled
    SPI1CON1bits.DISSDO = 0; // SDOx pin is controlled by the module
    SPI1CON1bits.MODE16 = 0; // Communication is byte-wide (8 bits)
    SPI1CON1bits.MSTEN = 1; // Master mode enabled
    SPI1CON1bits.SMP = 0; // Input data is sampled at the middle of data output time
    SPI1CON1bits.CKE = 1; // Serial output data changes on transition from Idle clock state to active clock state
    SPI1CON1bits.CKP = 0; // Idle state for clock is a low level; active state is a high level
    SPI1STATbits.SPIEN = 1; // Enable SPI module

    // Setup UART 1 SFRs to 333,333 baud

    U1MODEbits.STSEL = 0; // 1-Stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8-Data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud disabled
    U1MODEbits.BRGH = 1; //Speed mode 1 = High
    U1BRG = BRGVAL_BRGH_H + BRG_OFFSET; // Baud Rate setting

    U1STAbits.UTXISEL0 = 0; // Interrupt after one TX character is transmitted
    U1STAbits.UTXISEL1 = 0;
    IEC0bits.U1TXIE = 1; // Enable UART TX interrupt


    IEC0bits.U1RXIE = 1; // Enable UART RX interrupt
    U1STAbits.URXISEL0 = 0; // Interrupt after one RX character is received;
    U1STAbits.URXISEL1 = 0;

    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

    // Setup the 100 ms timer on Timer 2

    IPC1bits.T2IP0 = 1; // Timer 2 Interrupt Priority = 5   (1 means off)
    IPC1bits.T2IP1 = 0;
    IPC1bits.T2IP2 = 1;

    T2CONbits.TCS = 0; // internal clock
    T2CONbits.TCKPS0 = 1; // 256 Prescaler
    T2CONbits.TCKPS1 = 1;
    PR2 = 15625; // Clock ticks every (1/80MHz * 2 * 256 * 15625 = 100.00091ms interrupts

    IFS0bits.T2IF = 0; // Clear T2IF
    IEC0bits.T2IE = 1; // Enable the Interrupt

    T2CONbits.TON = 1; // Turn on 100ms Timer

}


void CommonLoaderApp_initialize_can_sfrs(void) {
    
    
    
    
}