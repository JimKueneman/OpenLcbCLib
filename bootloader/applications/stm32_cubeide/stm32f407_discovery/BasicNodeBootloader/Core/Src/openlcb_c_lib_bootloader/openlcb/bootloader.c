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

#ifndef NO_CHECKSUM

static bool _check_application_checksum(void) {

    const void *flash_min = NULL;
    const void *flash_max = NULL;
    const bootloader_app_header_t *app_header = NULL;

    _openlcb_driver->get_flash_boundaries(&flash_min, &flash_max, &app_header);

    /* Copy the header to RAM via the DI flash-read so Harvard architectures
     * (e.g. dsPIC33) can use table reads instead of data-pointer dereference. */
    bootloader_app_header_t header_copy;
    _openlcb_driver->read_flash_bytes((uint32_t)(uintptr_t) app_header, &header_copy, sizeof(header_copy));

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

#endif /* NO_CHECKSUM */

/* ====================================================================== */
/* Public API                                                              */
/* ====================================================================== */

bool Bootloader_init(const bootloader_can_driver_t *can_driver, const bootloader_openlcb_driver_t *openlcb_driver) {

    _openlcb_driver = openlcb_driver;

    bootloader_request_t request = openlcb_driver->is_bootloader_requested();
    openlcb_driver->set_status_led(BOOTLOADER_LED_REQUEST, request != BOOTLOADER_NOT_REQUESTED);

    if (request == BOOTLOADER_NOT_REQUESTED) {

#ifndef NO_CHECKSUM
        bool csum_ok = _check_application_checksum();
        openlcb_driver->set_status_led(BOOTLOADER_LED_CSUM_ERROR, !csum_ok);
#else
        bool csum_ok = true;
        openlcb_driver->set_status_led(BOOTLOADER_LED_CSUM_ERROR, false);
#endif

        if (csum_ok) {

            const void *flash_min = NULL;
            const void *flash_max = NULL;
            const bootloader_app_header_t *app_header = NULL;
            openlcb_driver->get_flash_boundaries(&flash_min, &flash_max, &app_header);

            /* Read the first word via the DI so Harvard architectures use
             * table reads.  Erased flash reads as 0xFFFFFFFF on all targets. */
            uint32_t first_word = 0;
            openlcb_driver->read_flash_bytes((uint32_t)(uintptr_t) flash_min, &first_word, sizeof(first_word));

            if (first_word != 0xFFFFFFFF) {

                openlcb_driver->cleanup_before_handoff();
                openlcb_driver->jump_to_application();
                /* jump_to_application() never returns on a successful jump. */

            }
            /* Flash is blank — no app present, fall through to bootloader mode. */

        }

    }

    memset(&bootloader_state, 0, sizeof(bootloader_state));

    /* If the application sent Freeze and then dropped back to us, the CT
     * is already waiting for data transfer — we start with firmware_active
     * so PIP reports Firmware Upgrade Active immediately.  For button or
     * no-valid-app entry, the CT must send Freeze first. */
    if (request == BOOTLOADER_REQUESTED_BY_APP) {

        bootloader_state.firmware_active = 1;

    }

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
