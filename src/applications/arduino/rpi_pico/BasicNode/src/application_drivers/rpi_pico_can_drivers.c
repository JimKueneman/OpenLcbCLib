#include <stdbool.h>
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
 * \file rpi_pico_can_drivers.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 31 Dec 2025
 */

#define ARDUINO_COMPATIBLE

#include "rpi_pico_can_drivers.h"

#include "../drivers/canbus/can_rx_statemachine.h"
#include "../drivers/canbus/can_types.h"
#include "../openlcb/openlcb_gridconnect.h"
#include "../utilities/mustangpeak_string_helper.h"

#ifdef ARDUINO_COMPATIBLE
#include "Arduino.h"
// include any header files the Raspberry Pi Pico need to compile under Arduino/PlatformIO
#endif

void RPiPicoCanDriver_setup(void)
{

  // Initialize Raspberry Pi Pico CAN features
}

void receive_task_or_interrupt(void *arg)
{

  // TODO: Spin up a task (if using RTOS) or implement an interrupt to call this function.
  // Once a Frame is received call:
  //      CanRxStatemachine_incoming_can_driver_callback(&can_msg);
  // with the newly received frame, UNLESS RPiPicoCanDriver_pause_can_rx() has been called then hold the
  // message until RPiPicoCanDriver_resume_can_rx() has been called
}

bool RPiPicoCanDriver_is_can_tx_buffer_clear(void)
{

  // TODO: Return if the hardware transmitter can take another frame (i.e. will RPiPicoCanDriver_transmit_raw_can_frame succeed

  return true; // TODO: Temporary
}

bool RPiPicoCanDriver_transmit_raw_can_frame(can_msg_t *msg)
{

  // TODO: Setup the native driver can buffer with the passed info and send it out on the CAN wire in Extended format (19 bit Identifier)

  return true; // TODO: Temporary
}

void RPiPicoCanDriver_pause_can_rx(void)
{

  // TODO: Resume sending incoming CAN frames to the OpenLcbCLib function in the task/thread/interrupt function above:
  //          CanRxStatemachine_incoming_can_driver_callback(&can_msg);
}

void RPiPicoCanDriver_resume_can_rx(void)
{

  // TODO: Pause sending incoming CAN frames to the OpenLcbCLib function in the task/thread/interrupt function above:
  //          CanRxStatemachine_incoming_can_driver_callback(&can_msg);
}
