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
 * \file dependency_injection.h
 *
 *
 * @author Jim Kueneman
* @date 11 Nov 2025
 */


// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DEPENDENCY_INJECTION__
#define	__DEPENDENCY_INJECTION__


#include <stdbool.h>
#include <stdint.h>

#include "esp32_wifi_gridconnect_drivers.h"
#include "esp32_drivers.h"

#include "src/openlcb/openlcb_types.h"
#include "src/drivers/common/can_types.h"

#define TRANSMIT_CAN_FRAME_FUNC &Esp32WiFiGridconnectDriver_transmit_raw_can_frame
#define IS_TX_BUFFER_EMPTY_FUNC &Esp32WiFiGridconnectDriver_is_can_tx_buffer_clear
#define LOCK_SHARED_RESOURCES_FUNC &Esp32Drivers_lock_shared_resources
#define UNLOCK_SHARED_RESOURCES_FUNC &Esp32Drivers_unlock_shared_resources
#define CONFIG_MEM_READ_FUNC &Esp32Drivers_config_mem_read
#define CONFIG_MEM_WRITE_FUNC Esp32Drivers_config_mem_write
#define OPERATIONS_REBOOT_FUNC &Esp32Drivers_reboot
#define OPERATIONS_FACTORY_RESET_FUNC NULL

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

extern void DependencyInjection_initialize(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DEPENDENCY_INJECTION__ */