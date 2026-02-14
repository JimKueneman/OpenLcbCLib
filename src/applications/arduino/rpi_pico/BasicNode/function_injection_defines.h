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
 * \file function_injection_defines.c
 *
 *
 * @author Jim Kueneman
 * @date 27 Nov 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __FUNCTION_INJECTION_DEFINES__
#define __FUNCTION_INJECTION_DEFINES__

// Function implementation includes
#include "callbacks.h"
#include "src/application_drivers/rpi_pico_can_drivers.h"
#include "src/application_drivers/rpi_pico_drivers.h"

// OpenlcbCLib includes
#include "src/node_definition/dependency_injection.h"
#include "src/openlcb/openlcb_buffer_store.h"
#include "src/drivers/canbus/can_tx_statemachine.h"

// ******************************************************************************
// Application Defined Drivers to interface the chosen hardware with the Library
// ******************************************************************************

// Implement Application defined CAN Transmit of a CAN Frame
#define TRANSMIT_CAN_FRAME_FUNC &RPiPicoCanDriver_transmit_raw_can_frame
// Implement Application defined test if the CAN Transmit buffer can accept another frame to send
#define IS_TX_BUFFER_EMPTY_FUNC &RPiPicoCanDriver_is_can_tx_buffer_clear
// Implement Application defined Lock access to the OpenLcb and CAN Buffers, Lists and FIFOs from interrupts/treads as well as stopping the 100ms timer
#define LOCK_SHARED_RESOURCES_FUNC &RPiPicoDrivers_lock_shared_resources
// Implement Application defined Unlocking access to the OpenLcb and CAN Buffers, Lists and FIFOs as well as stopping the 100ms timer
#define UNLOCK_SHARED_RESOURCES_FUNC &RPiPicoDrivers_unlock_shared_resources
// Implement the Application defined Configuration Memory Read Access to the EEPROM/FLASH/File/etc.
#define CONFIG_MEM_READ_FUNC &RPiPicoDrivers_config_mem_read
// Implement the Application defined Configuration Memory Write Access to the EEPROM/FLASH/File/etc.
#define CONFIG_MEM_WRITE_FUNC &RPiPicoDrivers_config_mem_write
// Implement Application defined the processor reboot
#define OPERATIONS_REQUEST_REBOOT_FUNC &RPiPicoDrivers_reboot
// Implement Application defined for Freeze (used during boot-loading)
#define OPERATIONS_REQUEST_FREEZE_FUNC NULL
// Implement Application defined for UnFreeze (used during boot-loading)
#define OPERATIONS_REQUEST_UNFREEZE_FUNC NULL
// Implement Application defined for Firmware Upgrade (used during boot-loading)
#define FIRMWARE_WRITE_FUNC NULL

// ******************************************************************************

// ******************************************************************************
// Application defined callback functions in dependency_injectors.h
// ******************************************************************************

#define ON_100MS_TIMER_CALLBACK &Callbacks_on_100ms_timer_callback
#define ON_CAN_RX_CALLBACK &Callbacks_on_can_rx_callback
#define ON_CAN_TX_CALLBACK &Callbacks_on_can_tx_callback
#define ON_ALIAS_CHANGE_CALLBACK &Callbacks_alias_change_callback
#define ON_LOGIN_COMPLETE_CALLBACK NULL

#define ON_CONSUMED_EVENT_IDENTIFIED NULL
#define ON_CONSUMED_EVENT_PCER NULL
#define ON_EVENT_LEARN_CALLBACK NULL

// ******************************************************************************

// Broadcast Time Callbacks
#define ON_BROADCAST_TIME_RECEIVED NULL
#define ON_BROADCAST_DATE_RECEIVED NULL
#define ON_BROADCAST_YEAR_RECEIVED NULL
#define ON_BROADCAST_RATE_RECEIVED NULL
#define ON_BROADCAST_CLOCK_STARTED NULL
#define ON_BROADCAST_CLOCK_STOPPED NULL
#define ON_BROADCAST_DATE_ROLLOVER NULL

// ******************************************************************************
// Application defined library extended feature functions in dependency_injectors.h
// ******************************************************************************

#define OPERATIONS_REQUEST_FACTORY_RESET_FUNC &Callbacks_operations_request_factory_reset
#define CONFIG_MEM_READ_DELAYED_REPLY_TIME_FUNC NULL
#define CONFIG_MEM_WRITE_DELAYED_REPLY_TIME_FUNC NULL

// ******************************************************************************
// Required cross connections between the CANBUS drivers and the OpenLcb drivers
// ******************************************************************************

// The CANBUS drivers need to allocate OpenLCB buffers when receiving CAN frames in order to place them
// in the OpenLcb Buffer FIFO for processing of the main loop.
#define OPENLCB_BUFFER_STORE_ALLOCATE_BUFFER_FUNC &OpenLcbBufferStore_allocate_buffer
// The Openlcb library needs to send messages to the CANBUS driver through this cross coupling
#define SEND_OPENLCB_MESSAGE_FUNC &CanTxStatemachine_send_openlcb_message

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FUNCTION_INJECTION_DEFINES__ */
