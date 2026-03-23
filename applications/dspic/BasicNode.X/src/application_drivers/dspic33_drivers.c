/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * @file dspic33_drivers.c
 *
 * General platform drivers for the dsPIC33EP512MC506 BasicNode demo.
 *
 * @author Jim Kueneman
 * @date 19 Mar 2026
 */

#include "dspic33_drivers.h"
#include "dspic33_can_drivers.h"

#include "xc.h"
#include "stdio.h"  // printf
#include <libpic30.h> // delay

#include "../openlcb_c_lib/openlcb/openlcb_config.h"

void BasicNodeDrivers_initialize(void) {

    // IO Pin Initialize -------------------------------------------------------

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    ANSELC = 0x00;
    // -------------------------------------------------------------------------

    // Oscillator Initialize --------------------------------------------------- 
    // Make sure the Fuse bits are set too

    //   011 = Primary Oscillator with PLL (XTPLL, HSPLL, ECPLL)

    // Setting output frequency to 160MHz
    PLLFBDbits.PLLDIV = 60 + PLLDIV_OFFSET; // This should be 60 for 80 Mhz.  Need 80 Mhz because the CAN module is limited to Fcy = 40 Mhz
    CLKDIV = 0x0001; // PreScaler divide by 3; Post Scaler divide by 2

    //    // Make sure PPS Multiple reconfigurations is selected in the Configuration Fuse Bits
    //    // CAN Pin Mapping
    //    RPINR26bits.C1RXR = 37; // RP37 CAN Rx (schematic naming is with respect to the MCU so this is the CAN_rx line)
    //    RPOR2bits.RP38R = _RPOUT_C1TX; // RP38 CAN Tx (schematic naming is with respect to the MCU so this is the CAN_tx line)
    //
    //    // UART Pin Mapping
    //    RPINR18bits.U1RXR = 42; // RP42 UART RX (schematic naming is with respect to the FTDI cable so this is the uart_tx line)
    //    RPOR4bits.RP43R = _RPOUT_U1TX; // RP43  UART TX (schematic naming is with respect to the FTDI cable so this is the uart_rx line)

    // Make sure PPS Multiple reconfigurations is selected in the Configuration Fuse Bits
    // CAN Pin Mapping
    RPINR26bits.C1RXR = 47; // RPI47 CAN Rx (schematic naming is with respect to the MCU so this is the CAN_rx line)
    RPOR8bits.RP118R = _RPOUT_C1TX; // RP118 CAN Tx (schematic naming is with respect to the MCU so this is the CAN_tx line)

    // UART Pin Mapping
    RPINR18bits.U1RXR = 119; // RPI119 UART RX (schematic naming is with respect to the FTDI cable so this is the uart_tx line)
    RPOR9bits.RP120R = _RPOUT_U1TX; // RP120  UART TX (schematic naming is with respect to the FTDI cable so this is the uart_rx line)

    // Setup the 100 ms timer on Timer 2

    IPC1bits.T2IP0 = 1; // Timer 2 Interrupt Priority = 5   (1 means off)
    IPC1bits.T2IP1 = 0;
    IPC1bits.T2IP2 = 1;

    T2CONbits.TCS = 0; // internal clock
    T2CONbits.TCKPS0 = 1; // 256 Prescaler
    T2CONbits.TCKPS1 = 1;
    PR2 = 15625; // Clock ticks every (1/40MHz (Fcy/Fp) * 256 * 15625 = 100.00091ms interrupts

    IFS0bits.T2IF = 0; // Clear T2IF
    IEC0bits.T2IE = 1; // Enable the Interrupt

    T2CONbits.TON = 1; // Turn on Timer 2

}

void BasicNodeDrivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    asm("RESET ");

}


uint16_t BasicNodeDrivers_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {

    char str[] = "dsPIC33E256MC506";

    for (int i = 0; i < count; i++) {

        (*buffer)[i] = 0x00;

    }

    switch (address) {

        case 0:

            for (int i = 0; i < count; i++) {

                (*buffer)[i] = str[i];

            }

            break;

        default:

            break;
    }

    return count;

}

uint16_t BasicNodeDrivers_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {

    return count;

}

void BasicNodeDrivers_lock_shared_resources(void) {

    Dspic33CanDriver_pause_can_rx();

}

void BasicNodeDrivers_unlock_shared_resources(void) {

    Dspic33CanDriver_resume_can_rx();

}

void __attribute__((interrupt(no_auto_psv))) _T2Interrupt(void) {

    IFS0bits.T2IF = 0; // Clear T2IF

    OpenLcb_100ms_timer_tick();

}


