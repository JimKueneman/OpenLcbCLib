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
 * \microcap_drivers.c
 *
 *  
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

#include "microcap_drivers.h"

#include "xc.h"
#include "stdio.h"  // printf
#include "local_drivers/_25AA1024/25AA1024_driver.h"

uart_rx_callback_t _uart_rx_callback_func = (void*) 0;
parameterless_callback_t _100ms_timer_sink_func = (void*) 0;

void MicrocapDrivers_setup(parameterless_callback_t _100ms_timer_sink) {
    
    _100ms_timer_sink_func = _100ms_timer_sink;
    
    
    // IO Pin Initialize -------------------------------------------------------

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    // -------------------------------------------------------------------------
    
    // Oscillator Initialize --------------------------------------------------- 
    // Make sure the Fuse bits are set too

    //   011 = Primary Oscillator with PLL (XTPLL, HSPLL, ECPLL)

    // Setting output frequency to 160MHz
    PLLFBDbits.PLLDIV = 60 + PLLDIV_OFFSET; // This should be 60 for 80 Mhz.  Need 80 Mhz because the CAN module is limited to Fcy = 40 Mhz
    CLKDIV = 0x0001; // PreScaler divide by 3; Post Scaler divide by 2

    
    // Peripheral Pin Select Initialize ----------------------------------------
    // Make sure PPS Multiple reconfigurations is selected in the Configuration Fuse Bits

    // CAN Pins
    RPINR26bits.C1RXR = 45; // RPI45 CAN RX
    RPOR4bits.RP43R = _RPOUT_C1TX; // RP43 CAN TX

    // UART Pins
    RPINR18bits.U1RXR = 44; // RPI44 UART RX
    RPOR4bits.RP42R = _RPOUT_U1TX; // RP42  UART TX
    
    // SPI1 for the 25AAxxx EEProm access are on the default SPI1 pins
    SPI_CLK_TRIS = 0; // CLK
    SPI_SDO_TRIS = 0; // SDO
    EEPROM_CS_TRIS = 0; // CS

    SPI_CLK = 0;
    SPI_SDO = 0;
    EEPROM_CS = 1;

    // Setup the SPI 1 SFRs
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
    
     // Setup UART 1 SFRs
    
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

void MicrocapDrivers_reboot(void) {
    
    asm("RESET ");
    
}

void MicrocapDrivers_assign_uart_rx_callback(uart_rx_callback_t uart_rx_callback) {
  
    _uart_rx_callback_func = uart_rx_callback;
    
}

uint16_olcb_t MicrocapDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {
    
    return _25AA1024_Driver_read(address, count, buffer);
    
}
   
uint16_olcb_t MicrocapDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {
    

    _25AA1024_Driver_write_latch_enable();
    _25AA1024_Driver_write(address, count, buffer);

    while (_25AA1024_Driver_write_in_progress()) {

    }

    return count;
    
}

void MicrocapDrivers_pause_100ms_timer() {
  
    T2CONbits.TON = 0; // Turn off 100ms Timer
    
}
    
void MicrocapDrivers_resume_100ms_timer() {
    
    T2CONbits.TON = 1; // Turn off 100ms Timer
    
}





void __attribute__((interrupt(no_auto_psv))) _U1TXInterrupt(void) {

    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag  

    return;
}

// UART1 Receive Interrupt

void __attribute__((interrupt(no_auto_psv))) _U1RXInterrupt(void) {

    IFS0bits.U1RXIF = 0; // Clear RX Interrupt flag 

    if (U1STAbits.URXDA == 1) {

        if (_uart_rx_callback_func)
            _uart_rx_callback_func(U1RXREG);

    }
    return;
}

void __attribute__((interrupt(no_auto_psv))) _T2Interrupt(void) {

    IFS0bits.T2IF = 0; // Clear T2IF

    // Increment any timer counters assigned
    if (_100ms_timer_sink_func)
        _100ms_timer_sink_func();

    return;
}
