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
 * \file main.c
 *
 *  main
 *
 * @author Jim Kueneman
 * @date 19 Mar 2026
 */

// DSPIC33EP512GP504 Configuration Bit Settings

// 'C' source line config statements

// FICD
#pragma config ICS = PGD1   // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF // JTAG Enable bit (JTAG is disabled)

// FPOR
#pragma config ALTI2C1 = OFF  // Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins)
#pragma config ALTI2C2 = OFF  // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
#pragma config WDTWIN = WIN25 // Watchdog Window Select bits (WDT Window is 25% of WDT period)

// FWDT
#pragma config WDTPOST = PS32768 // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128    // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON       // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF      // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF      // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FOSC
#pragma config POSCMD = HS    // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = OFF  // Peripheral pin select configuration (Allow multiple reconfigurations)
#pragma config FCKSM = CSDCMD // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = PRIPLL // Oscillator Source Selection (Primary Oscillator with PLL module (XT + PLL, HS + PLL, EC + PLL))
#pragma config PWMLOCK = OFF  // PWM Lock Enable bit (PWM registers may be written without key sequence)
#pragma config IESO = ON      // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF  // General Segment Code-Protect bit (General Segment Code protect is Disabled)

#define TEST_PIN_1401_TRIS _TRISA11
#define TEST_PIN_1401 _RA11

#define TEST_PIN_1402_TRIS _TRISB14
#define TEST_PIN_1402 _LATB14

#define TEST_PIN_1403_TRIS _TRISG9
#define TEST_PIN_1403 _LATG9

#define TEST_PIN_1404_TRIS _TRISA12
#define TEST_PIN_1404 _LATA12

#define LED_GREEN_TRIS _TRISA0
#define LED_GREEN _LATA0

#include <libpic30.h>

#include "xc.h"
#include "stdio.h" // printf
#include "string.h"
#include "stdlib.h"

#include "src/application_drivers/dspic33_drivers.h"
#include "src/application_drivers/dspic33_can_drivers.h"
#include "openlcb_user_config.h"
#include "src/application_callbacks/callbacks_can.h"
#include "src/application_callbacks/callbacks_olcb.h"
#include "src/application_callbacks/callbacks_config_mem.h"

#include "src/openlcb_c_lib/drivers/canbus/can_config.h"
#include "src/openlcb_c_lib/openlcb/openlcb_config.h"

#define NODE_ID 0x0501010107AA

static const can_config_t can_config = {

    .transmit_raw_can_frame  = &Dspic33CanDriver_transmit_can_frame,
    .is_tx_buffer_clear      = &Dspic33CanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &BasicNodeDrivers_lock_shared_resources,
    .unlock_shared_resources = &BasicNodeDrivers_unlock_shared_resources,
    .on_rx                   = &CallbacksCan_on_rx,
    .on_tx                   = &CallbacksCan_on_tx,
    .on_alias_change         = &CallbacksCan_on_alias_change,

};

static const openlcb_config_t openlcb_config = {

    .lock_shared_resources   = &BasicNodeDrivers_lock_shared_resources,
    .unlock_shared_resources = &BasicNodeDrivers_unlock_shared_resources,
    .config_mem_read         = &BasicNodeDrivers_config_mem_read,
    .config_mem_write        = &BasicNodeDrivers_config_mem_write,
    .reboot                  = &BasicNodeDrivers_reboot,
    .factory_reset           = &CallbacksConfigMem_factory_reset,
    .freeze                  = &CallbacksConfigMem_freeze,
    .unfreeze                = &CallbacksConfigMem_unfreeze,
    .firmware_write          = &CallbacksConfigMem_firmware_write,
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,

};

static void _initialize_io_early_for_test(void)
{

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELE = 0x00;

    LED_GREEN_TRIS = 0;

    TEST_PIN_1401_TRIS = 0;
    TEST_PIN_1402_TRIS = 0;
    TEST_PIN_1403_TRIS = 0;
    TEST_PIN_1404_TRIS = 0;

}

int main(void)
{

    _initialize_io_early_for_test();

    CallbacksOlcb_initialize();

    Dspic33CanDriver_initialize();
    BasicNodeDrivers_initialize();

    CanConfig_initialize(&can_config);
    OpenLcb_initialize(&openlcb_config);

    printf("MCU Initialized\n");

    OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);

    printf("Node Allocated\n");

    while (1)
    {

        OpenLcb_run();

    }

}
