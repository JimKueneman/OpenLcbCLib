/** @copyright
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
 * @file callbacks_config_mem.c
 *
 * Configuration memory callback implementations for the BasicNode application.
 *
 * The freeze callback implements the bootloader drop-back handshake: when the
 * Config Tool sends Freeze on the firmware space (0xEF), the application
 * caches its CAN alias in RTC NOINIT RAM, sets the magic request flag, and
 * calls esp_restart().  The bootloader detects the magic flag on startup and
 * enters firmware-update mode with the cached alias.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include "callbacks_config_mem.h"

#include <stdio.h>
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "../openlcb_c_lib/openlcb/openlcb_defines.h"
#include "../openlcb_c_lib/openlcb/openlcb_utilities.h"
#include "../../shared/bootloader_shared_ram.h"

void CallbacksConfigMem_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    printf("Factory Reset: NodeID = 0x%06llX\n", OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0));

}

void CallbacksConfigMem_firmware_write(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, write_result_t write_result) {

    printf("Firmware Write, buffer is in config_mem_write_request_info->writebuffer ");

    write_result(statemachine_info, config_mem_write_request_info, true);

}

void CallbacksConfigMem_freeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        /*
         * Cache the 12-bit CAN alias the application negotiated at startup.
         * The bootloader reuses it so the Config Tool sees the same alias
         * and can continue the firmware transfer without re-negotiation.
         */
        bootloader_shared_ram.cached_alias = statemachine_info->openlcb_node->alias;

        /*
         * Set the magic flag so the bootloader knows this was an application-
         * requested entry (not a power-on or button press).  It will start
         * with firmware_active = 1 and the Config Tool can send data
         * immediately without sending another Freeze.
         */
        bootloader_shared_ram.request_flag = BOOTLOADER_REQUEST_MAGIC;

        /*
         * Switch the OTA boot target to the bootloader partition (the
         * other OTA slot).  Without this call the ROM bootloader reboots
         * straight back into this application instead of the bootloader.
         *
         * esp_ota_get_next_update_partition(NULL) returns the inactive
         * OTA slot.  From the application's perspective that is the slot
         * containing the bootloader firmware.
         */
        const esp_partition_t *bootloader_part =
            esp_ota_get_next_update_partition(NULL);
        esp_ota_set_boot_partition(bootloader_part);

        /*
         * Reset the CPU.  RTC NOINIT memory survives esp_restart(), so
         * both shared RAM values are reliably visible when the bootloader
         * starts up.
         *
         * No peripheral teardown is needed -- esp_restart() handles the
         * shutdown sequence cleanly.
         */
        esp_restart();

    }

}

void CallbacksConfigMem_unfreeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        printf("Requesting Firmware firmware update complete, reboot");

    }

}
