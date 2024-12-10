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
 * \file mcu.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to initialize the device.  A new supported MCU/PC will create a file that handles the 
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// Global that things like libpic30.h use to calculate delay functions and such.
// MUST be before libpic30c.h

#define FCY 40000000UL

#include "xc.h"
#include <libpic30.h>
#include "../../../openlcb/openlcb_types.h"

uart_rx_callback_t McuDriver_uart_rx_callback_func;

// Oscillator ------------------------------------------------------------------
// Fine tune to get exactly 40Mhz

#define PLLDIV_OFFSET -2


// UART ------------------------------------------------------------------------
//#define FCY 40000000UL

#define FP FCY
#define BAUDRATE 333333
#define BRGVAL_BRGH_L ((FP/BAUDRATE)/16)-1
#define BRGVAL_BRGH_H ((FP/BAUDRATE)/4)-1

#define BRG_OFFSET 0


void McuDriver_Reboot(void) {
    
    asm("RESET ");
    
}

void McuDriver_initialization(void) {

    // UART Initialize ---------------------------------------------------------
    // -------------------------------------------------------------------------
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

  // ------------------------------------------------------------


    // Oscillator Initialize ---------------------------------------------------
    // -------------------------------------------------------------------------
    // Make sure the Fuse bits are set to

    //   011 = Primary Oscillator with PLL (XTPLL, HSPLL, ECPLL)

    // Setting output frequency to 140MHz
    PLLFBDbits.PLLDIV = 60 + PLLDIV_OFFSET; // This should be 60 for 80 Mhz.  Need 80 Mhz because the CAN module is limited to Fcy = 40 Mhz
    CLKDIV = 0x0001; // PreScaler divide by 3; Post Scaler divide by 2

    // PLLFBDbits.PLLDIV = 68;             // PLL multiplier M=68 140Mh
    // CLKDIV = 0x0000;         // PLL prescaler N1=2, PLL postscaler N2=2
    // -------------------------------------------------------------------------


    // IO Pin Initialize -------------------------------------------------------
    // -------------------------------------------------------------------------

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    // -------------------------------------------------------------------------


    // Peripheral Pin Select Initialize ----------------------------------------
    // -------------------------------------------------------------------------

    // Make sure PPS Multiple reconfigurations is selected in the Configuration Fuse Bits

    // CAN Pins
    RPINR26bits.C1RXR = 45; // RPI45 CAN RX
    RPOR4bits.RP43R = _RPOUT_C1TX; // RP43 CAN TX

    // UART Pins
    RPINR18bits.U1RXR = 44; // RPI44 UART RX
    RPOR4bits.RP42R = _RPOUT_U1TX; // RP42  UART TX
    // -------------------------------------------------------------------------


     /* Wait at least 4.3 microseconds (1/230400) before sending first char */
    __delay_us(10);
    // -------------
}


void __attribute__((interrupt(no_auto_psv))) _U1TXInterrupt(void) {

    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag  

    return;
}

// UART1 Receive Interrupt

void __attribute__((interrupt(no_auto_psv))) _U1RXInterrupt(void) {

    IFS0bits.U1RXIF = 0; // Clear RX Interrupt flag 

    if (U1STAbits.URXDA == 1) {

        if (McuDriver_uart_rx_callback_func)
            McuDriver_uart_rx_callback_func(U1RXREG);

    }
    return;
}
