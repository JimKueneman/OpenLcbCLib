
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
 * \file io_pinout.h
 *
 *  Assigns the IO to the dspic33EP512GP504
 *
 * @author Jim Kueneman
 * @date 3 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __IO_PINOUT__
#define	__IO_PINOUT__

#include <xc.h> // include processor files - each processor file is guarded.  


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

#define SPI_TX_BUFFER_EMPTY_FLAG _SPITBF
#define SPI_RX_BUFFER_EMPTY_FLAG _SPIRBF
#define SPI_BUFFER SPI1BUF

#define EEPROM_CS _RC2
#define EEPROM_CS_TRIS _TRISC2
#define EEPROM_HOLD _RC1
#define EEPROM_HOLD_TRIS _TRISC1

#define PORT_EXPANDER_CS _RC4
#define PORT_EXPANDER_CS_TRIS _TRISC4 
#define PORT_EXPANDER_RESET _RB4
#define PORT_EXPANDER_RESET_TRIS _TRISB4

#define LED _RB9
#define LED_TRIS _TRISB9

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void IO_PINOUT_intialize(void);

#endif	/* __IO_PINOUT__ */

