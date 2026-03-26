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
 * @file bootloader_main.c
 *
 * Entry point for the MSPM0G3507 standalone bootloader.
 * Wires the platform-specific driver functions into the DI structs and
 * hands them to the bootloader entry point.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include "openlcb_c_lib_bootloader/openlcb/bootloader.h"
#include "application_drivers/bootloader_drivers_can.h"
#include "application_drivers/bootloader_drivers_openlcb.h"

int main(void) {

    /*
     * CAN physical layer driver.
     *
     * The bootloader communicates over CAN bus using the OpenLCB protocol.
     * These three functions abstract the CAN hardware so the bootloader
     * library works with any CAN peripheral (MCAN, ECAN, MCP2515, etc.).
     */
    const bootloader_can_driver_t can_driver = {

        /* Read one received CAN frame from the hardware RX FIFO.
         * Returns true if a frame was available (and fills *frame),
         * or false if the FIFO was empty. Called in a polling loop. */
        .read_received_frame = BootloaderDriversCan_read_received_frame,

        /* Attempt to transmit one CAN frame. Returns true if the frame
         * was accepted by the hardware TX FIFO, or false if the FIFO
         * was full (caller will retry on the next loop iteration). */
        .try_send_frame = BootloaderDriversCan_try_send_frame,

        /* Return the CAN alias that the application was using before it
         * dropped back into the bootloader. This lets the bootloader
         * skip alias negotiation and reuse the application's alias for
         * faster startup. Returns 0 if no alias was cached (cold boot),
         * in which case the bootloader negotiates a new one. */
        .get_cached_alias = BootloaderDriversCan_get_cached_alias_passed_from_application

    };

    /*
     * OpenLCB / hardware driver.
     *
     * These functions abstract everything the bootloader needs beyond
     * CAN: identity, timing, LEDs, flash memory, and lifecycle control.
     * Each function is platform-specific — this is where you adapt the
     * bootloader to your particular microcontroller.
     */
    const bootloader_openlcb_driver_t openlcb_driver = {

        /* Return the 48-bit OpenLCB Node ID for this device. Typically
         * read from a protected flash sector or EEPROM that was written
         * during production programming. Must be globally unique. */
        .get_persistent_node_id = BootloaderDriversOpenlcb_get_persistent_node_id,

        /* Return a free-running counter that increments once every 100ms.
         * Used for protocol timeouts (alias negotiation, datagram retries).
         * Typically driven by a SysTick or hardware timer interrupt. */
        .get_100ms_timer_tick = BootloaderDriversOpenlcb_get_100ms_timer_tick,

        /* Turn a status LED on or off. The 'led' parameter identifies
         * which logical LED function is being set (active, writing,
         * checksum error, etc.). A simple board can map all of them to
         * one physical LED; a fancier board can use separate LEDs. */
        .set_status_led = BootloaderDriversOpenlcb_set_status_led,

        /* Check whether the bootloader should stay in bootloader mode.
         * Returns BOOTLOADER_REQUESTED_BY_APP if the application set the
         * magic value in shared RAM before resetting — this means the CT
         * already sent Freeze, so the bootloader starts with firmware_active
         * set and PIP reports Firmware Upgrade Active immediately.
         * Returns BOOTLOADER_REQUESTED_BY_BUTTON if a hardware button is
         * held during power-on — the CT must send Freeze before transfer.
         * Returns BOOTLOADER_NOT_REQUESTED for normal boot (jump to app). */
        .is_bootloader_requested = BootloaderDriversOpenlcb_is_bootloader_requested,

        /* Jump to the application firmware. On Cortex-M this relocates
         * the vector table (VTOR), sets the stack pointer from the app's
         * vector table, and branches to the app's reset handler. On
         * dsPIC this is a function pointer call to the app start address.
         * This function never returns. */
        .jump_to_application = BootloaderDriversOpenlcb_jump_to_application,

        /* Perform a full chip reset. Called after a successful firmware
         * update so the bootloader restarts cleanly and validates the
         * new image. On Cortex-M this calls NVIC_SystemReset(). */
        .reboot = BootloaderDriversOpenlcb_reboot,

        /* One-time hardware setup: configure clocks, GPIO, CAN peripheral,
         * SysTick timer, and anything else the bootloader needs. Also
         * clears the bootloader_request_flag on power-on reset so random
         * SRAM garbage does not falsely trigger bootloader mode.
         * Called once at startup before anything else. */
        .initialize_hardware = BootloaderDriversOpenlcb_initialize_hardware,

        /* Return the boundaries of the application flash region and the
         * location of the app_header struct. The bootloader uses these
         * to know where it is allowed to erase/write and where to find
         * the checksum data embedded in the firmware image. */
        .get_flash_boundaries = BootloaderDriversOpenlcb_get_flash_boundaries,

        /* Given an arbitrary flash address, return the start address and
         * size of the flash page (sector) that contains it. The bootloader
         * needs this because flash must be erased one full page at a time,
         * and page sizes vary by chip (1KB on MSPM0, 2KB on STM32F4, etc.). */
        .get_flash_page_info = BootloaderDriversOpenlcb_get_flash_page_info,

        /* Erase one flash page (sector) at the given address. Returns 0
         * on success or an OpenLCB error code (ERROR_PERMANENT) on failure.
         * The bootloader calls this before writing new firmware data. */
        .erase_flash_page = BootloaderDriversOpenlcb_erase_flash_page,

        /* Write a block of data to flash at the given address. The data
         * may not be aligned to the chip's native write size, so this
         * function must handle padding if needed (e.g. pad to 8 bytes on
         * MSPM0). Returns 0 on success or an OpenLCB error code on failure. */
        .write_flash_bytes = BootloaderDriversOpenlcb_write_flash_bytes,

        /* Called after all firmware data has been written. Reads the
         * app_header from flash, computes checksums over the pre-header
         * and post-header regions, and compares them against the values
         * embedded in the header by the post-link tool. Returns 0 if the
         * checksums match (image is valid) or ERROR_PERMANENT if not. */
        .finalize_flash = BootloaderDriversOpenlcb_finalize_flash,

        /* Compute a checksum over a block of data. This is the algorithm
         * that finalize_flash uses internally — it is passed as a function
         * pointer so the library does not depend on a specific checksum
         * algorithm. The implementation can use CRC-16, CRC-32, SHA, etc.
         * as long as the post-link tool uses the same algorithm. */
        .compute_checksum = BootloaderDriversOpenlcb_compute_checksum,
        .read_flash_bytes = BootloaderDriversOpenlcb_read_flash_bytes,

        /* Tear down all peripherals and ARM core state (SysTick, NVIC,
         * MCAN) so the receiving binary starts from a clean slate.
         * Called by jump_to_application() before the jump, and by the
         * application's Freeze handler before setting the magic value
         * and resetting into the bootloader. */
        .cleanup_before_handoff = BootloaderDriversOpenlcb_cleanup_before_handoff

    };

    Bootloader_entry(&can_driver, &openlcb_driver);

    return 0;

}
