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

#include "callbacks.h"
#include "node_parameters.h"
#include "src/application_drivers/ti_driverlib_can_driver.h"
#include "src/application_drivers/ti_driverlib_drivers.h"
#include "src/node_definition/dependency_injection.h"
#include "src/node_definition/dependency_injection_canbus.h"

#include "src/drivers/canbus/can_main_statemachine.h"
#include "src/drivers/canbus/can_rx_statemachine.h"
#include "src/drivers/canbus/can_tx_statemachine.h"
#include "src/drivers/canbus/can_types.h"
#include "src/drivers/canbus/can_utilities.h"
#include "src/openlcb/openlcb_login_statemachine.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/openlcb_node.h"

#define NODE_ID 0x0501010107EE
#define DELAY_TIME (50000000)

int main(void)
{

  can_msg_t can_msg;

  SYSCFG_DL_init();

  DependencyInjectionCanBus_initialize();
  DependencyInjection_initialize();

  TI_DriverLibCanDriver_initialize();
  TI_DriverLibDrivers_initialize();

  Callbacks_initialize();

  printf("Booted\n");

  OpenLcbNode_allocate(NODE_ID, &NodeParameters_main_node);

  while (1)
  {

    DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_TEST_B7_PIN);

    // delay_cycles(DELAY_TIME);

    CanMainStateMachine_run();
    OpenLcbLoginMainStatemachine_run();
    OpenLcbMainStatemachine_run();
  }
}
