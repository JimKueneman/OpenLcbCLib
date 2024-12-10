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
 * \file 25AA1024.c
 *
 * Driver for the MicroChip 25AA1024 EEPROM.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "25AA1024_driver.h"

#include "xc.h"
#include "../driver_configuration_memory.h"

void _flush_buffers() {

    uint8_t result;

    // Wait for any transmit to finish
    while (_SPITBF == 1) {
    }

    // Clear the Rx Buffer
    if (_SPIRBF == 1) {

        result = SPI1BUF;

    }

}

uint8_t _wait_for_reply() {

    // Wait for any transmit to finish
    while (_SPITBF == 1) {
    }

    // Clear the Rx Buffer
    while (_SPIRBF == 0) {

    }

    return SPI1BUF;

}

uint8_t _25AA1024_Driver_read_status_register() {

    _flush_buffers();

    _RB6 = 0; // CS

    // Transmit
    SPI1BUF = 0b00000101;

    _wait_for_reply();

    // clock in the reply from the EEPROM
    SPI1BUF = 0b00000000;

    uint8_t result = _wait_for_reply();

    _RB6 = 1; // CS

    return result;


}

void _25AA1024_Driver_write_status_register(uint8_t new_status) {

    _flush_buffers();

    _RB6 = 0; // CS

    // Transmit
    SPI1BUF = 0b00000101;

    _wait_for_reply();

    // write the next status
    SPI1BUF = new_status;

    _wait_for_reply();

    _RB6 = 1; // CS

}

void _25AA1024_Driver_write_latch_enable() {

    _flush_buffers();

    _RB6 = 0; // CS

    // Transmit
    SPI1BUF = 0b00000110;

    _wait_for_reply();

    _RB6 = 1; // CS


}

void _25AA1024_Driver_write_latch_disable() {

    _flush_buffers();

    _RB6 = 0; // CS

    // Transmit
    SPI1BUF = 0b00000100;

    _wait_for_reply();

    _RB6 = 1; // CS

}

uint8_t _25AA1024_Driver_read_byte(uint32_t address) {

    _flush_buffers();

    _RB6 = 0; // CS

    // Transmit
    SPI1BUF = 0b00000011;

    _wait_for_reply();

    SPI1BUF = (address >> 16) & 0xFF;

    _wait_for_reply();

    SPI1BUF = (address >> 8) & 0xFF;

    _wait_for_reply();

    SPI1BUF = address & 0xFF;

    _wait_for_reply();

    // Transmit
    SPI1BUF = 0b00000000;

    uint8_t result = _wait_for_reply();

    _RB6 = 1; // CS   

    return result;

}

uint16_t _25AA1024_Driver_read(uint32_t address, uint8_t count, configuration_memory_buffer_t* buffer) {

    uint8_t temp;

    _flush_buffers();

    _RB6 = 0; // CS

    // Transmit
    SPI1BUF = 0b00000011;

    _wait_for_reply();

    SPI1BUF = (address >> 16) & 0xFF;

    _wait_for_reply();

    SPI1BUF = (address >> 8) & 0xFF;

    _wait_for_reply();

    SPI1BUF = address & 0xFF;

    _wait_for_reply();

    for (int i = 0; i < count; i++) {

        // Transmit
        SPI1BUF = 0b00000000;

        temp = _wait_for_reply();

        (*buffer)[i] = temp;

    }

    _RB6 = 1; // CS
    
    return count;

}

void _25AA1024_Driver_write_byte(uint32_t address, uint8_t byte) {

    uint8_t temp;

    _flush_buffers();

    _RB6 = 0; // CS

    // Transmit
    SPI1BUF = 0b00000010;

    _wait_for_reply();

    SPI1BUF = (address >> 16) & 0xFF;

    _wait_for_reply();

    SPI1BUF = (address >> 8) & 0xFF;

    _wait_for_reply();

    SPI1BUF = address & 0xFF;

    _wait_for_reply();

    // Transmit
    SPI1BUF = byte;

    temp = _wait_for_reply();

    _RB6 = 1; // CS

}

uint16_t _25AA1024_Driver_write(uint32_t address, uint8_t count, configuration_memory_buffer_t* buffer) {

    uint8_t temp;

    _flush_buffers();

    _RB6 = 0; // CS

    // Transmit
    SPI1BUF = 0b00000010;

    _wait_for_reply();

    SPI1BUF = (address >> 16) & 0xFF;

    _wait_for_reply();

    SPI1BUF = (address >> 8) & 0xFF;

    _wait_for_reply();

    SPI1BUF = address & 0xFF;

    _wait_for_reply();

    for (int i = 0; i < count; i++) {

        // Transmit
        SPI1BUF = (*buffer)[i];

        temp = _wait_for_reply();

    }

    _RB6 = 1; // CS
    
    return count;

}

 uint8_t _25AA1024_Driver_write_in_progress() {
     
     if (_25AA1024_Driver_read_status_register() && 0x01 != 0)
         
       return 1;
               
     else
         
         return 0;
     
 }


