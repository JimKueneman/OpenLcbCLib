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
 * \file ti_driverlib_drivers.h
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 11 Nov 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __STM32_DRIVERLIB_DRIVERS__
#define __STM32_DRIVERLIB_DRIVERS__

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

#include "src/openlcb/openlcb_types.h"
#include "src/drivers/canbus/can_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void STM32_DriverLibDrivers_initialize(TIM_HandleTypeDef *htim7);

    extern void STM32_DriverLibDrivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    extern uint16_t STM32_DriverLibDrivers_config_mem_read(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

    extern uint16_t STM32_DriverLibDrivers_config_mem_write(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

    extern void STM32_DriverLibDrivers_config_mem_factory_reset(void);

    extern void STM32_DriverLibDrivers_lock_shared_resources(void);

    extern void STM32_DriverLibDrivers_unlock_shared_resources(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STM32_DRIVERLIB_CAN_DRIVER__ */
