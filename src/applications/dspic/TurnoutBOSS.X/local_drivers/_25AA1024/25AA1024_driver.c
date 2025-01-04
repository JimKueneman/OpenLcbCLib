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
#include "../../../../../drivers/driver_configuration_memory.h"
#include "../../io_pinout.h"

void _25aa1024_flush_buffers() {

    uint8_olcb_t result;

    // Wait for any transmit to finish
    while (SPI_TX_BUFFER_EMPTY_FLAG == 1) {
    }

    // Clear the Rx Buffer
    if (SPI_RX_BUFFER_EMPTY_FLAG == 1) {

        result = SPI_BUFFER;

    }

}

uint8_olcb_t _25aa1024_wait_for_reply() {

    // Wait for any transmit to finish
    while (SPI_TX_BUFFER_EMPTY_FLAG == 1) {
    }

    // Clear the Rx Buffer
    while (SPI_RX_BUFFER_EMPTY_FLAG == 0) {

    }

    return SPI_BUFFER;

}

uint8_olcb_t _25AA1024_Driver_read_status_register() {

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    // Transmit
    SPI_BUFFER = 0b00000101;

    _25aa1024_wait_for_reply();

    // clock in the reply from the EEPROM
    SPI_BUFFER = 0b00000000;

    uint8_olcb_t result = _25aa1024_wait_for_reply();

    EEPROM_CS = 1; // CS

    return result;


}

void _25AA1024_Driver_write_status_register(uint8_olcb_t new_status) {

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    // Transmit
    SPI_BUFFER = 0b00000101;

    _25aa1024_wait_for_reply();

    // write the next status
    SPI_BUFFER = new_status;

    _25aa1024_wait_for_reply();

    EEPROM_CS = 1; // CS

}

void _25AA1024_Driver_write_latch_enable() {

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    // Transmit
    SPI_BUFFER = 0b00000110;

    _25aa1024_wait_for_reply();

    EEPROM_CS = 1; // CS


}

void _25AA1024_Driver_write_latch_disable() {

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    // Transmit
    SPI_BUFFER = 0b00000100;

    _25aa1024_wait_for_reply();

    EEPROM_CS = 1; // CS

}

uint8_olcb_t _25AA1024_Driver_read_byte(uint32_olcb_t address) {

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    // Transmit
    SPI_BUFFER = 0b00000011;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = (address >> 16) & 0xFF;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = (address >> 8) & 0xFF;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = address & 0xFF;

    _25aa1024_wait_for_reply();

    // Transmit
    SPI_BUFFER = 0b00000000;

    uint8_olcb_t result = _25aa1024_wait_for_reply();

    EEPROM_CS = 1; // CS   

    return result;

}

uint16_olcb_t _25AA1024_Driver_read(uint32_olcb_t address, uint8_olcb_t count, configuration_memory_buffer_t* buffer) {

    uint8_olcb_t temp;

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    // Transmit
    SPI_BUFFER = 0b00000011;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = (address >> 16) & 0xFF;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = (address >> 8) & 0xFF;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = address & 0xFF;

    _25aa1024_wait_for_reply();

    for (int i = 0; i < count; i++) {

        // Transmit
        SPI_BUFFER = 0b00000000;

        temp = _25aa1024_wait_for_reply();

        (*buffer)[i] = temp;

    }

    EEPROM_CS = 1; // CS

    return count;

}

void _25AA1024_Driver_write_byte(uint32_olcb_t address, uint8_olcb_t byte) {

    uint8_olcb_t temp;

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    // Transmit
    SPI_BUFFER = 0b00000010;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = (address >> 16) & 0xFF;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = (address >> 8) & 0xFF;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = address & 0xFF;

    _25aa1024_wait_for_reply();

    // Transmit
    SPI_BUFFER = byte;

    temp = _25aa1024_wait_for_reply();

    EEPROM_CS = 1; // CS

}

void _25AA1024_Driver_erase_chip() {

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    SPI_BUFFER = 0b11000111;

    _25aa1024_wait_for_reply();

    EEPROM_CS = 1;


    while (_25AA1024_Driver_write_in_progress()) {


    }

}

uint16_olcb_t _25AA1024_Driver_write(uint32_olcb_t address, uint8_olcb_t count, configuration_memory_buffer_t* buffer) {

    uint8_olcb_t temp;

    _25aa1024_flush_buffers();

    EEPROM_CS = 0; // CS

    // Transmit
    SPI_BUFFER = 0b00000010;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = (address >> 16) & 0xFF;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = (address >> 8) & 0xFF;

    _25aa1024_wait_for_reply();

    SPI_BUFFER = address & 0xFF;

    _25aa1024_wait_for_reply();

    for (int i = 0; i < count; i++) {

        // Transmit
        SPI_BUFFER = (*buffer)[i];

        temp = _25aa1024_wait_for_reply();

    }

    EEPROM_CS = 1; // CS

    return count;

}

uint8_olcb_t _25AA1024_Driver_write_in_progress() {

    if (_25AA1024_Driver_read_status_register() && 0x01 != 0)

        return 1;

    else

        return 0;

}


