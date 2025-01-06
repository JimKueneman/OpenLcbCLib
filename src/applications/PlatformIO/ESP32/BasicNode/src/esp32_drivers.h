
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
 * \file dsPIC33EPxxxGP50x_drivers.h
 *
 *
 * @author Jim Kueneman
 * @date 3 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OSX_DRIVERS__
#define __OSX_DRIVERS__

#include "src/openlcb/openlcb_types.h"

// Assign the function pointer to where the UART Rx should call back with the byte it received
// WARNING: Is in the context of the interrupt, be careful
// void func(rx_data);
typedef void (*uart_rx_callback_t)(uint16_olcb_t);

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void Esp32Drivers_setup(parameterless_callback_t _100ms_timer_sink);

    extern void Esp32Drivers_reboot(void);

    extern uint16_olcb_t Esp32Drivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer);

    extern uint16_olcb_t Esp32Drivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer);

    extern void Esp32Drivers_pause_100ms_timer();

    extern void Esp32Drivers_resume_100ms_timer();

    extern uint8_olcb_t Esp32Drivers_100ms_is_connected(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OSX_DRIVERS__ */
