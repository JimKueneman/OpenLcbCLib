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
#include "node_parameters.h"
#include "../../../openlcb/openlcb_defines.h"
#else
#include "node_parameters.h"
#include "src/openlcb/openlcb_defines.h"
#endif


const node_parameters_t NodeParameters_main_node = {

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Mustangpeak Engineering",
    .snip.model = "TurnoutBoss",
    .snip.hardware_version = "Ver A",
    .snip.software_version = "0.1",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (
#ifdef SUPPORT_FIRMWARE_BOOTLOADER 
    PSI_FIRMWARE_UPGRADE |
#endif
    PSI_DATAGRAM |
    PSI_MEMORY_CONFIGURATION |
    PSI_EVENT_EXCHANGE |
    PSI_EVENT_EXCHANGE |
    PSI_ABBREVIATED_DEFAULT_CDI |
    PSI_SIMPLE_NODE_INFORMATION |
    PSI_CONFIGURATION_DESCRIPTION_INFO
    ),

    .configuration_options.high_address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY,
    .configuration_options.read_from_manufacturer_space_0xfc_supported = 1,
    .configuration_options.read_from_user_space_0xfb_supported = 1,
    .configuration_options.stream_read_write_supported = 0,
    .configuration_options.unaligned_reads_supported = 1,
    .configuration_options.unaligned_writes_supported = 1,
    .configuration_options.write_to_user_space_0xfb_supported = 1,
    .configuration_options.write_under_mask_supported = 1,
    .configuration_options.description = "These are options that defined the memory space capabilities",

    // Space 0xFF 
    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so 
    //    make sure the CDI maps these 2 items to the first 128 bytes as well
    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 1,
    .address_space_configuration_definition.low_address_valid = 0, // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0, // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 9378, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
    .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0, // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = ADDRESS_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 1,
    .address_space_config_memory.low_address_valid = 0, // assume the low address starts at 0
    .address_space_config_memory.low_address = 0, // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 700, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xFC
    .address_space_acdi_manufacturer.read_only = 1,
    .address_space_acdi_manufacturer.present = 1,
    .address_space_acdi_manufacturer.low_address_valid = 0, // assume the low address starts at 0
    .address_space_acdi_manufacturer.low_address = 0, // ignored if low_address_valid is false
    .address_space_acdi_manufacturer.highest_address = 125, // Predefined in the Configuration Description Definition Spec
    .address_space_acdi_manufacturer.address_space = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS,
    .address_space_acdi_manufacturer.description = "ACDI access manufacturer",

    // Space 0xFB
    .address_space_acdi_user.read_only = 0,
    .address_space_acdi_user.present = 1,
    .address_space_acdi_user.low_address_valid = 0, // assume the low address starts at 0
    .address_space_acdi_user.low_address = 0, // ignored if low_address_valid is false
    .address_space_acdi_user.highest_address = 128, // Predefined in the Configuration Description Definition Spec
    .address_space_acdi_user.address_space = ADDRESS_SPACE_ACDI_USER_ACCESS,
    .address_space_acdi_user.description = "ACDI access user storage",

#ifdef SUPPORT_FIRMWARE_BOOTLOADER
    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
    .address_space_firmware.low_address_valid = 0, // assume the low address starts at 0
    .address_space_firmware.low_address = 0, // Firmware ALWAYS assumes it starts at 0
    .address_space_firmware.highest_address = 0xFFFFFFFF, // Predefined in the Configuration Description Definition Spec
    .address_space_firmware.address_space = ADDRESS_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware update address space",

#endif

    .cdi =
    {
        0X3C, 0X3F, 0X78, 0X6D, 0X6C, 0X20, 0X76, 0X65, 0X72, 0X73, 0X69, 0X6F, 0X6E, 0X3D, 0X22, 0X31, 0X2E, 0X30, 0X22, 0X20, 0X65, 0X6E, 0X63, 0X6F, 0X64, 0X69, 0X6E, 0X67, 0X3D, 0X22, 0X55, 0X54, 0X46, 0X2D, 0X38, 0X22, 0X3F, 0X3E,    // <?xml version="1.0" encoding="UTF-8"?>
        0X3C, 0X3F, 0X78, 0X6D, 0X6C, 0X2D, 0X73, 0X74, 0X79, 0X6C, 0X65, 0X73, 0X68, 0X65, 0X65, 0X74, 0X20, 0X74, 0X79, 0X70, 0X65, 0X3D, 0X22, 0X74, 0X65, 0X78, 0X74, 0X2F, 0X78, 0X73, 0X6C, 0X22, 0X20, 0X68, 0X72, 0X65, 0X66, 0X3D, 0X22, 0X68, 0X74, 0X74, 0X70, 0X3A, 0X2F, 0X2F, 0X6F, 0X70, 0X65, 0X6E, 0X6C, 0X63, 0X62, 0X2E, 0X6F, 0X72, 0X67, 0X2F, 0X74, 0X72, 0X75, 0X6E, 0X6B, 0X2F, 0X70, 0X72, 0X6F, 0X74, 0X6F, 0X74, 0X79, 0X70, 0X65, 0X73, 0X2F, 0X78, 0X6D, 0X6C, 0X2F, 0X78, 0X73, 0X6C, 0X74, 0X2F, 0X63, 0X64, 0X69, 0X2E, 0X78, 0X73, 0X6C, 0X22, 0X3F, 0X3E,    // <?xml-stylesheet type="text/xsl" href="http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl"?>
        0X3C, 0X63, 0X64, 0X69, 0X20, 0X78, 0X6D, 0X6C, 0X6E, 0X73, 0X3A, 0X78, 0X73, 0X69, 0X3D, 0X22, 0X68, 0X74, 0X74, 0X70, 0X3A, 0X2F, 0X2F, 0X77, 0X77, 0X77, 0X2E, 0X77, 0X33, 0X2E, 0X6F, 0X72, 0X67, 0X2F, 0X32, 0X30, 0X30, 0X31, 0X2F, 0X58, 0X4D, 0X4C, 0X53, 0X63, 0X68, 0X65, 0X6D, 0X61, 0X2D, 0X69, 0X6E, 0X73, 0X74, 0X61, 0X6E, 0X63, 0X65, 0X22, 0X20, 0X78, 0X73, 0X69, 0X3A, 0X6E, 0X6F, 0X4E, 0X61, 0X6D, 0X65, 0X73, 0X70, 0X61, 0X63, 0X65, 0X53, 0X63, 0X68, 0X65, 0X6D, 0X61, 0X4C, 0X6F, 0X63, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3D, 0X22, 0X68, 0X74, 0X74, 0X70, 0X73, 0X3A, 0X2F, 0X2F, 0X6F, 0X70, 0X65, 0X6E, 0X6C, 0X63, 0X62, 0X2E, 0X6F, 0X72, 0X67, 0X2F, 0X73, 0X63, 0X68, 0X65, 0X6D, 0X61, 0X2F, 0X63, 0X64, 0X69, 0X2F, 0X31, 0X2F, 0X34, 0X2F, 0X63, 0X64, 0X69, 0X2E, 0X78, 0X73, 0X64, 0X22, 0X3E,    // <cdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="https://openlcb.org/schema/cdi/1/4/cdi.xsd">
        0X3C, 0X69, 0X64, 0X65, 0X6E, 0X74, 0X69, 0X66, 0X69, 0X63, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <identification>
        0X3C, 0X6D, 0X61, 0X6E, 0X75, 0X66, 0X61, 0X63, 0X74, 0X75, 0X72, 0X65, 0X72, 0X3E, 0X4D, 0X75, 0X73, 0X74, 0X61, 0X6E, 0X67, 0X70, 0X65, 0X61, 0X6B, 0X20, 0X45, 0X6E, 0X67, 0X69, 0X6E, 0X65, 0X65, 0X72, 0X69, 0X6E, 0X67, 0X3C, 0X2F, 0X6D, 0X61, 0X6E, 0X75, 0X66, 0X61, 0X63, 0X74, 0X75, 0X72, 0X65, 0X72, 0X3E,    // <manufacturer>Mustangpeak Engineering</manufacturer>
        0X3C, 0X6D, 0X6F, 0X64, 0X65, 0X6C, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X3C, 0X2F, 0X6D, 0X6F, 0X64, 0X65, 0X6C, 0X3E,    // <model>TurnoutBoss</model>
        0X3C, 0X68, 0X61, 0X72, 0X64, 0X77, 0X61, 0X72, 0X65, 0X56, 0X65, 0X72, 0X73, 0X69, 0X6F, 0X6E, 0X3E, 0X56, 0X65, 0X72, 0X20, 0X41, 0X3C, 0X2F, 0X68, 0X61, 0X72, 0X64, 0X77, 0X61, 0X72, 0X65, 0X56, 0X65, 0X72, 0X73, 0X69, 0X6F, 0X6E, 0X3E,    // <hardwareVersion>Ver A</hardwareVersion>
        0X3C, 0X73, 0X6F, 0X66, 0X74, 0X77, 0X61, 0X72, 0X65, 0X56, 0X65, 0X72, 0X73, 0X69, 0X6F, 0X6E, 0X3E, 0X30, 0X2E, 0X32, 0X3C, 0X2F, 0X73, 0X6F, 0X66, 0X74, 0X77, 0X61, 0X72, 0X65, 0X56, 0X65, 0X72, 0X73, 0X69, 0X6F, 0X6E, 0X3E,    // <softwareVersion>0.2</softwareVersion>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X44, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>Description</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X4D, 0X75, 0X73, 0X74, 0X61, 0X6E, 0X67, 0X70, 0X65, 0X61, 0X6B, 0X20, 0X54, 0X65, 0X73, 0X74, 0X20, 0X4E, 0X6F, 0X64, 0X65, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Mustangpeak Test Node</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X53, 0X74, 0X61, 0X74, 0X75, 0X73, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>Status</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X50, 0X72, 0X6F, 0X74, 0X6F, 0X74, 0X79, 0X70, 0X65, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Prototype</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X2F, 0X69, 0X64, 0X65, 0X6E, 0X74, 0X69, 0X66, 0X69, 0X63, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </identification>
        0X3C, 0X61, 0X63, 0X64, 0X69, 0X2F, 0X3E,    // <acdi/>
        0X3C, 0X73, 0X65, 0X67, 0X6D, 0X65, 0X6E, 0X74, 0X20, 0X6F, 0X72, 0X69, 0X67, 0X69, 0X6E, 0X3D, 0X22, 0X30, 0X22, 0X20, 0X73, 0X70, 0X61, 0X63, 0X65, 0X3D, 0X22, 0X32, 0X35, 0X33, 0X22, 0X3E,    // <segment origin="0" space="253">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X4C, 0X61, 0X79, 0X6F, 0X75, 0X74, 0X20, 0X43, 0X6F, 0X6E, 0X66, 0X69, 0X67, 0X75, 0X72, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X53, 0X65, 0X74, 0X75, 0X70, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Layout Configuration Setup</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X54, 0X68, 0X65, 0X20, 0X62, 0X61, 0X73, 0X69, 0X63, 0X20, 0X69, 0X6E, 0X66, 0X6F, 0X72, 0X6D, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X72, 0X65, 0X71, 0X75, 0X69, 0X72, 0X65, 0X64, 0X20, 0X74, 0X6F, 0X20, 0X67, 0X65, 0X74, 0X20, 0X79, 0X6F, 0X75, 0X72, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X20, 0X75, 0X70,    // <description>The basic information required to get your TurnoutBoss up
        0X61, 0X6E, 0X64, 0X20, 0X6F, 0X70, 0X65, 0X72, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X61, 0X6C, 0X20, 0X74, 0X6F, 0X20, 0X63, 0X72, 0X65, 0X61, 0X74, 0X65, 0X20, 0X61, 0X20, 0X66, 0X75, 0X6C, 0X6C, 0X79, 0X20, 0X73, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X65, 0X64, 0X20, 0X6C, 0X61, 0X79, 0X6F, 0X75, 0X74, 0X2E, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // and operational to create a fully signaled layout.</description>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X55, 0X73, 0X65, 0X72, 0X20, 0X49, 0X6E, 0X66, 0X6F, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>User Info</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X45, 0X6E, 0X74, 0X65, 0X72, 0X20, 0X61, 0X20, 0X6E, 0X61, 0X6D, 0X65, 0X20, 0X61, 0X6E, 0X64, 0X20, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X74, 0X6F, 0X20, 0X68, 0X65, 0X6C, 0X70, 0X20, 0X75, 0X6E, 0X69, 0X71, 0X75, 0X65, 0X6C, 0X79, 0X20, 0X69, 0X64, 0X65, 0X6E, 0X74, 0X69, 0X66, 0X79, 0X20, 0X74, 0X68, 0X69, 0X73, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X2E, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>Enter a name and description to help uniquely identify this TurnoutBoss.</description>
        0X3C, 0X73, 0X74, 0X72, 0X69, 0X6E, 0X67, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X36, 0X33, 0X22, 0X3E,    // <string size="63">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X55, 0X73, 0X65, 0X72, 0X20, 0X4E, 0X61, 0X6D, 0X65, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>User Name</name>
        0X3C, 0X2F, 0X73, 0X74, 0X72, 0X69, 0X6E, 0X67, 0X3E,    // </string>
        0X3C, 0X73, 0X74, 0X72, 0X69, 0X6E, 0X67, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X36, 0X34, 0X22, 0X3E,    // <string size="64">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X55, 0X73, 0X65, 0X72, 0X20, 0X44, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>User Description</name>
        0X3C, 0X2F, 0X73, 0X74, 0X72, 0X69, 0X6E, 0X67, 0X3E,    // </string>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X48, 0X6F, 0X77, 0X20, 0X74, 0X68, 0X69, 0X73, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X20, 0X69, 0X73, 0X20, 0X75, 0X73, 0X65, 0X64, 0X20, 0X6F, 0X6E, 0X20, 0X79, 0X6F, 0X75, 0X72, 0X20, 0X6C, 0X61, 0X79, 0X6F, 0X75, 0X74, 0X2E, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>How this TurnoutBoss is used on your layout.</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X53, 0X65, 0X6C, 0X65, 0X63, 0X74, 0X20, 0X63, 0X6F, 0X6E, 0X66, 0X69, 0X67, 0X75, 0X72, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X6F, 0X66, 0X20, 0X74, 0X68, 0X65, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X2E, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>Select configuration of the TurnoutBoss.</description>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X31, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>1</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X30, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>0</default>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X30, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>0</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X22, 0X4C, 0X65, 0X66, 0X74, 0X22, 0X20, 0X62, 0X6F, 0X61, 0X72, 0X64, 0X3A, 0X20, 0X74, 0X68, 0X72, 0X65, 0X65, 0X20, 0X62, 0X6C, 0X6F, 0X63, 0X6B, 0X73, 0X20, 0X28, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X2C, 0X20, 0X6D, 0X61, 0X69, 0X6E, 0X6C, 0X69, 0X6E, 0X65, 0X20, 0X62, 0X65, 0X74, 0X77, 0X65, 0X65, 0X6E, 0X2C, 0X20, 0X73, 0X69, 0X64, 0X69, 0X6E, 0X67, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>"Left" board: three blocks (turnout, mainline between, siding)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X31, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>1</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X22, 0X52, 0X69, 0X67, 0X68, 0X74, 0X22, 0X20, 0X62, 0X6F, 0X61, 0X72, 0X64, 0X3A, 0X20, 0X74, 0X77, 0X6F, 0X20, 0X62, 0X6C, 0X6F, 0X63, 0X6B, 0X73, 0X20, 0X28, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X2C, 0X20, 0X6D, 0X61, 0X69, 0X6E, 0X6C, 0X69, 0X6E, 0X65, 0X20, 0X74, 0X6F, 0X20, 0X6E, 0X65, 0X78, 0X74, 0X20, 0X73, 0X69, 0X64, 0X69, 0X6E, 0X67, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>"Right" board: two blocks (turnout, mainline to next siding)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // <hints>
        0X3C, 0X72, 0X61, 0X64, 0X69, 0X6F, 0X62, 0X75, 0X74, 0X74, 0X6F, 0X6E, 0X2F, 0X3E,    // <radiobutton/>
        0X3C, 0X2F, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // </hints>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X4C, 0X61, 0X79, 0X6F, 0X75, 0X74, 0X20, 0X43, 0X6F, 0X6E, 0X6E, 0X65, 0X63, 0X74, 0X69, 0X6F, 0X6E, 0X73, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Layout Connections</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X20, 0X4E, 0X6F, 0X64, 0X65, 0X20, 0X49, 0X44, 0X73, 0X20, 0X61, 0X64, 0X6A, 0X61, 0X63, 0X65, 0X6E, 0X74, 0X20, 0X74, 0X6F, 0X20, 0X74, 0X68, 0X69, 0X73, 0X20, 0X75, 0X6E, 0X69, 0X74, 0X2E, 0X20, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>TurnoutBoss Node IDs adjacent to this unit. </description>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X4C, 0X65, 0X66, 0X74, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Left</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X4E, 0X6F, 0X64, 0X65, 0X49, 0X44, 0X20, 0X6F, 0X66, 0X20, 0X74, 0X68, 0X65, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X20, 0X74, 0X6F, 0X20, 0X69, 0X6D, 0X6D, 0X65, 0X64, 0X69, 0X61, 0X74, 0X65, 0X20, 0X6C, 0X65, 0X66, 0X74, 0X20, 0X6F, 0X66, 0X20, 0X74, 0X68, 0X69, 0X73, 0X20, 0X75, 0X6E, 0X69, 0X74, 0X2E, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>NodeID of the TurnoutBoss to immediate left of this unit.</description>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X52, 0X69, 0X67, 0X68, 0X74, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Right</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X4E, 0X6F, 0X64, 0X65, 0X49, 0X44, 0X20, 0X6F, 0X66, 0X20, 0X74, 0X68, 0X65, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X20, 0X74, 0X6F, 0X20, 0X69, 0X6D, 0X6D, 0X65, 0X64, 0X69, 0X61, 0X74, 0X65, 0X20, 0X72, 0X69, 0X67, 0X68, 0X74, 0X20, 0X6F, 0X66, 0X20, 0X74, 0X68, 0X69, 0X73, 0X20, 0X75, 0X6E, 0X69, 0X74, 0X2E, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>NodeID of the TurnoutBoss to immediate right of this unit.</description>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X43, 0X6F, 0X6E, 0X74, 0X72, 0X6F, 0X6C, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Control</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X44, 0X65, 0X66, 0X69, 0X6E, 0X65, 0X20, 0X68, 0X6F, 0X77, 0X20, 0X74, 0X68, 0X69, 0X73, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X27, 0X73, 0X20, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X63, 0X6F, 0X6E, 0X74, 0X72, 0X6F, 0X6C, 0X2F, 0X66, 0X65, 0X65, 0X64, 0X62, 0X61, 0X63, 0X6B, 0X20, 0X69, 0X73, 0X20, 0X77, 0X69, 0X72, 0X65, 0X64, 0X20, 0X74, 0X6F, 0X20, 0X74, 0X68, 0X65, 0X20, 0X6C, 0X61, 0X79, 0X6F, 0X75, 0X74, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>Define how this TurnoutBoss's turnout control/feedback is wired to the layout</description>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X43, 0X6F, 0X6E, 0X74, 0X72, 0X6F, 0X6C, 0X20, 0X42, 0X75, 0X74, 0X74, 0X6F, 0X6E, 0X73, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Control Buttons</name>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X31, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>1</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X30, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>0</default>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X30, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>0</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X77, 0X6F, 0X20, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X63, 0X6F, 0X6E, 0X74, 0X72, 0X6F, 0X6C, 0X20, 0X62, 0X75, 0X74, 0X74, 0X6F, 0X6E, 0X73, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Two turnout control buttons</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X31, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>1</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X4F, 0X6E, 0X65, 0X20, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X63, 0X6F, 0X6E, 0X74, 0X72, 0X6F, 0X6C, 0X20, 0X62, 0X75, 0X74, 0X74, 0X6F, 0X6E, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>One turnout control button</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X46, 0X65, 0X65, 0X64, 0X62, 0X61, 0X63, 0X6B, 0X20, 0X53, 0X65, 0X6E, 0X73, 0X6F, 0X72, 0X73, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Feedback Sensors</name>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X32, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>2</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X30, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>0</default>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X30, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>0</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X55, 0X6E, 0X75, 0X73, 0X65, 0X64, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Unused</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X31, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>1</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X4F, 0X6E, 0X65, 0X20, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X70, 0X6F, 0X73, 0X69, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X73, 0X65, 0X6E, 0X73, 0X6F, 0X72, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>One turnout position sensor</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X32, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>2</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X77, 0X6F, 0X20, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X66, 0X65, 0X65, 0X64, 0X62, 0X61, 0X63, 0X6B, 0X20, 0X73, 0X65, 0X6E, 0X73, 0X6F, 0X72, 0X73, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Two turnout feedback sensors</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X68, 0X65, 0X61, 0X64, 0X20, 0X4F, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X73, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signalhead Options</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X49, 0X6E, 0X66, 0X6F, 0X72, 0X6D, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X6F, 0X6E, 0X20, 0X74, 0X68, 0X65, 0X20, 0X73, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X68, 0X65, 0X61, 0X64, 0X73, 0X20, 0X66, 0X6F, 0X72, 0X20, 0X74, 0X68, 0X69, 0X73, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X2E, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>Information on the signal heads for this TurnoutBoss.</description>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X31, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>1</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X30, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>0</default>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X30, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>0</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X61, 0X74, 0X20, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X70, 0X6F, 0X69, 0X6E, 0X74, 0X73, 0X20, 0X69, 0X73, 0X20, 0X61, 0X20, 0X64, 0X6F, 0X75, 0X62, 0X6C, 0X65, 0X20, 0X68, 0X65, 0X61, 0X64, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Signal at turnout points is a double head</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X31, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>1</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X61, 0X74, 0X20, 0X74, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X70, 0X6F, 0X69, 0X6E, 0X74, 0X73, 0X20, 0X69, 0X73, 0X20, 0X61, 0X20, 0X73, 0X69, 0X6E, 0X67, 0X6C, 0X65, 0X20, 0X68, 0X65, 0X61, 0X64, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Signal at turnout points is a single head</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X2F, 0X73, 0X65, 0X67, 0X6D, 0X65, 0X6E, 0X74, 0X3E,    // </segment>
        0X3C, 0X73, 0X65, 0X67, 0X6D, 0X65, 0X6E, 0X74, 0X20, 0X6F, 0X72, 0X69, 0X67, 0X69, 0X6E, 0X3D, 0X22, 0X31, 0X37, 0X30, 0X22, 0X20, 0X73, 0X70, 0X61, 0X63, 0X65, 0X3D, 0X22, 0X32, 0X35, 0X33, 0X22, 0X3E,    // <segment origin="170" space="253">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X48, 0X61, 0X72, 0X64, 0X77, 0X61, 0X72, 0X65, 0X20, 0X43, 0X6F, 0X6E, 0X66, 0X69, 0X67, 0X75, 0X72, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Hardware Configuration</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>
        0X49, 0X6E, 0X66, 0X6F, 0X72, 0X6D, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X61, 0X62, 0X6F, 0X75, 0X74, 0X20, 0X74, 0X68, 0X65, 0X20, 0X68, 0X61, 0X72, 0X64, 0X77, 0X61, 0X72, 0X65, 0X20, 0X63, 0X6F, 0X6E, 0X6E, 0X65, 0X63, 0X74, 0X65, 0X64, 0X20, 0X74, 0X6F, 0X20, 0X79, 0X6F, 0X75, 0X72, 0X20, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X42, 0X6F, 0X73, 0X73, 0X2E,    // Information about the hardware connected to your TurnoutBoss.
        0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </description>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X68, 0X65, 0X61, 0X64, 0X20, 0X6C, 0X69, 0X67, 0X68, 0X74, 0X20, 0X63, 0X6F, 0X6E, 0X66, 0X69, 0X67, 0X75, 0X72, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signalhead light configuration</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X44, 0X65, 0X66, 0X69, 0X6E, 0X65, 0X20, 0X74, 0X68, 0X65, 0X20, 0X73, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X68, 0X65, 0X61, 0X64, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X63, 0X6F, 0X6E, 0X66, 0X69, 0X67, 0X75, 0X72, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>Define the signal head LED configuration</description>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X20, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X41, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name> Signal A</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X54, 0X68, 0X65, 0X20, 0X73, 0X74, 0X72, 0X61, 0X69, 0X67, 0X68, 0X74, 0X20, 0X74, 0X68, 0X72, 0X6F, 0X75, 0X67, 0X68, 0X20, 0X70, 0X61, 0X74, 0X68, 0X20, 0X73, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>The straight through path signal</description>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X31, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>1</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X30, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>0</default>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X30, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>0</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X68, 0X72, 0X65, 0X65, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X20, 0X28, 0X69, 0X6E, 0X64, 0X69, 0X76, 0X69, 0X64, 0X75, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Three outputs (individual LED outputs)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X31, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>1</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X77, 0X6F, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X20, 0X28, 0X62, 0X69, 0X2D, 0X64, 0X69, 0X72, 0X65, 0X63, 0X74, 0X69, 0X6F, 0X6E, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X77, 0X69, 0X74, 0X68, 0X20, 0X6D, 0X75, 0X6C, 0X74, 0X69, 0X70, 0X6C, 0X65, 0X78, 0X65, 0X64, 0X20, 0X79, 0X65, 0X6C, 0X6C, 0X6F, 0X77, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Two outputs (bi-directional LED with multiplexed yellow)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X20, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X42, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name> Signal B</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X54, 0X68, 0X65, 0X20, 0X64, 0X69, 0X76, 0X65, 0X72, 0X67, 0X69, 0X6E, 0X67, 0X20, 0X70, 0X61, 0X74, 0X68, 0X20, 0X73, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>The diverging path signal</description>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X31, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>1</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X30, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>0</default>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X30, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>0</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X68, 0X72, 0X65, 0X65, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X20, 0X28, 0X69, 0X6E, 0X64, 0X69, 0X76, 0X69, 0X64, 0X75, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Three outputs (individual LED outputs)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X31, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>1</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X77, 0X6F, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X20, 0X28, 0X62, 0X69, 0X2D, 0X64, 0X69, 0X72, 0X65, 0X63, 0X74, 0X69, 0X6F, 0X6E, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X77, 0X69, 0X74, 0X68, 0X20, 0X6D, 0X75, 0X6C, 0X74, 0X69, 0X70, 0X6C, 0X65, 0X78, 0X65, 0X64, 0X20, 0X79, 0X65, 0X6C, 0X6C, 0X6F, 0X77, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Two outputs (bi-directional LED with multiplexed yellow)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X20, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X43, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name> Signal C</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X54, 0X68, 0X65, 0X20, 0X73, 0X74, 0X72, 0X61, 0X69, 0X67, 0X68, 0X74, 0X20, 0X74, 0X68, 0X72, 0X6F, 0X75, 0X67, 0X68, 0X20, 0X70, 0X61, 0X74, 0X68, 0X20, 0X73, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X61, 0X74, 0X20, 0X74, 0X68, 0X65, 0X20, 0X70, 0X6F, 0X69, 0X6E, 0X74, 0X73, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>The straight through path signal at the points</description>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X31, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>1</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X30, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>0</default>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X30, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>0</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X68, 0X72, 0X65, 0X65, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X20, 0X28, 0X69, 0X6E, 0X64, 0X69, 0X76, 0X69, 0X64, 0X75, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Three outputs (individual LED outputs)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X31, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>1</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X77, 0X6F, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X20, 0X28, 0X62, 0X69, 0X2D, 0X64, 0X69, 0X72, 0X65, 0X63, 0X74, 0X69, 0X6F, 0X6E, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X77, 0X69, 0X74, 0X68, 0X20, 0X6D, 0X75, 0X6C, 0X74, 0X69, 0X70, 0X6C, 0X65, 0X78, 0X65, 0X64, 0X20, 0X79, 0X65, 0X6C, 0X6C, 0X6F, 0X77, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Two outputs (bi-directional LED with multiplexed yellow)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X20, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X44, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name> Signal D</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X54, 0X68, 0X65, 0X20, 0X64, 0X69, 0X76, 0X65, 0X72, 0X67, 0X69, 0X6E, 0X67, 0X20, 0X70, 0X61, 0X74, 0X68, 0X20, 0X73, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X61, 0X74, 0X20, 0X74, 0X68, 0X65, 0X20, 0X70, 0X6F, 0X69, 0X6E, 0X74, 0X73, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>The diverging path signal at the points</description>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X31, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>1</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X30, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>0</default>
        0X3C, 0X6D, 0X61, 0X70, 0X3E,    // <map>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X30, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>0</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X68, 0X72, 0X65, 0X65, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X20, 0X28, 0X69, 0X6E, 0X64, 0X69, 0X76, 0X69, 0X64, 0X75, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Three outputs (individual LED outputs)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <relation>
        0X3C, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E, 0X31, 0X3C, 0X2F, 0X70, 0X72, 0X6F, 0X70, 0X65, 0X72, 0X74, 0X79, 0X3E,    // <property>1</property>
        0X3C, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E, 0X54, 0X77, 0X6F, 0X20, 0X6F, 0X75, 0X74, 0X70, 0X75, 0X74, 0X73, 0X20, 0X28, 0X62, 0X69, 0X2D, 0X64, 0X69, 0X72, 0X65, 0X63, 0X74, 0X69, 0X6F, 0X6E, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X77, 0X69, 0X74, 0X68, 0X20, 0X6D, 0X75, 0X6C, 0X74, 0X69, 0X70, 0X6C, 0X65, 0X78, 0X65, 0X64, 0X20, 0X79, 0X65, 0X6C, 0X6C, 0X6F, 0X77, 0X29, 0X3C, 0X2F, 0X76, 0X61, 0X6C, 0X75, 0X65, 0X3E,    // <value>Two outputs (bi-directional LED with multiplexed yellow)</value>
        0X3C, 0X2F, 0X72, 0X65, 0X6C, 0X61, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // </relation>
        0X3C, 0X2F, 0X6D, 0X61, 0X70, 0X3E,    // </map>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X72, 0X61, 0X63, 0X6B, 0X20, 0X44, 0X65, 0X74, 0X65, 0X63, 0X74, 0X6F, 0X72, 0X20, 0X53, 0X65, 0X6E, 0X73, 0X69, 0X74, 0X69, 0X76, 0X69, 0X74, 0X79, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Track Detector Sensitivity</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X41, 0X64, 0X6A, 0X75, 0X73, 0X74, 0X73, 0X20, 0X74, 0X68, 0X65, 0X20, 0X67, 0X61, 0X69, 0X6E, 0X20, 0X6F, 0X66, 0X20, 0X74, 0X68, 0X65, 0X20, 0X74, 0X72, 0X61, 0X63, 0X6B, 0X20, 0X64, 0X65, 0X74, 0X65, 0X63, 0X74, 0X6F, 0X72, 0X73, 0X20, 0X66, 0X6F, 0X72, 0X20, 0X64, 0X65, 0X74, 0X65, 0X63, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X73, 0X65, 0X6E, 0X73, 0X69, 0X74, 0X69, 0X76, 0X69, 0X74, 0X79, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>Adjusts the gain of the track detectors for detection sensitivity</description>
        0X3C, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E, 0X3C, 0X76, 0X69, 0X73, 0X69, 0X62, 0X69, 0X6C, 0X69, 0X74, 0X79, 0X20, 0X68, 0X69, 0X64, 0X65, 0X61, 0X62, 0X6C, 0X65, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X20, 0X68, 0X69, 0X64, 0X64, 0X65, 0X6E, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X2F, 0X3E, 0X3C, 0X2F, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // <hints><visibility hideable="yes" hidden="yes"/></hints>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X44, 0X65, 0X74, 0X65, 0X63, 0X74, 0X6F, 0X72, 0X20, 0X31, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Detector 1</name>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X36, 0X33, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>63</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X33, 0X31, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>31</default>
        0X3C, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // <hints>
        0X3C, 0X73, 0X6C, 0X69, 0X64, 0X65, 0X72, 0X20, 0X74, 0X69, 0X63, 0X6B, 0X53, 0X70, 0X61, 0X63, 0X69, 0X6E, 0X67, 0X3D, 0X22, 0X38, 0X22, 0X20, 0X69, 0X6D, 0X6D, 0X65, 0X64, 0X69, 0X61, 0X74, 0X65, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X20, 0X73, 0X68, 0X6F, 0X77, 0X56, 0X61, 0X6C, 0X75, 0X65, 0X3D, 0X22, 0X74, 0X72, 0X75, 0X65, 0X22, 0X2F, 0X3E,    // <slider tickSpacing="8" immediate="yes" showValue="true"/>
        0X3C, 0X2F, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // </hints>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X44, 0X65, 0X74, 0X65, 0X63, 0X74, 0X6F, 0X72, 0X20, 0X32, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Detector 2</name>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X36, 0X33, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>63</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X33, 0X31, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>31</default>
        0X3C, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // <hints>
        0X3C, 0X73, 0X6C, 0X69, 0X64, 0X65, 0X72, 0X20, 0X74, 0X69, 0X63, 0X6B, 0X53, 0X70, 0X61, 0X63, 0X69, 0X6E, 0X67, 0X3D, 0X22, 0X38, 0X22, 0X20, 0X69, 0X6D, 0X6D, 0X65, 0X64, 0X69, 0X61, 0X74, 0X65, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X20, 0X73, 0X68, 0X6F, 0X77, 0X56, 0X61, 0X6C, 0X75, 0X65, 0X3D, 0X22, 0X74, 0X72, 0X75, 0X65, 0X22, 0X2F, 0X3E,    // <slider tickSpacing="8" immediate="yes" showValue="true"/>
        0X3C, 0X2F, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // </hints>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X44, 0X65, 0X74, 0X65, 0X63, 0X74, 0X6F, 0X72, 0X20, 0X33, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Detector 3</name>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X36, 0X33, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>63</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X33, 0X31, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>31</default>
        0X3C, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // <hints>
        0X3C, 0X73, 0X6C, 0X69, 0X64, 0X65, 0X72, 0X20, 0X74, 0X69, 0X63, 0X6B, 0X53, 0X70, 0X61, 0X63, 0X69, 0X6E, 0X67, 0X3D, 0X22, 0X38, 0X22, 0X20, 0X69, 0X6D, 0X6D, 0X65, 0X64, 0X69, 0X61, 0X74, 0X65, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X20, 0X73, 0X68, 0X6F, 0X77, 0X56, 0X61, 0X6C, 0X75, 0X65, 0X3D, 0X22, 0X74, 0X72, 0X75, 0X65, 0X22, 0X2F, 0X3E,    // <slider tickSpacing="8" immediate="yes" showValue="true"/>
        0X3C, 0X2F, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // </hints>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X42, 0X72, 0X69, 0X67, 0X68, 0X74, 0X6E, 0X65, 0X73, 0X73, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal LED Brightness</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X41, 0X64, 0X6A, 0X75, 0X73, 0X74, 0X73, 0X20, 0X74, 0X68, 0X65, 0X20, 0X63, 0X75, 0X72, 0X72, 0X65, 0X6E, 0X74, 0X20, 0X74, 0X6F, 0X20, 0X74, 0X68, 0X65, 0X20, 0X73, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X68, 0X65, 0X61, 0X64, 0X20, 0X4C, 0X45, 0X44, 0X73, 0X20, 0X74, 0X6F, 0X20, 0X63, 0X6F, 0X6E, 0X74, 0X72, 0X6F, 0X6C, 0X20, 0X62, 0X72, 0X69, 0X67, 0X68, 0X74, 0X6E, 0X65, 0X73, 0X73, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>Adjusts the current to the signal head LEDs to control brightness</description>
        0X3C, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E, 0X3C, 0X76, 0X69, 0X73, 0X69, 0X62, 0X69, 0X6C, 0X69, 0X74, 0X79, 0X20, 0X68, 0X69, 0X64, 0X65, 0X61, 0X62, 0X6C, 0X65, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X20, 0X68, 0X69, 0X64, 0X64, 0X65, 0X6E, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X2F, 0X3E, 0X3C, 0X2F, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // <hints><visibility hideable="yes" hidden="yes"/></hints>
        0X3C, 0X69, 0X6E, 0X74, 0X20, 0X73, 0X69, 0X7A, 0X65, 0X3D, 0X22, 0X31, 0X22, 0X3E,    // <int size="1">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X4C, 0X45, 0X44, 0X20, 0X42, 0X72, 0X69, 0X67, 0X68, 0X74, 0X6E, 0X65, 0X73, 0X73, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal LED Brightness</name>
        0X3C, 0X6D, 0X69, 0X6E, 0X3E, 0X30, 0X3C, 0X2F, 0X6D, 0X69, 0X6E, 0X3E,    // <min>0</min>
        0X3C, 0X6D, 0X61, 0X78, 0X3E, 0X36, 0X33, 0X3C, 0X2F, 0X6D, 0X61, 0X78, 0X3E,    // <max>63</max>
        0X3C, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E, 0X33, 0X31, 0X3C, 0X2F, 0X64, 0X65, 0X66, 0X61, 0X75, 0X6C, 0X74, 0X3E,    // <default>31</default>
        0X3C, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // <hints>
        0X3C, 0X73, 0X6C, 0X69, 0X64, 0X65, 0X72, 0X20, 0X74, 0X69, 0X63, 0X6B, 0X53, 0X70, 0X61, 0X63, 0X69, 0X6E, 0X67, 0X3D, 0X22, 0X38, 0X22, 0X20, 0X69, 0X6D, 0X6D, 0X65, 0X64, 0X69, 0X61, 0X74, 0X65, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X20, 0X73, 0X68, 0X6F, 0X77, 0X56, 0X61, 0X6C, 0X75, 0X65, 0X3D, 0X22, 0X74, 0X72, 0X75, 0X65, 0X22, 0X2F, 0X3E,    // <slider tickSpacing="8" immediate="yes" showValue="true"/>
        0X3C, 0X2F, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // </hints>
        0X3C, 0X2F, 0X69, 0X6E, 0X74, 0X3E,    // </int>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X2F, 0X73, 0X65, 0X67, 0X6D, 0X65, 0X6E, 0X74, 0X3E,    // </segment>
        0X3C, 0X73, 0X65, 0X67, 0X6D, 0X65, 0X6E, 0X74, 0X20, 0X6F, 0X72, 0X69, 0X67, 0X69, 0X6E, 0X3D, 0X22, 0X32, 0X30, 0X30, 0X22, 0X20, 0X73, 0X70, 0X61, 0X63, 0X65, 0X3D, 0X22, 0X32, 0X35, 0X33, 0X22, 0X3E,    // <segment origin="200" space="253">
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X41, 0X64, 0X76, 0X61, 0X6E, 0X63, 0X65, 0X64, 0X20, 0X4F, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X73, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Advanced Options</name>
        0X3C, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // <group>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X50, 0X72, 0X6F, 0X64, 0X75, 0X63, 0X65, 0X72, 0X73, 0X20, 0X61, 0X6E, 0X64, 0X20, 0X43, 0X6F, 0X6E, 0X73, 0X75, 0X6D, 0X65, 0X72, 0X73, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Producers and Consumers</name>
        0X3C, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E, 0X43, 0X6F, 0X70, 0X79, 0X20, 0X61, 0X6E, 0X64, 0X20, 0X70, 0X61, 0X73, 0X74, 0X65, 0X20, 0X74, 0X68, 0X65, 0X73, 0X65, 0X20, 0X74, 0X6F, 0X20, 0X6F, 0X74, 0X68, 0X65, 0X72, 0X20, 0X6E, 0X6F, 0X64, 0X65, 0X73, 0X20, 0X74, 0X6F, 0X20, 0X6D, 0X61, 0X6B, 0X65, 0X20, 0X63, 0X6F, 0X6E, 0X6E, 0X65, 0X63, 0X74, 0X69, 0X6F, 0X6E, 0X73, 0X3C, 0X2F, 0X64, 0X65, 0X73, 0X63, 0X72, 0X69, 0X70, 0X74, 0X69, 0X6F, 0X6E, 0X3E,    // <description>Copy and paste these to other nodes to make connections</description>
        0X3C, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E, 0X3C, 0X76, 0X69, 0X73, 0X69, 0X62, 0X69, 0X6C, 0X69, 0X74, 0X79, 0X20, 0X68, 0X69, 0X64, 0X65, 0X61, 0X62, 0X6C, 0X65, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X20, 0X68, 0X69, 0X64, 0X64, 0X65, 0X6E, 0X3D, 0X22, 0X79, 0X65, 0X73, 0X22, 0X2F, 0X3E, 0X3C, 0X72, 0X65, 0X61, 0X64, 0X4F, 0X6E, 0X6C, 0X79, 0X2F, 0X3E, 0X3C, 0X2F, 0X68, 0X69, 0X6E, 0X74, 0X73, 0X3E,    // <hints><visibility hideable="yes" hidden="yes"/><readOnly/></hints>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X4D, 0X61, 0X69, 0X6E, 0X20, 0X4C, 0X65, 0X66, 0X74, 0X20, 0X4F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Main Left Occupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X4D, 0X61, 0X69, 0X6E, 0X20, 0X4C, 0X65, 0X66, 0X74, 0X20, 0X55, 0X6E, 0X6F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Main Left Unoccupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X4C, 0X65, 0X66, 0X74, 0X20, 0X4F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Left Occupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X4C, 0X65, 0X66, 0X74, 0X20, 0X55, 0X6E, 0X6F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Left Unoccupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X4D, 0X61, 0X69, 0X6E, 0X20, 0X43, 0X65, 0X6E, 0X74, 0X65, 0X72, 0X20, 0X4F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Main Center Occupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X4D, 0X61, 0X69, 0X6E, 0X20, 0X43, 0X65, 0X6E, 0X74, 0X65, 0X72, 0X20, 0X55, 0X6E, 0X6F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Main Center Unoccupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X64, 0X69, 0X6E, 0X67, 0X20, 0X4F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Siding Occupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X64, 0X69, 0X6E, 0X67, 0X20, 0X55, 0X6E, 0X6F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Siding Unoccupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X52, 0X69, 0X67, 0X68, 0X74, 0X20, 0X4F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Right Occupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X52, 0X69, 0X67, 0X68, 0X74, 0X20, 0X55, 0X6E, 0X6F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Right Unoccupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X4D, 0X61, 0X69, 0X6E, 0X20, 0X52, 0X69, 0X67, 0X68, 0X74, 0X20, 0X4F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Main Right Occupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X4D, 0X61, 0X69, 0X6E, 0X20, 0X52, 0X69, 0X67, 0X68, 0X74, 0X20, 0X55, 0X6E, 0X6F, 0X63, 0X63, 0X75, 0X70, 0X69, 0X65, 0X64, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Main Right Unoccupied</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X43, 0X6F, 0X6D, 0X6D, 0X61, 0X6E, 0X64, 0X20, 0X4E, 0X6F, 0X72, 0X6D, 0X61, 0X6C, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Command Normal (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X43, 0X6F, 0X6D, 0X6D, 0X61, 0X6E, 0X64, 0X20, 0X44, 0X69, 0X76, 0X65, 0X72, 0X67, 0X69, 0X6E, 0X67, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Command Diverging (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X46, 0X65, 0X65, 0X64, 0X62, 0X61, 0X63, 0X6B, 0X20, 0X4E, 0X6F, 0X72, 0X6D, 0X61, 0X6C, 0X20, 0X41, 0X63, 0X74, 0X69, 0X76, 0X65, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Feedback Normal Active (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X46, 0X65, 0X65, 0X64, 0X62, 0X61, 0X63, 0X6B, 0X20, 0X4E, 0X6F, 0X72, 0X6D, 0X61, 0X6C, 0X20, 0X49, 0X6E, 0X61, 0X63, 0X74, 0X69, 0X76, 0X65, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Feedback Normal Inactive (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X46, 0X65, 0X65, 0X64, 0X62, 0X61, 0X63, 0X6B, 0X20, 0X44, 0X69, 0X76, 0X65, 0X72, 0X67, 0X69, 0X6E, 0X67, 0X20, 0X41, 0X63, 0X74, 0X69, 0X76, 0X65, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Feedback Diverging Active (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X46, 0X65, 0X65, 0X64, 0X62, 0X61, 0X63, 0X6B, 0X20, 0X44, 0X69, 0X76, 0X65, 0X72, 0X67, 0X69, 0X6E, 0X67, 0X20, 0X49, 0X6E, 0X61, 0X63, 0X74, 0X69, 0X76, 0X65, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Feedback Diverging Inactive (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X42, 0X75, 0X74, 0X74, 0X6F, 0X6E, 0X20, 0X4E, 0X6F, 0X72, 0X6D, 0X61, 0X6C, 0X20, 0X4F, 0X70, 0X65, 0X6E, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Button Normal Open (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X42, 0X75, 0X74, 0X74, 0X6F, 0X6E, 0X20, 0X4E, 0X6F, 0X72, 0X6D, 0X61, 0X6C, 0X20, 0X43, 0X6C, 0X6F, 0X73, 0X65, 0X64, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Button Normal Closed (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X42, 0X75, 0X74, 0X74, 0X6F, 0X6E, 0X20, 0X44, 0X69, 0X76, 0X65, 0X72, 0X67, 0X69, 0X6E, 0X67, 0X20, 0X4F, 0X70, 0X65, 0X6E, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Button Diverging Open (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X42, 0X75, 0X74, 0X74, 0X6F, 0X6E, 0X20, 0X44, 0X69, 0X76, 0X65, 0X72, 0X67, 0X69, 0X6E, 0X67, 0X20, 0X43, 0X6C, 0X6F, 0X73, 0X65, 0X64, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Button Diverging Closed (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X4F, 0X62, 0X73, 0X65, 0X72, 0X76, 0X65, 0X64, 0X20, 0X4E, 0X6F, 0X72, 0X6D, 0X61, 0X6C, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Observed Normal (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X4F, 0X62, 0X73, 0X65, 0X72, 0X76, 0X65, 0X64, 0X20, 0X44, 0X69, 0X76, 0X65, 0X72, 0X67, 0X69, 0X6E, 0X67, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Observed Diverging (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X54, 0X75, 0X72, 0X6E, 0X6F, 0X75, 0X74, 0X20, 0X4F, 0X62, 0X73, 0X65, 0X72, 0X76, 0X65, 0X64, 0X20, 0X49, 0X6E, 0X20, 0X4D, 0X6F, 0X74, 0X69, 0X6F, 0X6E, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Turnout Observed In Motion (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X41, 0X20, 0X52, 0X65, 0X64, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal A Red (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X41, 0X20, 0X59, 0X65, 0X6C, 0X6C, 0X6F, 0X77, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal A Yellow (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X41, 0X20, 0X47, 0X72, 0X65, 0X65, 0X6E, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal A Green (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X41, 0X20, 0X44, 0X61, 0X72, 0X6B, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal A Dark (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X42, 0X20, 0X52, 0X65, 0X64, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal B Red (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X42, 0X20, 0X59, 0X65, 0X6C, 0X6C, 0X6F, 0X77, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal B Yellow (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X42, 0X20, 0X47, 0X72, 0X65, 0X65, 0X6E, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal B Green (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X42, 0X20, 0X44, 0X61, 0X72, 0X6B, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal B Dark (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X43, 0X20, 0X52, 0X65, 0X64, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal C Red (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X43, 0X20, 0X59, 0X65, 0X6C, 0X6C, 0X6F, 0X77, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal C Yellow (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X43, 0X20, 0X47, 0X72, 0X65, 0X65, 0X6E, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal C Green (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X43, 0X20, 0X44, 0X61, 0X72, 0X6B, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal C Dark (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X44, 0X20, 0X52, 0X65, 0X64, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal D Red (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X44, 0X20, 0X59, 0X65, 0X6C, 0X6C, 0X6F, 0X77, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal D Yellow (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X44, 0X20, 0X47, 0X72, 0X65, 0X65, 0X6E, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal D Green (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X44, 0X20, 0X44, 0X61, 0X72, 0X6B, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal D Dark (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X41, 0X20, 0X53, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State A Stop (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X41, 0X20, 0X4E, 0X6F, 0X6E, 0X73, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State A Nonstop (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X42, 0X20, 0X53, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State B Stop (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X42, 0X20, 0X4E, 0X6F, 0X6E, 0X73, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State B Nonstop (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X43, 0X44, 0X20, 0X53, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State CD Stop (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X43, 0X44, 0X20, 0X4E, 0X6F, 0X6E, 0X73, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State CD Nonstop (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X56, 0X69, 0X74, 0X61, 0X6C, 0X20, 0X4C, 0X6F, 0X67, 0X69, 0X63, 0X20, 0X48, 0X65, 0X6C, 0X64, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Vital Logic Held (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X56, 0X69, 0X74, 0X61, 0X6C, 0X20, 0X4C, 0X6F, 0X67, 0X69, 0X63, 0X20, 0X43, 0X6C, 0X65, 0X61, 0X72, 0X65, 0X64, 0X20, 0X4C, 0X65, 0X66, 0X74, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Vital Logic Cleared Left (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X56, 0X69, 0X74, 0X61, 0X6C, 0X20, 0X4C, 0X6F, 0X67, 0X69, 0X63, 0X20, 0X43, 0X6C, 0X65, 0X61, 0X72, 0X65, 0X64, 0X20, 0X52, 0X69, 0X67, 0X68, 0X74, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Vital Logic Cleared Right (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X56, 0X69, 0X74, 0X61, 0X6C, 0X20, 0X4C, 0X6F, 0X67, 0X69, 0X63, 0X20, 0X43, 0X6C, 0X65, 0X61, 0X72, 0X65, 0X64, 0X20, 0X42, 0X6F, 0X74, 0X68, 0X20, 0X28, 0X43, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Vital Logic Cleared Both (C)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X41, 0X20, 0X53, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State A Stop (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X41, 0X20, 0X4E, 0X6F, 0X6E, 0X73, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State A Nonstop (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X42, 0X20, 0X53, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State B Stop (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X42, 0X20, 0X4E, 0X6F, 0X6E, 0X73, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State B Nonstop (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X43, 0X44, 0X20, 0X53, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State CD Stop (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // <eventid>
        0X3C, 0X6E, 0X61, 0X6D, 0X65, 0X3E, 0X53, 0X69, 0X67, 0X6E, 0X61, 0X6C, 0X20, 0X53, 0X74, 0X61, 0X74, 0X65, 0X20, 0X43, 0X44, 0X20, 0X4E, 0X6F, 0X6E, 0X73, 0X74, 0X6F, 0X70, 0X20, 0X28, 0X50, 0X29, 0X3C, 0X2F, 0X6E, 0X61, 0X6D, 0X65, 0X3E,    // <name>Signal State CD Nonstop (P)</name>
        0X3C, 0X2F, 0X65, 0X76, 0X65, 0X6E, 0X74, 0X69, 0X64, 0X3E,    // </eventid>
        0X3C, 0X2F, 0X67, 0X72, 0X6F, 0X75, 0X70, 0X3E,    // </group>
        0X3C, 0X2F, 0X73, 0X65, 0X67, 0X6D, 0X65, 0X6E, 0X74, 0X3E,    // </segment>
        0X3C, 0X2F, 0X63, 0X64, 0X69, 0X3E, 0X00   // </cdi>
    },


};
