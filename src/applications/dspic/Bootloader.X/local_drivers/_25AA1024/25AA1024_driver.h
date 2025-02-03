
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
 * \file 25AA1024_driver.h
 *
 * Driver for the MicroChip 25AA1024 EEPROM.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __25AA1024_DRIVER__
#define	__25AA1024_DRIVER__

#include "../../src/openlcb/openlcb_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void _25AA1024_Driver_initialize(void);
    
    extern void _25AA1024_Driver_erase_chip(void);

    extern uint8_olcb_t _25AA1024_Driver_read_status_register(void);

    extern void _25AA1024_Driver_write_status_register(uint8_olcb_t new_status);

    extern void _25AA1024_Driver_write_latch_enable(void);

    extern void _25AA1024_Driver_write_latch_disable(void);

    extern void _25AA1024_Driver_write_byte(uint32_olcb_t address, uint8_olcb_t byte);

    extern uint16_olcb_t _25AA1024_Driver_write(uint32_olcb_t address, uint8_olcb_t count, configuration_memory_buffer_t* buffer);

    extern uint8_olcb_t _25AA1024_Driver_write_in_progress();

    extern uint8_olcb_t _25AA1024_Driver_read_byte(uint32_olcb_t address);

    extern uint16_olcb_t _25AA1024_Driver_read(uint32_olcb_t address, uint8_olcb_t count, configuration_memory_buffer_t* buffer);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __25AA1024_DRIVER__ */

