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
 * \file esp32_can_drivers.h
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 15 Nov 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __ESP32_CAN_DRIVERS__
#define __ESP32_CAN_DRIVERS__

#include <stdbool.h>
#include <stdint.h>

#include "src/openlcb/openlcb_types.h"
#include "src/drivers/common/can_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void Esp32WiFiGridconnectDriver_log_into_access_point(const char * ssid, const char * pass);

    extern bool Esp32WiFiGridconnectDriver_is_can_tx_buffer_clear(void);

    extern void Esp32WiFiGridconnectDriver_pause_can_rx(void);

    extern void Esp32WiFiGridconnectDriver_resume_can_rx(void);

    extern bool Esp32WiFiGridconnectDriver_transmit_raw_can_frame(can_msg_t *msg);

    extern bool Esp32WiFiGridconnectDriver_is_connected_to_access_point(void);

    extern bool Esp32WiFiGridconnectDriver_is_connected_to_server(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESP32_CAN_DRIVERS__ */
