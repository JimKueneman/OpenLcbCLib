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
 * \file ti_driverlib_can_driver.c
 *
 * Definition of the node at the application level.
 *
 * @author Jim Kueneman
 * @date 1 Jan 2026
 */

#include "stm32_driverlib_can_driver.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"

#include "../openlcb/openlcb_types.h"
#include "../drivers/canbus/can_types.h"
#include "../drivers/canbus/can_rx_statemachine.h"

static bool _is_transmitting = false;
static CAN_HandleTypeDef *_hcan1;

void STM32_DriverLibCanDriver_initialize(CAN_HandleTypeDef *hcan1)
{

	CAN_FilterTypeDef RxFilter;

	_hcan1 = hcan1;

	RxFilter.FilterActivation = CAN_FILTER_ENABLE;
	RxFilter.FilterBank = 0;
	RxFilter.SlaveStartFilterBank = 0;
	RxFilter.FilterFIFOAssignment = CAN_RX_FIFO0; // send to FIFO0
	RxFilter.FilterIdHigh = 0;
	RxFilter.FilterIdLow = 0;
	RxFilter.FilterMaskIdHigh = 0;
	RxFilter.FilterMaskIdLow = 0;
	RxFilter.FilterMode = CAN_FILTERMODE_IDMASK;
	RxFilter.FilterScale = CAN_FILTERSCALE_32BIT;

	HAL_CAN_ConfigFilter(hcan1, &RxFilter);

	// Start the CAN Module
	HAL_CAN_Start(hcan1);

	// Enabled Interrupts
	HAL_CAN_ActivateNotification(hcan1,
								 CAN_IT_TX_MAILBOX_EMPTY |
									 CAN_IT_RX_FIFO0_MSG_PENDING);
}

bool STM32_DriverLibCanDriver_is_can_tx_buffer_clear(void)
{

	STM32_DriverLibCanDriver_pause_can_rx();
	bool result = !_is_transmitting;
	STM32_DriverLibCanDriver_resume_can_rx();

	return result;
}

void STM32_DriverLibCanDriver_pause_can_rx(void)
{

	HAL_CAN_DeactivateNotification(_hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void STM32_DriverLibCanDriver_resume_can_rx(void)
{

	HAL_CAN_ActivateNotification(_hcan1,
								 CAN_IT_TX_MAILBOX_EMPTY |
									 CAN_IT_RX_FIFO0_MSG_PENDING);
}

bool STM32_DriverLibCanDriver_transmit_can_frame(can_msg_t *msg)
{

	CAN_TxHeaderTypeDef TxHeader;
	uint8_t aData[8];
	uint32_t TxMailBox;

	if (STM32_DriverLibCanDriver_is_can_tx_buffer_clear())
	{

		TxHeader.DLC = msg->payload_count;
		TxHeader.ExtId = msg->identifier;
		TxHeader.RTR = CAN_RTR_DATA;
		TxHeader.IDE = CAN_ID_EXT;
		TxHeader.TransmitGlobalTime = DISABLE;
		TxHeader.StdId = 0x00;

		TxMailBox = 0;

		for (int i = 0; i < msg->payload_count; i++)
		{ // Copy the data

			aData[i] = msg->payload[i];
		}

		if (HAL_CAN_AddTxMessage(_hcan1, &TxHeader, aData, &TxMailBox) == HAL_OK)
		{

			_is_transmitting = true;

			return true;
		}
	}

	return false;
}

// Override the WEAK defined version of this callback in the HAL
// We only send one at a time so only MailBox 0 will ever be used.

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{

	_is_transmitting = false;
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{

	_is_transmitting = false;
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{

	_is_transmitting = false;
}

// Override the WEAK defined version of this callback in the HAL
// The filter only points to FIFO 0 so that is all we need.

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

	CAN_RxHeaderTypeDef RxHeader;
	uint8_t aData[8];
	can_msg_t can_msg;

	memset(&RxHeader, 0x00, sizeof(RxHeader));
	memset(&can_msg, 0x00, sizeof(can_msg));

	while (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, aData) == HAL_OK)
	{

		if ((RxHeader.IDE == CAN_ID_EXT) && (RxHeader.RTR == CAN_RTR_DATA))
		{

			can_msg.state.allocated = true;
			can_msg.identifier = RxHeader.ExtId;
			can_msg.payload_count = RxHeader.DLC;

			for (int iData = 0; iData < RxHeader.DLC; iData++)
			{

				can_msg.payload[iData] = aData[iData];
			}

			CanRxStatemachine_incoming_can_driver_callback(&can_msg);
		}
	}
}
