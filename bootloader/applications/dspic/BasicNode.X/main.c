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

#include "../shared/bootloader_shared_ram.h"
#include "traps.h"

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
    .firmware_write          = NULL,
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,

};

static void _initialize_io_early_for_test(void)
{

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELE = 0x00;

    TEST_PIN_1401_TRIS = 0;
    TEST_PIN_1402_TRIS = 0;
    TEST_PIN_1403_TRIS = 0;
    TEST_PIN_1404_TRIS = 0;

}

int main(void)
{

    _initialize_io_early_for_test();

    /* Register VIVT handlers before any interrupts are enabled.
     * The bootloader owns the hardware IVT; these function pointers
     * let the bootloader's ISR stubs forward to our handlers. */
    bootloader_vivt_jumptable.timer_2_handler        = Dspic33Drivers_t2_interrupt_handler;
    bootloader_vivt_jumptable.can1_handler            = Dspic33CanDriver_c1_interrupt_handler;
    bootloader_vivt_jumptable.oscillatorfail_handler  = Traps_oscillator_fail_handler;
    bootloader_vivt_jumptable.addresserror_handler    = Traps_address_error_handler;
    bootloader_vivt_jumptable.stackerror_handler      = Traps_stack_error_handler;
    bootloader_vivt_jumptable.matherror_handler       = Traps_math_error_handler;
    bootloader_vivt_jumptable.dmacerror_handler       = Traps_dmac_error_handler;

    /* Temporary UART init — bootloader MCC values may be clobbered by app CRT0.
     * 333333 baud, 8N1, BRGH=1, U1BRG=29 @ FCY=40 MHz.
     * Remove once app has its own MCC UART setup. */
    U1MODEbits.STSEL  = 0;   /* 1 stop bit          */
    U1MODEbits.PDSEL  = 0;   /* 8-bit, no parity    */
    U1MODEbits.ABAUD  = 0;   /* no auto-baud        */
    U1MODEbits.BRGH   = 1;   /* high-speed mode     */
    U1BRG             = 29;  /* 333333 baud @ 40 MHz */
    U1MODEbits.UARTEN = 1;   /* enable UART module  */
    U1STAbits.UTXEN   = 1;   /* enable transmitter  */

    CallbacksOlcb_initialize();

    Dspic33CanDriver_initialize();
    BasicNodeDrivers_initialize();

    CanConfig_initialize(&can_config);
    OpenLcb_initialize(&openlcb_config);

    printf("MCU Initialized\n");

    OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);

    printf("Node Allocated\n");
    
    printf("Re-enable Global Interrupts\n");
     _GIE = 1;

    printf("=== VIVT DEBUG ===\n");
    printf("VIVT addr=%p\n", (void*)&bootloader_vivt_jumptable);
    printf("  t2_handler=%p\n",  (void*)bootloader_vivt_jumptable.timer_2_handler);
    printf("  can1_handler=%p\n", (void*)bootloader_vivt_jumptable.can1_handler);
    printf("  oscfail=%p\n",  (void*)bootloader_vivt_jumptable.oscillatorfail_handler);
    printf("  addrerr=%p\n",  (void*)bootloader_vivt_jumptable.addresserror_handler);
    printf("  stkerr=%p\n",   (void*)bootloader_vivt_jumptable.stackerror_handler);
    printf("  matherr=%p\n",  (void*)bootloader_vivt_jumptable.matherror_handler);
    printf("  dmacerr=%p\n",  (void*)bootloader_vivt_jumptable.dmacerror_handler);
    printf("IEC0=0x%04X (T2IE=%d)\n", IEC0, IEC0bits.T2IE);
    printf("IEC2=0x%04X (C1IE=%d)\n", IEC2, IEC2bits.C1IE);
    printf("T2CON=0x%04X (TON=%d)\n", T2CON, T2CONbits.TON);
    printf("C1CTRL1=0x%04X\n", C1CTRL1);
    printf("C1INTE=0x%04X\n", C1INTE);
    printf("C1INTF=0x%04X\n", C1INTF);
    printf("DMA0CON=0x%04X DMA1CON=0x%04X DMA2CON=0x%04X\n", DMA0CON, DMA1CON, DMA2CON);
    printf("DMA0STAL=0x%04X DMA1STAL=0x%04X DMA2STAL=0x%04X\n", DMA0STAL, DMA1STAL, DMA2STAL);
    printf("C1RXM0SID=0x%04X C1RXM0EID=0x%04X\n", C1RXM0SID, C1RXM0EID);
    printf("C1RXF0SID=0x%04X C1FEN1=0x%04X\n", C1RXF0SID, C1FEN1);
    printf("_GIE=%d\n", _GIE);
    printf("=================\n");

    while (1)
    {

        OpenLcb_run();

    }

}
