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
 * \file callbacks_config_mem.c
 *
 * Configuration memory callback implementations for the BasicNode demo.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include "callbacks_config_mem.h"

#include <stdio.h>

#include "../openlcb_c_lib/openlcb/openlcb_utilities.h"
#include "../openlcb_c_lib/openlcb/openlcb_defines.h"
#include "../../../../shared/bootloader_shared_ram.h"
#include "main.h"

void CallbacksConfigMem_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    printf("Factory Reset: NodeID = 0x%06llX\n",
            OpenLcbUtilities_extract_node_id_from_openlcb_payload(
                    statemachine_info->incoming_msg_info.msg_ptr, 0));

}

    /**
     * @brief Drops back to the bootloader when a firmware-space Freeze is received.
     *
     * @verbatim
     * @param statemachine_info                  Pointer to openlcb_statemachine_info_t context.
     * @param config_mem_operations_request_info Pointer to config_mem_operations_request_info_t describing the Freeze request.
     * @endverbatim
     */
void CallbacksConfigMem_freeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        /* Disable all interrupts so no ISR can fire between the shared RAM
         * writes below and the system reset.  Without this a CAN RX interrupt
         * could corrupt state or process a stale message while the handshake
         * variables are half-written.  We never re-enable -- the reset fires
         * with interrupts off. */
        __disable_irq();

        /* Cache the 12-bit CAN alias the application negotiated at startup.
         * The bootloader reuses it so the Config Tool sees the same alias
         * and can continue the firmware transfer without re-negotiation. */
        bootloader_shared_ram.cached_alias = statemachine_info->openlcb_node->alias;

        /* Set the magic flag so the bootloader knows this was an application-
         * requested entry (not a power-on or button press).  It will start
         * with firmware_active = 1 and the Config Tool can send data
         * immediately without sending another Freeze. */
        bootloader_shared_ram.request_flag = BOOTLOADER_REQUEST_MAGIC;

        /* Reset the entire CPU and all peripherals.  The HAL wrapper includes
         * a __DSB() barrier that flushes the write buffer before the reset
         * fires, so both shared RAM values are reliably visible afterward.
         *
         * No peripheral teardown (HAL_CAN_DeInit, SysTick stop, etc.) is
         * needed here -- NVIC_SystemReset resets all peripherals to their
         * power-on state.  __disable_irq() above is sufficient to prevent
         * any ISR from running in the window before the reset completes. */
        HAL_NVIC_SystemReset();

    }

}
