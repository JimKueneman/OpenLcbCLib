/** @copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * @file main.cpp
 *
 * ESP32 bootloader entry point.  Wires the platform-specific CAN and OpenLCB
 * driver functions into the two DI structs required by the bootloader library
 * and calls Bootloader_entry(), which never returns.
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#include <Arduino.h>

#include "application_drivers/bootloader_drivers_can.h"
#include "application_drivers/bootloader_drivers_openlcb.h"
#include "openlcb_c_lib_bootloader/openlcb/bootloader.h"

void setup() {

    Serial.begin(921600);

    const bootloader_can_driver_t can_driver = {

        .read_received_frame = BootloaderDriversCan_read_received_frame,
        .try_send_frame      = BootloaderDriversCan_try_send_frame,
        .get_cached_alias    = BootloaderDriversCan_get_cached_alias_passed_from_application,

    };

    const bootloader_openlcb_driver_t openlcb_driver = {

        .get_persistent_node_id  = BootloaderDriversOpenlcb_get_persistent_node_id,
        .get_100ms_timer_tick    = BootloaderDriversOpenlcb_get_100ms_timer_tick,
        .set_status_led          = BootloaderDriversOpenlcb_set_status_led,
        .is_bootloader_requested = BootloaderDriversOpenlcb_is_bootloader_requested,
        .jump_to_application     = BootloaderDriversOpenlcb_jump_to_application,
        .reboot                  = BootloaderDriversOpenlcb_reboot,
        .initialize_hardware     = BootloaderDriversOpenlcb_initialize_hardware,
        .get_flash_boundaries    = BootloaderDriversOpenlcb_get_flash_boundaries,
        .get_flash_page_info     = BootloaderDriversOpenlcb_get_flash_page_info,
        .erase_flash_page        = BootloaderDriversOpenlcb_erase_flash_page,
        .write_flash_bytes       = BootloaderDriversOpenlcb_write_flash_bytes,
        .finalize_flash          = BootloaderDriversOpenlcb_finalize_flash,
        .compute_checksum        = BootloaderDriversOpenlcb_compute_checksum,
        .read_flash_bytes        = BootloaderDriversOpenlcb_read_flash_bytes,
        .cleanup_before_handoff  = BootloaderDriversOpenlcb_cleanup_before_handoff,

    };

    Bootloader_entry(&can_driver, &openlcb_driver);

}

void loop() {

    /* Bootloader_entry() never returns under normal operation. */

}
