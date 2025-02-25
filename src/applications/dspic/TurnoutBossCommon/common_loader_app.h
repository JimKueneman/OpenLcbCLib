
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
 * \file common_loader_app.h
 *
 *
 * @author Jim Kueneman
 * @date 3 Jan 2025
 */
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef _COMMON_LOADER_APP__
#define	_COMMON_LOADER_APP__

#include <xc.h> // include processor files - each processor file is guarded.  

#include "../../../openlcb/openlcb_types.h"

#ifdef BOSS1

// 25AA1024
#define EEPROM_PAGE_SIZE_IN_BYTES 256
#define EEPROM_ADDRESS_SIZE_IN_BITS 24
#define EEPROM_SIZE_IN_BYTES 131072 
#endif

#ifdef BOSS2
#define EEPROM_PAGE_SIZE_IN_BYTES 16
#define EEPROM_ADDRESS_SIZE_IN_BITS 16
#define EEPROM_SIZE_IN_BYTES  1024 
#endif

#define NODE_ID_DEFAULT 0x050701010000
#define EOF_ADDRESS 0xFFFFFFFF

#define RESET_INSTRUCTION_SIZE 4
#define INSTRUCTION_ADDRESS_SIZE 2

#define INSTRUCTIONS_PER_ERASE_BLOCK 1024

#define DATA_START_ADDRESS 0x1000 // chip dependant

// Bootloader program code needs to be limited to not go past this in the linker file, example
#define APPLICATION_START_ADDRESS 0x00B000

// Bootloader won't write any addresses past this address as to not destroy the configuration bits
// This is the last page in the 512kB chip that contains the Configuration Bits DO NOT ERASE THIS PAGE
#define APPLICATION_END_ADDRESS 0x055800

// Where the checksum is and the addresses to calculate it are  32 bit each stored as StartAddress, EndAddress, Checksum
// 0x055800 is the end of the chip, 0x055000 is the page with the configuration bits and 0x054800 is the page before that
#define APPLICATION_CHECKSUM_ADDRESS 0x054800  

// Where the NodeID is stored in the flash, it is two 24 bit instructions at the start of a page boundry that is reserve for this purpose
#define GLOBAL_NODE_ID_ADDRESS 0x54000

#define BOOTLOADER_START_ADDRESS 0x000200
#define BOOTLOADER_END_ADDRESS APPLICATION_START_ADDRESS - INSTRUCTION_ADDRESS_SIZE

//#define VIVT_ADDRESS_OSCILLATOR_FAIL_INTERRUPT (APPLICATION_START_ADDRESS + RESET_INSTRUCTION_SIZE)                 // 0xB004
//#define VIVT_ADDRESS_ADDRESS_ERROR_INTERRUPT (VIVT_ADDRESS_OSCILLATOR_FAIL_INTERRUPT + INSTRUCTION_ADDRESS_SIZE)    // 0xB006
//#define VIVT_ADDRESS_STACK_ERROR_INTERRUPT (VIVT_ADDRESS_ADDRESS_ERROR_INTERRUPT + INSTRUCTION_ADDRESS_SIZE)        // 0xB008
//#define VIVT_ADDRESS_MATH_ERROR_INTERRUPT (VIVT_ADDRESS_STACK_ERROR_INTERRUPT + INSTRUCTION_ADDRESS_SIZE)           // 0xB00A
//#define VIVT_ADDRESS_DMAC_ERROR_INTERRUPT (VIVT_ADDRESS_MATH_ERROR_INTERRUPT + INSTRUCTION_ADDRESS_SIZE)            // 0xB00C
//#define VIVT_ADDRESS_T2_INTERRUPT (VIVT_ADDRESS_DMAC_ERROR_INTERRUPT + INSTRUCTION_ADDRESS_SIZE)                    // 0xB00E
//#define VIVT_ADDRESS_U1_RX_INTERRUPT (VIVT_ADDRESS_T2_INTERRUPT + INSTRUCTION_ADDRESS_SIZE)                         // 0xB010
//#define VIVT_ADDRESS_U1_TX_INTERRUPT (VIVT_ADDRESS_U1_RX_INTERRUPT + INSTRUCTION_ADDRESS_SIZE)                      // 0xB012
//#define VIVT_ADDRESS_C1_INTERRUPT (VIVT_ADDRESS_U1_TX_INTERRUPT + INSTRUCTION_ADDRESS_SIZE)                         // 0xB014


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


#define OCCUPANCY_DETECT_GAIN_1_CS_PIN _RB14
#define OCCUPANCY_DETECT_GAIN_1_CS_TRIS _TRISB14
#define OCCUPANCY_DETECT_GAIN_2_CS_PIN _RA1
#define OCCUPANCY_DETECT_GAIN_2_CS_TRIS _TRISA1
#define OCCUPANCY_DETECT_GAIN_3_CS_PIN _RB1
#define OCCUPANCY_DETECT_GAIN_3_CS_TRIS _TRISB1
#define OCCUPANCY_DETECT_GAIN_PIN _RB15
#define OCCUPANCY_DETECT_GAIN_TRIS _TRISB15
#define OCCUPANCY_DETECT_1_PIN !_RA7 // hardware logic inverted
#define OCCUPANCY_DETECT_1_TRIS _TRISA7
#define OCCUPANCY_DETECT_2_PIN !_RA0 // hardware logic inverted
#define OCCUPANCY_DETECT_2_TRIS _TRISA0
#define OCCUPANCY_DETECT_3_PIN !_RB0 // hardware logic inverted
#define OCCUPANCY_DETECT_3_TRIS _TRISB0

#define TURNOUT_POSITION_NORMAL_PIN !_RB13 // hardware logic inverted
#define TURNOUT_POSITION_STRAIGHT_TRIS _TRISB13
#define TURNOUT_POSITION_DIVERGING_PIN !_RA10 // hardware logic inverted
#define TURNOUT_POSITION_DIVERGING_TRIS _TRISA10

#define TURNOUT_DRIVER_PIN _RC6
#define TURNOUT_DRIVER_TRIS _TRISC6

#define TURNOUT_PUSHBUTTON_NORMAL_PIN !_RC7 // hardware logic inverted
#define TURNOUT_PUSHBUTTON_STRAIGHT_TRIS _TRISC7
#define TURNOUT_PUSHBUTTON_DIVERGING_PIN !_RC8 // hardware logic inverted
#define TURNOUT_PUSHBUTTON_DIVERGING_TRIS _TRISC8

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

#ifdef BOSS1
#define LED _RB9
#define LED_TRIS _TRISB9
#endif

#ifdef BOSS2

#define LED_BLUE_TRIS _TRISB9
#define LED_BLUE _RB9

#define LED_YELLOW_TRIS _TRISA8
#define LED_YELLOW _RA8

#define LED_GREEN_TRIS _TRISC0
#define LED_GREEN _RC0

#endif


// ECAN 80 Mhz oscillator
// Make sure FCY is defined in the compiler macros and set to 40000000UL (80Mhz/2)

#define   ECAN_SWJ 2-1
#define   ECAN_BRP 15
// These are 0 indexed so need to subtract one from the value in the ECAN Bit Rate Calculator Tool
#define   ECAN_PROP_SEG 3-1  
#define   ECAN_PHASESEG_1 3-1
#define   ECAN_PHASESEG_2 3-1 
#define   ECAN_TRIPLE_SAMPLE 1
#define   ECAN_PHASESEG_2_PROGRAMMAGLE 1

/* CAN Message Buffer Configuration */
#define ECAN1_MSG_BUF_LENGTH   32
#define ECAN1_MSG_LENGTH_BYTES 8
#define ECAN1_FIFO_LENGTH_BYTES (ECAN1_MSG_BUF_LENGTH * ECAN1_MSG_LENGTH_BYTES * 2)

#define MAX_CAN_FIFO_BUFFER  31
#define MIN_CAN_FIFO_BUFFER  8


typedef struct {
    
    parameterless_callback_t oscillatorfail_hander;
    parameterless_callback_t addresserror_hander;
    parameterless_callback_t stackerror_hander;
    parameterless_callback_t matherror_hander;
    parameterless_callback_t dmacerror_hander;
    parameterless_callback_t timer_2_hander;
    parameterless_callback_t u1_tx_hander;
    parameterless_callback_t u1_rx_hander;
    parameterless_callback_t c1_hander;
    parameterless_callback_t timer_1_hander;
    
} vivt_jumptable_t;

typedef struct {
    
    uint16_olcb_t interrupt_redirect:1;
    uint16_olcb_t started_from_app:1;
    uint16_olcb_t started_from_bootloader:1;
    uint16_olcb_t do_start: 1;
     
} bootloader_state_t;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

extern void CommonLoaderApp_initialize_sfrs(void);
extern void CommonLoaderApp_initialize_can_sfrs(void);
   
extern bootloader_state_t CommonLoaderApp_bootloader_state __attribute__((persistent address(DATA_START_ADDRESS))); // 2 bytes
extern uint16_olcb_t CommonLoaderApp_node_alias __attribute__((persistent address(DATA_START_ADDRESS + 2))); // 2 bytes
extern uint64_olcb_t CommonLoaderApp_node_id __attribute__((persistent address(DATA_START_ADDRESS + 4))); // 8 bytes
extern vivt_jumptable_t CommonLoaderApp_jumptable __attribute__((persistent address(DATA_START_ADDRESS + 12))); // 9 * 4 = 36 bytes
    
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* _COMMON_LOADER_APP__ */

