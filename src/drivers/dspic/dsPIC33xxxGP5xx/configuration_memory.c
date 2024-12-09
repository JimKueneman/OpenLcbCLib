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
 * \file configuration_memory.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to write/read configuration memory.  A new supported MCU/PC will create a file that handles the 
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


#include "xc.h"
#include "stdio.h" // printf
#include "../../../openlcb/openlcb_types.h"
#include "../../25AA1024/25AA1024_driver.h"


void DriverConfigurationMemory_initialization() {
    
        // SPI1 Initialize ---------------------------------------------------------
    // -------------------------------------------------------------------------


    _TRISB7 = 0; // CLK
    _TRISB8 = 0; // SDO
    _TRISB6 = 0; // CS

    _RB7 = 0;
    _RB8 = 0;
    _RB6 = 1;

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
   

    // -------------------------------------------------------------------------

    
    
}

uint16_t DriverConfigurationMemory_read(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {
    
    
   return _25AA1024_Driver_read(address, count, buffer);

    
}

uint16_t DriverConfigurationMemory_write(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {
    
    _25AA1024_Driver_write_latch_enable();
    _25AA1024_Driver_write(address, count, buffer);
    
    while (_25AA1024_Driver_write_in_progress()) {
        
    }
    
    return count;
    
}