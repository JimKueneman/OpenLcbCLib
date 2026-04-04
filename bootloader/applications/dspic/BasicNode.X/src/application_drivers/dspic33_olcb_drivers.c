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
 * @file dspic33_olcb_drivers.c
 *
 * General platform drivers for the dsPIC33EP512MC506 BasicNode demo.
 *
 * @author Jim Kueneman
 * @date 19 Mar 2026
 */

#include "dspic33_olcb_drivers.h"
#include "dspic33_can_drivers.h"

#include "xc.h"
#include <string.h>

#include "../openlcb_c_lib/openlcb/openlcb_config.h"

void Dspic33OlcbDrivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    asm("RESET ");

}


uint16_t Dspic33OlcbDrivers_config_mem_read(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {

    memset(*buffer, 0x00, count);

    switch (address) {

        case 0:
        {

            const char str[] = "dsPIC33E256MC506";
            uint16_t len = (count < sizeof(str)) ? count : sizeof(str);
            memcpy(*buffer, str, len);
            break;

        }

        default:

            break;

    }

    return count;

}

uint16_t Dspic33OlcbDrivers_config_mem_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer) {

    return count;

}

void Dspic33OlcbDrivers_lock_shared_resources(void) {

    Dspic33CanDriver_pause_can_rx();

}

void Dspic33OlcbDrivers_unlock_shared_resources(void) {

    Dspic33CanDriver_resume_can_rx();

}

/* 100ms timer handler -- called two different ways:
 *
 *   Standalone:  MCC's _T2Interrupt (in tmr2.c) calls TMR2_CallBack() via
 *                a function pointer.  TMR2_CallBack is declared __weak__ in
 *                tmr2.c; this override replaces it.  MCC's ISR clears T2IF.
 *
 *   Bootloaded:  TMR2_CallBack is NOT used.  The bootloader owns the
 *                hardware IVT so _T2Interrupt never runs.  Instead, the
 *                bootloader's T2 ISR stub reads
 *                bootloader_vivt_jumptable.timer_2_handler and calls
 *                Dspic33OlcbDrivers_t2_interrupt_handler() through that
 *                pointer.  main() registers it in the VIVT before enabling
 *                interrupts.  This function must clear T2IF itself because
 *                MCC's _T2Interrupt is not in the call path. */
void TMR2_CallBack(void) {

    OpenLcbConfig_100ms_timer_tick();

}

void Dspic33OlcbDrivers_t2_interrupt_handler(void) {

    IFS0bits.T2IF = 0;

    OpenLcbConfig_100ms_timer_tick();

}
