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


// Add any compiler specific includes

#include "../../openlcb/openlcb_types.h"


void DriverConfigurationMemory_initialization() {
    
    // SPI1 Initialize ---------------------------------------------------------
    // -------------------------------------------------------------------------
    
    // Any SPI initialization or other Initialize functions

    
    
}

uint16_t DriverConfigurationMemory_read(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {
    
    
    // Implement your Configuration Memory Read Here:
    
// Example;
//   return _25AA1024_Driver_read(address, count, buffer);

    return 0;
    
}

uint16_t DriverConfigurationMemory_write(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {
    
    
    // Implement your Configuration Memory Write Here:

// Example:
//    _25AA1024_Driver_write_latch_enable();
//    _25AA1024_Driver_write(address, count, buffer);
//    
//    while (_25AA1024_Driver_write_in_progress()) {
//        
//    }
//    
//    return count;
    
    return 0;
    
}