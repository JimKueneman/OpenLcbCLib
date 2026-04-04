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
 * @file bootloader_openlcb_statemachine.c
 *
 * OpenLCB protocol state machine for the standalone bootloader.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "bootloader_openlcb_statemachine.h"
#include "bootloader_types.h"
#include "../drivers/canbus/bootloader_can_statemachine.h"

/* ====================================================================== */
/* Internal state                                                          */
/* ====================================================================== */

static const bootloader_openlcb_driver_t *_openlcb_driver;

/* True when the current write offset falls in the binary pre-app skip region.
 * _flush_write_buffer() returns success immediately without touching flash. */
static bool _skip_write = false;

/* ====================================================================== */
/* Flash write buffer management                                           */
/* ====================================================================== */

static void _init_write_buffer(void) {

    memset(bootloader_state.write_buffer, 0xFF, BOOTLOADER_WRITE_BUFFER_SIZE);
    bootloader_state.write_buffer_index = 0;

}

static uint16_t _flush_write_buffer(void) {

    if (_skip_write) {

        _skip_write = false;
        _init_write_buffer();
        return 0;

    }

    uint32_t address    = bootloader_state.write_buffer_offset;
    uint32_t page_start = 0;
    uint32_t page_length = 0;

    _openlcb_driver->get_flash_page_info(address, &page_start, &page_length);

    /* Read current flash content and skip if it already matches.
     * This makes CT retries safe -- a retry re-sends the same data for an
     * address that was already programmed.  Without this check, writing
     * already-programmed flash words causes undefined behavior on some
     * targets (e.g. MSPM0 flash controller hangs permanently).
     *
     * The check must happen BEFORE the erase -- if the address is
     * page-aligned, erasing first would destroy the data we need to
     * compare against. */
    uint8_t flash_read_buf[BOOTLOADER_WRITE_BUFFER_SIZE];
    _openlcb_driver->read_flash_bytes(address, flash_read_buf,
            bootloader_state.write_buffer_index);

    if (memcmp(flash_read_buf, bootloader_state.write_buffer,
            bootloader_state.write_buffer_index) == 0) {

        bootloader_state.write_buffer_offset += bootloader_state.write_buffer_index;
        _init_write_buffer();
        return 0;

    }

    if (page_start == address) {

        _openlcb_driver->set_status_led(BOOTLOADER_LED_WRITING, true);
        uint16_t erase_result = _openlcb_driver->erase_flash_page(address);

        if (erase_result != 0) {

            _openlcb_driver->set_status_led(BOOTLOADER_LED_WRITING, false);
            return erase_result;

        }

    }

    uint16_t write_result = _openlcb_driver->write_flash_bytes(
            address,
            bootloader_state.write_buffer,
            bootloader_state.write_buffer_index);

    _openlcb_driver->set_status_led(BOOTLOADER_LED_WRITING, false);

    if (write_result != 0) { return write_result; }

    bootloader_state.write_buffer_offset += bootloader_state.write_buffer_index;
    _init_write_buffer();

    return 0;

}

static bool _normalize_write_offset(void) {

    uint32_t flash_min     = 0;
    uint32_t flash_max     = 0;
    uint32_t app_header    = 0;

    _openlcb_driver->get_flash_boundaries(&flash_min, &flash_max, &app_header);

    _skip_write = false;

    if (bootloader_state.write_buffer_offset >= (flash_max - flash_min)) {

        _skip_write = true;   /* past end of flash -- ACK without writing */
        return true;

    }

    bootloader_state.write_buffer_offset += flash_min;
    _init_write_buffer();

    return true;

}

/* ====================================================================== */
/* Big-endian helpers                                                       */
/* ====================================================================== */

static uint32_t _load_uint32_be(const uint8_t *byte_array) {

    return ((uint32_t) byte_array[0] << 24) |
            ((uint32_t) byte_array[1] << 16) |
            ((uint32_t) byte_array[2] << 8) |
            (uint32_t) byte_array[3];

}

/* ====================================================================== */
/* Memory Configuration command handler                                    */
/* ====================================================================== */

static void _handle_memconfig(uint16_t src_alias, uint64_t src_node_id, const uint8_t *data, uint8_t payload_length) {

    if (payload_length < 2) {

        BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_INVALID_ARGUMENTS);
        return;

    }

    uint8_t command = data[1];

    switch (command) {

        case CONFIG_MEM_FREEZE:

            if (payload_length < 3 || data[2] != CONFIG_MEM_SPACE_FIRMWARE) {

                BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_INVALID_ARGUMENTS);
                return;

            }

            bootloader_state.firmware_active = 1;
            bootloader_state.write_buffer_offset = 0;
            bootloader_state.write_buffer_index = 0;
            bootloader_state.write_src_alias = src_alias;
            _init_write_buffer();

            BootloaderCanSM_send_datagram_ok(src_alias, src_node_id);

            {

                uint64_t node_id = _openlcb_driver->get_persistent_node_id();
                uint8_t node_id_bytes[6];
                node_id_bytes[0] = (node_id >> 40) & 0xFF;
                node_id_bytes[1] = (node_id >> 32) & 0xFF;
                node_id_bytes[2] = (node_id >> 24) & 0xFF;
                node_id_bytes[3] = (node_id >> 16) & 0xFF;
                node_id_bytes[4] = (node_id >> 8) & 0xFF;
                node_id_bytes[5] = node_id & 0xFF;

                BootloaderCanSM_send_global(MTI_INITIALIZATION_COMPLETE, node_id_bytes, 6);

            }

            return;

        case CONFIG_MEM_WRITE_SPACE_IN_BYTE_6:
        {

            if (payload_length < 7) {

                BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_INVALID_ARGUMENTS);
                return;

            }

            uint8_t space = data[6];

            if (space != CONFIG_MEM_SPACE_FIRMWARE) {

                BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);
                return;

            }

            bootloader_state.write_buffer_offset = _load_uint32_be(data + 2);

            if (!_normalize_write_offset()) {

                BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);
                return;

            }

            uint8_t data_start = 7;
            uint8_t write_byte_count = payload_length - data_start;

            if (write_byte_count > BOOTLOADER_WRITE_BUFFER_SIZE) {

                write_byte_count = BOOTLOADER_WRITE_BUFFER_SIZE;

            }

            memcpy(bootloader_state.write_buffer, data + data_start, write_byte_count);
            bootloader_state.write_buffer_index = write_byte_count;

            BootloaderCanSM_send_datagram_ok(src_alias, src_node_id);

            uint16_t write_result = _flush_write_buffer();

            if (write_result == 0) {

                uint8_t reply[7];
                reply[0] = CONFIG_MEM_CONFIGURATION;
                reply[1] = CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
                reply[2] = data[2];
                reply[3] = data[3];
                reply[4] = data[4];
                reply[5] = data[5];
                reply[6] = CONFIG_MEM_SPACE_FIRMWARE;

                BootloaderCanSM_send_datagram(src_alias, src_node_id, reply, 7);

            } else {

                uint8_t reply[9];
                reply[0] = CONFIG_MEM_CONFIGURATION;
                reply[1] = CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
                reply[2] = data[2];
                reply[3] = data[3];
                reply[4] = data[4];
                reply[5] = data[5];
                reply[6] = CONFIG_MEM_SPACE_FIRMWARE;
                reply[7] = (write_result >> 8) & 0xFF;
                reply[8] = write_result & 0xFF;

                BootloaderCanSM_send_datagram(src_alias, src_node_id, reply, 9);

            }

            return;

        }

        case CONFIG_MEM_UNFREEZE:

            if (payload_length < 3 || data[2] != CONFIG_MEM_SPACE_FIRMWARE) {

                BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_INVALID_ARGUMENTS);
                return;

            }

            {

                uint16_t result = _openlcb_driver->finalize_flash(_openlcb_driver->compute_checksum);

                if (result != 0) {

                    bootloader_state.firmware_active = 0;
                    BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, result);
                    return;

                }

            }

            bootloader_state.firmware_active = 0;
            BootloaderCanSM_send_datagram_ok(src_alias, src_node_id);
            bootloader_state.request_reset = 1;
            return;

        case CONFIG_MEM_RESET_REBOOT:

            BootloaderCanSM_send_datagram_ok(src_alias, src_node_id);
            bootloader_state.request_reset = 1;
            return;

        case CONFIG_MEM_OPTIONS_CMD:
        {

            uint8_t reply[7];

            reply[0] = CONFIG_MEM_CONFIGURATION;
            reply[1] = CONFIG_MEM_OPTIONS_REPLY;
            reply[2] = 0x00;
            reply[3] = 0x00;
            reply[4] = CONFIG_OPTIONS_WRITE_LENGTH_RESERVED;
            reply[5] = CONFIG_MEM_SPACE_FIRMWARE;
            reply[6] = CONFIG_MEM_SPACE_FIRMWARE;

            BootloaderCanSM_send_datagram_ok(src_alias, src_node_id);
            BootloaderCanSM_send_datagram(src_alias, src_node_id, reply, 7);
            return;

        }

        case CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD:
        {

            if (payload_length < 3) {

                BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_INVALID_ARGUMENTS);
                return;

            }

            BootloaderCanSM_send_datagram_ok(src_alias, src_node_id);

            if (data[2] != CONFIG_MEM_SPACE_FIRMWARE) {

                uint8_t reply[3];

                reply[0] = CONFIG_MEM_CONFIGURATION;
                reply[1] = CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT;
                reply[2] = data[2];

                BootloaderCanSM_send_datagram(src_alias, src_node_id, reply, 3);
                return;

            }

            uint8_t reply[8];

            reply[0] = CONFIG_MEM_CONFIGURATION;
            reply[1] = CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT;
            reply[2] = CONFIG_MEM_SPACE_FIRMWARE;
            reply[3] = 0xFF;
            reply[4] = 0xFF;
            reply[5] = 0xFF;
            reply[6] = 0xFF;
            reply[7] = 0x00;

            BootloaderCanSM_send_datagram(src_alias, src_node_id, reply, 8);
            return;

        }

        default:

            BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);
            return;

    }

}

/* ====================================================================== */
/* PIP reply builder                                                       */
/* ====================================================================== */

static void _handle_pip(uint16_t src_alias, uint64_t src_node_id) {

    uint64_t pip_value = PSI_DATAGRAM | PSI_MEMORY_CONFIGURATION | PSI_SIMPLE_NODE_INFORMATION;

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

    BootloaderCanSM_send_addressed(MTI_PROTOCOL_SUPPORT_REPLY, src_alias, src_node_id, pip_data, 6);

}

/* ====================================================================== */
/* SNIP reply builder                                                      */
/* ====================================================================== */

static void _handle_snip(uint16_t src_alias, uint64_t src_node_id) {

    uint8_t snip_buf[72];
    uint8_t write_position = 0;

    snip_buf[write_position++] = 4;

    const char *manufacturer = BOOTLOADER_SNIP_MANUFACTURER;

    while (*manufacturer && write_position < 70) {

        snip_buf[write_position++] = *manufacturer++;

    }

    snip_buf[write_position++] = 0;

    const char *model = BOOTLOADER_SNIP_MODEL;

    while (*model && write_position < 70) {

        snip_buf[write_position++] = *model++;

    }

    snip_buf[write_position++] = 0;

    const char *hardware_str = BOOTLOADER_SNIP_HW_VERSION;

    while (*hardware_str && write_position < 70) {

        snip_buf[write_position++] = *hardware_str++;

    }

    snip_buf[write_position++] = 0;

    const char *software_str = BOOTLOADER_SNIP_SW_VERSION;

    while (*software_str && write_position < 70) {

        snip_buf[write_position++] = *software_str++;

    }

    snip_buf[write_position++] = 0;

    snip_buf[write_position++] = 2;

    const char *user_name = BOOTLOADER_SNIP_USER_NAME;

    while (*user_name && write_position < 71) {

        snip_buf[write_position++] = *user_name++;

    }

    snip_buf[write_position++] = 0;

    const char *user_description = BOOTLOADER_SNIP_USER_DESCRIPTION;

    while (*user_description && write_position < 71) {

        snip_buf[write_position++] = *user_description++;

    }

    snip_buf[write_position++] = 0;

    BootloaderCanSM_send_addressed(MTI_SIMPLE_NODE_INFO_REPLY, src_alias, src_node_id, snip_buf, write_position);

}

/* ====================================================================== */
/* Verify Node ID                                                          */
/* ====================================================================== */

static void _send_verified_node_id(void) {

    uint8_t node_data[6];
    uint64_t node_id = _openlcb_driver->get_persistent_node_id();

    node_data[0] = (node_id >> 40) & 0xFF;
    node_data[1] = (node_id >> 32) & 0xFF;
    node_data[2] = (node_id >> 24) & 0xFF;
    node_data[3] = (node_id >> 16) & 0xFF;
    node_data[4] = (node_id >> 8) & 0xFF;
    node_data[5] = node_id & 0xFF;

    BootloaderCanSM_send_global(MTI_VERIFIED_NODE_ID, node_data, 6);

}

/* ====================================================================== */
/* Public callbacks                                                        */
/* ====================================================================== */

void BootloaderOpenlcbSM_init(const bootloader_openlcb_driver_t *openlcb_driver) {

    _openlcb_driver = openlcb_driver;

}

void BootloaderOpenlcbSM_on_datagram_received(uint16_t src_alias, uint64_t src_node_id, const uint8_t *data, uint8_t payload_length) {

    if (payload_length < 1) { return; }

    if (data[0] == CONFIG_MEM_CONFIGURATION) {

        _handle_memconfig(src_alias, src_node_id, data, payload_length);

    } else {

        BootloaderCanSM_send_datagram_rejected(src_alias, src_node_id, ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL);

    }

}

void BootloaderOpenlcbSM_on_addressed_message(uint16_t mti, uint16_t src_alias, uint64_t src_node_id, const uint8_t *data, uint8_t payload_length) {

    (void) data;
    (void) payload_length;

    switch (mti) {

        case MTI_PROTOCOL_SUPPORT_INQUIRY:

            _handle_pip(src_alias, src_node_id);
            break;

        case MTI_SIMPLE_NODE_INFO_REQUEST:

            _handle_snip(src_alias, src_node_id);
            break;

        case MTI_VERIFY_NODE_ID_ADDRESSED:

            _send_verified_node_id();
            break;

        case MTI_DATAGRAM_OK_REPLY:

            break;

        default:
        {

            uint8_t oir_data[4];

            oir_data[0] = (ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL >> 8) & 0xFF;
            oir_data[1] = ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL & 0xFF;
            oir_data[2] = (mti >> 8) & 0xFF;
            oir_data[3] = mti & 0xFF;

            BootloaderCanSM_send_addressed(MTI_OPTIONAL_INTERACTION_REJECTED, src_alias, src_node_id, oir_data, 4);
            break;

        }

    }

}

void BootloaderOpenlcbSM_on_global_message(uint16_t mti, const uint8_t *data, uint8_t payload_length) {

    switch (mti) {

        case MTI_VERIFY_NODE_ID_GLOBAL:

            if (payload_length == 0) {

                _send_verified_node_id();

            } else if (payload_length >= 6) {

                uint64_t node_id = _openlcb_driver->get_persistent_node_id();
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
