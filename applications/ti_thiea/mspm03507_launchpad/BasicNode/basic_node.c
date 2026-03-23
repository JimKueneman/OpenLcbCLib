/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "stdio.h"
#include "string.h"

#include "ti_msp_dl_config.h"
#include <ti/driverlib/m0p/dl_interrupt.h>

#include "debug_tools.h"

#include "application_callbacks/callbacks_can.h"
#include "application_callbacks/callbacks_olcb.h"
#include "application_callbacks/callbacks_config_mem.h"
#include "openlcb_user_config.h"
#include "application_drivers/ti_driverlib_can_driver.h"
#include "application_drivers/ti_driverlib_drivers.h"

#include "openlcb_c_lib/drivers/canbus/can_config.h"
#include "openlcb_c_lib/openlcb/openlcb_config.h"

#define NODE_ID 0x0501010107EE
#define DELAY_TIME (50000000)

static const can_config_t can_config = {

    .transmit_raw_can_frame  = &TI_DriverLibCanDriver_transmit_can_frame,
    .is_tx_buffer_clear      = &TI_DriverLibCanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &TI_DriverLibDrivers_lock_shared_resources,
    .unlock_shared_resources = &TI_DriverLibDrivers_unlock_shared_resources,
    .on_rx                   = &CallbacksCan_on_rx,
    .on_tx                   = &CallbacksCan_on_tx,
    .on_alias_change         = &CallbacksCan_on_alias_change,

};

static const openlcb_config_t openlcb_config = {

    .lock_shared_resources   = &TI_DriverLibDrivers_lock_shared_resources,
    .unlock_shared_resources = &TI_DriverLibDrivers_unlock_shared_resources,
    .config_mem_read         = &TI_DriverLibDrivers_config_mem_read,
    .config_mem_write        = &TI_DriverLibDrivers_config_mem_write,
    .reboot                  = &TI_DriverLibDrivers_reboot,
    .factory_reset           = &CallbacksConfigMem_factory_reset,
    .freeze                  = &CallbacksConfigMem_freeze,
    .unfreeze                = &CallbacksConfigMem_unfreeze,
    .firmware_write          = &CallbacksConfigMem_firmware_write,
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,

};

int main(void)
{

  SYSCFG_DL_init();

  TI_DriverLibCanDriver_initialize();
  TI_DriverLibDrivers_initialize();

  CanConfig_initialize(&can_config);
  OpenLcb_initialize(&openlcb_config);

  CallbacksOlcb_initialize();

  printf("Booted\n");

  OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);

  while (1)
  {

  //  DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_TEST_B7_PIN);

  //   delay_cycles(DELAY_TIME);

    OpenLcb_run();
  }
}
