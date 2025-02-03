
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
 * \file bootloader_drivers.h
 *
 *
 * @author Jim Kueneman
 * @date 3 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __BOOTLOADER_DRIVERS__
#define	__BOOTLOADER_DRIVERS__


#include "../../../openlcb/openlcb_types.h"

// UART ------------------------------------------------------------------------
// with FCY = 40000000UL

#define FP FCY
#define BAUDRATE 333333
#define BRGVAL_BRGH_L ((FP/BAUDRATE)/16)-1
#define BRGVAL_BRGH_H ((FP/BAUDRATE)/4)-1

#define BRG_OFFSET 0

// Oscillator ------------------------------------------------------------------
// Fine tune to get exactly 40Mhz

#define PLLDIV_OFFSET -2

#define UART_TX _RB10
#define UART_RX _RB11
#define UART_CTS _RC9
#define UART_CTS_TRIS _TRISC9
#define RTS _RB12
#define UART_RTS_TRIS _TRISB12

#define SPI_CLK _RC3
#define SPI_CLK_TRIS _TRISC3
#define SPI_SDI _RA9
#define SPI_SDI_TRIS _TRISA9
#define SPI_SDO _RA4
#define SPI_SDO_TRIS _TRISA4

#define SPI_TX_BUFFER_EMPTY_FLAG _SPITBF
#define SPI_RX_BUFFER_EMPTY_FLAG _SPIRBF
#define SPI_BUFFER SPI1BUF

#define _25AAxxx_CS _RC2
#define _25AAxxx_CS_TRIS _TRISC2
#define _25AAxxx_HOLD _RC1
#define _25AAxxx_HOLD_TRIS _TRISC1

#define _MCP23S17_CS _RC4
#define _MCP23S17_CS_TRIS _TRISC4 
#define _MCP23S17_RESET _RB4
#define _MCP23S17_RESET_TRIS _TRISB4

#define LED _RB9
#define LED_TRIS _TRISB9

// Assign the function pointer to where the UART Rx should call back with the byte it received
// WARNING: Is in the context of the interrupt, be careful
// void func(rx_data);
typedef void (*uart_rx_callback_t) (uint16_olcb_t);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // OpenLcbCLib defined callback functions that much be defined
    
    extern void BootloaderDrivers_setup(parameterless_callback_t _100ms_timer_sink);

    extern void BootloaderDrivers_reboot(void);
    
    extern uint16_olcb_t BootloaderDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer);
    
    extern uint16_olcb_t BootloaderDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer);
    
    extern void BootloaderDrivers_pause_100ms_timer(void);
    
    extern void BootloaderDrivers_resume_100ms_timer(void);
    
    // Custom Driver functions
    
    extern void BootloaderDrivers_assign_uart_rx_callback(uart_rx_callback_t uart_rx_callback);
   

#endif	/* __BOOTLOADER_DRIVERS__ */

