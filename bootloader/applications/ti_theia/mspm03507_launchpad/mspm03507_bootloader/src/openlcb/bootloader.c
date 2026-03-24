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
 * @file bootloader.c
 *
 * Main entry point, boot decision logic, and main loop for the standalone
 * bootloader. This is the protocol-layer top level.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "bootloader.h"
#include "bootloader_types.h"
#include "bootloader_openlcb_statemachine.h"
#include "../drivers/canbus/bootloader_can_statemachine.h"

/** Global bootloader state shared with bootloader_openlcb_statemachine.c. */
bootloader_state_t bootloader_state;

/** Stored OpenLCB driver pointer for use by loop and checksum. */
static const bootloader_openlcb_driver_t *_openlcb_driver;

/* ====================================================================== */
/* Application checksum validation                                         */
/* ====================================================================== */

static bool _check_application_checksum(void) {

    const void *flash_min = NULL;
    const void *flash_max = NULL;
    const bootloader_app_header_t *app_header = NULL;

    _openlcb_driver->get_flash_boundaries(&flash_min, &flash_max, &app_header);

    /* Copy the header to RAM so we can compare after computing checksums. */
    bootloader_app_header_t header_copy;
    memcpy(&header_copy, app_header, sizeof(header_copy));

    /* Validate app_size is reasonable. */
    uint32_t flash_size = (uint32_t) ((uintptr_t) flash_max - (uintptr_t) flash_min);

    if (header_copy.app_size > flash_size) { return false; }

    /* Pre-checksum: flash_min to app_header. */
    uint32_t pre_size = (uint32_t) ((const uint8_t *) app_header - (const uint8_t *) flash_min);

    uint32_t checksum[BOOTLOADER_CHECKSUM_COUNT];
    memset(checksum, 0, sizeof(checksum));

    _openlcb_driver->compute_checksum(flash_min, pre_size, checksum);

    if (memcmp(header_copy.checksum_pre, checksum, sizeof(checksum)) != 0) {

        return false;

    }

    /* Post-checksum: after app_header to app_size. */
    uint32_t post_offset = (uint32_t) (sizeof(bootloader_app_header_t) + pre_size);
    uint32_t post_size = 0;

    if (post_offset < header_copy.app_size) {

        post_size = header_copy.app_size - post_offset;

    }

    memset(checksum, 0, sizeof(checksum));
    _openlcb_driver->compute_checksum((const uint8_t *) app_header + sizeof(bootloader_app_header_t), post_size, checksum);

    if (memcmp(header_copy.checksum_post, checksum, sizeof(checksum)) != 0) {

        return false;

    }

    return true;

}

/* ====================================================================== */
/* Public API                                                              */
/* ====================================================================== */

bool Bootloader_init(const bootloader_can_driver_t *can_driver, const bootloader_openlcb_driver_t *openlcb_driver) {

    _openlcb_driver = openlcb_driver;

    bool request = openlcb_driver->is_bootloader_requested();
    openlcb_driver->set_status_led(BOOTLOADER_LED_REQUEST, request);

    if (!request) {

        bool csum_ok = _check_application_checksum();
        openlcb_driver->set_status_led(BOOTLOADER_LED_CSUM_ERROR, !csum_ok);

        if (csum_ok) {

            openlcb_driver->jump_to_application();
            return true;

        }

    }

    memset(&bootloader_state, 0, sizeof(bootloader_state));
    BootloaderCanSM_init(can_driver, openlcb_driver);
    BootloaderOpenlcbSM_init(openlcb_driver);

    return false;

}

bool Bootloader_loop(void) {

    BootloaderCanSM_loop();

    if (bootloader_state.request_reset && BootloaderCanSM_is_initialized()) {

        _openlcb_driver->reboot();
        return true;

    }

    _openlcb_driver->set_status_led(BOOTLOADER_LED_ACTIVE, !BootloaderCanSM_is_initialized());

    return false;

}

void Bootloader_entry(const bootloader_can_driver_t *can_driver, const bootloader_openlcb_driver_t *openlcb_driver) {

    openlcb_driver->initialize_hardware();

    if (Bootloader_init(can_driver, openlcb_driver)) { return; }

    while (true) {

        if (Bootloader_loop()) { return; }

    }

}
