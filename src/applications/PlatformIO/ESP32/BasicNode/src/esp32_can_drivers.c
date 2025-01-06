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
 * \file esp32_can_drivers.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Jan 2025
 */

#include "esp32_can_drivers.h"

#include "src/drivers/common/can_types.h"
#include "src/openlcb/openlcb_gridconnect.h"
#include "src/utilities/mustangpeak_string_helper.h"

can_rx_callback_func_t internal_can_rx_callback_func;

uint8_olcb_t _is_connected = FALSE;

uint8_olcb_t Esp32CanDriver_is_connected(void)
{

    uint8_olcb_t result = _is_connected;
}

uint8_olcb_t Esp32CanDriver_is_can_tx_buffer_clear(uint16_olcb_t channel)
{

    return FALSE;
}

uint8_olcb_t Esp32CanDriver_transmit_raw_can_frame(uint8_olcb_t channel, can_msg_t *msg)
{

    return FALSE;
}

void Esp32CanDriver_pause_can_rx()
{
}

void Esp32CanDriver_resume_can_rx()
{
}

void Esp32CanDriver_setup(can_rx_callback_func_t can_rx_callback)
{

    internal_can_rx_callback_func = can_rx_callback;
}
