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
 * \file node_parameters.c
 *
 * Definition of the node at the application level.  
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#ifdef MPLAB
#include "turnoutboss_bootloader_node_parameters.h"
#include "../../../openlcb/openlcb_defines.h"
#include "../TurnoutBossCommon/turnoutboss_version.h"
#else
#include "node_parameters.h"
#include "src/openlcb/openlcb_defines.h"
#endif


const node_parameters_t NodeParameters_main_node = {

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Mustangpeak Engineering",
    .snip.model = "TurnoutBoss Bootloader",
    .snip.hardware_version = "Ver 2",
    .snip.software_version = TURNOUTBOSS_VERSION,
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (
    PSI_DATAGRAM |
    PSI_MEMORY_CONFIGURATION |
    PSI_SIMPLE_NODE_INFORMATION |
    PSI_FIRMWARE_UPGRADE
    ),

    .configuration_options.high_address_space = ADDRESS_SPACE_FIRMWARE,
    .configuration_options.low_address_space = ADDRESS_SPACE_FIRMWARE,
    .configuration_options.read_from_manufacturer_space_0xfc_supported = 0,
    .configuration_options.read_from_user_space_0xfb_supported = 0,
    .configuration_options.stream_read_write_supported = 0,
    .configuration_options.unaligned_reads_supported = 1,
    .configuration_options.unaligned_writes_supported = 1,
    .configuration_options.write_to_user_space_0xfb_supported = 1,
    .configuration_options.write_under_mask_supported = 0,
    .configuration_options.description = "",

    // Space 0xFF 
    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so 
    //    make sure the CDI maps these 2 items to the first 128 bytes as well
    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 0,
    .address_space_configuration_definition.low_address_valid = 0, // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0, // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 0, // this is the length of the .cdi file byte array in this initialization structure
    .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0, // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = ADDRESS_SPACE_ALL,
    .address_space_all.description = "",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0, // assume the low address starts at 0
    .address_space_config_memory.low_address = 0, // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "",

    // Space 0xFC
    .address_space_acdi_manufacturer.read_only = 1,
    .address_space_acdi_manufacturer.present = 0,
    .address_space_acdi_manufacturer.low_address_valid = 0, // assume the low address starts at 0
    .address_space_acdi_manufacturer.low_address = 0, // ignored if low_address_valid is false
    .address_space_acdi_manufacturer.highest_address = 0, // Predefined in the Configuration Description Definition Spec
    .address_space_acdi_manufacturer.address_space = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS,
    .address_space_acdi_manufacturer.description = "",

    // Space 0xFB
    .address_space_acdi_user.read_only = 0,
    .address_space_acdi_user.present = 0,
    .address_space_acdi_user.low_address_valid = 0, // assume the low address starts at 0
    .address_space_acdi_user.low_address = 0, // ignored if low_address_valid is false
    .address_space_acdi_user.highest_address = 0, // Predefined in the Configuration Description Definition Spec
    .address_space_acdi_user.address_space = ADDRESS_SPACE_ACDI_USER_ACCESS,
    .address_space_acdi_user.description = "",

    // Space 0xFB
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
    .address_space_firmware.low_address_valid = 0, // assume the low address starts at 0
    .address_space_firmware.low_address = 0, // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0xFFFFFFFF, 
    .address_space_firmware.address_space = ADDRESS_SPACE_FIRMWARE,
    .address_space_firmware.description = "",
    
                
    .cdi =
    {

    },


};