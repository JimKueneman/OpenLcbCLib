
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
 * \file turnoutboss_drivers.h
 *
 *
 * @author Jim Kueneman
 * @date 3 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __TURNOUTBOSS_BOOTLOADER_DRIVERS__
#define	__TURNOUTBOSS_BOOTLOADER_DRIVERS__

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif


// Assign the function pointer to where the UART Rx should call back with the byte it received
// WARNING: Is in the context of the interrupt, be careful
// void func(rx_data);
typedef void (*uart_rx_callback_t) (uint16_olcb_t);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // OpenLcbCLib defined callback functions that must be defined
    
    extern void TurnoutBossBootloaderDrivers_setup(parameterless_callback_t _100ms_timer_sink);

    extern void TurnoutBossBootloaderDrivers_reboot(void);
    
    extern uint16_olcb_t TurnoutBossBootloaderDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer);
    
    extern uint16_olcb_t TurnoutBossBootloaderDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer);
    
    extern void TurnoutBossBootloaderDrivers_factory_reset(void);
    
    extern void TurnoutBossBootloaderDrivers_pause_100ms_timer();
    
    extern void TurnoutBossBootloaderDrivers_resume_100ms_timer();
    
    // Custom Driver functions
    
    extern void TurnoutBossBootloaderDrivers_assign_uart_rx_callback(uart_rx_callback_t uart_rx_callback);
    
    extern void TurnoutBossBootloaderDrivers_u1_rx_interrupt_handler(void);
    
    extern void TurnoutBossBootloaderDrivers_u1_tx_interrupt_handler(void);
    
    extern void TurnoutBossBootloaderDrivers_t2_interrupt_handler(void);
   

#endif	/* __TURNOUTBOSS_DRIVERS__ */

