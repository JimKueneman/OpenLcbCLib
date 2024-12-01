/*
 * File:   driver_configuration_memory.c
 * Author: jimkueneman
 *
 * Created on December 1, 2024, 6:57 AM
 */


#include "xc.h"
#include "driver_configuration_memory.h"
#include "25AA1024/25AA1024_driver.h"


void DriverConfigurationMemory_initialization() {
    
    
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