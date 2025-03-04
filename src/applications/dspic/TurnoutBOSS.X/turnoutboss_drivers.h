
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
#ifndef __TURNOUTBOSS_DRIVERS__
#define	__TURNOUTBOSS_DRIVERS__

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

    // OpenLcbCLib defined callback functions that much be defined

    extern void TurnoutBossDrivers_setup(parameterless_callback_t _100ms_timer_sink);

    extern void TurnoutBossDrivers_reboot(void);

    extern uint16_olcb_t TurnoutBossDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer);

    extern uint16_olcb_t TurnoutBossDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer);
    
    extern void TurnoutBossDrivers_config_mem_factory_reset(void);

    extern void TurnoutBossDrivers_pause_100ms_timer(void);

    extern void TurnoutBossDrivers_resume_100ms_timer(void);

    // Custom Driver functions

    extern void TurnoutBossDrivers_assign_uart_rx_callback(uart_rx_callback_t uart_rx_callback);

    extern void TurnoutBossDrivers_u1_rx_interrupt_handler(void);

    extern void TurnoutBossDrivers_u1_tx_interrupt_handler(void);

    extern void TurnoutBossDrivers_t2_interrupt_handler(void);

    extern void TurnoutBossDrivers_t1_interrupt_handler(void);

    extern void TurnoutBossDrivers_pause_signal_calculation_timer(void);

    extern void TurnoutBossDrivers_resume_signal_calculation_timer(void);

    extern void TurnoutBossDrivers_set_signal_update_timer_sink(parameterless_callback_t signal_update_timer_sink_func);

#endif	/* __TURNOUTBOSS_DRIVERS__ */

