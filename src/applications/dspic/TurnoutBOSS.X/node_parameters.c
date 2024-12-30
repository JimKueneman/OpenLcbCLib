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

#include "node_parameters.h"
#include "../../../openlcb/openlcb_defines.h"

//#define SUPPORT_TRACTION

const node_parameters_t NodeParameters_main_node = {

    .consumer_count = 4,
    .producer_count = 4,


    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Mustangpeak Engineering",
    .snip.model = "TurnoutBOSS",
    .snip.hardware_version = "Ver A",
    .snip.software_version = "0.1",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

#ifdef SUPPORT_TRACTION
    .protocol_support = (PSI_DATAGRAM | PSI_MEMORY_CONFIGURATION | PSI_EVENT_EXCHANGE | PSI_EVENT_EXCHANGE | PSI_ABBREVIATED_DEFAULT_CDI | PSI_SIMPLE_NODE_INFORMATION | PSI_CONFIGURATION_DESCRIPTION_INFO | PSI_TRAIN_CONTROL | PSI_FUNCTION_DESCRIPTION | PSI_FUNCTION_CONFIGURATION),
#else
    .protocol_support = (PSI_DATAGRAM | PSI_MEMORY_CONFIGURATION | PSI_EVENT_EXCHANGE | PSI_EVENT_EXCHANGE | PSI_ABBREVIATED_DEFAULT_CDI | PSI_SIMPLE_NODE_INFORMATION | PSI_CONFIGURATION_DESCRIPTION_INFO),
#endif


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
    .address_space_configuration_definition.highest_address = 2395, // this is the length of the .cdi file byte array in this initialization structure
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
    .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
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

#ifdef SUPPORT_TRACTION
    // Space 0xFA
    .address_space_train_function_definition.read_only = 1,
    .address_space_train_function_definition.present = 1,
    .address_space_train_function_definition.low_address_valid = 0, // assume the low address starts at 0
    .address_space_train_function_definition.low_address = 0, // ignored if low_address_valid is false
    .address_space_train_function_definition.highest_address = 809, // Length of the FDI file in this structure
    .address_space_train_function_definition.address_space = ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO,
    .address_space_train_function_definition.description = "Train function definition info",


    // Space 0xF9
    .address_space_train_function_config_memory.read_only = 0,
    .address_space_train_function_config_memory.present = 1,
    .address_space_train_function_config_memory.low_address_valid = 0, // assume the low address starts at 0
    .address_space_train_function_config_memory.low_address = 0x100, // ignored if low_address_valid is false
    .address_space_train_function_config_memory.highest_address = 0x200, // This is important for multi node applications as the memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_train_function_config_memory.address_space = ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY,
    .address_space_train_function_config_memory.description = "Train function configuration memory storage",
#endif

    .cdi =
    {
    0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x31, 0x2E, 0x30, 0x22, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x3D, 0x22, 0x55, 0x54, 0x46, 0x2D, 0x38, 0x22, 0x3F, 0x3E,    // <?xml version="1.0" encoding="UTF-8"?>
    0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x2D, 0x73, 0x74, 0x79, 0x6C, 0x65, 0x73, 0x68, 0x65, 0x65, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3D, 0x22, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x78, 0x73, 0x6C, 0x22, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x74, 0x72, 0x75, 0x6E, 0x6B, 0x2F, 0x70, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x73, 0x2F, 0x78, 0x6D, 0x6C, 0x2F, 0x78, 0x73, 0x6C, 0x74, 0x2F, 0x63, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x6C, 0x22, 0x3F, 0x3E,    // <?xml-stylesheet type="text/xsl" href="http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl"?>
    0x3C, 0x63, 0x64, 0x69, 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x73, 0x69, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x31, 0x2F, 0x58, 0x4D, 0x4C, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2D, 0x69, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x22, 0x20, 0x78, 0x73, 0x69, 0x3A, 0x6E, 0x6F, 0x4E, 0x61, 0x6D, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x4C, 0x6F, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2F, 0x63, 0x64, 0x69, 0x2F, 0x31, 0x2F, 0x33, 0x2F, 0x63, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x64, 0x22, 0x3E,    // <cdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/cdi/1/3/cdi.xsd">
    0x3C, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <identification>
    0x3C, 0x6D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x3E, 0x4D, 0x75, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B, 0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E, 0x65, 0x65, 0x72, 0x69, 0x6E, 0x67, 0x3C, 0x2F, 0x6D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x3E,    // <manufacturer>Mustangpeak Engineering</manufacturer>
    0x3C, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x3E, 0x54, 0x75, 0x72, 0x6E, 0x6F, 0x75, 0x74, 0x42, 0x4F, 0x53, 0x53, 0x3C, 0x2F, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x3E,    // <model>TurnoutBOSS</model>
    0x3C, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, 0x56, 0x65, 0x72, 0x20, 0x41, 0x3C, 0x2F, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E,    // <hardwareVersion>Ver A</hardwareVersion>
    0x3C, 0x73, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, 0x30, 0x2E, 0x31, 0x3C, 0x2F, 0x73, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E,    // <softwareVersion>0.1</softwareVersion>
    0x3C, 0x6D, 0x61, 0x70, 0x3E,    // <map>
    0x3C, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <relation>
    0x3C, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E,    // <property>Description</property>
    0x3C, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, 0x4D, 0x75, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x4E, 0x6F, 0x64, 0x65, 0x3C, 0x2F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E,    // <value>Mustangpeak Test Node</value>
    0x3C, 0x2F, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // </relation>
    0x3C, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <relation>
    0x3C, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, 0x53, 0x74, 0x61, 0x74, 0x75, 0x73, 0x3C, 0x2F, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E,    // <property>Status</property>
    0x3C, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, 0x50, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x3C, 0x2F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E,    // <value>Prototype</value>
    0x3C, 0x2F, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // </relation>
    0x3C, 0x2F, 0x6D, 0x61, 0x70, 0x3E,    // </map>
    0x3C, 0x2F, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // </identification>
    0x3C, 0x61, 0x63, 0x64, 0x69, 0x2F, 0x3E,    // <acdi/>
    0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x6F, 0x72, 0x69, 0x67, 0x69, 0x6E, 0x3D, 0x22, 0x31, 0x32, 0x38, 0x22, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x22, 0x32, 0x35, 0x33, 0x22, 0x3E,    // <segment origin="128" space="253">
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x54, 0x72, 0x61, 0x63, 0x6B, 0x20, 0x44, 0x65, 0x74, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x20, 0x47, 0x61, 0x69, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Track Detector Gain</name>
    0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x41, 0x64, 0x6A, 0x75, 0x73, 0x74, 0x73, 0x20, 0x74, 0x68, 0x65, 0x20, 0x67, 0x61, 0x69, 0x6E, 0x20, 0x6F, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x72, 0x61, 0x63, 0x6B, 0x20, 0x64, 0x65, 0x74, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x73, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x64, 0x65, 0x74, 0x65, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x65, 0x6E, 0x73, 0x69, 0x74, 0x69, 0x76, 0x69, 0x74, 0x79, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <description>Adjusts the gain of the track detectors for detection sensitivity</description>
    0x3C, 0x69, 0x6E, 0x74, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x32, 0x22, 0x3E,    // <int size="2">
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x41, 0x70, 0x70, 0x72, 0x6F, 0x61, 0x63, 0x68, 0x20, 0x54, 0x72, 0x61, 0x63, 0x6B, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Approach Track</name>
    0x3C, 0x6D, 0x69, 0x6E, 0x3E, 0x20, 0x30, 0x3C, 0x2F, 0x6D, 0x69, 0x6E, 0x3E,    // <min> 0</min>
    0x3C, 0x6D, 0x61, 0x78, 0x3E, 0x36, 0x33, 0x3C, 0x2F, 0x6D, 0x61, 0x78, 0x3E,    // <max>63</max>
    0x3C, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x3E, 0x33, 0x31, 0x3C, 0x2F, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x3E,    // <default>31</default>
    0x3C, 0x2F, 0x69, 0x6E, 0x74, 0x3E,    // </int>
    0x3C, 0x69, 0x6E, 0x74, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x32, 0x22, 0x3E,    // <int size="2">
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x54, 0x68, 0x72, 0x6F, 0x75, 0x67, 0x68, 0x20, 0x54, 0x72, 0x61, 0x63, 0x6B, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Through Track</name>
    0x3C, 0x6D, 0x69, 0x6E, 0x3E, 0x20, 0x30, 0x3C, 0x2F, 0x6D, 0x69, 0x6E, 0x3E,    // <min> 0</min>
    0x3C, 0x6D, 0x61, 0x78, 0x3E, 0x36, 0x33, 0x3C, 0x2F, 0x6D, 0x61, 0x78, 0x3E,    // <max>63</max>
    0x3C, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x3E, 0x33, 0x31, 0x3C, 0x2F, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x3E,    // <default>31</default>
    0x3C, 0x2F, 0x69, 0x6E, 0x74, 0x3E,    // </int>
    0x3C, 0x69, 0x6E, 0x74, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x32, 0x22, 0x3E,    // <int size="2">
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x44, 0x69, 0x76, 0x65, 0x72, 0x67, 0x69, 0x6E, 0x67, 0x20, 0x54, 0x72, 0x61, 0x63, 0x6B, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Diverging Track</name>
    0x3C, 0x6D, 0x69, 0x6E, 0x3E, 0x20, 0x30, 0x3C, 0x2F, 0x6D, 0x69, 0x6E, 0x3E,    // <min> 0</min>
    0x3C, 0x6D, 0x61, 0x78, 0x3E, 0x36, 0x33, 0x3C, 0x2F, 0x6D, 0x61, 0x78, 0x3E,    // <max>63</max>
    0x3C, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x3E, 0x33, 0x31, 0x3C, 0x2F, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74, 0x3E,    // <default>31</default>
    0x3C, 0x2F, 0x69, 0x6E, 0x74, 0x3E,    // </int>
    0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E,    // </segment>
    0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x22, 0x32, 0x35, 0x33, 0x22, 0x3E,    // <segment space="253">
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4E, 0x65, 0x78, 0x74, 0x20, 0x42, 0x6C, 0x6F, 0x63, 0x6B, 0x73, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Next Blocks</name>
    0x3C, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,    // <group>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x41, 0x70, 0x70, 0x72, 0x6F, 0x61, 0x63, 0x68, 0x20, 0x54, 0x72, 0x61, 0x63, 0x6B, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Approach Track</name>
    0x3C, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // <eventid>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x20, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Next block occupied event</name>
    0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x20, 0x49, 0x44, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x69, 0x73, 0x20, 0x66, 0x69, 0x72, 0x65, 0x64, 0x20, 0x77, 0x68, 0x65, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x69, 0x73, 0x20, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <description>Enter the Event ID that is fired when the next block is occupied</description>
    0x3C, 0x2F, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // </eventid>
    0x3C, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // <eventid>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x75, 0x6E, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x20, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Next block unoccupied event</name>
    0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x20, 0x49, 0x44, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x69, 0x73, 0x20, 0x66, 0x69, 0x72, 0x65, 0x64, 0x20, 0x77, 0x68, 0x65, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x69, 0x73, 0x20, 0x75, 0x6E, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <description>Enter the Event ID that is fired when the next block is unoccupied</description>
    0x3C, 0x2F, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // </eventid>
    0x3C, 0x2F, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,    // </group>
    0x3C, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,    // <group>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x53, 0x74, 0x72, 0x61, 0x69, 0x67, 0x68, 0x74, 0x20, 0x54, 0x72, 0x61, 0x63, 0x6B, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Straight Track</name>
    0x3C, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // <eventid>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x20, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Next block occupied event</name>
    0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x20, 0x49, 0x44, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x69, 0x73, 0x20, 0x66, 0x69, 0x72, 0x65, 0x64, 0x20, 0x77, 0x68, 0x65, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x69, 0x73, 0x20, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <description>Enter the Event ID that is fired when the next block is occupied</description>
    0x3C, 0x2F, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // </eventid>
    0x3C, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // <eventid>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x75, 0x6E, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x20, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Next block unoccupied event</name>
    0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x20, 0x49, 0x44, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x69, 0x73, 0x20, 0x66, 0x69, 0x72, 0x65, 0x64, 0x20, 0x77, 0x68, 0x65, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x69, 0x73, 0x20, 0x75, 0x6E, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <description>Enter the Event ID that is fired when the next block is unoccupied</description>
    0x3C, 0x2F, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // </eventid>
    0x3C, 0x2F, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,    // </group>
    0x3C, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,    // <group>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x44, 0x69, 0x76, 0x65, 0x72, 0x67, 0x69, 0x6E, 0x67, 0x20, 0x54, 0x72, 0x61, 0x63, 0x6B, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Diverging Track</name>
    0x3C, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // <eventid>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x20, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Next block occupied event</name>
    0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x20, 0x49, 0x44, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x69, 0x73, 0x20, 0x66, 0x69, 0x72, 0x65, 0x64, 0x20, 0x77, 0x68, 0x65, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x69, 0x73, 0x20, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <description>Enter the Event ID that is fired when the next block is occupied</description>
    0x3C, 0x2F, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // </eventid>
    0x3C, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // <eventid>
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x75, 0x6E, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x20, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Next block unoccupied event</name>
    0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x20, 0x49, 0x44, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x69, 0x73, 0x20, 0x66, 0x69, 0x72, 0x65, 0x64, 0x20, 0x77, 0x68, 0x65, 0x6E, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x78, 0x74, 0x20, 0x62, 0x6C, 0x6F, 0x63, 0x6B, 0x20, 0x69, 0x73, 0x20, 0x75, 0x6E, 0x6F, 0x63, 0x63, 0x75, 0x70, 0x69, 0x65, 0x64, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <description>Enter the Event ID that is fired when the next block is unoccupied</description>
    0x3C, 0x2F, 0x65, 0x76, 0x65, 0x6E, 0x74, 0x69, 0x64, 0x3E,    // </eventid>
    0x3C, 0x2F, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,    // </group>
    0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E,    // </segment>
    0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x6F, 0x72, 0x69, 0x67, 0x69, 0x6E, 0x3D, 0x22, 0x30, 0x22, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x22, 0x32, 0x35, 0x33, 0x22, 0x3E,    // <segment origin="0" space="253">
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4F, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x73, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>Options</name>
    0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x64, 0x65, 0x66, 0x69, 0x6E, 0x65, 0x64, 0x20, 0x6E, 0x6F, 0x64, 0x65, 0x20, 0x69, 0x6E, 0x66, 0x6F, 0x72, 0x6D, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,    // <description>User defined node information</description>
    0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x33, 0x22, 0x3E,    // <string size="63">
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x4E, 0x61, 0x6D, 0x65, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>User Name</name>
    0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E,    // </string>
    0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x34, 0x22, 0x3E,    // <string size="64">
    0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,    // <name>User Description</name>
    0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E,    // </string>
    0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E,    // </segment>
    0x3C, 0x2F, 0x63, 0x64, 0x69, 0x3E, 0x00   // </cdi>

    },

#ifdef SUPPORT_TRACTION

    .fdi =
    {

        0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3D, 0x27, 0x31, 0x2E, 0x30, 0x27, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x3D, 0x27, 0x55, 0x54, 0x46, 0x2D, 0x38, 0x27, 0x3F, 0x3E, // <?xml version='1.0' encoding='UTF-8'?>
        0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x2D, 0x73, 0x74, 0x79, 0x6C, 0x65, 0x73, 0x68, 0x65, 0x65, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3D, 0x27, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x78, 0x73, 0x6C, 0x27, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3D, 0x27, 0x78, 0x73, 0x6C, 0x74, 0x2F, 0x66, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x6C, 0x27, 0x3F, 0x3E, // <?xml-stylesheet type='text/xsl' href='xslt/fdi.xsl'?>
        0x3C, 0x66, 0x64, 0x69, 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x73, 0x69, 0x3D, 0x27, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x31, 0x2F, 0x58, 0x4D, 0x4C, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2D, 0x69, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x27, 0x20, 0x78, 0x73, 0x69, 0x3A, 0x6E, 0x6F, 0x4E, 0x61, 0x6D, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x4C, 0x6F, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3D, 0x27, 0x20, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2F, 0x66, 0x64, 0x69, 0x2F, 0x31, 0x2F, 0x30, 0x2F, 0x66, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x64, 0x27, 0x3E, // <fdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation=' https://openlcb.org/schema/fdi/1/0/fdi.xsd'>
        0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x27, 0x32, 0x34, 0x39, 0x27, 0x3E, 0x3C, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E, 0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x2F, 0x3E, // <segment space='249'><group><name/>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, // <function size='1' kind='binary'>
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x48, 0x65, 0x61, 0x64, 0x6C, 0x69, 0x67, 0x68, 0x74, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Headlight</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x30, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, // <number>0</number>
        0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, // <function size='1' kind='binary'>
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x42, 0x65, 0x6C, 0x6C, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Bell</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x31, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, // <number>1</number>
        0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x6D, 0x6F, 0x6D, 0x65, 0x6E, 0x74, 0x61, 0x72, 0x79, 0x27, 0x3E, // <function size='1' kind='momentary'>
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x48, 0x6F, 0x72, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Horn</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x32, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, // <number>2</number>
        0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, 0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x44, 0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x20, 0x46, 0x30, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <function size='1' kind='binary'><name>Directional F0</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x31, 0x30, 0x30, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <number>100</number></function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, 0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x42, 0x6C, 0x61, 0x6E, 0x6B, 0x20, 0x46, 0x30, 0x20, 0x46, 0x6F, 0x72, 0x77, 0x61, 0x72, 0x64, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <function size='1' kind='binary'><name>Blank F0 Forward</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x31, 0x30, 0x31, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <number>101</number></function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, 0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x42, 0x6C, 0x61, 0x6E, 0x6B, 0x20, 0x46, 0x30, 0x20, 0x52, 0x65, 0x76, 0x65, 0x72, 0x73, 0x65, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <function size='1' kind='binary'><name>Blank F0 Reverse</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x31, 0x30, 0x32, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <number>102</number></function>
        0x3C, 0x2F, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E, 0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E, 0x3C, 0x2F, 0x66, 0x64, 0x69, 0x3E, 0x00 // </group></segment></fdi>

    }

#endif

};
