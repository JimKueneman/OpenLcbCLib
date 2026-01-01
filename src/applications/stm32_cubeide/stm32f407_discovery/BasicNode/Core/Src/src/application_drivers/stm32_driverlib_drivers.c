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

#include "stm32_driverlib_drivers.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32_driverlib_can_driver.h"

#include "../openlcb/openlcb_types.h"
#include "../drivers/canbus/can_types.h"
#include "../openlcb/openlcb_node.h"
#include "../openlcb/protocol_datagram_handler.h"

static TIM_HandleTypeDef *_htim7;

void STM32_DriverLibDrivers_initialize(TIM_HandleTypeDef *htim7)
{

	_htim7 = htim7;

	// Start Timer 7
	HAL_TIM_Base_Start_IT(htim7);
}

void STM32_DriverLibDrivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info)
{

	HAL_NVIC_SystemReset();
}

uint16_t STM32_DriverLibDrivers_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

	char str[] = "STM32F407 Discovery";

	for (int i = 0; i < count; i++)
	{

		(*buffer)[i] = 0x00;
	}

	switch (address)
	{

	case 0:

		for (int i = 0; i < count; i++)
		{

			(*buffer)[i] = str[i];
		}

		break;

	default:

		break;
	}

	return count;
}

uint16_t STM32_DriverLibDrivers_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

	return count;
}

void STM32_DriverLibDrivers_config_mem_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info)
{
	// Reset node (in statemachine_info struct) the  to factory settings
}

void STM32_DriverLibDrivers_lock_shared_resources(void)
{

	STM32_DriverLibCanDriver_pause_can_rx();

	HAL_TIM_Base_Stop(_htim7);
}

void STM32_DriverLibDrivers_unlock_shared_resources(void)
{

	STM32_DriverLibCanDriver_resume_can_rx();

	HAL_TIM_Base_Start_IT(_htim7);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if (htim == _htim7)
	{

		OpenLcbNode_100ms_timer_tick();
		ProtocolDatagramHandler_100ms_timer_tick();
	}
}
