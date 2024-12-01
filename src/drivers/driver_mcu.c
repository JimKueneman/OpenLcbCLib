
/*
 * File:   mcu_drv.c
 * Author: Jim Kueneman
 *
 * Created on February 13, 2024, 6:01 PM
 */

// Global that things like libpic30.h use to calculate delay functions and such.
// MUST be before libpic30c.h

#define FCY 40000000UL

#include "xc.h"
#include <libpic30.h>
#include "driver_mcu.h"

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


uart_rx_callback_t McuDriver_uart_rx_callback_func;





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

    /* Wait at least 4.3 microseconds (1/230400) before sending first char */
    __delay_us(10);
    // -------------------------------------------------------------------------


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


}





