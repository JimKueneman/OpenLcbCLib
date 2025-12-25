
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

#include "../../../openlcb/openlcb_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // OpenLcbCLib defined callback functions that much be defined

    extern void BasicNodeDrivers_initialize(void);

    extern void BasicNodeDrivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    extern uint16_t BasicNodeDrivers_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

    extern uint16_t BasicNodeDrivers_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

    extern void BasicNodeDrivers_lock_shared_resources(void);

    extern void BasicNodeDrivers_unlock_shared_resources(void);


#endif	/* __TURNOUTBOSS_DRIVERS__ */

