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

// Configuration bits are defined in mcc_generated_files/system.c (MCC-generated).
// Do NOT add #pragma config here -- having them in two files is a compiler error.

#define TEST_PIN_1401_TRIS _TRISA11
#define TEST_PIN_1401 _RA11

#define TEST_PIN_1402_TRIS _TRISB14
#define TEST_PIN_1402 _LATB14

#define TEST_PIN_1403_TRIS _TRISG9
#define TEST_PIN_1403 _LATG9

#define TEST_PIN_1404_TRIS _TRISA12
#define TEST_PIN_1404 _LATA12

#include "xc.h"
#include "string.h"
#include "stdlib.h"

#include "mcc_generated_files/system.h"
#include "src/application_drivers/dspic33_olcb_drivers.h"
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
    .lock_shared_resources   = &Dspic33OlcbDrivers_lock_shared_resources,
    .unlock_shared_resources = &Dspic33OlcbDrivers_unlock_shared_resources,
    .on_rx                   = &CallbacksCan_on_rx,
    .on_tx                   = &CallbacksCan_on_tx,
    .on_alias_change         = &CallbacksCan_on_alias_change,

};

static const openlcb_config_t openlcb_config = {

    .lock_shared_resources   = &Dspic33OlcbDrivers_lock_shared_resources,
    .unlock_shared_resources = &Dspic33OlcbDrivers_unlock_shared_resources,
    .config_mem_read         = &Dspic33OlcbDrivers_config_mem_read,
    .config_mem_write        = &Dspic33OlcbDrivers_config_mem_write,
    .reboot                  = &Dspic33OlcbDrivers_reboot,
    .factory_reset           = &CallbacksConfigMem_factory_reset,
    .freeze                  = &CallbacksConfigMem_freeze,
    .unfreeze                = NULL,
    .firmware_write          = NULL,
    .on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,

};

static void _initialize_io_early_for_test(void) {

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELE = 0x00;

    TEST_PIN_1401_TRIS = 0;
    TEST_PIN_1402_TRIS = 0;
    TEST_PIN_1403_TRIS = 0;
    TEST_PIN_1404_TRIS = 0;

}

/* Register application interrupt handlers into the Virtual Interrupt Vector
 * Table (VIVT) in shared persistent SRAM.
 *
 * The bootloader owns the hardware IVT at flash address 0x0004.  After the
 * bootloader jumps to this application, all interrupts still fire into the
 * bootloader's ISR stubs.  Each stub checks its corresponding function pointer
 * in bootloader_vivt_jumptable; if non-NULL it calls through to the handler
 * registered here.
 *
 * This must run before _GIE is enabled.  The bootloader disables GIE in
 * cleanup_before_handoff() before jumping, so we have a safe window to
 * populate the table.
 *
 * When running standalone (no bootloader), these pointers are unused -- the
 * MCC ISRs call the handlers directly via weak callback overrides or
 * hand-edits.  Registering them here is harmless and keeps the code ready
 * for either mode. */
static void _register_vivt_handlers(void) {

    bootloader_vivt_jumptable.timer_2_handler        = Dspic33OlcbDrivers_t2_interrupt_handler;
    bootloader_vivt_jumptable.can1_handler            = Dspic33CanDriver_c1_interrupt_handler;
    bootloader_vivt_jumptable.oscillatorfail_handler  = Traps_oscillator_fail_handler;
    bootloader_vivt_jumptable.addresserror_handler    = Traps_address_error_handler;
    bootloader_vivt_jumptable.stackerror_handler      = Traps_stack_error_handler;
    bootloader_vivt_jumptable.matherror_handler       = Traps_math_error_handler;
    bootloader_vivt_jumptable.dmacerror_handler       = Traps_dmac_error_handler;

}

int main(void) {

    _initialize_io_early_for_test();
    _register_vivt_handlers();

    CallbacksOlcb_initialize();

    SYSTEM_Initialize();
    Dspic33CanDriver_initialize();

    CanConfig_initialize(&can_config);
    OpenLcb_initialize(&openlcb_config);

    OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);

    // All setup, re-enable all the interrupts (important if a bootloader jump here was made)
    _GIE = 1;

    while (1) {

        OpenLcb_run();

    }

}
