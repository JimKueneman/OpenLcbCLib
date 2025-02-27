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
 * \file MCP23S17_driver.c
 *
 * Driver for the MicroChip 25AA1024 EEPROM.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "MCP23S17_driver.h"

#include "xc.h"
#include <libpic30.h> // delay
#include "../../../../../openlcb/openlcb_types.h"
#include "../../../TurnoutBossCommon/common_loader_app.h"

void _mcp23s17_flush_buffers(void) {

    uint8_olcb_t result;

    // Wait for any transmit to finish
    while (SPI_TX_BUFFER_EMPTY_FLAG == 1) {
    }

    // Clear the Rx Buffer
    if (SPI_RX_BUFFER_EMPTY_FLAG == 1) {

        result = SPI_BUFFER;

    }

}

uint8_olcb_t _mcp23s17_wait_for_reply(void) {

    // Wait for any transmit to finish
    while (SPI_TX_BUFFER_EMPTY_FLAG == 1) {
    }

    // Clear the Rx Buffer
    while (SPI_RX_BUFFER_EMPTY_FLAG == 0) {

    }

    return SPI_BUFFER;

}

void _access_register(uint8_olcb_t access_type, uint8_olcb_t _register, uint8_olcb_t data) {

    _mcp23s17_flush_buffers();

    _MCP23S17_CS = 0;

    SPI_BUFFER = access_type;

    _mcp23s17_wait_for_reply();

    SPI_BUFFER = _register;

    _mcp23s17_wait_for_reply();

    SPI_BUFFER = data;

    _mcp23s17_wait_for_reply();

    _MCP23S17_CS = 1;

}

void _write_register(uint8_olcb_t _register, uint8_olcb_t data) {

    _access_register(ADDRESS_WRITE_MASK, _register, data);  // 0bxxxxAAAx assumes address 0000 here.  More general solution would have this as a parameter

}

uint8_olcb_t _read_register(uint8_olcb_t _register) {
    
    // the data here is just to clock out the reply it is not actually used for anything

    _access_register(ADDRESS_READ_MASK, _register, 0x00);   // 0bxxxxAAAx assumes address 0000 here.  More general solution would have this as a parameter
    
    return SPI_BUFFER;

}


void MCP23S17Driver_initialize(void) {
    
    // Datasheet says the CS needs a strobe on powerup to reset the chip in mode 1, 1
    _MCP23S17_CS = 0;
    __delay32(2);
    _MCP23S17_CS = 1;

    _write_register(IODIRA, 0b11000000); // Set direction top 2 bits are not used so leave them as inputs
    _write_register(IODIRB, 0b10000000); // Set direction top 1 bit is not used so leave as inputs, Pin 7 is the chip select for LED brightness gain and needs to be high
    
    
    // Turn on all the LEDs on boot for a test the logic will reset them correctly eventually.
    _write_register(OLATA, 0b00111111); // Set output to high 
    _write_register(OLATB, 0b01111111); // Set output to high 

}

void MCP23S17Driver_set_signals(uint8_olcb_t aspect_A, uint8_olcb_t aspect_B, uint8_olcb_t aspect_C, uint8_olcb_t aspect_D) { // 0b00000RGY

    aspect_A = aspect_A & 0b00000111;
    aspect_B = aspect_B & 0b00000111;
    aspect_C = aspect_C & 0b00000111;
    aspect_D = aspect_D & 0b00000111;
    
    
    // Had to change the order of the C signal bits to make the layout easier
    uint8_olcb_t temp_port_D = ((aspect_D >> 2) & 0b00000001) | ((aspect_D << 2) & 0b00000100) | (aspect_D & 0b00000010);
    
    
    uint8_olcb_t port = aspect_C | (aspect_B << 3); // keep the chip select (CS) for the brightness gain adjust high
    _write_register(OLATA, port);

    
    port = 0b01000000 | aspect_A | (temp_port_D << 3);
    _write_register(OLATB, port);
    
}

void MCP23S17Driver_set_signal_brightness_cs(void) { 
    
    uint8_olcb_t port = _read_register(OLATB);
        
    port = port & 0b10111111;  // clear the CS bit
            
    _write_register(OLATB, port);
    
}

void MCP23S17Driver_clear_signal_brightness_cs(void) { 
    
    uint8_olcb_t port = _read_register(OLATB);
    port = port | 0b01000000;  // clear the CS bit
            
    _write_register(OLATB, port);
    
}


