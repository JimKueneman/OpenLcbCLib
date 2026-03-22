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
 * \file openlcb_user_config.c
 *
 * Definition of the node at the application level.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "openlcb_user_config.h"
#include "openlcb_c_lib/openlcb/openlcb_types.h"
#include "openlcb_c_lib/openlcb/openlcb_defines.h"

const node_parameters_t OpenLcbUserConfig_node_parameters = {

    .snip = {
        .mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
        .name = "Basic OpenLcb Node",
        .model = "Test Application",
        .hardware_version = "0.0.1",
        .software_version = "0.0.1",
        .user_version = 2 // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO |
                         PSI_FIRMWARE_UPGRADE),

    .consumer_count_autocreate = 2,
    .producer_count_autocreate = 2,

    .configuration_options = {
        .write_under_mask_supported = true,
        .unaligned_reads_supported = true,
        .unaligned_writes_supported = true,
        .read_from_manufacturer_space_0xfc_supported = true,
        .read_from_user_space_0xfb_supported = true,
        .write_to_user_space_0xfb_supported = true,
        .stream_read_write_supported = false,
        .high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .low_address_space = CONFIG_MEM_SPACE_FIRMWARE,
        .description = "These are options that defined the memory space capabilities"
    },

    // Space 0xFF
    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so
    //    make sure the CDI maps these 2 items to the first 128 bytes as well
    .address_space_configuration_definition = {
        .present = true,
        .read_only = true,
        .low_address_valid = false, // assume the low address starts at 0
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .highest_address = (1081 - 1), // length of the .cdi file byte array contents        .low_address = 0, // ignored if low_address_valid is false
        .description = "Configuration definition info"
    },

    // Space 0xFE
    .address_space_all = {
        .present = true,
        .read_only = true,
        .low_address_valid = true,
        .address_space = CONFIG_MEM_SPACE_ALL,
        .highest_address = 0xFFFF,
        .low_address = 0x1234,
        .description = "All memory Info"
    },

    // Space 0xFD
    .address_space_config_memory = {
        .present = true,
        .read_only = false,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .highest_address = (0x0200 - 1),
        .low_address = 0,
        .description = "Configuration memory storage"
    },

    // Space 0xFC
    .address_space_acdi_manufacturer = {
        .present = true,
        .read_only = true,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,
        .highest_address = (125 - 1), // Zero indexed: 1 + 41 + 41 + 21 + 21 = 125
        .low_address = 0,
        .description = "ACDI access manufacturer"
    },

    // Space 0xFB
    .address_space_acdi_user = {
        .present = true,
        .read_only = false,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,
        .highest_address = (128 - 1), // Zero indexed: 1 + 63 + 64 = 128 bytes
        .low_address = 0,
        .description = "ACDI access user storage"
    },

    // Space 0xEF
    .address_space_firmware = {
        .present = true,
        .read_only = false,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_FIRMWARE,
        .highest_address = 0xFFFFFFFF,
        .low_address = 0,
        .description = "Firmware update address space"
    },

    .cdi = NULL,

    .fdi = NULL,

};
