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
 * \file driver_configuration_memory.h
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to write/read configuration memory.  A new supported MCU/PC will create a file that handles the 
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVER_CONFIGURATION_MEMORY__
#define	__DRIVER_CONFIGURATION_MEMORY__

#include "../openlcb/openlcb_types.h"

typedef uint16_olcb_t(*configuration_mem_callback_t) (uint32_olcb_t, uint16_olcb_t, configuration_memory_buffer_t*);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void DriverConfigurationMemory_initialization(
            configuration_mem_callback_t configuration_mem_read_callback,
            configuration_mem_callback_t configuration_mem_write_callback,
            parameterless_callback_t configuration_mem_factory_reset_callback
            );

    extern configuration_mem_callback_t DriverConfigurationMemory_get_read_callback(void);

    extern configuration_mem_callback_t DriverConfigurationMemory_get_write_callback(void);
    
    extern parameterless_callback_t DriverConfigurationMemory_get_factory_reset_callback(void);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVER_CONFIGURATION_MEMORY__ */

