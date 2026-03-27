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
 * @file dspic33_olcb_drivers.h
 *
 * General platform drivers for the dsPIC33EP512MC506 BasicNode demo.
 * Oscillator, GPIO, SPI, timer, and config memory access.
 *
 * @author Jim Kueneman
 * @date 19 Mar 2026
 */

#ifndef __DSPIC33_OLCB_DRIVERS__
#define __DSPIC33_OLCB_DRIVERS__

#include <xc.h>
#include "../openlcb_c_lib/openlcb/openlcb_types.h"

// SPI Pins --------------------------------------------------------------------

#define SPI_CLK _RC3
#define SPI_CLK_TRIS _TRISC3
#define SPI_SDI _RA9
#define SPI_SDI_TRIS _TRISA9
#define SPI_SDO _RA4
#define SPI_SDO_TRIS _TRISA4

#define SPI_TX_BUFFER_EMPTY_FLAG _SPITBF
#define SPI_RX_BUFFER_EMPTY_FLAG _SPIRBF
#define SPI_BUFFER SPI1BUF

// EEPROM Chip Select ----------------------------------------------------------

#define _25AAxxx_CS _RC2
#define _25AAxxx_CS_TRIS _TRISC2
#define _25AAxxx_HOLD _RC1
#define _25AAxxx_HOLD_TRIS _TRISC1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void Dspic33OlcbDrivers_t2_interrupt_handler(void);

    extern void Dspic33OlcbDrivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    extern uint16_t Dspic33OlcbDrivers_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

    extern uint16_t Dspic33OlcbDrivers_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

    extern void Dspic33OlcbDrivers_lock_shared_resources(void);

    extern void Dspic33OlcbDrivers_unlock_shared_resources(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DSPIC33_OLCB_DRIVERS__ */
