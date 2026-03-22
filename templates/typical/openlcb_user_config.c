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
 * @date 18 Mar 2026
 */

#include "openlcb_user_config.h"
#include "openlcb_c_lib/openlcb/openlcb_types.h"
#include "openlcb_c_lib/openlcb/openlcb_defines.h"

const node_parameters_t OpenLcbUserConfig_node_parameters = {

    // 1. snip
    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Basic OpenLcb Node",
    .snip.model = "Test Application",
    .snip.hardware_version = "0.0.1",
    .snip.software_version = "0.0.1",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    // 2. protocol_support
    .protocol_support = (PSI_DATAGRAM |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO |
                         PSI_FIRMWARE_UPGRADE),

    // 3-4. consumer_count_autocreate / producer_count_autocreate
    // For internal testing only do not set to anything but 0
    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    // 5. configuration_options
    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .configuration_options.read_from_manufacturer_space_0xfc_supported = true,
    .configuration_options.read_from_user_space_0xfb_supported = true,
    .configuration_options.stream_read_write_supported = false,
    .configuration_options.unaligned_reads_supported = true,
    .configuration_options.unaligned_writes_supported = true,
    .configuration_options.write_to_user_space_0xfb_supported = true,
    .configuration_options.write_under_mask_supported = true,
    .configuration_options.description = "",

    // 6. Space 0xFF
    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so
    //    make sure the CDI maps these 2 items to the first 128 bytes as well
    .address_space_configuration_definition.read_only = true,
    .address_space_configuration_definition.present = true,
    .address_space_configuration_definition.low_address_valid = false,    // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,              // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = (1081 - 1), // length of the .cdi file byte array contents
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "",

    // 7. Space 0xFE
    .address_space_all.read_only = true,
    .address_space_all.present = true,
    .address_space_all.low_address_valid = true,
    .address_space_all.low_address = 0x1234,
    .address_space_all.highest_address = 0xFFFF,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "",

    // 8. Space 0xFD
    .address_space_config_memory.read_only = false,
    .address_space_config_memory.present = true,
    .address_space_config_memory.low_address_valid = false,
    .address_space_config_memory.low_address = 0,
    .address_space_config_memory.highest_address = (0x0200 - 1),
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "",

    // 9. Space 0xFC
    .address_space_acdi_manufacturer.read_only = true,
    .address_space_acdi_manufacturer.present = true,
    .address_space_acdi_manufacturer.low_address_valid = false,
    .address_space_acdi_manufacturer.low_address = 0,
    .address_space_acdi_manufacturer.highest_address = (125 - 1), // Zero indexed: 1 + 41 + 41 + 21 + 21 = 125
    .address_space_acdi_manufacturer.address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,
    .address_space_acdi_manufacturer.description = "",

    // 10. Space 0xFB
    .address_space_acdi_user.read_only = false,
    .address_space_acdi_user.present = true,
    .address_space_acdi_user.low_address_valid = false,
    .address_space_acdi_user.low_address = 0,
    .address_space_acdi_user.highest_address = (128 - 1), // Zero indexed: 1 + 63 + 64 = 128 bytes
    .address_space_acdi_user.address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,
    .address_space_acdi_user.description = "",

    // 11. Space 0xFA - Train Function Definition Information (not used in typical node)
    .address_space_train_function_definition_info.read_only = true,
    .address_space_train_function_definition_info.present = false,
    .address_space_train_function_definition_info.low_address_valid = false,
    .address_space_train_function_definition_info.low_address = 0,
    .address_space_train_function_definition_info.highest_address = 0,
    .address_space_train_function_definition_info.address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO,
    .address_space_train_function_definition_info.description = "",

    // 12. Space 0xF9 - Train Function Configuration Memory (not used in typical node)
    .address_space_train_function_config_memory.read_only = false,
    .address_space_train_function_config_memory.present = false,
    .address_space_train_function_config_memory.low_address_valid = false,
    .address_space_train_function_config_memory.low_address = 0,
    .address_space_train_function_config_memory.highest_address = 0,
    .address_space_train_function_config_memory.address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIG_MEMORY,
    .address_space_train_function_config_memory.description = "",

    // 13. Space 0xEF
    .address_space_firmware.read_only = false,
    .address_space_firmware.present = true,
    .address_space_firmware.low_address_valid = false,
    .address_space_firmware.low_address = 0,
    .address_space_firmware.highest_address = 0xFFFFFFFF,
    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .address_space_firmware.description = "",

    // 14. cdi — pointer to a const uint8_t array defined above (or NULL if unused)
    .cdi = NULL,

    // 15. fdi — pointer to a const uint8_t array defined above (or NULL if unused)
    .fdi = NULL,

};
