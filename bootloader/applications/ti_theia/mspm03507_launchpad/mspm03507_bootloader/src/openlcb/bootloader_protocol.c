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
 * @file bootloader_protocol.c
 *
 * Protocol message handlers for the standalone bootloader.
 * Handles PIP, SNIP, Verify Node ID, Memory Configuration commands
 * (FREEZE, WRITE, UNFREEZE, RESET, Get Options, Get Address Space Info).
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "bootloader_protocol.h"
#include "bootloader_types.h"
#include "bootloader_transport.h"
#include "bootloader_driver.h"
#include "../crc/bootloader_app_header.h"

/* ====================================================================== */
/* Flash write buffer management                                           */
/* ====================================================================== */

static void _init_write_buffer(void) {

    memset(bootloader_state.write_buffer, 0xFF, BOOTLOADER_WRITE_BUFFER_SIZE);
    bootloader_state.write_buffer_index = 0;

}

static void _flush_write_buffer(void) {

    const void *address = (const void *) (uintptr_t) bootloader_state.write_buffer_offset;
    const void *page_start = NULL;
    uint32_t page_length = 0;

    BootloaderDriver_get_flash_page_info(address, &page_start, &page_length);

    if (page_start == address) {

        BootloaderDriver_led(BOOTLOADER_LED_WRITING, true);
        BootloaderDriver_erase_flash_page(address);

    }

    BootloaderDriver_write_flash(
            address,
            bootloader_state.write_buffer,
            bootloader_state.write_buffer_index);

    BootloaderDriver_led(BOOTLOADER_LED_WRITING, false);

    bootloader_state.write_buffer_offset += bootloader_state.write_buffer_index;
    _init_write_buffer();

}

static bool _normalize_write_offset(void) {

    const void *flash_min = NULL;
    const void *flash_max = NULL;
    const bootloader_app_header_t *app_header = NULL;

    BootloaderDriver_get_flash_boundaries(&flash_min, &flash_max, &app_header);

    if (bootloader_state.write_buffer_offset >=
            ((uintptr_t) flash_max - (uintptr_t) flash_min)) {

        return false;

    }

    bootloader_state.write_buffer_offset += (uintptr_t) flash_min;
    _init_write_buffer();

    return true;

}

/* ====================================================================== */
/* Big-endian helpers                                                       */
/* ====================================================================== */

static uint32_t _load_uint32_be(const uint8_t *ptr) {

    return ((uint32_t) ptr[0] << 24) |
            ((uint32_t) ptr[1] << 16) |
            ((uint32_t) ptr[2] << 8) |
            (uint32_t) ptr[3];

}

/* ====================================================================== */
/* Memory Configuration command handler                                    */
/* ====================================================================== */

static void _handle_memconfig(uint16_t src_alias, const uint8_t *data, uint8_t len) {

    if (len < 2) {

        BootloaderTransport_send_datagram_rejected(src_alias, ERROR_PERMANENT_INVALID_ARGUMENTS);
        return;

    }

    uint8_t command = data[1];

    switch (command) {

        case CONFIG_MEM_FREEZE:

            if (len < 3 || data[2] != CONFIG_MEM_SPACE_FIRMWARE) {

                BootloaderTransport_send_datagram_rejected(
                        src_alias, ERROR_PERMANENT_INVALID_ARGUMENTS);
                return;

            }

            /* Reset write state for new firmware session. */
            bootloader_state.firmware_active = 1;
            bootloader_state.write_buffer_offset = 0;
            bootloader_state.write_buffer_index = 0;
            bootloader_state.write_src_alias = src_alias;
            _init_write_buffer();

            BootloaderTransport_send_datagram_ok(src_alias);
            return;

        case CONFIG_MEM_WRITE_SPACE_IN_BYTE_6:
        {

            if (len < 7) {

                BootloaderTransport_send_datagram_rejected(
                        src_alias, ERROR_PERMANENT_INVALID_ARGUMENTS);
                return;

            }

            /* Check space byte — must be firmware (0xEF). */
            uint8_t space = data[6];

            if (space != CONFIG_MEM_SPACE_FIRMWARE) {

                BootloaderTransport_send_datagram_rejected(
                        src_alias, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);
                return;

            }

            /* Extract address. */
            bootloader_state.write_buffer_offset = _load_uint32_be(data + 2);

            if (!_normalize_write_offset()) {

                BootloaderTransport_send_datagram_rejected(
                        src_alias, ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);
                return;

            }

            /* Copy data bytes into write buffer. */
            uint8_t data_start = 7;
            uint8_t data_len = len - data_start;

            if (data_len > BOOTLOADER_WRITE_BUFFER_SIZE) {

                data_len = BOOTLOADER_WRITE_BUFFER_SIZE;

            }

            memcpy(bootloader_state.write_buffer, data + data_start, data_len);
            bootloader_state.write_buffer_index = data_len;

            /* Flush and acknowledge. */
            _flush_write_buffer();
            BootloaderTransport_send_datagram_ok(src_alias);
            return;

        }

        case CONFIG_MEM_UNFREEZE:

            if (len < 3 || data[2] != CONFIG_MEM_SPACE_FIRMWARE) {

                BootloaderTransport_send_datagram_rejected(
                        src_alias, ERROR_PERMANENT_INVALID_ARGUMENTS);
                return;

            }

            {

                uint16_t result = BootloaderDriver_flash_complete();

                if (result != 0) {

                    BootloaderTransport_send_datagram_rejected(src_alias, result);
                    return;

                }

            }

            bootloader_state.firmware_active = 0;
            BootloaderTransport_send_datagram_ok(src_alias);
            bootloader_state.request_reset = 1;
            return;

        case CONFIG_MEM_RESET_REBOOT:

            BootloaderTransport_send_datagram_ok(src_alias);
            bootloader_state.request_reset = 1;
            return;

        case CONFIG_MEM_OPTIONS_CMD:
        {

            /* Reply with supported capabilities. */
            uint8_t reply[7];

            reply[0] = CONFIG_MEM_CONFIGURATION;
            reply[1] = 0x82;   /* Get Options reply */
            reply[2] = 0x00;   /* Available commands high: write */
            reply[3] = 0x00;   /* Available commands low */
            reply[4] = 0x00;   /* Write lengths */
            reply[5] = CONFIG_MEM_SPACE_FIRMWARE;  /* Highest space */
            reply[6] = CONFIG_MEM_SPACE_FIRMWARE;  /* Lowest space */

            BootloaderTransport_send_datagram_ok(src_alias);
            BootloaderTransport_send_datagram(src_alias, reply, 7);
            return;

        }

        case CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD:
        {

            if (len < 3) {

                BootloaderTransport_send_datagram_rejected(
                        src_alias, ERROR_PERMANENT_INVALID_ARGUMENTS);
                return;

            }

            if (data[2] != CONFIG_MEM_SPACE_FIRMWARE) {

                BootloaderTransport_send_datagram_rejected(
                        src_alias, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);
                return;

            }

            /* Get flash boundaries for size info. */
            const void *flash_min = NULL;
            const void *flash_max = NULL;
            const bootloader_app_header_t *app_header = NULL;

            BootloaderDriver_get_flash_boundaries(&flash_min, &flash_max, &app_header);

            uint32_t highest_addr = (uint32_t) ((uintptr_t) flash_max - (uintptr_t) flash_min);

            uint8_t reply[8];

            reply[0] = CONFIG_MEM_CONFIGURATION;
            reply[1] = CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT;
            reply[2] = CONFIG_MEM_SPACE_FIRMWARE;
            reply[3] = (highest_addr >> 24) & 0xFF;
            reply[4] = (highest_addr >> 16) & 0xFF;
            reply[5] = (highest_addr >> 8) & 0xFF;
            reply[6] = highest_addr & 0xFF;
            reply[7] = 0x02;  /* Flags: writable (bit 1) */

            BootloaderTransport_send_datagram_ok(src_alias);
            BootloaderTransport_send_datagram(src_alias, reply, 8);
            return;

        }

        default:

            BootloaderTransport_send_datagram_rejected(
                    src_alias, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);
            return;

    }

}

/* ====================================================================== */
/* PIP reply builder                                                       */
/* ====================================================================== */

static void _handle_pip(uint16_t src_alias) {

    uint64_t pip_value = PSI_DATAGRAM |
            PSI_MEMORY_CONFIGURATION |
            PSI_SIMPLE_NODE_INFORMATION;

    if (bootloader_state.firmware_active) {

        pip_value |= PSI_FIRMWARE_UPGRADE_ACTIVE;

    } else {

        pip_value |= PSI_FIRMWARE_UPGRADE;

    }

    uint8_t pip_data[6];

    pip_data[0] = (pip_value >> 16) & 0xFF;
    pip_data[1] = (pip_value >> 8) & 0xFF;
    pip_data[2] = pip_value & 0xFF;
    pip_data[3] = 0;
    pip_data[4] = 0;
    pip_data[5] = 0;

    BootloaderTransport_send_addressed(
            MTI_PROTOCOL_SUPPORT_REPLY,
            src_alias, pip_data, 6);

}

/* ====================================================================== */
/* SNIP reply builder                                                      */
/* ====================================================================== */

static void _handle_snip(uint16_t src_alias) {

    uint8_t snip_buf[72];
    uint8_t pos = 0;

    /* Version 4 for manufacturer info. */
    snip_buf[pos++] = 4;

    /* Manufacturer name. */
    const char *mfg = BOOTLOADER_SNIP_MANUFACTURER;

    while (*mfg && pos < 70) { snip_buf[pos++] = *mfg++; }

    snip_buf[pos++] = 0;

    /* Model name. */
    const char *model = BOOTLOADER_SNIP_MODEL;

    while (*model && pos < 70) { snip_buf[pos++] = *model++; }

    snip_buf[pos++] = 0;

    /* Hardware version. */
    const char *hw = BOOTLOADER_SNIP_HW_VERSION;

    while (*hw && pos < 70) { snip_buf[pos++] = *hw++; }

    snip_buf[pos++] = 0;

    /* Software version. */
    const char *sw = BOOTLOADER_SNIP_SW_VERSION;

    while (*sw && pos < 70) { snip_buf[pos++] = *sw++; }

    snip_buf[pos++] = 0;

    /* Version 2 for user info. */
    snip_buf[pos++] = 2;

    /* User name. */
    const char *uname = BOOTLOADER_SNIP_USER_NAME;

    while (*uname && pos < 71) { snip_buf[pos++] = *uname++; }

    snip_buf[pos++] = 0;

    /* User description. */
    const char *udesc = BOOTLOADER_SNIP_USER_DESCRIPTION;

    while (*udesc && pos < 71) { snip_buf[pos++] = *udesc++; }

    snip_buf[pos++] = 0;

    BootloaderTransport_send_addressed(
            MTI_SIMPLE_NODE_INFO_REPLY,
            src_alias, snip_buf, pos);

}

/* ====================================================================== */
/* Verify Node ID                                                          */
/* ====================================================================== */

static void _send_verified_node_id(void) {

    uint8_t node_data[6];
    uint64_t node_id = BootloaderDriver_node_id();

    node_data[0] = (node_id >> 40) & 0xFF;
    node_data[1] = (node_id >> 32) & 0xFF;
    node_data[2] = (node_id >> 24) & 0xFF;
    node_data[3] = (node_id >> 16) & 0xFF;
    node_data[4] = (node_id >> 8) & 0xFF;
    node_data[5] = node_id & 0xFF;

    BootloaderTransport_send_global(
            MTI_VERIFIED_NODE_ID,
            node_data, 6);

}

/* ====================================================================== */
/* Transport callbacks                                                     */
/* ====================================================================== */

void BootloaderProtocol_on_datagram_received(
        uint16_t src_alias,
        const uint8_t *data,
        uint8_t len) {

    if (len < 1) { return; }

    if (data[0] == CONFIG_MEM_CONFIGURATION) {

        _handle_memconfig(src_alias, data, len);

    } else {

        BootloaderTransport_send_datagram_rejected(
                src_alias, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);

    }

}

void BootloaderProtocol_on_addressed_message(
        uint16_t mti,
        uint16_t src_alias,
        const uint8_t *data,
        uint8_t len) {

    switch (mti) {

        case MTI_PROTOCOL_SUPPORT_INQUIRY:

            _handle_pip(src_alias);
            break;

        case MTI_SIMPLE_NODE_INFO_REQUEST:

            _handle_snip(src_alias);
            break;

        case MTI_VERIFY_NODE_ID_ADDRESSED:

            _send_verified_node_id();
            break;

        case MTI_DATAGRAM_OK_REPLY:

            /* Acknowledge received for our outgoing datagram — nothing to do. */
            break;

        default:
        {

            /* §3.3.4 / §3.5.1: OIR data = error code (2 bytes) + MTI (2 bytes). */
            uint8_t oir_data[4];

            oir_data[0] = (ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL >> 8) & 0xFF;
            oir_data[1] = ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL & 0xFF;
            oir_data[2] = (mti >> 8) & 0xFF;
            oir_data[3] = mti & 0xFF;

            BootloaderTransport_send_addressed(
                    MTI_OPTIONAL_INTERACTION_REJECTED,
                    src_alias, oir_data, 4);
            break;

        }

    }

}

void BootloaderProtocol_on_global_message(
        uint16_t mti,
        const uint8_t *data,
        uint8_t len) {

    switch (mti) {

        case MTI_VERIFY_NODE_ID_GLOBAL:

            if (len == 0) {

                /* No Node ID — reply unconditionally (§3.4.2 line 130). */
                _send_verified_node_id();

            } else if (len >= 6) {

                /* Optional Node ID present — reply only if match (§3.4.2 line 132). */
                uint64_t node_id = BootloaderDriver_node_id();
                uint64_t received_id =
                        ((uint64_t) data[0] << 40) |
                        ((uint64_t) data[1] << 32) |
                        ((uint64_t) data[2] << 24) |
                        ((uint64_t) data[3] << 16) |
                        ((uint64_t) data[4] << 8) |
                        (uint64_t) data[5];

                if (received_id == node_id) {

                    _send_verified_node_id();

                }

            }

            break;

        default:

            break;

    }

}
