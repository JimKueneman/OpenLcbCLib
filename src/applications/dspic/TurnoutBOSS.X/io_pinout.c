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
 * \io_pinout.c
 *
 * Assigns the IO to the dspic33EP512GP504
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

#include "io_pinout.h"

#include "xc.h"

void IO_PINOUT_intialize(void) {
    
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
    
    EEPROM_CS_TRIS = 0;
    EEPROM_CS = 0;
    
    EEPROM_HOLD_TRIS = 0;
    EEPROM_HOLD = 1;

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
