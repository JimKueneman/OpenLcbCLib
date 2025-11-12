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
 * \file ti_driverlib_drivers.c
 *
 * Definition of the node at the application level.  
 *
 * @author Jim Kueneman
 * @date 11 Nov 2024
 */

 #include "ti_driverlib_drivers.h"

#include <ti/driverlib/m0p/dl_interrupt.h>
#include "ti_msp_dl_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "debug_tools.h"
#include "ti_driverlib_can_driver.h"

#include "src/openlcb/openlcb_types.h"
#include "src/drivers/common/can_types.h" 

 
 void TI_DriverLibDrivers_initialize(void) {

 }

 void TI_DriverLibDrivers_reboot(void) {

 }

 uint16_t TI_DriverLibDrivers_config_mem_read(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {

    for (int i = 0; i < count; i++) {

        (*buffer)[i] = 0x00;

    }
    
    return count;
 }

 uint16_t TI_DriverLibDrivers_config_mem_write(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {

    return count;

 }
    
 void TI_DriverLibDrivers_config_mem_factory_reset(void) {

 }

 void TI_DriverLibDrivers_lock_shared_resources(void) {

    TI_DriverLibCanDriver_pause_can_rx();

    // shut off the 100ms timer TODO:
 }

 void TI_DriverLibDrivers_unlock_shared_resources(void) {

    TI_DriverLibCanDriver_pause_can_rx();

    // turn on the 100ms timer TODO:

 }