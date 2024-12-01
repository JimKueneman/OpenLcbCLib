/*
 * File:   driver_configuration_memory.c
 * Author: jimkueneman
 *
 * Created on December 1, 2024, 6:57 AM
 */


#include "xc.h"
#include "stdio.h" // printf
#include "driver_configuration_memory.h"
#include "25AA1024/25AA1024_driver.h"


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

uint16_t DriverConfigurationMemory_read(uint32_t address, uint16_t count, DriverConfigurationMemory_buffer_t* buffer) {
    
    
   return _25AA1024_Driver_read(address, count, buffer);

    
}

uint16_t DriverConfigurationMemory_write(uint32_t address, uint16_t count, DriverConfigurationMemory_buffer_t* buffer) {
    
    _25AA1024_Driver_write_latch_enable();
    _25AA1024_Driver_write(address, count, buffer);
    
    while (_25AA1024_Driver_write_in_progress()) {
        
    }
    
    return count;
    
}